cmake_minimum_required(VERSION 3.9)
project(approx LANGUAGES C)

add_executable(approx test.c approx.c)
target_compile_options(approx PRIVATE -Wall -Wextra -pedantic -O3)

find_package(OpenMP REQUIRED)
target_link_libraries(approx PRIVATE OpenMP::OpenMP_C m)
