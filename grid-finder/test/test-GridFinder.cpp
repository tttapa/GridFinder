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
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = M_PI / 7;
    BresenhamLine line = {center, angle, W, H};
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
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = M_PI / 7;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_NEAR(result, expect, 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccurateZero) {
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = 0;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_NEAR(result, expect, 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccuratePI) {
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = M_PI;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_NEAR(result, expect, 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccuratePI2) {
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = M_PI_2;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_NEAR(result, expect, 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccurateMinusPI2) {
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = 2 * M_PI - M_PI_2;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_NEAR(result, expect, 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccuratePI4) {
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = M_PI_4;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_NEAR(result, expect, 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, findLineAngleAccurateRandomAngle) {
    constexpr size_t W = 410;
    constexpr size_t H = 308;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = 23.0 / 17;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    double result = gm.findLineAngleAccurate(center);
    double expect = angle;

    EXPECT_NEAR(result, expect, 2 * M_PI / gm.HOUGH_ANGLE_RESOLUTION);
}

TEST(GridMask, getWidth) {
    GridMask<9, 7> gm = {{{
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 0, 0, 1, 0, 1, 0, 1},
        {1, 0, 0, 1, 1, 0, 1, 1, 1},
        {1, 0, 0, 1, 1, 0, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
    }}};

    int cos     = BresenhamLine::cos(0);
    int sin     = BresenhamLine::sin(0);
    Pixel pixel = {0, 3};

    GridMask<9, 7> gm_draw = {};
    BresenhamLine l        = {pixel, cos, sin, 9, 7};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    size_t result = gm.getWidthAtPointOnLine(pixel, cos, sin);
    size_t expect = 3;
    // (7, 5) has a width of 3, starting from the base line through (7, 3)

    EXPECT_EQ(result, expect);
}

#include <Degrees.hpp>

TEST(GridMask, averageAngle) {
    using GM   = GridMask<1, 1>;
    double eps = 2 * M_PI * numeric_limits<double>::epsilon();
    EXPECT_NEAR(GM::averageAngle(350_deg, 10_deg), 0_deg, eps);
    EXPECT_NEAR(GM::averageAngle(10_deg, 350_deg), 180_deg, eps);
    EXPECT_NEAR(GM::averageAngle(270_deg, 90_deg), 0_deg, eps);
    EXPECT_NEAR(GM::averageAngle(181_deg, 179_deg), 0_deg, eps);
    EXPECT_NEAR(GM::averageAngle(179_deg, 181_deg), 180_deg, eps);
    EXPECT_NEAR(GM::averageAngle(45_deg, 135_deg), 90_deg, eps);
    EXPECT_NEAR(GM::averageAngle(135_deg, 45_deg), 270_deg, eps);
    EXPECT_NEAR(GM::averageAngle(357_deg, 359_deg), 358_deg, eps);
    EXPECT_NEAR(GM::averageAngle(359_deg, 357_deg), 178_deg, eps);
    EXPECT_NEAR(GM::averageAngle(0_deg, 2_deg), 1_deg, eps);
    EXPECT_NEAR(GM::averageAngle(2_deg, 0_deg), 181_deg, eps);
    EXPECT_NEAR(GM::averageAngle(0_deg, 0_deg), 0_deg, eps);
    EXPECT_NEAR(GM::averageAngle(359_deg, 359_deg), 359_deg, eps);
    EXPECT_NEAR(GM::averageAngle(180_deg, 180_deg), 180_deg, eps);
    EXPECT_NEAR(GM::averageAngle(269_deg, 271_deg), 270_deg, eps);
    EXPECT_NEAR(GM::averageAngle(181_deg, 179_deg), 0_deg, eps);
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