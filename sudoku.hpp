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

#include <cassert>
#include <string>
#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <vector>

#include "util.hpp"

class sudoku
{
public:
    typedef std::array<char, 81> board_t;

    sudoku()
    {
        init();
        reset();
    }

    explicit sudoku(std::string const &board_str)
        : sudoku()
    {
        assert(board_.size() == 81);
        for (size_t i = 0; i < 81U; ++i)
        {
            board_[i] = board_str.at(i) == '.'
                            ? EMPTY
                            : board_str.at(i);
        }
    }

    explicit sudoku(board_t const &board)
        : sudoku()
    {
        this->board_ = board;
    }

    void init()
    {
        rng_.seed(static_cast<uint32_t>(util::make_seed()));
        // warmup RNG
        for (int i = 0; i < 10'000; ++i)
        {
            (void)rng_();
        }
        for (size_t i = 0; i < 9; ++i)
        {
            guess_num_[i] = static_cast<char>(i + '1');
        }
        for (size_t i = 0; i < 81; ++i)
        {
            unvisited_[i] = i;
        }
    }

    void reset()
    {
        std::fill(board_.begin(), board_.end(), EMPTY);
        solutions_.clear();
        shuffle_guesses();
    }

    inline void shuffle_guesses()
    {
        std::shuffle(guess_num_.begin(), guess_num_.end(), rng_);
    }

    /**
     * @brief Find empty cell.
     *
     * @param[out] row the row of the cell found, if any
     * @param[out] col the columns of the cell found, if any
     * @return true if an empty cell could be found
     * @return false otherwise
     */
    bool find_free_cell(size_t &row, size_t &col)
    {
        for (size_t i = 0; i < 81; ++i)
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
        size_t row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            ++n;
            return;
        }
        for (size_t i = 0; i < 9; ++i)
        {
            if (n > 2)
            {
                break;
            }
            if (is_safe(row, col, guess_num_[i]))
            {
                set(row, col, guess_num_[i]);
                count_solutions(n);
            }
            set(row, col, EMPTY); // backtrack
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
     * @brief Solve Sudoku.
     *
     * This is a recursive function implementing a backtracking algorithm.
     *
     * @return true if there's at least one empty cell
     * @return false if no empty cell left, thus Sudoku is solved
     */
    bool solve()
    {
        size_t row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            solutions_.push_back(board_);
            return true;
        }
        for (int i = 0; i < 9; ++i)
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
     * @brief Returns a string describing the Sudoku's difficulty.
     *
     */
    std::string level() const
    {
        typedef struct
        {
            int empty_cells;
            std::string description;
        } e2d;
        auto difficulty = empty_count();
        static const std::vector<e2d> lvl = {
            {25, "LEAD"},
            {35, "GOLD"},
            {45, "PLATINUM"},
            {52, "TITANIUM"},
            {58, "HARDENED STEEL"},
            {64, "TUNGSTEN"}};
        auto const &it = std::find_if(lvl.begin(), lvl.end(), [difficulty](e2d const &c) -> bool
                                      { return difficulty <= c.empty_cells; });
        return it->description;
    }

    /**
     * @brief Generate Sudoku with "diagonal" algorithm.
     *
     * This function generates a valid Sudoku by randomly filling three diagonal 3x3 boxes.
     * After solving the Sudoku each non-empty cell is checked if it can be cleared and the Sudoku still has exactly one solution.
     * If no unchecked non-empty cell is left or the desired amount of empty cells is reached, the function returns.
     *
     * @param difficulty 25..64 where 64 is crazy hard
     * @return true if Sudoku contains the desired amount of empty cells
     * @return false otherwise
     */
    std::vector<board_t> generate(const int difficulty)
    {
        for (size_t i = 0; i < 9; i += 3)
        {
            size_t num_idx = 0;
            shuffle_guesses();
            for (size_t row = 0; row < 3; ++row)
            {
                for (size_t col = 0; col < 3; ++col)
                {
                    set(row + i, col + i, guess_num_[num_idx++]);
                }
            }
        }
        solve();
        std::cout << "\n# solutions: " << solutions_.size() << '\n';
        std::vector<sudoku::board_t> actual_solutions;
        for (auto const &board : solutions_)
        {
            sudoku solution(board);
            std::cout << "Trying ...\n"
                      << solution << std::endl;
            // visit cells in random order until all are visited
            // or the desired amount of empty cells is reached
            int empty_cells = difficulty;
            size_t visited_idx = unvisited_.size();
            std::shuffle(unvisited_.begin(), unvisited_.end(), rng_);
            while (empty_cells > 0 && visited_idx-- > 0)
            {
                size_t const pos = unvisited_.at(visited_idx);
                char const cell_copy = board.at(pos);
                solution[pos] = EMPTY;
                if (solution.solution_count() == 1)
                {
                    --empty_cells;
                }
                else
                {
                    solution[pos] = cell_copy;
                }
            }
            if (empty_cells == 0)
            {
                actual_solutions.push_back(solution.board());
                std::cout << "\u001b[32;1mYippieh!\n\n"
                          << solution << "\u001b[0m\n";
            }
            else
            {
                std::cout << empty_cells << " cells above limit."
                          << " \u001b[31;1mDiscarded.\u001b[0m\n\n";
            }
        }
        return actual_solutions;
    }

    std::vector<board_t> const &solutions() const
    {
        return solutions_;
    }

    inline void set(size_t idx, char value)
    {
        board_[idx] = value;
    }

    inline char &operator[](size_t idx)
    {
        return board_[idx];
    }

    inline board_t const &board() const
    {
        return board_;
    }

    friend std::ostream &operator<<(std::ostream &os, const sudoku &game);

private:
    static constexpr char EMPTY = '0';

    /**
     * @brief Holds the Sudoku cells in a flattened array.
     *
     */
    board_t board_;

    std::vector<board_t> solutions_;

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

    /**
     * @brief List of unvisited cells used in `generate()`.
     */
    std::array<size_t, 81> unvisited_;

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
        for (size_t row_idx = row * 9; row_idx < row * 9 + 9; ++row_idx)
        {
            if (board_.at(row_idx) == num)
            {
                return false;
            }
            if (board_.at(col_idx) == num)
            {
                return false;
            }
            col_idx += 9;
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
};

#endif // __SUDOKU_HPP__
