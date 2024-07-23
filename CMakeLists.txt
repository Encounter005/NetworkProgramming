cmake_minimum_required(VERSION 3.5)
project(NetworkProgramming)

# setting C++ Standard
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-variable")
# setting source code directory
add_subdirectory(src)
add_subdirectory(executable)
