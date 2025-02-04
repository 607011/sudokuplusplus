/*
    Copyright (c) 2023 Oliver Lau, oliver@ersatzworld.net

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

#include <array>
#include <cassert>
#include <ctime>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <optional>
#include <ranges>
#include <vector>

#include "easy_set.hpp"
#include "util.hpp"

class sudoku
{
public:
    typedef std::array<char, 81U> board_t;

    enum
    {
        Row,
        Column,
        Block
    };

    sudoku()
    {
        init();
        reset();
    }

    explicit sudoku(std::string const &board_str)
        : sudoku()
    {
        assert(board_.size() == 81U);
        for (unsigned int i = 0; i < 81U; ++i)
        {
            board_[i] = board_str.at(i) == '.'
                            ? EMPTY
                            : board_str.at(i);
        }
        calc_all_candidates();
        // dump_candidates();
    }

    explicit sudoku(board_t const &board)
        : sudoku()
    {
        board_ = board;
    }

    void init()
    {
        rng_.seed(static_cast<uint32_t>(util::make_seed()));
        // warmup RNG
        for (int i = 0; i < 10'000; ++i)
        {
            (void)rng_();
        }
        for (unsigned int i = 0; i < 9U; ++i)
        {
            guess_num_[i] = static_cast<char>(i + '1');
        }
    }

    void reset()
    {
        std::fill(board_.begin(), board_.end(), EMPTY);
        solved_boards_.clear();
        shuffle_guesses();
    }

    inline void shuffle_guesses()
    {
        std::shuffle(guess_num_.begin(), guess_num_.end(), rng_);
    }

    inline char const &guess_num(unsigned int idx) const
    {
        return guess_num_.at(idx);
    }

    /**
     * @brief Find empty cell.
     *
     * @param[out] row the row of the cell found, if any
     * @param[out] col the columns of the cell found, if any
     * @return true if an empty cell could be found
     * @return false otherwise
     */
    bool find_free_cell(unsigned int &row, unsigned int &col)
    {
        for (unsigned int i = 0; i < 81; ++i)
        {
            if (board_.at(i) == EMPTY)
            {
                row = i / 9;
                col = i % 9;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Count the number of solutions.
     *
     * This is a recursive function acting as a solver, implemented as a backtracker.
     *
     * @param[out] n the number of solutions
     */
    void count_solutions(int &n)
    {
        unsigned int row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            ++n;
            return;
        }
        for (unsigned int i = 0; i < 9; ++i)
        {
            if (is_safe(row, col, guess_num_[i]))
            {
                set(row, col, guess_num_[i]);
                count_solutions(n);
                set(row, col, EMPTY); // backtrack
            }
        }
    }

    /**
     * @brief Get number of Sudoku's solutions.
     *
     * @return int number of solutions
     */
    inline int solution_count()
    {
        int n = 0;
        count_solutions(n);
        return n;
    }

    /**
     * @brief Determine if Sudoku has more than one solution.
     *
     * This is a recursive function acting as a solver, implemented as a backtracker.
     *
     * @param[out] n the number of solutions
     */
    bool count_solutions_limited(int &n)
    {
        unsigned int row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            return ++n > 1;
        }
        for (unsigned int i = 0; i < 9; ++i)
        {
            if (is_safe(row, col, guess_num_[i]))
            {
                set(row, col, guess_num_[i]);
                count_solutions_limited(n);
                set(row, col, EMPTY); // backtrack
            }
        }
        return n == 1;
    }

    /**
     * @brief Check if Sudoku has exactly one solution.
     *
     * @return true if Sudoku has one clear solution, false otherwise.
     */
    inline bool has_one_clear_solution()
    {
        int n = 0;
        return count_solutions_limited(n);
    }

    /** WIP */
    void random_fill()
    {
        std::array<unsigned char, 81> unvisited;
        for (unsigned char i = 0; i < 81; ++i)
        {
            unvisited[i] = i;
        }
        while (true)
        {
            std::shuffle(unvisited.begin(), unvisited.end(), rng_);
            for (unsigned int i = 0; i < 81; ++i)
            {
                std::cout << '.' << std::flush;
                auto idx = unvisited.at(i);
                shuffle_guesses();
                for (char num : guess_num_)
                {
                    if (is_safe(idx, num))
                    {
                        set(idx, num);
                        if (i >= 17 && has_one_clear_solution())
                        {
                            std::cout << "Solving ..." << std::flush;
                            solve();
                            return;
                        }
                        set(idx, EMPTY);
                    }
                    std::cout << num << std::flush;
                }
            }
            std::cout << "**RETRY**" << std::flush;
        }
    }

    /**
     * @brief Solve Sudoku.
     *
     * This is a recursive function implementing a backtracking algorithm.
     *
     * @return true if there's at least one empty cell
     * @return false if no empty cell left, thus Sudoku is solved
     */
    bool solve()
    {
        unsigned int row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            solved_boards_.push_back(board_);
            return true;
        }
        for (unsigned int i = 0; i < 9; ++i)
        {
            if (is_safe(row, col, guess_num_[i]))
            {
                set(row, col, guess_num_[i]);
                solve();
                set(row, col, EMPTY); // backtrack
            }
        }
        return false;
    }

    bool solve_single()
    {
        unsigned int row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            return true;
        }
        for (size_t i = 0; i < 9; ++i)
        {
            if (is_safe(row, col, guess_num_[i]))
            {
                set(row, col, guess_num_[i]);
                if (solve_single())
                {
                    return true;
                }
                set(row, col, EMPTY); // backtrack
            }
        }
        return false;
    }

    auto get_row(int row_idx) const
    {
        return board_ | std::views::drop(row_idx * 9) | std::views::take(9);
    }

    auto get_col(int col_idx) const
    {
        return std::views::iota(0, 9) |
               std::views::transform([this, col_idx](int row)
                                     { return board_.at(static_cast<size_t>(row * 9 + col_idx)); });
    }

    auto get_block(int block_idx)
    {
        int block_row = (block_idx / 3) * 3;
        int block_col = (block_idx % 3) * 3;

        return std::views::iota(0, 9) |
               std::views::transform([this, block_row, block_col](int index)
                                     {
                int row = block_row + index / 3;
                int col = block_col + index % 3;
                return board_.at(static_cast<size_t>(row * 9 + col)); });
    }

    auto get_candidates_row(int row_idx) const
    {
        return candidates_ | std::views::drop(row_idx * 9) | std::views::take(9);
    }

    auto get_candidates_col(int col_idx) const
    {
        return std::views::iota(0, 9) |
               std::views::transform([this, col_idx](int row)
                                     { return candidates_.at((static_cast<size_t>(row * 9 + col_idx))); });
    }

    auto get_candidates_block(int block_idx) const
    {
        int block_row = (block_idx / 3) * 3;
        int block_col = (block_idx % 3) * 3;

        return std::views::iota(0, 9) |
               std::views::transform([this, block_row, block_col](int index)
                                     {
                int row = block_row + index / 3;
                int col = block_col + index % 3;
                return candidates_.at(static_cast<size_t>(row * 9 + col)); });
    }

    bool solve_like_a_human()
    {
        // TODO: implement lots of techniques ;-)
        return false;
    }

    /**
     * @brief Dump board as flattened array to output stream
     *
     * @param os std::ostream to dump to
     */
    inline void dump(std::ostream &os) const
    {
        os.write(board_.data(), static_cast<std::streamsize>(board_.size()));
    }

    /**
     * @brief Count empty cells.
     *
     * @return int number of empty cells
     */
    inline int empty_count() const
    {
        return static_cast<int>(std::count(board_.begin(), board_.end(), EMPTY));
    }

    /**
     * @brief Get all calculated solutions of this Sudoku game.
     *
     * @return std::vector<board_t> const&
     */
    std::vector<board_t> const &solved_boards() const
    {
        return solved_boards_;
    }

    /**
     * @brief Place a digit on the flattened board at the specified index.
     *
     * @param idx The index to place the digit at
     * @param value The digit to place
     */
    inline void set(unsigned int idx, char value)
    {
        board_[idx] = value;
    }

    /**
     * @brief Get the value at the specified index.
     *
     * @param idx
     * @param value
     */
    inline char &operator[](unsigned int idx)
    {
        return board_[idx];
    }

    /**
     * @brief Get the value at the specified index.
     *
     * @param idx
     * @param value
     */
    inline char at(unsigned int idx) const
    {
        return board_.at(idx);
    }

    inline char at(unsigned int row, unsigned int col)
    {
        return board_.at(row * 9 + col);
    }

    /**
     * @brief Get the flattened board.
     *
     * @return board_t const&
     */
    inline board_t const &board() const
    {
        return board_;
    }

    /**
     * @brief Get the Mersenne-Twister based random number generator `sudoku` uses internally.
     *
     * @return std::mt19937&
     */
    inline std::mt19937 &rng()
    {
        return rng_;
    }

    /**
     * @brief Set the contents of a certain cell.
     *
     * @param row the cell's row
     * @param col the cell's column
     * @param num the cell's new value
     */
    inline void set(size_t row, size_t col, char num)
    {
        board_[row * 9 + col] = num;
    }

    /**
     * @brief Get contents of a certain cell.
     *
     * @param row the cell's row
     * @param col the cell's column
     * @return char cell contents
     */
    inline char get(size_t row, size_t col) const
    {
        return board_.at(row * 9 + col);
    }

    static inline size_t get_row_for(size_t idx) { return idx / 9; }
    static inline size_t get_col_for(size_t idx) { return idx % 9; }
    static inline size_t get_block_for(size_t idx) { return 3 * (get_row_for(idx) / 3) + get_col_for(idx) / 3; }

    /**
     * @brief Check if placing a number at the designated destinaton is safe.
     *
     * The function check if the given number is either present in
     * the given row or column or 3x3 box.
     *
     * @param row row to place into
     * @param col column to place into
     * @param num number to place
     * @return true if safe
     * @return false otherwise
     */
    bool is_safe(size_t row, size_t col, char num) const
    {
        // check row and column
        size_t col_idx = col;
        for (size_t row_idx = row * 9; row_idx < row * 9 + 9; ++row_idx, col_idx += 9)
        {
            if (board_.at(row_idx) == num)
            {
                return false;
            }
            if (board_.at(col_idx) == num)
            {
                return false;
            }
        }
        // check 3x3 box
        row -= row % 3;
        col -= col % 3;
        for (size_t i = row; i < row + 3; ++i)
        {
            for (size_t j = col; j < col + 3; ++j)
            {
                if (get(i, j) == num)
                {
                    return false;
                }
            }
        }
        return true;
    }

    inline bool is_safe(size_t idx, char num) const
    {
        size_t row = get_row_for(idx);
        size_t col = get_col_for(idx);
        return is_safe(row, col, num);
    }

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

    std::array<easy_set<char>, 81> candidates_;

    /**
     * @brief Holds all solutions to the current game.
     *
     */
    std::vector<board_t> solved_boards_;

    /**
     * @brief Helper array with shuffled digits from 1 to 9
     *
     */
    std::array<char, 9> guess_num_;

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

