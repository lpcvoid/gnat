cmake_minimum_required(VERSION 3.21)
project(mosquitto_cpp_wrapper)

set(CMAKE_CXX_STANDARD 20)

add_executable(mosquitto_cpp_wrapper main.cpp gnat.hpp)
target_link_libraries(mosquitto_cpp_wrapper mosquitto)
