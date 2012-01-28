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
// basic classes for encapsulating the add dialogs

#include "res/resource.h"
#include "plAddDlgs.h"
#include "plEditDlg.h"


#include "plResMgr/plLocalization.h"
#include "pfLocalizationMgr/pfLocalizationDataMgr.h"

#include <vector>
#include <map>

extern HINSTANCE gInstance;

// very simple subclass for edit controls (and combo boxes) so that they only accept alphanumeric values
class AlphaNumericEditCtrl
{
    int fCtrlID;
    HWND fOwner, fEditBox;
    LONG_PTR fPrevProc;

public:
    AlphaNumericEditCtrl() : fCtrlID(0), fOwner(NULL), fEditBox(NULL), fPrevProc(NULL) {}
    ~AlphaNumericEditCtrl() {}

    void Setup(int ctrlID, HWND owner, bool comboBox);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

std::map<int, AlphaNumericEditCtrl> editBoxMap;

// basic setup of the edit control
void AlphaNumericEditCtrl::Setup(int ctrlID, HWND owner, bool comboBox)
{
    fCtrlID = ctrlID;
    fOwner = owner;

    // if we're a combo box, we need to subclass the edit control, not the combo box
    if (comboBox)
    {
        COMBOBOXINFO cbinfo;
        cbinfo.cbSize = sizeof(COMBOBOXINFO);
        GetComboBoxInfo(GetDlgItem(fOwner, fCtrlID), &cbinfo);
        fEditBox = cbinfo.hwndItem;
    }
    else
        fEditBox = GetDlgItem(fOwner, fCtrlID);

    // subclass the edit box so we can filter input (don't ask me why we have to double cast the
    // function pointer to get rid of the compiler warning)
    fPrevProc = SetWindowLongPtr(fEditBox, GWLP_WNDPROC, (LONG)(LONG_PTR)AlphaNumericEditCtrl::WndProc);

    editBoxMap[fCtrlID] = *this;
}

// Message handler for our edit box
LRESULT CALLBACK AlphaNumericEditCtrl::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int ctrlID = GetDlgCtrlID(hWnd);
    if (editBoxMap.find(ctrlID) == editBoxMap.end()) // control ID doesn't exist, so it's probably a combo boxes' edit ctrl
        ctrlID = GetDlgCtrlID(GetParent(hWnd)); // so grab the parent's ID number instead
    switch (message)
    {
    case WM_CHAR:
        {
            AlphaNumericEditCtrl editBox = editBoxMap[ctrlID];
            char theChar = (char)wParam;

            // we only accept 0-9, a-z, A-Z, or backspace
            if ((theChar < '0' || theChar > '9') && (theChar < 'a' || theChar > 'z') && (theChar < 'A' || theChar >'Z') && !(theChar == VK_BACK))
            {
                MessageBeep(-1); // alert the user
                return FALSE; // and make sure the default handler doesn't get it
            }
        }
    }
    // Any messages we don't process must be passed onto the original window function
    return CallWindowProc((WNDPROC)editBoxMap[ctrlID].fPrevProc, hWnd, message, wParam, lParam);
}

