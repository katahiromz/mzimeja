add_executable(imepad WIN32 imepad.cpp imepad_res.rc)
target_link_libraries(imepad kernel32 user32 gdi32 advapi32 comctl32 imm32)

# do statically link
set_target_properties(imepad PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(imepad PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(imepad PROPERTIES LINK_SEARCH_END_STATIC 1)
