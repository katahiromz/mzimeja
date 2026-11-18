# libime.a
set(LIBIME_SOURCES
    cand_info.cpp
    comp_str.cpp
    config.cpp
    convert.cpp
    debug.cpp
    imm.cpp
    immsec.cpp
    input.cpp
    keychar.cpp
    main.cpp
    postal.cpp
    process.cpp
    regword.cpp
    ui.cpp
    uicand.cpp
    uicomp.cpp
    uiguide.cpp
    uistate.cpp
    vksub.cpp)

# Add Vibrato engine if enabled
if(USE_VIBRATO AND VIBRATO_FOUND)
    list(APPEND LIBIME_SOURCES vibrato_engine.cpp)
    message(STATUS "Adding vibrato_engine.cpp to build")
endif()

# Link objects directly into the IME so exports are not omitted by archive extraction.
add_library(libime_objects OBJECT ${LIBIME_SOURCES})
target_include_directories(libime_objects PRIVATE ${CMAKE_SOURCE_DIR})

add_library(libime STATIC $<TARGET_OBJECTS:libime_objects>)
set_target_properties(libime PROPERTIES PREFIX "")

# mzimeja.ime
add_library(ime SHARED mzimeja.def mzimeja_res.rc $<TARGET_OBJECTS:libime_objects>)
target_link_libraries(ime kernel32 user32 gdi32 advapi32 comctl32 imm32 shlwapi)

# Link Vibrato library if enabled
if(USE_VIBRATO AND VIBRATO_FOUND)
    # Add dependency on vibrato_c_build target
    add_dependencies(ime vibrato_c_build)
    
    target_link_libraries(ime ${VIBRATO_LIBRARY})
    
    # Windows-specific libraries required by Vibrato (Rust runtime dependencies)
    if(WIN32)
        target_link_libraries(ime ws2_32 userenv bcrypt ntdll)
    endif()
    
    message(STATUS "Linking Vibrato library: ${VIBRATO_LIBRARY}")
endif()

target_compile_options(ime PRIVATE -DIME_DLL=1)
set_target_properties(ime PROPERTIES PREFIX "")
set_target_properties(ime PROPERTIES SUFFIX "")
set_target_properties(ime PROPERTIES OUTPUT_NAME "mzimeja.ime")

# imetests.exe
add_executable(imetests tests.cpp mzimeja_res.rc)
target_link_libraries(imetests libime kernel32 user32 gdi32 advapi32 comctl32 imm32 shlwapi)

add_executable(ime_load_test ../test/ime_load_test.cpp)
add_dependencies(ime_load_test ime)

# Link Vibrato for tests too
if(USE_VIBRATO AND VIBRATO_FOUND)
    add_dependencies(imetests vibrato_c_build)
    target_link_libraries(imetests ${VIBRATO_LIBRARY})
    if(WIN32)
        target_link_libraries(imetests ws2_32 userenv bcrypt ntdll)
    endif()
endif()

# do statically link
set_target_properties(ime PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(ime PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(ime PROPERTIES LINK_SEARCH_END_STATIC 1)
