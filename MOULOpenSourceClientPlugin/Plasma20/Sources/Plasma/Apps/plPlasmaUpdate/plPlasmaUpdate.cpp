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
#include "plPlasmaUpdate.h"
#include "resource.h"
#include <windowsx.h>
#include <commctrl.h>
#include <direct.h>
#include "jvCoreUtil.h"
#include "jvDialogResizer.h"

#include "hsTypes.h"
#include "../plFile/plFileUtils.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "hsStream.h"
#include "plManifest.h"
#include "hsUtils.h"
#include "../plStatusLog/plStatusLog.h"

static plPlasmaUpdate* gInst = nil;

#define WM_UPDATE_SERVER WM_APP+1

std::string plPlasmaUpdate::fUserName = "dataserver";
std::string plPlasmaUpdate::fPassword = "parabledata";

plPlasmaUpdate::plPlasmaUpdate() : fCanExit(true), fProgressType(kValidating), fResizer(nil), fAutoDownload(false)
{
	INITCOMMONCONTROLSEX icc = {0};
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&icc);
	gInst = this;

	_getcwd(fIniPath, sizeof(fIniPath));
	char lastChar = fIniPath[strlen(fIniPath)];
	if (lastChar != '\\' && lastChar != '/')
		strcat(fIniPath, "\\");
	strcat(fIniPath, "ParableUpdate.ini");

	fFileGrabber = new plNetShareFileGrabber;
}

plPlasmaUpdate::~plPlasmaUpdate()
{
	delete fResizer;
	if (fFileGrabber)
		delete fFileGrabber;
}

bool plPlasmaUpdate::Create()
{
	if (!fServers.GetServerInfo())
		return false;

	ICreateDialog(IDD_UPDATE, NULL);
	return true;
}

