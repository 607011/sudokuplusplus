#ifndef __SUDOKU_HPP__
#define __SUDOKU_HPP__

#include <random>
#include <algorithm>
#include <ctime>

class sudoku
{
public:
    sudoku()
    {
        init();
        reset();
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
        for (int i = 0; i < 81; ++i)
        {
            board[i] = EMPTY;
        }
        shuffle_guesses();
    }

    void shuffle_guesses()
    {
        for (int i = 0; i < 9; ++i)
        {
            std::swap(guess_num[i], guess_num[rng() % 9]);
        }
    }

    bool find_free_cell(int &row, int &col)
    {
        int i, j;
        for (i = 0; i < 9; ++i)
        {
            for (j = 0; j < 9; ++j)
            {
                if (get(i, j) == EMPTY)
                {
                    row = i;
                    col = j;
                    return true;
                }
            }
        }
        row = 9;
        col = 9;
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

#undef DEBUG

    void generate(int difficulty)
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
#ifndef DEBUG
            std::cout << '\r' << n_tries << " ... " << std::flush;
#else
            std::cout << std::endl;
            print();
#endif
            ++n_tries;
            n_solutions = 0;
            count_solutions(n_solutions);
            if (n_solutions != 1)
            {
                reset();
            }
        } while (n_solutions != 1);
    }

    friend std::ostream &operator<<(std::ostream &os, const sudoku &game);

private:
    int board[81];
    int guess_num[9];
    std::mt19937 rng;
    static constexpr int EMPTY = 0;

    inline int get(int row, int col) const
    {
        return board[row * 9 + col];
    }

    inline void set(int row, int col, int num)
    {
        board[row * 9 + col] = num;
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
};

#endif // __SUDOKU_HPP__
