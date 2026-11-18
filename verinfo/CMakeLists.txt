mzimeja_resource(VERINFO_RESOURCE verinfo_res.rc)
add_executable(verinfo WIN32 verinfo.cpp ${VERINFO_RESOURCE})
target_link_libraries(verinfo kernel32 user32)

# do statically link
set_target_properties(verinfo PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(verinfo PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(verinfo PROPERTIES LINK_SEARCH_END_STATIC 1)
