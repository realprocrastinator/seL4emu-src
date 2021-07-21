# The goal of this lib is to make itself linked with `dl` which depends on gcc without messing things up!

# TODO: 1) make ninja understand the change we made in the src code
#       2) generate files directly to the build folders instead of copying

cmake_minimum_required(VERSION 3.7.2)

project(libminimusllibc C)

message("")
message("CMake says Hello from libminimusllibc lib!")
message("")

# We make an attempt to extract compiler options from CMake
get_property(compile_options DIRECTORY PROPERTY COMPILE_OPTIONS)
message("Compile with options: ${compile_options}")

message("Makefile is at: ${CMAKE_CURRENT_SOURCE_DIR}/Makefile")

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/build/libminimusllibc.a
    COMMAND
        ${CMAKE_COMMAND} -E env
        NK_CFLAGS="${compile_options}"
        make -f "${CMAKE_CURRENT_SOURCE_DIR}/Makefile"
        "SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        "BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND
        cp -r ${CMAKE_CURRENT_BINARY_DIR}/build/libminimusllibc.a ${CMAKE_CURRENT_BINARY_DIR}/libminimusllibc.a
    COMMAND
        cp -r ${CMAKE_CURRENT_BINARY_DIR}/build/include ${CMAKE_CURRENT_BINARY_DIR}/
)

add_custom_target(
    minimuslc_gen ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/build/libminimusllibc.a
)

add_library(minimusllibc_imported STATIC IMPORTED)
add_dependencies(minimusllibc_imported minimuslc_gen)
set_property(
    TARGET minimusllibc_imported
    PROPERTY IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/libminimusllibc.a"
)

add_library(minimusllibc INTERFACE)
add_dependencies(minimusllibc minimusllibc_imported)
set_property(TARGET minimusllibc PROPERTY INTERFACE_LINK_LIBRARIES minimusllibc_imported)
target_include_directories(
    minimusllibc 
    INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}/include"
)