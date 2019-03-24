#pragma once

#include <uint.hpp>  // uint

class CenterPointOutLineIterator {
  public:
    CenterPointOutLineIterator(uint_t length)
        : length(length), center((length - 1) / 2), x(center) {}

    uint_t next() {
        uint_t result = x;
        x = 2 * center - x;
        if (result <= center)
            ++x;
        return result;
    }

    bool hasNext() const { return x < length; }

    uint_t getCenter() const {
        return center;
    }

  private:
    const uint_t length;
    const uint_t center;
    uint_t x;
};