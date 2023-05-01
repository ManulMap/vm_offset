cmake_minimum_required(VERSION 3.24)
project(vm_hook)

set(CMAKE_CXX_STANDARD 23)
add_library(vmh INTERFACE)
target_include_directories(vmh INTERFACE include/)

enable_testing()
add_subdirectory(test/)