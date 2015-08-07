; -- Blue.iss --


[Setup]
AppName=Blue
AppVerName=the blue programming language version 1.7.5
DefaultDirName={pf}\Blue
DefaultGroupName=Blue
Uninstallable=yes
ChangesAssociations=yes
OutputDir=installer
InfoBeforeFile="..\readme.txt"
LicenseFile="..\gpl.txt"
;ChangesEnvironment=yes
;UninstallDisplayIcon={app}\MyProg.exe
AppCopyright="Copyright (C) 2007-2008,  Erik R Lechak"


[Files]
Source: "blue.exe"; DestDir: "{app}"
Source: "blue.ico"; DestDir: "{app}"
Source: "c:\MinGW\bin\freetype6.dll"; DestDir: "{app}"
Source: "c:\MinGW\bin\libpng13.dll"; DestDir: "{app}"
Source: "c:\MinGW\bin\zlib1.dll"; DestDir: "{app}"
Source: "c:\MinGW\bin\libgnurx-0.dll"; DestDir: "{app}"
Source: "*.dll"; DestDir: "{app}\lib"
Source: "..\resource\*"; DestDir: "{app}\resource"
Source: "..\module\*"; DestDir: "{app}\module"
Source: "..\*.h" ; DestDir: "{app}\build"
Source: "blue_implib.a" ; DestDir: "{app}\build"


[Registry]
; Blue Location Lookup
Root: HKLM; Subkey: "Software\Blue"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Blue\Location"; ValueType: string; ValueData: "{app}"
; File Associations
Root: HKCR; Subkey: ".bl"; ValueType: string; ValueName: ""; ValueData: "Blue.File"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "Blue.File"; ValueType: string; ValueName: ""; ValueData: "Blue Source Code"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Blue.File\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\blue.ico,0"
Root: HKCR; Subkey: "Blue.File\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\blue.exe"" ""%1"""




