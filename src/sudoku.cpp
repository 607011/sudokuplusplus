/*
    Copyright (c) 2023-2025 Oliver Lau, oliver@ersatzworld.net

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

#include <iostream>

#include "sudoku.hpp"

const easy_set<char> sudoku::EMPTY_SET = {'0'};
const easy_set<char> sudoku::ALL_DIGITS = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

sudoku::sudoku(void)
{
    init();
    reset();
    reset_resolutions();
}

sudoku::sudoku(std::string const &board_str)
    : sudoku()
{
    for (size_t i = 0; i < 81; ++i)
    {
        board_[i] = board_str.at(i) == '.'
                        ? EMPTY
                        : board_str.at(i);
    }
    calc_all_candidates();
}

sudoku::sudoku(board_t const &board)
    : sudoku()
{
    board_ = board;
}

void sudoku::init(void)
{
    rng_.seed(static_cast<uint32_t>(util::make_seed()));
    // warmup RNG
    for (int i = 0; i < 10'000; ++i)
    {
        (void)rng_();
    }
    for (size_t i = 0; i < 9; ++i)
    {
        guess_digit_[i] = static_cast<char>(i + '1');
    }
}

void sudoku::reset_resolutions(void)
{
    resolutions_ = {{
        {"obvious single", 0},
        {"hidden single", 0},
        {"obvious pair", 0},
        {"hidden pair", 0},   // not implemented yet
        {"pointing pair", 0}, // not implemented yet
        {"skyscraper", 0},    // not implemented yet
        {"triple", 0}         // not implemented yet
        // and many, many more to come ...
    }};
}

void sudoku::reset(void)
{
    std::fill(board_.begin(), board_.end(), EMPTY);
    solved_boards_.clear();
    shuffle_guesses();
}

void sudoku::shuffle_guesses(void)
{
    std::shuffle(guess_digit_.begin(), guess_digit_.end(), rng_);
}

char const &sudoku::guess_digit(size_t idx) const
{
    return guess_digit_.at(idx);
}

bool sudoku::find_free_cell(int &row, int &col)
{
    for (size_t i = 0; i < board_.size(); ++i)
    {
        if (board_.at(i) == EMPTY)
        {
            row = get_row_for(i);
            col = get_col_for(i);
            return true;
        }
    }
    return false;
}

void sudoku::count_solutions(int &n)
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
        if (is_safe(row, col, guess_digit_.at(i)))
        {
            set(row, col, guess_digit_.at(i));
            count_solutions(n);
            set(row, col, EMPTY); // backtrack
        }
    }
}

int sudoku::solution_count(void)
{
    int n = 0;
    count_solutions(n);
    return n;
}

bool sudoku::count_solutions_limited(int &n)
{
    int row, col;
    bool some_free = find_free_cell(row, col);
    if (!some_free)
    {
        return ++n > 1;
    }
    for (size_t i = 0; i < 9; ++i)
    {
        if (is_safe(row, col, guess_digit_.at(i)))
        {
            set(row, col, guess_digit_.at(i));
            count_solutions_limited(n);
            set(row, col, EMPTY); // backtrack
        }
    }
    return n == 1;
}

bool sudoku::has_one_clear_solution()
{
    int n = 0;
    return count_solutions_limited(n);
}

/* WIP */
void sudoku::random_fill(void)
{
    std::array<unsigned char, 81> unvisited;
    for (unsigned char i = 0; i < unvisited.size(); ++i)
    {
        unvisited[i] = i;
    }
    while (true)
    {
        std::shuffle(unvisited.begin(), unvisited.end(), rng_);
        for (unsigned int i = 0; i < 81; ++i)
        {
            std::cout << '.' << std::flush;
            auto idx = unvisited.at(i);
            shuffle_guesses();
            for (char num : guess_digit_)
            {
                if (is_safe(idx, num))
                {
                    set(idx, num);
                    if (i >= 17 && has_one_clear_solution())
                    {
                        std::cout << "Solving ..." << std::flush;
                        solve();
                        return;
                    }
                    set(idx, EMPTY);
                }
                std::cout << num << std::flush;
            }
        }
        std::cout << "**RETRY**" << std::flush;
    }
}

