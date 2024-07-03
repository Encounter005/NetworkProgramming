cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)

aux_source_directory(. SOURCE)
add_library(src ${SOURCE})
