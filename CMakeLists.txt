cmake_minimum_required(VERSION 3.10)

project(beebasm)

add_compile_options(-Wall -W -Wcast-qual -Wshadow -Wcast-align -Wold-style-cast -Woverloaded-virtual)

# Existing Makefile does a glob to find source files, so we do the same.
FILE(GLOB CPPSources src/*.cpp)

add_executable(beebasm ${CPPSources})

install(TARGETS beebasm DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/beebasm.1 DESTINATION share/man/man1)

enable_testing()

add_test(NAME Runs COMMAND ./beebasm -i demo.6502 -do demo.ssd -boot Code -v)
