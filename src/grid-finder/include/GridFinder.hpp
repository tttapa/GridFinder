#pragma once

#include <ANSIColors.hpp>
#include <Angle.hpp>
#include <Bresenham.hpp>
#include <CenterPointOutLineIterator.hpp>
#include <HelperStructs.hpp>
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
using std::optional;

/**
 * @brief   A class for finding the lines and intersections in an image of a 
 *          grid.
 * @tparam  W
 *          The width of the image in pixels.
 * @tparam  H
 *          The height of the image in pixels.
 */
#if 1
template <size_t W, size_t H>
#else
constexpr uint H = 308;
constexpr uint W = 410;
#endif
class GridFinder {
  public:
    /// The type of the image mask.
    using Img_t = TMatrix<uint8_t, H, W>;

    /// Constructor from a given mask.
    GridFinder(const Img_t &mask) : mask(mask) {}
    /// Default constructor (creates a mask of all zeros).
    GridFinder() : mask{} {}

#pragma region Finding the angle of a line......................................

    /// When this many consecutive black pixels are encountered, stop following
    /// the line.
    constexpr static uint HOUGH_MAX_GAP = 16;

    /**
     * @brief   Starting from the given pixel, move in the direction of the 
     *          given angle, and count the number of white pixels, weighing them
     *          more as the distance increases. When more than #HOUGH_MAX_GAP 
     *          consecutive black pixels are encountered, stop looking any 
     *          farther.
     * @param   px
     *          The starting point.
     * @param   angle
     *          The direction to look in.
     */
    HoughResult hough(Pixel px, angle_t angle) const {
        BresenhamLine line = {px, angle, W, H};
        uint previousWhite = 0;
        while (line.hasNext()) {
            Pixel point = line.next();
            // If pixel is white
            if (get(point))
                previousWhite = line.getCurrentLength();
            else if (line.getCurrentLength() - previousWhite >= HOUGH_MAX_GAP)
                break;
        }
        return {angle, previousWhite};
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

#if 0
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
#endif

    /** 
     * @brief   Similar to #findLineAngle, but is more accurate, because
     *          it uses the average of the range of angles with the hightest
     *          vote counts. It also doesn't scan 360°, but takes 2@p N samples 
     *          around a given center angle.
     */
    template <uint N>
    HoughResult findLineAngleAccurateRange(Pixel px,
                                           angle_t centerAngle) const {
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
        // If the 0-2π discontinuity is in the first part of the range
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
        // If the 0-2π discontinuity is in the second part of the range
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
        } else {
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
            cerr << ANSIColors::yellow
                 << "Warning: angle with maximum count could lie outside of "
                    "the specified range\r\n"
                 << ANSIColors::reset;

        if (first_max != houghRes.rbegin())
            --first_max;
        if (last_max != houghRes.begin())
            --last_max;

        return {
            angle_t::average(first_max->angle, last_max->angle),
            first_max->count,  // TODO: should this be max->count?
        };
    }

#pragma endregion

#pragma region Finding the middle of a line.....................................

    /**
     * @brief   The maximum number of pixels that can be black within a line,
     *          while still being detected correctly.
     */
    static const uint MAX_GAP = 10;

    /**
     * @brief   "Lines" that are wider than this value are ignored, to prevent
     *          white blobs (of sunlight, for example) from being detected as
     *          random lines.
     */
    static const uint MAX_LINE_WIDTH = 32;

    /**
     * @brief   Get the width of the line at a given point.
     * 
     * Start at the given pixel, and move in the given direction for 
     * @p maxLineGap pixels, in each pixel, move in the perpendicular direction 
     * until a black pixel is encountered. At this point, the half width of the 
     * line from the base line is known.  
     * Finally, the maximum of all distances is returned.
     * 
     * If the width at any point exceeds the #MAX_LINE_WIDTH, #MAX_LINE_WIDTH is 
     * returned.
     * 
     * @param   pixel
     *          A point on the line.
     * @param   angle 
     *          The slope of the line.
     * @param   maxLineGap
     *          The maximum gap that could be in the line. Do this many width 
     *          samples, and return the maximum.
     * @param   plus90deg
     *          Should 90° be added or subtracted to get the perpendicular
     *          direction?
     * @return  
     *          The maximum half width of the line, over the @p maxLineGap
     *          pixels along the given line.
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
            // perpendicular to it, until you find a black pixel, until you
            // fall off the canvas, or until the maximum line width is exceeded.
            BresenhamLine perpendicular = {pixelAlongLine, perpendicularAngle,
                                           W, H};
            while (perpendicular.hasNext() &&
                   perpendicular.getCurrentLength() <= MAX_LINE_WIDTH) {
                Pixel pixel = perpendicular.next();
                if (get(pixel) == 0x00)
                    break;
            }
            if (perpendicular.getCurrentLength() > MAX_LINE_WIDTH)
                // throw std::runtime_error("Error: Endless loop detected");
                return MAX_LINE_WIDTH;

            // If we found a black pixel before exceeding the maximum line width
            if (perpendicular.getCurrentLength() > maxWidthSoFar)
                // getCurrentLength() gives the length to the first black pixel,
                // so that's one pixel too much. Fix at the end of the function.
                maxWidthSoFar = perpendicular.getCurrentLength();
        }
        return maxWidthSoFar - 1;
    }

    /**
     * @brief   Find the middle of a line.
     * 
     * @param   pointOnLine
     *          A point that lies on the line. 
     * @param   lineAngle
     *          An estimate of the slope of the line.
     * @param   maxLineGap
     *          The maximum gap of black pixels that can be recovered.
     * @return  // TODO 
     */
    optional<GetMiddleResult> getMiddle(Pixel pointOnLine, CosSin lineAngle,
                                        uint maxLineGap = MAX_GAP) const {
        // If the given point does not lie on a line, we can't find the middle
        // of the line
        if (get(pointOnLine) == 0x00)
            return std::nullopt;

        CosSin oppositeAngle = lineAngle.opposite();
        // Get the half width of the line in four directions.
        // U2   U1
        // ↑↑↑ ↑↑↑
        // ---o---> line
        // ↓↓↓ ↓↓↓
        // L2   L1
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

        // If either of the widths exceeds the #MAX_LINE_WIDTH, the result is
        // invalid: it is probably a large white spot, not a thin line
        if (widthUpper >= MAX_LINE_WIDTH || widthLower >= MAX_LINE_WIDTH)
            return std::nullopt;

        // Use the difference between the two widths to move the given point
        // towards the center
        int middlePointCorrection_x2 = widthUpper - widthLower;
        uint middlePointCorrDistance = std::abs(middlePointCorrection_x2) / 2;
        // Should we move up or down? (+90° or -90° from the estimated angle)
        bool corrDirection = middlePointCorrection_x2 > 0;
        CosSin corrAngle   = lineAngle.perpendicular(corrDirection);
        Pixel middle = move(pointOnLine, corrAngle, middlePointCorrDistance);
        return GetMiddleResult{middle, widthUpper + widthLower - 1};
    }

    /// When we're at an intersection and no width can be determined, how
    /// far should we jump along the line before trying again.
    constexpr static uint RETRY_JUMP_DISTANCE = MAX_LINE_WIDTH;  // TODO

    /**
     * @brief   In exceptional cases, we might want to find the middle of a line
     *          in a point that happens to lie at the intersection of the line
     *          with another line. In that case, we first have to move along the
     *          line to a point away from the intersection, and calculate the 
     *          middle in this point. Otherwise, the width would be extremely
     *          large.
     * 
     * @param   start 
     *          A point on the line.
     * @param   angle 
     *          The slope of the line.
     * @return  // TODO
     */
    optional<GetMiddleResult> getMiddleWithRetries(Pixel start,
                                                   angle_t angle) const {
        optional<GetMiddleResult> middle = std::nullopt;
        Pixel previousPixel;
        do {
            previousPixel = start;
            middle        = getMiddle(start, angle);
            start         = move(start, angle, RETRY_JUMP_DISTANCE);
        } while (!middle.has_value() && previousPixel != start);
        return middle;
    }

#pragma endregion

#pragma region Getting and Setting Pixels.......................................
    /**
     * @brief   Get the value of the given pixel.
     * 
     * @param   x 
     *          The x-coordinate of the pixel to check.
     * @param   y 
     *          The y-coordinate of the pixel to check.
     * @return  The value of the given pixel.
     */
    inline constexpr uint8_t get(uint x, uint y) const { return mask[y][x]; }
    /**
     * @brief   Get the value of the given pixel.
     * 
     * @param   px 
     *          The pixel to check.
     * @return  The value of the given pixel.
     */
    inline constexpr uint8_t get(Pixel px) const { return get(px.x, px.y); }
    /**
     * @brief   Set the value of the given pixel.
     * 
     * @param   px 
     *          The pixel to set the value of.
     * @param   value
     *          The value to set the pixel to.
     */
    inline constexpr void set(Pixel px, uint8_t value = 0xFF) {
        mask[px.y][px.x] = value;
    }

#pragma endregion

#pragma region Printing and Drawing.............................................

    /// Print the mask as "pixels" to the given output stream.
    std::ostream &print(std::ostream &os) const {
        for (uint y = 0; y < H; ++y) {
            for (uint x = 0; x < W; ++x)
                os << (get(x, y) ? "⬤ " : "◯ ");
            os << "\r\n";
        }
        return os;
    }

    /// Print the mask as a C++ Matrix to the given output stream.
    std::ostream &printMaskMatrix(std::ostream &os) const {
        os << "TMatrix<uint8_t, " << H << ", " << W << "> mask = {{\r\n";
        for (const auto &row : mask) {
            os << "    {";
            uint ctr = W;
            for (uint8_t el : row) {
                os << std::hex << std::showbase << +el
                   << (--ctr == 0 ? "" : ", ");
            }
            os << "},\r\n";
        }
        return os << "}};";
    }

    /// Draw a line on the mask. Used only for testing.
    uint drawLine(Pixel pixel, int cos, int sin) {
        BresenhamLine line = {pixel, cos, sin, W, H};
        return drawLine(line);
    }
    /// Draw a line on the mask. Used only for testing.
    uint drawLine(BresenhamLine line) {
        while (line.hasNext())
            set(line.next());
        return line.getCurrentLength();
    }
    /// Draw a line on the mask. Used only for testing.
    uint drawLine(Pixel pixel, CosSin angle) {
        BresenhamLine line = {pixel, angle, W, H};
        return drawLine(line);
    }

#pragma endregion

#pragma region Utilities........................................................

    /** 
     * @brief   Move a pixel in a given direction for a given distance.
     * 
     * @param   start
     *          The pixel to start from.
     * @param   angle
     *          The direction to move in.
     * @param   distance
     *          The number of pixels to move.
     * @return  The pixel after the move.
     */
    Pixel move(Pixel start, CosSin angle, uint distance) const {
        BresenhamLine path = {start, angle, W, H};
        Pixel end;
        while (path.hasNext() && path.getCurrentLength() <= distance)
            end = path.next();
        return end;
    }

    /// Get the center pixel of the frame.
    constexpr static Pixel center() { return {(W - 1) / 2, (H - 1) / 2}; }

    /// Get the point of intersection of two lines.
    static Point intersect(LineResult a, LineResult b) {
        return Line::intersect(Line(a.lineCenter, a.angle),
                               Line(b.lineCenter, b.angle));
    }

#pragma endregion

#pragma region Finding the first line...........................................

    // TODO: can we make this dynamic?
    /// The minimum width the first line must have.
    constexpr static uint MINIMUM_START_LINE_WIDTH = 10;
    /// The minimum number of Hough votes the first line must have.
    /// @see    HoughResult::count
    constexpr static uint MINIMIM_START_LINE_WEIGHTED_VOTE_COUNT = (W + H) / 10;

    /**
     * @brief   Get an estimate of the line through a given pixel.
     *          Find an estimate of the slope, then use that to determine the 
     *          middle and the width of the line.
     * 
     * @param   point
     *          Find the line that goes through this point.
     * @return
     *          Returns no result if no line can be found that has a vote count 
     *          higher than #MINIMIM_START_LINE_WEIGHTED_VOTE_COUNT, or if the 
     *          line candidate has a width that's less than 
     *          #MINIMUM_START_LINE_WIDTH.  
     *          Also doesn't return an estimate if #getMiddleWithRetries fails.
     */
    optional<FirstLineEstimate> getFirstLineEstimate(Pixel point) const {
        // Find an estimate for the slope of the line.
        // Doesn't have to be accurate, just to find the width.
        HoughResult firstResult = findLineAngle(point);
        angle_t firstAngle      = firstResult.angle;

        // There could be noise, so the first "line" could be just a white
        // bit of noise
        if (firstResult.count < MINIMIM_START_LINE_WEIGHTED_VOTE_COUNT)
            return std::nullopt;

        optional<GetMiddleResult> middle =
            getMiddleWithRetries(point, firstAngle);

        if (!middle.has_value() || middle->width < MINIMUM_START_LINE_WIDTH)
            return std::nullopt;

        return FirstLineEstimate{middle->pixel, middle->width, firstAngle};
    }

    /// Don't use vertical lines as the first line, as this can result in
    /// finding a first square that's not in the center of the frame.
    constexpr static uint MAXIMUM_VERTICAL_START_LINE_WIDTH = 32;

    /**
     * @brief   Find the vertical range of white pixels closest to the center of
     *          the given column, then try to find an estimate of the line going
     *          through the center of this range.
     * 
     * ### Why near-vertical lines are a problem
     * 
     * Consider the following example: all letters are white pixels, all dots 
     * are black. The arrow indicates the column we're searching in, and `C` is
     * the center of the frame. 
     * ```
     *       ↓
     * X X X X X X X
     * · · X · · · ·
     * · · X · · · ·
     * · · X · · · ·
     * X X X X X X X
     * · · X · · · ·
     * · · · C · · ·
     * · · · R · · ·
     * X X X R X X X
     * · · · M · · ·
     * · · · R · · ·
     * · · · R · · ·
     * X X X X X X X
     * ```
     * We're looking for the range of white pixels in the indicated column, 
     * closest to the center `C`. This turns out to be the range labeled with 
     * `R` (including `C`). The middlepoint of that range is `M`, so that'll be
     * the starting point for our first line.  
     * However, notice how we ended up in a square that's farther away from the 
     * center than we'd like. The square above it is safer, because the bottom
     * or top edge of squares far away from the center might not be in the 
     * frame.
     * 
     * ### Solution
     * 
     * To get around this problem, a maximum vertical range length is defined, 
     * so the algorithm ignores near-vertical lines, and finds only horizontal
     * and diagonal lines. If it does find a vertical line, it just moves 
     * sideways, to a different column, where it will find a horizontal line.
     * 
     * @param   x
     *          The x-coordinate of the column to search in.
     * @return  
     *          Returns no result if the vertical range of white pixels is too
     *          long, which would indicate a vertical line.  
     *          Also returns no result if #getFirstLineEstimate fails, or if no
     *          white pixels are found in the given column.
     */
    optional<FirstLineEstimate> getFirstLineEstimate(uint x) const {
        if (x >= W)
            throw std::out_of_range("x out of range");

        // Iterate like this: `... 6  4  2  0  1  3  5 ...` to find the pixels
        // close to the center first.
        CenterPointOutLineIterator c = {H};

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
            if (last_white - first_white >= MAXIMUM_VERTICAL_START_LINE_WIDTH)
                return std::nullopt;
            else
                return getFirstLineEstimate({x, y});
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
            if (first_white - last_white >= MAXIMUM_VERTICAL_START_LINE_WIDTH)
                return std::nullopt;
            else
                return getFirstLineEstimate({x, y});

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
            if (last_white - first_white >= MAXIMUM_VERTICAL_START_LINE_WIDTH)
                return std::nullopt;
            else
                return getFirstLineEstimate({x, y});
        }
    }

