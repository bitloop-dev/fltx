#include <atomic>
#include <cmath>
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <filesystem>

#include <fltx.h>
#include "helpers/bmp_writer.h"

using namespace bl;
using namespace bl::literals;

static unsigned char to_byte(double x)
{
    if (x < 0.0) x = 0.0;
    if (x > 255.0) x = 255.0;
    return static_cast<unsigned char>(x);
}

int main()
{
    // switch which underlying type is used
    using flt = f128;

    // mandelbrot iteration limit / coordinate / zoom
    constexpr int max_iter = 20000;
    constexpr flt center_x = bl::parse<flt>("-1.73200006480238126967529761198455");
    constexpr flt center_y = bl::parse<flt>("0.00000019235376499049335337716270");
    constexpr flt zoom     = bl::parse<flt>("2.0e+28");

    // target bitmap width / height
    constexpr int width  = 1024;
    constexpr int height = 1024;

    bmp_writer image(width, height);
    std::atomic<int> rows_done = 0;

    // determine Mandelbrot-plane distance per pixel and image-centre offset
    const flt scale_x = 4.0 / (zoom * width);
    const flt scale_y = 4.0 / (zoom * height);
    const flt half_w  = width * 0.5;
    const flt half_h  = height * 0.5;

    // determine ideal thread count to split up work
    std::size_t thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) thread_count = 1;
    if (thread_count > (std::size_t)height) thread_count = height;

    // calculate how many rows to assign to each thread
    const int rows_per_thread = (height + (int)thread_count - 1) / (int)thread_count;

    // mandelbrot kernel to process rows [row_begin, row_end)
    auto render_rows = [&](int row_begin, int row_end)
    {
        for (int row = row_begin; row < row_end; ++row)
        {
            for (int px = 0; px < width; ++px)
            {
                flt cx = center_x + (flt(px) - half_w) * scale_x;
                flt cy = center_y + (flt(row) - half_h) * scale_y;
                flt x = 0, y = 0;

                int iter = 0;
                while (iter < max_iter)
                {
                    flt xx = x * x - y * y + cx;
                    y = 2.0 * x * y + cy;
                    x = xx;
                    ++iter;

                    // cheap low-precision escape check
                    if (bl::sqr((f64)x) + bl::sqr((f64)y) > 4.0)
                        break;
                }

                // choose a nice colour
                unsigned char r, g, b;
                if (iter == max_iter)
                {
                    r = g = b = 0;
                }
                else
                {
                    constexpr f64 tau = 6.28318530717958647692;
                    f64 t = iter * 0.005;

                    r = to_byte(127.5 + 127.5 * std::cos(tau * (t + 0.00)));
                    g = to_byte(127.5 + 127.5 * std::cos(tau * (t + 0.15)));
                    b = to_byte(127.5 + 127.5 * std::cos(tau * (t + 0.32)));
                }

                image.setPixel(px, row, { r, g, b });
            }

            // update progress
            rows_done.fetch_add(1, std::memory_order_relaxed);
        }
    };

    // create threads, assign rows
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    const auto render_start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < thread_count; ++i)
    {
        int row_begin = (int)i * rows_per_thread;
        int row_end   = row_begin + rows_per_thread;

        if (row_end > height)
            row_end = height;

        if (row_begin < row_end)
            threads.emplace_back(render_rows, row_begin, row_end);
    }

    // track percentage complete
    int last_percent = -1;
    while (true)
    {
        int done = rows_done.load(std::memory_order_relaxed);
        int percent = (done * 100) / height;

        if (percent != last_percent)
        {
            std::cout << "\r" << percent << "% complete" << std::flush;
            last_percent = percent;
        }

        if (done >= height)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // wait for all threads to complete
    for (std::thread& thread : threads)
        thread.join();

    const auto render_end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> render_time = render_end - render_start;

    std::cout << "\r100% complete\n";
    std::cout << "generation took " << render_time.count() << " seconds\n";

    // save image (and print path)
    const std::filesystem::path output_path = std::filesystem::absolute("mandelbrot.bmp");
    image.save(output_path);
    std::cout << "wrote: " << output_path.string() << "\n";
}
