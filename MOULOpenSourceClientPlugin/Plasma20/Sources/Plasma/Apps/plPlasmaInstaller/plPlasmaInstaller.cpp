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
#include "HeadSpin.h"
#include "plPlasmaInstaller.h"
#include "resource.h"
#include <windowsx.h>
#include <commctrl.h>

#include "../plFile/hsFiles.h"
#include "plUnzip.h"
#include "plInstallerReg.h"
#include "../plFile/plBrowseFolder.h"
#include "plSetPlasmaPath.h"

plPlasmaInstaller::plPlasmaInstaller()
{
	fDailyDir[0] = '\0';
	fDidGet = false;
	fStatusList = nil;

	INITCOMMONCONTROLSEX icc = {0};
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&icc);
}

void plPlasmaInstaller::Create()
{
	ICreateDialog(IDD_INSTALLER, NULL);
}

static const char* kAllClientExes = "AllClientExes.zip";
static const char* kAllDllsRelease = "AllDllsRelease.zip";
static const char* kScripts = "Scripts.zip";
static const char* kTools = "AllToolsRelease.zip";

bool FileExists(const char* path, const char* filename)
{
	char fullpath[MAX_PATH];
	sprintf(fullpath, "%s%s", path, filename);
	HANDLE hFile = CreateFile(fullpath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return true;
	}

	return false;
}

bool plPlasmaInstaller::IGetDailyDir()
{
	// Get the branch
	HWND hBuild = GetDlgItem(fDlg, IDC_BUILD_COMBO);
	int idx = ComboBox_GetCurSel(hBuild);
	int buildServer = ComboBox_GetItemData(hBuild, idx);

	HWND hTime = GetDlgItem(fDlg, IDC_TIME_COMBO);
	idx = ComboBox_GetCurSel(hTime);
	int time = ComboBox_GetItemData(hTime, idx);
	
	// Get the build date
	SYSTEMTIME date;
	DateTime_GetSystemtime(GetDlgItem(fDlg, IDC_BRANCH_DATE), &date);
	char dateStr[] = "xx-xx-xxxx";
	sprintf(dateStr, "%02d-%02d-%04d", date.wMonth, date.wDay, date.wYear);

	fDailyDir[0] = '\0';

	IAddStatusLine("Searching for %s build...", dateStr);


	char buildDir[MAX_PATH];

	static const char* kMainBuild = "\\\\Plasmabuild\\Output\\";
	static const char* kBranchBuild = "\\\\Branchbuild\\Output\\";
	static const char* kActiveBuild = "\\\\Activebuild\\Output\\";
	static const char* kInternalMain = "Main-Internal\\";
	static const char* kInternalBranch = "Branch-Internal\\";
	static const char* kInternalActive = "Active-Internal\\";

	switch (buildServer)
	{
	case kBuildMain:	strcpy(buildDir, kMainBuild);	break;
	case kBuildBranch:	strcpy(buildDir, kBranchBuild);	break;
	case kBuildActive:	strcpy(buildDir, kActiveBuild);	break;
	}

	switch (time)
	{
	case kNightly:
		strcat(buildDir, "Nightly\\");
		break;
	case kAfternoon:
		strcat(buildDir, "Afternoon\\");
		break;
	case kEvening:
		strcat(buildDir, "Evening\\");
		break;
	}

	strcat(buildDir, dateStr);
	strcat(buildDir, "\\");

	switch (buildServer)
	{
	case kBuildMain:	strcat(buildDir, kInternalMain);	break;
	case kBuildBranch:	strcat(buildDir, kInternalBranch);	break;
	case kBuildActive:	strcat(buildDir, kInternalActive);	break;
	}

	if (FileExists(buildDir, kAllClientExes) && FileExists(buildDir, kAllDllsRelease) && FileExists(buildDir, kScripts))
	{
		strcpy(fDailyDir, buildDir);

		const char* serverName = nil;
		switch (buildServer)
		{
		case kBuildMain:	serverName = "Main";	break;
		case kBuildBranch:	serverName = "Branch";	break;
		case kBuildActive:	serverName = "Active";	break;
		}
		IAddStatusLine("Found %s at %s", serverName, fDailyDir);

		EnableWindow(GetDlgItem(fDlg, IDC_GET_BUTTON), TRUE);
		return true;
	}

	IAddStatusLine("Couldn't find build");
	EnableWindow(GetDlgItem(fDlg, IDC_GET_BUTTON), FALSE);
	return false;
}

