#ifndef SourceDir
  #error SourceDir must be supplied by Build-Installer.ps1
#endif
#ifndef PublishDir
  #error PublishDir must contain the self-contained win-x64 publish output
#endif
#ifndef AppVersion
  #define AppVersion "1.0.0.0"
#endif

[Setup]
AppId={{8CF0F370-8CC2-4B09-A19C-996378145C9A}
AppName=PixelTool
AppVersion={#AppVersion}
DefaultDirName={localappdata}\Programs\PixelTool
DefaultGroupName=PixelTool
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#InstallerOutput}
OutputBaseFilename=PixelTool-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\PixelTool.exe

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked

[Files]
; Explicit allowlist: never copy Asset/Assets, Editor state, build intermediates or logs.
Source: "{#PublishDir}\*"; DestDir: "{app}"; Excludes: "Asset\*,Assets\*,Editor\*,Engine\*,LSP\*,LuaCode\*,*.pdb,*.log"; Flags: ignoreversion recursesubdirs createallsubdirs
; Engine Lua files are generated into the selected project by EditorManager.
Source: "{#SourceDir}\Engine\*"; DestDir: "{app}\Engine"; Flags: ignoreversion recursesubdirs createallsubdirs skipifsourcedoesntexist
Source: "{#SourceDir}\LSP\*"; DestDir: "{app}\LSP"; Excludes: "log\*,logs\*,*.log,meta\default utf8\CS.lua,meta\default utf8\UnityEngine*.lua"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\PixelTool"; Filename: "{app}\PixelTool.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\PixelTool"; Filename: "{app}\PixelTool.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\PixelTool.exe"; Description: "Launch PixelTool"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent

[Code]
function InitializeSetup(): Boolean;
var
  VCInstalled: Cardinal;
  Missing: String;
begin
  Missing := '';
  VCInstalled := 0;
  RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', VCInstalled);
  if VCInstalled <> 1 then
    Missing := Missing + 'Microsoft Visual C++ v14 Runtime (x64)' + #13#10 + 'https://aka.ms/vs/17/release/vc_redist.x64.exe' + #13#10;
  Result := Missing = '';
  if not Result then
    MsgBox('Install the following prerequisites, then run this installer again:' + #13#10#13#10 + Missing, mbError, MB_OK);
end;
