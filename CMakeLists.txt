cmake_minimum_required(VERSION 3.15)
project(code)

set(CMAKE_CXX_STANDARD 14)

add_definitions(-O2)

include_directories(db)
include_directories(lib)
include_directories(src)

add_executable(code
        db/bpt.hpp
        db/hash_map.h
        lib/bst.hpp
        lib/mytools.hpp
        lib/timetype.hpp
        src/cmdprocessor.hpp
        src/main.cpp
        src/systemcore.hpp)
