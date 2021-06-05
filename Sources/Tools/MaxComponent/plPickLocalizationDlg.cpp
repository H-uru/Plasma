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

#include "MaxMain/MaxAPI.h"
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
        plMaxMessageBox(hDlg, _T("Localization data manger is not initialized! (BTW, this is BAD)"), _T("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    fTree = GetDlgItem(hDlg, IDC_LOCALIZATIONTREE);
    TreeView_DeleteAllItems(fTree);

    ST::string ageName, setName, itemName;
    auto tokens = fPath.split('.');
    if (tokens.size() >= 1)
        ageName = tokens[0];
    if (tokens.size() >= 2)
        setName = tokens[1];
    if (tokens.size() >= 3)
        itemName = tokens[2];

    IAddLocalizations(ageName, setName, itemName);
    IUpdateValue(hDlg);

    return true;
}

HTREEITEM plPickLocalizationDlg::IAddVar(const ST::string& name, const ST::string& match, HTREEITEM hParent)
{
    ST::wchar_buffer nameBuf = name.to_wchar();

    TVINSERTSTRUCTW tvi = {0};
    tvi.hParent = hParent;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = nameBuf.data();
    tvi.item.cchTextMax = nameBuf.size();

    HTREEITEM hItem = (HTREEITEM)SendMessageW(fTree, TVM_INSERTITEM, 0, (LPARAM)&tvi);

    if (name == match)
    {
        TreeView_SelectItem(fTree, hItem);
        TreeView_EnsureVisible(fTree, hItem);
    }

    return hItem;
}

void plPickLocalizationDlg::IAddLocalizations(const ST::string& ageName, const ST::string& setName, const ST::string& itemName)
{
    std::vector<ST::string> ages = pfLocalizationDataMgr::Instance().GetAgeList();

    for (int curAge = 0; curAge < ages.size(); curAge++)
    {
        HTREEITEM hAgeItem = IAddVar(ages[curAge], ageName, TVI_ROOT);

        std::vector<ST::string> sets = pfLocalizationDataMgr::Instance().GetSetList(ages[curAge]);
        for (int curSet = 0; curSet < sets.size(); curSet++)
        {
            std::vector<ST::string> elements = pfLocalizationDataMgr::Instance().GetElementList(ages[curAge], sets[curSet]);

            HTREEITEM hSetItem = IAddVar(sets[curSet], setName, hAgeItem);
            for (int curElement = 0; curElement < elements.size(); curElement++)
                IAddVar(elements[curElement], itemName, hSetItem);
        }
    }
}

void plPickLocalizationDlg::IUpdateValue(HWND hDlg)
{
    fPath.clear();

    HTREEITEM hItem = TreeView_GetSelection(fTree);

    std::vector<ST::string> path;
    while (hItem)
    {
        TCHAR s[200];
        TVITEM tvi = {0};
        tvi.hItem = hItem;
        tvi.mask = TVIF_TEXT;
        tvi.pszText = s;
        tvi.cchTextMax = 200;
        TreeView_GetItem(fTree, &tvi);
        path.emplace_back(T2ST(tvi.pszText));
        hItem = TreeView_GetParent(fTree, hItem);
    }

    ST::string_stream ss;
    while (!path.empty())
    {
        ss << path.back();
        path.pop_back();
        if (!path.empty())
            ss << '.';
    }
    fPath = ss.to_string();

    SetDlgItemText(hDlg, IDC_LOCALIZATIONSTRING, ST2T(fPath));

    IUpdateOkBtn(hDlg);
}

void plPickLocalizationDlg::IUpdateOkBtn(HWND hDlg)
{
    HWND hOk = GetDlgItem(hDlg, IDOK);

    TCHAR s[512];
    GetDlgItemText(hDlg, IDC_LOCALIZATIONSTRING, s, 511);

    EnableWindow(hOk, *s != _T('\0') && IValidatePath());
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
