/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "MigrationTask.h"
#include "OptionalDialog.h"
#include "commctrl.h" 
#include "resource.h"
#include "windowsx.h"
#include "shellapi.h"

#define SHARENAME "\\\\data.dni\\Parable-Root"
#define LIVECLIENTDIR "\\dataservers\\live\\game_Clients\\drcExplorer"
#define LIVEEXPANDEDINSTALLDIR "\\dataservers\\live\\game_Install\\Expanded"


int MigrationTask_Backup::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// SSH In & Run the backup Script...
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	si.lpTitle = "Server Backup Running";
	si.dwXCountChars = 80;
	si.dwYCountChars = 25;
	ZeroMemory( &pi, sizeof(pi) );
	ret = !CreateProcess(NULL,"ssh2 parable@build.bone.cyan.com \"backuplive.sh\"",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hProcess,INFINITE);
	
	return ret;
}


int MigrationTask_CleanUp::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// Clean up the Test Directory
	char* dataloc;

	switch (GetServer())
	{
	case kTest:
		dataloc = SHARENAME "\\dataservers\\test\\game_Data";
		break;
	case kLast:
		dataloc = SHARENAME "\\dataservers\\test-last\\game_Data";
		break;
	case kBranch:
		dataloc = SHARENAME "\\dataservers\\branch\\game_Data";
		break;
	}

	
	ShellExecute(hDlg,"open",dataloc,NULL,NULL,SW_SHOWNORMAL);

	if (MessageBox(hDlg, "Press OK to Continue...","Continue", MB_OKCANCEL) == IDCANCEL)
		ret = -1;
	
	return ret;
}



int MigrationTask_PatchBuilder::Run(HINSTANCE hInst, HWND hDlg)
{
	// Connect to the Data Servers by mapping the share
	
	// Run the patch builder on the Test and Live directories
	
	return 0;
}


int MigrationTask_DataMigration::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// SSH In; Copy the Data to the live
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	si.lpTitle = "Migrate the Data sets";
	si.dwXCountChars = 80;
	si.dwYCountChars = 25;
	ZeroMemory( &pi, sizeof(pi) );

	char* migratecmd;
	switch (GetServer())
	{
	case kTest:
		migratecmd = "ssh2 parable@build.bone.cyan.com \"migrate-test-data-live.sh\"";
		break;
	case kLast:
		migratecmd = "ssh2 parable@build.bone.cyan.com \"migrate-test-last-data-live.sh\"";
		break;
	case kBranch:
		migratecmd = "ssh2 parable@build.bone.cyan.com \"migrate-branch-data-live.sh\"";
		break;
	}

	ret = !CreateProcess(NULL,migratecmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hProcess,INFINITE);
	
	return ret;
}


int MigrationTask_InstallClient::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// migrate the client support files
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	si.lpTitle = "Migrate Client Support";
	si.dwXCountChars = 80;
	si.dwYCountChars = 25;
	ZeroMemory( &pi, sizeof(pi) );

	char* migratecmd;
	switch (GetServer())
	{
	case kTest:
		migratecmd = "ssh2 parable@build.bone.cyan.com \"migrate-test-client-live.sh\"";
		break;
	case kLast:
		migratecmd = "ssh2 parable@build.bone.cyan.com \"migrate-test-last-client-live.sh\"";
		break;
	case kBranch:
		migratecmd = "ssh2 parable@build.bone.cyan.com \"migrate-branch-client-live.sh\"";
		break;
	}

	ret = CreateProcess(NULL,migratecmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hProcess,INFINITE);
	
	// Connect to the Data Servers by mapping the share
	NETRESOURCE netres;
	memset(&netres,0,sizeof(netres));
	netres.lpRemoteName = SHARENAME;
	netres.dwDisplayType = RESOURCETYPE_DISK;
	ret = (WNetAddConnection3(hDlg,&netres,NULL,NULL,CONNECT_INTERACTIVE) != NO_ERROR);
	
	if (ret == 0)
	{
		char exe[MAX_PATH];
		OPENFILENAME ofn;
		SHFILEOPSTRUCT shFileOpt;
		// Choose a client to copy across
		memset(&exe, 0, sizeof(exe));
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFile = exe;
		ofn.nMaxFile = sizeof(exe);
		ofn.lpstrFilter = "Executable (*.EXE)\0*.EXE\0All (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrTitle = "Choose client exe to be used as drcExplorer.exe";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		while(!GetOpenFileName(&ofn));
		
		// copy the client across
		memset(&shFileOpt,0,sizeof(shFileOpt));
		shFileOpt.hwnd = hDlg;
		shFileOpt.wFunc = FO_COPY;
		shFileOpt.pFrom = exe;
		shFileOpt.pTo = SHARENAME LIVECLIENTDIR "\\drcExplorer.exe\0";
		ret = SHFileOperation(&shFileOpt);
		
		// Choose a client config to copy across
		memset(&exe, 0, sizeof(exe));
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFile = exe;
		ofn.nMaxFile = sizeof(exe);
		ofn.lpstrFilter = "Executable (*.EXE)\0*.EXE\0All (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrTitle = "Choose client setup exe to be used as drcConfig.exe";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		while(!GetOpenFileName(&ofn));
		
		// copy the client across
		memset(&shFileOpt,0,sizeof(shFileOpt));
		shFileOpt.hwnd = hDlg;
		shFileOpt.wFunc = FO_COPY;
		shFileOpt.pFrom = exe;
		shFileOpt.pTo = SHARENAME LIVEEXPANDEDINSTALLDIR "\\drcConfig.exe\0";
		ret = SHFileOperation(&shFileOpt);
		
		// Choose a patcher to copy across
		memset(&exe, 0, sizeof(exe));
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFile = exe;
		ofn.nMaxFile = sizeof(exe);
		ofn.lpstrFilter = "Executable (*.EXE)\0*.EXE\0All (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrTitle = "Choose patcher exe to be used as drcPatcher.exe";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		while(!GetOpenFileName(&ofn));
		
		// copy the client across
		memset(&shFileOpt,0,sizeof(shFileOpt));
		shFileOpt.hwnd = hDlg;
		shFileOpt.wFunc = FO_COPY;
		shFileOpt.pFrom = exe;
		shFileOpt.pTo = SHARENAME LIVEEXPANDEDINSTALLDIR "\\drcPatcher.exe\0";
		ret = SHFileOperation(&shFileOpt);
		
		// Choose a python.dll to copy across
		memset(&exe, 0, sizeof(exe));
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hDlg;
		ofn.lpstrFile = exe;
		ofn.nMaxFile = sizeof(exe);
		ofn.lpstrFilter = "Dynamic Lib (*.DLL)\0*.DLL\0All (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrTitle = "Choose python dll to be used as cypython21.dll";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		while(!GetOpenFileName(&ofn));
		
		// copy the client across
		memset(&shFileOpt,0,sizeof(shFileOpt));
		shFileOpt.hwnd = hDlg;
		shFileOpt.wFunc = FO_COPY;
		shFileOpt.pFrom = exe;
		shFileOpt.pTo = SHARENAME LIVEEXPANDEDINSTALLDIR "\\cypython21.dll\0";
		ret = SHFileOperation(&shFileOpt);
	}
	
	return ret;
}


