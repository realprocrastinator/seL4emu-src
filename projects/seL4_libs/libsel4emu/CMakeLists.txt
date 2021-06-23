# The goal of this lib is to make itself linked with `minilibc` which is a copy of musllibc but with different symbol names and implements the minimal stdlib functionalities!

cmake_minimum_required(VERSION 3.7.2)

project(libsel4emu C)

message("")
message("CMake says Hello from seL4emu lib!")
message("")

# We make an attempt to extract compiler options from CMake
get_property(compile_options DIRECTORY PROPERTY COMPILE_OPTIONS)
message("Compile with options: ${compile_options}")
add_compile_options(${compile_options})

add_library(sel4emu STATIC src/sel4emu.c)
target_include_directories(
    sel4emu
    PUBLIC 
      include
)

add_dependencies(sel4emu muslc minimusllibc)
target_link_libraries(sel4emu PUBLIC muslc minimusllibc)
