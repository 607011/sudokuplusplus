#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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
    int difficulty = argc == 2
                         ? std::max(1, std::min(6, std::atoi(argv[1])))
                         : 3;
    std::cout << "Generating games for difficulty " << difficulty << " ..." << std::endl;
    std::cout << "(Press Ctrl+C to break.)" << std::endl;
    sudoku game;
    while (true)
    {
        game.generate(difficulty);
        std::cout << std::endl
                  << game
                  << std::endl;
        std::stringstream ss;
        ss << "sudoku-" << iso_datetime() << '-' << difficulty << ".txt";
        std::ofstream out(ss.str());
        out << game;
        game.reset();
    }
    return EXIT_SUCCESS;
}
