project(verde)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/verde/)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

add_subdirectory(yaml-cpp)
add_subdirectory(verde)
add_subdirectory(verde-demo)
