cmake_minimum_required(VERSION 3.23)

project(02-tcp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(client client.cpp)
target_compile_features(client PRIVATE cxx_std_20)
target_compile_options(client PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/../shared)

add_executable(client_exp experiment/client_exp.cpp)
target_compile_features(client_exp PRIVATE cxx_std_20)
target_compile_options(client_exp PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
target_include_directories(client_exp PRIVATE ${CMAKE_SOURCE_DIR}/../shared)
