include(CMakeFindDependencyMacro)

#find_dependency will correctly forward REQUIRED or QUIET args to the consumer
#find_package is only for internal use
find_dependency(cmake_helpers REQUIRED)
find_dependency(resource_utils REQUIRED)

if(NOT TARGET resource_builder::resource_builder)
  include(${CMAKE_CURRENT_LIST_DIR}/resource_builderTargets.cmake)
endif()

# This is for catkin compatibility.
set(resource_builder_LIBRARIES resource_builder::resource_builder)

get_target_property(
    resource_builder_INCLUDE_DIRS
    resource_builder::resource_builder
    INTERFACE_INCLUDE_DIRECTORIES
)

