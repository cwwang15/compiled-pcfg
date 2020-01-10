cmake_minimum_required(VERSION 3.15)
project(compiledPCFG)

set(CMAKE_CXX_STANDARD 11)

add_executable(pcfg_guesser ./src/pcfg_guesser.c)
