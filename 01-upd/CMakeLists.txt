cmake_minimum_required(VERSION 3.23)

project(01-udp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Executaboles
add_executable(sender sender.cpp)


target_compile_features(sender PRIVATE cxx_std_20)

#Compiler Flags
target_compile_options(sender PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

target_include_directories(sender PRIVATE ${CMAKE_SOURCE_DIR}/../shared)

# Experiment blaster
add_executable(sender_exp experiment/sender_exp.cpp)
target_compile_features(sender_exp PRIVATE cxx_std_20)
target_compile_options(sender_exp PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
target_include_directories(sender_exp PRIVATE ${CMAKE_SOURCE_DIR}/../shared)