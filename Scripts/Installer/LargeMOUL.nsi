!include nsDialogs.nsh

;--------------------------------

!macro BIMAGE IMAGE PARMS
	Push $0
	GetTempFileName $0
	File /oname=$0 "${IMAGE}"
	SetBrandingImage ${PARMS} $0
	Delete $0
	Pop $0
!macroend


; The name of the installer
Name "Myst Online: Uru Live"

; The file to write
OutFile "MOULInstaller.exe"

XPStyle on

; Add branding image to the installer (an image placeholder on the side).
; It is not enough to just add the placeholder, we must set the image too...
; We will later set the image in every pre-page function.
; We can also set just one persistent image in .onGUIInit
AddBrandingImage top 56

Function .onInit
	ClearErrors
	ReadRegStr $0 HKLM SOFTWARE\MOUL "Install_Dir"
	IfErrors init.done
	MessageBox MB_YESNO|MB_ICONQUESTION "Myst Online: Uru Live has already been installed on this system.$\nWould you like to proceed with this installation?" \
		IDYES init.done
	Quit
init.done:
FunctionEnd


Function .onInstSuccess

  Exec "$INSTDIR\UruLauncher.exe"

FunctionEnd


;LicenseText "Here is some text"
LicenseData "..\..\Docs\ReleaseNotes\TOS.txt"

; The default installation directory
InstallDir "$PROGRAMFILES\Uru Live"
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\MOUL "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

; pages
Page custom nsDialogsWelcome
Page license licenseImage
Page directory dirImage
Page instfiles instImage



Function nsDialogsWelcome
	nsDialogs::Create 1018
	Pop $0

	!insertmacro BIMAGE "..\..\Sources\Plasma\Apps\plClient\res\banner.bmp" /RESIZETOFIT

	${NSD_CreateLabel} 0 0u 75% 100u "Welcome to Myst Online: Uru Live!$\r$\nCyan Worlds is proud to restore Myst Online: Uru Live back to the players!$\r$\n$\r$\nThis version is free for all to enjoy, explore, solve, and share with others.$\r$\n$\r$\nSign up at http://www.urulive.com $\r$\n$\r$\nEnjoy!"
	Pop $0

	nsDialogs::Show
FunctionEnd

Function licenseImage
	!insertmacro BIMAGE "..\..\Sources\Plasma\Apps\plClient\res\banner.bmp" /RESIZETOFIT
FunctionEnd

Function dirImage
	!insertmacro BIMAGE "..\..\Sources\Plasma\Apps\plClient\res\banner.bmp" /RESIZETOFIT
FunctionEnd

Function instImage
	!insertmacro BIMAGE "..\..\Sources\Plasma\Apps\plClient\res\banner.bmp" /RESIZETOFIT
FunctionEnd


; The stuff to install
Section "Myst Online: Uru Live (required)"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File UruLauncher.exe
  File /r avi
  File /r sfx
  File /r dat

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\MOUL "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MOUL" "DisplayName" "Myst Online: Uru Live (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MOUL" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"

  ; write the run as admin for UruLauncher.exe
  WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\layers" "$INSTDIR\UruLauncher.exe" "RUNASADMIN"

  ; Create desktop shortcuts
  CreateShortCut "$DESKTOP\Myst Online - Uru Live.lnk" "$INSTDIR\UruLauncher.exe" "" "$INSTDIR\UruLauncher.exe" 0

  ; Create Start Menu items
  CreateDirectory "$SMPROGRAMS\Uru Live"
  CreateShortCut "$SMPROGRAMS\Uru Live\Myst Online - Uru Live.lnk" "$INSTDIR\UruLauncher.exe" "" "$INSTDIR\UruLauncher.exe" 0
  CreateShortCut "$SMPROGRAMS\Uru Live\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

; uninstall stuff
UninstallText "This will remove all the Myst Online: Uru Live files from your computer.$\r$\nHowever, this will not remove your settings files in My Documents/Uru Live. Additionally, it will not remove your Myst Online: Uru Live account and your online progress will be preserved."

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MOUL"
  DeleteRegKey HKLM SOFTWARE\MOUL

  ; remove files
  Delete "$INSTDIR\avi\*.*"
  RMDir  "$INSTDIR\avi"
  Delete "$INSTDIR\dat\*.*"
  RMDir  "$INSTDIR\dat"
  Delete "$INSTDIR\sfx\StreamingCache\*.*"
  RMDir  "$INSTDIR\sfx\StreamingCache"
  Delete "$INSTDIR\sfx\*.*"
  RMDir  "$INSTDIR\sfx"
  ; remove shortcuts, if any.
  Delete "$DESKTOP\Myst Online -  Uru Live.lnk"
  Delete "$SMPROGRAMS\Uru Live\*.*"
  RMDir  "$SMPROGRAMS\Uru Live"
  ; remove all the files at the root of the program
  Delete "$INSTDIR\*.*"


  RMDir "$INSTDIR"
SectionEnd
