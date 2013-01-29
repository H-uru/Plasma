/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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
#define CLASSNAME L"plLocalizationEditor"
#define WINDOWNAME L"plLocalizationEditor"
#define IDC_REGTREEVIEW 1000

#define FILE_MENU_POS 0 // 0-based index of the file sub menu

#include "pnAllCreatables.h"
#include "plResMgr/plResMgrCreatable.h"

// These are so that we don't have to link in stuff we don't have to
#include "plMessage/plResMgrHelperMsg.h"
#include "plMessage/plAgeLoadedMsg.h"
REGISTER_CREATABLE(plResMgrHelperMsg);
REGISTER_CREATABLE(plAgeLoadedMsg);
REGISTER_CREATABLE(plAgeLoaded2Msg);
REGISTER_CREATABLE(plAgeBeginLoadingMsg);
REGISTER_CREATABLE(plInitialAgeStateLoadedMsg);

#include "pfLocalizationMgr/pfLocalizationMgr.h"
#include "pfLocalizationMgr/pfLocalizationDataMgr.h"
#include "plResMgr/plResManager.h"

#include "plLocTreeView.h"
#include "plEditDlg.h"

#include "HeadSpin.h"
#include <Commdlg.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "res/resource.h"

HINSTANCE gInstance = NULL;
HWND gMainWindow = NULL;
HWND gTreeView = NULL; // the tree view for display of localization strings
extern HWND gEditDlg; // the main edit dialog for the localization strings
std::wstring gCurPath = L""; // current data path

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL WinInit(HINSTANCE hInst, int nCmdShow);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    HACCEL accelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));

    if (!WinInit(hInst, nCmdShow))
        return -1;

    plResManager *rMgr = new plResManager;
    hsgResMgr::Init(rMgr);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(gMainWindow, accelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pfLocalizationMgr::Shutdown();

    hsgResMgr::Shutdown();

    return 0;
}

BOOL WinInit(HINSTANCE hInst, int nCmdShow)
{
    LoadLibrary(L"Riched20.dll"); // so we can use our rich edit control
    gInstance = hInst;

    WNDCLASSEX wcEx;
    wcEx.cbSize = sizeof(WNDCLASSEX);
    wcEx.style = CS_HREDRAW | CS_VREDRAW;
    wcEx.lpfnWndProc = MainWndProc;
    wcEx.cbClsExtra = 0;
    wcEx.cbWndExtra = 0;
    wcEx.hInstance = hInst;
    wcEx.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPICON));
    wcEx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcEx.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wcEx.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);
    wcEx.lpszClassName = CLASSNAME;
    wcEx.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    if (!RegisterClassEx(&wcEx))
        return FALSE;

    DWORD dwStyle = WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
    DWORD dwExStyle = WS_EX_CONTROLPARENT;

    // Create a window
    gMainWindow = CreateWindowEx(dwExStyle, CLASSNAME, WINDOWNAME, dwStyle, 10, 10, 800, 500, NULL, NULL, hInst, NULL);
    if (gMainWindow == NULL)
        return FALSE;

    return TRUE;
}

void SetWindowTitle(HWND hWnd, std::wstring path)
{
    std::wstring title = L"plLocalizationEditor";
    if (path != L"")
        title += L"-" + path;

    SetWindowText(hWnd, title.c_str());
}

BOOL CALLBACK AboutDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(msg == WM_COMMAND)
        EndDialog(hWnd, 0);
    return 0;
}

void RequestSaveOnExit()
{
    if (gCurPath == L"") // no data open
        return;

    static bool alreadyRequested = false; // make sure we don't ask multiple times
    if (alreadyRequested)
        return;
    alreadyRequested = true;

    SaveLocalizationText(); // make sure any changed text is saved to the manager

    int res = MessageBox(NULL, L"Do you wish to save your changes?", L"Save Changes", MB_ICONQUESTION | MB_YESNO);
    if (res == IDYES)
    {
        // save it to a new directory
        BROWSEINFO bInfo;
        LPITEMIDLIST itemList;
        LPMALLOC shMalloc;
        wchar_t path[MAX_PATH];

        memset(&bInfo, 0, sizeof(bInfo));
        bInfo.hwndOwner = NULL;
        bInfo.pidlRoot = NULL;
        bInfo.pszDisplayName = path;
        bInfo.lpszTitle = L"Select a directory to save the localization data to:";
        bInfo.ulFlags = BIF_EDITBOX;

        itemList = SHBrowseForFolder(&bInfo);
        if (itemList != NULL)
        {
            plWaitCursor waitCursor;

            SHGetPathFromIDList(itemList, path);
            SHGetMalloc(&shMalloc);
            shMalloc->Free(itemList);
            shMalloc->Release();

            gCurPath = path;
            char *sPath = hsWStringToString(gCurPath.c_str());
            pfLocalizationDataMgr::Instance().WriteDatabaseToDisk(sPath);
            delete [] sPath;
        }
    }
}

