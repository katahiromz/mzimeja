TARGETPATH=obj
TARGETTYPE=DYNLINK
TARGETEXT=ime
UMTYPE=windows
DLLENTRY=_DllMainCRTStartup
DLLBASE=0x00500000

C_DEFINES=-DWINVER=0x40A

!if "$(DDKBUILDENV)" == "checked"
C_DEFINES=$(C_DEFINES) -DDEBUG=1
!endif
USE_MSVCRT=1

NO_NTDLL=1

INCLUDES=..; \
         $(BASEDIR)\src\ime\inc;

SOURCES= \
        ..\config.cc \
        ..\data.cc \
        ..\dic.cc \
        ..\dic2.cc \
        ..\fakeime.cc \
        ..\fdebug.cc \
        ..\imm.cc \
        ..\immsec.cc \
        ..\input.cc \
        ..\process.cc \
        ..\regword.cc \
        ../subs.cc \
        ../toascii.cc \
        ../ui.cc \
        ../uicand.cc \
        ../uicomp.cc \
        ../uiguide.cc \
        ../uistate.cc \

TARGETLIBS= \
        $(SDK_LIB_PATH)\advapi32.lib \
        $(SDK_LIB_PATH)\comctl32.lib \
        $(SDK_LIB_PATH)\gdi32.lib \
        $(SDK_LIB_PATH)\imm32.lib \
        $(SDK_LIB_PATH)\kernel32.lib \
        $(SDK_LIB_PATH)\user32.lib

NTTARGETFILES=copyfile

