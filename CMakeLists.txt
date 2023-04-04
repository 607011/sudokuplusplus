cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

if(UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "-pthread -O0 -g3 -ggdb -DDEBUG -pedantic -Wall -Wextra -Wconversion -Wreorder -Wuninitialized -Wtype-limits -Wno-pragmas")
  set(CMAKE_CXX_FLAGS_RELEASE "-pthread -O3 -DNDEBUG -Wno-deprecated")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS_DEBUG "/Od /Wall /Zi /wd5045 /wd4514 /wd4820 /D_CRT_SECURE_NO_WARNINGS /D__STDC_WANT_SECURE_LIB__")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /W1 /DNDEBUG /D_CRT_SECURE_NO_WARNINGS /D__STDC_WANT_SECURE_LIB__")
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
