; Installer Settings for Inno Setup
; (Debug Version)

#define MyAppName "MZ-IME日本語入力"
#define MyAppVersion "1.0.1.0"
#define MyAppPublisher "片山博文MZ"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2FCD745D-F3C2-4115-B537-D6AE6E066B82}
AppName={#MyAppName}
AppVerName={#MyAppName} {#MyAppVersion} (デバッグ版)
AppPublisher={#MyAppPublisher}
AppPublisherURL=https://katahiromz.fc2.page/
AppSupportURL=https://katahiromz.fc2.page/mzimeja
AppUpdatesURL=https://katahiromz.fc2.page/mzimeja
DefaultDirName={pf}\mzimeja
DefaultGroupName={#MyAppName}
OutputDir=.
OutputBaseFilename=mzimeja-{#MyAppVersion}d-setup
Compression=lzma
SolidCompression=yes
VersionInfoVersion={#MyAppVersion}
VersionInfoTextVersion={#MyAppVersion}
AlwaysRestart=yes
UninstallRestartComputer=yes
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64
DisableDirPage=yes

[Languages]
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"

[Files]
Source: "README_ja.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "ChangeLog.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\basic.dic"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\name.dic"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\kanji.dat"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\radical.dat"; DestDir: "{app}"; Flags: ignoreversion
Source: "res\postal.dat"; DestDir: "{app}"; Flags: ignoreversion
Source: "build32\Debug\mzimeja.ime"; DestDir: "{app}\x86"; Flags: ignoreversion
Source: "build64\Debug\mzimeja.ime"; DestDir: "{app}\x64"; Flags: ignoreversion; Check: IsWin64
Source: "build32\Debug\ime_setup32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build64\Debug\ime_setup64.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64
Source: "build32\Debug\imepad.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build32\Debug\dict_compile.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build32\Debug\verinfo.exe"; DestDir: "{app}"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\README_ja.txt"; Filename: "{app}\README_ja.txt"
Name: "{group}\LICENSE.txt"; Filename: "{app}\LICENSE.txt"
Name: "{group}\ChangeLog.txt"; Filename: "{app}\ChangeLog.txt"
Name: "{group}\MZ-IMEパッド"; Filename: "{app}\imepad.exe"
Name: "{group}\バージョン情報"; Filename: "{app}\verinfo.exe"
Name: "{group}\アンインストール"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\ime_setup32.exe"; Parameters: "/i"; Check: not(IsWin64)
Filename: "{app}\ime_setup64.exe"; Parameters: "/i"; Check: IsWin64

[UninstallRun]
Filename: "{app}\ime_setup32.exe"; Parameters: "/u"; Check: not(IsWin64)
Filename: "{app}\ime_setup64.exe"; Parameters: "/u"; Check: IsWin64

[Registry]
Root: HKCU; Subkey: "Software\Katayama Hirofumi MZ\mzimeja"; Flags: uninsdeletekey
