; Installer Settings for Inno Setup

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2FCD745D-F3C2-4115-B537-D6AE6E066B82}
AppName=MZ-IME���{�����
AppVerName=MZ-IME���{����� 0.3
AppPublisher=�ЎR����MZ
AppPublisherURL=http://katahiromz.web.fc2.com/
AppSupportURL=http://katahiromz.web.fc2.com/
AppUpdatesURL=http://katahiromz.web.fc2.com/
DefaultDirName={pf}\mzimeja
DefaultGroupName=MZ-IME���{�����
OutputDir=.
OutputBaseFilename=mzimeja-0.3-setup
Compression=lzma
SolidCompression=yes
VersionInfoVersion=0.3
VersionInfoTextVersion=0.3

[Languages]
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Files]
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\mzimeja.dic"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\kanji.dat"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\radical.dat"; DestDir: "{app}"; Flags: ignoreversion
Source: "projects\Debug\mzimeja.ime"; DestDir: "{app}"; Flags: ignoreversion
Source: "projects\Debug\setup.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\README.txt"; Filename: "{app}\README.txt"
Name: "{group}\LICENSE.txt"; Filename: "{app}\LICENSE.txt"
Name: "{group}\�A���C���X�g�[��"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\setup.exe"; Description: "{cm:LaunchProgram,MZ-IME���{�����}"; Flags: nowait
