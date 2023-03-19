cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

if(UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb -DDEBUG -pedantic -Wall -Wextra -Wconversion -Wreorder -Wuninitialized -Wtype-limits -Wno-pragmas")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wno-deprecated")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS_DEBUG "/Od /Wall /Zi")
  set(CMAKE_CXX_FLAGS_RELEASE "/O3 /W1 /DNDEBUG")
endif(UNIX)

project(sudoku)

add_executable(sudoku
  main.cpp
  sudoku.cpp
  util.cpp
)

if(UNIX)
  set_target_properties(sudoku PROPERTIES LINK_FLAGS_RELEASE "-dead_strip")
endif(UNIX)

add_executable(sudoku2svg
  sudoku2svg.cpp
)

if(UNIX)
add_custom_command(TARGET sudoku
  POST_BUILD
  COMMAND strip sudoku)
endif(UNIX)

target_include_directories(sudoku
	PRIVATE ${PROJECT_INCLUDE_DIRS}
)

install(TARGETS sudoku RUNTIME DESTINATION bin)
