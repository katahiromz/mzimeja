# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.10)

# project name, version, and languages
project(mzimeja VERSION 1.0.1.0 LANGUAGES CXX RC)

# -D_DEBUG or -DNDEBUG ?
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

# set output directory (build32/ and build64/)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build64)
else()
    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build32)
endif()
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

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO")
endif()

##############################################################################

# Unicode support
add_definitions(-DUNICODE=1 -D_UNICODE=1)

# Add include directories
include_directories(include)

##############################################################################
# Vibrato support (optional)

option(USE_VIBRATO "Use Vibrato for morphological analysis" OFF)

if(USE_VIBRATO)
    message(STATUS "Vibrato support enabled")
    
    # Check if Rust/Cargo is available
    find_program(CARGO_EXECUTABLE cargo)
    
    if(CARGO_EXECUTABLE)
        message(STATUS "Cargo found: ${CARGO_EXECUTABLE}")
        
        # Set paths for Vibrato
        set(VIBRATO_C_DIR "${CMAKE_SOURCE_DIR}/third_party/vibrato-c")
        set(VIBRATO_INCLUDE_DIR "${VIBRATO_C_DIR}/include")
        
        # Determine build profile and target directory
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CARGO_BUILD_TYPE "debug")
            set(CARGO_BUILD_FLAG "")
        else()
            set(CARGO_BUILD_TYPE "release")
            set(CARGO_BUILD_FLAG "--release")
        endif()
        
        set(VIBRATO_TARGET_DIR "${VIBRATO_C_DIR}/target/${CARGO_BUILD_TYPE}")
        
        # Determine library name based on platform
        if(WIN32)
            if(MSVC)
                set(VIBRATO_LIBRARY_NAME "vibrato_c.lib")
            else()
                set(VIBRATO_LIBRARY_NAME "libvibrato_c.a")
            endif()
        else()
            set(VIBRATO_LIBRARY_NAME "libvibrato_c.a")
        endif()
        
        set(VIBRATO_LIBRARY "${VIBRATO_TARGET_DIR}/${VIBRATO_LIBRARY_NAME}")
        
        # Add custom command to build vibrato-c with Cargo
        add_custom_command(
            OUTPUT ${VIBRATO_LIBRARY}
            COMMAND ${CARGO_EXECUTABLE} build ${CARGO_BUILD_FLAG}
            WORKING_DIRECTORY ${VIBRATO_C_DIR}
            COMMENT "Building vibrato-c with Cargo (${CARGO_BUILD_TYPE} mode)"
            VERBATIM
        )
        
        # Create a custom target for the Vibrato library
        add_custom_target(vibrato_c_build DEPENDS ${VIBRATO_LIBRARY})
        
        # Set variables for use in ime/CMakeLists.txt
        add_definitions(-DHAVE_VIBRATO=1)
        include_directories(${VIBRATO_INCLUDE_DIR})
        set(VIBRATO_FOUND TRUE)
        
        message(STATUS "Vibrato will be built from source")
        message(STATUS "  Library: ${VIBRATO_LIBRARY}")
        message(STATUS "  Include: ${VIBRATO_INCLUDE_DIR}")
    else()
        message(WARNING "Cargo not found. Please install Rust to build Vibrato.")
        message(STATUS "Vibrato support will be disabled")
        set(USE_VIBRATO OFF)
        set(VIBRATO_FOUND FALSE)
    endif()
else()
    message(STATUS "Vibrato support disabled")
    set(VIBRATO_FOUND FALSE)
endif()

##############################################################################

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
