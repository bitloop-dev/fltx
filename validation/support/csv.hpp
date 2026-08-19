#ifndef FLTX_TESTS_SUPPORT_CSV_INCLUDED
#define FLTX_TESTS_SUPPORT_CSV_INCLUDED

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#ifdef small
#undef small
#endif
#endif

namespace fltx::tests
{
    [[nodiscard]] inline std::string csv_escape(std::string_view value)
    {
        if (value.find_first_of(",\"\r\n") == std::string_view::npos)
            return std::string(value);

        std::string out{ "\"" };
        for (char c : value)
        {
            out += c;
            if (c == '"')
                out += '"';
        }
        out += '"';
        return out;
    }

    class csv_writer
    {
    public:
        explicit csv_writer(std::filesystem::path final_path)
            : final_path_(std::move(final_path)),
              partial_path_(
                  final_path_.parent_path() /
                  (final_path_.stem().string() + ".partial" + final_path_.extension().string()))
        {
            if (!final_path_.parent_path().empty())
                std::filesystem::create_directories(final_path_.parent_path());
            stream_.open(partial_path_, std::ios::binary | std::ios::trunc);
            if (!stream_)
                throw std::runtime_error("cannot open metrics output: " + partial_path_.string());
        }

        void row(std::initializer_list<std::string> fields)
        {
            bool first = true;
            for (const std::string& field : fields)
            {
                if (!first)
                    stream_ << ',';
                stream_ << csv_escape(field);
                first = false;
            }
            stream_ << '\n';
            stream_.flush();
            if (!stream_)
                throw std::runtime_error("failed to write metrics output: " + partial_path_.string());
            ++rows_written_;
        }

        [[nodiscard]] std::size_t rows_written() const noexcept
        {
            return rows_written_;
        }

        void finish()
        {
            stream_.flush();
            stream_.close();
#if defined(_WIN32)
            if (!MoveFileExW(
                    partial_path_.c_str(),
                    final_path_.c_str(),
                    MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            {
                throw std::runtime_error(
                    "cannot publish metrics output (Windows error " +
                    std::to_string(GetLastError()) + ")");
            }
#else
            std::error_code error;
            std::filesystem::rename(partial_path_, final_path_, error);
            if (error)
                throw std::runtime_error("cannot publish metrics output: " + error.message());
#endif
        }

        ~csv_writer()
        {
            if (stream_.is_open())
                stream_.close();
        }

        csv_writer(const csv_writer&) = delete;
        csv_writer& operator=(const csv_writer&) = delete;

    private:
        std::filesystem::path final_path_;
        std::filesystem::path partial_path_;
        std::ofstream stream_;
        std::size_t rows_written_ = 0;
    };
}

#endif
