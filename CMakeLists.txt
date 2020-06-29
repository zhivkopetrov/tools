#Author Zhivko Petrov
cmake_minimum_required(VERSION 3.10.2)

if(NOT CMAKE_HELPERS_INCLUDED)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake_helpers/helpers.cmake)
    set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_helpers/find_modules")
endif()

if(NOT TARGET utils)
    add_subdirectory(utils)
endif()
if(NOT TARGET resource_utils)
    add_subdirectory(resource_utils)
endif()

add_subdirectory(resource_builder)



