# Sudoku++

**Sudoku generator and solver**

The generator works with a variety of algorithms to produce Sudokus:

`prefill`: This generator first fills three independent 3x3 boxes with random numbers. Then it solves the board. For each solution the generator tries to clear as many cells as requested. If enough cells could be cleared the board is valid, otherwise disposed of.

`prefill-single`: Same as `prefill`, but only the first solution found is used.

`mincheck`: This generator produces valid minimal boards with the specified number of empty cells. Each board is checked if it has one clear solution. If there's no clear solution, the process repeats.

## Build

### Linux (Ubuntu)

#### Prerequisites

- C++ compiler with support for C++20, especially `std::ranges`.

In terminal:

```
sudo apt install git cmake g++
```

There's probably more to install depending on your distribution.

#### Actual build

In terminal:

```
git clone https://github.com/607011/sudokuplusplus.git sudoku++
cd sudoku++
git submodule init
git submodule update --remote --merge
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### macOS (with Homebrew)

#### Prerequisites

In terminal:

```
brew install cmake git
```

#### Actual build

In terminal:

```
https://github.com/607011/sudokuplusplus.git sudoku++
cd sudoku++
git submodule init
git submodule update --remote --merge
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Windows (tested on Windows 11)

#### Prerequisites

In PowerShell or Command Prompt:

```
winget install Git.Git
winget install Kitware.CMake
winget install Microsoft.VisualStudio.2022.Community
```

#### Actual build

In [Visual Studio Developer Command Prompt](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022):

```
https://github.com/607011/sudokuplusplus.git sudoku++
cd sudoku++
git submodule init
git submodule update --remote --merge
md build
cd build
cmake ..
cmake --build . --config Release
```

This will generate `sudoku.exe` and `sudoku2svg.exe`.


## Generate sudokus

```
sudoku -d [N] -T [thread_count]
```

where `N` is an integer number between 25 and 64, meaning fields left empty. `thread_count` determines the number of games being generated concurrently. If not given, the number of CPU cores will be used.

Each Sudoku found will be written to a text file named like sudoku-[ISO8601DateTime]-[difficulty] [seq_no].txt with a contents like (`0` denotes an empty field):

```
007000000060000800000020031000032004805090000070006000501000000000500060000400070
```

## Solve sudokus

Read Sudoku from file and solve it:

```
./sudoku --solve-file sudoku-20230318T160133-61.txt
```

Read Sudoku from command line and solve it:

```
./sudoku --solve 007000000060000800000020031000032004805090000070006000501000000000500060000400070
```

## Printable Sudokus

You can convert a Sudoku file to SVG with `sudoku2svg`, e.g.:

```
./sudoku2svg sudoku-20230318T160133-61.txt sudoku.svg
```


## Work In Progress

I'm working on a human-like solver that tries to solve a Sudoku game by applying a variety of techniques like "obvious/hidden singles/pairs", "pointing pairs", "X-wing" an so on. That'll be a lot of work …

