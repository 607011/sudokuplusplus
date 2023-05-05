# Sudoku generator and solver

## Build

### Linux (Ubuntu)

#### Prerequisites

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
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
```

This will generate `sudoku.exe` and `sudoku2svg.exe`.


## Generate sudokus

```
sudoku -d [difficulty] -T [thread_count]
```

where `difficulty` is an integer number between 25 and 64, meaning fields left empty. 64 is hardest possible.
`thread_count` determines the number of games being generated concurrently. If not given, the number of cores will be used.

Each Sudoku found will be written to a text file named like sudoku-[ISO8601DateTime]-[difficulty] [seq_no].txt with a contents like (`0` denotes an empty field):

```
007000000060000800000020031000032004805090000070006000501000000000500060000400070
```

## Solve sudokus

Read Sudoku from file and solve it:

```
./sudoku --solve sudoku-20230318T160133-61.txt
```

## Printable Sudokus

You can convert a Sudoku file to SVG with `sudoku2svg`, e.g.:

```
./sudoku2svg < sudoku-20230318T160133-61.txt > sudoku.svg
```
