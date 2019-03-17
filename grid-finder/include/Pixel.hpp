#pragma once

#include <ostream>
#include <cstddef>

struct Pixel {
    Pixel(int x, int y) : x(x), y(y) {}
    int x, y;
    bool isValid() const { return x >= 0 && y >= 0; }
    bool operator==(Pixel other) const {
        return this->x == other.x && this->y == other.y;
    }
    bool inRange(size_t w, size_t h) const {
        return (size_t) x < w && (size_t) y < h;
    }
};

inline std::ostream &operator<<(std::ostream &os, Pixel p) {
    return os << '(' << p.x << ", " << p.y << ')';
}