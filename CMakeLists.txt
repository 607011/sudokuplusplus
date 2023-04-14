cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

if(UNIX)
  set(CMAKE_CXX_FLAGS "-pthread")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g3 -ggdb -DDEBUG -pedantic -Wall -Wextra -Wconversion -Wreorder -Wuninitialized -Wtype-limits -Wno-pragmas")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -Wno-deprecated")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS_DEBUG "/Od /Wall /Zi /wd5045 /wd4514 /wd4820 /D_CRT_SECURE_NO_WARNINGS /D__STDC_WANT_SECURE_LIB__")
  set(CMAKE_CXX_FLAGS_RELEASE "/O2 /W1 /DNDEBUG /D_CRT_SECURE_NO_WARNINGS /D__STDC_WANT_SECURE_LIB__")
endif(UNIX)

if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -glldb")
endif()

project(sudoku)

add_executable(sudoku
  src/main.cpp
  src/sudoku.cpp
  src/util.cpp
)

if(UNIX)
  set_target_properties(sudoku PROPERTIES LINK_FLAGS_RELEASE "-dead_strip")
endif(UNIX)

add_executable(sudoku2svg
  src/sudoku2svg.cpp
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
