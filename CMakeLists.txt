cmake_minimum_required(VERSION 3.5)
project(pii)

set(CMAKE_CXX_STANDARD 11)

include_directories(
        ~/pi/usr/local/include/)

set(SOURCE_FILES
        main.cpp
        XBeeHandler.cpp
        XBeeHandler.h
        colors.h)

add_executable(pii ${SOURCE_FILES})