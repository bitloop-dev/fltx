#pragma once

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

struct rgb_pixel
{
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
};

class bmp_writer
{
    int width{};
    int height{};
    int row_stride{};
    std::vector<std::uint8_t> pixels;

public:
    bmp_writer(int w, int h)
        : width(check_dimension(w)),
          height(check_dimension(h)),
          row_stride((width * 3 + 3) & ~3),
          pixels(static_cast<std::size_t>(row_stride) * static_cast<std::size_t>(height))
    {}

    [[nodiscard]] int getWidth() const noexcept
    {
        return width;
    }

    [[nodiscard]] int getHeight() const noexcept
    {
        return height;
    }

    void setPixel(int x, int y, rgb_pixel pixel) noexcept
    {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);

        const std::size_t bmp_y = static_cast<std::size_t>(height - 1 - y);
        const std::size_t i = bmp_y * static_cast<std::size_t>(row_stride) +
                                      static_cast<std::size_t>(x) * 3u;

        pixels[i + 0] = pixel.b;
        pixels[i + 1] = pixel.g;
        pixels[i + 2] = pixel.r;
    }

    void save(const std::filesystem::path& path) const
    {
        std::ofstream out(path, std::ios::binary);
        if (!out)
            throw std::runtime_error("failed to open BMP output file");

        const auto pixel_bytes = static_cast<std::uint32_t>(pixels.size());
        const auto file_size   = static_cast<std::uint32_t>(54u + pixel_bytes);

        out.put('B');
        out.put('M');
        write_u32(out, file_size);
        write_u16(out, 0);
        write_u16(out, 0);
        write_u32(out, 54);

        write_u32(out, 40);
        write_u32(out, static_cast<std::uint32_t>(width));
        write_u32(out, static_cast<std::uint32_t>(height));
        write_u16(out, 1);
        write_u16(out, 24);
        write_u32(out, 0);
        write_u32(out, pixel_bytes);
        write_u32(out, 2835);
        write_u32(out, 2835);
        write_u32(out, 0);
        write_u32(out, 0);

        out.write(reinterpret_cast<const char*>(pixels.data()),
                  static_cast<std::streamsize>(pixels.size()));

        if (!out)
            throw std::runtime_error("failed to write BMP output file");
    }

private:

    static int check_dimension(int value)
    {
        if (value <= 0)
            throw std::invalid_argument("bmp_writer dimensions must be positive");

        return value;
    }

    static void write_u16(std::ofstream& out, std::uint16_t value)
    {
        out.put(static_cast<char>(value & 0xffu));
        out.put(static_cast<char>((value >> 8) & 0xffu));
    }

    static void write_u32(std::ofstream& out, std::uint32_t value)
    {
        write_u16(out, static_cast<std::uint16_t>(value));
        write_u16(out, static_cast<std::uint16_t>(value >> 16));
    }
};
