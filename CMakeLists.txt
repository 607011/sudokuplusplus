cmake_minimum_required(VERSION 3.16)

project(sudoku
        VERSION 0.9
        DESCRIPTION "Sudoku Solver and Generator"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Curses REQUIRED)
include_directories()

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
  src/util.cpp
)

if(UNIX)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET sudoku
      POST_BUILD
      COMMAND strip sudoku)
  endif()
endif(UNIX)

if(UNIX)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET sudoku2svg
      POST_BUILD
      COMMAND strip sudoku2svg)
  endif()
endif(UNIX)

target_include_directories(sudoku
	PRIVATE ${PROJECT_INCLUDE_DIRS}
  3rdparty/getopt-cpp/include
  PUBLIC
  ${CURSES_INCLUDE_DIR}
)

target_include_directories(sudoku2svg
	PRIVATE ${PROJECT_INCLUDE_DIRS}
  3rdparty/getopt-cpp/include
)


target_link_libraries(sudoku ${CURSES_LIBRARY})

install(TARGETS sudoku RUNTIME DESTINATION bin)


enable_testing()

add_executable(test_sudoku
  src/sudoku.cpp
  src/util.cpp
  tests/test_sudoku.cpp
)
target_link_libraries(
  test_sudoku
  GTest::gtest_main
)

add_executable(test_set
  tests/test_set.cpp
)
target_link_libraries(
  test_set
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_sudoku)
gtest_discover_tests(test_set)
