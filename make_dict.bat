cd
if exist build\dict_compile.exe goto label1
if exist build\Debug\dict_compile.exe goto label2
if exist build\Release\dict_compile.exe goto label3
if exist ..\build\Release\dict_compile.exe goto label4

:label1
build\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
build\dict_compile.exe res\name.dat res\name.dic
goto label5

:label2
build\Debug\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
build\Debug\dict_compile.exe res\name.dat res\name.dic
goto label5

:label3
build\Release\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
build\Release\dict_compile.exe res\name.dat res\name.dic
goto label5

:label4
..\build\Release\dict_compile.exe res\mzimeja.dat res\mzimeja.dic
..\build\Release\dict_compile.exe res\name.dat res\name.dic
goto label5

:label5
