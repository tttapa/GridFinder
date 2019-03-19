#include <GridFinder.hpp>
#include <gtest/gtest.h>

#include <fstream>

using namespace std;

TEST(GridMask, getStartingPointFullLine) {
    GridMask<4, 9> gm = {{{
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
    }}};

    auto result = gm.getStartingPoint(2);
    ASSERT_EQ(result.pixel, Pixel(2, 4));
    ASSERT_EQ(result.width, 9);
}

TEST(GridMask, getStartingPointBottom) {
    GridMask<4, 9> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
    }}};

    auto result = gm.getStartingPoint(2);
    ASSERT_EQ(result.pixel, Pixel(2, 6));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, true);
}

TEST(GridMask, getStartingPointBottomBorder) {
    GridMask<4, 9> gm = {{{
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
    }}};

    auto result = gm.getStartingPoint(2);
    ASSERT_EQ(result.pixel, Pixel(2, 7));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, true);
}

TEST(GridMask, getStartingPointTop) {
    GridMask<4, 9> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
    }}};

    auto result = gm.getStartingPoint(2);
    ASSERT_EQ(result.pixel, Pixel(2, 2));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, false);
}

TEST(GridMask, getStartingPointTopBorder) {
    GridMask<4, 9> gm = {{{
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
    }}};

    auto result = gm.getStartingPoint(2);
    ASSERT_EQ(result.pixel, Pixel(2, 1));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, false);
}

TEST(GridMask, getStartingPointOtherColumnEvenLast) {
    GridMask<4, 6> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
    }}};

    auto result = gm.getStartingPoint();
    ASSERT_EQ(result.pixel, Pixel(3, 4));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, true);
}

TEST(GridMask, getStartingPointOtherColumnEvenFirst) {
    GridMask<4, 6> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
    }}};

    auto result = gm.getStartingPoint();
    ASSERT_EQ(result.pixel, Pixel(0, 4));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, true);
}

TEST(GridMask, getStartingPointOtherColumnOddFirst) {
    GridMask<5, 6> gm = {{{
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
    }}};

    auto result = gm.getStartingPoint();
    ASSERT_EQ(result.pixel, Pixel(0, 4));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, true);
}

TEST(GridMask, getStartingPointOtherColumnOddLast) {
    GridMask<5, 6> gm = {{{
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
    }}};

    auto result = gm.getStartingPoint();
    ASSERT_EQ(result.pixel, Pixel(4, 4));
    ASSERT_EQ(result.width, 3);
    ASSERT_EQ(result.above, true);
}

TEST(GridMask, getStartingPointNoWhitePixels) {
    GridMask<5, 6> gm = {{{
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    }}};

    try {
        gm.getStartingPoint();
        FAIL();
    } catch (std::runtime_error &e) {
    }
}

TEST(GridMask, Hough) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = M_PI / 7;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double resolution = 1e3;
    vector<HoughResult> houghRes;
    houghRes.reserve(resolution);
    double step = 2 * M_PI / resolution;
    for (double angle = 0; angle < 2 * M_PI; angle += step)
        houghRes.emplace_back(gm.hough(center, angle));

    double result = max_element(houghRes.begin(), houghRes.end())->angle;
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= step);
}

TEST(GridMask, findLineAngleAccurate) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = M_PI / 7;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccurateZero) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = 0;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccuratePI) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = M_PI;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccuratePI2) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = M_PI_2;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccurateMinusPI2) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = 2 * M_PI - M_PI_2;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccuratePI4) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = M_PI_4;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccurateRandomAngle) {
    GridMask<410, 308> gm = {};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = 23.0 / 17;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

#include <CenterPointOutLineIterator.hpp>

TEST(CenterPointOutLineIterator, CenterPointOutLineIteratorOdd) {
    CenterPointOutLineIterator c(7);
    vector<size_t> result;
    while (c.hasNext())
        result.push_back(c.next());
    vector<size_t> expect = {3, 4, 2, 5, 1, 6, 0};
    ASSERT_EQ(result, expect);
}

TEST(CenterPointOutLineIterator, CenterPointOutLineIteratorEven) {
    CenterPointOutLineIterator c(6);
    vector<size_t> result;
    while (c.hasNext())
        result.push_back(c.next());
    vector<size_t> expect = {2, 3, 1, 4, 0, 5};
    ASSERT_EQ(result, expect);
}