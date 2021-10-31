cmake_minimum_required(VERSION 3.21)

project(prime-gen)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 17)
list(APPEND FLAGS "${CMAKE_CXX_FLAGS}")
list(APPEND FLAGS "-Wall")
list(APPEND FLAGS "-Wshadow")
list(APPEND FLAGS "-Wpointer-arith")
list(APPEND FLAGS "-Wcast-qual")
list(APPEND FLAGS "-Wstrict-prototypes")
list(APPEND FLAGS "-Wmissing-prototypes")
list(APPEND FLAGS "-Werror=implicit-function-declaration")
list(JOIN FLAGS " " FLAGS)
set(CMAKE_C_FLAGS "${FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

include_directories(include)
add_subdirectory(src)

