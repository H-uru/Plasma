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
#include "MaxAPI.h"

#include "pnKeyedObject/plKey.h"
#include "plMessageBox/hsMessageBox.h"

#include "plMaxNode.h"
#include "MaxComponent/plComponent.h"
#include "resource.h"

#include "plGetLocationDlg.h"

#include "MaxComponent/plMiscComponents.h"
#include "MaxExport/plErrorMsg.h"

plGetLocationDlg::plGetLocationDlg() : fNode(), fErrMsg(), fDefaultLocation()
{
}

plGetLocationDlg& plGetLocationDlg::Instance()
{
    static plGetLocationDlg theInstance;
    return theInstance;
}

bool plGetLocationDlg::GetLocation(plMaxNode *node, plErrorMsg *errMsg)
{
    fNode = node;
    fErrMsg = errMsg;

    // If an XRef doesn't have a location, tell the user and stop the export
    if (node->IsXRef())
    {
        fErrMsg->Set(
            true, "Convert Error",
            ST::format(
                "XRef object \"{}\" does not have a location",
                node->GetName()
            )
        ).Show();
        return false;
    }

    // If we have a default location, use it and exit
    if (fDefaultLocation)
    {
        ISetLocation(fDefaultLocation);
        return true;
    }

    // If we're not showing prompts, just fail if there isn't a location
    if (hsMessageBox_SuppressPrompts)
    {
        fErrMsg->Set(true, "Convert Error",
            ST::format("Object {} doesn't have a location component", M2ST(node->GetName()))
        );
        fErrMsg->Show();
        return false;
    }

    INT_PTR ret = DialogBox(hInstance,
                        MAKEINTRESOURCE(IDD_GET_LOCATION),
                        GetCOREInterface()->GetMAXHWnd(),
                        ForwardDlgProc);

    return (ret != 0);
}

void plGetLocationDlg::ResetDefaultLocation()
{
    fDefaultLocation = nullptr;
}

void plGetLocationDlg::IListRooms(plMaxNode *node, HWND hList)
{
    // If node is a room component, add it's name to the list
    plComponentBase *comp = node->ConvertToComponent();
    if(comp && (comp->ClassID() == ROOM_CID || comp->ClassID() == PAGEINFO_CID))
    {
        int idx = (int)SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)node->GetName());
        SendMessage(hList, LB_SETITEMDATA, idx, (LPARAM)node);
    }

    // Recursively add all the nodes children
    for (int i = 0; i < node->NumberOfChildren(); i++)
        IListRooms((plMaxNode*)node->GetChildNode(i), hList);
}

void plGetLocationDlg::IAddSelection(HWND hList, bool setDefault)
{
    int sel = (int)SendMessage(hList, LB_GETCURSEL, 0, 0);
    if (sel != LB_ERR)
    {
        // Get the node and component for the selected room component
        plMaxNode *node = (plMaxNode*)SendMessage(hList, LB_GETITEMDATA, sel, 0);
        ISetLocation(node);

        if (setDefault)
            fDefaultLocation = node;
    }
}

void plGetLocationDlg::ISetLocation(plMaxNode *locNode)
{
    plComponent *comp = (plComponent*)locNode->ConvertToComponent();

    // Add the roomless node to the target list and run the convert pass that gives it a room
    comp->AddTarget(fNode);         // Might want to fix this...
    comp->SetupProperties(fNode, nullptr);
}

INT_PTR plGetLocationDlg::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

INT_PTR plGetLocationDlg::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            HWND hList = GetDlgItem(hDlg, IDC_LIST_LOC);
            SendMessage(hList, LB_RESETCONTENT, 0, 0);
            IListRooms((plMaxNode*)GetCOREInterface()->GetRootNode(), hList);

            // Set the prompt text
            TCHAR buf[256];
            _sntprintf(buf, std::size(buf), _T("The object \"%s\" does not have a location. Either pick a location and press OK, or press Cancel to stop the convert."), fNode->GetName());
            SetDlgItemText(hDlg, IDC_PROMPT, buf);

            // No room components found.  Tell user to create one and cancel convert.
            if (SendMessage(hList, LB_GETCOUNT, 0, 0) == 0)
            {
                fErrMsg->Set(true, "Convert Error", "No location component found.  Create one and convert again.");
                fErrMsg->Show();
                EndDialog(hDlg, 0);
            }
            // Else set the first room as the current selection
            else
                SendMessage(hList, LB_SETCURSEL, 0, 0);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            {
            bool setDefault = (IsDlgButtonChecked(hDlg, IDC_CHECK_DEFAULT) == BST_CHECKED);
            IAddSelection(GetDlgItem(hDlg, IDC_LIST_LOC), setDefault);
            EndDialog(hDlg, 1);
            }
            return TRUE;

        case IDCANCEL:
            // Stop the convert.  No message since they know they canceled.
            fErrMsg->Set(true);
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    }

    return FALSE;
}
