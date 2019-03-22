#include <Degrees.hpp>
#include <GridFinder.hpp>
#include <gtest/gtest.h>

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

    double result = max_element(houghRes.begin(), houghRes.end())->angle.rad();
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

    double result = gm.findLineAngleAccurate(center).angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
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

    double result = gm.findLineAngleAccurate(center).angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
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

    double result = gm.findLineAngleAccurate(center).angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
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

    double result = gm.findLineAngleAccurate(center).angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
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

    double result = gm.findLineAngleAccurate(center).angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
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

    double result = gm.findLineAngleAccurate(center).angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
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

    double result = gm.findLineAngleAccurate(center).angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
}

TEST(GridMask, findLineAngleAccurateRange110) {
    constexpr size_t W = 160;
    constexpr size_t H = 90;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = 110_deg;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    // gm.print(cout);

    double result =
        gm.findLineAngleAccurateRange<angle_t::resolution() / 8>(center, angle)
            .angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
}

TEST(GridMask, findLineAngleAccurateRange20) {
    constexpr size_t W = 160;
    constexpr size_t H = 90;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = 20_deg;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    // gm.print(cout);

    double result =
        gm.findLineAngleAccurateRange<angle_t::resolution() / 8>(center, angle)
            .angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
}

TEST(GridMask, findLineAngleAccurateRange340) {
    constexpr size_t W = 160;
    constexpr size_t H = 90;
    GridMask<W, H> gm  = {};
    Pixel center       = {W / 2, H / 2};
    double angle       = 340_deg;
    BresenhamLine line = {center, angle, W, H};
    while (line.hasNext())
        gm.set(line.next());

    // gm.print(cout);

    double result =
        gm.findLineAngleAccurateRange<angle_t::resolution() / 8>(center, angle)
            .angle.rad();
    double expect = angle;

    EXPECT_NEAR(result, expect, angle_t::step());
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

    CosSin angle = angle_t(0_deg);
    Pixel pixel  = {0, 3};

    GridMask<9, 7> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 7};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    size_t result = gm.getWidthAtPointOnLine(pixel, angle);
    size_t expect = 3;
    // (7, 5) has a width of 3, starting from the base line through (7, 3)

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getWidth45) {
    GridMask<9, 9> gm = {{{
        {1, 1, 1, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 0, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 1, 1, 1},
    }}};

    CosSin angle = angle_t(45_deg);

    Pixel pixel = {0, 0};

    GridMask<9, 9> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 9};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    size_t result = gm.getWidthAtPointOnLine(pixel, angle, 9);
    size_t expect = 3;
    // (7, 5) has a width of 3, starting from the base line through (7, 3)

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getWidth135) {
    GridMask<9, 9> gm = {{{
        {0, 0, 0, 0, 0, 0, 1, 1, 1},
        {0, 0, 0, 0, 0, 1, 1, 1, 1},
        {0, 0, 1, 0, 1, 1, 1, 1, 1},
        {0, 0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 1, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0, 0},
        {1, 1, 1, 1, 1, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 0, 0, 0, 0, 0, 0},
    }}};

    CosSin angle = angle_t(135_deg);

    Pixel pixel = {8, 0};

    GridMask<9, 9> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 9};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    size_t result = gm.getWidthAtPointOnLine(pixel, angle, 9);
    size_t expect = 3;
    // (7, 5) has a width of 3, starting from the base line through (7, 3)

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getWidth225) {
    GridMask<9, 9> gm = {{{
        {1, 1, 1, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 0, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 1, 1, 1},
    }}};

    CosSin angle = angle_t(225_deg);

    Pixel pixel = {8, 8};

    GridMask<9, 9> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 9};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    size_t result = gm.getWidthAtPointOnLine(pixel, angle, 9);
    size_t expect = 3;
    // (7, 5) has a width of 3, starting from the base line through (7, 3)

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getMiddleHorizontal) {
    GridMask<9, 9> gm = {{{
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 1},
        {0, 0, 1, 0, 1, 1, 1, 1, 0},
        {1, 1, 1, 0, 1, 1, 1, 0, 1},
        {1, 1, 1, 1, 1, 0, 1, 1, 1},
        {1, 1, 1, 1, 1, 0, 1, 1, 1},
        {0, 0, 1, 0, 1, 0, 1, 1, 1},
        {0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
    }}};

    CosSin angle = angle_t(0);

    Pixel pixel = {4, 2};

    GridMask<9, 9> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 9};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    Pixel result = gm.getMiddle(pixel, angle, 4).pixel;  // TODO: check width
    Pixel expect = {4, 4};
    // Row 4 is the center of the horizontal line with thickness 7

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getMiddleVertical) {
    GridMask<9, 9> gm = {{{
        {0, 1, 1, 1, 1, 1, 1, 1, 0},
        {1, 1, 1, 1, 1, 1, 1, 1, 0},
        {0, 1, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 1, 1, 1, 1, 1, 0},
        {0, 1, 1, 1, 1, 1, 1, 1, 0},
    }}};

    CosSin angle = angle_t(M_PI_2);

    Pixel pixel = {2, 4};

    GridMask<9, 9> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 9};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    Pixel result = gm.getMiddle(pixel, angle, 4).pixel;  // TODO: check width
    Pixel expect = {4, 4};
    // Row column 4 is the center of the horizontal line with thickness 7
    // The white pixel in (0, 1) is excluded by the small max_gap of 4

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getMiddleDiagonalAbove) {
    GridMask<9, 9> gm = {{{
        {1, 1, 1, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 0, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 0, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 1, 1, 1},
    }}};

    CosSin angle = angle_t(M_PI_4);

    Pixel pixel = {3, 5};

    GridMask<9, 9> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 9};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    Pixel result = gm.getMiddle(pixel, angle, 4).pixel;  // TODO: check width
    Pixel expect = {4, 4};

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getMiddleDiagonalBelow) {
    GridMask<9, 9> gm = {{{
        {1, 1, 1, 0, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1, 0, 1, 0, 0},
        {0, 1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 0},
        {0, 0, 1, 0, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 1, 1, 1},
    }}};

    CosSin angle = angle_t(M_PI_4);

    Pixel pixel = {5, 3};

    GridMask<9, 9> gm_draw = {};
    BresenhamLine l        = {pixel, angle, 9, 9};
    while (l.hasNext())
        gm_draw.set(l.next());
    gm_draw.print(cout) << endl;
    gm.print(cout);

    Pixel result = gm.getMiddle(pixel, angle, 4).pixel;  // TODO: check width
    Pixel expect = {4, 4};

    EXPECT_EQ(result, expect);
}

