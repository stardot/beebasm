cmake_minimum_required(VERSION 3.10)

project(beebasm LANGUAGES CXX)

add_compile_options(-Wall -W -Wcast-qual -Wshadow -Wcast-align -Wold-style-cast -Woverloaded-virtual)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Existing Makefile does a glob to find source files, so we do the same.
FILE(GLOB CPPSources src/*.cpp)

add_executable(beebasm ${CPPSources})
target_link_libraries(beebasm stdc++ m)

install(TARGETS beebasm DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/beebasm.1 DESTINATION share/man/man1)

enable_testing()

add_test(NAME Runs COMMAND ./beebasm -i ${CMAKE_SOURCE_DIR}/demo.6502 -do demo.ssd -boot Code -v)
add_test(NAME Tests COMMAND python3 ${CMAKE_SOURCE_DIR}/test/testrunner.py -v)
