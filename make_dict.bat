cd
if exist build\Debug\dict_compile.exe goto label2
if exist build\Release\dict_compile.exe goto label3
if exist ..\build\Release\dict_compile.exe goto label4
if exist build\dict_compile.exe goto label1
echo ERROR: dict_compile.exe not found
exit /b 1

:label1
build\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
build\dict_compile.exe res\name.dat res\name.dic
build\dict_compile.exe res\testdata.dat res\testdata.dic
goto label5

:label2
build\Debug\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
build\Debug\dict_compile.exe res\name.dat res\name.dic
build\Debug\dict_compile.exe res\testdata.dat res\testdata.dic
goto label5

:label3
build\Release\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
build\Release\dict_compile.exe res\name.dat res\name.dic
build\Release\dict_compile.exe res\testdata.dat res\testdata.dic
goto label5

:label4
..\build\Release\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
..\build\Release\dict_compile.exe res\name.dat res\name.dic
..\build\Release\dict_compile.exe res\testdata.dat res\testdata.dic
goto label5

:label5
exit /b 0
