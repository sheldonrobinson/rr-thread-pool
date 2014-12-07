cmake_minimum_required(VERSION 2.6)
project(rr-thread-pool)

ADD_DEFINITIONS(-std=c++11)

file(GLOB SOURCES *.cpp *.h)
include_directories(BEFORE .)
add_executable(unittest ${SOURCES})


