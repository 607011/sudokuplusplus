#include <gtest/gtest.h>
#include "../src/sudoku.hpp"

#include <string>
#include <array>

TEST(SudokuTest, BasicAssertions)
{
    for (size_t i = 0; i < 81; ++i)
    {
        EXPECT_EQ(sudoku::get_row_for(i), i / 9);
        EXPECT_EQ(sudoku::get_col_for(i), i % 9);
        EXPECT_EQ(sudoku::get_box_for(i), 3 * (i / 9 / 3) + (i % 9) / 3);
    }
}

TEST(SudokuTest, GetRowAssertions)
{
    sudoku game("000280500500000090470300010032010000910008200060000007600000000003000001000906000");
    std::array<std::string, 9> results = {
        "000280500",
        "500000090",
        "470300010",
        "032010000",
        "910008200",
        "060000007",
        "600000000",
        "003000001",
        "000906000"};
    for (int i = 0; i < 9; ++i)
    {
        auto row = game.get_row(i);
        std::string result(std::begin(row), std::end(row));
        EXPECT_STREQ(result.c_str(), results.at(i).c_str());
    }
}

TEST(SudokuTest, GetColAssertions)
{
    sudoku game("000280500500000090470300010032010000910008200060000007600000000003000001000906000");
    std::array<std::string, 9> results = {
        "054090600",
        "007316000",
        "000200030",
        "203000009",
        "800100000",
        "000080006",
        "500020000",
        "091000000",
        "000007010"};
    for (int i = 0; i < 9; ++i)
    {
        auto col = game.get_col(i);
        std::string result(std::begin(col), std::end(col));
        EXPECT_STREQ(result.c_str(), results.at(i).c_str());
    }
}

TEST(SudokuTest, GetBlockAssertions)
{
    sudoku game("000280500500000090470300010032010000910008200060000007600000000003000001000906000");
    std::array<std::string, 9> results = {
        "000500470",
        "280000300",
        "500090010",
        "032910060",
        "010008000",
        "000200007",
        "600003000",
        "000000906",
        "000001000"};
    for (int i = 0; i < 9; ++i)
    {
        auto col = game.get_box(i);
        std::string result(std::begin(col), std::end(col));
        EXPECT_STREQ(result.c_str(), results.at(i).c_str());
    }
}
