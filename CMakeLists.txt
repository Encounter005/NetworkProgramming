cmake_minimum_required(VERSION 3.5)
project(NetworkProgramming)

# setting compiler
set(CMAKE_CXX_COMPILER "g++")
# setting C++ Standard
set(CMAKE_CXX_STANDARD 20)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -fcoroutines -O2 -Wall -Wextra -Weffc++ -Werror=uninitialized  -Werror=return-type -Wconversion -Wsign-compare -Werror=unused-result -Werror=suggest-override -Wzero-as-null-pointer-constant -Wmissing-declarations -Wold-style-cast -Wnon-virtual-dtor -Wl,--copy-dt-needed-entries"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")
message(STATUS "BOOST_SYSTEM: ${BOOST_SYSTEM}")
message(STATUS "BOOST_FILE_SYSTEM: ${BOOST_FILE_SYSTEM}")

# setting source code directory
add_subdirectory(src)
add_subdirectory(executable)
add_subdirectory(proto)
include_directories(${PROTO_GEN_CXX_INCLUDE_DIRS})
