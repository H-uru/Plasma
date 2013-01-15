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
// Basic edit dialog stuff
#include "plEditDlg.h"
#include "res/resource.h"
#include "plLocTreeView.h"
#include "plAddDlgs.h"

#include "pfLocalizationMgr/pfLocalizationDataMgr.h"

#include <map>

HWND gEditDlg = NULL;
extern HINSTANCE gInstance;
extern HWND gTreeView;

// global data for this dialog
plString gCurrentPath;

// split a subtitle path up into its component parts
void SplitLocalizationPath(plString path, plString &ageName, plString &setName, plString &locName, plString &locLanguage)
{
    ageName = setName = locName = locLanguage = "";

    std::vector<plString> tokens = path.Tokenize(".");
    if (tokens.size() >= 1)
        ageName = tokens[0];
    if (tokens.size() >= 2)
        setName = tokens[1];
    if (tokens.size() >= 3)
        locName = tokens[2];
    if (tokens.size() >= 4)
        locLanguage = tokens[3];
}

// saves the current localization text to the data manager
void SaveLocalizationText()
{
    if (gCurrentPath.IsEmpty())
        return; // no path to save

    uint32_t textLen = (uint32_t)SendMessage(GetDlgItem(gEditDlg, IDC_LOCALIZATIONTEXT), WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
    wchar_t *buffer = new wchar_t[textLen + 2];
    GetDlgItemTextW(gEditDlg, IDC_LOCALIZATIONTEXT, buffer, textLen + 1);
    buffer[textLen + 1] = 0;
    plString plainTextData = plString::FromWchar(buffer);
    delete [] buffer;
    plString ageName, setName, elementName, elementLanguage;
    SplitLocalizationPath(gCurrentPath, ageName, setName, elementName, elementLanguage);

    plString name = plString::Format("%s.%s.%s", ageName.c_str(), setName.c_str(), elementName.c_str());
    pfLocalizationDataMgr::Instance().SetElementPlainTextData(name, elementLanguage, plainTextData);
}

// Reset all controls to their default values (except for static controls)
void ResetDlgDefaults()
{
    SetDlgItemTextW(gEditDlg, IDC_LOCALIZATIONTEXT, L"");
}

// Enable/disable all edit controls (some won't enable/disable unless an audio file is loaded and the subtitle is timed)
void EnableDlg(BOOL enable)
{
    if (!enable)
        ResetDlgDefaults(); // reset controls to defaults

    EnableWindow(GetDlgItem(gEditDlg, IDC_LOCALIZATIONTEXT), enable);
}

// updates the edit dialog based on the path specified
void UpdateEditDlg(plString locPath)
{
    if (locPath == gCurrentPath)
        return;

    gCurrentPath = locPath;

    plString itemText = plString::Format("Text (%s):", locPath.c_str());
    SetDlgItemTextW(gEditDlg, IDC_LOCPATH, itemText.ToWchar());

    plString ageName, setName, elementName, elementLanguage;
    SplitLocalizationPath(locPath, ageName, setName, elementName, elementLanguage);

    // now make sure they've drilled down deep enough to enable the dialog
    if (elementLanguage.IsEmpty()) // not deep enough
        EnableDlg(FALSE);
    else
    {
        EnableDlg(TRUE);
        plString key = plString::Format("%s.%s.%s", ageName.c_str(), setName.c_str(), elementName.c_str());
        plString elementText = pfLocalizationDataMgr::Instance().GetElementPlainTextData(key, elementLanguage);
        SetDlgItemTextW(gEditDlg, IDC_LOCALIZATIONTEXT, elementText.ToWchar());
    }

    // now to setup the add/delete buttons
    if (!elementLanguage.IsEmpty()) // they have selected a language
    {
        SetDlgItemText(gEditDlg, IDC_ADD, L"Add Localization");
        EnableWindow(GetDlgItem(gEditDlg, IDC_ADD), TRUE);
        SetDlgItemText(gEditDlg, IDC_DELETE, L"Delete Localization");
        if (elementLanguage != "English") // don't allow them to delete the default language
            EnableWindow(GetDlgItem(gEditDlg, IDC_DELETE), TRUE);
        else
            EnableWindow(GetDlgItem(gEditDlg, IDC_DELETE), FALSE);
    }
    else // they have selected something else
    {
        SetDlgItemText(gEditDlg, IDC_ADD, L"Add Element");
        EnableWindow(GetDlgItem(gEditDlg, IDC_ADD), TRUE);
        SetDlgItemText(gEditDlg, IDC_DELETE, L"Delete Element");
        if (!elementName.IsEmpty()) // they have selected an individual element
        {
            std::vector<plString> elementNames = pfLocalizationDataMgr::Instance().GetElementList(ageName, setName);
            if (elementNames.size() > 1) // they can't delete the only subtitle in a set
                EnableWindow(GetDlgItem(gEditDlg, IDC_DELETE), TRUE);
            else
                EnableWindow(GetDlgItem(gEditDlg, IDC_DELETE), FALSE);
        }
        else
            EnableWindow(GetDlgItem(gEditDlg, IDC_DELETE), FALSE);
    }
}

BOOL HandleCommandMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int wmID, wmEvent;
    wmID = LOWORD(wParam);
    wmEvent = HIWORD(wParam);

    switch (wmEvent)
    {
    case BN_CLICKED:
        switch (wmID)
        {
        case IDC_ADD:
            {
                SaveLocalizationText(); // save any current changes to the database

                plString buttonText;
                wchar_t buff[256];
                GetDlgItemText(gEditDlg, IDC_ADD, buff, 256);
                buttonText = plString::FromWchar(buff);

                if (buttonText == "Add Element")
                {
                    plAddElementDlg dlg(gCurrentPath);
                    if (dlg.DoPick(gEditDlg))
                    {
                        plString path = dlg.GetValue(); // path is age.set.name
                        if (!pfLocalizationDataMgr::Instance().AddElement(path))
                            MessageBox(gEditDlg, L"Couldn't add new element because one already exists with that name!", L"Error", MB_ICONERROR | MB_OK);
                        else
                        {
                            gCurrentPath = "";
                            plLocTreeView::ClearTreeView(gTreeView);
                            plLocTreeView::FillTreeViewFromData(gTreeView, path);
                            UpdateEditDlg(path);
                        }
                    }
                }
                else if (buttonText == "Add Localization")
                {
                    plAddLocalizationDlg dlg(gCurrentPath);
                    if (dlg.DoPick(gEditDlg))
                    {
                        plString newLanguage = dlg.GetValue();
                        plString ageName, setName, elementName, elementLanguage;
                        SplitLocalizationPath(gCurrentPath, ageName, setName, elementName, elementLanguage);
                        plString key = plString::Format("%s.%s.%s", ageName.c_str(), setName.c_str(), elementName.c_str());
                        if (!pfLocalizationDataMgr::Instance().AddLocalization(key, newLanguage))
                            MessageBox(gEditDlg, L"Couldn't add additional localization!", L"Error", MB_ICONERROR | MB_OK);
                        else
                        {
                            plString path = plString::Format("%s.%s", key.c_str(), newLanguage.c_str());
                            gCurrentPath = "";
                            plLocTreeView::ClearTreeView(gTreeView);
                            plLocTreeView::FillTreeViewFromData(gTreeView, path);
                            UpdateEditDlg(path);
                        }
                    }
                }
                return FALSE;
            }
        case IDC_DELETE:
            {
                SaveLocalizationText(); // save any current changes to the database

                plString messageText = plString::Format("Are you sure that you want to delete %s?", gCurrentPath.c_str());
                int res = MessageBoxW(gEditDlg, messageText.ToWchar(), L"Delete", MB_ICONQUESTION | MB_YESNO);
                if (res == IDYES)
                {
                    plString buttonText;
                    wchar_t buff[256];
                    GetDlgItemText(gEditDlg, IDC_DELETE, buff, 256);
                    buttonText = plString::FromWchar(buff);

                    if (buttonText == "Delete Element")
                    {
                        if (!pfLocalizationDataMgr::Instance().DeleteElement(gCurrentPath))
                            MessageBox(gEditDlg, L"Couldn't delete element!", L"Error", MB_ICONERROR | MB_OK);
                        else
                        {
                            plString path = gCurrentPath;
                            gCurrentPath = "";
                            plLocTreeView::ClearTreeView(gTreeView);
                            plLocTreeView::FillTreeViewFromData(gTreeView, path);
                            UpdateEditDlg(path);
                        }
                    }
                    else if (buttonText == "Delete Localization")
                    {
                        plString ageName, setName, elementName, elementLanguage;
                        SplitLocalizationPath(gCurrentPath, ageName, setName, elementName, elementLanguage);
                        plString key = plString::Format("%s.%s.%s", ageName.c_str(), setName.c_str(), elementName.c_str());
                        if (!pfLocalizationDataMgr::Instance().DeleteLocalization(key, elementLanguage))
                            MessageBox(gEditDlg, L"Couldn't delete localization!", L"Error", MB_ICONERROR | MB_OK);
                        else
                        {
                            plString path = gCurrentPath;
                            gCurrentPath = "";
                            plLocTreeView::ClearTreeView(gTreeView);
                            plLocTreeView::FillTreeViewFromData(gTreeView, path);
                            UpdateEditDlg(path);
                        }
                    }
                }
            }
            return FALSE;
        }
    }
    return (BOOL)DefWindowProc(hWnd, msg, wParam, lParam);
}

// our dialog's window procedure
BOOL CALLBACK EditDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        {
            gEditDlg = hWnd;
            EnableDlg(FALSE);
        }
        break;

    case WM_COMMAND:
        return HandleCommandMessage(hWnd, msg, wParam, lParam);
    }

    return FALSE;
}