LRESULT CALLBACK HandleCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam))
    {
    case ID_FILE_EXIT:
        RequestSaveOnExit();
        PostQuitMessage(0);
        break;
    case ID_FILE_OPENDATADIRECTORY:
        {
            BROWSEINFO bInfo;
            LPITEMIDLIST itemList;
            LPMALLOC shMalloc;
            wchar_t path[MAX_PATH];

            memset(&bInfo, 0, sizeof(bInfo));
            bInfo.hwndOwner = hWnd;
            bInfo.pidlRoot = NULL;
            bInfo.pszDisplayName = path;
            bInfo.lpszTitle = L"Select a localization data directory:";
            bInfo.ulFlags = BIF_USENEWUI | BIF_VALIDATE | BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON;

            itemList = SHBrowseForFolder(&bInfo);
            if (itemList != NULL)
            {
                plWaitCursor waitCursor;

                SHGetPathFromIDList(itemList, path);
                SHGetMalloc(&shMalloc);
                shMalloc->Free(itemList);
                shMalloc->Release();

                pfLocalizationMgr::Shutdown();

                char *sPath = hsWStringToString(path);
                pfLocalizationMgr::Initialize(sPath);
                delete [] sPath;

                plLocTreeView::ClearTreeView(gTreeView);
                plLocTreeView::FillTreeViewFromData(gTreeView, "");

                gCurPath = path;
                SetWindowTitle(hWnd, path);

                HMENU menu = GetMenu(hWnd);
                HMENU fileMenu = GetSubMenu(menu, FILE_MENU_POS);
                EnableMenuItem(fileMenu, ID_FILE_SAVETOCUR, MF_ENABLED);
                EnableMenuItem(fileMenu, ID_FILE_SAVETONEW, MF_ENABLED);
            }
        }
        break;
    case ID_FILE_SAVETOCUR:
        {
            SaveLocalizationText(); // make sure any changed text is saved to the manager

            // save it to our current directory
            int res = MessageBox(hWnd, L"Are you sure you want to save to the current directory? Current data will be overwritten!", L"Save to Current Directory", MB_ICONQUESTION | MB_YESNOCANCEL);
            if (res == IDYES)
            {
                plWaitCursor waitCursor;
                char *sPath = hsWStringToString(gCurPath.c_str());
                pfLocalizationDataMgr::Instance().WriteDatabaseToDisk(sPath);
                delete [] sPath;
            }
            else if (res == IDNO)
                SendMessage(hWnd, WM_COMMAND, (WPARAM)ID_FILE_SAVETONEW, (LPARAM)0);
            // and if it's cancel we don't do anything
        }
        break;
    case ID_FILE_SAVETONEW:
        {
            SaveLocalizationText(); // make sure any changed text is saved to the manager

            // save it to a new directory
            BROWSEINFO bInfo;
            LPITEMIDLIST itemList;
            LPMALLOC shMalloc;
            wchar_t path[MAX_PATH];

            memset(&bInfo, 0, sizeof(bInfo));
            bInfo.hwndOwner = hWnd;
            bInfo.pidlRoot = NULL;
            bInfo.pszDisplayName = path;
            bInfo.lpszTitle = L"Select a directory to save the localization data to:";
            bInfo.ulFlags = BIF_EDITBOX;

            itemList = SHBrowseForFolder(&bInfo);
            if (itemList != NULL)
            {
                plWaitCursor waitCursor;

                SHGetPathFromIDList(itemList, path);
                SHGetMalloc(&shMalloc);
                shMalloc->Free(itemList);
                shMalloc->Release();

                gCurPath = path;
                SetWindowTitle(hWnd, path);
                char *sPath = hsWStringToString(gCurPath.c_str());
                pfLocalizationDataMgr::Instance().WriteDatabaseToDisk(sPath);
                delete [] sPath;
            }
        }
        break;

    case ID_HELP_ABOUT:
        DialogBox(gInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDialogProc);
        break;
    }
    return 0;
}

void SizeControls(HWND parent)
{
    RECT clientRect, editRect;

    GetClientRect(parent, &clientRect);
    GetClientRect(gEditDlg, &editRect);

    SetWindowPos(gTreeView, NULL, 0, 0, clientRect.right - editRect.right - 4, clientRect.bottom, 0);

    OffsetRect(&editRect, clientRect.right - editRect.right, (clientRect.bottom >> 1) - (editRect.bottom >> 1));
    SetWindowPos(gEditDlg, NULL, editRect.left, editRect.top, 0, 0, SWP_NOSIZE);
}

void InitWindowControls(HWND hWnd)
{
    RECT clientRect;

    GetClientRect(hWnd, &clientRect);

    gTreeView = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, L"Tree View", WS_VISIBLE | WS_CHILD | WS_BORDER |
        TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
        0, 0, 0, 0, hWnd, (HMENU)IDC_REGTREEVIEW, gInstance, NULL);

    gEditDlg = CreateDialog(gInstance, MAKEINTRESOURCE(IDD_EDITDLG), hWnd, EditDlgProc);

    SizeControls(hWnd);
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        InitCommonControls();
        InitWindowControls(hWnd);
        break;
    case WM_CLOSE:
        RequestSaveOnExit();
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        RequestSaveOnExit();
        plLocTreeView::ClearTreeView(gTreeView);
        PostQuitMessage(0);
        break;
    case WM_SIZING:
    case WM_SIZE:
        SizeControls(hWnd);
        break;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
            pmmi->ptMinTrackSize.x = 800;
            pmmi->ptMinTrackSize.y = 500;
            return 0;
        }
    case WM_NOTIFY:
        if(wParam == IDC_REGTREEVIEW)
        {
            SaveLocalizationText(); // save any current changes to the database

            NMHDR *hdr = (NMHDR*)lParam;
            if(hdr->code == TVN_SELCHANGED)
                plLocTreeView::SelectionChanged(gTreeView);
            else if(hdr->code == NM_DBLCLK)
                plLocTreeView::SelectionDblClicked(gTreeView);
            UpdateEditDlg(plLocTreeView::GetPath());
        }
        break;
    case WM_COMMAND:
        return HandleCommand(hWnd, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

/* Enable themes in Windows XP and later */
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
