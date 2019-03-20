#include <Bresenham.hpp>

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

BresenhamLine::BresenhamLine(Pixel start, double angle, size_t w, size_t h)
    : BresenhamLine(start, cos(angle), sin(angle), w, h) {}

BresenhamLine::BresenhamLine(Pixel start, int cos, int sin, size_t w, size_t h)
    : px0(start), px(start),  // Starting point
      dx(cos), dy(sin),  // cosine and sine of the slope of the line (scaled)
      adx(std::abs(dx)), ady(std::abs(dy)),  // absolute values of cos and sin
      xinc(sgn(dx)), yinc(sgn(dy)),          // increment steps for x and y
      steep(ady > adx),  // whether to increment x or y on each iteration
      w(w), h(h)         // size of the canvas
{}

bool BresenhamLine::hasNext() const { return px.inRange(w, h); }

size_t BresenhamLine::getCurrentLength() const { return length; }

Pixel BresenhamLine::next() {
    if (!hasNext())
        throw std::out_of_range("No more pixels on line withing canvas!");
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
    ++length;
    return result;
}