// plAddElementDlg - dialog for adding a single element
BOOL CALLBACK plAddElementDlg::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static plAddElementDlg* pthis = NULL;

    switch (msg)
    {
    case WM_INITDIALOG:
        pthis = (plAddElementDlg*)lParam;
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
        else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_PARENTAGE)
        {
            wchar_t buff[256];
            // we do this whole get sel, get item because get text won't return the updated text
            int index = (int)SendMessage(GetDlgItem(hDlg, IDC_PARENTAGE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(GetDlgItem(hDlg, IDC_PARENTAGE), CB_GETLBTEXT, (WPARAM)index, (LPARAM)buff);

            pthis->fAgeName = buff;
            pthis->fAgeChanged = true;
            pthis->IUpdateDlg(hDlg);
        }
        else if (HIWORD(wParam) == CBN_EDITCHANGE && LOWORD(wParam) == IDC_PARENTAGE)
        {
            wchar_t buff[256];
            GetDlgItemTextW(hDlg, IDC_PARENTAGE, buff, 256);

            pthis->fAgeName = buff;
            pthis->fAgeChanged = true;
            pthis->IUpdateDlg(hDlg, false);
        }
        else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_PARENTSET)
        {
            wchar_t buff[256];
            // we do this whole get sel, get item because get text won't return the updated text
            int index = (int)SendMessage(GetDlgItem(hDlg, IDC_PARENTSET), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(GetDlgItem(hDlg, IDC_PARENTSET), CB_GETLBTEXT, (WPARAM)index, (LPARAM)buff);

            pthis->fSetName = buff;
            pthis->IUpdateDlg(hDlg);
        }
        else if (HIWORD(wParam) == CBN_EDITCHANGE && LOWORD(wParam) == IDC_PARENTSET)
        {
            wchar_t buff[256];
            GetDlgItemTextW(hDlg, IDC_PARENTSET, buff, 256);

            pthis->fSetName = buff;
            pthis->IUpdateDlg(hDlg, false);
        }
        else if (HIWORD(wParam) == EN_UPDATE && LOWORD(wParam) == IDC_ELEMENTNAME)
        {
            wchar_t buff[256];
            GetDlgItemTextW(hDlg, IDC_ELEMENTNAME, buff, 256);
            pthis->fElementName = buff;

            pthis->IUpdateDlg(hDlg);
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
    }
    return FALSE;
}

bool plAddElementDlg::IInitDlg(HWND hDlg)
{
    HWND listCtrl = GetDlgItem(hDlg, IDC_PARENTAGE);
    std::vector<std::wstring> ageNames = pfLocalizationDataMgr::Instance().GetAgeList();

    // add the age names to the list
    for (int i = 0; i < ageNames.size(); i++)
        SendMessage(listCtrl, CB_ADDSTRING, (WPARAM)0, (LPARAM)ageNames[i].c_str());

    // select the age we were given
    SendMessage(listCtrl, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)fAgeName.c_str());

    AlphaNumericEditCtrl ageCtrl, setCtrl, subCtrl;
    ageCtrl.Setup(IDC_PARENTAGE, hDlg, true);
    setCtrl.Setup(IDC_PARENTSET, hDlg, true);
    subCtrl.Setup(IDC_ELEMENTNAME, hDlg, false);

    fAgeChanged = true;

    IUpdateDlg(hDlg);
    return true;
}

void plAddElementDlg::IUpdateDlg(HWND hDlg, bool setFocus)
{
    std::wstring pathStr = L"Path: " + fAgeName + L"." + fSetName + L"." + fElementName;
    SetDlgItemTextW(hDlg, IDC_PATH, pathStr.c_str());

    if (fAgeChanged) // we only update this if the age changed (saves time and prevents weird bugs, like typing backwards)
    {
        // now add the sets
        HWND listCtrl = GetDlgItem(hDlg, IDC_PARENTSET);
        SendMessage(listCtrl, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
        std::vector<std::wstring> setNames = pfLocalizationDataMgr::Instance().GetSetList(fAgeName);

        // add the set names to the list
        for (int i = 0; i < setNames.size(); i++)
            SendMessage(listCtrl, CB_ADDSTRING, (WPARAM)0, (LPARAM)setNames[i].c_str());

        // select the set we currently have
        int ret = (int)SendMessage(listCtrl, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)fSetName.c_str());
        if (ret == CB_ERR) // couldn't find the string, so just set it as the current string in the edit box
            SetDlgItemTextW(hDlg, IDC_PARENTSET, fSetName.c_str());

        fAgeChanged = false;
    }

    if (fSetName != L"" && setFocus)
        SetFocus(GetDlgItem(hDlg, IDC_ELEMENTNAME));

    if (fSetName != L"" && fElementName != L"")
        EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
    else
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
}

plAddElementDlg::plAddElementDlg(std::wstring parentPath)
{
    // throw away vars
    std::wstring element, lang;

    SplitLocalizationPath(parentPath, fAgeName, fSetName, element, lang);
}