    /// If no valid first line is found in a column, don't try the column right
    /// next to it, but go a little farther.
    constexpr static uint FIRST_LINE_INVALID_HORIZONTAL_JUMP = 10;

    /**
     * @brief   Get an estimate of the first line.
     *          Searches in multiple columns, starting from the center, taking
     *          jumps of #FIRST_LINE_INVALID_HORIZONTAL_JUMP pixels if no valid
     *          line is found in the currenc column.
     * 
     * @return  Returns no result if no valid lines are found across the entire
     *          frame.
     */
    optional<FirstLineEstimate> getFirstLineEstimate() const {
        CenterPointOutLineIterator c = {W / FIRST_LINE_INVALID_HORIZONTAL_JUMP};
        while (c.hasNext()) {
            uint x = c.next() * FIRST_LINE_INVALID_HORIZONTAL_JUMP;
            if (auto fle = getFirstLineEstimate(x); fle.has_value())
                return fle;
        }
        return std::nullopt;
    }

    /**
     * @brief   Get the first two lines. These are the two half lines that make
     *          up the line closest to the center of the frame.
     * 
     * We use two lines here instead of one, because their slopes don't differ
     * by exactly 180°, because of lense distorion.  
     * This makes our intersection a bit more accurate, and it's not too 
     * expensive.
     * 
     * @return
     *          Returns no result if @c getFirstLineEstimate fails.
     */
    std::array<optional<LineResult>, 2> getFirstTwoHalfLines() {
        optional<FirstLineEstimate> start = getFirstLineEstimate();
        if (!start.has_value())
            return {std::nullopt, std::nullopt};

        constexpr uint range = angle_t::resolution() / 40;  // 2 * 9°
        HoughResult result1  = findLineAngleAccurateRange<range>(
            start->middle, start->angleEstimate);
        HoughResult result2 = findLineAngleAccurateRange<range>(
            start->middle, start->angleEstimate.opposite());
        return {{
            LineResult{start->middle, start->width, result1.angle},
            LineResult{start->middle, start->width, result2.angle},
        }};
    }

#pragma endregion

#pragma region Finding the next perpendicular line..............................

