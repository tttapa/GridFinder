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

// #define DEBUG

using std::cerr;
using std::cout;
using std::endl;

/**
 * @brief   Struct as a result of the hough function. It gives the most 
 *          plausible angle, and a vote count that's an indication of how many
 *          white pixels lie on that line (may be weighted and truncated).
 * @see     GridMask::hough
 */
struct HoughResult {
    angle_t angle;
    uint count;
    bool operator<(HoughResult other) const { return count < other.count; }
};

/**
 * @brief   Function for printing a HoughResult.
 * @see     HoughResult
 */
inline std::ostream &operator<<(std::ostream &os, HoughResult h) {
    return os << h.angle << " rad: " << h.count;
}

struct LineResult {
    Pixel lineCenter;
    uint width;
    angle_t angle;
};
inline std::ostream &operator<<(std::ostream &os, LineResult l) {
    return os << "LineResult(" << l.lineCenter << ", " << l.width << ", "
              << l.angle << ")";
}

template <uint W, uint H>
class GridMask {
  public:
    using Img_t = TMatrix<uint8_t, H, W>;

    GridMask(const Img_t &mask) : mask(mask) {}
    GridMask() : mask{} {}

    struct FirstLineEstimate {
        Pixel middle;
        uint width;
        angle_t angleEstimate;
    };

    // TODO: can we make this dynamic?
    constexpr static uint MINIMUM_START_LINE_WIDTH = 10;
    constexpr static uint MINIMIM_START_LINE_WEIGHTED_VOTE_COUNT =
        (W + H) / 2;  // TODO

    std::optional<FirstLineEstimate> getFirstLine(Pixel point) const {
        // Find an estimate for the slope of the line.
        // Doesn't have to be accurate, just to find the width.
        HoughResult firstResult = findLineAngle(point);
        angle_t firstAngle      = firstResult.angle;

        // There could be noise, so the first "line" could be just a white
        // bit of noise
        if (firstResult.count < MINIMIM_START_LINE_WEIGHTED_VOTE_COUNT)
            return std::nullopt;

        GetMiddleResult middle = getMiddleWithRetries(point, firstAngle);

        if (!middle.valid || middle.width < MINIMUM_START_LINE_WIDTH)
            return std::nullopt;

        return {true, middle.pixel, middle.width, firstAngle};
    }

    // Don't use vertical lines as the first line, as this can result in finding
    // a first square that's not in the center of the frame.
    constexpr static uint MAXIMUM_START_LINE_WIDTH = 32;

    std::optional<FirstLineEstimate> getFirstLineEstimate(uint x) const {
        if (x >= W)
            throw std::out_of_range("x out of range");

        CenterPointOutLineIterator c = H;

        // If the center pixel is white, look for the first black pixels in both
        // directions
        if (get(x, c.getCenter())) {
            uint first_white;
            uint y = c.getCenter();
            // Find the lowest white pixel of the line through the center
            while (y < H) {
                if (get(x, y))
                    first_white = y;
                else
                    break;
                --y;
            }
            // Find the highest white pixel of the line through the center
            uint last_white;
            y = c.getCenter();
            while (y < H) {
                if (get(x, y))
                    last_white = y;
                else
                    break;
                ++y;
            }
            y = (first_white + last_white) / 2;
            assert(last_white >= first_white);
            if (last_white - first_white >= MAXIMUM_START_LINE_WIDTH)
                return std::nullopt;
            else
                return getFirstLine({x, y});
        }
        // If the center pixel is not white, look for the first white pixel in
        // both directions
        uint first_white = H;
        while (c.hasNext()) {
            uint y = c.next();
            if (get(x, y)) {
                first_white = y;
                break;
            }
        }
        if (first_white >= H)
            return std::nullopt;

        uint last_white = first_white;
        // If the first white pixel is below the center
        if (first_white < c.getCenter()) {
            // Look downwards for the last white pixel of that line
            uint y = first_white;
            while (y < H) {
                if (get(x, y))
                    last_white = y;
                else
                    break;
                --y;
            }
            y = (first_white + last_white) / 2;
            assert(first_white >= last_white);
            if (first_white - last_white >= MAXIMUM_START_LINE_WIDTH)
                return std::nullopt;
            else
                return getFirstLine({x, y});

            // If the first white pixel is above the center
        } else {
            // Look upwards for the last white pixel of that line
            uint y = first_white;
            while (y < H) {
                if (get(x, y))
                    last_white = y;
                else
                    break;
                ++y;
            }
            y = (first_white + last_white) / 2;
            assert(last_white >= first_white);
            if (last_white - first_white >= MAXIMUM_START_LINE_WIDTH)
                return std::nullopt;
            else
                return getFirstLine({x, y});
        }
    }

