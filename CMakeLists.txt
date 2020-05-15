cmake_minimum_required(VERSION 3.15)
project(compiledPCFG)

set(CMAKE_CXX_STANDARD 11)

add_executable(pcfg_guesser ./src/pcfg_guesser.c src/pcfg_guesser.h ./src/banner_info.c
        ./src/banner_info.h ./src/base_structure_io.c ./src/base_structure_io.h ./src/command_line.c
        ./src/command_line.h ./src/config_parser.c ./src/config_parser.h ./src/global_def.h
        ./src/grammar.h ./src/grammar_io.c ./src/grammar_io.h ./src/helper_io.c ./src/helper_io.h
        ./src/makefile  ./src/pcfg_pqueue.c ./src/pcfg_pqueue.h
        ./src/pqueue.c ./src/pqueue.h ./src/tty.c ./src/tty.h
        ./src/pwdstructmap.c ./src/pwdstructmap.h ./src/hashmap.c ./src/hashmap.h)