bool sudoku::solve(void)
{
    int row, col;
    bool some_free = find_free_cell(row, col);
    if (!some_free)
    {
        solved_boards_.push_back(board_);
        return true;
    }
    for (size_t i = 0; i < 9; ++i)
    {
        if (is_safe(row, col, guess_digit_.at(i)))
        {
            set(row, col, guess_digit_.at(i));
            solve();
            set(row, col, EMPTY); // backtrack
        }
    }
    return false;
}

bool sudoku::solve_single(void)
{
    int row, col;
    bool some_free = find_free_cell(row, col);
    if (!some_free)
    {
        return true;
    }
    for (size_t i = 0; i < 9; ++i)
    {
        if (is_safe(row, col, guess_digit_.at(i)))
        {
            set(row, col, guess_digit_.at(i));
            if (solve_single())
            {
                return true;
            }
            set(row, col, EMPTY); // backtrack
        }
    }
    return false;
}

bool sudoku::is_solved()
{
    return std::all_of(std::begin(board_), std::end(board_), [](char cell)
                       { return cell != EMPTY; });
}

void sudoku::resolve_single(int row, int col, char digit)
{
    set(row, col, digit);
}

std::optional<sudoku::single_result_t> sudoku::eliminate_obvious_single(void)
{
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            size_t idx = static_cast<size_t>(row * 9 + col);
            if (notes_.at(idx).size() != 1)
                continue;
            return single_result_t{row, col, *notes_.at(idx).begin()};
        }
    }
    return std::nullopt;
}

std::array<easy_set<char>, 9> sudoku::get_notes_for_unit(unit_t unit_type, int unit_index) const
{
    std::array<easy_set<char>, 9> unit;
    switch (unit_type)
    {
    case Row:
    {
        auto candidates = get_notes_for_row(unit_index);
        std::copy(std::begin(candidates), std::end(candidates), std::begin(unit));
        break;
    }
    case Column:
    {
        auto candidates = get_notes_for_col(unit_index);
        std::copy(std::begin(candidates), std::end(candidates), std::begin(unit));
        break;
    }
    break;
    case Box:
    {
        auto candidates = get_notes_for_box(unit_index);
        std::copy(std::begin(candidates), std::end(candidates), std::begin(unit));
        break;
    }
    default:
        break;
    }
    return unit;
}

std::optional<sudoku::single_result_t> sudoku::find_first_hidden_single_in_unit(unit_t unit_type)
{
    for (int unit_index = 0; unit_index < 9; ++unit_index)
    {
        auto unit = get_notes_for_unit(unit_type, unit_index);
        for (char digit : ALL_DIGITS)
        {
            auto digit_count = std::accumulate(std::begin(unit), std::end(unit), size_t(0),
                                               [digit](size_t total, const easy_set<char> &set)
                                               {
                                                   return total + set.count(digit);
                                               });
            if (digit_count != 1)
                continue;
            for (int i = 0; i < 9; ++i)
            {
                auto candidates = unit.at((size_t)i);
                if (candidates.has(digit))
                {
                    switch (unit_type)
                    {
                    case Row:
                        return single_result_t{unit_index, i, digit};
                    case Column:
                        return single_result_t{i, unit_index, digit};
                    case Box:
                    {
                        int row_start = 3 * (unit_index / 3);
                        int col_start = 3 * (unit_index % 3);
                        int row = row_start + i / 3;
                        int col = col_start + i % 3;
                        return single_result_t{row, col, digit};
                    }
                    }
                }
            }
        }
    }
    return std::nullopt;
}

std::optional<sudoku::single_result_t> sudoku::find_first_hidden_single(void)
{
    for (sudoku::unit_t unit_type : ALL_UNITS)
    {
        auto result = find_first_hidden_single_in_unit(unit_type);
        if (result)
            return result;
    }
    return std::nullopt;
}

