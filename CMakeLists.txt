cmake_minimum_required(VERSION 3.5)
project(NetworkProgramming)

# setting C++ Standard
set(CMAKE_CXX_STANDARD 17)
# setting source code directory
set(SOURCE_CODE_DIR "${PROJECT_SOURCE_DIR}/src")



find_package(Boost 1.71.0 REQUIRED COMPONENTS system filesystem)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_subdirectory(src)
    add_definitions("-g")
    add_executable(NetworkProgramming main.cpp)
    target_link_libraries(NetworkProgramming ${Boost_LIBRARIES})
endif()
