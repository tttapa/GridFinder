#pragma once

#include <uint.hpp>  // uint

class CenterPointOutLineIterator {
  public:
    CenterPointOutLineIterator(uint length)
        : length(length), center((length - 1) / 2), x(center) {}

    uint next() {
        uint result = x;
        x = 2 * center - x;
        if (result <= center)
            ++x;
        return result;
    }

    bool hasNext() const { return x < length; }

    uint getCenter() const {
        return center;
    }

  private:
    const uint length;
    const uint center;
    uint x;
};