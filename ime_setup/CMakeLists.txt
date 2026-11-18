# ime_setup32.exe and ime_setup64.exe
add_executable(ime_setup WIN32 ime_setup.cpp)
add_project_resource(ime_setup ime_setup_res.rc utf8)
target_link_libraries(ime_setup kernel32 user32 imm32 shlwapi)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set_target_properties(ime_setup PROPERTIES OUTPUT_NAME ime_setup64)
else()
    set_target_properties(ime_setup PROPERTIES OUTPUT_NAME ime_setup32)
endif()

# do statically link
set_target_properties(ime_setup PROPERTIES LINK_DEPENDS_NO_SHARED 1)
set_target_properties(ime_setup PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(ime_setup PROPERTIES LINK_SEARCH_END_STATIC 1)
