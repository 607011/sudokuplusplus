#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

#include "sudoku.hpp"

std::string iso_datetime()
{
    time_t now;
    time(&now);
    char buf[sizeof("20230318T091601")];
    strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", gmtime(&now));
    return std::string(buf);
}

int main(int argc, char *argv[])
{
    fseek(stdin, 0, SEEK_END);
    if (ftell(stdin) > 0)
    {
        rewind(stdin);
        std::string data;
        std::string line;
        while (std::getline(std::cin, line)) {
            data.append(line);
        }
        if (data.length() != 81)
        {
            std::cerr << "Board data must contain exactly 81 digits." << std::endl;
            return EXIT_FAILURE;
        }
        sudoku game(data);
        std::cout << game << std::endl;
        game.solve();
        std::cout << "# solutions: " << game.count_solutions() << std::endl;
        std::cout << std::endl
                  << game
                  << std::endl;
    }
    else
    {
        int difficulty = argc == 2
                             ? std::max(1, std::min(6, std::atoi(argv[1])))
                             : 3;
        std::cout << "Generating games with difficulty " << difficulty << " ..." << std::endl;
        std::cout << "(Press Ctrl+C to break.)" << std::endl;
        sudoku game;
        while (true)
        {
            game.generate(difficulty, sudoku::DIAGONAL);
            std::cout << std::endl
                      << game
                      << std::endl;
            std::stringstream ss;
            ss << "sudoku-" << iso_datetime() << '-' << difficulty << ".txt";
            std::ofstream out(ss.str());
            game.dump(out);
            game.reset();
        }
    }
    return EXIT_SUCCESS;
}
