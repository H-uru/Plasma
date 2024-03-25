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
#ifndef _pfGUINotifyMsg_h_
#define _pfGUINotifyMsg_h_

#include "HeadSpin.h"

#include "pnMessage/plMessage.h"

/////////////////////////////////////////////////////////////////////////////
//
//  MESSAGE    : plGUINotifyMsg
//  PARAMETERS : none
//
//  PURPOSE    : This is the message that notifies someone (either a responder or activator)
//             : that some event or transition of state has happened on a GUI control
//
//
class pfGUINotifyMsg : public plMessage
{
protected:
    plKey       fControlKey;        // who start this mess
    uint32_t    fEvent;             // what was the event that happened

public:
    pfGUINotifyMsg() : plMessage(), fEvent() { }
    pfGUINotifyMsg(const plKey &s, const plKey &r, const double* t)
        : plMessage(s, r, t), fEvent() { }
    ~pfGUINotifyMsg() {}

    CLASSNAME_REGISTER( pfGUINotifyMsg );
    GETINTERFACE_ANY( pfGUINotifyMsg, plMessage );

    enum GUIEventType
    {
        kShowHide = 1,      // show or hide change
        kAction,            // button clicked, ListBox click on item, EditBox hit enter, MultiLineEdit hit link
        kValueChanged,      // value changed in control
        kDialogLoaded,      // the dialog is now loaded and ready for action
        kFocusChange,       // when one of its controls loses focus to another
        kExitMode,          // GUI Exit Mode key was pressed
        kInterestingEvent,  // GUI interesting-ness has changed
        kSpecialAction,     // meaning depends on control functionality (see below) 
        kMessageHistoryUp,  // up key to scroll back in history
        kMessageHistoryDown,// down key to scroll forward in history
        kEndEventList
    };

/////////////////
// Currently, the following is the only event types that get produced:
//
// kDialog
//    kShowHide         - when the dialog is shown or hidden
// kButton
//    kAction           - when the button clicked (should be on mouse button up)
// kListBox
//    kAction           - single click on item(s)
// kEditBox
//    kAction           - enter key hit
//    kSpecialAction    - tab key hit (for autocompletion on Python side)
//    kMessageHistoryUp - up key hit
//    kMessageHistoryDown - down key hit
// kMultiLineEdit
//    kAction           - clicked a link
//    kValueChanged     - the content or position of the control changed
// kUpDownPair
//    kValueChanged     - the value of the pair has been changed
// kKnob
//    kValueChanged     - the value of the knob has been changed
// kCheckBox
//    kValueChanged     - the checkbox state has been changed
// kRadioGroup
//    kValueChanged     - the radio group button has been changed
//
// Control types that don't create events
//
// kDraggable
// kTextBox
// kDragBar

    void SetEvent(plKey key, uint32_t event)
    {
        fControlKey = std::move(key);
        fEvent = event;
    }

    plKey GetControlKey() { return fControlKey; }
    uint32_t GetEvent() { return fEvent; }

    // IO 
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};



#endif // _pfGUINotifyMsg_h_