private: // methods
    void calc_all_candidates()
    {
        for (auto candidates : candidates_)
        {
            candidates.clear();
        }
        std::array<easy_set<char>, 9U> row_forbidden;
        std::array<easy_set<char>, 9U> col_forbidden;
        std::array<easy_set<char>, 9U> block_forbidden;
        for (size_t i = 0; i < 9; ++i)
        {
            const auto row_data = get_row((int)i);
            row_forbidden[i] = easy_set<char>(std::begin(row_data), std::end(row_data)) - EMPTY_SET;
            const auto col_data = get_col((int)i);
            col_forbidden[i] = easy_set<char>(std::begin(col_data), std::end(col_data)) - EMPTY_SET;
            const auto block_data = get_block((int)i);
            block_forbidden[i] = easy_set<char>(std::begin(block_data), std::end(block_data)) - EMPTY_SET;
        }
        for (size_t i = 0; i < candidates_.size(); ++i)
        {
            size_t row = get_row_for(i);
            size_t col = get_col_for(i);
            size_t block = get_block_for(i);
            if (get(row, col) == EMPTY)
            {
                candidates_[i] =
                    ALL_DIGITS -
                    row_forbidden.at(row) -
                    col_forbidden.at(col) -
                    block_forbidden.at(block);
            }
        }
    }

    static void dump_set(easy_set<char> const &s)
    {
        for (char e : s)
        {
            std::cout << ' ' << e;
        }
        std::cout << std::endl;
    }

    void dump_candidates()
    {
        for (int row = 0; row < 9; ++row)
        {
            for (int col = 0; col < 9; ++col)
            {
                std::cout << "(" << row << "," << col << ") ";
                for (char candidate : candidates_.at(static_cast<size_t>(row * 9 + col)))
                {
                    std::cout << ' ' << candidate;
                }
                std::cout << std::endl;
            }
        }
    }
};

std::ostream &operator<<(std::ostream &, const sudoku::board_t &);

#endif // __SUDOKU_HPP__
