#pragma once

#include <cstddef>  // size_t

class CenterPointOutLineIterator {
  public:
    CenterPointOutLineIterator(size_t length)
        : length(length), center((length - 1) / 2), x(center) {}

    size_t next() {
        size_t result = x;
        x = 2 * center - x;
        if (result <= center)
            ++x;
        return result;
    }

    bool hasNext() const { return x < length; }

    size_t getCenter() const {
        return center;
    }

  private:
    const size_t length;
    const size_t center;
    size_t x;
};