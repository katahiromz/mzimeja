# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008" .
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles" .
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

# project name and language
project(mzimeja CXX)

# set program information
set(PROGRAM_MAJOR 0)
set(PROGRAM_MINOR 2)
set(PROGRAM_BUILD 0)
set(PROGRAM_NAME "mzimeja")
set(PROGRAM_AUTHORS "Katayama Hirofumi MZ")
set(PROGRAM_YEARS "2016")
set(VERSION_STRING "${PROGRAM_MAJOR}.${PROGRAM_MINOR}.${PROGRAM_BUILD}")
set(VERSION_INFO_STRING "${PROGRAM_NAME} ${VERSION_STRING} by ${PROGRAM_AUTHORS}")

# check build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# make uppercase string of build type
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

# check build type again
if (NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# define _DEBUG on Debug version
if (uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -D_DEBUG")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

# set output directory (build/)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# include directories
include_directories(include)

# set shared library prefix and postfix to none
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX "")

# Win32 or not?
if (CMAKE_HOST_WIN32)
    # enable Win32 resource
    ENABLE_LANGUAGE(RC)

    # add library mzimeja_cxx
    add_library(mzimeja_cxx STATIC
        config.cpp
        convert.cpp
        imm.cpp
        immsec.cpp
        process.cpp
        regword.cpp
        ui.cpp
        uicand.cpp
        uicomp.cpp
        uiguide.cpp
        uistate.cpp
        input.cpp
        comp_str.cpp
        cand_info.cpp
        keychar.cpp)

    # add compile options to mzimeja_cxx
    target_compile_options(mzimeja_cxx PRIVATE -municode
        -DUNICODE -D_UNICODE -finput-charset=CP932 -fexec-charset=CP932)

    # add library mzimeja.ime
    add_library(mzimeja.ime SHARED mzimeja.cpp mzimeja_res.rc)

    # link libraries to mzimeja.ime
    target_link_libraries(mzimeja.ime mzimeja_cxx imm32.lib comctl32.lib)

    # add executable setup.exe
    add_executable(setup WIN32 setup.cpp setup_res.rc)

    # add compile options to setup.exe
    target_compile_options(setup PRIVATE -DUNICODE -D_UNICODE)

    # link libraries to setup.exe
    target_link_libraries(setup imm32.lib)

    # add compile options to mzimeja_cxx
    target_compile_options(setup PRIVATE -municode
        -DUNICODE -D_UNICODE -finput-charset=CP932 -fexec-charset=CP932)
else()
    message(FATAL_ERROR "To build this project, use Win32 C++ compiler")
endif()

##############################################################################