bool plAddElementDlg::DoPick(HWND parent)
{
    INT_PTR ret = DialogBoxParam(gInstance, MAKEINTRESOURCE(IDD_ADDELEMENT),
        parent, IDlgProc, (LPARAM)this);

    editBoxMap.clear();

    return (ret != 0);
}

// plAddLocalizationDlg - dialog for adding a single localization
BOOL CALLBACK plAddLocalizationDlg::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static plAddLocalizationDlg* pthis = NULL;

    switch (msg)
    {
    case WM_INITDIALOG:
        pthis = (plAddLocalizationDlg*)lParam;
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
        else if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_LANGUAGE)
        {
            wchar_t buff[256];
            // we do this whole get sel, get item because get text won't return the updated text
            int index = (int)SendMessage(GetDlgItem(hDlg, IDC_LANGUAGE), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            SendMessage(GetDlgItem(hDlg, IDC_LANGUAGE), CB_GETLBTEXT, (WPARAM)index, (LPARAM)buff);

            pthis->fLanguageName = buff;
            pthis->IUpdateDlg(hDlg);
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
    }
    return FALSE;
}

std::vector<std::wstring> IGetAllLanguageNames()
{
    int numLocales = plLocalization::GetNumLocales();
    std::vector<std::wstring> retVal;

    for (int curLocale = 0; curLocale <= numLocales; curLocale++)
    {
        const char *name = plLocalization::GetLanguageName((plLocalization::Language)curLocale);
        wchar_t *wName = hsStringToWString(name);
        retVal.push_back(wName);
        delete [] wName;
    }

    return retVal;
}

bool plAddLocalizationDlg::IInitDlg(HWND hDlg)
{
    std::wstring pathStr = L"Path: " + fAgeName + L"." + fSetName + L"." + fElementName;
    SetDlgItemTextW(hDlg, IDC_PATH, pathStr.c_str());

    std::vector<std::wstring> existingLanguages;
    existingLanguages = pfLocalizationDataMgr::Instance().GetLanguages(fAgeName, fSetName, fElementName);

    std::vector<std::wstring> missingLanguages = IGetAllLanguageNames();
    for (int i = 0; i < existingLanguages.size(); i++) // remove all languages we already have
    {
        for (int j = 0; j < missingLanguages.size(); j++)
        {
            if (missingLanguages[j] == existingLanguages[i])
            {
                missingLanguages.erase(missingLanguages.begin() + j);
                j--;
            }
        }
    }

    HWND listCtrl = GetDlgItem(hDlg, IDC_LANGUAGE);
    // see if any languages are missing
    if (missingLanguages.size() == 0)
    {
        // none are missing, so disable the control
        EnableWindow(listCtrl, FALSE);
        IUpdateDlg(hDlg);
        return true;
    }

    // add the missing languages to the list
    for (int i = 0; i < missingLanguages.size(); i++)
        SendMessage(listCtrl, CB_ADDSTRING, (WPARAM)0, (LPARAM)missingLanguages[i].c_str());

    // select the first language in the list
    SendMessage(listCtrl, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
    // and put it's value into the internal variable
    wchar_t buff[256];
    GetDlgItemText(hDlg, IDC_LANGUAGE, buff, 256);
    fLanguageName = buff;

    IUpdateDlg(hDlg);
    return true;
}

void plAddLocalizationDlg::IUpdateDlg(HWND hDlg)
{
    if (fLanguageName != L"")
        EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
    else
        EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
}

plAddLocalizationDlg::plAddLocalizationDlg(std::wstring parentPath)
{
    // throw away vars
    std::wstring lang;

    SplitLocalizationPath(parentPath, fAgeName, fSetName, fElementName, lang);
}

bool plAddLocalizationDlg::DoPick(HWND parent)
{
    INT_PTR ret = DialogBoxParam(gInstance, MAKEINTRESOURCE(IDD_ADDLOCALIZATION),
        parent, IDlgProc, (LPARAM)this);

    return (ret != 0);
}
