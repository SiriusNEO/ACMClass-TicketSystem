cmake_minimum_required(VERSION 3.16)
project(code)

set(CMAKE_CXX_STANDARD 14)

include_directories(db)
include_directories(lib)
include_directories(src)

add_executable(code
        db/allcachedmanager.hpp
        db/bpt.hpp
        db/hash_map.h
        lib/bst.hpp
        lib/hashtable.hpp
        lib/mytools.hpp
        lib/timetype.hpp
        src/cmdprocessor.hpp
        src/main.cpp
        src/systemcore.hpp)
