# Sudoku generator and solver

## Build

```
CMAKE_BUILD_TYPE=Release cmake .
cmake --build .
```

## Generate sudokus

```
./sudoku [difficulty]
```

where `difficulty` is an integer number between 1 and 6, 6 being hardest.

Each Sudoku found will be written to a text file named like sudoku-[ISO8601DateTime]-[difficulty].txt with a contents like (`0` denotes an empty field):

```
780006090045000001020350000003000407070805000000000809010000308000507000007900010
```

## Solve sudokus

Read Sudoku from file and solve it:

```
./sudoku < sudoku-20230318T160133-3.txt
```

Read Sudoku from stdin and solve it:

```
./sudoku <<<"780006090045000001020350000003000407070805000000000809010000308000507000007900010"
```


## Printable Sudokus

You can convert a Sudoku file to SVG with `sudoku2svg`, e.g.:

```
./sudoku2svg < sudoku-20230318T160133-3.txt > sudoku.svg
```
