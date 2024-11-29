cmake_minimum_required(VERSION 3.10)

project(MyBenchmark)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable testing of the benchmark library." FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Enable installation of benchmark. (Projects embedding benchmark may want to turn this OFF.)" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Enable building the unit tests which depend on gtest" FORCE)
add_subdirectory(benchmark EXCLUDE_FROM_ALL)

add_executable(integrate integral.cpp)

target_link_libraries(integrate PRIVATE benchmark::benchmark)
