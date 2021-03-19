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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "HeadSpin.h"
#include "hsStream.h"

#include "MaxMain/MaxAPI.h"

#include <set>
#include <string>
#include <vector>

#include "plExportDlg.h"
#include "MaxComponent/plComponentBase.h"
#include "MaxComponent/plMiscComponents.h"
#include "MaxMain/resource.h"
#include "MaxMain/plMaxCFGFile.h"
#include "MaxMain/plMaxNode.h"
#include "plFileSystem.h"

extern HINSTANCE hInstance;

class plExportDlgImp : public plExportDlg
{
protected:
    HWND fDlg;          // Handle to the setup dialog
    bool fPreshade;
    bool fPhysicalsOnly;
    bool fLightMap;
    char fExportPage[256];
    plFileName fExportSourceDir;
    bool fExporting;
    bool fAutoExporting;
    bool fExportFile;

    int fXPos, fYPos;

    DWORD fLastExportTime;

    static INT_PTR CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    void IDestroy();

    void IExportCurrentFile(const char* exportPath);
    void IDoExport();

    void IInitDlg(HWND hDlg);
    void IGetRadio(HWND hDlg);

public:
    plExportDlgImp();
    ~plExportDlgImp();

    void Show() override;

    bool IsExporting() override { return fExporting; }
    bool IsAutoExporting() override { return fAutoExporting; }

    bool GetDoPreshade() override { return fPreshade; }
    bool GetPhysicalsOnly() override { return fPhysicalsOnly; }
    bool GetDoLightMap() override { return fLightMap; }
    const char* GetExportPage() override;

    void StartAutoExport() override;
};

plExportDlgImp::plExportDlgImp()
    : fDlg(), fPreshade(true), fPhysicalsOnly(false), fLightMap(true),
      fLastExportTime(), fExporting(false), fAutoExporting(false)
{
    plFileName path = plMaxConfig::GetPluginIni();
    fXPos = GetPrivateProfileIntW(L"Export", L"X", 0, path.WideString().data());
    fYPos = GetPrivateProfileIntW(L"Export", L"Y", 30, path.WideString().data());

    wchar_t buffer[MAX_PATH];
    GetPrivateProfileStringW(L"Export", L"Dir", L"", buffer, sizeof(buffer),
                             path.WideString().data());
    fExportSourceDir = ST::string::from_wchar(buffer);

    memset(fExportPage, 0, sizeof(fExportPage));
}

BOOL WritePrivateProfileIntW(LPCWSTR lpAppName, LPCWSTR lpKeyName, int val, LPCWSTR lpFileName)
{
    wchar_t buf[12];
    swprintf(buf, 12, L"%d", val);

    return WritePrivateProfileStringW(lpAppName, lpKeyName, buf, lpFileName);
}

plExportDlgImp::~plExportDlgImp()
{
    plFileName path = plMaxConfig::GetPluginIni();
    WritePrivateProfileIntW(L"Export", L"X", fXPos, path.WideString().data());
    WritePrivateProfileIntW(L"Export", L"Y", fYPos, path.WideString().data());

    WritePrivateProfileStringW(L"Export", L"Dir", fExportSourceDir.WideString().data(),
                               path.WideString().data());
}

plExportDlg& plExportDlg::Instance()
{
    static plExportDlgImp theInstance;
    return theInstance;
}

INT_PTR plExportDlgImp::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return ((plExportDlgImp&)Instance()).DlgProc(hDlg, msg, wParam, lParam);
}

const char* plExportDlgImp::GetExportPage()
{
    if (fExportPage[0] == '\0')
        return nullptr;
    else
        return fExportPage;
}

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

//  EnableWindow(GetDlgItem(hDlg, IDC_EXPORT_PATH), !fExportFile);
    EnableWindow(GetDlgItem(hDlg, IDC_BROWSE_EXPORT), !fExportFile);
}

