cmake_minimum_required(VERSION 3.22)
project(test_package C)

find_package(VLasagna REQUIRED)

add_executable(test_package test_package.c)
target_link_libraries(test_package PRIVATE VLasagna::Core)