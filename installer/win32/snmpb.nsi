; SNMPB win32 installer
; Martin Jolicoeur, 2006-2011

!include "MUI.nsh"
!include "FileFunc.nsh"

/*
_____________________________________________________________________________
 
                       File Association
_____________________________________________________________________________
 
 Based on code taken from http://nsis.sourceforge.net/File_Association 
 
 Usage in script:
 1. !include "FileAssociation.nsh"
 2. [Section|Function]
      ${FileAssociationFunction} "Param1" "Param2" "..." $var
    [SectionEnd|FunctionEnd]
 
 FileAssociationFunction=[RegisterExtension|UnRegisterExtension]
 
_____________________________________________________________________________
 
 ${RegisterExtension} "[executable]" "[extension]" "[description]"
 
"[executable]"     ; executable which opens the file format
                   ;
"[extension]"      ; extension, which represents the file format to open
                   ;
"[description]"    ; description for the extension. This will be display in Windows Explorer.
                   ;
 
 
 ${UnRegisterExtension} "[extension]" "[description]"
 
"[extension]"      ; extension, which represents the file format to open
                   ;
"[description]"    ; description for the extension. This will be display in Windows Explorer.
                   ;
 
_____________________________________________________________________________
 
                         Macros
_____________________________________________________________________________
 
 Change log window verbosity (default: 3=no script)
 
 Example:
 !include "FileAssociation.nsh"
 !insertmacro RegisterExtension
 ${FileAssociation_VERBOSE} 4   # all verbosity
 !insertmacro UnRegisterExtension
 ${FileAssociation_VERBOSE} 3   # no script
*/
 
 
!ifndef FileAssociation_INCLUDED
!define FileAssociation_INCLUDED
 
!include Util.nsh
 
!verbose push
!verbose 3
!ifndef _FileAssociation_VERBOSE
  !define _FileAssociation_VERBOSE 3
!endif
!verbose ${_FileAssociation_VERBOSE}
!define FileAssociation_VERBOSE `!insertmacro FileAssociation_VERBOSE`
!verbose pop
 
!macro FileAssociation_VERBOSE _VERBOSE
  !verbose push
  !verbose 3
  !undef _FileAssociation_VERBOSE
  !define _FileAssociation_VERBOSE ${_VERBOSE}
  !verbose pop
!macroend
 
 
 
!macro RegisterExtensionCall _EXECUTABLE _EXTENSION _DESCRIPTION
  !verbose push
  !verbose ${_FileAssociation_VERBOSE}
  Push `${_DESCRIPTION}`
  Push `${_EXTENSION}`
  Push `${_EXECUTABLE}`
  ${CallArtificialFunction} RegisterExtension_
  !verbose pop
!macroend
 
!macro UnRegisterExtensionCall _EXTENSION _DESCRIPTION
  !verbose push
  !verbose ${_FileAssociation_VERBOSE}
  Push `${_EXTENSION}`
  Push `${_DESCRIPTION}`
  ${CallArtificialFunction} UnRegisterExtension_
  !verbose pop
!macroend
 
 
 
!define RegisterExtension `!insertmacro RegisterExtensionCall`
!define un.RegisterExtension `!insertmacro RegisterExtensionCall`
 
!macro RegisterExtension
!macroend
 
!macro un.RegisterExtension
!macroend
 
!macro RegisterExtension_
  !verbose push
  !verbose ${_FileAssociation_VERBOSE}
 
  Exch $R2 ;exe
  Exch
  Exch $R1 ;ext
  Exch
  Exch 2
  Exch $R0 ;desc
  Exch 2
  Push $0
  Push $1
 
  ReadRegStr $1 HKCR $R1 ""  ; read current file association
  StrCmp "$1" "" NoBackup  ; is it empty
  StrCmp "$1" "$R0" NoBackup  ; is it our own
    WriteRegStr HKCR $R1 "backup_val" "$1"  ; backup current value
