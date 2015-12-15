cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(SHLIB "-L/home/kdehairy/usr/libs")

include_directories("/home/kdehairy/usr/include")

set(SOURCE_FILES
        DexDiff.cpp DexInput.h
        neutralizer/neutralize.cpp
        neutralizer/neutralize.h
        neutralizer/hashmap/hashmap.c
        neutralizer/hashmap/hashmap.h
        neutralizer/hashmap/hashmap_string.c
        neutralizer/hashmap/hashmap_string.h
        diff_generator/diff_generator.cpp
        diff_generator/diff_generator.h
        bsdiff_helper.cpp bsdiff_helper.h)

add_executable(dxdiff ${SOURCE_FILES})
target_link_libraries(dxdiff
        ${SHLIB}
        wdex bsdiff)