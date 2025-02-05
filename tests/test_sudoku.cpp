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

TEST(SudokuTest, IndexAssertions)
{
    size_t idx = 0;
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            EXPECT_EQ(idx++, sudoku::index_for(row, col));
        }
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

TEST(SudokuTest, GetBoxAssertions)
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

TEST(SudokuTest, RowCandidatesAssertions)
{
    sudoku game("000280500500000090470300010032010000910008200060000007600000000003000001000906000");
    {
        for (int row = 0; row < 9; ++row)
        {
            auto notes1 = game.get_notes_for_row(row);
            auto notes2 = game.get_notes_for_unit(sudoku::Row, row);
            auto it1 = std::begin(notes1);
            auto it2 = std::begin(notes2);
            EXPECT_EQ(*it1, *it2);
        }
    }
}

TEST(SudokuTest, ColCandidatesAssertions)
{
    sudoku game("000280500500000090470300010032010000910008200060000007600000000003000001000906000");
    {
        for (int col = 0; col < 9; ++col)
        {
            auto notes1 = game.get_notes_for_col(col);
            auto notes2 = game.get_notes_for_unit(sudoku::Column, col);
            auto it1 = std::begin(notes1);
            auto it2 = std::begin(notes2);
            EXPECT_EQ(*it1, *it2);
        }
    }
}

TEST(SudokuTest, BoxCandidatesAssertions)
{
    sudoku game("000280500500000090470300010032010000910008200060000007600000000003000001000906000");
    std::array<std::array<easy_set<char>, 9>, 9> expected = {
        {/* box 0 */
         {{{'1', '3'},
           {'9'},
           {'9', '6', '1'},
           {},
           {'2', '8'},
           {'8', '6', '1'},
           {},
           {},
           {'9', '6', '8'}}},
         /* box 1 */
         {{{},
           {},
           {'7', '4', '9', '1'},
           {'7', '4', '1', '6'},
           {'7', '4', '6'},
           {'7', '4', '1'},
           {},
           {'9', '6', '5'},
           {'5', '9'}}},
         /* box 2 */
         {{{},
           {'7', '6', '4', '3'},
           {'6', '4', '3'},
           {'7', '6', '4', '3', '8'},
           {},
           {'6', '4', '3', '8', '2'},
           {'6', '8'},
           {},
           {'8', '2', '6'}}},
         /* box 3 */
         {{{'8', '7'},
           {},
           {},
           {},
           {},
           {'7', '5', '4'},
           {'8'},
           {},
           {'8', '5', '4'}}},
         /* box 4 */
         {{{'7', '6', '5', '4'},
           {},
           {'7', '5', '4', '9'},
           {'7', '6', '5', '4'},
           {'7', '6', '5', '4', '3'},
           {},
           {'5', '4'},
           {'5', '4', '9', '3', '2'},
           {'5', '4', '9', '3', '2'}}},
         /* box 5 */
         {{{'8', '4', '9', '6'},
           {'8', '6', '5', '4'},
           {'8', '6', '5', '4', '9'},
           {},
           {'6', '5', '4', '3'},
           {'6', '5', '4', '3'},
           {'4', '9', '1', '3', '8'},
           {'5', '4', '3', '8'},
           {}}},
         /* box 6 */
         {{{},
           {'8', '5', '2', '4', '9'},
           {'9', '8', '7', '5', '4', '1'},
           {'8', '7', '2'},
           {'8', '5', '2', '4', '9'},
           {},
           {'8', '2', '7', '1'},
           {'8', '2', '5', '4'},
           {'8', '7', '5', '4', '1'}}},
         /* box 7 */
         {{{'8', '7', '5', '4', '1'},
           {'5', '4', '3', '2', '7'},
           {'7', '5', '4', '3', '2', '1'},
           {'8', '7', '5', '4'},
           {'5', '2', '7', '4'},
           {'5', '2', '7', '4'},
           {},
           {'5', '4', '3', '2', '7'},
           {}}},
         /* box 8 */
         {{{'7', '4', '9', '3', '8'},
           {'8', '5', '4', '3', '2', '7'},
           {'9', '5', '4', '3', '8', '2'},
           {'8', '7', '4', '9', '6'},
           {'8', '6', '5', '2', '7', '4'},
           {},
           {'7', '4', '3', '8'},
           {'8', '5', '4', '3', '2', '7'},
           {'5', '4', '3', '8', '2'}}}}};
    for (int box = 0; box < 9; ++box)
    {
        auto notes1 = game.get_notes_for_box(box);
        auto notes2 = game.get_notes_for_unit(sudoku::Box, box);
        auto it1 = std::begin(notes1);
        auto it2 = std::begin(notes2);
        auto it3 = std::begin(expected.at(box));
        for (; it1 != std::end(notes1); ++it1, ++it2, ++it3)
        {
            EXPECT_EQ(*it1, *it2);
            EXPECT_EQ(*it1, *it2);
            EXPECT_EQ(*it1, *it3);
        }
    }
}