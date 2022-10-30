add_executable(ime_setup WIN32 ime_setup.cpp ime_setup_res.rc)
target_link_libraries(ime_setup kernel32 user32 imm32 shlwapi)

# do statically link
set_target_properties(ime_setup PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(ime_setup PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(ime_setup PROPERTIES LINK_SEARCH_END_STATIC 1)
