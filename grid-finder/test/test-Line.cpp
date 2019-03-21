#include <Line.hpp>
#include <gtest/gtest.h>

TEST(Line, intersect) {
    Line a       = {{3, 7, -5}};
    Line b       = {{2, -11, 13}};
    Point result = Line::intersect(a, b);
    Point expect = {-36.0 / 47.0, 49.0 / 47.0};
    EXPECT_EQ(result, expect);
}