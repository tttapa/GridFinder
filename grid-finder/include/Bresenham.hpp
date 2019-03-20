#pragma once

#include <Pixel.hpp>
#include <cmath>
#include <limits>

class BresenhamLine {
  public:
    BresenhamLine(Pixel start, double angle, size_t w, size_t h);
    BresenhamLine(Pixel start, int cos, int sin, size_t w, size_t h);

    bool hasNext() const;
    Pixel next();

    static int cos(double angle) {
        return round(errorScalingFactor * std::cos(angle));
    }

    static int sin(double angle) {
        return round(errorScalingFactor * std::sin(angle));
    }

    size_t getCurrentLength() const;

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
};
