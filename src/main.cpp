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

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>
#include <string>
#include <cstdio>
#include <utility>
#include <thread>
#include <mutex>
#include <chrono>
#include <unordered_map>

#include <getopt.hpp>
#include <ncurses.h>

#include "sudoku.hpp"

typedef std::function<void(int, std::mutex &, long long &, long long &)> generator_thread_t;

std::atomic<bool> do_quit{false};
WINDOW *win = nullptr;

#define ATTENTION_PAIR (1)
#define STATUS_ROW (20)

void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        do_quit = true;
    }
}

std::string iso_datetime_now()
{
    time_t now;
    time(&now);
    char buf[sizeof("20230318T091601") + 1];
    strftime(buf, sizeof(buf), "%Y%m%dT%H%M%S", gmtime(&now));
    return std::string(buf);
}

int solve(std::string const &board_data, bool manually)
{
    sudoku game(board_data);
    auto empty_count = game.empty_count();
    std::cout << "Trying to solve\n\n"
              << game << '\n';
    if (manually)
    {
        int num_steps;
        game.solve_like_a_human(num_steps);
        std::cout << "Solved game after " << num_steps << " steps:\n";
        for (const auto &[name, removed_count] : game.resolutions())
        {
            std::cout << " - " << name << " removed " << removed_count << " candidates\n";
        }
        game.print_board();
    }
    else
    {
        game.solve();
        std::cout << "number of solutions: " << game.solution_count() << " (" << game.solved_boards().size() << ")\n"
                  << "empty cells: " << empty_count << " of max. 64\n\n"
                  << game.solved_boards().front()
                  << "\n";
    }
    return EXIT_SUCCESS;
}

void about_to_exit()
{
    move(STATUS_ROW, 2);
    clrtoeol();
    attron(COLOR_PAIR(ATTENTION_PAIR));
    mvprintw(STATUS_ROW, 2, " Exiting ... please be patient! ");
    attroff(COLOR_PAIR(ATTENTION_PAIR));
}

void board_found(sudoku::board_t const &board, std::chrono::time_point<std::chrono::high_resolution_clock> const &t0, int num_empty_cells, bool complete, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
{
    std::lock_guard locker(output_mutex);
    move(5, 0);
    for (int i = 0; i < 81; ++i)
    {
        char c = board.at((size_t)i);
        if (c == '0')
            c = ' ';
        int row = i / 9;
        int col = i % 9;
        int row_offset = row / 3;
        int col_offset = 2 * (col / 3);
        mvaddch(7 + row + row_offset, 4 + (2 * col) + col_offset, c);
    }
    if (complete)
    {
        ++n_games_valid;
        std::string filename = "sudoku-" + iso_datetime_now() + "-" + std::to_string(num_empty_cells) + ".txt";
        if (std::filesystem::exists(filename))
        {
            int seq_no = 0;
            do
            {
                filename = "sudoku-" + iso_datetime_now() + "-" + std::to_string(num_empty_cells) + " (" + std::to_string(seq_no) + ").txt";
                ++seq_no;
            } while (std::filesystem::exists(filename));
        }

        std::ofstream out(filename);
        out.write(board.data(), static_cast<std::streamsize>(board.size()));
        out << std::endl;
        attron(A_BOLD);
        std::string serialized_board(std::begin(board), std::end(board));
        mvprintw(STATUS_ROW, 2, "%s saved to '%s' ...", serialized_board.c_str(), filename.c_str());
    }
    attroff(A_BOLD);
    auto t1 = std::chrono::high_resolution_clock().now();
    auto dt = t1 > t0 ? t1 - t0 : std::chrono::duration<float, std::milli>(1);
    ++n_games_produced;
    mvprintw(4, 2, "games/sec          : %.1f ", static_cast<float>(n_games_produced) * 1e3f / dt.count());
    mvprintw(5, 2, "valid / total games: %lld / %lld", n_games_valid, n_games_produced);
    refresh();
}

void incremental_fill_generator_thread(int num_empty_cells, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
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
    while (!do_quit)
    {
        game.random_fill();
        // visit cells in random order until all are visited
        // or the desired amount of empty cells is reached
        int empty_cells = num_empty_cells;
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
            board_found(game.board(), t0, num_empty_cells, complete, output_mutex, n_games_valid, n_games_produced);
        }
    }
    about_to_exit();
}

