; SNMPB win32 installer
; Martin Jolicoeur, 2006

!include "MUI.nsh"
!include "FileFunc.nsh"

!define /date NOW "%d%m%Y"

!insertmacro Locate

;--------------------------------
;General

  ;Name and file
  Name "SnmpB"
  OutFile "snmpb-v${NOW}.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SnmpB"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "..\..\license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "SnmpB application" SecApp

  SetOutPath "$INSTDIR"
  SectionIn RO

  File ..\..\app\release\snmpb.exe
  File $%QTDIR%\bin\Qt3Support*
  File $%QTDIR%\bin\QtCore*
  File $%QTDIR%\bin\QtGui*
  File $%QTDIR%\bin\QtNetwork*
  File $%QTDIR%\bin\QtSql*
  File $%QTDIR%\bin\QtXml*
  File $%QTDIR%\bin\mingw*
  File /r /x .svn ..\..\app\images

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Standard MIBs & PIBs" SecMIBS

  SetOutPath "$INSTDIR"

  File /r /x .svn /x Makefile* /x test ..\..\libsmi\mibs
  File /r /x .svn /x Makefile* ..\..\libsmi\pibs
  StrCpy $R0 $INSTDIR\mibs
  StrCpy $R1 $INSTDIR\pibs
  StrCpy $R2 $R0
  ${Locate} "$R2" "/L=F" "move_file"
  ${Locate} "$R2" "/L=DE" "del_dir"
  StrCpy $R2 $R1
  ${Locate} "$R2" "/L=F" "move_file"
  ${Locate} "$R2" "/L=DE" "del_dir"

SectionEnd

Function move_file
        Rename $R9 '$R2\$R7'
	Push $0
FunctionEnd

Function del_dir
        RMDir /REBOOTOK $R9
	Push $0
FunctionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecApp ${LANG_ENGLISH} "SnmpB application and related DLLs."
  LangString DESC_SecMIBS ${LANG_ENGLISH} "Standard MIB and PIB files extracted from RFCs."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
     !insertmacro MUI_DESCRIPTION_TEXT ${SecApp} $(DESC_SecApp)
     !insertmacro MUI_DESCRIPTION_TEXT ${SecMIBS} $(DESC_SecMIBS)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; Quick and dirty ...
  RMDir /r "$INSTDIR"

SectionEnd
