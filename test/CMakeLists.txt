cmake_minimum_required(VERSION 3.24)

include(CTest)

add_executable(no_inh no_inh.cc)

add_executable(single_inh single_inh.cc)

add_executable(fail_mul_inh fail_mul_inh.cc)

add_executable(fail_virt_inh fail_virt_inh.cc)


include_directories(PRIVATE ../include)


set_target_properties(fail_mul_inh fail_virt_inh PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        EXCLUDE_FROM_DEFAULT_BUILD TRUE)


add_test(NAME no_inheritance COMMAND no_inh)

add_test(NAME single_inheritance COMMAND single_inh)

add_test(NAME fail_multiple_inheritance
        COMMAND ${CMAKE_COMMAND} --build vm_offset --target fail_mul_inh --config $<CONFIGURATION>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_test(NAME fail_virtual_inheritance
        COMMAND ${CMAKE_COMMAND} --build vm_offset --target fail_virt_inh --config $<CONFIGURATION>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

set_tests_properties(fail_multiple_inheritance fail_virtual_inheritance
                    PROPERTIES WILL_FAIL TRUE)