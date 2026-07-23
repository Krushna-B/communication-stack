cmake_minimum_required(VERSION 3.23)

project(00-bytes-and-encoding)

#Executaboles
add_executable(main main.cpp)


target_compile_features(main PRIVATE cxx_std_20)

#Compiler Flags
target_compile_options(main PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
