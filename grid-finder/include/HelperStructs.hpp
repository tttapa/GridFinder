#include <Angle.hpp>
#include <Line.hpp>
#include <Pixel.hpp>
#include <optional>

#pragma region Structs..........................................................

/**
 * @brief   Struct as a result of the hough function. It gives the most 
 *          plausible angle, and a vote count that's an indication of how many
 *          white pixels lie on that line (may be weighted and truncated).
 * @see     GridFinder::hough
 */
struct HoughResult {
    angle_t angle;
    uint count;
    bool operator<(HoughResult other) const { return count < other.count; }
};

struct LineResult {
    Pixel lineCenter;
    uint width;
    angle_t angle;
};

struct FirstLineEstimate {
    Pixel middle;
    uint width;
    angle_t angleEstimate;
};

struct GetMiddleResult {
    Pixel pixel;
    uint width;
};

struct Square {
    std::array<std::optional<LineResult>, 5> lines;
    std::array<std::optional<Point>, 4> points;
};

#pragma endregion
#pragma region Printing.........................................................

/**
 * @brief   Function for printing a HoughResult.
 * @see     HoughResult
 */
inline std::ostream &operator<<(std::ostream &os, HoughResult h) {
    return os << h.angle << " rad: " << h.count;
}

inline std::ostream &operator<<(std::ostream &os, LineResult l) {
    return os << "LineResult(" << l.lineCenter << ", " << l.width << ", "
              << l.angle << ")";
}

#pragma endregion