BOOL CALLBACK plPlasmaUpdate::ILoginWinProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hDlg, IDC_USERNAME));
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
		{
			bool ok = (LOWORD(wParam) == IDOK);
			if (ok)
			{
				char username[25];
				char password[25];
			
				GetDlgItemText(hDlg, IDC_USERNAME, username, 25);
				GetDlgItemText(hDlg, IDC_PASSWORD, password, 25);

				fUserName = username;
				hsAssert(false, "who uses this program?");
			//	plChallengeResponse::HashPassword(password, fPassword);
			}
			EndDialog(hDlg, ok);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void plPlasmaUpdate::IInit()
{
	char curServerAddress[256];
	GetPrivateProfileString("PlasmaUpdate", "ServerAddress", "", curServerAddress, sizeof(curServerAddress), fIniPath);
	bool external = (GetPrivateProfileInt("PlasmaUpdate", "External", 0, fIniPath) != 0);

	HWND hCombo = GetDlgItem(fDlg, IDC_BUILD_COMBO);

	for (int i = 0; i < fServers.GetNumServers(); i++)
	{
		std::string& serverAddress = fServers.GetServerAddress(i);
		std::string& serverName = fServers.GetServerName(i);
		std::string& currentDir = fServers.GetServerCurrentDir(i);

		if (!fFileGrabber->IsServerAvailable(serverAddress.c_str(), currentDir.c_str()))
			continue;

		bool thisServer = (serverAddress == curServerAddress);

		int idx = ComboBox_AddString(hCombo, serverName.c_str());
		ComboBox_SetItemData(hCombo, idx, MAKELPARAM(i, 0));
		if (thisServer && !external)
			ComboBox_SetCurSel(hCombo, idx);

		std::string extName = serverName + " (External)";
		idx = ComboBox_AddString(hCombo, extName.c_str());
		ComboBox_SetItemData(hCombo, idx, MAKELPARAM(i, 1));
		if (thisServer && external)
			ComboBox_SetCurSel(hCombo, idx);
	}

	if (ComboBox_GetCurSel(hCombo) == -1)
		ComboBox_SetCurSel(hCombo, 0);

	SendMessage(fDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(jvCoreUtil::GetHInstance(), MAKEINTRESOURCE(IDI_ICON)));
	SendMessage(fDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(jvCoreUtil::GetHInstance(), MAKEINTRESOURCE(IDI_ICON)));

	fResizer = new jvDialogResizer(fDlg);
	fResizer->AddControl(IDC_BUILD_COMBO,	jvDialogResizer::kResizeX);
	fResizer->AddControl(IDC_STATUS_LIST,	jvDialogResizer::kResizeX | jvDialogResizer::kResizeY);
	fResizer->AddControl(IDC_PROGRESS,		jvDialogResizer::kLockBottom | jvDialogResizer::kResizeX);
	fResizer->AddControl(IDC_DL_TEXT,		jvDialogResizer::kLockBottom | jvDialogResizer::kResizeX);
	fResizer->AddControl(IDC_DL_BUTTON,		jvDialogResizer::kLockBottom | jvDialogResizer::kCenterX);
	fResizer->SetSize(360, 320);
	fResizer->LoadPosAndSize("PlasmaUpdate");

	bool goTime = true;
	if (fFileGrabber->NeedsAuth())
	{
		/*
		if (!DialogBox(NULL, MAKEINTRESOURCE(IDD_PLASMAUPDATE_LOGIN), fDlg, ILoginWinProc))
			goTime = false;
		else
		*/
			fFileGrabber->SetUsernamePassword(fUserName, fPassword);
	}

	if (goTime)
	{
		ShowWindow(fDlg, SW_SHOW);
		PostMessage(fDlg, WM_UPDATE_SERVER, 0, 0);
	}
	else
		PostQuitMessage(0);
}

void plPlasmaUpdate::IShutdown()
{
	fResizer->SavePosAndSize("PlasmaUpdate");
	delete fResizer;
	fResizer = NULL;

	IDeleteManifests();
}

void plPlasmaUpdate::IEnableCtrls(bool enable)
{
	fCanExit = enable;
	EnableWindow(GetDlgItem(fDlg, IDC_BUILD_COMBO), enable);

	HWND hDlButton = GetDlgItem(fDlg, IDC_DL_BUTTON);

	if (fManifests.empty())
		SetWindowText(hDlButton, "Close");
	else
		SetWindowText(hDlButton, "Download");

	EnableWindow(hDlButton, enable);

	if (enable)
		SetFocus(hDlButton);
}

void plPlasmaUpdate::IDeleteManifests()
{
	for (int i = 0; i < fManifests.size(); i++)
		delete fManifests[i];
	fManifests.clear();
}

bool plPlasmaUpdate::IGetManifests(const char* serverRoot, bool external)
{
	IDeleteManifests();

	char filePath[MAX_PATH];
	sprintf(filePath, "%sCurrent.txt", serverRoot);

	enum Sections
	{
		kVersion,
		kInternal,
		kExternal,
		kAll
	};
	int curSection = kVersion;

	hsRAMStream s;
	hsRAMStream manifestStream;

	if (fFileGrabber->FileToStream(filePath, &s))
	{
		char buf[256];
		while (s.ReadLn(buf, sizeof(buf)))
		{
			if (buf[0] == '[')
			{
				if (hsStrEQ(buf, "[Version]"))
					curSection = kVersion;
				else if (hsStrEQ(buf, "[Internal]"))
					curSection = kInternal;
				else if (hsStrEQ(buf, "[External]"))
					curSection = kExternal;
				else if (hsStrEQ(buf, "[All]"))
					curSection = kAll;
			}
			else
			{
				if (curSection == kVersion)
				{
					int version = atoi(buf);
					if (version != 1)
					{
						hsMessageBox("Your copy of PlasmaUpdate is out of date.\nPlease get the latest version.", "Error", hsMessageBoxNormal, hsMessageBoxIconError);
						return false;
					}
				}
				else if ((!external && curSection == kInternal)
						|| (external && curSection == kExternal)
						|| curSection == kAll)
				{
					//if (curSection == kAll && !(!strcmp(buf, "Data\\Movies.mfs") || !strcmp(buf, "Data\\Sounds.mfs")))
					//	continue;

					sprintf(filePath, "%s%s", serverRoot, buf);

					fFileGrabber->MakeProperPath(filePath);

					manifestStream.Reset();
					fFileGrabber->FileToStream(filePath, &manifestStream);

					plFileUtils::StripFile(filePath);

					plManifest* manifest = new plManifest(ILog);
					manifest->Read(&manifestStream, filePath, buf);
					fManifests.push_back(manifest);
				}
			}
		}

		return true;
	}

	return false;
}

void plPlasmaUpdate::IUpdateServer()
{
	char buf[256];

	IEnableCtrls(false);

	SetDlgItemText(fDlg, IDC_DL_TEXT, "Checking for updates...");

	//
	// Figure out what server we're checking
	//
	bool external = false;
	char serverRoot[MAX_PATH];

	{
		HWND hCombo = GetDlgItem(fDlg, IDC_BUILD_COMBO);
		int idx = ComboBox_GetCurSel(hCombo);
		LPARAM data = ComboBox_GetItemData(hCombo, idx);
		int server = LOWORD(data);
		external = (HIWORD(data) != 0);

		sprintf(serverRoot, "/%s/", fServers.GetServerCurrentDir(server).c_str());
		const char* serverName = fServers.GetServerAddress(server).c_str();

		ILog("===== Server set to %s %s =====", serverName, external ? "external" : "internal");

		WritePrivateProfileString("PlasmaUpdate", "ServerAddress", serverName, fIniPath);
		WritePrivateProfileString("PlasmaUpdate", "External", external ? "1" : "0", fIniPath);

		fFileGrabber->SetServer(serverName);
	}

	//
	// Get the latest publish notes
	//
	{
		HWND hList = GetDlgItem(fDlg, IDC_STATUS_LIST);
		ListBox_ResetContent(hList);

		char updateFile[MAX_PATH];
		if (external)
			sprintf(updateFile, "%sUpdates-External.txt", serverRoot);
		else
			sprintf(updateFile, "%sUpdates-Internal.txt", serverRoot);

		hsRAMStream updates;
		fFileGrabber->MakeProperPath(updateFile);
		if (fFileGrabber->FileToStream(updateFile, &updates))
		{
			while (updates.ReadLn(buf, sizeof(buf)))
				ListBox_InsertString(hList, 0, buf);
		}
	}

	//
	// Get the manifests
	//
	bool gotManifests = IGetManifests(serverRoot, external);
	UInt32 dlSize = 0;

	fProgressType = kValidating;

	if (gotManifests)
	{
		int i;

		UInt32 numFiles = 0;
		for (i = 0; i < fManifests.size(); i++)
			numFiles += fManifests[i]->NumFiles();

		HWND hProgress = GetDlgItem(fDlg, IDC_PROGRESS);
		SendMessage(hProgress, PBM_SETRANGE32, 0, numFiles);

		for (i = 0; i < fManifests.size(); i++)
		{
			fManifests[i]->ValidateFiles(ProgressFunc);
			dlSize += fManifests[i]->DownloadSize();
		}

		SendMessage(hProgress, PBM_SETPOS, 0, 0);
	}

	// Print how many megs there are to download
	if (dlSize == 0)
	{
		strcpy(buf, "No updates to download");
		IDeleteManifests();
	}
	else
	{
		float dlMegs = float(dlSize) / (1024.f*1024.f);
		if (dlMegs < .1)
			dlMegs = .1;
		sprintf(buf, "%.1f MB of updates to download", dlMegs);
	}
	SetDlgItemText(fDlg, IDC_DL_TEXT, buf);

	IEnableCtrls(true);

	if (fAutoDownload)
		PostMessage(fDlg, WM_COMMAND, MAKEWPARAM(IDC_DL_BUTTON, BN_CLICKED), LPARAM(GetDlgItem(fDlg, IDC_DL_BUTTON)));
}

void plPlasmaUpdate::IDownloadUpdates()
{
	fProgressType = kDownloading;

	IEnableCtrls(false);

	int i;

	UInt32 dlSize = 0;
	for (i = 0; i < fManifests.size(); i++)
		dlSize += fManifests[i]->DownloadSize();

	HWND hProgress = GetDlgItem(fDlg, IDC_PROGRESS);
	SendMessage(hProgress, PBM_SETRANGE32, 0, dlSize);

	for (i = 0; i < fManifests.size(); i++)
		fManifests[i]->DownloadUpdates(ProgressFunc, fFileGrabber);

	SendMessage(hProgress, PBM_SETPOS, 0, 0);

	EnableWindow(GetDlgItem(fDlg, IDC_DL_BUTTON), false);
	SetDlgItemText(fDlg, IDC_DL_TEXT, "No updates to download");

	IDeleteManifests();

	IEnableCtrls(true);

	if (fAutoDownload)
		PostMessage(fDlg, WM_COMMAND, MAKEWPARAM(IDC_DL_BUTTON, BN_CLICKED), LPARAM(GetDlgItem(fDlg, IDC_DL_BUTTON)));
}

void plPlasmaUpdate::ProgressFunc(const char* name, int delta)
{
	static const char* lastName = nil;
	if (lastName != name)
	{
		lastName = name;

		char buf[256];
		if (gInst->fProgressType == kValidating)
			strcpy(buf, "Checking ");
		else
			strcpy(buf, "Downloading ");
		strcat(buf, name);

		SetDlgItemText(gInst->fDlg, IDC_DL_TEXT, buf);
	}

	SendDlgItemMessage(gInst->fDlg, IDC_PROGRESS, PBM_DELTAPOS, delta, 0);

	jvBaseDlg::PumpQueue();
}

void plPlasmaUpdate::ILog(const char* format, ...)
{
	static plStatusLog* log = nil;

	if (!log)
		log = plStatusLogMgr::GetInstance().CreateStatusLog(0, "PlasmaUpdate.log");

	va_list args;
	va_start(args, format);
	log->AddLineV(format, args);
	va_end(args);
}

BOOL plPlasmaUpdate::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		IInit();
		SetFocus(GetDlgItem(fDlg, IDC_DL_BUTTON));
		return FALSE;

	case WM_CLOSE:
		if (fCanExit)
			DestroyWindow(hDlg);
		return TRUE;

	case WM_DESTROY:
		IShutdown();
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_DL_BUTTON)
		{
			if (fManifests.empty())
				SendMessage(fDlg, WM_CLOSE, 0, 0);
			else
				IDownloadUpdates();
			return TRUE;
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_BUILD_COMBO)
		{
			IUpdateServer();
			return TRUE;
		}
		break;

	case WM_UPDATE_SERVER:
		IUpdateServer();
		return TRUE;
	}

	return FALSE;
}
