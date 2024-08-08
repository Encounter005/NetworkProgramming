cmake_minimum_required(VERSION 3.5)
project(NetworkProgramming)

# setting C++ Standard
set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -O2  -Wall -Wextra -Weffc++ -Werror=uninitialized  -Werror=return-type -Wconversion -Wsign-compare -Werror=unused-result -Werror=suggest-override -Wzero-as-null-pointer-constant -Wmissing-declarations -Wold-style-cast -Wnon-virtual-dtor -Wl,--copy-dt-needed-entries"
)


find_package(Boost 1.71.0 REQUIRED COMPONENTS system filesystem)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

if(Boost_FOUND)
# setting source code directory
add_subdirectory(src)
add_subdirectory(executable)
add_subdirectory(proto)
include_directories(${PROTO_GEN_CXX_INCLUDE_DIRS})

endif()
