#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

project(libsel4vchan C)

add_compile_options(-std=gnu99)

file(GLOB sources src/*.c)

add_library(sel4vchan STATIC EXCLUDE_FROM_ALL ${sources})
target_include_directories(sel4vchan PUBLIC include)
target_link_libraries(sel4vchan muslc sel4 sel4simple utils sel4utils sel4vka sel4vspace sel4dma)
