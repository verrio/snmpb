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
  Icon "..\..\app\images\snmpb.ico"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SnmpB"

;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER
  Var INI_VALUE

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "..\..\license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\SnmpB" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

  !insertmacro MUI_PAGE_INSTFILES

  Page custom MiscOpt

  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Reserve Files  
  ReserveFile "snmpb.ini"
  !insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

;--------------------------------
;Installer Sections

Section "SnmpB application" SecApp

  SetOutPath "$INSTDIR"
  SectionIn RO

  File ..\..\app\release\snmpb.exe
  File $%QTDIR%\bin\QtCore*
  File $%QTDIR%\bin\QtGui*
  File $%QTDIR%\bin\QtNetwork*
  File $%QTDIR%\bin\QtSql*
  File $%QTDIR%\bin\QtXml*
  File $%QTDIR%\bin\mingw*

  ;Store installation folder
  WriteRegStr HKCU "Software\SnmpB" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\SnmpB.lnk" "$INSTDIR\snmpb.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

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

Function .onInit

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "snmpb.ini"
  
FunctionEnd

LangString TEXT_IO_TITLE ${LANG_ENGLISH} "Miscellaneous installation options"
LangString TEXT_IO_SUBTITLE ${LANG_ENGLISH} "Installs configuration file, desktop shortcut, ..."

Function MiscOpt

  !insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "snmpb.ini"

  !insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "snmpb.ini" "Field 1" "State"
  
  StrCmp $INI_VALUE "1" "" +2
     CreateShortCut "$DESKTOP\SnmpB.lnk" "$INSTDIR\SnmpB.exe"

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

  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
    
  Delete "$DESKTOP\SnmpB.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\SnmpB.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  DeleteRegKey /ifempty HKCU "Software\SnmpB"

SectionEnd
