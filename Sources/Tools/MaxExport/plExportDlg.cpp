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
#include "hsTypes.h"
#include "plExportDlg.h"
#include "../MaxMain/resource.h"
#include "max.h"

#include "../MaxMain/plMaxCFGFile.h"

#include <vector>
#include <string>
using std::vector;
using std::string;

extern HINSTANCE hInstance;

class plExportDlgImp : public plExportDlg
{
protected:
	HWND fDlg;			// Handle to the setup dialog
	bool fPreshade;
	bool fPhysicalsOnly;
	bool fLightMap;
	char fExportPage[256];
	char fExportSourceDir[MAX_PATH];
	bool fExporting;
	bool fAutoExporting;
	bool fExportFile;

	int fXPos, fYPos;

	DWORD fLastExportTime;

	static BOOL CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	void IDestroy();

	void IExportCurrentFile(const char* exportPath);
	void IDoExport();

	void IInitDlg(HWND hDlg);
	void IGetRadio(HWND hDlg);

public:
	plExportDlgImp();
	~plExportDlgImp();

	virtual void Show();

	virtual bool IsExporting() { return fExporting; }
	virtual bool IsAutoExporting() { return fAutoExporting; }

	virtual bool GetDoPreshade() { return fPreshade; }
	virtual bool GetPhysicalsOnly() { return fPhysicalsOnly; }
	virtual bool GetDoLightMap() { return fLightMap; }
	virtual const char* GetExportPage();

	virtual void StartAutoExport();
};

plExportDlgImp::plExportDlgImp() : fDlg(NULL), fPreshade(true), fPhysicalsOnly(false), fLightMap(true), fLastExportTime(0), fExporting(false), fAutoExporting(false)
{
	const char* path = plMaxConfig::GetPluginIni();
	fXPos = GetPrivateProfileInt("Export", "X", 0, path);
	fYPos = GetPrivateProfileInt("Export", "Y", 30, path);

	GetPrivateProfileString("Export", "Dir", "", fExportSourceDir, sizeof(fExportSourceDir), path);

	memset(fExportPage, 0, sizeof(fExportPage));
}

BOOL WritePrivateProfileInt(LPCSTR lpAppName, LPCSTR lpKeyName, int val, LPCSTR lpFileName)
{
	char buf[30];
	itoa(val, buf, 10);

	return WritePrivateProfileString(lpAppName, lpKeyName, buf, lpFileName);
}

plExportDlgImp::~plExportDlgImp()
{
	const char* path = plMaxConfig::GetPluginIni();
	WritePrivateProfileInt("Export", "X", fXPos, path);
	WritePrivateProfileInt("Export", "Y", fYPos, path);

	WritePrivateProfileString("Export", "Dir", fExportSourceDir, path);
}

plExportDlg& plExportDlg::Instance()
{
	static plExportDlgImp theInstance;
	return theInstance;
}

BOOL plExportDlgImp::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ((plExportDlgImp&)Instance()).DlgProc(hDlg, msg, wParam, lParam);
}

const char* plExportDlgImp::GetExportPage()
{
	if (fExportPage[0] == '\0')
		return nil;
	else
		return fExportPage;
}

#include "../MaxComponent/plComponentBase.h"
#include "../MaxComponent/plMiscComponents.h"
#include "../MaxMain/plMaxNode.h"
#include "hsStlSortUtils.h"
#include <set>

typedef std::set<plComponentBase*> CompSet;

static void GetPagesRecur(plMaxNode* node, CompSet& comps)
{
	if (!node)
		return;

	plComponentBase* comp = node->ConvertToComponent();
	if (comp && (comp->ClassID() == ROOM_CID || comp->ClassID() == PAGEINFO_CID))
	{
		comps.insert(comp);
	}

	for (int i = 0; i < node->NumberOfChildren(); i++)
		GetPagesRecur((plMaxNode*)node->GetChildNode(i), comps);
}

static const char* kAllPages = "(All Pages)";

void plExportDlgImp::IGetRadio(HWND hDlg)
{
	fExportFile = (IsDlgButtonChecked(hDlg, IDC_RADIO_FILE) == BST_CHECKED);

	EnableWindow(GetDlgItem(hDlg, IDC_PAGE_COMBO), fExportFile);

//	EnableWindow(GetDlgItem(hDlg, IDC_EXPORT_PATH), !fExportFile);
	EnableWindow(GetDlgItem(hDlg, IDC_BROWSE_EXPORT), !fExportFile);
}

