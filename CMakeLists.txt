cmake_minimum_required( VERSION 3.2 FATAL_ERROR )
#set(CMAKE_CXX_STANDARD 11)   # should work with C++11 or later, but don't specify because this (-std=c++17 setting) needs to match the version ROOT was built with for string_view compatibility
project(TTreeIterator)

if(APPLE)
    add_definitions(-DAPPLE)
endif()

add_custom_target(TTreeIterator_version
        COMMENT "Generating TTreeIteratorVersion.h ..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/versioning
        COMMAND ${CMAKE_COMMAND} -DOUTPUT_FILE=${CMAKE_BINARY_DIR}/versioning/temp_version.h
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/versioning.cmake
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/versioning/temp_version.h
        ${CMAKE_BINARY_DIR}/versioning/TTreeIteratorVersion.h
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/versioning/temp_version.h)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package( ROOT COMPONENTS RIO Core Tree Hist ROOTTPython)
message(STATUS "Using ROOT From: ${ROOT_INCLUDE_DIRS}")
include(${ROOT_USE_FILE})

include_directories(${ROOT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(${ROOT_CXX_FLAGS})

## get googletest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.10.0
)
FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        main  # need main for benchmark::benchmark
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()
FetchContent_GetProperties(googlebenchmark)
if(NOT googlebenchmark_POPULATED)
    FetchContent_Populate(googlebenchmark)
    add_subdirectory(${googlebenchmark_SOURCE_DIR} ${googlebenchmark_BINARY_DIR})
endif()

add_subdirectory(TTreeIterator)
