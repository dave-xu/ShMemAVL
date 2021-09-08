# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
project (ShMemAVLServer)

#set(CMAKE_BUILD_TYPE Debug)

# Create a sources variable with a link to all cpp files to compile
set(SOURCE_FILES
    src/ShMemAVLServer.cpp
)

set(COMMON_INCLUDE
        ${PROJECT_SOURCE_DIR}/../ShMemUtils/include
        ${PROJECT_SOURCE_DIR}/../ShMemAVL/include
)

# Add an executable with the above sources
add_executable(ShMemAVLServer 
	${SOURCE_FILES}
)

# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(ShMemAVLServer
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${COMMON_INCLUDE}
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# link the new library target with the binary target
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux") 
target_link_libraries(ShMemAVLServer
    PRIVATE 
    ShMemUtils
    ShMemAVL
    Threads::Threads
    rt)
else()
target_link_libraries(ShMemAVLServer
    PRIVATE 
    ShMemUtils
    ShMemAVL)
endif()

add_executable(ServerLauncher
    src/ServerLauncher.cpp
)

target_include_directories(ServerLauncher
    PRIVATE
        ${COMMON_INCLUDE}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux") 
target_link_libraries(ServerLauncher
    PRIVATE 
    ShMemUtils
    Threads::Threads
    rt)
else()
target_link_libraries(ServerLauncher
    PRIVATE 
    ShMemUtils)
endif()
