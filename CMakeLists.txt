#Author Zhivko Petrov
cmake_minimum_required(VERSION 3.10.2)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake_helpers/helpers.cmake)
set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_helpers/find_modules)

add_subdirectory(utils)
add_subdirectory(resource_utils)
add_subdirectory(resource_builder)