void plExportDlgImp::IInitDlg(HWND hDlg)
{
    // Set the client path
    plFileName path = plMaxConfig::GetClientPath(false, true);
    SetDlgItemText(hDlg, IDC_CLIENT_PATH, path.AsString().c_str());

    // Set the preshade button
    CheckDlgButton(hDlg, IDC_PRESHADE_CHECK, fPreshade ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_PHYSICAL_CHECK, fPhysicalsOnly ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_LIGHTMAP_CHECK, fLightMap ? BST_CHECKED : BST_UNCHECKED);

    char buf[256];
    sprintf(buf, "Last export took %d:%02d", fLastExportTime/60, fLastExportTime%60);
    SetDlgItemText(hDlg, IDC_LAST_EXPORT, buf);

    SetWindowPos(hDlg, nullptr, fXPos, fYPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

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

    SetDlgItemTextW(hDlg, IDC_EXPORT_PATH, fExportSourceDir.WideString().data());
}

#include "plFile/plBrowseFolder.h"

INT_PTR plExportDlgImp::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
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
                    plFileName path = plMaxConfig::GetClientPath(true);
                    if (path.IsValid())
                        SetDlgItemTextW(hDlg, IDC_CLIENT_PATH, path.WideString().data());
                    return TRUE;
                }
                else if (resID == IDC_RADIO_FILE || resID == IDC_RADIO_DIR)
                {
                    IGetRadio(hDlg);
                    return TRUE;
                }
                else if (resID == IDC_BROWSE_EXPORT)
                {
                    fExportSourceDir = plBrowseFolder::GetFolder(fExportSourceDir,
                                              "Choose the source directory",
                                              hDlg);
                    SetDlgItemTextW(hDlg, IDC_EXPORT_PATH, fExportSourceDir.WideString().data());
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

void plExportDlgImp::IDoExport()
{
    fExporting = true;

    // Hide the window, since we don't get control back until the export is done
    ShowWindow(fDlg, SW_HIDE);

    // Do the export
    wchar_t exportPathTEMP[MAX_PATH];
    GetDlgItemTextW(fDlg, IDC_CLIENT_PATH, exportPathTEMP, std::size(exportPathTEMP));
    plFileName exportPath = plFileName::Join(ST::string::from_wchar(exportPathTEMP), "Export.prd");

    // For export time stats
    DWORD exportTime = timeGetTime();

    if (fExportFile)
        IExportCurrentFile(exportPath.AsString().c_str());
    else
    {
        std::vector<plFileName> sources = plFileSystem::ListDir(fExportSourceDir, "*.max");
        for (auto iter = sources.begin(); iter != sources.end(); ++iter)
        {
            if (GetCOREInterface()->LoadFromFile(iter->AsString().c_str()))
                IExportCurrentFile(exportPath.AsString().c_str());
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
        fDlg = nullptr;
    }
}

void plExportDlgImp::Show()
{
    if (!fDlg)
        fDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_EXPORT), GetCOREInterface()->GetMAXHWnd(), ForwardDlgProc);
}

static bool IsExcluded(const plFileName& fileName, std::vector<plFileName>& excludeFiles)
{
    for (int i = 0; i < excludeFiles.size(); i++)
    {
        if (fileName == excludeFiles[i])
            return true;
    }

    return false;
}

static bool AutoExportDir(const char* inputDir, const char* outputDir, const plFileName& groupFiles, std::vector<plFileName>& excludeFiles)
{
    bool exportedFile = false;

    char outputFileName[MAX_PATH];
    sprintf(outputFileName, "%s\\Export.prd", outputDir);

    char outputLog[MAX_PATH];
    sprintf(outputLog, "%s\\AutoExport.log", outputDir);
    
    char doneDir[MAX_PATH];
    sprintf(doneDir, "%s\\Done\\", inputDir);
    CreateDirectory(doneDir, nullptr);

    // Don't give missing bitmap warnings
    TheManager->SetSilentMode(TRUE);

    std::vector<plFileName> sources = plFileSystem::ListDir(inputDir, "*.max");
    for (auto iter = sources.begin(); iter != sources.end(); ++iter)
    {
        if (IsExcluded(iter->GetFileName(), excludeFiles))
            continue;

        // If we're doing grouped files, and this isn't one, keep looking
        if (groupFiles.IsValid() && groupFiles != iter->GetFileName())
            continue;

        hsUNIXStream log;
        if (log.Open(outputLog, "ab"))
        {
            log.WriteFmt("{}\r\n", iter->GetFileName());
            log.Close();
        }

        if (GetCOREInterface()->LoadFromFile(iter->AsString().c_str()))
        {
            plFileSystem::Move(*iter, plFileName::Join(inputDir, "Done", iter->GetFileName()));

            GetCOREInterface()->ExportToFile(outputFileName, TRUE);
            exportedFile = true;

            // If we're not doing grouped files, this is it, we exported our one file
            if (!groupFiles.IsValid())
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

static void GetFileNameSection(const char* configFile, const char* keyName, std::vector<plFileName>& strings)
{
    char source[256];
    GetPrivateProfileString("Settings", keyName, "", source, sizeof(source), configFile);

    char* seps = ",";
    char* token = strtok(source, seps);
    while (token != nullptr)
    {
        strings.push_back(token);
        token = strtok(nullptr, seps);
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

    if (inputDir[0] == '\0' || outputDir[0] == '\0')
        return;

    fAutoExporting = true;

    // If we're doing an autoexport, suppress prompts now
    hsMessageBox_SuppressPrompts = true;

    // Files to ignore
    std::vector<plFileName> excludeFiles;
    GetFileNameSection(configFile, "ExcludeFiles", excludeFiles);

    //
    // Get the file substrings to export in one session
    //
    std::vector<plFileName> groupedFiles;
    GetFileNameSection(configFile, "GroupedFiles", groupedFiles);

    for (int i = 0; i < groupedFiles.size(); i++)
    {
        if (AutoExportDir(inputDir, outputDir, groupedFiles[i], excludeFiles))
        {
            ShutdownMax();
            fAutoExporting = false;
            return;
        }
    }

    if (AutoExportDir(inputDir, outputDir, "", excludeFiles))
    {
        ShutdownMax();
        fAutoExporting = false;
        return;
    }

    DeleteFile(configFile);

    fAutoExporting = false;
    ShutdownMax();
}
