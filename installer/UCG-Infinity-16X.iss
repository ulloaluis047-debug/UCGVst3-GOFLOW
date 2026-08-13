#define AppName "UCG Infinity 16X"
#define AppVersion "0.2.0"
#define AppPublisher "UCG Corp"
#define AppExeName "UCG Infinity 16X.exe"

[Setup]
AppId={{B370D9B8-16A0-4B41-9A7B-16C0016A2026}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\UCG Corp\UCG Infinity 16X
DefaultGroupName=UCG Corp
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=UCG-Infinity-16X-Setup-v0.2
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
UninstallDisplayName={#AppName}

[Dirs]
Name: "{commoncf64}\VST3"

[Files]
Source: "payload\VST3\UCG Infinity 16X.vst3\*"; DestDir: "{commoncf64}\VST3\UCG Infinity 16X.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "payload\Standalone\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\UCG Corp\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Crear acceso directo en el escritorio"; GroupDescription: "Accesos directos:"; Flags: unchecked

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Abrir {#AppName}"; Flags: nowait postinstall skipifsilent
