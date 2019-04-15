#include <Angle.hpp>
#include <Degrees.hpp>
#include <gtest/gtest.h>
#include <limits>

TEST(Angle, sincos) {
    angle_t angle = 60_deg;
    EXPECT_EQ(angle.cos(),
              static_cast<int>(std::round(
                  std::cos(M_PI / 3) * (std::numeric_limits<int>::max() / 2))));
    EXPECT_EQ(angle.sin(),
              static_cast<int>(std::round(
                  std::sin(M_PI / 3) * (std::numeric_limits<int>::max() / 2))));
}

TEST(Angle, average) {
    double eps = 2 * M_PI * std::numeric_limits<double>::epsilon();
    EXPECT_NEAR(angle_t::average(350_deg, 10_deg).rad(), 0_deg, eps);
    EXPECT_NEAR(angle_t::average(10_deg, 350_deg).rad(), 180_deg, eps);
    EXPECT_NEAR(angle_t::average(270_deg, 90_deg).rad(), 0_deg, eps);
    EXPECT_NEAR(angle_t::average(181_deg, 179_deg).rad(), 0_deg, eps);
    EXPECT_NEAR(angle_t::average(179_deg, 181_deg).rad(), 180_deg, eps);
    EXPECT_NEAR(angle_t::average(45_deg, 135_deg).rad(), 90_deg, eps);
    EXPECT_NEAR(angle_t::average(135_deg, 45_deg).rad(), 270_deg, eps);
    EXPECT_NEAR(angle_t::average(357_deg, 359_deg).rad(), 358_deg, eps);
    EXPECT_NEAR(angle_t::average(359_deg, 357_deg).rad(), 178_deg, eps);
    EXPECT_NEAR(angle_t::average(0_deg, 2_deg).rad(), 1_deg, eps);
    EXPECT_NEAR(angle_t::average(2_deg, 0_deg).rad(), 181_deg, eps);
    EXPECT_NEAR(angle_t::average(0_deg, 0_deg).rad(), 0_deg, eps);
    EXPECT_NEAR(angle_t::average(359_deg, 359_deg).rad(), 359_deg, eps);
    EXPECT_NEAR(angle_t::average(180_deg, 180_deg).rad(), 180_deg, eps);
    EXPECT_NEAR(angle_t::average(269_deg, 271_deg).rad(), 270_deg, eps);
    EXPECT_NEAR(angle_t::average(181_deg, 179_deg).rad(), 0_deg, eps);
}