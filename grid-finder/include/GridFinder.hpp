#pragma once

#include <ANSIColors.hpp>
#include <Bresenham.hpp>
#include <CenterPointOutLineIterator.hpp>
#include <Matrix.hpp>
#include <algorithm>  // max_element
#include <cstdint>
#include <iostream>
#include <limits>

using std::cerr;
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
            houghRes.at(i) = hough(px, step * i);
        return std::max_element(houghRes.begin(), houghRes.end())->angle;
    }

    double findLineAngleAccurate(Pixel px) {
        std::array<HoughResult, HOUGH_ANGLE_RESOLUTION> houghRes;
        double step = 2 * M_PI / HOUGH_ANGLE_RESOLUTION;
        for (size_t i = 0; i < HOUGH_ANGLE_RESOLUTION; ++i)
            houghRes.at(i) = hough(px, step * i);
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

    template <size_t N>
    double findLineAngleAccurateRange(Pixel px, double centerAngle) {
        static_assert(2 * N < HOUGH_ANGLE_RESOLUTION);
        std::array<HoughResult, 2 * N + 1> houghRes;
        double step             = 2 * M_PI / HOUGH_ANGLE_RESOLUTION;
        size_t centerAngleIndex = round(centerAngle / step);

        // If the range contains no discontinuities
        // ├───┬───┬───╤═══╤═══╤═══╪═══╤═══╤═══╤───┬───┬───┤
        // 0          C-N          C          C+N         R-1
        // C = center
        // N = number of steps around the center
        // R = resolution (R steps = 360°)
        if (centerAngleIndex - N < HOUGH_ANGLE_RESOLUTION &&
            centerAngleIndex + N < HOUGH_ANGLE_RESOLUTION) {
            size_t initialAngleIndex = centerAngleIndex - N;
            for (size_t i = 0; i <= 2 * N; ++i)
                houghRes.at(i) = hough(px, step * (initialAngleIndex + i));
        }
        // If the 0-2PI discontinuity is in the first part of the range
        // (< centerAngle)
        // ╞═══╪═══╤═══╤═══╤───┬───┬───┬───┬───┬───┬───╤═══╡
        // 0   C          C+N                         C-N R-1
        else if (centerAngleIndex + N < HOUGH_ANGLE_RESOLUTION) {
            size_t initialAngleIndex =
                HOUGH_ANGLE_RESOLUTION + centerAngleIndex - N;
            size_t i = 0;
            for (size_t j = initialAngleIndex; j < HOUGH_ANGLE_RESOLUTION;
                 ++i, ++j)
                houghRes.at(i) = hough(px, step * j);
            for (size_t j = 0; j <= centerAngleIndex + N; ++i, ++j)
                houghRes.at(i) = hough(px, step * j);
        }
        // If the 0-2PI discontinuity is in the second part of the range
        // (< centerAngle)
        // ╞═══╤───┬───┬───┬───┬───┬───┬───╤═══╤═══╤═══╪═══╡
        // 0  C+N                         C-N          C  R-1
        else if (centerAngleIndex - N < HOUGH_ANGLE_RESOLUTION) {
            size_t initialAngleIndex = centerAngleIndex - N;
            size_t i                 = 0;
            for (size_t j = initialAngleIndex; j < HOUGH_ANGLE_RESOLUTION;
                 ++i, ++j)
                houghRes.at(i) = hough(px, step * j);
            for (size_t j = 0;
                 j <= N + centerAngleIndex - HOUGH_ANGLE_RESOLUTION; ++i, ++j)
                houghRes.at(i) = hough(px, step * j);
        } else {
            assert(false);
        }
        // If there are multiple elements with the same maximum count,
        // max_element will return an iterator to the first element with this
        // count
        auto first_max = std::max_element(houghRes.begin(), houghRes.end());
        // Find the first element after the first maximum that has a lower count
        // and then subtract one to get the last element that has the maximum
        // count
        // If no element with a lower count is found, find_if returns
        // houghRes.end(), which means that last_max will be an iterator to
        // the last element of the array.
        auto last_max = std::find_if(first_max, houghRes.end(), [&](auto res) {
            return res.count < first_max->count;
        });
        if (first_max == houghRes.begin() || last_max == houghRes.end())
            cerr << ANSIColors::red
                 << "Warning: angle with maximum count could lie outside of "
                    "the specified range\r\n"
                 << ANSIColors::reset;
        --last_max;

        return averageAngle(first_max->angle, last_max->angle);
    }

    /**
     * @brief   The maximum number of pixels that can be black within a line,
     *          while still being detected correctly.
     */
    // static const size_t MAX_GAP = (W + H) / 2 / 10;
    static const size_t MAX_GAP = 10;

    size_t getWidthAtPointOnLine(Pixel pixel, int cos, int sin,
                                 size_t max_gap = MAX_GAP,
                                 bool plus90deg = true) {
        BresenhamLine alongLine = {pixel, cos, sin, W, H};
        auto [cosPerp, sinPerp] = getPerpendicularCosSin(cos, sin, plus90deg);
        size_t maxWidth         = 0;
        // Follow a path along the given line for max_gap pixels
        for (size_t i = 0; i <= max_gap && alongLine.hasNext(); ++i) {
            // For each pixel along this path, move away from the line,
            // perpendicular to it, untill you find a black pixel, or until you
            // fall off the canvas.
            BresenhamLine perpendicular = {alongLine.next(), cosPerp, sinPerp,
                                           W, H};
            size_t ctr                  = 0;
            while (perpendicular.hasNext() && ++ctr < 64) {
                Pixel pixel = perpendicular.next();
                cout << pixel << " - ";
                if (get(pixel) == 0x00)
                    break;
            }
            cout << (perpendicular.getCurrentLength() - 1) << endl;
            if (ctr >= 64)
                cerr << ANSIColors::redb << "endless loop" << endl
                     << ANSIColors::reset;

            // If we found a black pixel before going off the canvas
            if (perpendicular.getCurrentLength() > maxWidth)
                // getCurrentLength() gives the length to the first black pixel,
                // so that's one pixel too much. Fix at the end of the function.
                maxWidth = perpendicular.getCurrentLength();
        }
        return maxWidth - 1;
    }

    Pixel getMiddle(Pixel pointOnLine, int cos, int sin,
                    size_t max_gap = MAX_GAP) {
        auto [ocos, osin] = getOppositeCosSin(cos, sin);
        cout << " cos = " << cos << endl;
        cout << " sin = " << sin << endl;
        cout << "ocos = " << ocos << endl;
        cout << "osin = " << osin << endl;
        cout << "widthUpper1" << endl;
        size_t widthUpper1 =
            getWidthAtPointOnLine(pointOnLine, cos, sin, max_gap / 2, true);
        cout << ANSIColors::blackb << "widthUpper1 = " << widthUpper1
             << ANSIColors::reset << endl;
        cout << "widthLower1" << endl;
        size_t widthLower1 =
            getWidthAtPointOnLine(pointOnLine, cos, sin, max_gap / 2, false);
        cout << ANSIColors::blackb << "widthLower1 = " << widthLower1
             << ANSIColors::reset << endl;
        cout << "widthUpper2" << endl;
        size_t widthUpper2 =
            getWidthAtPointOnLine(pointOnLine, ocos, osin, max_gap / 2, false);
        cout << ANSIColors::blackb << "widthUpper2 = " << widthUpper2
             << ANSIColors::reset << endl;
        cout << "widthLower2" << endl;
        size_t widthLower2 =
            getWidthAtPointOnLine(pointOnLine, ocos, osin, max_gap / 2, true);
        cout << ANSIColors::blackb << "widthLower2 = " << widthLower2
             << ANSIColors::reset << endl;

        int middlePointCorrection = std::max(widthUpper1, widthUpper2) -
                                    std::max(widthLower1, widthLower2);
        cout << "middlePointCorrection = " << middlePointCorrection << endl;

        auto [corrCos, corrSin] =
            getPerpendicularCosSin(cos, sin, middlePointCorrection > 0);
        BresenhamLine corr = {pointOnLine, corrCos, corrSin, W, H};
        Pixel middle       = pointOnLine;
        size_t middlePointCorrectionDistance =
            std::abs(middlePointCorrection)  / 2;
        while (corr.hasNext() &&
               corr.getCurrentLength() <= middlePointCorrectionDistance)
            middle = corr.next();
        return middle;
    }

    template <class T>
    std::tuple<T, T> getPerpendicularCosSin(T cos, T sin,
                                            bool plus90deg = true) {
        return plus90deg ? std::tuple<T, T>{-sin, cos}
                         : std::tuple<T, T>{sin, -cos};
    }

    template <class T>
    std::tuple<T, T> getOppositeCosSin(T cos, T sin) {
        return {-cos, -sin};
    }

    double static getPerpendicularAngle(double angle) {
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
                os << (get(x, y) ? "⬤ " : "◯ ");
            os << "\r\n";
        }
        return os;
    }

  private:
    Img_t mask;
};