int sudoku::resolve_pair(sudoku::pair_result_t const &result)
{
    int removed_count = 0;
    switch (result.unit_type)
    {
    case Row:
    {
        const int row = result.cell1.row;
        auto row_notes = get_notes_for_row(row);
        for (int col = 0; col < (int)row_notes.size(); ++col)
        {
            if ((result.cell1.row == row && result.cell1.col == col) ||
                (result.cell2.row == row && result.cell2.col == col))
                continue;
            removed_count += static_cast<int>(row_notes[(size_t)col].size() - (row_notes[(size_t)col] - result.pair).size());
            row_notes[(size_t)col] -= result.pair;
        }
        break;
    }
    case Column:
    {
        const int col = result.cell1.col;
        auto row_notes = get_notes_for_col(col);
        for (int row = 0; row < (int)row_notes.size(); ++row)
        {
            if ((result.cell1.row == row && result.cell1.col == col) ||
                (result.cell2.row == row && result.cell2.col == col))
                continue;
            removed_count += static_cast<int>(row_notes[row].size() - (row_notes[row] - result.pair).size());
            row_notes[row] -= result.pair;
        }
        break;
    }
    case Box:
    {
        int row_start = result.cell1.row / 3;
        int col_start = result.cell1.col / 3;
        auto box_notes = get_notes_for_box(row_start * 3 + col_start);
        for (int row_offset = 0; row_offset < 3; ++row_offset)
        {
            for (int col_offset = 0; col_offset < 3; ++col_offset)
            {
                int row = row_start + row_offset;
                int col = col_start + col_offset;
                if ((result.cell1.row == row && result.cell1.col == col) ||
                    (result.cell2.row == row && result.cell2.col == col))
                    continue;
                int box_idx = row_offset * 3 + col_offset;
                removed_count += box_notes[box_idx].size() - (box_notes[box_idx] - result.pair).size();
                box_notes[box_idx] -= result.pair;
            }
        }
        break;
    }
    default:
        break;
    }
    return removed_count;
}

std::optional<sudoku::pair_result_t> sudoku::find_obvious_pair_in_unit(unit_t unit_type, int unit_index)
{
    auto unit = get_notes_for_unit(unit_type, unit_index);
    std::cout << "find_obvious_pair_in_unit(\"" << UNIT_STRINGS.at(unit_type) << "\", " << unit_index << ")\n";
    for (int i = 0; i < 9; ++i)
    {
        for (int j = i + 1; j < 9; ++j)
        {
            easy_set<char> cell1_candidates = unit.at((size_t)i);
            easy_set<char> cell2_candidates = unit.at((size_t)j);
            if (cell1_candidates.size() == 0 && cell2_candidates.size() == 0)
                continue;
            auto pair = cell1_candidates & cell2_candidates;
            if (pair.size() != 2 || cell1_candidates.size() != 2 || cell2_candidates.size() != 2)
                continue;
            int row1, col1, row2, col2;
            switch (unit_type)
            {
            case Row:
            {
                row1 = row2 = unit_index;
                col1 = i;
                col2 = j;
                break;
            }
            case Column:
            {
                row1 = i;
                row2 = j;
                col1 = col2 = unit_index;
                break;
            }
            case Box:
            {
                const int row_start = 3 * (unit_index / 3);
                const int col_start = 3 * (unit_index % 3);
                row1 = row_start + i / 3;
                col1 = col_start + i % 3;
                row2 = row_start + j / 3;
                col2 = col_start + j % 3;
                break;
            }
            default:
                break;
            }
            return pair_result_t{
                {row1, col1},
                {row2, col2},
                pair,
                unit_type,
                0};
        }
    }
    return std::nullopt;
}

std::optional<sudoku::pair_result_t> sudoku::eliminate_obvious_pair(void)
{
    for (unit_t unit_type : ALL_UNITS)
    {
        for (int unit_index = 0; unit_index < 9; ++unit_index)
        {
            auto result = find_obvious_pair_in_unit(unit_type, unit_index);
            if (result)
            {
                result->removed_count = resolve_pair(result.value());
                if (result->removed_count > 0)
                    return result;
            }
        }
    }
    return std::nullopt;
}

