cmake_minimum_required(VERSION 3.23)

project(Hydrogen)
set(CMAKE_CXX_STANDARD 20)

add_executable(hydro src/main.cpp src/tokenization.h
        src/parser.h
        src/generation.h
        src/arena.h)