    // If no valid first line is found in a column, don't try the column next
    // to it, but go a little farther.
    constexpr static uint FIRST_LINE_INVALID_HORIZONTAL_JUMP = 10;

    // ONLY USED IN FIRST PART OF ALGORITHM TO FIND FIRST LINE
    FirstLineEstimate getFirstLineEstimate() const {
        CenterPointOutLineIterator c = W / FIRST_LINE_INVALID_HORIZONTAL_JUMP;
        while (c.hasNext()) {
            uint x = c.next() * FIRST_LINE_INVALID_HORIZONTAL_JUMP;
            if (FirstLineEstimate fle = getFirstLineEstimate(x); fle.valid)
                return fle;
        }
        throw std::runtime_error("Error: no white pixels found");
    }

    constexpr static uint HOUGH_MAX_GAP = 32;

    // TODO: HoughResult --> ? (better name - line with vote count)
    // TODO: hough() --> countVotes()
    /**
     * @brief   Starting from the given pixel, move in the direction of the 
     *          given angle, and count the number of white pixels, weighing them
     *          more as the distance increases. When more than HOUGH_MAX_GAP 
     *          consecutive black pixels are encountered, stop looking any 
     *          farther.
     * @param   px
     *          The starting point.
     * @param   angle
     *          The direction to look in.
     */
    HoughResult hough(Pixel px, angle_t angle) const {
        BresenhamLine line   = {px, angle, W, H};
        uint count           = 0;
        uint_fast16_t weight = 0;
        uint previousWhite   = 0;
        while (line.hasNext()) {
            Pixel point = line.next();
            // If pixel is white, then add weight to count
            if (get(point)) {
                // TODO: can we use line.getCurrentLength as weight?
                count += weight++;
                previousWhite = line.getCurrentLength();
            } else if (line.getCurrentLength() - previousWhite <=
                       HOUGH_MAX_GAP) {
                break;
            }
        }
        return {angle, count};
    }

    /** 
     * @brief   Scan 360° around a given pixel, and find the angle with the most
     *          white pixels. This version is not very accurate, as it doesn't
     *          handle the case where multiple angles have the same maximum 
     *          count.  
     *          On the other hand, it's a bit faster than the more accurate 
     *          alternatives, making it a good candidate for finding the first
     *          angle estimate, to narrow down the search space.
     */
    HoughResult findLineAngle(Pixel px) const {
        std::array<HoughResult, angle_t::resolution()> houghRes;
        for (uint i = 0; i < angle_t::resolution(); ++i)
            houghRes.at(i) = hough(px, angle_t(i));
        return *std::max_element(houghRes.begin(), houghRes.end());
    }

    HoughResult findLineAngleAccurate(Pixel px) {
        std::array<HoughResult, angle_t::resolution()> houghRes;
        for (uint i = 0; i < angle_t::resolution(); ++i)
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
            if (last_max == max)  // If we're back where we started
                break;            // don't keep going around in circles
        }

        // Find first angle with the same count as max_count (- 1 step)
        while (first_max->count == max->count) {
            if (first_max == houghRes.begin())
                first_max = houghRes.end();
            --first_max;
            if (first_max == max)  // If we're back where we started
                break;             // don't keep going around in circles
        }

