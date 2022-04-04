#Author Zhivko Petrov

cmake_minimum_required(VERSION 3.5)

project(
    resource_builder
    LANGUAGES CXX
    VERSION 1.0.0
)

find_package(cmake_helpers REQUIRED)
find_package(resource_utils REQUIRED)

set(_INC_FOLDER_NAME include)
set(_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${_INC_FOLDER_NAME}/${PROJECT_NAME})
set(_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(
    ${PROJECT_NAME} 
        ${_INC_DIR}/FileBuilder.h
        ${_INC_DIR}/FileParser.h
        ${_INC_DIR}/ResourceParser.h
        ${_INC_DIR}/SyntaxChecker.h
    
        ${_SRC_DIR}/FileBuilder.cpp
        ${_SRC_DIR}/FileParser.cpp
        ${_SRC_DIR}/main.cpp
        ${_SRC_DIR}/ResourceParser.cpp
        ${_SRC_DIR}/SyntaxChecker.cpp
)

target_include_directories(
    ${PROJECT_NAME} 
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${_INC_FOLDER_NAME}>
        $<INSTALL_INTERFACE:${_INC_FOLDER_NAME}>
)

target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE 
        resource_utils::resource_utils
)

if(NOT DISABLE_ROS_TOOLING)
    # Ament uses non-monolith build (separate build and install steps).
    # The helpers.cmake has to be included manually.
    # This allows access to the helpers.cmake user defined macros
    # In the case of monolith build - the file is automatically included
    # from the cmake_helpersConfig.cmake when the package is consumed by find_package()
	include(${cmake_helpers_DIR}/helpers.cmake)
	
    enable_ros_tooling_for_target(
        ${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/package.xml
    )
endif()

set_target_cpp_standard(${PROJECT_NAME} 20)
enable_target_warnings(${PROJECT_NAME})
set_target_visibility(${PROJECT_NAME})
install_and_export_target(${PROJECT_NAME} ${_INC_FOLDER_NAME})

#invoke include-what-you-use (if it's enabled) for the current project
if(USE_IWYU)
    enable_target_include_what_you_use(${PROJECT_NAME})
endif(USE_IWYU)

#generate symbolic link for resourceBuilder after build is ready
#also add the -f (--force) command in order to
#override the existing symbolic link (if any)
# add_custom_command(
#     TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND
#     ln -sf tools/resource_builder/${PROJECT_NAME} resourceBuilderSymLink)
#            
# #move symbolic link to project build directory
# add_custom_command(
#     TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND
#     mv resourceBuilderSymLink ${CMAKE_SOURCE_DIR}/build/res_builder)



