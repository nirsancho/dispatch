cmake_minimum_required(VERSION 2.8.9)
project (dispatch)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(dispatch)
add_subdirectory(tests)