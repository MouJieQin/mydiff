cmake_minimum_required (VERSION 2.8)

SET(mydiff_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${mydiff_ROOT_DIR}/src)

project (mydiff)

#add_definitions(-std=c++11 -O0 -g -W -Wall)
add_definitions(-std=c++11 -O3 -W -Wall)

#add_subdirectory(src/lib)

aux_source_directory(${mydiff_ROOT_DIR}/src/tpcds DIR_LIB_SRCS)
add_executable(mydiff ${mydiff_ROOT_DIR}/src/main/mydiff-main.cpp ${DIR_LIB_SRCS})

#target_link_libraries(mydiff lib)

install(TARGETS mydiff DESTINATION /usr/local/bin)
