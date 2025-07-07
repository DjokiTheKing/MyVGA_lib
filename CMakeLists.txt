cmake_minimum_required(VERSION 3.13)
project(MyVGA_lib C CXX ASM)

add_library(MyVGA_lib STATIC
    ${CMAKE_CURRENT_LIST_DIR}/headers/MyVga.h
    ${CMAKE_CURRENT_LIST_DIR}/headers/MyVga.hpp
)

file(GLOB_RECURSE PIO_FILES "${CMAKE_CURRENT_LIST_DIR}/pios/*.pio")

foreach(FILE_PATH ${PIO_FILES})
    pico_generate_pio_header(MyVGA_lib ${FILE_PATH})
endforeach()

target_include_directories(MyVGA_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/headers
    ${CMAKE_CURRENT_LIST_DIR}/fonts
)

target_link_libraries(MyVGA_lib PUBLIC 
        hardware_dma
        hardware_pio
        pico_stdlib
)
