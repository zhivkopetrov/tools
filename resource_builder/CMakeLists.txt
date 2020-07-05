#Author Zhivko Petrov

cmake_minimum_required(VERSION 3.10.2)

project(resource_builder)

set(_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
   
add_executable(
    ${PROJECT_NAME} 
        "${_INCLUDE_DIR}/FileBuilder.h"
        "${_INCLUDE_DIR}/FileParser.h"
        "${_INCLUDE_DIR}/ResourceParser.h"
        "${_INCLUDE_DIR}/SyntaxChecker.h"
    
        "${_SRC_DIR}/FileBuilder.cpp"
        "${_SRC_DIR}/FileParser.cpp"
        "${_SRC_DIR}/main.cpp"
        "${_SRC_DIR}/ResourceParser.cpp"
        "${_SRC_DIR}/SyntaxChecker.cpp"
)

set_target_cpp_standard(${PROJECT_NAME} 17)
enable_target_warnings(${PROJECT_NAME})
set_target_visibility(${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${_INCLUDE_DIR}
)

target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE
        resource_utils
)

#invoke include-what-you-use (if it's enabled) for the current project
if(USE_IWYU)
    enable_target_include_what_you_use(${PROJECT_NAME})
endif(USE_IWYU)


## TODO think of a better way! Maybe export/install/import?

#generate symbolic link for resourceBuilder after build is ready
#also add the -f (--force) command in order to
#override the existing symbolic link (if any)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    ln -sf tools/resource_builder/${PROJECT_NAME} resourceBuilderSymLink)
           
#move symbolic link to project build directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
    mv resourceBuilderSymLink ${CMAKE_SOURCE_DIR}/build/res_builder)