TEST(GridMask, getMiddleCross) {
    GridMask<80, 80> gm = {};

    CosSin angle              = angle_t(45_deg);
    CosSin perpendicularAngle = angle.perpendicular();

    for (size_t i = 0; i < 4; ++i) {
        gm.drawLine({0, i}, angle);
        gm.drawLine({i, 0}, angle);
        gm.drawLine({79, i}, perpendicularAngle);
        gm.drawLine({79 - i, 0}, perpendicularAngle);
    }

    gm.print(cout);

    Pixel pixel  = {40, 40};
    Pixel result = gm.getMiddle(pixel, angle, 4).pixel;  // TODO: check width

    EXPECT_FALSE(result.isValid());
}

/*
TEST(GridMask, getFirstLines) {
    GridMask<40 + 19, 19> gm = {};

    angle_t angle = 22.5_deg;

    for (size_t i = 0; i < 5; ++i) {
        gm.drawLine({0, i}, angle);
    }
    gm.print(cout);
    auto firstLine = gm.getFirstLines();

    GridMask<40 + 19, 19> gmp = {};
    gmp.drawLine(firstLine.lineCenter, firstLine.angle);
    gmp.print(cout);

    EXPECT_EQ(firstLine.lineCenter, Pixel(9, 15));
    EXPECT_EQ(firstLine.width, 5);
    EXPECT_EQ(firstLine.angle, angle);
}
*/