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
#include "hsStringTokenizer.h"

#include "hsWindows.h"
#include <max.h>
#include "resource.h"
#include <vector>

#include "plPickLocalizationDlg.h"
#include "pfLocalizationMgr/pfLocalizationDataMgr.h"

#include "MaxMain/plMaxCFGFile.h"
#include "MaxMain/plMaxAccelerators.h"

////////////////////////////////////////////////////////////////////

bool plPickLocalizationDlg::DoPick()
{
    plMaxAccelerators::Disable();

    INT_PTR ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PICK_LOCALIZATION),
        GetCOREInterface()->GetMAXHWnd(), IDlgProc, (LPARAM)this);

    plMaxAccelerators::Enable();

    return (ret != 0);
}

hsStringTokenizer locIzer;
char locToken[200];

bool plPickLocalizationDlg::IInitDlg(HWND hDlg)
{
    if (!pfLocalizationDataMgr::InstanceValid())
    {
        MessageBox(hDlg, "Localization data manger is not initialized! (BTW, this is BAD)", "Error", MB_ICONERROR + MB_OK);
        return false;
    }

    fTree = GetDlgItem(hDlg, IDC_LOCALIZATIONTREE);
    TreeView_DeleteAllItems(fTree);

    std::string ageName = "", setName = "", itemName = "";
    locIzer.Reset(fPath.c_str(), ".");
    if (locIzer.Next(locToken, 200))
        ageName = locToken;
    if (locIzer.Next(locToken, 200))
        setName = locToken;
    if (locIzer.Next(locToken, 200))
        itemName = locToken;

    IAddLocalizations(ageName, setName, itemName);
    IUpdateValue(hDlg);

    return true;
}

std::string WStringToString(std::wstring val)
{
    std::string retVal;
    char *buff = hsWStringToString(val.c_str());
    retVal = buff;
    delete [] buff;
    return retVal;
}

HTREEITEM plPickLocalizationDlg::IAddVar(std::string name, std::string match, HTREEITEM hParent)
{
    TVINSERTSTRUCT tvi = {};
    tvi.hParent = hParent;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = (char*)name.c_str();
    tvi.item.cchTextMax = name.length();
    tvi.item.lParam = (LPARAM)nullptr;

    HTREEITEM hItem = TreeView_InsertItem(fTree, &tvi);

    if (name == match)
    {
        TreeView_SelectItem(fTree, hItem);
        TreeView_EnsureVisible(fTree, hItem);
    }

    return hItem;
}

void plPickLocalizationDlg::IAddLocalizations(std::string ageName, std::string setName, std::string itemName)
{
    std::vector<ST::string> ages = pfLocalizationDataMgr::Instance().GetAgeList();

    for (int curAge = 0; curAge < ages.size(); curAge++)
    {
        HTREEITEM hAgeItem = IAddVar(ages[curAge].c_str(), ageName, TVI_ROOT);

        std::vector<ST::string> sets = pfLocalizationDataMgr::Instance().GetSetList(ages[curAge]);
        for (int curSet = 0; curSet < sets.size(); curSet++)
        {
            std::vector<ST::string> elements = pfLocalizationDataMgr::Instance().GetElementList(ages[curAge], sets[curSet]);

            HTREEITEM hSetItem = IAddVar(sets[curSet].c_str(), setName, hAgeItem);
            for (int curElement = 0; curElement < elements.size(); curElement++)
                IAddVar(elements[curElement].c_str(), itemName, hSetItem);
        }
    }
}

void plPickLocalizationDlg::IUpdateValue(HWND hDlg)
{
    fPath = "";

    HTREEITEM hItem = TreeView_GetSelection(fTree);

    std::vector<std::string> path;
    while (hItem)
    {
        char s[200];
        TVITEM tvi = {0};
        tvi.hItem = hItem;
        tvi.mask = TVIF_TEXT;
        tvi.pszText = s;
        tvi.cchTextMax = 200;
        TreeView_GetItem(fTree, &tvi);
        path.push_back(tvi.pszText);
        hItem = TreeView_GetParent(fTree, hItem);
    }

    while (!path.empty())
    {
        fPath.append(path.back());
        path.pop_back();
        if (!path.empty())
            fPath.append(".");
    }

    SetDlgItemText(hDlg, IDC_LOCALIZATIONSTRING, fPath.c_str());

    IUpdateOkBtn(hDlg);
}

void plPickLocalizationDlg::IUpdateOkBtn(HWND hDlg)
{
    HWND hOk = GetDlgItem(hDlg, IDOK);

    char s[512];
    GetDlgItemText(hDlg, IDC_LOCALIZATIONSTRING, s, 511);

    EnableWindow(hOk, strlen(s)>0 && IValidatePath());
}

bool plPickLocalizationDlg::IValidatePath()
{
    std::string ageName = "", setName = "", itemName = "";
    locIzer.Reset(fPath.c_str(), ".");
    if (locIzer.Next(locToken, 200))
        ageName = locToken;
    if (locIzer.Next(locToken, 200))
        setName = locToken;
    if (locIzer.Next(locToken, 200))
        itemName = locToken;

    if (ageName == "")
        return false; // no age, so not valid
    if (setName == "")
        return false; // no set, so not valid
    if (itemName == "")
        return false; // no item, so not valid
    return true;
}

INT_PTR CALLBACK plPickLocalizationDlg::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static plPickLocalizationDlg* pthis = nullptr;

    switch (msg)
    {
    case WM_INITDIALOG:
        pthis = (plPickLocalizationDlg*)lParam;
        if (!pthis->IInitDlg(hDlg))
            EndDialog(hDlg, 0);
        return FALSE;

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK)
        {
            EndDialog(hDlg, 1);
            return TRUE;
        }
        else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam)
        {
        case SC_CLOSE:
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        NMHDR *nmhdr = (NMHDR*)lParam;
        if (nmhdr->idFrom == IDC_LOCALIZATIONTREE)
        {
            switch (nmhdr->code)
            {
            case TVN_SELCHANGED:
                pthis->IUpdateValue(hDlg);
                return TRUE;

            case NM_DBLCLK:
                if (pthis->IValidatePath()) // only close the dialog if it's a valid path
                    EndDialog(hDlg, 1);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}