std::optional<sudoku::pair_result_t> sudoku::find_hidden_pair_in_unit(unit_t unit_type, int unit_index)
{
    std::cout << "find_hidden_pair_in_unit(\"" << UNIT_STRINGS.at(unit_type) << "\", " << unit_index << ")\n";
    auto unit = get_notes_for_unit(unit_type, unit_index);

    // count occurrences of candidates
    std::array<int, 9> candidate_count{0};
    for (auto candidates : unit)
    {
        if (candidates.empty())
            continue;
        for (char candidate : candidates)
        {
            size_t idx = static_cast<size_t>(candidate - '1');
            ++candidate_count[idx];
        }
    }
    // select only those that occur twice
    std::vector<int> potential_pairs;
    potential_pairs.reserve(9);
    for (int i = 0; i < (int)candidate_count.size(); ++i)
    {
        if (candidate_count.at((size_t)i) == 2)
        {
            potential_pairs.push_back(i);
        }
    }
    if (potential_pairs.size() < 2)
        return std::nullopt;

    // the nested loop is a decent way to iterate over all combinations of digit pairs
    for (int i = 0; i < 9; ++i)
    {
        for (int j = i + 1; j < 9; ++j)
        {
            easy_set<char> pair;
            pair.emplace(static_cast<char>(potential_pairs.at(0) + '1'));
            pair.emplace(static_cast<char>(potential_pairs.at(1) + '1'));
            std::vector<int> pair_cells;
            bool other_candidates_present = false;
            for (int k = 0; k < (int)unit.size(); ++k)
            {
                easy_set<char> cell_candidates = unit.at((size_t)k);
                if (cell_candidates.empty() || pair.is_subset_of(cell_candidates))
                    continue;
                pair_cells.push_back(k);
                if (cell_candidates.size() > 2)
                {
                    other_candidates_present = true;
                }
            }
            if (pair_cells.size() != 2 || !other_candidates_present)
                continue;

            int cell1_index = pair_cells.at(0);
            int cell2_index = pair_cells.at(1);
            int row1, col1, row2, col2;
            switch (unit_type)
            {
            case Row:
            {
                row1 = row2 = unit_index;
                col1 = cell1_index;
                col2 = cell2_index;
                break;
            }
            case Column:
            {
                row1 = cell1_index;
                row2 = cell2_index;
                col1 = col2 = unit_index;
                break;
            }
            case Box:
            {
                const int row_start = 3 * (unit_index / 3);
                const int col_start = 3 * (unit_index % 3);
                row1 = row_start + i / 3;
                col1 = col_start + i % 3;
                row2 = row_start + j / 3;
                col2 = col_start + j % 3;
                break;
            }
            default:
                break;
            }
            return pair_result_t{
                {row1, col1},
                {row2, col2},
                pair,
                unit_type,
                0};
        }
    }
    return std::nullopt;
}

std::optional<sudoku::pair_result_t> sudoku::eliminate_hidden_pair(void)
{
    for (unit_t unit_type : ALL_UNITS)
    {
        for (int unit_index = 0; unit_index < 9; ++unit_index)
        {
            auto result = find_hidden_pair_in_unit(unit_type, unit_index);
            if (result)
            {
                result->removed_count = resolve_pair(result.value());
                if (result->removed_count > 0)
                    return result;
            }
        }
    }
    return std::nullopt;
}

