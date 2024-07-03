cmake_minimum_required(VERSION 3.5)
project(NetworkProgramming)

# setting C++ Standard
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-variable")
# setting source code directory
add_subdirectory(src)
include_directories("${PROJECT_SOURCE_DIR}/src")


find_package(Boost 1.71.0 REQUIRED COMPONENTS system filesystem)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(server server_asio.cpp)
    add_executable(client client_asio.cpp)
    target_link_libraries(server src)
    target_link_libraries(server ${Boost_LIBRARIES})
    target_link_libraries(client ${Boost_LIBRARIES})
    target_link_libraries(client src)
endif()