void plPlasmaInstaller::IInit()
{
	const char* clientDir = plInstallerReg::GetClientDir();
	SetDlgItemText(fDlg, IDC_CLIENT_EDIT, clientDir);

	const char* maxDir = plInstallerReg::GetMaxDir();
	SetDlgItemText(fDlg, IDC_3DSMAX_EDIT, maxDir);

	fStatusList = GetDlgItem(fDlg, IDC_STATUS_LIST);

	HWND hCombo = GetDlgItem(fDlg, IDC_BUILD_COMBO);
	int idx = ComboBox_AddString(hCombo, "Main");
	ComboBox_SetItemData(hCombo, idx, kBuildMain);
	ComboBox_SetCurSel(hCombo, idx);
	idx = ComboBox_AddString(hCombo, "Branch");
	ComboBox_SetItemData(hCombo, idx, kBuildBranch);
	idx = ComboBox_AddString(hCombo, "Active");
	ComboBox_SetItemData(hCombo, idx, kBuildActive);

	HWND hTime = GetDlgItem(fDlg, IDC_TIME_COMBO);
	idx = ComboBox_AddString(hTime, "Nightly");
	ComboBox_SetItemData(hTime, idx, kNightly);
	ComboBox_SetCurSel(hTime, idx);
	idx = ComboBox_AddString(hTime, "Afternoon");
	ComboBox_SetItemData(hTime, idx, kAfternoon);
	idx = ComboBox_AddString(hTime, "Evening");
	ComboBox_SetItemData(hTime, idx, kEvening);

	CheckDlgButton(fDlg, IDC_CLIENT_CHECK, BST_CHECKED);
	CheckDlgButton(fDlg, IDC_SCRIPTS_CHECK, BST_CHECKED);
	CheckDlgButton(fDlg, IDC_PLUGINS_CHECK, BST_CHECKED);

	ShowWindow(fDlg, SW_SHOW);

	IGetDailyDir();
}

