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

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfGUIDialogNotifyProc                                                   //
//                                                                          //
//  Helper dialog proc that takes all control events and turns them into    //
//  notify messages that get sent out.                                      //
//////////////////////////////////////////////////////////////////////////////


#include "pfGUIDialogNotifyProc.h"

#include "HeadSpin.h"

#include "pfGUIDialogMod.h"
#include "pfGUIButtonMod.h"
#include "pfGUIControlMod.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIEditBoxMod.h"
#include "pfGUIListBoxMod.h"
#include "pfGUIListElement.h"
#include "pfGUIMultiLineEditCtrl.h"

#include "pfMessage/pfGUINotifyMsg.h"

void pfGUIDialogNotifyProc::ISendNotify( plKey ctrlKey, uint32_t event )
{
    pfGUINotifyMsg  *notify = new pfGUINotifyMsg(fDialog->GetKey(), fReceiver, nullptr);
    notify->SetEvent(std::move(ctrlKey), event);
    notify->Send();
}


void pfGUIDialogNotifyProc::DoSomething( pfGUIControlMod *ctrl )
{
    if (pfGUIButtonMod::ConvertNoRef(ctrl) != nullptr ||
        pfGUIListBoxMod::ConvertNoRef(ctrl) != nullptr ||
        pfGUIEditBoxMod::ConvertNoRef(ctrl) != nullptr)
    {
        // only fire the button if it is triggering
        // ... all other types just fire
        pfGUIButtonMod* btn = pfGUIButtonMod::ConvertNoRef( ctrl );
        if ( !btn || btn->IsTriggering() )
            ISendNotify( ctrl->GetKey(), pfGUINotifyMsg::kAction );
    }
    else
        ISendNotify( ctrl->GetKey(), pfGUINotifyMsg::kValueChanged );
}

void pfGUIDialogNotifyProc::HandleExtendedEvent( pfGUIControlMod *ctrl, uint32_t event )
{
    pfGUIEditBoxMod* edit = pfGUIEditBoxMod::ConvertNoRef(ctrl);
    if (edit) {
        if (event == pfGUIEditBoxMod::kWantAutocomplete)
            ISendNotify(ctrl->GetKey(), pfGUINotifyMsg::kSpecialAction);
        else if (event == pfGUIEditBoxMod::kWantMessageHistoryUp)
            ISendNotify(ctrl->GetKey(), pfGUINotifyMsg::kMessageHistoryUp);
        else if (event == pfGUIEditBoxMod::kWantMessageHistoryDown)
            ISendNotify(ctrl->GetKey(), pfGUINotifyMsg::kMessageHistoryDown);
    }

    pfGUIMultiLineEditCtrl* mlEdit = pfGUIMultiLineEditCtrl::ConvertNoRef(ctrl);
    if (mlEdit) {
        if (event == pfGUIMultiLineEditCtrl::kLinkClicked)
            ISendNotify(ctrl->GetKey(), pfGUINotifyMsg::kAction);
    }

    pfGUIButtonMod* button = pfGUIButtonMod::ConvertNoRef(ctrl);
    if (button) {
        if (event == pfGUIButtonMod::kDoubleClick)
            ISendNotify(ctrl->GetKey(), pfGUINotifyMsg::kSpecialAction);
    }
}

void pfGUIDialogNotifyProc::OnInit()
{
    if ( fDialog )
        ISendNotify( fDialog->GetKey(), pfGUINotifyMsg::kDialogLoaded );
    else
        ISendNotify(nullptr, pfGUINotifyMsg::kDialogLoaded);
}

void pfGUIDialogNotifyProc::OnShow()
{
    if ( fDialog )
        ISendNotify( fDialog->GetKey(), pfGUINotifyMsg::kShowHide );
    else
        ISendNotify(nullptr, pfGUINotifyMsg::kShowHide);
}

void pfGUIDialogNotifyProc::OnHide()
{
    if ( fDialog )
        ISendNotify( fDialog->GetKey(), pfGUINotifyMsg::kShowHide );
    else
        ISendNotify(nullptr, pfGUINotifyMsg::kShowHide);
}

void pfGUIDialogNotifyProc::OnDestroy()
{
}

void pfGUIDialogNotifyProc::OnControlEvent( ControlEvt event )
{
    if( event == kExitMode )
        ISendNotify((fDialog != nullptr) ? fDialog->GetKey() : nullptr, pfGUINotifyMsg::kExitMode);
}

// Called when the dialog's focused control changes
void pfGUIDialogNotifyProc::OnCtrlFocusChange( pfGUIControlMod *oldCtrl, pfGUIControlMod *newCtrl )
{
    if ( newCtrl )
        ISendNotify( newCtrl->GetKey(), pfGUINotifyMsg::kFocusChange);
    else
        ISendNotify(nullptr, pfGUINotifyMsg::kFocusChange);

}

void pfGUIDialogNotifyProc::OnInterestingEvent( pfGUIControlMod *ctrl )
{
    ISendNotify((ctrl != nullptr) ? ctrl->GetKey() : nullptr, pfGUINotifyMsg::kInterestingEvent);
}
