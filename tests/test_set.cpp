#include <gtest/gtest.h>
#include "../src/easy_set.hpp"

TEST(SetTest, BasicAssertions)
{
    easy_set<int> set1 = {1, 2, 3, 4, 5};
    easy_set<int> set2 = {4, 5, 6, 7, 8};

    // Subtraction
    easy_set<int> diffSet = set1 - set2;
    easy_set<int> diffResult = {1, 2, 3};
    EXPECT_TRUE(diffSet == diffResult);

    // Union
    easy_set<int> unionSet = set1 + set2;
    easy_set<int> unionResult = {1, 2, 3, 4, 5, 6, 7, 8};
    EXPECT_TRUE(unionSet == unionResult);

    // Intersection
    easy_set<int> intersectSet = set1 & set2;
    easy_set<int> intersectResult = {4, 5};
    EXPECT_TRUE(intersectSet == intersectResult);

    // Symmetric difference
    easy_set<int> symDiffSet = set1 ^ set2;
    easy_set<int> symDiffResult = {1, 2, 3, 6, 7, 8};
    EXPECT_TRUE(symDiffSet == symDiffResult);
}
