; Installer Settings for Inno Setup

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2FCD745D-F3C2-4115-B537-D6AE6E066B82}
AppName=MZ-IME日本語入力
AppVerName=MZ-IME日本語入力 0.1
AppPublisher=片山博文MZ
AppPublisherURL=http://katahiromz.web.fc2.com/
AppSupportURL=http://katahiromz.web.fc2.com/
AppUpdatesURL=http://katahiromz.web.fc2.com/
DefaultDirName={pf}\mzimeja
DefaultGroupName=MZ-IME日本語入力
OutputDir=.
OutputBaseFilename=mzimeja-0.1-setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Files]
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "Debug\setup.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "mzimeja.dic"; DestDir: "{app}"; Flags: ignoreversion
Source: "Debug\mzimeja.ime"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\README.txt"; Filename: "{app}\README.txt"
Name: "{group}\LICENSE.txt"; Filename: "{app}\LICENSE.txt"
Name: "{group}\アンインストール"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\setup.exe"; Description: "{cm:LaunchProgram,MZ-IME日本語入力}"; Flags: nowait
