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
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"
#include "pfGUIControlMod.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIListElement.h"
#include "pfGUIButtonMod.h"     // Next three are for notify stuff
#include "pfGUIListBoxMod.h"
#include "pfGUIEditBoxMod.h"

#include "pfMessage/pfGUINotifyMsg.h"

#include "plgDispatch.h"
#include "hsResMgr.h"


pfGUIDialogNotifyProc::pfGUIDialogNotifyProc( plKey &r )
{
    fReceiver = r;
}


void pfGUIDialogNotifyProc::ISendNotify( plKey ctrlKey, uint32_t event )
{
    pfGUINotifyMsg  *notify = new pfGUINotifyMsg( fDialog->GetKey(), fReceiver, nil );
    notify->SetEvent( ctrlKey, event );
    plgDispatch::MsgSend( notify );
}


void pfGUIDialogNotifyProc::DoSomething( pfGUIControlMod *ctrl )
{
    if( pfGUIButtonMod::ConvertNoRef( ctrl ) != nil ||
        pfGUIListBoxMod::ConvertNoRef( ctrl ) != nil ||
        pfGUIEditBoxMod::ConvertNoRef( ctrl ) != nil )
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
    pfGUIEditBoxMod *edit = pfGUIEditBoxMod::ConvertNoRef( ctrl );
    if(edit != nil && event == pfGUIEditBoxMod::kWantAutocomplete)
    {
        //send notify, somebody will do something with that (like python script)
        ISendNotify( ctrl->GetKey(), pfGUINotifyMsg::kSpecialAction );
    }
}

void pfGUIDialogNotifyProc::OnInit( void )
{
    if ( fDialog )
        ISendNotify( fDialog->GetKey(), pfGUINotifyMsg::kDialogLoaded );
    else
        ISendNotify( nil, pfGUINotifyMsg::kDialogLoaded );
}

void pfGUIDialogNotifyProc::OnShow( void )
{
    if ( fDialog )
        ISendNotify( fDialog->GetKey(), pfGUINotifyMsg::kShowHide );
    else
        ISendNotify( nil, pfGUINotifyMsg::kShowHide );
}

void pfGUIDialogNotifyProc::OnHide( void )
{
    if ( fDialog )
        ISendNotify( fDialog->GetKey(), pfGUINotifyMsg::kShowHide );
    else
        ISendNotify( nil, pfGUINotifyMsg::kShowHide );
}

void pfGUIDialogNotifyProc::OnDestroy( void )
{
}

void pfGUIDialogNotifyProc::OnControlEvent( ControlEvt event )
{
    if( event == kExitMode )
        ISendNotify( ( fDialog != nil ) ? fDialog->GetKey() : nil, pfGUINotifyMsg::kExitMode );
}

// Called when the dialog's focused control changes
void pfGUIDialogNotifyProc::OnCtrlFocusChange( pfGUIControlMod *oldCtrl, pfGUIControlMod *newCtrl )
{
    if ( newCtrl )
        ISendNotify( newCtrl->GetKey(), pfGUINotifyMsg::kFocusChange);
    else
        ISendNotify( nil, pfGUINotifyMsg::kFocusChange);

}

void pfGUIDialogNotifyProc::OnInterestingEvent( pfGUIControlMod *ctrl )
{
    ISendNotify( ( ctrl != nil ) ? ctrl->GetKey() : nil, pfGUINotifyMsg::kInterestingEvent );
}
