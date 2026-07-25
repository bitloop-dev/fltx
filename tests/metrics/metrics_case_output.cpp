#include "metrics_case_output.h"

#include <catch2/interfaces/catch_interfaces_config.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <streambuf>

namespace bl::test::metrics
{
    namespace
    {
        class metrics_console_capture_buffer final : public std::streambuf
        {
        public:
            explicit metrics_console_capture_buffer(std::streambuf* output) noexcept
                : output_(output)
            {
            }

            [[nodiscard]] const std::string& text() const noexcept
            {
                return text_;
            }

        private:
            int overflow(int ch) override
            {
                if (ch == traits_type::eof())
                    return traits_type::not_eof(ch);

                const char c = static_cast<char>(ch);
                text_.push_back(c);
                return output_ != nullptr
                    ? output_->sputc(c)
                    : ch;
            }

            std::streamsize xsputn(const char* text, std::streamsize count) override
            {
                if (count > 0)
                    text_.append(text, static_cast<std::size_t>(count));

                return output_ != nullptr
                    ? output_->sputn(text, count)
                    : count;
            }

            int sync() override
            {
                return output_ != nullptr ? output_->pubsync() : 0;
            }

            std::streambuf* output_ = nullptr;
            std::string text_;
        };

        struct metrics_console_capture_state
        {
            std::ostream* stream = nullptr;
            std::streambuf* original = nullptr;
            std::unique_ptr<metrics_console_capture_buffer> capture;
        };

        [[nodiscard]] metrics_console_capture_state& metrics_console_capture()
        {
            static metrics_console_capture_state state;
            return state;
        }

        void start_metrics_console_capture(std::ostream& out)
        {
            if (!metrics_verbose_enabled())
                return;

            metrics_console_capture_state& state = metrics_console_capture();
            if (state.capture != nullptr)
                return;

            state.stream = &out;
            state.original = out.rdbuf();
            state.capture = std::make_unique<metrics_console_capture_buffer>(state.original);
            out.rdbuf(state.capture.get());
        }

        [[nodiscard]] std::string stop_metrics_console_capture()
        {
            metrics_console_capture_state& state = metrics_console_capture();
            if (state.capture == nullptr)
                return {};

            if (state.stream != nullptr)
                state.stream->flush();

            std::string text = state.capture->text();
            if (state.stream != nullptr)
                state.stream->rdbuf(state.original);

            state = {};
            return text;
        }

        void append_html_escaped(std::string& out, char ch)
        {
            switch (ch)
            {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out.push_back(ch); break;
            }
        }

        void append_html_escaped(std::string& out, std::string_view text)
        {
            for (char ch : text)
                append_html_escaped(out, ch);
        }

        [[nodiscard]] int parse_ansi_number(std::string_view text) noexcept
        {
            int value = 0;
            for (char ch : text)
            {
                if (ch < '0' || ch > '9')
                    return -1;
                value = value * 10 + (ch - '0');
            }
            return value;
        }

        [[nodiscard]] std::vector<int> parse_ansi_sgr_codes(std::string_view text)
        {
            std::vector<int> codes;
            if (text.empty())
            {
                codes.push_back(0);
                return codes;
            }

            std::size_t first = 0;
            while (first <= text.size())
            {
                const std::size_t last = text.find(';', first);
                const std::string_view token = last == std::string_view::npos
                    ? text.substr(first)
                    : text.substr(first, last - first);
                codes.push_back(token.empty() ? 0 : parse_ansi_number(token));

                if (last == std::string_view::npos)
                    break;
                first = last + 1;
            }
            return codes;
        }

