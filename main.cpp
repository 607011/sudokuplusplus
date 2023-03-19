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

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <cstdio>

#include "sudoku.hpp"

std::string iso_datetime_now()
{
    time_t now;
    time(&now);
    char buf[sizeof("20230318T091601")];
    strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", gmtime(&now));
    return std::string(buf);
}

int solve()
{
    std::string board_data;
    std::string line;
    while (std::getline(std::cin, line))
    {
        board_data.append(line);
    }
    if (board_data.length() != 81)
    {
        std::cerr << "Board data must contain exactly 81 digits." << std::endl;
        return EXIT_FAILURE;
    }
    sudoku game(board_data);
    std::cout << game << std::endl;
    game.solve();
    std::cout << "# solutions: " << game.solution_count() << std::endl;
    std::cout << std::endl
              << game
              << std::endl;
    return EXIT_SUCCESS;
}

int generate(int difficulty)
{
    std::cout << "Generating games with difficulty " << difficulty << " ..." << std::endl;
    std::cout << "(Press Ctrl+C to break.)" << std::endl;
    sudoku game;
    while (true)
    {
        bool ok = game.generate(difficulty, sudoku::DIAGONAL);
        std::cout << "# empty cells: " << game.empty_count();
        if (!ok)
        {
            std::cout << " ... discarding." << std::endl
                      << std::endl;
            game.reset();
            continue;
        }
        std::cout << std::endl
                  << game
                  << std::endl;
        std::stringstream ss;
        ss << "sudoku-" << iso_datetime_now() << '-' << difficulty << ".txt";
        std::string filename = ss.str();
        if (std::filesystem::exists(filename))
        {
            int seq_no = 0;
            do
            {
                std::stringstream ss;
                ss << "sudoku-" << iso_datetime_now() << '-' << difficulty << " (" << seq_no << ").txt";
                filename = ss.str();
                ++seq_no;
            } while (std::filesystem::exists(filename));
        }
        std::cout << "Saving to " << filename << " ... " << std::endl
                  << std::endl;
        std::ofstream out(filename);
        game.dump(out);
#ifdef WITH_GENERATIONS
        for (auto const& generation : game.generations())
        {
            out << std::endl;
            out.write(generation.data(), generation.size());
        }
#endif
        game.reset();
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    fseek(stdin, 0, SEEK_END);
    if (ftell(stdin) > 0)
    {
        rewind(stdin);
        return solve();
    }
    else
    {
        int difficulty = argc == 2
                             ? std::max(25, std::min(64, std::atoi(argv[1])))
                             : 3;
        return generate(difficulty);
    }
    return EXIT_SUCCESS;
}
