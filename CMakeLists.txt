cmake_minimum_required(VERSION 3.0.0)

set(proj sysprog)
project(${proj} C)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
# set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/apue.h)

file(GLOB TARGET_SRC "./src/coro/*.c", "./src/coro/*.h" )

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lrt")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")

add_executable(${proj} ${TARGET_SRC})