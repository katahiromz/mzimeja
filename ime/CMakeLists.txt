# mzimeja.ime
add_library(ime SHARED
    cand_info.cpp
    comp_str.cpp
    config.cpp
    convert.cpp
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
    mzimeja.def
    mzimeja_res.rc)
target_link_libraries(ime kernel32 user32 gdi32 advapi32 comctl32 imm32 shlwapi)
set_target_properties(ime PROPERTIES PREFIX "")
set_target_properties(ime PROPERTIES SUFFIX "")
set_target_properties(ime PROPERTIES OUTPUT_NAME "mzimeja.ime")

# UTF-8
if(MSVC)
    target_compile_options(ime PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
endif()

# do statically link
set_target_properties(ime PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(ime PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(ime PROPERTIES LINK_SEARCH_END_STATIC 1)
