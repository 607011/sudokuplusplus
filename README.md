# Sudoku generator

## Build

```
CMAKE_BUILD_TYPE=Release cmake .
cmake --build .
```

## Run

```
./sudoku [difficulty]
```

where `difficulty` is an integer number between 1 and 6, 6 being hardest.

Each Sudoku found will be written to a text file named like sudoku-[ISO8601DateTime]-[difficulty].txt with a contents like (`0` denotes an empty field):

```
780006090
045000001
020350000
003000407
070805000
000000809
010000308
000507000
007900010
```