int MigrationTask_GenerateClientManifest::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// Run generate manifest on the directoy on the server
	// Connect to the Data Servers by mapping the share
	NETRESOURCE netres;
	memset(&netres,0,sizeof(netres));
	netres.lpRemoteName = SHARENAME;
	netres.dwDisplayType = RESOURCETYPE_DISK;
	ret = (WNetAddConnection3(hDlg,&netres,NULL,NULL,CONNECT_INTERACTIVE) != NO_ERROR);
	
	if (ret == 0)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		ret = !CreateProcess(NULL,"plGenClientManifest.exe client=drcExplorer.exe dir=" SHARENAME LIVECLIENTDIR,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
		WaitForSingleObject(pi.hProcess,INFINITE);
	}
	
	return ret;
}


int MigrationTask_DropStoredGames::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// SSH In & Run the drop Script...
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	si.lpTitle = "Live Game Reset";
	si.dwXCountChars = 80;
	si.dwYCountChars = 25;
	ZeroMemory( &pi, sizeof(pi) );
	ret = CreateProcess(NULL,"ssh2 parable@build.bone.cyan.com \"reset-live-games.sh\"",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hProcess,INFINITE);
	
	return ret;
}


int MigrationTask_InstallAges::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// SSH In; Copy the Age Files from the data to the server AgeFiles folder
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	si.lpTitle = "Install Ages";
	si.dwXCountChars = 80;
	si.dwYCountChars = 25;
	ZeroMemory( &pi, sizeof(pi) );
	ret = !CreateProcess(NULL,"ssh2 parable@build.bone.cyan.com \"install-live-ages.sh\"",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hProcess,INFINITE);
	
	return ret;
}


int MigrationTask_CopyTestServers::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// SSH In; Delete ~/Servers; copy ~/Servers-Test to ~/Servers
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	si.lpTitle = "Copy Test Servers";
	si.dwXCountChars = 80;
	si.dwYCountChars = 25;
	ZeroMemory( &pi, sizeof(pi) );
	switch (GetServer())
	{
	case kTest:
		ret = !CreateProcess(NULL,
			"ssh2 parable@build.bone.cyan.com \"rm -frv ~/Servers; cp -rv ~/Servers-Test ~/Servers\"",
			NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
		break;
	case kLast:
		ret = !CreateProcess(NULL,
			"ssh2 parable@build.bone.cyan.com \"rm -frv ~/Servers; cp -rv ~/Servers-Test-Last ~/Servers\"",
			NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
		break;
	case kBranch:
		ret = !CreateProcess(NULL,
			"ssh2 parable@build.bone.cyan.com \"rm -frv ~/Servers; cp -rv ~/Servers-Branch ~/Servers\"",
			NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
		break;
	}
	WaitForSingleObject(pi.hProcess,INFINITE);
	
	return ret;
}


int MigrationTask_StartLiveServers::Run(HINSTANCE hInst, HWND hDlg)
{
	int ret = 0;
	
	// SSH In; Start the Live Servers
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USECOUNTCHARS | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_MINIMIZE;
	si.lpTitle = "Start Live Servers";
	si.dwXCountChars = 80;
	si.dwYCountChars = 25;
	ZeroMemory( &pi, sizeof(pi) );
	ret = !CreateProcess(NULL,"ssh2 parable@build.bone.cyan.com \"start-live-servers.sh\"",NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);
	WaitForSingleObject(pi.hProcess,INFINITE);
	
	return ret;
}


