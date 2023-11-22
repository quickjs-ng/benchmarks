cmake_minimum_required(VERSION 3.9)

project(quickjs_bench LANGUAGES C)

add_subdirectory(quickjs)
link_libraries(qjs)

add_executable(run_sunspider_like run_sunspider_like.c)
add_executable(run_octane run_octane.c)