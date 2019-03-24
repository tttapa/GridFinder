#pragma once

#include <ANSIColors.hpp>
#include <Angle.hpp>
#include <Bresenham.hpp>
#include <CenterPointOutLineIterator.hpp>
#include <Line.hpp>
#include <Matrix.hpp>
#include <algorithm>  // max_element
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>

using std::cerr;
using std::cout;
using std::endl;

struct HoughResult {
    angle_t angle;
    size_t count;
    bool operator<(HoughResult other) const {
        return this->count < other.count;
    }
};

inline std::ostream &operator<<(std::ostream &os, HoughResult h) {
    return os << h.angle << " rad: " << h.count;
}

struct LineResult {
    constexpr LineResult() : LineResult(false, {}, 0, {}) {}
    constexpr LineResult(Pixel lineCenter, size_t width, angle_t angle)
        : valid(true), lineCenter(lineCenter), width(width), angle(angle) {}
    constexpr LineResult(bool valid, Pixel lineCenter, size_t width,
                         angle_t angle)
        : valid(valid), lineCenter(lineCenter), width(width), angle(angle) {}
    constexpr static LineResult invalid() { return {}; }
    bool valid;
    Pixel lineCenter;
    size_t width;
    angle_t angle;
};
inline std::ostream &operator<<(std::ostream &os, LineResult l) {
    if (!l.valid)
        return os << "LineResult::invalid()";
    else
        return os << "LineResult(" << l.lineCenter << ", " << l.width << ", "
                  << l.angle << ")";
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
    HoughResult hough(Pixel px, angle_t angle) {
        BresenhamLine line   = {px, angle, W, H};
        size_t count         = 0;
        uint_fast16_t weight = 0;
        while (line.hasNext()) {
            Pixel point = line.next();
            // If pixel is white, then add weight to count
            // Mask == 0xFFFF if pixel is white, 0x0000 if it's black
            if (get(point))
                count += weight++;
            //    count += 1;
        }
        return {angle, count};
    }

    HoughResult findLineAngle(Pixel px) {
        std::array<HoughResult, angle_t::resolution()> houghRes;
        for (size_t i = 0; i < angle_t::resolution(); ++i)
            houghRes.at(i) = hough(px, angle_t(i));
        return *std::max_element(houghRes.begin(), houghRes.end());
    }

    HoughResult findLineAngleAccurate(Pixel px) {
#ifdef DEBUG
        cout << "findLineAngleAccurate(" << px << ")" << endl;
#endif
        std::array<HoughResult, angle_t::resolution()> houghRes;
        for (size_t i = 0; i < angle_t::resolution(); ++i)
            houghRes.at(i) = hough(px, angle_t(i));
        auto max       = std::max_element(houghRes.begin(), houghRes.end());
        auto first_max = max;
        auto last_max  = max;

        // Find last angle with the same count as max_count (+ 1 step)
        while (last_max->count == max->count) {
            ++last_max;
            // .end() gives index of last element + 1
            if (last_max == houghRes.end())
                last_max = houghRes.begin();
            if (last_max == max)
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
#ifdef DEBUG
        cout << "first_max = " << first_max->angle << endl;
        cout << "last_max = " << last_max->angle << endl;
        cout << "count = " << max->count << endl;
#endif

        return {
            angle_t::average(first_max->angle, last_max->angle),
            max->count,
        };
    }

    template <size_t N>
    HoughResult findLineAngleAccurateRange(Pixel px, angle_t centerAngle) {
        static_assert(2 * N < angle_t::resolution());
        std::array<HoughResult, 2 * N + 1> houghRes;
        size_t centerAngleIndex = centerAngle.getIndex();

        // If the range contains no discontinuities
        // ├───┬───┬───╤═══╤═══╤═══╪═══╤═══╤═══╤───┬───┬───┤
        // 0          C-N          C          C+N         R-1
        // C = center
        // N = number of steps around the center
        // R = resolution (R steps = 360°)
        if (centerAngleIndex - N < angle_t::resolution() &&
            centerAngleIndex + N < angle_t::resolution()) {
            size_t initialAngleIndex = centerAngleIndex - N;
            for (size_t i = 0; i <= 2 * N; ++i)
                houghRes.at(i) = hough(px, angle_t(initialAngleIndex + i));
        }
        // If the 0-2PI discontinuity is in the first part of the range
        // (< centerAngle)
        // ╞═══╪═══╤═══╤═══╤───┬───┬───┬───┬───┬───┬───╤═══╡
        // 0   C          C+N                         C-N R-1
        else if (centerAngleIndex + N < angle_t::resolution()) {
            size_t initialAngleIndex =
                angle_t::resolution() + centerAngleIndex - N;
            size_t i = 0;
            for (size_t j = initialAngleIndex; j < angle_t::resolution();
                 ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
            for (size_t j = 0; j <= centerAngleIndex + N; ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
        }
        // If the 0-2PI discontinuity is in the second part of the range
        // (< centerAngle)
        // ╞═══╤───┬───┬───┬───┬───┬───┬───╤═══╤═══╤═══╪═══╡
        // 0  C+N                         C-N          C  R-1
        else if (angle_t::resolution() - N < angle_t::resolution()) {
            size_t initialAngleIndex = centerAngleIndex - N;
            size_t i                 = 0;
            for (size_t j = initialAngleIndex; j < angle_t::resolution();
                 ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
            for (size_t j = 0;
                 j <= N + centerAngleIndex - angle_t::resolution(); ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
        } else {
            assert(false);
        }
        // If there are multiple elements with the same maximum count,
        // max_element will return an iterator to the first element with this
        // count
        auto max = std::max_element(houghRes.begin(), houghRes.end());
        // Find the first element after the first maximum that has a lower count
        // and then subtract one to get the last element that has the maximum
        // count
        // If no element with a lower count is found, find_if returns
        // houghRes.end(), which means that last_max will be an iterator to
        // the last element of the array.
        auto first_max = std::find_if(
            std::make_reverse_iterator(max), houghRes.rend(),
            [&](auto res) { return res.count < max->count - max->count / 8; });
        auto last_max = std::find_if(max, houghRes.end(), [&](auto res) {
            return res.count < max->count - max->count / 8;
        });
        if (first_max == houghRes.rend() || last_max == houghRes.end())
            cerr << ANSIColors::red
                 << "Warning: angle with maximum count could lie outside of "
                    "the specified range\r\n"
                 << ANSIColors::reset;

        if (first_max != houghRes.rbegin())
            --first_max;
        if (last_max != houghRes.begin())
            --last_max;

        return {
            angle_t::average(first_max->angle, last_max->angle),
            first_max->count,
        };
    }

    /**
     * @brief   The maximum number of pixels that can be black within a line,
     *          while still being detected correctly.
     */
    // static const size_t MAX_GAP = (W + H) / 2 / 10;
    static const size_t MAX_GAP = 10;

    static const size_t maxLineWidth = 32;

    size_t getWidthAtPointOnLine(Pixel pixel, CosSin angle,
                                 size_t max_gap = MAX_GAP,
                                 bool plus90deg = true) {
        BresenhamLine alongLine   = {pixel, angle, W, H};
        CosSin perpendicularAngle = angle.perpendicular(plus90deg);
        size_t maxWidth           = 0;
        // Follow a path along the given line for max_gap pixels
        for (size_t i = 0; i <= max_gap && alongLine.hasNext(); ++i) {
            Pixel pixelAlongLine = alongLine.next();
            // For each pixel along this path, move away from the line,
            // perpendicular to it, untill you find a black pixel, or until you
            // fall off the canvas.
            BresenhamLine perpendicular = {pixelAlongLine, perpendicularAngle,
                                           W, H};
            while (perpendicular.hasNext() &&
                   perpendicular.getCurrentLength() < maxLineWidth) {
                Pixel pixel = perpendicular.next();
                if (get(pixel) == 0x00)
                    break;
            }
            if (perpendicular.getCurrentLength() > maxLineWidth)
                // throw std::runtime_error("Error: Endless loop detected");
                return maxLineWidth;

            // If we found a black pixel before going off the canvas
            if (perpendicular.getCurrentLength() > maxWidth)
                // getCurrentLength() gives the length to the first black pixel,
                // so that's one pixel too much. Fix at the end of the function.
                maxWidth = perpendicular.getCurrentLength();
        }
#ifdef DEBUG
        cout << "getWidthAtPointOnLine(" << pixel << ") → " << (maxWidth - 1)
             << endl;
#endif
        return maxWidth - 1;
    }

    struct GetMiddleResult {
        Pixel pixel;
        size_t width;
        bool valid;
    };

    GetMiddleResult getMiddle(Pixel pointOnLine, angle_t angle,
                              size_t max_gap = MAX_GAP) {
#ifdef DEBUG
        cout << "getMiddle(" << pointOnLine << ", " << angle << ")" << endl;
#endif
        if (get(pointOnLine) == 0x00)
            return {Pixel(), 0, false};  // return invalid pixel

        CosSin oppositeAngle = angle.opposite();
        size_t widthUpper1 =
            getWidthAtPointOnLine(pointOnLine, angle, max_gap / 2, true);
        size_t widthLower1 =
            getWidthAtPointOnLine(pointOnLine, angle, max_gap / 2, false);
        size_t widthUpper2 = getWidthAtPointOnLine(pointOnLine, oppositeAngle,
                                                   max_gap / 2, false);
        size_t widthLower2 = getWidthAtPointOnLine(pointOnLine, oppositeAngle,
                                                   max_gap / 2, true);

        size_t widthUpper = std::max(widthUpper1, widthUpper2);
        size_t widthLower = std::max(widthLower1, widthLower2);

        if (widthUpper >= maxLineWidth || widthLower >= maxLineWidth)
            return {Pixel(), 0, false};  // return invalid pixel

        int middlePointCorrection = widthUpper - widthLower;

        bool corrDirection = middlePointCorrection > 0;
        CosSin corrAngle   = angle.perpendicular(corrDirection);
        size_t middlePointCorrectionDistance =
            std::abs(middlePointCorrection) / 2;

        BresenhamLine corr = {pointOnLine, corrAngle, W, H};
        Pixel middle       = pointOnLine;
        while (corr.hasNext() &&
               corr.getCurrentLength() <= middlePointCorrectionDistance)
            middle = corr.next();
        return {middle, widthUpper + widthLower - 1, true};
    }

    // When we're at an intersection and no width can be determined, how
    // far should we jump along the line before trying again.
    constexpr static size_t RETRY_JUMP_DISTANCE = (W + H) / 20;  // TODO

    GetMiddleResult getMiddleWithRetries(Pixel start, angle_t angle) {
#ifdef DEBUG
        cout << "getMiddleWithRetries(" << start << ", " << angle << ")"
             << endl;
#endif
        GetMiddleResult middle;
        Pixel previousPixel;
        do {
            previousPixel = start;
            middle        = getMiddle(start, angle);
            start         = move(start, angle, RETRY_JUMP_DISTANCE);
        } while (!middle.valid && previousPixel != start);
        return middle;
    }

    constexpr static size_t MINIMIM_LINE_WEIGHTED_VOTE_COUNT =
        (W + H) / 2;  // TODO

    std::array<LineResult, 2> getFirstLines() {
        auto start = getStartingPoint();
        // Doesn't have to be accurate, just to find the width
        HoughResult firstResult = findLineAngle(start.pixel);
        angle_t firstAngle      = firstResult.angle;
        // There could be noise, so the first "line" could be just a white
        // bit of noise
        // TODO
        if (firstResult.count < MINIMIM_LINE_WEIGHTED_VOTE_COUNT)
            throw std::runtime_error("TODO: find a new starting position");

        GetMiddleResult middle = getMiddleWithRetries(start.pixel, firstAngle);
        if (!middle.valid)
            throw std::runtime_error("TODO: No middle point found");

        constexpr size_t range = angle_t::resolution() / 24;  // 2 * 15°
        HoughResult result1 =
            findLineAngleAccurateRange<range>(middle.pixel, firstAngle);
        HoughResult result2 = findLineAngleAccurateRange<range>(
            middle.pixel, firstAngle.opposite());

        return {{{middle.pixel, middle.width, result1.angle},
                 {middle.pixel, middle.width, result2.angle}}};
    }

    Pixel move(Pixel start, CosSin angle, size_t distance) {
        BresenhamLine path = {start, angle, W, H};
        Pixel end;
        while (path.hasNext() &&
               path.getCurrentLength() < distance)  // TODO: OBOE?
            end = path.next();
        return end;
    }

    constexpr static Pixel center() { return {(W - 1) / 2, (H - 1) / 2}; }

    LineResult findNextLine(LineResult line, size_t minDistance = 0,
                            size_t offset = 0) {
#ifdef DEBUG
        cout << "findNextLine(" << line << ", minDistance=" << minDistance
             << ", offset=" << offset << endl;
#endif
        Line lline     = {line.lineCenter, line.angle};
        bool direction = lline.leftOfPoint(center());
        angle_t angle  = line.angle;
        angle_t perp   = angle.perpendicular(direction);

        Pixel searchStart =
            move(line.lineCenter, perp, 2 * line.width + offset);
        if (minDistance)
            searchStart = move(searchStart, line.angle, minDistance);
#ifdef DEBUG
        cout << "searchStart = " << searchStart << endl;
#endif

        size_t minWidth = line.width / 2;

        BresenhamLine path = {searchStart, angle, W, H};
        Pixel pixel;
        do {
            pixel = path.next();
            while (path.hasNext() && get(pixel) == 0x00)
                pixel = path.next();
            Pixel firstWhite = pixel;
            while (path.hasNext() && get(pixel) != 0x00)
                pixel = path.next();
            Pixel firstBlack = pixel;  // TODO: OBOE, but I don't really care
            Pixel middle     = Pixel::average(firstBlack, firstWhite);
            LineResult possibleLine = checkLine(middle, perp, minWidth);
#ifdef DEBUG
            cout << possibleLine << endl;
#endif
            if (possibleLine.valid)
                return possibleLine;
        } while (path.hasNext());
        return LineResult::invalid();
    }

    LineResult checkLine(Pixel pixel, angle_t angle, size_t minWidth) {
#ifdef DEBUG
        cout << "checkLine(" << pixel << ", " << angle
             << ", minWidth=" << minWidth << ")" << endl;
#endif
        GetMiddleResult middle = getMiddle(pixel, angle);
        if (!middle.valid || middle.width < minWidth)
            // throw std::runtime_error("TODO: No middle point found");
            return LineResult::invalid();

        constexpr size_t range = angle_t::resolution() / 24;  // 2 * 15°
        HoughResult result =
            findLineAngleAccurateRange<range>(middle.pixel, angle);

// if (result.count < MINIMIM_LINE_WEIGHTED_VOTE_COUNT)
// throw std::runtime_error("TODO: find a new starting position");
#ifdef DEBUG
        cout << "checkLine: result.count = " << result.count << endl;
#endif
        bool valid = result.count >= MINIMIM_LINE_WEIGHTED_VOTE_COUNT;
        return {valid, middle.pixel, middle.width, result.angle};
    }

    std::tuple<std::array<std::optional<LineResult>, 5>,
               std::array<std::optional<Point>, 4>>
    findSquare() {
        auto firstLines = getFirstLines();
        auto secondLine = findNextLine(firstLines[0]);
        auto thirdLine  = findNextLine(firstLines[1]);

        std::array<std::optional<LineResult>, 5> lines;
        std::array<std::optional<Point>, 4> points;

        lines[0] = firstLines[0];
        lines[1] = firstLines[1];

        if (secondLine.valid && thirdLine.valid) {
            lines[2]  = secondLine;
            lines[3]  = thirdLine;
            points[0] = intersect(firstLines[0], secondLine);
            points[1] = intersect(firstLines[1], thirdLine);
            size_t minDistance =
                std::max(std::abs(points[0]->x - points[1]->x),
                         std::abs(points[0]->y - points[1]->y));
            minDistance -= minDistance / 4;  // use 3/4 of distance
            size_t offset          = 0;
            const size_t maxOffset = minDistance / 2;
            const size_t offsetIncr =
                std::max(secondLine.width, thirdLine.width);
            LineResult fourthLine;
            while (!fourthLine.valid && offset < maxOffset) {
#ifdef DEBUG
                cout << "Find fourth line ============================" << endl;
#endif
                fourthLine = findNextLine(secondLine, minDistance, offset);
                if (!fourthLine.valid)
                    fourthLine = findNextLine(thirdLine, minDistance, offset);
                offset += offsetIncr;
            }
            if (fourthLine.valid) {
                lines[4]  = fourthLine;
                points[2] = intersect(secondLine, fourthLine);
                points[3] = intersect(thirdLine, fourthLine);
            }
        } else if (secondLine.valid) {
            lines[2] = secondLine;
            // TODO
        } else if (thirdLine.valid) {
            lines[3] = thirdLine;
            // TODO
        } else {
            // TODO
        }

        return {lines, points};
    }

    static Point intersect(LineResult a, LineResult b) {
        return Line::intersect(Line(a.lineCenter, a.angle),
                               Line(b.lineCenter, b.angle));
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

    size_t drawLine(Pixel pixel, int cos, int sin) {
        BresenhamLine line = {pixel, cos, sin, W, H};
        return drawLine(line);
    }

    size_t drawLine(BresenhamLine line) {
        while (line.hasNext())
            set(line.next());
        return line.getCurrentLength();
    }

    size_t drawLine(Pixel pixel, double angle) {
        BresenhamLine line = {pixel, angle, W, H};
        return drawLine(line);
    }

    size_t drawLine(Pixel pixel, CosSin angle) {
        BresenhamLine line = {pixel, angle, W, H};
        return drawLine(line);
    }

  private:
    Img_t mask;
};