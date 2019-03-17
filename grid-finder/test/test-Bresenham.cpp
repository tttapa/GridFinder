#include <gtest/gtest.h>
#include <Bresenham.hpp>
#include <cmath>

using namespace std;

TEST(BresenhamLine, lowPosXPosY) {
    BresenhamLine line = {{4, 4}, atan2(1, 2), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {5, 4},
        {6, 5},
        {7, 5}, 
        {8, 6},
    };

    ASSERT_EQ(result, expect);
}

TEST(BresenhamLine, highPosXPosY) {
    BresenhamLine line = {{4, 4}, atan2(2, 1), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {4, 5},
        {5, 6},
        {5, 7}, 
        {6, 8},
    };

    ASSERT_EQ(result, expect);
}

TEST(BresenhamLine, NegXPosY) {
    BresenhamLine line = {{4, 4}, atan2(2, -1), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {4, 5},
        {3, 6},
        {3, 7}, 
        {2, 8},
    };

    ASSERT_EQ(result, expect);
}

TEST(BresenhamLine, lowNegXPosY) {
    BresenhamLine line = {{4, 4}, atan2(1, -2), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {3, 4},
        {2, 5},
        {1, 5}, 
        {0, 6},
    };

    ASSERT_EQ(result, expect);
}

TEST(BresenhamLine, lowNegXNegY) {
    BresenhamLine line = {{4, 4}, atan2(-1, -2), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {3, 4},
        {2, 3},
        {1, 3}, 
        {0, 2},
    };

    ASSERT_EQ(result, expect);
}

TEST(BresenhamLine, highNegXNegY) {
    BresenhamLine line = {{4, 4}, atan2(-2, -1), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {4, 3},
        {3, 2},
        {3, 1}, 
        {2, 0},
    };

    ASSERT_EQ(result, expect);
}

TEST(BresenhamLine, highPosXNegY) {
    BresenhamLine line = {{4, 4}, atan2(-2, 1), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {4, 3},
        {5, 2},
        {5, 1}, 
        {6, 0},
    };

    ASSERT_EQ(result, expect);
}

TEST(BresenhamLine, lowPosXNegY) {
    BresenhamLine line = {{4, 4}, atan2(-1, 2), 9, 9};
    vector<Pixel> result;

    while (line.hasNext()) {
        Pixel px = line.next();
        result.push_back(px);
        cout << px << endl;
    }

    vector<Pixel> expect = {
        {4, 4},
        {5, 4},
        {6, 3},
        {7, 3}, 
        {8, 2},
    };

    ASSERT_EQ(result, expect);
}