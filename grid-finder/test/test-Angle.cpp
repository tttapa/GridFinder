#include <Angle.hpp>
#include <gtest/gtest.h>

TEST(Angle, sincos) {
    angle_t angle = 60;
    EXPECT_EQ(angle.cos(),
              static_cast<int>(std::round(
                  std::cos(M_PI / 3) * (std::numeric_limits<int>::max() / 2))));
    EXPECT_EQ(angle.sin(),
              static_cast<int>(std::round(
                  std::sin(M_PI / 3) * (std::numeric_limits<int>::max() / 2))));
}