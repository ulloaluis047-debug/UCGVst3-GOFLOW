#define AppName "UCG Voice Wonder Pro"
#define AppVersion "1.0.0"
#define Publisher "UCG Corp Finance"

[Setup]
AppId={{E492D226-A869-4CF7-91D1-BC3DCE72190F}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#Publisher}
DefaultDirName={commoncf64}\VST3\UCG Voice Wonder.vst3
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\..\dist
OutputBaseFilename=UCG-Voice-Wonder-Pro-Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName=UCG Voice Wonder Pro VST3
VersionInfoVersion=1.0.0.0

[Files]
Source: "..\..\dist\UCG Voice Wonder.vst3\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\README.md"; DestDir: "{app}\Documentation"; Flags: ignoreversion
Source: "..\THIRD_PARTY_LICENSES.md"; DestDir: "{app}\Documentation"; Flags: ignoreversion

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

