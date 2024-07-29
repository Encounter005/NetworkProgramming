cmake_minimum_required(VERSION 3.5)
project(NetworkProgramming)

# setting C++ Standard
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -O2  -Wall -Wextra -Weffc++
-Werror=uninitialized
-Werror=return-type
-Wconversion -Wsign-compare
-Werror=unused-result
-Werror=suggest-override
-Wzero-as-null-pointer-constant
-Wmissing-declarations
-Wold-style-cast
-Wnon-virtual-dtor   "
)
# setting source code directory
add_subdirectory(src)
add_subdirectory(executable)
