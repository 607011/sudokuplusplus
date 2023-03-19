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
    sudoku()
    {
        init();
        reset();
    }

    explicit sudoku(std::string const &board_str)
        : sudoku()
    {
        assert(board.size() == 81);
        for (size_t i = 0; i < 81U; ++i)
        {
            board[i] = board_str.at(i);
        }
    }

    void init()
    {
        rng.seed(util::make_seed());
        // warmup RNG
        for (int i = 0; i < 10'000; ++i)
        {
            rng();
        }
        for (int i = 0; i < 9; ++i)
        {
            guess_num[i] = static_cast<char>(i + '1');
        }
        for (int i = 0; i < 81; ++i)
        {
            unvisited[i] = i;
        }
    }

    void reset()
    {
        std::fill(board.begin(), board.end(), EMPTY);
        shuffle_guesses();
    }

    inline void shuffle_guesses()
    {
        std::shuffle(guess_num.begin(), guess_num.end(), rng);
    }

    /**
     * @brief Find empty cell.
     *
     * @param[out] row the row of the cell found, if any
     * @param[out] col the columns of the cell found, if any
     * @return true if an empty cell could be found
     * @return false otherwise
     */
    bool find_free_cell(int &row, int &col)
    {
        for (int i = 0; i < 81; ++i)
        {
            if (board.at(i) == EMPTY)
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
        int row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            ++n;
            return;
        }
        for (int i = 0; i < 9; ++i)
        {
            if (n > 2)
            {
                break;
            }
            if (is_safe(row, col, guess_num[i]))
            {
                set(row, col, guess_num[i]);
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
        int row, col;
        bool some_free = find_free_cell(row, col);
        if (!some_free)
        {
            return true;
        }
        for (int i = 0; i < 9; ++i)
        {
            if (is_safe(row, col, guess_num[i]))
            {
                set(row, col, guess_num[i]);
                if (solve())
                {
                    return true;
                }
            }
            set(row, col, EMPTY); // backtrack
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
        os.write(board.data(), board.size());
    }

    /**
     * @brief Count empty cells.
     *
     * @return int number of empty cells
     */
    inline int empty_count() const
    {
        return std::count(board.begin(), board.end(), EMPTY);
    }

#ifdef WITH_GENERATIONS
    std::vector<std::array<char, 81>> const &generations() const
    {
        return evolution;
    }
#endif

    /**
     * @brief Generate Sudoku with "diagonal" algorithm.
     *
     * This function generates a valid Sudoku by randomly filling three diagonal 3x3 boxes.
     * After solving the Sudoku each non-empty cell is checked, if it can be cleared and the Sudoku still has exactly one solution.
     * If no unchecked non-empty cell is left or the desired amount of empty cells is reached, the function returns.
     *
     * @param difficulty 25..64 where 64 is crazy hard
     * @return true if Sudoku contains the desired amount of empty cells
     * @return false otherwise
     */
    bool generate(const int difficulty)
    {
#ifdef WITH_GENERATIONS
        evolution.clear();
#endif
        for (int i = 0; i < 9; i += 3)
        {
            int num_idx = 0;
            shuffle_guesses();
            for (int row = 0; row < 3; ++row)
            {
                for (int col = 0; col < 3; ++col)
                {
                    set(row + i, col + i, guess_num[num_idx++]);
                }
            }
        }
        solve();
        std::cout << "Trying ..." << std::endl
                  << *this << std::endl;
        // visit cells in random order until all are visited
        // or the desired amount of empty cells is reached
        int empty_cells = difficulty;
        int visited_idx = static_cast<int>(unvisited.size());
        std::shuffle(unvisited.begin(), unvisited.end(), rng);
        while (empty_cells > 0 && visited_idx-- > 0)
        {
            const int pos = unvisited.at(visited_idx);
            if (board[pos] != EMPTY)
            {
                auto board_copy = board;
                board[pos] = EMPTY;
                if (solution_count() == 1)
                {
                    --empty_cells;
#ifdef WITH_GENERATIONS
                    evolution.push_back(board);
#endif
                }
                else
                {
                    board = board_copy;
                }
            }
        }
        return empty_cells == 0;
    }

    friend std::ostream &operator<<(std::ostream &os, const sudoku &game);

private:
    static constexpr char EMPTY = '0';

#ifdef WITH_GENERATIONS
    std::vector<std::array<char, 81>> evolution;
#endif

    /**
     * @brief Holds the Sudoku cells in a flattened array.
     *
     */
    std::array<char, 81> board;

    /**
     * @brief Helper array with shuffled digits from 1 to 9
     *
     */
    std::array<char, 9> guess_num;

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
    std::mt19937 rng;

    /**
     * @brief List of unvisited cells used in `generate()`.
     */
    std::array<int, 81> unvisited;

    /**
     * @brief Set the contents of a certain cell.
     *
     * @param row the cell's row
     * @param col the cell's column
     * @param num the cell's new value
     */
    inline void set(int row, int col, char num)
    {
        board[static_cast<size_t>(row * 9 + col)] = num;
    }

    /**
     * @brief Get contents of a certain cell.
     *
     * @param row the cell's row
     * @param col the cell's column
     * @return char cell contents
     */
    inline char get(int row, int col) const
    {
        return board[static_cast<size_t>(row * 9 + col)];
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
    bool is_safe(int row, int col, int num) const
    {
        // check row and column
        int col_idx = col;
        for (int row_idx = row * 9; row_idx < row * 9 + 9; ++row_idx)
        {
            if (board[row_idx] == num)
            {
                return false;
            }
            if (board[col_idx] == num)
            {
                return false;
            }
            col_idx += 9;
        }
        // check 3x3 box
        row -= row % 3;
        col -= col % 3;
        for (int i = row; i < row + 3; ++i)
        {
            for (int j = col; j < col + 3; ++j)
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
