#pragma once

#include <cstddef>  // size_t

class CenterPointOutLineIterator {
  public:
    CenterPointOutLineIterator(size_t height)
        : height(height), center((height - 1) / 2), x(center) {}

    size_t next() {
        size_t result = x;
        x = 2 * center - x;
        if (result <= center)
            ++x;
        return result;
    }

    bool hasNext() const { return x < height; }

    size_t getCenter() const {
        return center;
    }

  private:
    const size_t height;
    const size_t center;
    size_t x;
};