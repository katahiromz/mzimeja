cd
if exist build\Debug\dict_compile.exe goto label2
if exist build\Release\dict_compile.exe goto label3
if exist ..\build\Release\dict_compile.exe goto label4
if exist build\dict_compile.exe goto label1
echo ERROR: dict_compile.exe not found
exit /b 1

:label1
set DICT_COMPILE=build\dict_compile.exe
goto label5

:label2
set DICT_COMPILE=build\Debug\dict_compile.exe
goto label5

:label3
set DICT_COMPILE=build\Release\dict_compile.exe
goto label5

:label4
set DICT_COMPILE=..\build\Release\dict_compile.exe
goto label5

:label5
%DICT_COMPILE% res\mzimeja.dat res\mzimeja.dic
%DICT_COMPILE% res\name.dat res\name.dic
%DICT_COMPILE% res\testdata.dat res\testdata.dic

exit /b 0
