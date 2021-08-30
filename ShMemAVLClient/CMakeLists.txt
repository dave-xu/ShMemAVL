# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (ShMemAVLClient)

#set(CMAKE_BUILD_TYPE Debug)

# Create a sources variable with a link to all cpp files to compile
set(SOURCE_FILES
    src/ShMemAVLClient.cpp
)

set(HEADER_FILES
	include/ShMemAVLClient.h
)

set(COMMON_INCLUDE
        ${PROJECT_SOURCE_DIR}../../ShMemUtils/include
        ${PROJECT_SOURCE_DIR}../../ShMemAVL/include
)

# Add an executable with the above sources
add_executable(ShMemAVLClient 
	${SOURCE_FILES}
    ${HEADER_FILES}
)

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(ShMemAVLClient
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${COMMON_INCLUDE}
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# link the new library target with the binary target
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux") 
target_link_libraries(ShMemAVLClient
    PRIVATE 
    ShMemUtils
    ShMemAVL
    Threads::Threads
    rt)
else()
target_link_libraries(ShMemAVLClient
    PRIVATE 
    ShMemUtils
    ShMemAVL)
endif()
