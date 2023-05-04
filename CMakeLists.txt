cmake_minimum_required(VERSION 3.16)

project(sudoku
        VERSION 0.9
        DESCRIPTION "Sudoku Solver and Generator"
        LANGUAGES CXX)

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

set(BOOST_ROOT $ENV{BOOST_ROOT})
find_package(Boost 1.81.0 REQUIRED COMPONENTS program_options)
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost lib dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost libs: ${Boost_LIBRARIES}")

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
  PUBLIC ${Boost_INCLUDE_DIRS}
)

target_link_libraries(sudoku
	${Boost_LIBRARIES}
)

install(TARGETS sudoku RUNTIME DESTINATION bin)
