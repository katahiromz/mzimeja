include_directories(../include)
add_executable(dict_compile dict_compile.cpp dict_compile_res.rc)
target_link_libraries(dict_compile kernel32 user32)

# UTF-8
if(MSVC)
    target_compile_options(dict_compile PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
endif()
