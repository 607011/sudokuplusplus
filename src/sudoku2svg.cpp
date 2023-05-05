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
#include <string>
#include <fstream>
#include <iostream>

#include <getopt.hpp>

#include "util.hpp"

namespace
{
    void usage()
    {
        std::cout
            << "USAGE:\n\n"
               "  sudoku2svg SUDOKU_FILENAME SVG_FILENAME\n\n";
    }

}

int main(int argc, char *argv[])
{
    std::string sudoku_filename{};
    std::string svg_filename{};
    using argparser = argparser::argparser;
    argparser opt(argc, argv);
    opt
        .pos([&sudoku_filename](std::string const &val)
             { sudoku_filename = val; })
        .pos([&svg_filename](std::string const &val)
             { svg_filename = val; });
    try
    {
        opt();
    }
    catch (::argparser::argument_required_exception const &e)
    {
        std::cerr << "\u001b[31;1mERROR:\u001b[0m " << e.what() << "\n\n";
        usage();
        return EXIT_FAILURE;
    }

    if (sudoku_filename.empty() || svg_filename.empty())
    {
        std::cerr << "\u001b[31;1mERROR:\u001b[0m A filename is missing.\n\n";
        usage();
        return EXIT_FAILURE;
    }

    std::cout << "Reading from " << sudoku_filename << " ...\n";
    std::ifstream fin{sudoku_filename};
    std::string data;
    std::string line;
    while (std::getline(fin, line))
    {
        data.append(line);
    }
    data = util::trim(data, " \t\n\r");
    if (data.length() != 81)
    {
        std::cerr << "Board data must contain exactly 81 digits.\n";
        return EXIT_FAILURE;
    }
    int cell_size = 40;
    int padding = cell_size / 10;
    std::string stroke_color = "#222";

    std::cout << "Writing SVG to " << svg_filename << " ...\n";
    std::ofstream fout{svg_filename};
    fout << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << (9 * cell_size + 2 * padding) << "\" height=\"" << (9 * cell_size + 2 * padding) << "\" version=\"1.1\">\n"
         << " <style>\n"
         << " text {\n"
         << "  font-family: \"Courier New\", Courier, monospace;\n"
         << "  font-size: " << (static_cast<float>(cell_size) / 1.618f) << "px;\n"
         << "  text-anchor: middle;\n"
         << "  dominant-baseline: middle;\n"
         << "  color: " << stroke_color << '\n'
         << " }\n"
         << " </style>\n"
         << " <g transform=\"translate(" << padding << " " << padding << ")\" stroke=\"" << stroke_color << "\">\n"
         << "  <rect x=\"0\" y=\"0\" width=\"" << (9 * cell_size) << "\" height=\"" << (9 * cell_size) << "\" fill=\"white\" />\n";
    for (int i = 0; i < 10; ++i)
    {
        float stroke_width = (i % 3) == 0
                                 ? 2.f
                                 : 0.5f;
        fout << "  <line stroke-width=\"" << stroke_width << "\" x1=\"" << (i * cell_size) << "\" y1=\"0\" x2=\"" << (i * cell_size) << "\" y2=\"" << (9 * cell_size) << "\"/>\n"
             << "  <line stroke-width=\"" << stroke_width << "\" x1=\"0\" y1=\"" << (i * cell_size) << "\" x2=\"" << (9 * cell_size) << "\" y2=\"" << (i * cell_size) << "\"/>\n";
    }
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            char cell = data.at(static_cast<size_t>(row * 9 + col));
            if (cell != '0')
            {
                fout << "  <text x=\"" << ((col + 0.5) * cell_size) << "\" y=\"" << ((row + 0.5) * cell_size) << "\">" << cell << "</text>" << std::endl;
            }
        }
    }
    fout << " </g>\n"
         << "</svg>\n";
    std::cout << "Ready.\n";
    return EXIT_SUCCESS;
}