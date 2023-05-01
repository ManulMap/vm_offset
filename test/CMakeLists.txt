cmake_minimum_required(VERSION 3.24)

include(CTest)

add_executable(no_inh no_inh.cc)

include_directories(PRIVATE ../include)

add_test(NAME no_inheritance COMMAND no_inh)