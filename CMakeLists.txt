cmake_minimum_required(VERSION 3.16)
project(TicketSystem_2021_main)

set(CMAKE_CXX_STANDARD 14)

add_executable(TicketSystem_2021_main src/main.cpp src/cmdprocessor.hpp src/systemcore.hpp lib/toolfunctions.hpp lib/fixedstr.hpp lib/hashtable.hpp lib/linklist.hpp lib/timetype.hpp)