/**
 * @brief This Sudoku generator produces valid minimal boards with the specified number of empty cells.
 *
 * Each board is then checked if it has one clear solution. If there's no clear solution, the process repeats.
 */
void mincheck_generator_thread(int num_empty_cells, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
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
    while (!do_quit)
    {
        std::shuffle(unvisited.begin(), unvisited.end(), game.rng());
        unsigned int unvisited_idx = 0;
        int num_placed = 81 - num_empty_cells;
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
            board_found(game.board(), t0, num_empty_cells, true, output_mutex, n_games_valid, n_games_produced);
        }
        else
        {
            board_found(game.board(), t0, num_empty_cells, false, output_mutex, n_games_valid, n_games_produced);
        }
        game.reset();
    }
    about_to_exit();
}

/**
 * @brief This Sudoku generator fills three independent 3x3 boxes with random numbers.
 *
 * The board is then solved. For each solution the generator tries to clear as many cells as requested.
 * If enough cells could be cleared the board is valid, otherwise disposed of.
 */
void prefill_generator_thread(int num_empty_cells, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
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
    std::array<unsigned int, 81> unvisited;
    std::iota(unvisited.begin(), unvisited.end(), 0);
    auto t0 = std::chrono::high_resolution_clock().now();
    while (!do_quit)
    {
        // populate board
        unsigned int num_idx = 0;
        for (auto board_idx : DIAGONAL3X3)
        {
            game[board_idx] = game.guess_digit(num_idx);
            if (++num_idx == 9)
            {
                num_idx = 0;
                game.shuffle_guesses();
            }
        }
        // generate all solutions
        game.solve();
        for (sudoku::board_t const &board : game.solved_boards())
        {
            sudoku possible_solution(board);
            // visit cells in random order until all are visited
            // or the desired amount of empty cells is reached
            int empty_cells = num_empty_cells;
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
                board_found(possible_solution.board(), t0, num_empty_cells, complete, output_mutex, n_games_valid, n_games_produced);
            }
        }
        game.reset();
    }
    about_to_exit();
}

/**
 * @brief This Sudoku generator fills three independent 3x3 blocks with random numbers.
 *
 * The board is then solved. For the first solution the generator tries to clear as many cells as requested.
 * If enough cells could be cleared the board is valid, otherwise disposed of.
 */