    /// Perpendicular lines have to be long enough to be counted as a real line.
    constexpr static uint MINIMIM_LINE_WEIGHTED_VOTE_COUNT = (W + H) / 10;

    /**
     * @brief   Follow the given line to find the line perpendicular to it.
     *          Move along the line for @p minDistance pixels, then move away
     *          from the line, perpendicular to it for twice its width plus the
     *          given offset. Then move from this point parallel to the original
     *          line, and search for the next white pixel, which is probably 
     *          part of the line perpendicular to the given line.
     * 
     * ```
     *     minDistance
     *     /   \
     *    +-----s--->  search path
     *    |            2×width + offset
     * ---o--------->  line
     * ```
     * 
     * @param   line
     *          The line to follow.
     * @param   direction
     *          Should 90° be added or subtracted to get the perpendicular
     *          direction?     
     * @param   minDistance 
     *          The minimum distance between the given center point of the line
     *          and the perpendicular line. Essentially moves the starting point
     *          of the search along the line for this distance.
     * @param   offset
     *          The offset of the search path, measured perpendicularly from the
     *          line.
     * @return  // TODO
     */
    optional<LineResult> findNextLine(LineResult line, bool direction,
                                      uint minDistance = 0,
                                      uint offset      = 0) const {
        angle_t angle = line.angle;
        angle_t perp  = angle.perpendicular(direction);

        Pixel searchStart = line.lineCenter;
        searchStart       = move(searchStart, perp, 2 * line.width + offset);
        if (minDistance)
            searchStart = move(searchStart, line.angle, minDistance);

        uint minWidth = line.width / 3;

        BresenhamLine path = {searchStart, angle, W, H};
        Pixel pixel;
        do {
            // Follow the search path
            pixel = path.next();
            // Find the first white pixel
            while (path.hasNext() && get(pixel) == 0x00)
                pixel = path.next();
            Pixel firstWhite = pixel;
            // Find the first black pixel
            while (path.hasNext() && get(pixel) != 0x00)
                pixel = path.next();
            Pixel firstBlack = pixel;  // TODO: OBOE, but I don't really care
            // In the center of this range of white pixels, try to find the
            // perpendicular line
            Pixel middle = Pixel::average(firstBlack, firstWhite);
            optional<LineResult> possibleLine =
                checkLine(middle, perp, minWidth);
            // If a line has been found, return it
            if (possibleLine)
                return possibleLine;
            // Otherwise, keep looking along the search path
        } while (path.hasNext());
        // If we reach the end of the frame without finding a perpendicular line
        // return no result, so the calling function can call again with a
        // different offset (or give up)
        return std::nullopt;
    }

