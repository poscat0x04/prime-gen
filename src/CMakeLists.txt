set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_library(bn STATIC bn.c)
add_executable(prime-gen main.c)

target_link_libraries(prime-gen bn)
