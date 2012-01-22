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
#include "plLocTreeView.h"
#include "plEditDlg.h"

#include "HeadSpin.h"
#include <commctrl.h>
#include <shlwapi.h>
#include "res\resource.h"

#include <vector>
#include <string>


#include "pfLocalizationMgr/pfLocalizationDataMgr.h"

extern HINSTANCE gInstance;

std::wstring plLocTreeView::fPath = L"";

HTREEITEM AddLeaf(HWND hTree, HTREEITEM hParent, std::wstring text, bool sort = true)
{
    TVITEM tvi = {0};
    tvi.mask       = TVIF_TEXT | TVIF_PARAM;
    tvi.pszText    = (wchar_t*)text.c_str();
    tvi.cchTextMax = (int)text.length();
    tvi.lParam     = NULL;

    TVINSERTSTRUCT tvins = {0};
    tvins.item         = tvi;
    tvins.hParent      = hParent;
    if (sort)
        tvins.hInsertAfter = TVI_SORT;
    else
        tvins.hInsertAfter = TVI_LAST;

    return TreeView_InsertItem(hTree, &tvins);
}

void plLocTreeView::FillTreeViewFromData(HWND treeCtrl, std::wstring selectionPath)
{
    std::wstring targetAge, targetSet, targetElement, targetLang;
    SplitLocalizationPath(selectionPath, targetAge, targetSet, targetElement, targetLang);
    bool ageMatched = false;
    bool setMatched = false;
    bool elementMatched = false;

    std::vector<std::wstring> ages = pfLocalizationDataMgr::Instance().GetAgeList();
    for (int curAge = 0; curAge < ages.size(); curAge++)
    {
        // add the age to the tree
        HTREEITEM ageItem = AddLeaf(treeCtrl, NULL, ages[curAge]);

        if (ages[curAge] == targetAge)
        {
            TreeView_SelectItem(treeCtrl, ageItem);
            TreeView_EnsureVisible(treeCtrl, ageItem);
            ageMatched = true;
        }
        else
            ageMatched = false;

        std::vector<std::wstring> sets = pfLocalizationDataMgr::Instance().GetSetList(ages[curAge]);
        for (int curSet = 0; curSet < sets.size(); curSet++)
        {
            std::vector<std::wstring> elements = pfLocalizationDataMgr::Instance().GetElementList(ages[curAge], sets[curSet]);

            HTREEITEM setItem = AddLeaf(treeCtrl, ageItem, sets[curSet]);

            if ((sets[curSet] == targetSet) && ageMatched)
            {
                TreeView_SelectItem(treeCtrl, setItem);
                TreeView_EnsureVisible(treeCtrl, setItem);
                setMatched = true;
            }
            else
                setMatched = false;

            for (int curElement = 0; curElement < elements.size(); curElement++)
            {
                HTREEITEM subItem = AddLeaf(treeCtrl, setItem, elements[curElement]);

                if (elements[curElement] == targetElement && setMatched)
                {
                    TreeView_SelectItem(treeCtrl, subItem);
                    TreeView_EnsureVisible(treeCtrl, subItem);
                    elementMatched = true;

                    if (targetLang.empty())
                        targetLang = L"English";
                }
                else
                    elementMatched = false;

                std::vector<std::wstring> languages = pfLocalizationDataMgr::Instance().GetLanguages(ages[curAge], sets[curSet], elements[curElement]);
                for (int curLang = 0; curLang < languages.size(); curLang++)
                {
                    HTREEITEM langItem = AddLeaf(treeCtrl, subItem, languages[curLang]);

                    if (languages[curLang] == targetLang && elementMatched)
                    {
                        TreeView_SelectItem(treeCtrl, langItem);
                        TreeView_EnsureVisible(treeCtrl, langItem);
                    }
                }
            }
        }
    }
}

void plLocTreeView::ClearTreeView(HWND treeCtrl)
{
    TreeView_DeleteAllItems(treeCtrl);
}

void plLocTreeView::SelectionChanged(HWND treeCtrl)
{
    HTREEITEM hItem = TreeView_GetSelection(treeCtrl);
    std::vector<std::wstring> path;
    fPath = L"";

    while (hItem)
    {
        wchar_t s[200];
        TVITEM tvi = {0};
        tvi.hItem = hItem;
        tvi.mask = TVIF_TEXT;
        tvi.pszText = s;
        tvi.cchTextMax = 200;
        TreeView_GetItem(treeCtrl, &tvi);
        path.push_back(tvi.pszText);
        hItem = TreeView_GetParent(treeCtrl, hItem);
    }

    while (!path.empty())
    {
        fPath += path.back();

        path.pop_back();
        if (!path.empty())
            fPath += L".";
    }
}

void plLocTreeView::SelectionDblClicked(HWND treeCtrl)
{
    HTREEITEM hItem = TreeView_GetSelection(treeCtrl);
    std::vector<std::wstring> path;
    fPath = L"";

    while (hItem)
    {
        wchar_t s[200];
        TVITEM tvi = {0};
        tvi.hItem = hItem;
        tvi.mask = TVIF_TEXT;
        tvi.pszText = s;
        tvi.cchTextMax = 200;
        TreeView_GetItem(treeCtrl, &tvi);
        path.push_back(tvi.pszText);
        hItem = TreeView_GetParent(treeCtrl, hItem);
    }

    while (!path.empty())
    {
        fPath += path.back();

        path.pop_back();
        if (!path.empty())
            fPath += L".";
    }
}
