cmake_minimum_required(VERSION 3.10)
project(Trabalho9)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Raylib
find_package(raylib)

# Find all source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp")

# Add executable
add_executable(Trabalho9 ${SOURCES})

# Include directories
target_include_directories(Trabalho9 PUBLIC . include src)

# Link with Raylib
if(raylib_FOUND)
    target_link_libraries(Trabalho9 raylib)
else()
    # If raylib is not found as a package, try to link it manually
    # This might require adjusting the path to the library
    target_link_libraries(Trabalho9 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11)
endif()
