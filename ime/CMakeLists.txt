add_library(ime_convert STATIC convert.cpp keychar.cpp)

# Workaround for Shift_JIS
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ime_convert PRIVATE -finput-charset=CP932 -fexec-charset=CP932)
endif()

add_library(ime SHARED
    cand_info.cpp
    comp_str.cpp
    config.cpp
    imm.cpp
    immsec.cpp
    input.cpp
    main.cpp
    process.cpp
    regword.cpp
    ui.cpp
    uicand.cpp
    uicomp.cpp
    uiguide.cpp
    uistate.cpp
    mzimeja.def
    mzimeja_res.rc)
target_link_libraries(ime kernel32 user32 gdi32 advapi32 comctl32 imm32 ime_convert)
set_target_properties(ime PROPERTIES PREFIX "")
set_target_properties(ime PROPERTIES SUFFIX "")
set_target_properties(ime PROPERTIES OUTPUT_NAME "mzimeja.ime")

# do statically link
set_target_properties(ime PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(ime PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(ime PROPERTIES LINK_SEARCH_END_STATIC 1)
