#include <iostream>

#include "sudoku.hpp"

std::ostream &operator<<(std::ostream &os, const sudoku &game)
{
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            os << game.get(row, col);
        }
        os << std::endl;
    }
    return os;
}