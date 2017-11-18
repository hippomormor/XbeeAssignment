cmake_minimum_required(VERSION 3.5)
project(pii)

set(CMAKE_CXX_STANDARD 11)

include_directories(
        ~/pi/usr/local/include/
        inc/)

set(SOURCE_FILES
        main.cpp
        XBeeHandler.cpp
        Socket.cpp
        XBeeHandler.h
        colors.h)

add_executable(pii ${SOURCE_FILES})