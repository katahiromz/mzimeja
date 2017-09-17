add_library(mzimeja_ime SHARED
    cand_info.cpp
    comp_str.cpp
    config.cpp
    convert.cpp
    imm.cpp
    immsec.cpp
    input.cpp
    keychar.cpp
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
target_link_libraries(mzimeja_ime kernel32 user32 gdi32 advapi32 comctl32 imm32)
set_target_properties(mzimeja_ime PROPERTIES PREFIX "")
set_target_properties(mzimeja_ime PROPERTIES SUFFIX "")
set_target_properties(mzimeja_ime PROPERTIES OUTPUT_NAME "mzimeja.ime")

# do statically link
set_target_properties(mzimeja_ime PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(mzimeja_ime PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(mzimeja_ime PROPERTIES LINK_SEARCH_END_STATIC 1)
