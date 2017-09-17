add_executable(setup WIN32 setup.cpp setup_res.rc)
target_link_libraries(setup kernel32 user32 imm32)

# do statically link
set_target_properties(setup PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(setup PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(setup PROPERTIES LINK_SEARCH_END_STATIC 1)
