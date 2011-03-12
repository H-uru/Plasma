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
#include "hsTypes.h"
#include "SceneViewer.h"
#include "SceneSync.h"

#include "../MaxMain/plMaxCFGFile.h"

#include "../MaxMain/resource.h"

// For ShellExecute
#include <shellapi.h>

static const char *kDebugClientExe = "plClient_dbg.exe";
static const char *kReleaseClientExe = "plClient.exe";

static const char *kSemaphoreName = "PlasmaSceneViewer";
static const char *kPipeName = "\\\\.\\pipe\\PlasmaSceneViewer";

SceneViewer::SceneViewer() : fhDlg(NULL)
{
	// Get the plugin CFG dir
	const char *plugFile = plMaxConfig::GetPluginIni();

	fUpdate = (GetPrivateProfileInt("SceneViewer", "Update", 1, plugFile) != 0);
	fUpdateFreq = GetPrivateProfileInt("SceneViewer", "UpdateFreq", 500, plugFile);
	fLoadOld = (GetPrivateProfileInt("SceneViewer", "LoadOld", 0, plugFile) != 0);
	fReleaseExe = (GetPrivateProfileInt("SceneViewer", "ReleaseExe", 1, plugFile) != 0);
}

SceneViewer::~SceneViewer()
{
	// Make sure the client is shut down
	ISetRunning(false);

	// Get the plugin CFG dir
	const char *plugFile = plMaxConfig::GetPluginIni();

	char buf[20];
	WritePrivateProfileString("SceneViewer", "Update", fUpdate ? "1" : "0", plugFile);
	WritePrivateProfileString("SceneViewer", "UpdateFreq", itoa(fUpdateFreq, buf, 10), plugFile);
	WritePrivateProfileString("SceneViewer", "LoadOld", fLoadOld ? "1" : "0", plugFile);
	WritePrivateProfileString("SceneViewer", "ReleaseExe", fReleaseExe ? "1" : "0", plugFile);
}

SceneViewer &SceneViewer::Instance()
{
	static SceneViewer theInstance;
	return theInstance;
}

void SceneViewer::Show()
{
	// If the dialog is already created, make sure it is visible
	if (fhDlg)
	{
		if (IsIconic(fhDlg))
			ShowWindow(fhDlg, SW_RESTORE);
	}
	else
	{
		fhDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SCENEVIEWER), GetCOREInterface()->GetMAXHWnd(), ForwardDlgProc);
	}
}

// Toggles the client and SceneSync
bool SceneViewer::IToggleRunning()
{
	fRunning = !fRunning;
	return ISetRunning(fRunning);
}

// Starts/Stops the client and SceneSync
bool SceneViewer::ISetRunning(bool running)
{
	if (running)
	{
		// The client actually is running, hmmm
		if (SceneSync::Instance().IsClientRunning())
			return true;

		// If we're not loading old data, or we are but it's not there, try to create some.
		if (!fLoadOld || !SceneSync::Instance().CanLoadOldResMgr())
		{
			// If creation fails, fail
			if (!SceneSync::Instance().CreateClientData())
				return false;
		}

		char path[MAX_PATH];
		SceneSync::Instance().GetOutputDir(path);
		strcat(path, "dat\\");

		// Start the client
		char *options = TRACKED_NEW char[strlen(path)+2+strlen(kSemaphoreName)+strlen(kPipeName)+6];
		sprintf(options, "-s %s %s \"%s\"", kSemaphoreName, kPipeName, path);

		int ret = (int)ShellExecute(NULL,
									"open",
									fReleaseExe ? kReleaseClientExe : kDebugClientExe,
									options,
									plMaxConfig::GetClientPath(),
									SW_SHOWNORMAL);
		delete [] options;

		// Client start failed
		if (ret < 32)
			return false;

		// Start client sync
		SceneSync::Instance().SetUpdateFreq(fUpdateFreq);
		SceneSync::Instance().BeginClientSync(kSemaphoreName, kPipeName);

		return true;
	}
	else
	{
		if (SceneSync::Instance().IsClientRunning())
			SceneSync::Instance().EndClientSync(false);
		return true;
	}
}

BOOL SceneViewer::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

