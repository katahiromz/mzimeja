if exist Release\dict_compile.exe goto label2
:label1
Debug\dict_compile.exe ..\res\mzimeja.dat ..\res\mzimeja.dic
Debug\dict_compile.exe ..\res\name.dat ..\res\name.dic
goto label3
:label2
Release\dict_compile.exe ..\res\mzimeja.dat ..\res\mzimeja.dic
Release\dict_compile.exe ..\res\name.dat ..\res\name.dic
:label3