BOOL plPlasmaInstaller::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		IInit();
		SetFocus(GetDlgItem(fDlg, IDC_GET_BUTTON));
		return FALSE;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		return TRUE;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				PostMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE;

			case IDC_BROWSE_3DSMAX:
			case IDC_BROWSE_CLIENT:
				IGetFolder(LOWORD(wParam) == IDC_BROWSE_CLIENT);
				return TRUE;

			case IDC_GET_BUTTON:
				if (fDidGet)
					PostMessage(hDlg, WM_CLOSE, 0, 0);
				else
					IGet();
				return TRUE;
			}
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE && (LOWORD(wParam) == IDC_TIME_COMBO || LOWORD(wParam) == IDC_BUILD_COMBO))
		{
			IGetDailyDir();
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			if (nmhdr->idFrom == IDC_BRANCH_DATE && nmhdr->code == DTN_CLOSEUP/*DTN_DATETIMECHANGE*/)
			{
				IGetDailyDir();
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

void plPlasmaInstaller::IExtractZip(const char* filename, const char* dest)
{
	plUnzip unzip;
	if (unzip.Open(filename))
	{
		IAddStatusLine("Extracting %s...", filename);

		char buf[MAX_PATH];
		while (unzip.ExtractNext(dest, buf))
			IAddStatusLine("    %s", buf);
		IAddStatusLine("    %s", buf);

		unzip.Close();
	}
}

void plPlasmaInstaller::IGet()
{
	bool getClient  = (IsDlgButtonChecked(fDlg, IDC_CLIENT_CHECK) == BST_CHECKED);
	bool getScripts = (IsDlgButtonChecked(fDlg, IDC_SCRIPTS_CHECK) == BST_CHECKED);
	bool getPlugins = (IsDlgButtonChecked(fDlg, IDC_PLUGINS_CHECK) == BST_CHECKED);
	bool getTools   = (IsDlgButtonChecked(fDlg, IDC_TOOLS_CHECK) == BST_CHECKED);
	
	const char* clientDir = plInstallerReg::GetClientDir();
	if (*clientDir == '\0' && (getClient || getScripts))
	{
		MessageBox(fDlg, "You need to set your client directory", "Plasma Installer", MB_OK | MB_ICONASTERISK);
		return;
	}
	const char* maxDir = plInstallerReg::GetMaxDir();
	if (*maxDir == '\0' && getPlugins)
	{
		MessageBox(fDlg, "You need to set your 3dsmax directory", "Plasma Installer", MB_OK | MB_ICONASTERISK);
		return;
	}

	HWND hGetButton = GetDlgItem(fDlg, IDC_GET_BUTTON);
	EnableWindow(hGetButton, FALSE);
	HCURSOR hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	char buf[MAX_PATH];

	if (getScripts)
	{
		sprintf(buf, "%s%s", fDailyDir, kScripts);
		IExtractZip(buf, clientDir);
	}

	if (getClient)
	{
		sprintf(buf, "%s%s", fDailyDir, kAllClientExes);
		IExtractZip(buf, clientDir);
	}

	if (getPlugins)
	{
		sprintf(buf, "%s%s", fDailyDir, kAllDllsRelease);
		char pluginDir[MAX_PATH];
		sprintf(pluginDir, "%s\\plugins", maxDir);
		IExtractZip(buf, pluginDir);

		IAddStatusLine("Updating PlasmaMAX2.ini...");
		sprintf(buf, "%s\\plugcfg\\PlasmaMAX2.ini", maxDir);
		WritePrivateProfileString("SceneViewer", "Directory", clientDir, buf);
	}

	if (getTools)
	{
		sprintf(buf, "%s%s", fDailyDir, kTools);

		char toolBuf[MAX_PATH];
		sprintf(toolBuf, "%s\\Tools", clientDir);
		IExtractZip(buf, toolBuf);
	}

	IAddStatusLine("Updating path...");
	SetPlasmaPath(clientDir);

	IAddStatusLine("Done");

	SetCursor(hOldCursor);

	fDidGet = true;
	SetWindowText(hGetButton, "Close");
	EnableWindow(hGetButton, TRUE);
}

void plPlasmaInstaller::IGetFolder(bool client)
{
	char path[MAX_PATH];
	if (client)
		strcpy(path, plInstallerReg::GetClientDir());
	else
		strcpy(path, plInstallerReg::GetMaxDir());

	if (plBrowseFolder::GetFolder(path, path))
	{
		if (client)
		{
			SetDlgItemText(fDlg, IDC_CLIENT_EDIT, path);
			plInstallerReg::SetClientDir(path);
		}
		else
		{
			SetDlgItemText(fDlg, IDC_3DSMAX_EDIT, path);
			plInstallerReg::SetMaxDir(path);
		}
	}
}

void plPlasmaInstaller::IAddStatusLine(const char* format, ...)
{
	if (!format || *format == '\0')
		return;

	va_list args;
	va_start(args, format);

	char buf[2048];
	int numWritten = _vsnprintf(buf, sizeof(buf), format, args);
	hsAssert(numWritten > 0, "Buffer too small");

	va_end(args);

	int idx = ListBox_AddString(fStatusList, buf);
	ListBox_SetCurSel(fStatusList, idx);

	// Pump the message queue
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
