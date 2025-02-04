#include <gtest/gtest.h>
#include "../src/easy_set.hpp"

const easy_set<int> set1 = {1, 2, 3, 4, 5};
const easy_set<int> set2 = {4, 5, 6, 7, 8};
const easy_set<int> ALL_DIGITS = {1, 2, 3, 4, 5, 6, 7, 8, 9};

TEST(SetTest, Subtraction1)
{
    easy_set<int> diffSet = set1 - set2;
    easy_set<int> diffResult = {1, 2, 3};
    EXPECT_TRUE(diffSet == diffResult);
}

TEST(SetTest, Subtraction2)
{
    easy_set<int> diffSet = set2 - set1;
    easy_set<int> diffResult = {6, 7, 8};
    EXPECT_TRUE(diffSet == diffResult);
}

TEST(SetTest, Subtraction3)
{
    easy_set<int> rowForbidden = {5, 2, 8};
    easy_set<int> colForbidden = {8, 6};
    easy_set<int> boxForbidden = {2, 8, 3};
    easy_set<int> diffResult = {1, 4, 7, 9};
    EXPECT_TRUE(ALL_DIGITS - rowForbidden - colForbidden - boxForbidden == diffResult);
}

TEST(SetTest, Union)
{
    easy_set<int> unionSet = set1 + set2;
    easy_set<int> unionResult = {1, 2, 3, 4, 5, 6, 7, 8};
    EXPECT_TRUE(unionSet == unionResult);
}

TEST(SetTest, Intersection)
{
    easy_set<int> intersectSet = set1 & set2;
    easy_set<int> intersectResult = {4, 5};
    EXPECT_TRUE(intersectSet == intersectResult);
}

TEST(SetTest, SymmetricDifference)
{
    easy_set<int> symDiffSet = set1 ^ set2;
    easy_set<int> symDiffResult = {1, 2, 3, 6, 7, 8};
    EXPECT_TRUE(symDiffSet == symDiffResult);
}
