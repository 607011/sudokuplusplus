cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(LIBCXX_ENABLE_INCOMPLETE_FEATURES)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -ggdb -DDEBUG -pedantic -Wall -Wextra -Wconversion -Wreorder -Wuninitialized -Wtype-limits -Wno-pragmas")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wno-deprecated")

project(sudoku)

add_executable(sudoku
  main.cpp
  sudoku.cpp
  util.cpp
)
set_target_properties(sudoku PROPERTIES LINK_FLAGS_RELEASE "-dead_strip")

add_executable(sudoku2svg
  sudoku2svg.cpp
)


if(UNIX)
  set(PLATFORM_DEPENDENT_LIBRARIES, "-lpthread")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread -O0 -ggdb")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread -O2")
else()
  set(PLATFORM_DEPENDENT_LIBRARIES, "")
endif()

add_custom_command(TARGET sudoku
  POST_BUILD
  COMMAND strip sudoku)

target_include_directories(sudoku
	PRIVATE ${PROJECT_INCLUDE_DIRS}
)

install(TARGETS sudoku RUNTIME DESTINATION bin)
