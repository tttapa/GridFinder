#pragma once

#include <Angle.hpp>
#include <Pixel.hpp>
#include <cmath>
#include <limits>

class BresenhamLine {
  public:
    BresenhamLine(Pixel start, int cos, int sin, size_t w, size_t h)
        : px0(start), px(start),  // Starting point
          dx(cos),
          dy(sin),  // cosine and sine of the slope of the line (scaled)
          adx(std::abs(dx)),
          ady(std::abs(dy)),             // absolute values of cos and sin
          xinc(sgn(dx)), yinc(sgn(dy)),  // increment steps for x and y
          steep(ady > adx),  // whether to increment x or y on each iteration
          w(w), h(h) {       // size of the canvas
        if (steep)
            error = (adx - ady) / 2;
        else
            error = (ady - adx) / 2;
    }

    BresenhamLine(Pixel start, CosSin angle, size_t w, size_t h)
        : BresenhamLine(start, angle.cos, angle.sin, w, h) {}

    BresenhamLine(Pixel start, double angle, size_t w, size_t h)
        : BresenhamLine(start, cos(angle), sin(angle), w, h) {}

    bool hasNext() const { return px.inRange(w, h); }

    size_t getCurrentLength() const { return length; }

    Pixel next() {
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

    static int cos(double angle) {
        return round(errorScalingFactor * std::cos(angle));
    }

    static int sin(double angle) {
        return round(errorScalingFactor * std::sin(angle));
    }

  private:
    const static int errorScalingFactor = std::numeric_limits<int>::max() / 2;
    const Pixel px0;
    Pixel px;
    int dx, dy;
    int adx, ady;
    int xinc, yinc;
    bool steep;
    size_t w, h;
    int error;
    size_t length = 0;

    template <typename T>
    static constexpr int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }
};
