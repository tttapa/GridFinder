#pragma once

#include <cstddef>
#include <ostream>

struct Pixel {
    constexpr Pixel(size_t x, size_t y) : x(x), y(y) {}
    constexpr Pixel() : x(-1), y(-1) {}
    size_t x, y;
    bool isValid() const { return x != (size_t) -1 && y != (size_t) -1; }
    bool operator==(Pixel other) const {
        return this->x == other.x && this->y == other.y;
    }
    bool operator!=(Pixel other) const {
        return this->x != other.x || this->y != other.y;
    }
    bool inRange(size_t w, size_t h) const {
        // Unsigned comparison also checks for >= 0
        return x < w && y < h;
    }
    static Pixel average(Pixel a, Pixel b) {
        return {(a.x + b.x) / 2, (a.y + b.y) / 2};
    }
};

inline std::ostream &operator<<(std::ostream &os, Pixel p) {
    return os << '(' << p.x << ", " << p.y << ')';
}