void prefill_single_generator_thread(int num_empty_cells, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
{
    static const std::array<uint8_t, 27> DIAGONAL3X3{
        0, 1, 2, 9, 10, 11, 18, 19, 20,
        30, 31, 32, 39, 40, 41, 48, 49, 50,
        60, 61, 62, 69, 70, 71, 78, 79, 80};
    output_mutex.lock();
    sudoku game;
    output_mutex.unlock();
    std::array<unsigned int, 81> unvisited;
    std::iota(unvisited.begin(), unvisited.end(), 0);
    auto t0 = std::chrono::high_resolution_clock().now();
    while (!do_quit)
    {
        // populate board
        unsigned int num_idx = 0;
        for (auto board_idx : DIAGONAL3X3)
        {
            game[board_idx] = game.guess_digit(num_idx);
            if (++num_idx == 9)
            {
                num_idx = 0;
                game.shuffle_guesses();
            }
        }

        game.solve_single();

        // visit cells in random order until all are visited
        // or the desired amount of empty cells is reached
        int empty_cells = num_empty_cells;
        auto visited_idx = unvisited.size();
        std::shuffle(unvisited.begin(), unvisited.end(), game.rng());
        while (empty_cells > 0 && visited_idx-- > 0)
        {
            auto const pos = unvisited.at(visited_idx);
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
        board_found(game.board(), t0, num_empty_cells, complete, output_mutex, n_games_valid, n_games_produced);
        game.reset();
    }
    about_to_exit();
}

void symmetric_generator_thread(int num_empty_cells, std::mutex &output_mutex, long long &n_games_valid, long long &n_games_produced)
{
    static const std::array<uint8_t, 27> DIAGONAL3X3{
        0, 1, 2, 9, 10, 11, 18, 19, 20,
        30, 31, 32, 39, 40, 41, 48, 49, 50,
        60, 61, 62, 69, 70, 71, 78, 79, 80};
    output_mutex.lock();
    sudoku game;
    output_mutex.unlock();
    std::array<unsigned int, 81> unvisited;
    std::iota(unvisited.begin(), unvisited.end(), 0);
    auto t0 = std::chrono::high_resolution_clock().now();
    while (!do_quit)
    {
        // populate board
        unsigned int num_idx = 0;
        for (auto board_idx : DIAGONAL3X3)
        {
            game[board_idx] = game.guess_digit(num_idx);
            if (++num_idx == 9)
            {
                num_idx = 0;
                game.shuffle_guesses();
            }
        }

        game.solve_single();

        // visit cells in random order until all are visited
        // or the desired amount of empty cells is reached
        int empty_cells = num_empty_cells;
        auto visited_idx = unvisited.size();
        while (empty_cells > 0 && visited_idx-- > 0)
        {
            auto const pos = unvisited.at(visited_idx);
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
        board_found(game.board(), t0, num_empty_cells, complete, output_mutex, n_games_valid, n_games_produced);
        game.reset();
    }
    about_to_exit();
}

void draw_header()
{
    const char *const HEADER = " Sudoku Generator ";
    [[maybe_unused]] int win_height;
    int win_width;
    getmaxyx(stdscr, win_height, win_width);
    attron(A_BOLD);
    mvprintw(0, (win_width - (int)strlen(HEADER)) / 2, HEADER);
    attroff(A_BOLD);
}

void draw_sudoku_grid()
{
    for (int i = 0; i <= 3; ++i)
    {
        mvhline(4 * i + 6, 2, ACS_HLINE, 25);
    }
    for (int j = 0; j <= 3; ++j)
    {
        mvvline(6, 2 + j * 8, ACS_VLINE, 13);
    }
    mvaddch(6, 2, ACS_ULCORNER);
    mvaddch(6, 26, ACS_URCORNER);
    mvaddch(18, 2, ACS_LLCORNER);
    mvaddch(18, 26, ACS_LRCORNER);
    mvaddch(6, 10, ACS_TTEE);
    mvaddch(6, 18, ACS_TTEE);
    mvaddch(18, 10, ACS_BTEE);
    mvaddch(18, 18, ACS_BTEE);
    mvaddch(10, 10, ACS_PLUS);
    mvaddch(10, 18, ACS_PLUS);
    mvaddch(14, 10, ACS_PLUS);
    mvaddch(14, 18, ACS_PLUS);
}

void place_scrollable_output()
{
}

std::function<void()> redraw;

void resize_handler(int sig)
{
    if (sig == SIGWINCH)
    {
        endwin();
        refresh();
        redraw();
    }
}

int generate(int num_empty_cells, unsigned int thread_count, std::string const &algorithm_name, generator_thread_t const &generator)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    init_pair(ATTENTION_PAIR, COLOR_WHITE, COLOR_RED);

    int win_height;
    int win_width;
    getmaxyx(stdscr, win_height, win_width);

    win = newwin(win_height - 4, win_width - 4, 2, 2);

    scrollok(win, TRUE);
    box(win, 0, 0);

    redraw = [num_empty_cells, thread_count, algorithm_name]() -> void
    {
        clear();
        border(0, 0, 0, 0, 0, 0, 0, 0);
        draw_header();
        mvprintw(1, 2, "#empty cells wanted: %d", num_empty_cells);
        mvprintw(2, 2, "#threads           : %u", thread_count);
        mvprintw(3, 2, "algorithm          : %s", algorithm_name.c_str());
        draw_sudoku_grid();
        refresh();
    };

    redraw();

    std::signal(SIGWINCH, resize_handler);

    std::vector<std::thread> threads;
    threads.reserve(std::thread::hardware_concurrency());
    std::mutex output_mutex;
    long long n_games_produced = 0;
    long long n_games_valid = 0;
    for (auto i = 0U; i < thread_count; ++i)
    {
        threads.emplace_back(generator,
                             num_empty_cells,
                             std::ref(output_mutex),
                             std::ref(n_games_valid),
                             std::ref(n_games_produced));
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    move(STATUS_ROW, 2);
    clrtoeol();
    mvprintw(STATUS_ROW, 2, "Exiting ...");
    refresh();
    endwin();
    return EXIT_SUCCESS;
}

void usage()
{
    std::cout << "** Sudoku Solver and Generator **\n"
                 "Written by Oliver Lau. Copyright (c) 2023-2025\n\n"
                 "This program will solve a Sudoku served via stdin.\n"
                 "Without any input, Sudokus will be generated.\n\n"
              << "Examples:\n"
                 "\n"
                 "Generate Sudokus with 62 empty cells in 4 threads, using the 'prefill' algorithm:\n"
                 "\n"
                 "   sudoku -d 62 -T 4 --algorithm prefill\n"
                 "\n"
                 "Algorithm descriptions:\n"
                 "\n"
                 "   mincheck\n"
                 "\n"
                 "       1. Produce a randomly filled valid board.\n"
                 "       2. Check board if it has one clear solution.\n"
                 "          If there's no clear solution, repeat.\n"
                 "\n"
                 "   prefill\n"
                 "\n"
                 "       1. Fill three independent 3x3 boxes with random numbers.\n"
                 "       2. Solve the board.\n"
                 "       3. For each solution clear as many cells as requested.\n"
                 "          If enough cells could be cleared the board is valid, otherwise disposed of.\n"
                 "\n"
                 "   prefill-single\n"
                 "\n"
                 "       This is the default algorithm\n"
                 "\n"
                 "       1. Fill three independent 3x3 boxes with random numbers.\n"
                 "       2. Calculate the first solution the board.\n"
                 "       3. Clear as many cells as requested.\n"
                 "          If enough cells could be cleared the board is valid, otherwise disposed of.\n"
                 "\n"
                 "   incremental-fill\n"
                 "\n"
                 "       1. [...] TODO\n"
                 "\n"
                 "Each Sudoku found will be written to a text file named like sudoku-[ISO8601DateTime]-[empty_cells] [seq_no].txt with a contents like (`0` denotes an empty field):\n"
                 "\n"
                 "   007000000\\\n"
                 "   060000800\\\n"
                 "   000020031\\\n"
                 "   000032004\\\n"
                 "   805090000\\\n"
                 "   070006000\\\n"
                 "   501000000\\\n"
                 "   000500060\\\n"
                 "   000400070\\\n"
                 "\n"
                 "Read Sudoku from file and solve it:\n"
                 "\n"
                 "   sudoku --solve-file sudoku61.txt\n"
                 "\n"
                 "Or solve Sudoku serialized as a string:\n"
                 "\n"
                 "   sudoku --solve 008007006000090000012000040100483900000560020000000000000050009000000061001600030\n"
                 "\n";
}

int main(int argc, char *argv[])
{
    std::string const DEFAULT_ALGORITHM = "prefill-single";
    std::unordered_map<std::string, generator_thread_t> const ALGORITHMS = {
        {"prefill-single", &prefill_single_generator_thread},
        {"prefill", &prefill_generator_thread},
        {"mincheck", &mincheck_generator_thread},
        {"incremental-fill", &incremental_fill_generator_thread}};
    int num_empty_cells{61};
    unsigned int thread_count{std::thread::hardware_concurrency()};
    std::string sudoku_filename{};
    std::string board_data{};
    int verbosity{0};
    generator_thread_t generator = prefill_single_generator_thread;
    std::string algorithm_name{"prefill-single"};
    bool solve_manually = false;

    using argparser = argparser::argparser;
    argparser opt(argc, argv);
    opt
        .reg({"-?", "--help"}, argparser::no_argument, "Show this help", [](std::string const &)
             {
                usage();
                exit(EXIT_SUCCESS); })
        .reg({"--solve"}, argparser::required_argument, "Solve a given Sudoku", [&board_data](std::string const &val)
             { board_data = val; })
        .reg({"-m", "--manually"}, argparser::no_argument, "Try to solve like a human would", [&solve_manually](std::string const &)
             { solve_manually = true; })
        .reg({"--solve-file"}, "FILE", argparser::required_argument, "Solve a Sudoku contained in FILE", [&sudoku_filename](std::string const &val)
             { sudoku_filename = val; })
        .reg({"-d", "--empty-cells"}, "NUM", argparser::required_argument, "Produce Sudokus with NUM cells", [&num_empty_cells](std::string const &val)
             { num_empty_cells = std::max(25, std::min(std::stoi(val), 64)); })
        .reg({"-T", "--threads"}, "NUM", argparser::required_argument, "Run generators in NUM threads", [&thread_count](std::string const &val)
             { thread_count = static_cast<unsigned int>(std::stoi(val)); })
        .reg({"-v", "--verbose"}, argparser::no_argument, "Increase verbosity of output", [&verbosity](std::string const &)
             { ++verbosity; })
        .reg({"-a", "--algorithm"}, "ALGO", argparser::required_argument, "Use algorithm ALGO to generate Sudokus", [&ALGORITHMS, &generator, &algorithm_name](std::string const &val)
             {
                if (ALGORITHMS.find(val) != ALGORITHMS.end())
                {
                    algorithm_name = val;
                    generator = ALGORITHMS.at(algorithm_name);
                }
                else
                {
                    std::cerr << "\u001b[31;1mERROR:\u001b[0m invalid algorithm: '" << val << "'\n\n"
                            << "Choose one of\n";
                    for (auto const &a : ALGORITHMS)
                    {
                        std::cerr << " - " << a.first << "\n";
                    }
                    std::cerr << "\nType `sudoku --help` for help.\n\n";
                    exit(EXIT_FAILURE);
                } });
    try
    {
        opt();
    }
    catch (::argparser::argument_required_exception const &e)
    {
        std::cerr << "\u001b[31;1mERROR:\u001b[0m] " << e.what() << "\n\n";
        usage();
        return EXIT_FAILURE;
    }

    if (!sudoku_filename.empty() && !board_data.empty())
    {
        std::cerr << "\u001b[31;1mERROR:\u001b[0m Only one of `--solve` or `--solve-file` is allowed.\n\n";
        return EXIT_FAILURE;
    }
    if (!sudoku_filename.empty())
    {
        std::ifstream fin{sudoku_filename};
        std::string line;
        while (std::getline(fin, line))
        {
            board_data.append(line);
        }
    }
    if (!board_data.empty())
    {
        board_data = util::trim(board_data, " \t\r\n");
        if (board_data.length() != 81)
        {
            std::cerr << "\u001b[31;1mERROR:\u001b[0m Board data must contain exactly 81 digits.\n";
            return EXIT_FAILURE;
        }
        return solve(board_data, solve_manually);
    }

    std::signal(SIGINT, signal_handler);
    int rc = generate(num_empty_cells, thread_count, algorithm_name, generator);

    return rc;
}