bool sudoku::next_step(void)
{
    if (is_solved())
        return false;
    bool progress_made = false;
    auto result = eliminate_obvious_single();
    if (result)
    {
        std::cout << "obvious single " << result->digit << " found at " << result->row << ',' << result->col << "\n";
        print_board();
        resolve_single(result->row, result->col, result->digit);
        calc_all_candidates();
        progress_made = true;
        ++resolutions_["obvious single"];
    }
    if (!progress_made)
    {
        auto result = find_first_hidden_single();
        if (result)
        {
            std::cout << "hidden single " << result->digit << " found at " << result->row << ',' << result->col << "\n";
            print_board();
            resolve_single(result->row, result->col, result->digit);
            calc_all_candidates();
            progress_made = true;
            ++resolutions_["hidden single"];
        }
    }
    if (!progress_made)
    {
        auto result = eliminate_obvious_pair();
        if (result)
        {
            std::cout << "obvious pair (" << *(result->pair.begin()) << ' ' << *(std::next(result->pair.begin())) << ") found at ("
                      << result->cell1.row << ',' << result->cell1.col << ' '
                      << result->cell2.row << ',' << result->cell2.col << ")\n";

            print_board();
            progress_made = true;
            resolutions_["obvious pair"] += result->removed_count;
        }
    }
    if (!progress_made)
    {
        auto result = eliminate_hidden_pair();
        if (result)
        {
            std::cout << "hidden pair (" << *(result->pair.begin()) << ' ' << *(std::next(result->pair.begin())) << ") found at ("
                      << result->cell1.row << ',' << result->cell1.col << ' '
                      << result->cell2.row << ',' << result->cell2.col << ")\n";

            print_board();
            progress_made = true;
            resolutions_["hidden pair"] += result->removed_count;
        }
    }
    return !is_solved();
}

const std::unordered_map<std::string, int> &sudoku::resolutions(void) const
{
    return resolutions_;
}

bool sudoku::solve_like_a_human(int &num_steps)
{
    reset_resolutions();
    bool not_solved = true;
    num_steps = 0;
    while (not_solved)
    {
        not_solved = next_step();
        ++num_steps;
    }
    return false;
}

void sudoku::calc_all_candidates(void)
{
    for (auto candidates : notes_)
    {
        candidates.clear();
    }
    std::array<easy_set<char>, 9> row_forbidden;
    std::array<easy_set<char>, 9> col_forbidden;
    std::array<easy_set<char>, 9> box_forbidden;
    for (size_t i = 0; i < 9; ++i)
    {
        const auto row_data = get_row((int)i);
        row_forbidden[i] = easy_set<char>(std::begin(row_data), std::end(row_data)) - EMPTY_SET;
        const auto col_data = get_col((int)i);
        col_forbidden[i] = easy_set<char>(std::begin(col_data), std::end(col_data)) - EMPTY_SET;
        const auto box_data = get_box((int)i);
        box_forbidden[i] = easy_set<char>(std::begin(box_data), std::end(box_data)) - EMPTY_SET;
    }
    for (size_t i = 0; i < notes_.size(); ++i)
    {
        int row = get_row_for(i);
        int col = get_col_for(i);
        int box = get_box_for(i);
        if (get(row, col) == EMPTY)
        {
            notes_[i] =
                ALL_DIGITS -
                row_forbidden.at((size_t)row) -
                col_forbidden.at((size_t)col) -
                box_forbidden.at((size_t)box);
        }
        else
        {
            notes_[i].clear();
        }

        // std::cout << "notes (" << row << ',' << col << "): ";
        // dump_set(notes_.at(i));
    }
}

void sudoku::dump_set(easy_set<char> const &s)
{
    for (char e : s)
    {
        std::cout << ' ' << e;
    }
    std::cout << std::endl;
}

void sudoku::dump_candidates(void)
{
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            std::cout << "(" << row << "," << col << ") ";
            for (char candidate : notes_.at(static_cast<size_t>(row * 9 + col)))
            {
                std::cout << ' ' << candidate;
            }
            std::cout << std::endl;
        }
    }
}

/**
 * @brief Dump board as flattened array to output stream
 *
 * @param os std::ostream to dump to
 */
void sudoku::dump(std::ostream &os) const
{
    os.write(board_.data(), static_cast<std::streamsize>(board_.size()));
}

void sudoku::print_board(void) const
{
    for (int row = 0; row < 9; ++row)
    {
        for (char digit : get_row(row))
        {
            std::cout << ' ' << (digit == EMPTY ? '.' : digit);
        }
        std::cout << '\n';
    }
}

