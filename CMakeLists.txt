cmake_minimum_required(VERSION 3.7...3.26)

# Allow IDE Source Tree Visualization
if(${CMAKE_VERSION} VERSION_LESS 3.26)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
    cmake_policy(SET CMP0143 NEW)
endif()

project(Sirius)

add_subdirectory(Sirius)