BOOL SceneViewer::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			fhDlg = hDlg;

			// Set the exe to use
			HWND hExeCombo = GetDlgItem(hDlg, IDC_EXE);
			ComboBox_AddString(hExeCombo, "Release");
			ComboBox_AddString(hExeCombo, "Debug");
			ComboBox_SetCurSel(hExeCombo, fReleaseExe ? 0 : 1);

			// Set the client path
			const char *path = plMaxConfig::GetClientPath(false, true);
			ICustEdit *edit = GetICustEdit(GetDlgItem(hDlg, IDC_CLIENT_PATH));
			edit->SetText((char*)path);

			// Set the "Load old data" checkbox
			HWND hLoadOld = GetDlgItem(hDlg, IDC_REUSE_DATA);
			Button_SetCheck(hLoadOld, fLoadOld ? BST_CHECKED : BST_UNCHECKED);
			Button_Enable(hLoadOld, SceneSync::Instance().CanLoadOldResMgr());

			// Set the update controls
			float val = float(fUpdateFreq) / 1000.f;
			ISpinnerControl *spin = SetupFloatSpinner(hDlg, IDC_SPINNER, IDC_EDIT, 0.1, 1.f, val);
			spin->Enable(fUpdate);
			CheckDlgButton(hDlg, IDC_UPDATE, fUpdate ? BST_CHECKED : BST_UNCHECKED);

			IEnableSetupControls(!SceneSync::Instance().IsClientRunning());
		}
		return TRUE;

	case WM_COMMAND:
		// Start/Stop SceneViewer
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_START)
		{
			IToggleRunning();
			IEnableSetupControls(!SceneSync::Instance().IsClientRunning());
			return TRUE;
		}
		// Close dialog
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(hDlg);
			fhDlg = NULL;
			return TRUE;
		}
		// Browse for directory
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_DIR)
		{
			const char *path = plMaxConfig::GetClientPath(true);
			if (path)
			{
				ICustEdit *edit = GetICustEdit(GetDlgItem(hDlg, IDC_CLIENT_PATH));
				edit->SetText((char*)path);
			}

			return TRUE;
		}
		// "Load old data" selection changed
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_REUSE_DATA)
		{
			fLoadOld = (Button_GetCheck((HWND)lParam) == BST_CHECKED);
			return TRUE;
		}
		// Release/Debug exe selection changed
		else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_EXE)
		{
			int sel = ComboBox_GetCurSel((HWND)lParam);
			fReleaseExe = (sel == 0);
			return TRUE;
		}
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_UPDATE)
		{
			fUpdate = (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);

			ISpinnerControl *spin = GetISpinner(GetDlgItem(hDlg, IDC_SPINNER));
			spin->Enable(fUpdate);
			ReleaseISpinner(spin);

			// If update was turned on, send out an update message so any dirty objects
			// will be reconverted right away
			if (fUpdate)
				SceneSync::Instance().SetUpdateFreq(fUpdateFreq);

			return TRUE;
		}
		break;
		
	// Update frequency changed
	case CC_SPINNER_CHANGE:
		if (LOWORD(wParam) == IDC_SPINNER)
		{
			ISpinnerControl *spin = (ISpinnerControl*)lParam;
			float val = spin->GetFVal();
			fUpdateFreq = int(val*1000.f);
			SceneSync::Instance().SetUpdateFreq(fUpdateFreq);
		}
		return TRUE;

	// Type in directory
	case WM_CUSTEDIT_ENTER:
		if (wParam == IDC_CLIENT_PATH)
		{
			ICustEdit *edit = GetICustEdit((HWND)lParam);

			char path[MAX_PATH];
			edit->GetText(path, sizeof(path));
			plMaxConfig::SetClientPath(path);
		}
		return TRUE;
	}

	return FALSE;
}

void SceneViewer::IEnableSetupControls(bool enable)
{
	ICustEdit *edit = GetICustEdit(GetDlgItem(fhDlg, IDC_CLIENT_PATH));
	edit->Enable(enable);

	EnableWindow(GetDlgItem(fhDlg, IDC_DIR), enable);
	EnableWindow(GetDlgItem(fhDlg, IDC_EXE), enable);
	EnableWindow(GetDlgItem(fhDlg, IDC_REUSE_DATA), enable);

	SetWindowText(GetDlgItem(fhDlg, IDC_START), enable ? "Start" : "Stop");
}
