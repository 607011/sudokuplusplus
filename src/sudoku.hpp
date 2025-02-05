/*
    Copyright (c) 2023-2025 Oliver Lau, oliver@ersatzworld.net

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef __SUDOKU_HPP__
#define __SUDOKU_HPP__

#include <algorithm>
#include <array>
#include <cassert>
#include <ctime>
#include <iostream>
#include <optional>
#include <random>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include "easy_set.hpp"
#include "util.hpp"

class sudoku
{
public:
    typedef std::array<char, 81> board_t;

    typedef enum
    {
        Row,
        Column,
        Box
    } unit_t;

    typedef struct
    {
        int row;
        int col;
        char digit;
    } single_result_t;

    typedef struct
    {
        int row;
        int col;
    } coord_t;

    typedef struct
    {
        coord_t cell1;
        coord_t cell2;
        easy_set<char> pair;
        unit_t unit_type;
        int removed_count;
    } pair_result_t;

    const std::array<unit_t, 3> ALL_UNITS = {Row, Column, Box};
    const std::unordered_map<unit_t, std::string> UNIT_STRINGS = {
        {Row, "row"},
        {Column, "column"},
        {Box, "box"}};

    sudoku(void);
    explicit sudoku(std::string const &board_str);
    explicit sudoku(board_t const &board);
    void init(void);
    void reset_resolutions(void);
    void reset(void);
    void shuffle_guesses(void);
    char const &guess_digit(size_t idx) const;
    const std::unordered_map<std::string, int> &resolutions(void) const;

    /**
     * @brief Find empty cell.
     *
     * @param[out] row the row of the cell found, if any
     * @param[out] col the columns of the cell found, if any
     * @return true if an empty cell could be found, false otherwise
     */
    bool find_free_cell(int &row, int &col);

    /**
     * @brief Count the number of solutions.
     *
     * This is a recursive function acting as a solver, implemented as a backtracker.
     *
     * @param[out] n the number of solutions
     */
    void count_solutions(int &n);

    /**
     * @brief Get number of Sudoku's solutions.
     *
     * @return int number of solutions
     */
    int solution_count(void);

    /**
     * @brief Determine if Sudoku has more than one solution.
     *
     * This is a recursive function acting as a solver, implemented as a backtracker.
     *
     * @param[out] n the number of solutions
     */
    bool count_solutions_limited(int &n);

    /**
     * @brief Check if Sudoku has exactly one solution.
     *
     * @return true if Sudoku has one clear solution, false otherwise.
     */
    bool has_one_clear_solution(void);

    void random_fill(void);

    /**
     * @brief Solve Sudoku.
     *
     * This is a recursive function implementing a backtracking algorithm.
     *
     * @return true if there's at least one empty cell
     * @return false if no empty cell left, thus Sudoku is solved
     */
    bool solve(void);
    bool solve_single(void);

    /**
     * @brief Get a row of the board.
     *
     * @param row_idx
     * @return iterator over the row's cells
     */
    std::ranges::subrange<board_t::const_iterator> get_row(int row_idx) const
    {
        return board_ | std::views::drop(row_idx * 9) | std::views::take(9);
    }

    /**
     * @brief Get a column of the board.
     *
     * @param col_idx
     * @return interator over the column's cells
     */
    auto get_col(int col_idx) const
    {
        return std::views::iota(0, 9) |
               std::views::transform([this, col_idx](int row) -> char
                                     { return board_.at(static_cast<size_t>(row * 9 + col_idx)); });
    }

    /**
     * @brief Get a box of the board.
     *
     * @param box_idx
     * @return iterator over the box's cells
     */
    auto get_box(int box_idx) const
    {
        const int box_row = (box_idx / 3) * 3;
        const int box_col = (box_idx % 3) * 3;
        return std::views::iota(0, 9) |
               std::views::transform([this, box_row, box_col](int index) -> char
                                     {
                const int row = box_row + index / 3;
                const int col = box_col + index % 3;
                return board_.at(static_cast<size_t>(row * 9 + col)); });
    }

    auto get_box(int box_idx)
    {
        const int box_row = (box_idx / 3) * 3;
        const int box_col = (box_idx % 3) * 3;
        return std::views::iota(0, 9) |
               std::views::transform([this, box_row, box_col](int index) -> char &
                                     {
                const int row = box_row + index / 3;
                const int col = box_col + index % 3;
                return board_[static_cast<size_t>(row * 9 + col)]; });
    }

    /**
     * @brief Get the notes for a row
     *
     * @param row_idx
     * @return iterator over notes
     */
    auto get_notes_for_row(int row_idx) const
    {
        return notes_ | std::views::drop(row_idx * 9) | std::views::take(9);
    }

    /**
     * @brief Get the notes for a row
     *
     * @param row_idx
     * @return iterator over notes
     */
    std::span<easy_set<char>, 9> get_notes_for_row(int row_idx)
    {
        return std::span<easy_set<char>, 9>{&notes_[static_cast<size_t>(row_idx * 9)], 9};
    }

    /**
     * @brief Get the notes for a column
     *
     * @param col_idx
     * @return iterator over notes
     */
    auto get_notes_for_col(int col_idx) const
    {
        return std::views::iota(0, 9) |
               std::views::transform([this, col_idx](int row) -> easy_set<char> const &
                                     { return notes_.at((static_cast<size_t>(row * 9 + col_idx))); });
    }

    /**
     * @brief Get the notes for a column
     *
     * @param col_idx
     * @return iterator over notes
     */
    auto get_notes_for_col(int col_idx)
    {
        return std::views::iota(0, 9) |
               std::views::transform([this, col_idx](int row) -> easy_set<char> &
                                     { return notes_[static_cast<size_t>(row * 9 + col_idx)]; });
    }

    /**
     * @brief Get the notes for a box
     *
     * @param box_idx
     * @return iterator over notes
     */
    auto get_notes_for_box(int box_idx) const
    {
        const int box_row = (box_idx / 3) * 3;
        const int box_col = (box_idx % 3) * 3;
        return std::views::iota(0, 9) |
               std::views::transform([this, box_row, box_col](int index) -> easy_set<char> const &
                                     {
                int row = box_row + index / 3;
                int col = box_col + index % 3;
                return notes_.at(static_cast<size_t>(row * 9 + col)); });
    }

    /**
     * @brief Get the notes for a box
     *
     * @param box_idx
     * @return iterator over notes
     */
    auto get_notes_for_box(int box_idx)
    {
        const int box_row = (box_idx / 3) * 3;
        const int box_col = (box_idx % 3) * 3;
        return std::views::iota(0, 9) |
               std::views::transform([this, box_row, box_col](int index) -> easy_set<char> &
                                     {
                int row = box_row + index / 3;
                int col = box_col + index % 3;
                return notes_[static_cast<size_t>(row * 9 + col)]; });
    }

    /**
     * @brief Check if board is solved.
     *
     * @return true if board is solved, false otherwise
     */
    bool is_solved(void);
    void resolve_single(int row, int col, char digit);
    std::optional<single_result_t> eliminate_obvious_single(void);
    std::array<easy_set<char>, 9> get_notes_for_unit(unit_t unit_type, int unit_index) const;
    std::optional<single_result_t> find_first_hidden_single_in_unit(unit_t unit_type);
    std::optional<single_result_t> find_first_hidden_single(void);
    int resolve_pair(pair_result_t const &result);
    std::optional<pair_result_t> find_obvious_pair_in_unit(unit_t unit_type, int unit_index);
    std::optional<pair_result_t> eliminate_obvious_pair(void);
    std::optional<pair_result_t> find_hidden_pair_in_unit(unit_t unit_type, int unit_index);
    std::optional<pair_result_t> eliminate_hidden_pair(void);
    bool next_step(void);
    bool solve_like_a_human(int &);
    void calc_all_candidates(void);
    static void dump_set(easy_set<char> const &s);
    void dump_candidates(void);

    /**
     * @brief Dump board as flattened array to output stream
     *
     * @param os std::ostream to dump to
     */
    void dump(std::ostream &os) const;

    void print_board(void) const;

    /**
     * @brief Count empty cells.
     *
     * @return int number of empty cells
     */
    int empty_count(void) const;

    /**
     * @brief Get all calculated solutions of this Sudoku game.
     *
     * @return std::vector<board_t> const&
     */
    std::vector<board_t> const &solved_boards(void) const;

    /**
     * @brief Place a digit on the flattened board at the specified index.
     *
     * @param idx The index to place the digit at
     * @param digit The digit to place
     */
    void set(size_t idx, char digit);

    /**
     * @brief Set the contents of a certain cell.
     *
     * @param row the cell's row
     * @param col the cell's column
     * @param digit the cell's new value
     */
    void set(int row, int col, char digit);

    /**
     * @brief Get contents of a certain cell.
     *
     * @param row the cell's row
     * @param col the cell's column
     * @return cell contents
     */
    char get(int row, int col) const;

    /**
     * @brief Get the value at the specified index.
     *
     * @param idx
     */
    char &operator[](size_t idx);

    /**
     * @brief Get the value at the specified index.
     *
     * @param idx
     */
    char at(size_t idx) const;

    /**
     * @brief Get the digit at the specified row and column.
     *
     * @param row
     * @param col
     */
    char at(int row, int col) const;

    /**
     * @brief Get the flattened board.
     *
     * @return board_t const&
     */
    board_t const &board(void) const;

    /**
     * @brief Get the Mersenne-Twister based random number generator `sudoku` uses internally.
     *
     * @return std::mt19937&
     */
    std::mt19937 &rng(void);

    static int get_row_for(size_t idx);
    static int get_col_for(size_t idx);
    static int get_box_for(size_t idx);
    static size_t index_for(int row, int col);

    /**
     * @brief Check if placing a number at the designated destinaton is safe.
     *
     * The function check if the given number is either present in
     * the given row or column or 3x3 box.
     *
     * @param row row to place into
     * @param col column to place into
     * @param digit digit to place
     * @return true if safe
     * @return false otherwise
     */
    bool is_safe(int row, int col, char digit) const;
    bool is_safe(size_t idx, char digit) const;

    friend std::ostream &operator<<(std::ostream &os, const sudoku &game);

    /**
     * @brief Value of an empty field
     *
     */
    static constexpr char EMPTY = '0';

    static const easy_set<char> EMPTY_SET;
    static const easy_set<char> ALL_DIGITS;

private:
    /**
     * @brief Holds the Sudoku cells in a flattened array.
     *
     */
    board_t board_;

    std::array<easy_set<char>, 81> notes_;

    /**
     * @brief Holds all solutions to the current game.
     *
     */
    std::vector<board_t> solved_boards_;

    /**
     * @brief Helper array with shuffled digits from 1 to 9
     *
     */
    std::array<char, 9> guess_digit_;

    /**
     * @brief Random number generator for a couple of uses.
     *
     * The Mersenne-Twister is known for speed, quite good distribution
     * and a looooooong period of 2^19937-1.
     *
     * A better variant of MT exists: SFMT is better in terms of speed
     * and robustness against statistical tests.
     * TODO: Exchange MT19937 for SFMT (http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/SFMT/)
     */
    std::mt19937 rng_;

    std::unordered_map<std::string, int> resolutions_;
};

std::ostream &operator<<(std::ostream &, const sudoku::board_t &);

#endif // __SUDOKU_HPP__
