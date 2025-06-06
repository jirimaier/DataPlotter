﻿; Script generated by the Inno Setup Script Wizard.
; Updated for clarity, maintainability, and added functionality, while retaining original file locations.
#define MyAppName "Data Plotter"
#define MyAppPublisher "Jiří Maier"
#define MyAppURL "https://github.com/jirimaier/DataPlotter"
#define MyAppExeName "DataPlotter.exe"
#define AppVersionUnderscore StringChange(AppVersion, ".", "_")  ; Replace dots with underscores

[Setup]
; The value of AppId uniquely identifies this application. Generate a new GUID for each version.
AppId={{EE20AEC4-BA49-49E6-A31B-B5663CC84E3F}}
AppName={#MyAppName}
AppVersion={#AppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
ArchitecturesInstallIn64BitMode=x64compatible
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=..\..\documentation\license.txt
OutputDir=deploy
OutputBaseFilename=DataPlotter_{#AppVersionUnderscore}_setup_Win64
SetupIconFile=..\..\icons\icon.ico
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequiredOverridesAllowed=dialog

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"

[Files]
; Keep file locations as originally specified.
Source: "deploy\DataPlotter\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "deploy\VC_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
; Define icons with original application executable name.
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; Execute the VC redistributable installer without forcing a reboot.
Filename: "{tmp}\VC_redist.x64.exe"; Parameters: "/quiet /norestart"; Flags: waituntilterminated
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent

[Tasks]
; Option to create a desktop icon, unchecked by default.
Name: "desktopicon"; Description: "Create a desktop icon"; GroupDescription: "Additional Icons"; Flags: unchecked