NoBackup:
  WriteRegStr HKCR $R1 "" "$R0"  ; set our file association
 
  ReadRegStr $0 HKCR $R0 ""
  StrCmp $0 "" 0 Skip
    WriteRegStr HKCR "$R0" "" "$R0"
    WriteRegStr HKCR "$R0\shell" "" "open"
    WriteRegStr HKCR "$R0\DefaultIcon" "" "$R2,0"
Skip:
  WriteRegStr HKCR "$R0\shell\open\command" "" '"$R2" "%1"'
  WriteRegStr HKCR "$R0\shell\edit" "" "Edit $R0"
  WriteRegStr HKCR "$R0\shell\edit\command" "" '"$R2" "%1"'
 
  Pop $1
  Pop $0
  Pop $R2
  Pop $R1
  Pop $R0
 
  !verbose pop
!macroend
 
 
 
!define UnRegisterExtension `!insertmacro UnRegisterExtensionCall`
!define un.UnRegisterExtension `!insertmacro UnRegisterExtensionCall`
 
!macro UnRegisterExtension
!macroend
 
!macro un.UnRegisterExtension
!macroend
 
!macro UnRegisterExtension_
  !verbose push
  !verbose ${_FileAssociation_VERBOSE}
 
  Exch $R1 ;desc
  Exch
  Exch $R0 ;ext
  Exch
  Push $0
  Push $1
 
  ReadRegStr $1 HKCR $R0 ""
  StrCmp $1 $R1 0 NoOwn ; only do this if we own it
  ReadRegStr $1 HKCR $R0 "backup_val"
  StrCmp $1 "" 0 Restore ; if backup="" then delete the whole key
  DeleteRegKey HKCR $R0
  Goto NoOwn
 
Restore:
  WriteRegStr HKCR $R0 "" $1
  DeleteRegValue HKCR $R0 "backup_val"
  DeleteRegKey HKCR $R1 ;Delete key with association name settings
 
NoOwn:
 
  Pop $1
  Pop $0
  Pop $R1
  Pop $R0
 
  !verbose pop
!macroend
 
!endif # !FileAssociation_INCLUDED

!define /date NOW "%d%m%Y"

!insertmacro Locate

;--------------------------------
;General

  ;Name and file
  Name "SnmpB"
;  OutFile "snmpb-v${NOW}.exe"
  OutFile "snmpb-1.0.exe"
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
  File $%QTLOC%\Qt5Core.dll
  File $%QTLOC%\Qt5Gui.dll
  File $%QTLOC%\Qt5Widgets.dll
  File $%MINGWLOC%\libgcc_s_sjlj-1.dll
  File $%MINGWLOC%\libwinpthread-1.dll
  File $%MINGWLOC%\libstdc++-6.dll

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

  !insertmacro MUI_INSTALLOPTIONS_READ $INI_VALUE "snmpb.ini" "Field 2" "State"

  StrCmp $INI_VALUE "1" "" NoFileAssoc
     ${registerExtension} "$INSTDIR\snmpb.exe" ".mib" "MIB File"
     ${registerExtension} "$INSTDIR\snmpb.exe" ".MIB" "MIB File"
     ${registerExtension} "$INSTDIR\snmpb.exe" ".pib" "MIB File"
     ${registerExtension} "$INSTDIR\snmpb.exe" ".PIB" "MIB File"
     ${registerExtension} "$INSTDIR\snmpb.exe" ".smi" "MIB File"
     ${registerExtension} "$INSTDIR\snmpb.exe" ".SMI" "MIB File"
NoFileAssoc:

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

  ${unregisterExtension} ".mib" "MIB File"
  ${unregisterExtension} ".MIB" "MIB File"
  ${unregisterExtension} ".pib" "MIB File"
  ${unregisterExtension} ".PIB" "MIB File"
  ${unregisterExtension} ".smi" "MIB File"
  ${unregisterExtension} ".SMI" "MIB File"

SectionEnd
