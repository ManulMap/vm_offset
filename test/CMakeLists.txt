cmake_minimum_required(VERSION 3.24)

include(CTest)

add_executable(no_inh no_inh.cc)

add_executable(single_inh single_inh.cc)

include_directories(PRIVATE ../include)


add_test(NAME no_inheritance COMMAND no_inh)

add_test(NAME single_inheritance COMMAND single_inh)