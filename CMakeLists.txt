cmake_minimum_required(VERSION 3.16)

project(covering)

set(CMAKE_CXX_STANDARD 17)
link_libraries(stdc++fs)

include_directories(src)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
    CHECK_CXX_COMPILER_FLAG("-mtune=native" COMPILER_SUPPORTS_MTUNE_NATIVE)
    if(COMPILER_SUPPORTS_MARCH_NATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()
    if(COMPILER_SUPPORTS_MTUNE_NATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=native")
    endif()
endif()


include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11
        GIT_TAG c2ea58c7f9bb2a1da2d3d7f5b462121ac6a07f16  # v2.3.1 commit tag
)
FetchContent_MakeAvailable(cli11)

set(CGAL_DIR "$ENV{CGAL_HOME}")
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

find_package(CGAL REQUIRED)
include(${CGAL_USE_FILE})

add_subdirectory(src)
