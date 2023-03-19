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

#include "util.hpp"

class sudoku
{
public:
    typedef enum
    {
        RANDOMIZED,
        DIAGONAL,
    } generation_algorithm_t;

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
        for (int i = 0; i < 1000; ++i)
        {
            rng();
        }
        for (size_t i = 0; i < 9; ++i)
        {
            guess_num[i] = static_cast<char>(i + '1');
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

    bool find_free_cell(int &row, int &col)
    {
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if (is_empty(i, j))
                {
                    row = i;
                    col = j;
                    return true;
                }
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
        for (size_t i = 0; i < 9; ++i)
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
            set(row, col, EMPTY);
        }
    }

    /**
     * @brief Get number of Sudoku's solutions.
     * 
     * @return int number of solutions
     */
    int solution_count()
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
        for (size_t i = 0; i < 9; ++i)
        {
            if (is_safe(row, col, guess_num[i]))
            {
                set(row, col, guess_num[i]);
                if (solve())
                {
                    return true;
                }
            }
            set(row, col, EMPTY);
        }
        return false;
    }

    /**
     * @brief Generate Sudoku.
     * 
     * @param difficulty 1..6 where 6 is crazy hard
     * @param algo the method to generate the Sudoku with
     * @return true if generation was successful
     * @return false otherwise
     */
    bool generate(int difficulty, generation_algorithm_t algo = DIAGONAL)
    {
        switch (algo)
        {
        case RANDOMIZED:
            return generate_randomized(difficulty);
            break;
        case DIAGONAL:
            return generate_diagonal(difficulty);
            break;
        }
    }

    /**
     * @brief Dump board as flattened array to output stream
     * 
     * @param os std::ostream to dump to
     */
    void dump(std::ostream &os) const
    {
        for (size_t i = 0; i < 81; ++i)
        {
            os << board.at(i);
        }
    }

    /**
     * @brief Count empty cells.
     * 
     * @return int number of empty cells
     */
    int empty_count() const
    {
        return std::count(board.begin(), board.end(), EMPTY);
    }

#undef DEBUG

    friend std::ostream &operator<<(std::ostream &os, const sudoku &game);

private:
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
    std::mt19937 rng;
    static constexpr char EMPTY = '0';

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
     * @brief Check if a certain cell is empty.
     * 
     * @param row the row of the cell to check
     * @param col the column of the cell to check
     * @return true if empty
     * @return false if not empty
     */
    inline bool is_empty(int row, int col) const
    {
        return get(row, col) == EMPTY;
    }

    inline int min_empty_cells_for_difficulty(int difficulty) const
    {
        static const int EMPTY_CELLS[7] = {
            -1,
            25,
            35,
            45,
            55,
            58,
            64};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
        if (difficulty < 1 || difficulty > sizeof(EMPTY_CELLS))
#pragma GCC diagnostic pop
        {
            return -1;
        }
        return EMPTY_CELLS[difficulty];
    }

    bool is_safe(int row, int col, int num) const
    {
        for (int i = 0; i < 9; ++i)
        {
            if (get(row, i) == num)
            {
                return false;
            }
            if (get(i, col) == num)
            {
                return false;
            }
        }
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

    bool generate_randomized(int difficulty)
    {
        int n_solutions = 0;
        int n_tries = 1;
        do
        {
            int n = 81 - min_empty_cells_for_difficulty(difficulty);
            while (n > 0)
            {
                int row = rng() % 9;
                int col = rng() % 9;
                char num = '1' + static_cast<char>(rng() % 9);
                if (is_safe(row, col, num))
                {
                    set(row, col, num);
                    --n;
                }
            }
            ++n_tries;
            if (solution_count() != 1)
            {
                reset();
            }
        } while (n_solutions != 1);
        return true;
    }

    /**
     * @brief A helper structure for `generate_diagonal()`.
     * 
     */
    struct cell_pos
    {
        int row;
        int col;
    };

    /**
     * @brief Generate Sudoku with "diagonal" algorithm.
     * 
     * This function generates a valid Sudoku by randomly filling three diagonal 3x3 boxes.
     * After solving the Sudoku each non-empty cell is checked, if it can be cleared and the Sudoku still has exactly one solution.
     * If no unchecked non-empty cell is left or the desired amount of empty cells is reached, the function returns.
     * 
     * @param difficulty 1..6 where 6 is crazy hard
     * @return true if Sudoku contains the desired amount of empty cells
     * @return false otherwise
     */
    bool generate_diagonal(int difficulty)
    {
        for (int i = 0; i < 9; i += 3)
        {
            size_t num_idx = 0;
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
        std::cout << "Trying ..." << std::endl << *this << std::endl;
        int empty_cells = min_empty_cells_for_difficulty(difficulty);
        // build a list of all unvisited cells
        size_t i = 0;
        std::vector<cell_pos> empty_pos(81);
        for (int row = 0; row < 9; ++row)
        {
            for (int col = 0; col < 9; ++col)
            {
                empty_pos[row * 9 + col] = {row, col};
            }
        }
        std::shuffle(empty_pos.begin(), empty_pos.end(), rng);
        while (empty_cells > 0 && empty_pos.size() > 0)
        {
            auto pos = empty_pos.back();
            empty_pos.pop_back();
            int row = pos.row;
            int col = pos.col;
            if (get(row, col) != EMPTY)
            {
                auto board_copy = board;
                set(row, col, EMPTY);
                if (solution_count() > 1)
                {
                    board = board_copy;
                }
                else
                {
                    --empty_cells;
                }
            }
        }
        return empty_cells == 0;
    }
};

#endif // __SUDOKU_HPP__
