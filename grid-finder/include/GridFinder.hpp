#pragma once

#include <Matrix.hpp>
#include <cstdint>

#include <limits>

#include <iostream>

#include <Bresenham.hpp>
#include <CenterPointOutLineIterator.hpp>

#include <algorithm>  // max_element

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
    GridMask() : mask{} {}

    Pixel getStartingPixel(size_t x) const {
        if (x >= W)
            throw std::out_of_range("x out of range");

        CenterPointOutLineIterator c = H;

        // If the center pixel is white, look for the first black pixels in both
        // directions
        if (get(x, c.getCenter())) {
            size_t first_white;
            size_t y = c.getCenter();
            // Find the lowest white pixel of the line through the center
            while (y < H) {
                if (get(x, y))
                    first_white = y;
                else
                    break;
                --y;
            }
            // Find the highest white pixel of the line through the center
            size_t last_white;
            y = c.getCenter();
            while (y < H) {
                if (get(x, y))
                    last_white = y;
                else
                    break;
                ++y;
            }
            y = (first_white + last_white) / 2;
            return Pixel(x, y);

            // If the center pixel is not white, look for the first white pixel in
            // both directions
        } else {
            size_t first_white = H;
            while (c.hasNext()) {
                size_t y = c.next();
                if (get(x, y)) {
                    first_white = y;
                    break;
                }
            }
            if (first_white >= H)
                return Pixel(-1, -1);

            size_t last_white =
                first_white;  // initialization keeps the compiler happy
            // If the first white pixel is below the center
            if (first_white < c.getCenter()) {
                // Look downwards for the last white pixel of that line
                size_t y = first_white;
                while (y < H) {
                    if (get(x, y))
                        last_white = y;
                    else
                        break;
                    --y;
                }
                // If the first white pixel is above the center
            } else {
                // Look upwards for the last white pixel of that line
                size_t y = first_white;
                while (y < H) {
                    if (get(x, y))
                        last_white = y;
                    else
                        break;
                    ++y;
                }
            }
            size_t y = (first_white + last_white) / 2;
            return Pixel(x, y);
        }
    }

    Pixel getStartingPixel() const {
        CenterPointOutLineIterator c = W;
        while (c.hasNext()) {
            if (Pixel pixel = getStartingPixel(c.next()); pixel.isValid())
                return pixel;
        }
        throw std::runtime_error("Error: no white pixels found");
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

    constexpr static size_t HOUGH_ANGLE_RESOLUTION = 360;

    double findLineAngle(Pixel px) {
        std::array<HoughResult, HOUGH_ANGLE_RESOLUTION> houghRes;
        double step = M_2_PI / HOUGH_ANGLE_RESOLUTION;
        for (size_t i = 0; i < HOUGH_ANGLE_RESOLUTION; ++i)
            houghRes[i] = hough(px, step * i);
        return std::max_element(houghRes.begin(), houghRes.end())->angle;
    }

    double findLineAngleAccurate(Pixel px) {
        std::array<HoughResult, HOUGH_ANGLE_RESOLUTION> houghRes;
        double step = 2 * M_PI / HOUGH_ANGLE_RESOLUTION;
        for (size_t i = 0; i < HOUGH_ANGLE_RESOLUTION; ++i) {
            houghRes[i] = hough(px, step * i);
            // cout << houghRes[i].angle << ", " << houghRes[i].count << endl;
        }
        auto max       = std::max_element(houghRes.begin(), houghRes.end());
        auto first_max = max;
        auto last_max  = max;
        while (last_max->count == max->count) {
            ++last_max;
            if (last_max == houghRes.end())
                last_max = houghRes.begin();
            else if (last_max == max)
                break;
        }
        while (first_max->count == max->count) {
            if (first_max == houghRes.begin())
                first_max = houghRes.end();
            --first_max;
            if (first_max == max)
                break;
        }
        double first_angle = first_max->angle;
        double last_angle  = last_max->angle;
        if (first_angle - last_angle > M_PI)
            first_angle -= 2 * M_PI;
        if (last_angle - first_angle > M_PI)
            last_angle -= 2 * M_PI;
        double angle = (first_angle + last_angle) / 2;
        return angle;
    }

    inline constexpr uint8_t get(size_t x, size_t y) const {
        return mask[y][x];
    }
    inline constexpr uint8_t get(Pixel px) const { return mask[px.y][px.x]; }
    inline constexpr void set(Pixel px) { mask[px.y][px.x] = 0xFF; }

  private:
    Img_t mask;
};