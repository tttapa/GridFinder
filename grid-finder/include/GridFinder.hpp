#pragma once

#include <Matrix.hpp>
#include <cstdint>

#include <limits>

#include <iostream>

#include <Bresenham.hpp>

using std::cout;
using std::endl;

struct HoughResult {
    double angle;
    size_t count;
    bool operator<(HoughResult other) const {
        return this->count < other.count;
    }
};

inline std::ostream &operator<<(std::ostream &os, HoughResult h) {
    return os << h.angle << " rad: " << h.count;
}

template <size_t W, size_t H>
class GridMask {
  public:
    using Img_t = TMatrix<uint8_t, H, W>;
    GridMask(const Img_t &mask) : mask(mask) {}

    Pixel getStartingPixel(size_t x) const {
        if (x >= W)
            throw std::out_of_range("x out of range");
        int firstWhite = -1;
        for (size_t y = 0; y < H; ++y) {
            if (firstWhite < 0 && get(x, y) != 0) {
                firstWhite = y;
            } else if (firstWhite >= 0 && get(x, y) == 0) {
                unsigned int lastWhite = y - 1;
                int y_middle = firstWhite + (lastWhite - firstWhite) / 2;
                return Pixel(x, y_middle);
            }
        }
        if (firstWhite < 0) {
            return Pixel(-1, -1);
        } else {
            int y_middle = firstWhite + (H - firstWhite) / 2;
            return Pixel(x, y_middle);
        }
    }

    Pixel getStartingPixel() const {
        int x0      = (W - 1) / 2;
        int x       = 0;
        Pixel pixel = getStartingPixel(x + x0);
        while (!pixel.isValid() && x + x0 < (int) W && x + x0 >= 0) {
            cout << "x = " << x << endl;
            pixel = getStartingPixel(x0 + x);
            x     = x > 0 ? -x : -x + 1;
        }
        if (!pixel.isValid())
            throw std::runtime_error("Error: no white pixels found");
        return pixel;
    }

    HoughResult hough(Pixel px, double angle) {
        BresenhamLine line   = {px, angle, W, H};
        size_t count         = 0;
        uint_fast16_t weight = 0;
        while (line.hasNext()) {
            Pixel point = line.next();
            // count += get(point) >> 7;  // convert from 255/0 to 1/0
            count += weight++ & (get(point) << 8 | get(point));
        }
        return {angle, count};
    }

    inline constexpr uint8_t get(size_t x, size_t y) const {
        return mask[y][x];
    }
    inline constexpr uint8_t get(Pixel px) const { return mask[px.y][px.x]; }
    inline constexpr void set(Pixel px) { mask[px.y][px.x] = 0xFF; }

  private:
    Img_t mask;
};