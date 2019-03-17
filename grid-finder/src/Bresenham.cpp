#include <Bresenham.hpp>
#include <cmath>

BresenhamLine::BresenhamLine(Pixel start, double angle, size_t w, size_t h)
    : px(start), w(w), h(h) {
    dx    = std::round(len * std::cos(angle));
    dy    = std::round(len * std::sin(angle));
    adx   = std::abs(dx);
    ady   = std::abs(dy);
    steep = ady > adx;
    error = -len / 2;
    xinc  = dx > 0 ? 1 : -1;
    yinc  = dy > 0 ? 1 : -1;
}

bool BresenhamLine::hasNext() const { return px.inRange(w, h); }

Pixel BresenhamLine::next() {
    if (!hasNext())
        throw std::out_of_range("No more pixels on line withing frame!");
    Pixel result = px;
    if (steep) {
        if (error >= 0) {
            px.x += xinc;
            error -= 2 * ady;
        }
        px.y += yinc;
        error += 2 * adx;
    } else {
        if (error >= 0) {
            px.y += yinc;
            error -= 2 * adx;
        }
        px.x += xinc;
        error += 2 * ady;
    }
    return result;
}
