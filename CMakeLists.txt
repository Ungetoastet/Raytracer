cmake_minimum_required(VERSION 3.10)
project(Raytracer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set optimization flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -Ofast -march=native")

# Collect header files
file(GLOB HEADER_FILES "*.h")

# Main executable
add_executable(main main.cpp ${HEADER_FILES})

# Add unit tests
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(tests ${TEST_SOURCES} tests/catch_amalgamated.cpp)

# Enable testing
enable_testing()
add_test(NAME RaytracerTests COMMAND tests)
