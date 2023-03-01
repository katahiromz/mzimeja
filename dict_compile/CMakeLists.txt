include_directories(../include)
add_executable(dict_compile dict_compile.cpp dict_compile_res.rc)
target_link_libraries(dict_compile kernel32 user32)
target_compile_options(dict_compile PRIVATE -finput-charset=CP932 -fexec-charset=CP932)
