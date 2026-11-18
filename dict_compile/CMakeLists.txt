include_directories(../include)
mzimeja_resource(DICT_COMPILE_RESOURCE dict_compile_res.rc)
add_executable(dict_compile dict_compile.cpp ${DICT_COMPILE_RESOURCE})
target_link_libraries(dict_compile kernel32 user32)
