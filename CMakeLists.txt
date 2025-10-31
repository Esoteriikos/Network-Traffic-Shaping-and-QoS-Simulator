cmake_minimum_required(VERSION 3.10)
project(NetworkTrafficSimulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(network_sim ${SOURCES})

# Link pthread for Linux/Mac
if(UNIX)
    target_link_libraries(network_sim pthread)
endif()

# Set output directory
set_target_properties(network_sim PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
