#ifndef __SUDOKU_HPP__
#define __SUDOKU_HPP__

#include <cassert>
#include <string>
#include <array>
#include <random>
#include <algorithm>
#include <ctime>

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
        assert(board.length() == 81);
        for (int i = 0; i < 81; ++i)
        {
            board[i] = board_str.at(i) - '0';
        }
    }

    void init()
    {
        rng.seed((unsigned int)time(nullptr));
        // warmup RNG
        for (int i = 0; i < 1000; ++i)
        {
            rng();
        }
        for (int i = 0; i < 9; ++i)
        {
            guess_num[i] = i + 1;
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
            set(row, col, EMPTY);
        }
    }

    int count_solutions()
    {
        int n = 0;
        count_solutions(n);
        return n;
    }


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
            set(row, col, EMPTY);
        }
        return false;
    }

    void generate(int difficulty, generation_algorithm_t algo)
    {
        switch (algo)
        {
        case RANDOMIZED:
            generate_randomized(difficulty);
            break;
        case DIAGONAL:
            generate_diagonal(difficulty);
        }
    }

    void dump(std::ostream &os) const
    {
        for (int i = 0; i < 81; ++i)
        {
            os << static_cast<char>(board[i] + '0');
        }
    }

#undef DEBUG

    friend std::ostream &operator<<(std::ostream &os, const sudoku &game);

private:
    std::array<int, 81> board;
    std::array<int, 9> guess_num;
    std::mt19937 rng;
    static constexpr int EMPTY = 0;

    inline void set(int row, int col, int num)
    {
        board[row * 9 + col] = num;
    }

    inline int get(int row, int col) const
    {
        return board[row * 9 + col];
    }

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
            52,
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
                int num = 1 + rng() % 9;
                if (is_safe(row, col, num))
                {
                    set(row, col, num);
                    --n;
                }
            }
            ++n_tries;
            if (count_solutions() != 1)
            {
                reset();
            }
        } while (n_solutions != 1);
        return true;
    }

    void generate_diagonal(int difficulty)
    {
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
        int empty_cells = min_empty_cells_for_difficulty(difficulty);
        while (empty_cells > 0)
        {
            int row = rng() % 9;
            int col = rng() % 9;
            if (get(row, col) != EMPTY)
            {
                auto board_copy = board;
                set(row, col, EMPTY);
                int n_solutions = 0;
                count_solutions(n_solutions);
                if (n_solutions > 1)
                {
                    board = board_copy;
                }
                else
                {
                    --empty_cells;
                }
            }
        }
    }
};

#endif // __SUDOKU_HPP__
