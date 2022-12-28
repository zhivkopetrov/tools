#Author Zhivko Petrov
cmake_minimum_required(VERSION 3.10.2)

project(tools)

set(resource_builder_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource_builder)

# executables
add_subdirectory(${resource_builder_DIR})
