# Sudoku generator and solver

## Build

```
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Generate sudokus

```
./sudoku -d 62 -T 4
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
./sudoku < sudoku-20230318T160133-61.txt
```

Read Sudoku from stdin and solve it:

```
./sudoku <<<"007000000060000800000020031000032004805090000070006000501000000000500060000400070"
```


## Printable Sudokus

You can convert a Sudoku file to SVG with `sudoku2svg`, e.g.:

```
./sudoku2svg < sudoku-20230318T160133-61.txt > sudoku.svg
```
