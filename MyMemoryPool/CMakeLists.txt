cmake_minimum_required(VERSION 3.25)
project(MemoryPool VERSION 0.0.1 LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Release") 

add_executable(memory_pool_benchmark benchmark.cpp)
target_link_libraries(memory_pool_benchmark Threads::Threads)