    /**
     * @brief   Equivalent to #findNextLine(LineResult,bool,uint,uint)
     *          but also supports optional #LineResult%s.
     */
    optional<LineResult> findNextLine(optional<LineResult> line, bool direction,
                                      uint minDistance = 0,
                                      uint offset      = 0) const {
        return line.has_value()
                   ? findNextLine(*line, direction, minDistance, offset)
                   : std::nullopt;
    }

    /**
     * @brief   Check if there could be a valid line going through the given
     *          point, and if so, find and return the actual line.
     * 
     * @param   pixel
     *          Find a line that goes through this point. 
     * @param   angle 
     *          An estimate of the angle of the line.
     * @param   minWidth 
     *          Only accept a line that's wider than this minimum width.
     * @return  // TODO
     */
    optional<LineResult> checkLine(Pixel pixel, angle_t angle,
                                   uint minWidth) const {
        optional<GetMiddleResult> middle = getMiddle(pixel, angle);
        if (!middle.has_value() || middle->width <= minWidth)
            return std::nullopt;

        constexpr uint range = angle_t::resolution() / 40;  // 2 * 9°
        HoughResult result =
            findLineAngleAccurateRange<range>(middle->pixel, angle);

        if (result.count < MINIMIM_LINE_WEIGHTED_VOTE_COUNT)
            return std::nullopt;
        return LineResult{middle->pixel, middle->width, result.angle};
    }

#pragma endregion

#pragma region Finding the center square(main function).........................

