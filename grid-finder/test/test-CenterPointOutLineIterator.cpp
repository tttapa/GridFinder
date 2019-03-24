#include <CenterPointOutLineIterator.hpp>
#include <gtest/gtest.h>
#include <vector>

using std::vector;

TEST(CenterPointOutLineIterator, CenterPointOutLineIteratorOdd) {
    CenterPointOutLineIterator c(7);
    vector<uint> result;
    while (c.hasNext())
        result.push_back(c.next());
    vector<uint> expect = {3, 4, 2, 5, 1, 6, 0};
    ASSERT_EQ(result, expect);
}

TEST(CenterPointOutLineIterator, CenterPointOutLineIteratorEven) {
    CenterPointOutLineIterator c(6);
    vector<uint> result;
    while (c.hasNext())
        result.push_back(c.next());
    vector<uint> expect = {2, 3, 1, 4, 0, 5};
    ASSERT_EQ(result, expect);
}