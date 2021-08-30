# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (ShMemAVLLib)

#set(CMAKE_BUILD_TYPE Debug)

set(COMMON_INCLUDE
        ${PROJECT_SOURCE_DIR}../../ShMemUtils/include
)

# Create a sources variable with a link to all cpp files to compile
set(SOURCE_FILES
    src/ShMemAVLTree.cpp
)

set(HEADER_FILES
	include/ShMemAVLTree.h
)

# Add an executable with the above sources
add_library(ShMemAVL STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(ShMemAVL
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${COMMON_INCLUDE}
)