/**
 * @brief Count empty cells.
 *
 * @return int number of empty cells
 */
int sudoku::empty_count(void) const
{
    return static_cast<int>(std::count(board_.begin(), board_.end(), EMPTY));
}

/**
 * @brief Get all calculated solutions of this Sudoku game.
 *
 * @return std::vector<board_t> const&
 */
std::vector<sudoku::board_t> const &sudoku::solved_boards() const
{
    return solved_boards_;
}

/**
 * @brief Place a digit on the flattened board at the specified index.
 *
 * @param idx The index to place the digit at
 * @param digit The digit to place
 */
void sudoku::set(size_t idx, char digit)
{
    board_[idx] = digit;
}

/**
 * @brief Set the contents of a certain cell.
 *
 * @param row the cell's row
 * @param col the cell's column
 * @param digit the cell's new value
 */
void sudoku::set(int row, int col, char digit)
{
    board_[index_for(row, col)] = digit;
}

/**
 * @brief Get contents of a certain cell.
 *
 * @param row the cell's row
 * @param col the cell's column
 * @return cell contents
 */
char sudoku::get(int row, int col) const
{
    return board_.at(index_for(row, col));
}

/**
 * @brief Get the value at the specified index.
 *
 * @param idx
 */
char &sudoku::operator[](size_t idx)
{
    return board_[idx];
}

/**
 * @brief Get the value at the specified index.
 *
 * @param idx
 */
char sudoku::at(size_t idx) const
{
    return board_.at(idx);
}

/**
 * @brief Get the digit at the specified row and column.
 *
 * @param row
 * @param col
 */
char sudoku::at(int row, int col) const
{
    return board_.at(static_cast<size_t>(row * 9 + col));
}

/**
 * @brief Get the flattened board.
 *
 * @return board_t const&
 */
sudoku::board_t const &sudoku::board(void) const
{
    return board_;
}

/**
 * @brief Get the Mersenne-Twister based random number generator `sudoku` uses internally.
 *
 * @return std::mt19937&
 */
std::mt19937 &sudoku::rng(void)
{
    return rng_;
}

int sudoku::get_row_for(size_t idx) { return static_cast<int>(idx / 9); }
int sudoku::get_col_for(size_t idx) { return static_cast<int>(idx % 9); }
int sudoku::get_box_for(size_t idx) { return 3 * (get_row_for(idx) / 3) + get_col_for(idx) / 3; }
size_t sudoku::index_for(int row, int col) { return static_cast<size_t>(row * 9 + col); }

/**
 * @brief Check if placing a number at the designated destinaton is safe.
 *
 * The function check if the given number is either present in
 * the given row or column or 3x3 box.
 *
 * @param row row to place into
 * @param col column to place into
 * @param digit digit to place
 * @return true if safe
 * @return false otherwise
 */
bool sudoku::is_safe(int row, int col, char digit) const
{
    // check row and column
    int col_idx = col;
    for (int row_idx = row * 9; row_idx < row * 9 + 9; ++row_idx, col_idx += 9)
    {
        if (board_.at((size_t)row_idx) == digit)
        {
            return false;
        }
        if (board_.at((size_t)col_idx) == digit)
        {
            return false;
        }
    }
    // check 3x3 box
    row -= row % 3;
    col -= col % 3;
    for (int i = row; i < row + 3; ++i)
    {
        for (int j = col; j < col + 3; ++j)
        {
            if (get(i, j) == digit)
            {
                return false;
            }
        }
    }
    return true;
}

bool sudoku::is_safe(size_t idx, char digit) const
{
    int row = get_row_for(idx);
    int col = get_col_for(idx);
    return is_safe(row, col, digit);
}

std::ostream &operator<<(std::ostream &os, const sudoku::board_t &board)
{
    for (int i = 0; i < 81; i += 9)
    {
        os.write(board.data() + i, 9);
        os << '\n';
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const sudoku &game)
{
    os << game.board_;
    return os;
}
