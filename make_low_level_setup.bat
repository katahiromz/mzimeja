set VERSION=1.0.1.0
set DEST_DIR=mzimeja-%VERSION%-LL-setup
set OUTPUT=mzimeja-%VERSION%-LL-setup.exe
if not exist "%DEST_DIR%" mkdir "%DEST_DIR%"
if not exist "%DEST_DIR%\x86" mkdir "%DEST_DIR%\x86"
if exist archive.7z del archive.7z
for %%F in (README_ja.txt LICENSE.txt ChangeLog.txt res\basic.dic res\name.dic res\kanji.dat res\radical.dat res\postal.dat build32\Release\ime_setup32.exe build32\Release\imepad.exe build32\Release\dict_compile.exe build32\Release\verinfo.exe) do copy %%F "%DEST_DIR%"
for %%F in (build32\Release\mzimeja.ime) do copy %%F "%DEST_DIR%\x86"
C:\7z2409-extra\7za.exe a archive.7z "%DEST_DIR%\*.*" "%DEST_DIR%\x86\*.*"
copy /b "C:\Program Files\7-Zip\7z.sfx" + archive.7z "%OUTPUT%"
del archive.7z
pause