        return {
            angle_t::average(first_max->angle, last_max->angle),
            max->count,
        };
    }

    /** 
     * @brief   Similar to `findLineAngle`
     */
    template <uint N>
    HoughResult findLineAngleAccurateRange(Pixel px, angle_t centerAngle) {
        static_assert(2 * N < angle_t::resolution());
        std::array<HoughResult, 2 * N + 1> houghRes;
        uint centerAngleIndex = centerAngle.getIndex();

        // If the range contains no discontinuities
        // ├───┬───┬───╤═══╤═══╤═══╪═══╤═══╤═══╤───┬───┬───┤
        // 0          C-N          C          C+N         R-1
        // C = center
        // N = number of steps around the center
        // R = resolution (R steps = 720°)
        if (centerAngleIndex - N < angle_t::resolution() &&
            centerAngleIndex + N < angle_t::resolution()) {
            uint initialAngleIndex = centerAngleIndex - N;
            for (uint i = 0; i <= 2 * N; ++i)
                houghRes.at(i) = hough(px, angle_t(initialAngleIndex + i));
        }
        // If the 0-2PI discontinuity is in the first part of the range
        // (< centerAngle)
        // ╞═══╪═══╤═══╤═══╤───┬───┬───┬───┬───┬───┬───╤═══╡
        // 0   C          C+N                         C-N R-1
        else if (centerAngleIndex + N < angle_t::resolution()) {
            uint initialAngleIndex =
                angle_t::resolution() + centerAngleIndex - N;
            uint i = 0;
            for (uint j = initialAngleIndex; j < angle_t::resolution();
                 ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
            for (uint j = 0; j <= centerAngleIndex + N; ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
        }
        // If the 0-2PI discontinuity is in the second part of the range
        // (> centerAngle)
        // ╞═══╤───┬───┬───┬───┬───┬───┬───╤═══╤═══╤═══╪═══╡
        // 0  C+N                         C-N          C  R-1
        else if (angle_t::resolution() - N < angle_t::resolution()) {
            uint initialAngleIndex = centerAngleIndex - N;
            uint i                 = 0;
            for (uint j = initialAngleIndex; j < angle_t::resolution();
                 ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
            for (uint j = 0; j <= N + centerAngleIndex - angle_t::resolution();
                 ++i, ++j)
                houghRes.at(i) = hough(px, angle_t(j));
        } else {  // TODO: dangerous when flying!
            throw std::runtime_error("Something is very wrong");
        }
        // If there are multiple elements with the same maximum count,
        // max_element will return an iterator to the first element with this
        // count
        auto max = std::max_element(houghRes.begin(), houghRes.end());

        // Find the first element after the first maximum that has a lower count
        // than ⅞ of the maximum count
        // and then subtract one to get the last element that has the maximum
        // count.
        // If no element with a lower count is found, find_if returns
        // houghRes.end(), which means that last_max will be an iterator to
        // the last element of the array.
        // In that case, there's probably something wrong: the line might be
        // too thick, and all counts within the specified range are very high.

        // predicate res.count < 7 * max->count / 8;
        auto predicateLessThanSevenEighths = [&](auto res) {
            return res.count < max->count - max->count / 8;
        };
        // std::find_if returns an iterator the first element that satisfies the
        // predicate
        //
        auto first_max =
            std::find_if(std::make_reverse_iterator(max),  //
                         houghRes.rend(), predicateLessThanSevenEighths);
        auto last_max =
            std::find_if(max,  //
                         houghRes.end(), predicateLessThanSevenEighths);

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
    static const uint MAX_GAP = 10;
    // static const uint MAX_GAP = (W + H) / 2 / 10;

    static const uint maxLineWidth = 32;

    /**
     * @brief   Get the width of the line at a given point.
     * 
     * Start at the given pixel, and move in the given direction for maxLineGap
     * pixels, in each pixel, move in the perpendicular direction until a black
     * pixel is encountered. At this point, the half width of the line from the
     * base line is known.  
     * Finally, the maximum of all distances is returned.
     * 
     * If the width at any point exceeds the maxLineWidth, maxLineWidth is 
     * returned.
     * 
     * @param pixel 
     * @param angle 
     * @param maxLineGap 
     * @param plus90deg 
     * @return uint 
     */
    uint getWidthAtPointOnLine(Pixel pixel, CosSin angle,
                               uint maxLineGap = MAX_GAP,
                               bool plus90deg  = true) const {
        BresenhamLine alongLine   = {pixel, angle, W, H};
        CosSin perpendicularAngle = angle.perpendicular(plus90deg);
        uint maxWidthSoFar        = 0;
        // Follow a path along the given line for maxLineGap pixels
        for (uint i = 0; i <= maxLineGap && alongLine.hasNext(); ++i) {
            Pixel pixelAlongLine = alongLine.next();
            // For each pixel along this path, move away from the line,
            // perpendicular to it, untill you find a black pixel, until you
            // fall off the canvas, or until the maximum line width is exceeded.
            BresenhamLine perpendicular = {pixelAlongLine, perpendicularAngle,
                                           W, H};
            while (perpendicular.hasNext() &&
                   perpendicular.getCurrentLength() <= maxLineWidth) {
                Pixel pixel = perpendicular.next();
                if (get(pixel) == 0x00)
                    break;
            }
            if (perpendicular.getCurrentLength() > maxLineWidth)
                // throw std::runtime_error("Error: Endless loop detected");
                return maxLineWidth;

            // If we found a black pixel before exceeding the maximum line width
            if (perpendicular.getCurrentLength() > maxWidthSoFar)
                // getCurrentLength() gives the length to the first black pixel,
                // so that's one pixel too much. Fix at the end of the function.
                maxWidthSoFar = perpendicular.getCurrentLength();
        }
#ifdef DEBUG
        cout << "getWidthAtPointOnLine(" << pixel << ") → "
             << (maxWidthSoFar - 1) << endl;
#endif
        return maxWidthSoFar - 1;
    }

    struct GetMiddleResult {
        Pixel pixel;
        uint width;
    };

    /**
     * @brief Find the middle of a line.
     * 
     * @param   pointOnLine
     *          A point that lies on the line. 
     * @param   angle
     *          An estimate of the slope of the line.
     * @param   maxLineGap
     *          The maximum gap of black pixels that can be recovered.
     * @return  TODO 
     */
    std::optional<GetMiddleResult> getMiddle(Pixel pointOnLine,
                                             CosSin lineAngle,
                                             uint maxLineGap = MAX_GAP) const {
        // If the given point does not lie on a line, we can't find the middle
        // of the line
        if (get(pointOnLine) == 0x00)
            return std::nullopt;

        CosSin oppositeAngle = lineAngle.opposite();
        // Get the half width of the line in four directions.
        uint widthUpper1 = getWidthAtPointOnLine(pointOnLine, lineAngle,  //
                                                 maxLineGap / 2, true);
        uint widthLower1 = getWidthAtPointOnLine(pointOnLine, lineAngle,  //
                                                 maxLineGap / 2, false);
        uint widthUpper2 = getWidthAtPointOnLine(pointOnLine, oppositeAngle,  //
                                                 maxLineGap / 2, false);
        uint widthLower2 = getWidthAtPointOnLine(pointOnLine, oppositeAngle,  //
                                                 maxLineGap / 2, true);

        // Use the maximum of the upper width and the maximum of the lower width
        uint widthUpper = std::max(widthUpper1, widthUpper2);
        uint widthLower = std::max(widthLower1, widthLower2);

        // If either of the widths exceeds the maxLineWidth, the result is
        // invalid: it is probably a large white spot, not a thin line
        if (widthUpper >= maxLineWidth || widthLower >= maxLineWidth)
            return std::nullopt;

        // Use the difference between the two widths to move the given point
        // towards the center
        int middlePointCorrection_x2 = widthUpper - widthLower;
        uint middlePointCorrectionDistance =
            std::abs(middlePointCorrection_x2) / 2;
        // Should we move up or down? (+90° or -90° from the estimated angle)
        bool corrDirection = middlePointCorrection_x2 > 0;
        CosSin corrAngle   = lineAngle.perpendicular(corrDirection);

        BresenhamLine corr = {pointOnLine, corrAngle, W, H};
        Pixel middle       = corr.next();  // == pointOnLine
        while (corr.hasNext() &&
               corr.getCurrentLength() <= middlePointCorrectionDistance)
            middle = corr.next();
        return {middle, widthUpper + widthLower - 1, true};
    }

    // When we're at an intersection and no width can be determined, how
    // far should we jump along the line before trying again.
    constexpr static uint RETRY_JUMP_DISTANCE = (W + H) / 20;  // TODO

    /**
     * @brief   In exceptional cases, we might want to find the middle of a line
     *          in a point that happens to lie at the intersection of the line
     *          with another line. In that case, we first have to move along the
     *          line to a point away from the intersection, and calculate the 
     *          middle in this point. Otherwise, the width would be extremely
     *          large.
     * 
     * @param start 
     * @param angle 
     * @return GetMiddleResult 
     */
    GetMiddleResult getMiddleWithRetries(Pixel start, angle_t angle) const {
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

    constexpr static uint MINIMIM_LINE_WEIGHTED_VOTE_COUNT =
        (W + H) / 2;  // TODO

    std::array<std::optional<LineResult>, 2> getFirstLines() {
        auto start = getFirstLineEstimate();

        constexpr uint range = angle_t::resolution() / 40;  // 2 * 9°
        HoughResult result1  = findLineAngleAccurateRange<range>(
            start.middle, start.angleEstimate);
        HoughResult result2 = findLineAngleAccurateRange<range>(
            start.middle, start.angleEstimate.opposite());
#ifdef DEBUG
        cout << "result1.angle = " << result1.angle << endl;
        cout << "result2.angle = " << result2.angle << endl;
#endif
        return {{{start.middle, start.width, result1.angle},
                 {start.middle, start.width, result2.angle}}};
    }

    Pixel move(Pixel start, CosSin angle, uint distance) const {
        BresenhamLine path = {start, angle, W, H};
        Pixel end;
        while (path.hasNext() &&
               path.getCurrentLength() <= distance)  // TODO: OBOE?
            end = path.next();
        return end;
    }

    constexpr static Pixel center() { return {(W - 1) / 2, (H - 1) / 2}; }

    LineResult findNextLine(LineResult line, uint minDistance = 0,
                            uint offset = 0) {
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

        uint minWidth = line.width / 2;

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

    LineResult checkLine(Pixel pixel, angle_t angle, uint minWidth) {
#ifdef DEBUG
        cout << "checkLine(" << pixel << ", " << angle
             << ", minWidth=" << minWidth << ")" << endl;
#endif
        GetMiddleResult middle = getMiddle(pixel, angle);
        if (!middle.valid || middle.width < minWidth)
            // throw std::runtime_error("TODO: No middle point found");
            return LineResult::invalid();

        constexpr uint range = angle_t::resolution() / 40;  // 2 * 9°
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
        std::array<LineResult, 2> firstLines;
        try {
            firstLines = getFirstLines();
        } catch (std::runtime_error &e) {
            cerr << ANSIColors::redb << e.what() << ANSIColors::reset << endl;
            return {};
        }
        auto secondLine = findNextLine(firstLines[0]);
        auto thirdLine  = findNextLine(firstLines[1]);

        std::array<std::optional<LineResult>, 5> lines;
        std::array<std::optional<Point>, 4> points;

        lines[0] = firstLines[0];
        lines[1] = firstLines[1];

        if (secondLine.valid && thirdLine.valid) {
            lines[2]         = secondLine;
            lines[3]         = thirdLine;
            points[0]        = intersect(firstLines[0], secondLine);
            points[1]        = intersect(firstLines[1], thirdLine);
            uint minDistance = std::max(std::abs(points[0]->x - points[1]->x),
                                        std::abs(points[0]->y - points[1]->y));
            minDistance -= minDistance / 4;  // use 3/4 of distance
            uint offset           = 0;
            const uint maxOffset  = minDistance / 2;
            const uint offsetIncr = std::max(secondLine.width, thirdLine.width);
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
    inline constexpr uint8_t get(uint x, uint y) const { return mask[y][x]; }
    inline constexpr uint8_t get(Pixel px) const { return mask[px.y][px.x]; }
    inline constexpr void set(Pixel px) { mask[px.y][px.x] = 0xFF; }

    std::ostream &print(std::ostream &os) {
        for (uint y = 0; y < H; ++y) {
            for (uint x = 0; x < W; ++x)
                os << (get(x, y) ? "⬤ " : "◯ ");
            os << "\r\n";
        }
        return os;
    }

    uint drawLine(Pixel pixel, int cos, int sin) {
        BresenhamLine line = {pixel, cos, sin, W, H};
        return drawLine(line);
    }

    uint drawLine(BresenhamLine line) {
        while (line.hasNext())
            set(line.next());
        return line.getCurrentLength();
    }

    uint drawLine(Pixel pixel, double angle) {
        BresenhamLine line = {pixel, angle, W, H};
        return drawLine(line);
    }

    uint drawLine(Pixel pixel, CosSin angle) {
        BresenhamLine line = {pixel, angle, W, H};
        return drawLine(line);
    }

  private:
    Img_t mask;
};