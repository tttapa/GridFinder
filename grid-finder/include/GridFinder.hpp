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

    // TODO: StartingPoint --> middle of line to search next + width
    struct StartingPoint {
        StartingPoint(bool found) : found(found) {}
        StartingPoint(Pixel pixel, size_t width, bool above)
            : found(true), pixel(pixel), width(width), above(above) {}
        bool found;
        Pixel pixel;
        size_t width;
        bool above;

        explicit operator bool() const { return found; }
    };

    StartingPoint getStartingPoint(size_t x) const {
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
            y            = (first_white + last_white) / 2;
            size_t width = last_white - first_white + 1;
            bool above =
                (last_white - c.getCenter()) > (c.getCenter() - first_white);
            return {
                Pixel(x, y),
                width,
                above,
            };

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
                return false;

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
                y            = (first_white + last_white) / 2;
                size_t width = first_white - last_white + 1;
                bool above   = false;
                return {
                    Pixel(x, y),
                    width,
                    above,
                };
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
                y            = (first_white + last_white) / 2;
                size_t width = last_white - first_white + 1;
                bool above   = true;
                return {
                    Pixel(x, y),
                    width,
                    above,
                };
            }
        }
    }

    // ONLY USED IN FIRST PART OF ALGORITHM TO FIND FIRST LINE
    StartingPoint getStartingPoint() const {
        CenterPointOutLineIterator c = W;
        while (c.hasNext()) {
            if (StartingPoint sp = getStartingPoint(c.next()))
                return sp;
        }
        throw std::runtime_error("Error: no white pixels found");
    }

    // TODO: HoughResult --> ? (better name - line with vote count)
    // TODO: hough() --> countVotes()
    HoughResult hough(Pixel px, double angle) {
        BresenhamLine line   = {px, angle, W, H};
        size_t count         = 0;
        uint_fast16_t weight = 0;
        while (line.hasNext()) {
            Pixel point = line.next();
            // If pixel is white, then add weight to count
            // Mask == 0xFFFF if pixel is white, 0x0000 if it's black
            uint_fast16_t mask = get(point) << 8 | get(point);
            count += weight & mask;
            ++weight;
        }
        return {angle, count};
    }

    constexpr static size_t HOUGH_ANGLE_RESOLUTION = 360;

    double findLineAngle(Pixel px) {
        std::array<HoughResult, HOUGH_ANGLE_RESOLUTION> houghRes;
        double step = 2 * M_PI / HOUGH_ANGLE_RESOLUTION;
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

        // Find last angle with the same count as max_count (+ 1 step)
        while (last_max->count == max->count) {
            ++last_max;
            if (last_max ==
                houghRes.end())  // .end() gives index of last element + 1
                last_max = houghRes.begin();
            else if (last_max == max)
                break;
        }

        // Find first angle with the same count as max_count (- 1 step)
        while (first_max->count == max->count) {
            if (first_max == houghRes.begin())
                first_max = houghRes.end();
            --first_max;
            if (first_max == max)
                break;
        }

        return averageAngle(first_max->angle, last_max->angle);
    }

    /**
     * @brief   The maximum number of pixels that can be black within a line,
     *          while still being detected correctly.
     */
    // static const size_t MAX_GAP = (W + H) / 2 / 10;
    static const size_t MAX_GAP = 10;

    size_t getWidthAtPointOnLine(Pixel pixel, int cos, int sin) {
        BresenhamLine alongLine = {pixel, cos, sin, W, H};
        auto [cosPerp, sinPerp] = getPerpendicularCosSin(cos, sin);
        size_t maxWidth         = 0;
        // Follow a path along the given line for MAX_GAP pixels
        for (size_t i = 0; i <= MAX_GAP && alongLine.hasNext(); ++i) {
            // For each pixel along this path, move away from the line,
            // perpendicular to it, untill you find a black pixel, or until you
            // fall off the canvas.
            BresenhamLine perpendicular = {alongLine.next(), cosPerp, sinPerp,
                                           W, H};
            bool success                = false;
            // success == true if black pixel found, false if off canvas
            while (perpendicular.hasNext()) {
                Pixel pixel = perpendicular.next();
                cout << pixel << " - ";
                if (get(pixel) == 0x00) {
                    success = true;
                    break;
                }
            }
            cout << (perpendicular.getCurrentLength() - 1) << endl;

            // If we found a black pixel before going off the canvas
            if (success && perpendicular.getCurrentLength() > maxWidth)
                // getCurrentLength() gives the length to the first black pixel,
                // so that's one pixel too much. Fix at the end of the function.
                maxWidth = perpendicular.getCurrentLength();
        }
        return maxWidth - 1;
    }

    template <class T>
    std::tuple<T, T> getPerpendicularCosSin(T cos, T sin) {
        return {-sin, cos};
    }

    double static getPerpendicularAngle(double angle, bool above) {
        return normalizeAngle(angle + M_PI_2);
    }

    double static normalizeAngle(double angle) {
        if (angle >= 2 * M_PI)
            angle -= 2 * M_PI;
        return angle;
    }

    static double averageAngle(double first_angle, double last_angle) {
        if (first_angle > last_angle)
            last_angle += 2 * M_PI;
        double angle = (first_angle + last_angle) / 2;
        return normalizeAngle(angle);
    }

    // TODO: getValue() (0 or 255)
    inline constexpr uint8_t get(size_t x, size_t y) const {
        return mask[y][x];
    }
    inline constexpr uint8_t get(Pixel px) const { return mask[px.y][px.x]; }
    inline constexpr void set(Pixel px) { mask[px.y][px.x] = 0xFF; }

    std::ostream &print(std::ostream &os) {
        for (size_t y = 0; y < H; ++y) {
            for (size_t x = 0; x < W; ++x)
                os << (get(x, y) ? "\xe2\xac\xa4 " : "\xe2\x97\xaf ");
            os << "\r\n";
        }
        return os;
    }

  private:
    Img_t mask;
};