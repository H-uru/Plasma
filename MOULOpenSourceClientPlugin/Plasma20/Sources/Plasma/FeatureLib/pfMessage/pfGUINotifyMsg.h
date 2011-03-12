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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef _pfGUINotifyMsg_h_
#define _pfGUINotifyMsg_h_

#include "../pnMessage/plMessage.h"
#include "hsResMgr.h"
#include "../pnModifier/plSingleModifier.h"
#include "hsUtils.h"



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
	plKey		fControlKey;		// who start this mess
	UInt32		fEvent;				// what was the event that happened

public:
	pfGUINotifyMsg() : plMessage() {}
	pfGUINotifyMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : plMessage(s, r, t) {}
	~pfGUINotifyMsg() {}

	CLASSNAME_REGISTER( pfGUINotifyMsg );
	GETINTERFACE_ANY( pfGUINotifyMsg, plMessage );

	enum GUIEventType
	{
		kShowHide = 1,		// show or hide change
		kAction,			// button clicked, ListBox click on item, EditBox hit enter
		kValueChanged,		// value changed in control
		kDialogLoaded,		// the dialog is now loaded and ready for action
		kFocusChange,		// when one of its controls loses focus to another
		kExitMode,			// GUI Exit Mode key was pressed
		kInterestingEvent,	// GUI interesting-ness has changed
		kEndEventList
	};

/////////////////
// Currently, the following is the only event types that get produced:
//
// kDialog
//    kShowHide			- when the dialog is shown or hidden
// kButton
//    kAction			- when the button clicked (should be on mouse button up)
// kListBox
//    kAction			- single click on item(s)
// kEditBox
//    kAction			- enter key hit
// kUpDownPair
//    kValueChanged		- the value of the pair has been changed
// kKnob
//    kValueChanged		- the value of the knob has been changed
// kCheckBox
//    kValueChanged		- the checkbox state has been changed
// kRadioGroup
//    kValueChanged		- the radio group button has been changed
//
// Control types that don't create events
//
// kDraggable
// kTextBox
// kDragBar

	void SetEvent( plKey &key, UInt32 event)
	{
		fControlKey = key;
		fEvent = event;
	}

	plKey GetControlKey() { return fControlKey; }
	UInt32 GetEvent() { return fEvent; }

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fControlKey = mgr->ReadKey(stream);
		fEvent = stream->ReadSwap32();
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		mgr->WriteKey(stream, fControlKey);
		stream->WriteSwap32(fEvent);
	}
};



#endif // _pfGUINotifyMsg_h_
