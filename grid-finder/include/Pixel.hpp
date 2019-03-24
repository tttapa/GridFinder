#pragma once

#include <uint.hpp>
#include <ostream>

struct Pixel {
    constexpr Pixel(uint x, uint y) : x(x), y(y) {}
    constexpr Pixel() : x(-1), y(-1) {}
    uint x, y;
    bool isValid() const { return x != (uint) -1 && y != (uint) -1; }
    bool operator==(Pixel other) const {
        return this->x == other.x && this->y == other.y;
    }
    bool operator!=(Pixel other) const {
        return this->x != other.x || this->y != other.y;
    }
    bool inRange(uint w, uint h) const {
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