        [[nodiscard]] std::string ansi_rgb(int r, int g, int b)
        {
            char buffer[8] = {};
            std::snprintf(
                buffer,
                sizeof(buffer),
                "#%02X%02X%02X",
                std::clamp(r, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(b, 0, 255));
            return std::string(buffer);
        }

        struct html_ansi_style
        {
            std::string foreground;
            std::string background;
            bool span_open = false;
        };

        void reopen_html_span(std::string& out, html_ansi_style& style)
        {
            if (style.span_open)
            {
                out += "</span>";
                style.span_open = false;
            }

            if (style.foreground.empty() && style.background.empty())
                return;

            out += "<span style=\"";
            if (!style.foreground.empty())
            {
                out += "color:";
                out += style.foreground;
                out += ';';
            }
            if (!style.background.empty())
            {
                out += "background-color:";
                out += style.background;
                out += ';';
            }
            out += "\">";
            style.span_open = true;
        }

        void apply_ansi_sgr_codes(std::string& out, html_ansi_style& style, const std::vector<int>& codes)
        {
            html_ansi_style next = style;
            next.span_open = style.span_open;

            for (std::size_t i = 0; i < codes.size(); ++i)
            {
                const int code = codes[i];
                switch (code)
                {
                case 0:
                    next.foreground.clear();
                    next.background.clear();
                    break;
                case 31:
                    next.foreground = "#EF4444";
                    break;
                case 32:
                    next.foreground = "#22C55E";
                    break;
                case 39:
                    next.foreground.clear();
                    break;
                case 49:
                    next.background.clear();
                    break;
                case 38:
                case 48:
                    if (i + 4 < codes.size() && codes[i + 1] == 2)
                    {
                        std::string color = ansi_rgb(codes[i + 2], codes[i + 3], codes[i + 4]);
                        if (code == 38)
                            next.foreground = std::move(color);
                        else
                            next.background = std::move(color);
                        i += 4;
                    }
                    break;
                default:
                    break;
                }
            }

            if (next.foreground != style.foreground || next.background != style.background)
            {
                style.foreground = std::move(next.foreground);
                style.background = std::move(next.background);
                reopen_html_span(out, style);
            }
        }

        [[nodiscard]] std::string ansi_console_text_to_html(std::string_view text)
        {
            std::string body;
            body.reserve(text.size() * 2);
            html_ansi_style style;

            for (std::size_t i = 0; i < text.size(); ++i)
            {
                if (text[i] == '\033' && i + 1 < text.size() && text[i + 1] == '[')
                {
                    const std::size_t end = text.find('m', i + 2);
                    if (end != std::string_view::npos)
                    {
                        apply_ansi_sgr_codes(
                            body,
                            style,
                            parse_ansi_sgr_codes(text.substr(i + 2, end - (i + 2))));
                        i = end;
                        continue;
                    }
                }

                append_html_escaped(body, text[i]);
            }

            if (style.span_open)
                body += "</span>";

            std::string html;
            html.reserve(body.size() + 1024);
            html += "<!doctype html>\n"
                "<html lang=\"en\">\n"
                "<head>\n"
                "<meta charset=\"utf-8\">\n"
                "<title>fltx metrics console output</title>\n"
                "<style>\n"
                ":root { color-scheme: dark; }\n"
                "body { margin: 0; background: #0f1115; color: #e5e7eb; }\n"
                "pre { box-sizing: border-box; min-height: 100vh; margin: 0; padding: 0 16px; "
                "font: 13px/1 Consolas, \"Cascadia Mono\", \"Courier New\", monospace; "
                "white-space: pre; overflow: auto; }\n"
                "</style>\n"
                "</head>\n"
                "<body>\n"
                "<pre>";
            html += body;
            html += "</pre>\n</body>\n</html>\n";
            return html;
        }

        [[nodiscard]] std::string metrics_console_html_output_path()
        {
            #ifdef FLTX_METRICS_REPORT_SUFFIX
            constexpr std::string_view report_suffix = FLTX_METRICS_REPORT_SUFFIX;
            #else
            constexpr std::string_view report_suffix = "";
            #endif

            return metrics_output_path(
                "console",
                metrics_report_suffix_for_current_filter(report_suffix),
                "html");
        }

        void write_metrics_console_html_report(std::ostream& out, std::string_view console_text)
        {
            if (console_text.empty())
                return;

            const std::string output_path = metrics_console_html_output_path();
            write_text_file(output_path, ansi_console_text_to_html(console_text));

            if (metrics_verbose_enabled())
            {
                out << "\n[metrics report]\n"
                    << "console html = " << metrics_report_display_path(output_path) << '\n';
            }
        }

        void write_metrics_console_legend(std::ostream& out)
        {
            if (!metrics_verbose_enabled())
                return;

            out << "\n[metrics legend]\n\n"
                << "bits accurate       = estimated matching binary bits versus the operation oracle; value-returning f128/f256 rows use an MPFR-backed target reference.\n\n"

                << "sample score        = clamp(finite-domain bits / target bits, 0, 1); targets: f128=106 bits, f256=212 bits, reduced near expansion underflow.\n"
                << "domain score        = normal finite-domain magnitude quality: 50% mean sample score, 30% 1st-percentile sample score, 20% worst sample score.\n"

                << "Inf/NaN             = Inf/NaN probe support: Both, Inf, NaN, No, or unavailable (-).\n"
                << "bench ns            = median ns/iteration across timing trials.\n\n"

                << "preferred reference = preferred backend picked from faster comparable results.\n"
                << "cppdd               = boost::multiprecision::cpp_double_double\n"
                << "mpfr64              = boost::multiprecision::mpfr_float_backend<64>\n"
                << "ddreal              = qdpp double-double type\n"
                << "qdreal              = qdpp quad-double type\n\n";
        }
    }

    void start_metrics_console_html_capture(std::ostream& out)
    {
        start_metrics_console_capture(out);
    }

    void write_and_stop_metrics_console_html_capture(std::ostream& out)
    {
        write_metrics_console_html_report(out, stop_metrics_console_capture());
    }

    class metrics_case_report_listener final : public Catch::EventListenerBase
    {
    public:
        using Catch::EventListenerBase::EventListenerBase;

        static std::string getDescription()
        {
            return "prints consolidated metrics tables and writes complete metrics CSV reports";
        }

        void testRunStarting(const Catch::TestRunInfo&) override
        {
            clear_pending_metrics_case_reports();
            clear_realtime_metrics_console();
            clear_metrics_console_streamed_this_run();
            set_metrics_filter_arguments(m_config->getTestsOrTags());
            write_metrics_console_legend(Catch::cout());
        }

        void testRunEnded(const Catch::TestRunStats&) override
        {
            write_pending_metrics_case_reports(Catch::cout());
        }
    };
}

CATCH_REGISTER_LISTENER(bl::test::metrics::metrics_case_report_listener)
