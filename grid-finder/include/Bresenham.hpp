#pragma once

#include <Pixel.hpp>
#include <limits>

class BresenhamLine {
  public:
    BresenhamLine(Pixel start, double angle, size_t w, size_t h);
    bool hasNext() const;
    Pixel next();

  private:
    const static int len = std::numeric_limits<int>::max() / 2;
    Pixel px;
    size_t w, h;
    int dx, dy;
    int adx, ady;
    int xinc, yinc;
    int error;
    bool steep;
};
