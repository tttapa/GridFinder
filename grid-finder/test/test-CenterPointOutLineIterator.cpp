#include <CenterPointOutLineIterator.hpp>
#include <gtest/gtest.h>
#include <vector>

using std::vector;

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