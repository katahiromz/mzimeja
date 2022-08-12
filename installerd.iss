; Installer Settings for Inno Setup
; (Debug Version)

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2FCD745D-F3C2-4115-B537-D6AE6E066B82}
AppName=MZ-IME���{�����
AppVerName=MZ-IME���{�����(�f�o�b�O��) 0.9.1
AppPublisher=�ЎR����MZ
AppPublisherURL=http://katahiromz.web.fc2.com/
AppSupportURL=http://katahiromz.web.fc2.com/
AppUpdatesURL=http://katahiromz.web.fc2.com/
DefaultDirName={pf}\mzimeja
DefaultGroupName=MZ-IME���{�����
OutputDir=.
OutputBaseFilename=mzimeja-0.9.1-debug-setup
Compression=lzma
SolidCompression=yes
VersionInfoVersion=0.9.1
VersionInfoTextVersion=0.9.1

[Languages]
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Files]
Source: "READMEJP.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\mzimeja.dic"; DestDir: "{app}\res"; Flags: ignoreversion
Source: "res\name.dic"; DestDir: "{app}\res"; Flags: ignoreversion
Source: "res\kanji.dat"; DestDir: "{app}\res"; Flags: ignoreversion
Source: "res\radical.dat"; DestDir: "{app}\res"; Flags: ignoreversion

; Source: "build\mzimeja.ime"; DestDir: "{app}"; Flags: ignoreversion
; Source: "build\setup.exe"; DestDir: "{app}"; Flags: ignoreversion
; Source: "build\imepad.exe"; DestDir: "{app}"; Flags: ignoreversion
; Source: "build\dict_compile.exe"; DestDir: "{app}"; Flags: ignoreversion
; Source: "build\verinfo.exe"; DestDir: "{app}"; Flags: ignoreversion

Source: "build\Debug\mzimeja.ime"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Debug\setup.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Debug\imepad.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Debug\dict_compile.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\Debug\verinfo.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\READMEJP.txt"; Filename: "{app}\READMEJP.txt"
Name: "{group}\LICENSE.txt"; Filename: "{app}\LICENSE.txt"
Name: "{group}\MZ-IME�p�b�h"; Filename: "{app}\imepad.exe"
Name: "{group}\�o�[�W�������"; Filename: "{app}\verinfo.exe"
Name: "{group}\�A���C���X�g�[��"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\setup.exe"; Parameters: "/i"

[UninstallRun]
Filename: "{app}\setup.exe"; Parameters: "/u"
