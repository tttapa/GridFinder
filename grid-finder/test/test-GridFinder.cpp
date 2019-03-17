#include <GridFinder.hpp>
#include <gtest/gtest.h>

using namespace std;

TEST(GridMask, getStartingPixel) {
    GridMask<4, 6> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    }}};

    Pixel result = gm.getStartingPixel(2);
    Pixel expect = {2, 2};
    ASSERT_EQ(result, expect);
}

TEST(GridMask, getStartingPixelBottom) {
    GridMask<4, 6> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
    }}};

    Pixel result = gm.getStartingPixel(2);
    Pixel expect = {2, 4};
    ASSERT_EQ(result, expect);
}

TEST(GridMask, getStartingPixelOtherColumnEvenLast) {
    GridMask<4, 6> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
    }}};

    Pixel result = gm.getStartingPixel();
    Pixel expect = {3, 4};
    ASSERT_EQ(result, expect);
}

TEST(GridMask, getStartingPixelOtherColumnEvenFirst) {
    GridMask<4, 6> gm = {{{
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
    }}};

    Pixel result = gm.getStartingPixel();
    Pixel expect = {0, 4};
    ASSERT_EQ(result, expect);
}

TEST(GridMask, getStartingPixelOtherColumnOddFirst) {
    GridMask<5, 6> gm = {{{
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
    }}};

    Pixel result = gm.getStartingPixel();
    Pixel expect = {0, 4};
    ASSERT_EQ(result, expect);
}

TEST(GridMask, getStartingPixelOtherColumnOddLast) {
    GridMask<5, 6> gm = {{{
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
        {0, 0, 0, 0, 1},
    }}};

    Pixel result = gm.getStartingPixel();
    Pixel expect = {4, 4};
    ASSERT_EQ(result, expect);
}

TEST(GridMask, getStartingPixelNoWhitePixels) {
    GridMask<5, 6> gm = {{{
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    }}};

    try {
        gm.getStartingPixel();
        FAIL();
    } catch (std::exception &e) {
    }
}

TEST(GridMask, Hough) {
    GridMask<410, 308> gm = {{}};
    Pixel center          = {410 / 2, 308 / 2};
    double angle          = M_PI / 17;
    BresenhamLine line    = {center, angle, 410, 308};
    while (line.hasNext())
        gm.set(line.next());

    vector<HoughResult> houghResults;
    double resolution = 1e4;
    double step       = M_2_PI / resolution;
    for (double angle = 0; angle < 2 * M_PI; angle += step)
        houghResults.push_back(gm.hough(center, angle));

    double result =
        std::max_element(houghResults.begin(), houghResults.end())->angle;
    double expect = angle;

    EXPECT_TRUE(abs(result - expect) <= step);
}