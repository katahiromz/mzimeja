# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.5)

# project name and languages
project(mzimeja CXX RC)

# -D_DEBUG or -DNDEBUG ?
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# set output directory (build/)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Win32 or not?
if (NOT WIN32)
    message(FATAL_ERROR "To build this project, use Win32 C++ compiler")
endif(NOT WIN32)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBRARIES OFF)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    SET(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -municode")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    SET(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -municode")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")
endif()

# replace "/MD" with "/MT" (building without runtime DLLs)
if(MSVC)
    set(CompilerFlags
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    foreach(CompilerFlags ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
    endforeach()
endif()

##############################################################################

# Unicode support
add_definitions(-DUNICODE=1 -D_UNICODE=1)

# USE_LOGFILE option
option(USE_LOGFILE "Use log file" OFF)
if(USE_LOGFILE)
    add_definitions(-DUSE_LOGFILE)
endif()
message(STATUS "USE_LOGFILE = ${USE_LOGFILE}")

# Add include directories
include_directories(include)

# Sub-directories
add_subdirectory(ime)
add_subdirectory(imepad)
add_subdirectory(ime_setup)
add_subdirectory(verinfo)
add_subdirectory(dict_compile)

# Build the dictionary data
add_custom_target(make_dict COMMAND cmd /C make_dict.bat
    DEPENDS res/basic.dat res/name.dat)
add_dependencies(make_dict dict_compile)

##############################################################################
