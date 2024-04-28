#define MyAppName "esp32-cam-yolo"
#define MyAppVersion "0.0.3"
#define MyAppPublisher "SciCrop"
#define MyAppURL "https://github.com/Scicrop/esp32-cam-yolo"
#define MyAppExeName "esp32-cam-yolo.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{3BF65FD2-598B-4238-8045-9889A14DC1F8}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=C:\Users\info\git\esp32-cam-yolo\LICENSE
OutputDir=C:\Users\info\git\esp32-cam-yolo\dist
OutputBaseFilename=esp32-cam-yolo-{#MyAppVersion}
SetupIconFile=C:\Users\info\git\esp32-cam-yolo\dist\icon.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\info\git\esp32-cam-yolo\dist\app.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\info\git\esp32-cam-yolo\dist\.env_template"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\info\git\esp32-cam-yolo\dist\CH341SER.EXE"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\info\git\esp32-cam-yolo\dist\yolov3.cfg"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\info\git\esp32-cam-yolo\dist\coco.names"; DestDir: "{app}"; Flags: ignoreversion


[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\app.exe"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\app.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\CH341SER.EXE"; Description: "Install USB Camera Driver"; Flags: nowait postinstall