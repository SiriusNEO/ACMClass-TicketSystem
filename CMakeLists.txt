cmake_minimum_required(VERSION 3.16)
project(TicketSystem_2021_main)

set(CMAKE_CXX_STANDARD 14)

add_executable(TicketSystem_2021_main
               src/main.cpp src/cmdprocessor.hpp src/systemcore.hpp
               lib/mytool.hpp lib/hashtable.hpp lib/timetype.hpp
               db/blocklist.h db/filemanager.hpp lib/mytools.hpp)