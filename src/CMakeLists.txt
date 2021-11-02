set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_library(bn STATIC ../include/bn.h bn.c bn_word.c bn_shift.c bn_conv.c bn_print.c bn_div.c consts.c bn_mod.c bn_add.c bn_mul.c bn_sqr.c)
add_executable(prime-gen main.c)

target_link_libraries(prime-gen bn)