void plExportDlgImp::IInitDlg(HWND hDlg)
{
	// Set the client path
	const char* path = plMaxConfig::GetClientPath(false, true);
	SetDlgItemText(hDlg, IDC_CLIENT_PATH, path);

	// Set the preshade button
	CheckDlgButton(hDlg, IDC_PRESHADE_CHECK, fPreshade ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_PHYSICAL_CHECK, fPhysicalsOnly ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hDlg, IDC_LIGHTMAP_CHECK, fLightMap ? BST_CHECKED : BST_UNCHECKED);

	char buf[256];
	sprintf(buf, "Last export took %d:%02d", fLastExportTime/60, fLastExportTime%60);
	SetDlgItemText(hDlg, IDC_LAST_EXPORT, buf);

	SetWindowPos(hDlg, NULL, fXPos, fYPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	//
	// Get the names of all the pages in this scene and put them in the combo
	//
	HWND hPages = GetDlgItem(hDlg, IDC_PAGE_COMBO);
	ComboBox_AddString(hPages, kAllPages);

	bool foundPage = false;

	CompSet comps;
	GetPagesRecur((plMaxNode*)GetCOREInterface()->GetRootNode(), comps);
	for (CompSet::iterator it = comps.begin(); it != comps.end(); it++)
	{
		const char* page = LocCompGetPage(*it);
		if (page)
		{
			int idx = ComboBox_AddString(hPages, page);
			if (!strcmp(page, fExportPage))
			{
				foundPage = true;
				ComboBox_SetCurSel(hPages, idx);
			}
		}
	}

	if (!foundPage)
	{
		fExportPage[0] = '\0';
		ComboBox_SetCurSel(hPages, 0);
	}

	CheckRadioButton(hDlg, IDC_RADIO_FILE, IDC_RADIO_DIR, IDC_RADIO_FILE);
	IGetRadio(hDlg);

	SetDlgItemText(hDlg, IDC_EXPORT_PATH, fExportSourceDir);
}

#include "../plFile/plBrowseFolder.h"

BOOL plExportDlgImp::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		IInitDlg(hDlg);
		return TRUE;

	case WM_COMMAND:
		{
			int cmd = HIWORD(wParam);
			int resID = LOWORD(wParam);

			if (cmd == BN_CLICKED)
			{
				if (resID == IDCANCEL)
				{
					IDestroy();
					return TRUE;
				}
				else if (resID == IDC_EXPORT)
				{
					IDoExport();
					return TRUE;
				}
				else if (resID == IDC_PRESHADE_CHECK)
				{
					fPreshade = (IsDlgButtonChecked(hDlg, IDC_PRESHADE_CHECK) == BST_CHECKED);
					return TRUE;
				}
				else if (resID == IDC_PHYSICAL_CHECK)
				{
					fPhysicalsOnly = (IsDlgButtonChecked(hDlg, IDC_PHYSICAL_CHECK) == BST_CHECKED);
					return TRUE;
				}
				else if (resID == IDC_LIGHTMAP_CHECK)
				{
					fLightMap = (IsDlgButtonChecked(hDlg, IDC_LIGHTMAP_CHECK) == BST_CHECKED);
					return TRUE;
				}
				else if (resID == IDC_DIR)
				{
					// Get a new client path
					const char* path = plMaxConfig::GetClientPath(true);
					if (path)
						SetDlgItemText(hDlg, IDC_CLIENT_PATH, path);
					return TRUE;
				}
				else if (resID == IDC_RADIO_FILE || resID == IDC_RADIO_DIR)
				{
					IGetRadio(hDlg);
					return TRUE;
				}
				else if (resID == IDC_BROWSE_EXPORT)
				{
					plBrowseFolder::GetFolder(fExportSourceDir,
												fExportSourceDir,
												"Choose the source directory",
												hDlg);
					SetDlgItemText(hDlg, IDC_EXPORT_PATH, fExportSourceDir);
					return TRUE;
				}
			}
			else if (cmd == CBN_SELCHANGE && resID == IDC_PAGE_COMBO)
			{
				int sel = ComboBox_GetCurSel((HWND)lParam);
				// If the user selected a page, save it
				if (sel != 0 && sel != CB_ERR)
					ComboBox_GetText((HWND)lParam, fExportPage, sizeof(fExportPage));
				// Else, clear it (export all pages)
				else
					fExportPage[0] = '\0';
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

void plExportDlgImp::IExportCurrentFile(const char* exportPath)
{
	// Delete the old prd so we don't get the stupid overwrite warning
	DeleteFile(exportPath);

	GetCOREInterface()->ExportToFile(exportPath);
}

#include "../plFile/hsFiles.h"

void plExportDlgImp::IDoExport()
{
	fExporting = true;

	// Hide the window, since we don't get control back until the export is done
	ShowWindow(fDlg, SW_HIDE);

	// Do the export
	char exportPath[MAX_PATH];
	GetDlgItemText(fDlg, IDC_CLIENT_PATH, exportPath, sizeof(exportPath));
	strcat(exportPath, "Export.prd");

	// For export time stats
	DWORD exportTime = timeGetTime();

	if (fExportFile)
		IExportCurrentFile(exportPath);
	else
	{
		hsFolderIterator sourceDir(fExportSourceDir);
		while (sourceDir.NextFileSuffix(".max")) 
		{
			char exportFile[MAX_PATH];
			sourceDir.GetPathAndName(exportFile);

			if (GetCOREInterface()->LoadFromFile(exportFile))
				IExportCurrentFile(exportPath);
		}
	}

	fLastExportTime = (timeGetTime() - exportTime) / 1000;

	IDestroy();

	fExporting = false;
}

void plExportDlgImp::IDestroy()
{
	if (fDlg)
	{
		// Save the window pos
		RECT rect;
		GetWindowRect(fDlg, &rect);
		fXPos = rect.left;
		fYPos = rect.top;

		DestroyWindow(fDlg);
		fDlg = NULL;
	}
}

void plExportDlgImp::Show()
{
	if (!fDlg)
		fDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_EXPORT), GetCOREInterface()->GetMAXHWnd(), ForwardDlgProc);
}

static bool IsExcluded(const char* fileName, vector<string>& excludeFiles)
{
	for (int i = 0; i < excludeFiles.size(); i++)
	{
		if (!strcmp(fileName, excludeFiles[i].c_str()))
			return true;
	}

	return false;
}

static bool AutoExportDir(const char* inputDir, const char* outputDir, const char* groupFiles, vector<string>& excludeFiles)
{
	bool exportedFile = false;

	char outputFileName[MAX_PATH];
	sprintf(outputFileName, "%s\\Export.prd", outputDir);

	char outputLog[MAX_PATH];
	sprintf(outputLog, "%s\\AutoExport.log", outputDir);
	
	char doneDir[MAX_PATH];
	sprintf(doneDir, "%s\\Done\\", inputDir);
	CreateDirectory(doneDir, NULL);

	// Don't give missing bitmap warnings
	TheManager->SetSilentMode(TRUE);
	
	hsFolderIterator sourceDir(inputDir);
	while (sourceDir.NextFileSuffix(".max")) 
	{
		char exportFile[MAX_PATH];
		sourceDir.GetPathAndName(exportFile);

		if (IsExcluded(sourceDir.GetFileName(), excludeFiles))
			continue;

		// If we're doing grouped files, and this isn't one, keep looking
		if (groupFiles && strncmp(sourceDir.GetFileName(), groupFiles, strlen(groupFiles)) != 0)
			continue;

		hsUNIXStream log;
		if (log.Open(outputLog, "ab"))
		{
			log.WriteFmt("%s\r\n", sourceDir.GetFileName());
			log.Close();
		}

		if (GetCOREInterface()->LoadFromFile(exportFile))
		{
			sprintf(doneDir, "%s\\Done\\%s", inputDir, sourceDir.GetFileName());
			MoveFileEx(exportFile, doneDir, MOVEFILE_REPLACE_EXISTING);

			GetCOREInterface()->ExportToFile(outputFileName, TRUE);
			exportedFile = true;

			// If we're not doing grouped files, this is it, we exported our one file
			if (!groupFiles)
				break;
		}
	}

	return exportedFile;
}

// I'm sure there's a better way to do this but I can't find it in the docs
static void ShutdownMax()
{
	// If we're auto-exporting, write out a file to let the build scripts know
	// we're done writing to disk, and if we don't exit soon we probably crashed
	if (plExportDlg::Instance().IsAutoExporting())
	{
		hsUNIXStream s;
		s.Open("log\\AutoExportDone.txt", "wb");
		s.Close();
	}
	GetCOREInterface()->FlushUndoBuffer();
	SetSaveRequiredFlag(FALSE);
	PostMessage(GetCOREInterface()->GetMAXHWnd(), WM_CLOSE, 0, 0);
}

static void GetStringSection(const char* configFile, const char* keyName, vector<string>& strings)
{
	char source[256];
	GetPrivateProfileString("Settings", keyName, "", source, sizeof(source), configFile);

	char* seps = ",";
	char* token = strtok(source, seps);
	while (token != NULL)
	{
		strings.push_back(token);
		token = strtok(NULL, seps);
	}
}

void plExportDlgImp::StartAutoExport()
{
	char configFile[MAX_PATH];
	strcpy(configFile, GetCOREInterface()->GetDir(APP_PLUGCFG_DIR));
	strcat(configFile, "\\AutoExport.ini");

	char inputDir[MAX_PATH];
	GetPrivateProfileString("Settings", "MaxInputDir", "", inputDir, sizeof(inputDir), configFile);

	char outputDir[MAX_PATH];
	GetPrivateProfileString("Settings", "MaxOutputDir", "", outputDir, sizeof(outputDir), configFile);

	if (inputDir[0] == '\0' || outputDir == '\0')
		return;

	fAutoExporting = true;

	// If we're doing an autoexport, suppress prompts now
	hsMessageBox_SuppressPrompts = true;

	// Files to ignore
	vector<string> excludeFiles;
	GetStringSection(configFile, "ExcludeFiles", excludeFiles);

	//
	// Get the file substrings to export in one session
	//
	vector<string> groupedFiles;
	GetStringSection(configFile, "GroupedFiles", groupedFiles);

	for (int i = 0; i < groupedFiles.size(); i++)
	{
		if (AutoExportDir(inputDir, outputDir, groupedFiles[i].c_str(), excludeFiles))
		{
			ShutdownMax();
			fAutoExporting = false;
			return;
		}
	}

	if (AutoExportDir(inputDir, outputDir, NULL, excludeFiles))
	{
		ShutdownMax();
		fAutoExporting = false;
		return;
	}

	DeleteFile(configFile);

	fAutoExporting = false;
	ShutdownMax();
}
