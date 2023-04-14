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
#include <chrono>

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
    auto empty_count = game.empty_count();
    std::string level = game.level();
    std::cout << game << std::endl;
    game.solve();
    std::cout << "number of solutions: " << game.solution_count() << " (" << game.solved_boards().size() << ")\n"
              << "level of difficulty: " << level << " (" << empty_count << " of 64)\n\n"
              << game.solved_boards().front()
              << std::endl;
    return EXIT_SUCCESS;
}

void board_found(sudoku::board_t const &board, std::chrono::steady_clock::time_point const &t0, int difficulty, int empty_cells, bool complete, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
{
    std::lock_guard locker(output_mutex);
    if (complete)
    {
        ++n_games_valid;
        std::cout << "\n\n\u001b[32;1mSuccess!\n\n";
        for (int i = 0; i < 81; i += 9)
        {
            std::cout.write(board.data() + i, 9);
            std::cout << '\n';
        }
        std::cout << "\u001b[0m\n";
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
        std::cout << "\u001b[32mSaving to " << filename << " ... \u001b[0m\n\n"
                  << std::flush;
        std::ofstream out(filename);
        out.write(board.data(), static_cast<std::streamsize>(board.size()));
    }
    else
    {
        std::cout << empty_cells << " cells above limit."
                  << " \u001b[31;1mDiscarded.\u001b[0m\n\n";
    }
    auto t1 = std::chrono::high_resolution_clock().now();
    auto dt = t1 > t0 ? t1 - t0 : std::chrono::duration<float, std::milli>(1);
    ++n_games_produced;
    std::cout << std::setprecision(3) << (n_games_produced * 1e3f / dt.count()) << " games/sec; "
              << n_games_produced << " games total; of these "
              << n_games_valid << " with specified difficulty " << difficulty << ".\n"
              << std::endl;
}

void incremental_fill_generator_thread(int difficulty, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
{
    output_mutex.lock();
    sudoku game;
    output_mutex.unlock();
    std::array<unsigned int, 81U> unvisited;
    for (unsigned int i = 0; i < 81U; ++i)
    {
        unvisited[i] = i;
    }
    auto t0 = std::chrono::high_resolution_clock().now();
    while (true)
    {
        game.random_fill();
        std::cout << "Trying ...\n"
                  << game << std::endl;
        // visit cells in random order until all are visited
        // or the desired amount of empty cells is reached
        int empty_cells = difficulty;
        unsigned int visited_idx = unvisited.size();
        std::shuffle(unvisited.begin(), unvisited.end(), game.rng());
        while (empty_cells > 0 && visited_idx-- > 0)
        {
            unsigned int const pos = unvisited.at(visited_idx);
            char const cell_copy = game.at(pos);
            game[pos] = sudoku::EMPTY;
            if (game.has_one_clear_solution())
            {
                --empty_cells;
            }
            else
            {
                game[pos] = cell_copy;
            }
        }
        const bool complete = empty_cells == 0;
        {
            board_found(game.board(), t0, difficulty, empty_cells, complete, output_mutex, n_games_valid, n_games_produced);
        }
    }
}

/**
 * @brief This Sudoku generator produces valid minimal boards with the specified number of empty cells.
 *
 * Each board is then checked if it has one clear solution. If there's no clear solution, the process repeats.
 */
void mincheck_generator_thread(int difficulty, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
{
    output_mutex.lock();
    sudoku game;
    output_mutex.unlock();
    std::array<unsigned int, 81U> unvisited;
    for (unsigned int i = 0; i < 81U; ++i)
    {
        unvisited[i] = i;
    }
    auto t0 = std::chrono::high_resolution_clock().now();
    while (true)
    {
        std::shuffle(unvisited.begin(), unvisited.end(), game.rng());
        unsigned int unvisited_idx = 0;
        int num_placed = 81 - difficulty;
        while (num_placed > 0)
        {
            unsigned int visit_idx = unvisited.at(unvisited_idx);
            char num = '1' + static_cast<char>(game.rng()() % 9);
            if (game.is_safe(visit_idx, num))
            {
                game[visit_idx] = num;
                --num_placed;
            }
            ++unvisited_idx;
            if (unvisited_idx == 81)
            {
                unvisited_idx = 0;
                std::shuffle(unvisited.begin(), unvisited.end(), game.rng());
            }
        }
        if (game.has_one_clear_solution())
        {
            board_found(game.board(), t0, difficulty, 0, true, output_mutex, n_games_valid, n_games_produced);
        }
        else
        {
            board_found(game.board(), t0, difficulty, 0, false, output_mutex, n_games_valid, n_games_produced);
        }
        game.reset();
    }
}

/**
 * @brief This Sudoku generator fills three independent 3x3 blocks with random numbers.
 *
 * The board is then solved. For each solution the generator tries to clear as many cells as required by the difficulty level.
 * If enough cells could be cleared the board is valid, otherwise disposed of.
 */
void prefill_generator_thread(int difficulty, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
{
    static const std::array<uint8_t, 27> DIAGONAL3X3{
        0, 1, 2, 9, 10, 11, 18, 19, 20,
        30, 31, 32, 39, 40, 41, 48, 49, 50,
        60, 61, 62, 69, 70, 71, 78, 79, 80};
    // static const std::array<uint8_t, 27> INDEP3X3{
    //     0, 1, 2, 9, 10, 11, 18, 19, 20,
    //     33, 34, 35, 42, 43, 44, 51, 52, 53,
    //     57, 58, 59, 66, 67, 68, 75, 76, 77};
    output_mutex.lock();
    sudoku game;
    output_mutex.unlock();
    std::array<unsigned int, 81U> unvisited;
    for (unsigned int i = 0; i < 81U; ++i)
    {
        unvisited[i] = i;
    }
    auto t0 = std::chrono::high_resolution_clock().now();
    while (true)
    {
        // populate board
        unsigned int num_idx = 0;
        for (auto board_idx : DIAGONAL3X3)
        {
            game[board_idx] = game.guess_num(num_idx);
            if (++num_idx == 9)
            {
                num_idx = 0;
                game.shuffle_guesses();
            }
        }
        // generate all solutions
        game.solve();
        std::cout << "# solutions: " << game.solved_boards().size() << "\n\n";
        for (sudoku::board_t const &board : game.solved_boards())
        {
            sudoku possible_solution(board);
            std::cout << "Trying ...\n"
                      << possible_solution << std::endl;
            // visit cells in random order until all are visited
            // or the desired amount of empty cells is reached
            int empty_cells = difficulty;
            unsigned int visited_idx = unvisited.size();
            std::shuffle(unvisited.begin(), unvisited.end(), game.rng());
            while (empty_cells > 0 && visited_idx-- > 0)
            {
                unsigned int const pos = unvisited.at(visited_idx);
                char const cell_copy = board.at(pos);
                possible_solution[pos] = sudoku::EMPTY;
                if (possible_solution.has_one_clear_solution())
                {
                    --empty_cells;
                }
                else
                {
                    possible_solution[pos] = cell_copy;
                }
            }
            const bool complete = empty_cells == 0;
            {
                board_found(possible_solution.board(), t0, difficulty, empty_cells, complete, output_mutex, n_games_valid, n_games_produced);
            }
        }
        game.reset();
    }
}

int generate(int difficulty, unsigned int thread_count)
{
    std::cout << "Generating games with difficulty " << difficulty
              << " in " << thread_count << " thread" << (thread_count == 1 ? "" : "s")
              << " ...\n"
              << "(Press Ctrl+C to break.)" << std::endl;

    std::vector<std::thread> threads;
    threads.reserve(std::thread::hardware_concurrency());
    std::mutex output_mutex;
    long long n_games_produced = 0;
    long long n_games_valid = 0;
    for (auto i = 0U; i < thread_count; ++i)
    {
        threads.emplace_back(&prefill_generator_thread,
                             difficulty,
                             std::ref(output_mutex),
                             std::ref(n_games_valid),
                             std::ref(n_games_produced));
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
    int difficulty = argc > 1
                         ? std::max(25, std::min(64, std::atoi(argv[1])))
                         : 50;
    unsigned int thread_count = argc > 2
                                    ? static_cast<unsigned int>(std::atoi(argv[2]))
                                    : std::thread::hardware_concurrency();
    return generate(difficulty, thread_count);
}