    /**
     * @brief   This is the main function: it looks for a complete square of the
     *          grid, as close to the center of the frame as possible.
     * 
     * @image   html square.png
     * 
     * @param   initialTries (default: 1)
     *          When finding the first two corners, this many Bresenham lines will
     *          run parallel to the initial line in order to prevent a bad result
     *          because of a small gap in the line.
     * @param   initialTriesFactor (default: 2.0)
     *          When finding the first two corners, the parallel Bresenham lines
     *          be separated by this factor multiplied by the initial line width.
     * 
     * @return  // TODO 
     */
    Square findSquare(int initialTries = 1, float initialTriesFactor = 2.0f) {
        static_assert(initialTries >= 1);
        static_assert(initialTries > 0.0f);
        Square sq = {};

        try {
            // Get the line closest to the center of the frame.
            // the result is an array of two half-lines, approximately opposite
            // of each other
            auto firstLines = getFirstTwoHalfLines();
            sq.lines[0]     = firstLines[0];
            sq.lines[1]     = firstLines[1];

            // Return empty square if we couldn't find the first line.
            if(!sq.lines[0].has_value())
                return sq;

            // Determine the direction to turn in (+90° or -90°)
            // Check if the first line lies left or right of the center of the frame
            // and pick the direction that will result in the square containing the
            // center point (if possible)
            bool direction = false;
            Line mathLine = {sq.lines[0]->lineCenter, sq.lines[0]->angle};
            direction     = mathLine.leftOfPoint(center());

            // Try finding the first two corners multiple times. We'll remember the
            // ones closest to the frame center, so if there's a hole in the image's
            // line, one of the Bresenham lines will still likely find the correct
            // perpendicular line.
            bool firstCornerFound  = false;
            bool secondCornerFound = false;
            float dist1, dist2, currentDistance;
            Point initialPoint = sq.lines[0].lineCenter;
            Point temp;
            for (uint i = 0; i < initialTries; i++) {
                // Second & third line...
                uint jump1   = std::round(initialTriesFactor * sq.lines[2].width);
                uint jump2   = std::round(initialTriesFactor * sq.lines[3].width);
                sq.lines[2] = findNextLine(sq.lines[0], direction, , jump1);
                sq.lines[3] = findNextLine(sq.lines[1], !direction, , jump2);

                // First corner: remember closest point to the frame center.
                if (sq.lines[2].has_value()) {
                    temp            = intersect(*sq.lines[0], *sq.lines[2]);
                    currentDistance = Point::distsq(initialPoint, temp);
                    if (!firstCornerFound || currentDistance < dist1) {
                        firstCornerFound = true;
                        dist1            = currentDistance;
                        sq.points[0]     = temp;
                    }
                }

                // First corner: remember closest point to the frame center.
                if (sq.lines[3].has_value()) {
                    temp            = intersect(*sq.lines[1], *sq.lines[3]);
                    currentDistance = Point::distsq(initialPoint, temp);
                    if (!secondCornerFound || currentDistance < dist2) {
                        secondCornerFound = true;
                        dist2             = currentDistance;
                        sq.points[1]      = temp;
                    }
                }
            }

            // If we found all lines so far (first two, second, and third)
            if (firstCornerFound && secondCornerFound) {

                // Calculate the distance between the first two points
                // (note that this is not the Euclidian distance, but the
                // number of pixels that would be drawn by Bresenham)
                uint minDistance = std::floor(
                    std::max(std::abs(sq.points[0]->x - sq.points[1]->x),
                             std::abs(sq.points[0]->y - sq.points[1]->y)));
                // Use 3/4 of this distance as a minimum distance for where to
                // start looking for the fourth line, as it's very likely that
                // the other sides of the square are roughly the same length as
                // the first side
                minDistance -= minDistance / 4;
                uint offset          = 0;
                const uint maxOffset = minDistance / 2;
                const uint offsetIncr =
                    std::max(sq.lines[2]->width, sq.lines[3]->width);

                // Search for the fourth line
                while (!sq.lines[4].has_value() && offset < maxOffset) {
                    sq.lines[4] =  // find the fourth line along the second
                        findNextLine(sq.lines[2], direction, minDistance,
                                     offset);
                    if (!sq.lines[4].has_value())  // if not found along second
                        sq.lines[4] =  // find the fourth line along the third
                            findNextLine(sq.lines[3], !direction, minDistance,
                                         offset);
                    // next time, try again with a different offset
                    offset += offsetIncr;
                }
                // If we found a fourth line, calculate the intersections
                if (sq.lines[4].has_value()) {
                    sq.points[2] = intersect(*sq.lines[2], *sq.lines[4]);
                    sq.points[3] = intersect(*sq.lines[3], *sq.lines[4]);
                }
            } else if (sq.lines[2].has_value()) {
                // TODO
            } else if (sq.lines[3].has_value()) {
                // TODO
            } else {
                // TODO
            }
        } catch (std::exception &e) {
            cerr << ANSIColors::redb << e.what() << ANSIColors::reset << endl;
        }
        return sq;
    }

#pragma endregion

  private:
    /// The matrix containing the mask values.
    Img_t mask;
};
