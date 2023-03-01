include_directories(../include)
add_executable(dict_compile dict_compile.cpp dict_compile_res.rc)
target_link_libraries(dict_compile kernel32 user32)
