cmake_minimum_required(VERSION 3.23)

project(01-udp)

#Executaboles
add_executable(sender sender.cpp)


target_compile_features(sender PRIVATE cxx_std_20)

#Compiler Flags
target_compile_options(sender PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

target_include_directories(sender PRIVATE ${CMAKE_SOURCE_DIR}/../shared)