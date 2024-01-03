cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_BUILD_TYPE "Debug")

add_executable(main main.cpp)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Threads REQUIRED)

target_link_libraries(main PRIVATE Threads::Threads)

