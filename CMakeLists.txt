cmake_minimum_required(VERSION 3.6)
project(smartoffice_srv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lboost_system -lboost_regex -g3")

set(SOURCE_FILES main.cpp smartoffice-srv.h)
add_executable(smartoffice_srv ${SOURCE_FILES})