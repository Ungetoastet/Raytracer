cmake_minimum_required(VERSION 3.10)
project(Raytracer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -Ofast -march=native")

# Collect header files
file(GLOB HEADER_FILES "Include/*.h")

# Main executable
add_executable(main main.cpp ${HEADER_FILES})

# Add unit tests
file(GLOB TEST_SOURCES "Tests/*.cpp")
add_executable(tests ${TEST_SOURCES} Tests/catch_amalgamated.cpp)
