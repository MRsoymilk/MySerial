!include "MUI2.nsh"
; MySerial Installation Script (ASCII-only, no plugins required)
Name "MySerial"

!define MUI_ICON "D:\work\output\auto_build\MySerial\nsis_MySerial.ico"

!ifndef VERSION
  !define VERSION "0.0.0"
!endif

!ifndef OUTDIR
  !define OUTDIR "D:\work\output\MySerial"
!endif

!ifndef TIMESTAMP
  !define TIMESTAMP "unknown"
!endif

OutFile "${OUTDIR}\MySerial_${VERSION}_${TIMESTAMP}.exe"
InstallDir "$PROGRAMFILES\MySerial"
RequestExecutionLevel admin

!include MUI2.nsh

!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
Page custom OptionsPage OptionsPageLeave
!insertmacro MUI_PAGE_INSTFILES
!define MUI_PAGE_CUSTOMFUNCTION_PRE LaunchApp
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Var RemoveConfig
Var RemoveLog
Var LaunchProgram

!define REGKEY "Software\MySerial"
!define REGVALUE_INSTALLDIR "InstallDir"

Function .onInit
  ; Initialize plugins directory for temporary files
  InitPluginsDir

  ; Extract kill_MySerial.bat from the source directory
  File /nonfatal /oname=$PLUGINSDIR\kill_MySerial.bat "D:\work\output\MySerial\latest\kill_MySerial.bat"

  ; Run the batch file to terminate MySerial.exe
  nsExec::ExecToStack '"$PLUGINSDIR\kill_MySerial.bat"'
  Pop $0 ; Return value
  Pop $1 ; Output (if any)

  ; Check if the batch file failed (non-zero return code)
  StrCmp $0 0 +3
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Cannot terminate MySerial.exe. Please close it manually and retry, or cancel to abort." IDRETRY retry
    Abort
  retry:
    nsExec::ExecToStack '"$PLUGINSDIR\kill_MySerial.bat"'
    Pop $0
    Pop $1
    StrCmp $0 0 +2
      MessageBox MB_OK|MB_ICONEXCLAMATION "Failed to terminate MySerial.exe. Continuing installation anyway."

  ; Read the previous installation path from the registry
  ReadRegStr $INSTDIR HKLM "${REGKEY}" "${REGVALUE_INSTALLDIR}"
FunctionEnd

Function OptionsPage
  !insertmacro MUI_HEADER_TEXT "Installation Options" "Select which folders to keep and whether to launch the program"
  nsDialogs::Create 1018
  Pop $0

  ${NSD_CreateCheckbox} 0 10u 100% 10u "Keep config directory"
  Pop $RemoveConfig
  ${NSD_SetState} $RemoveConfig ${BST_CHECKED}

  ${NSD_CreateCheckbox} 0 30u 100% 10u "Keep log directory"
  Pop $RemoveLog
  ${NSD_SetState} $RemoveLog ${BST_CHECKED}

  ${NSD_CreateCheckbox} 0 50u 100% 10u "Launch MySerial after installation"
  Pop $LaunchProgram
  ${NSD_SetState} $LaunchProgram ${BST_CHECKED}

  nsDialogs::Show
FunctionEnd

Function OptionsPageLeave
  ${NSD_GetState} $RemoveConfig $RemoveConfig
  ${NSD_GetState} $RemoveLog $RemoveLog
  ${NSD_GetState} $LaunchProgram $LaunchProgram
FunctionEnd

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite on

  ; Install all files (except config, log, and kill_MySerial.bat)
  File /r /x "config" /x "log" /x "kill_MySerial.bat" "D:\work\output\MySerial\latest\*.*"

  ; Install config and log directories based on user selection
  ${If} $RemoveConfig == ${BST_CHECKED}
    File /nonfatal /r "D:\work\output\MySerial\latest\config"
  ${EndIf}

  ${If} $RemoveLog == ${BST_CHECKED}
    File /nonfatal /r "D:\work\output\MySerial\latest\log"
  ${EndIf}

  ; Delete existing desktop shortcut if it exists
  Delete "$DESKTOP\MySerial.lnk"

  ; Create desktop shortcut for MySerial.exe
  CreateShortCut "$DESKTOP\MySerial.lnk" "$INSTDIR\MySerial.exe" "" "$INSTDIR\MySerial.exe" 0

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${REGKEY}" "${REGVALUE_INSTALLDIR}" "$INSTDIR"
SectionEnd

Section "Uninstall"
  ; Remove desktop shortcut
  Delete "$DESKTOP\MySerial.lnk"

  ; Remove installation directory
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "${REGKEY}"
SectionEnd

Function LaunchApp
  ${If} $LaunchProgram == ${BST_CHECKED}
    ExecShell "" "$INSTDIR\MySerial.exe"
  ${EndIf}
FunctionEnd