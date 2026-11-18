include_directories(../include)
add_executable(dict_compile dict_compile.cpp)
add_project_resource(dict_compile dict_compile_res.rc sjis)
target_link_libraries(dict_compile kernel32 user32)
