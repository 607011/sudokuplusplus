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
#include <string>
#include <cstdio>
#include <utility>
#include <thread>
#include <mutex>

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
    size_t empty_count = game.empty_count();
    std::string level = game.level();
    std::cout << game << std::endl;
    game.solve();
    std::cout << "number of solutions: " << game.solution_count() << std::endl
              << "level of difficulty: " << level << " (" << empty_count << " of 64)" << std::endl
              << std::endl
              << game
              << std::endl;
    return EXIT_SUCCESS;
}

int generate(int difficulty, unsigned int thread_count)
{
    std::cout << "Generating games with difficulty " << difficulty << " in " << thread_count << " thread(s) ..." << std::endl;
    std::cout << "(Press Ctrl+C to break.)" << std::endl;

    std::vector<std::thread> threads;
    std::mutex output_mutex;
    long n_games_produced = 0;
    for (auto _i = 0; _i < thread_count; ++_i)
    {
        threads.emplace_back(
            [difficulty, &output_mutex, &n_games_produced]()
            {
                output_mutex.lock();
                sudoku game;
                output_mutex.unlock();
                auto t0 = time(nullptr);
                while (true)
                {
                    bool ok = game.generate(difficulty);
                    {
                        std::lock_guard locker(output_mutex);
                        auto t1 = time(nullptr);
                        ++n_games_produced;
                        auto dt = t1 > t0 ? t1 - t0 : 1;
                        std::cout << (n_games_produced / dt) << " games/sec" << std::endl;
                        std::cout << "# empty cells: " << game.empty_count();
                        if (!ok)
                        {
                            std::cout << " ... \u001b[31;1mdiscarded.\u001b[0m" << std::endl
                                      << std::endl;
                        }
                        else
                        {
                            std::cout << std::endl
                                      << std::endl
                                      << "\u001b[32;1mSuccess!" << std::endl
                                      << std::endl
                                      << game
                                      << "\u001b[0m" << std::endl;
                            std::string filename = "sudoku-" + iso_datetime_now() + "-" + std::to_string(difficulty) + ".txt";
                            if (std::filesystem::exists(filename))
                            {
                                int seq_no = 0;
                                do
                                {
                                    filename = "sudoku-" + iso_datetime_now() + "-" + std::to_string(difficulty) + " (" + std::to_string(seq_no) + ").txt";
                                    ++seq_no;
                                } while (std::filesystem::exists(filename));
                            }
                            std::cout << "\u001b[32mSaving to " << filename << " ... \u001b[0m" << std::endl
                                      << std::endl;
                            std::ofstream out(filename);
                            game.dump(out);
#ifdef WITH_GENERATIONS
                            for (auto const &generation : game.generations())
                            {
                                out << std::endl;
                                out.write(generation.data(), generation.size());
                            }
#endif
                        }
                    }
                    game.reset();
                }
            });
    }

    for (auto &thread : threads)
    {
        thread.join();
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
    int difficulty = argc == 2
                         ? std::max(25, std::min(64, std::atoi(argv[1])))
                         : 50;
    return generate(difficulty, static_cast<int>(std::thread::hardware_concurrency()));
}
