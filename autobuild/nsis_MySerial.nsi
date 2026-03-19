!include "MUI2.nsh"

; ================= params =================
!ifndef APP_NAME
  !define APP_NAME "MySerial"
!endif

!ifndef APP_ARGS
  !define APP_ARGS ""
!endif

!ifndef VERSION
  !define VERSION "0.0.0"
!endif

!ifndef OUTDIR
  !define OUTDIR "D:\work\output\MySerial"
!endif

!ifndef TIMESTAMP
  !define TIMESTAMP "unknown"
!endif

; ================= basic info =================
Name "${APP_NAME}"

!define MUI_ICON "D:\work\output\auto_build\MySerial\nsis_MySerial.ico"

OutFile "${OUTDIR}\${APP_NAME}_${VERSION}_${TIMESTAMP}.exe"
InstallDir "$PROGRAMFILES\${APP_NAME}"
RequestExecutionLevel admin

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

!define REGKEY "Software\${APP_NAME}"
!define REGVALUE_INSTALLDIR "InstallDir"

; ================= init =================
Function .onInit
  InitPluginsDir

  File /nonfatal /oname=$PLUGINSDIR\kill_MySerial.bat "D:\work\output\MySerial\latest\kill_MySerial.bat"

  nsExec::ExecToStack '"$PLUGINSDIR\kill_MySerial.bat"'
  Pop $0
  Pop $1

  StrCmp $0 0 +3
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Cannot terminate MySerial.exe. Please close it manually and retry, or cancel to abort." IDRETRY retry
    Abort

  retry:
    nsExec::ExecToStack '"$PLUGINSDIR\kill_MySerial.bat"'
    Pop $0
    Pop $1
    StrCmp $0 0 +2
      MessageBox MB_OK|MB_ICONEXCLAMATION "Failed to terminate MySerial.exe. Continuing installation anyway."

  ReadRegStr $INSTDIR HKLM "${REGKEY}" "${REGVALUE_INSTALLDIR}"
FunctionEnd

; ================= self option =================
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

  ${NSD_CreateCheckbox} 0 50u 100% 10u "Launch ${APP_NAME} after installation"
  Pop $LaunchProgram
  ${NSD_SetState} $LaunchProgram ${BST_CHECKED}

  nsDialogs::Show
FunctionEnd

Function OptionsPageLeave
  ${NSD_GetState} $RemoveConfig $RemoveConfig
  ${NSD_GetState} $RemoveLog $RemoveLog
  ${NSD_GetState} $LaunchProgram $LaunchProgram
FunctionEnd

; ================= install =================
Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite on

  File /r /x "config" /x "log" /x "kill_MySerial.bat" "D:\work\output\MySerial\latest\*.*"

  ${If} $RemoveConfig == ${BST_CHECKED}
    File /nonfatal /r "D:\work\output\MySerial\latest\config"
  ${EndIf}

  ${If} $RemoveLog == ${BST_CHECKED}
    File /nonfatal /r "D:\work\output\MySerial\latest\log"
  ${EndIf}

  Delete "$DESKTOP\${APP_NAME}.lnk"

  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\MySerial.exe" "${APP_ARGS}" "$INSTDIR\MySerial.exe" 0

  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${REGKEY}" "${REGVALUE_INSTALLDIR}" "$INSTDIR"

SectionEnd

; ================= uninstall =================
Section "Uninstall"
  Delete "$DESKTOP\${APP_NAME}.lnk"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "${REGKEY}"
SectionEnd

; ================= app start =================
Function LaunchApp
  ${If} $LaunchProgram == ${BST_CHECKED}
    ExecShell "" "$INSTDIR\MySerial.exe" "${APP_ARGS}"
  ${EndIf}
FunctionEnd