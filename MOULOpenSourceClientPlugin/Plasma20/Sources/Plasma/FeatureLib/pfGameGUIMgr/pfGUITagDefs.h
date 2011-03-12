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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfGUITagDefs.cpp														//
//	List of Tag IDs for the GameGUIMgr										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUITagDefs_h
#define _pfGUITagDefs_h

#include "pfGameGUIMgr.h"

//// Tag List ////////////////////////////////////////////////////////////////
//	Here's the actual list of tags. It's basically a list of konstants, but
//	they get translated into two things:
//			1. An enum, to send as a UInt32 to the GetDialogFromTag() and
//			   GetControlFromTag() functions.
//			2. A string, which gets put in a dropdown box in the appropriate
//			   MAX component, which sets the given control's tag ID to the
//			   right konstant.


// Step 1: Add your konstant to the end of this list

enum
{
	kKIMainDialog = 1,
	kKITestEditBox,
	kKIEntryDlg,
	kKICloseButton,
	kKITestControl2,
	kKIAddButton,
	kKIEditButton,
	kKIRemoveButton,
	kKIYesNoDlg,
	kKIYesBtn,
	kKINoBtn,
	kKIStaticText,
	kKITestControl3,
	kKIMiniDialog,
	kPlayerBook,
	kPBLinkToBtn,
	kPBSaveLinkBtn,
	kPBSaveSlotRadio,
	kPBSaveSlotPrev1,
	kPBSaveSlotPrev2,
	kPBSaveSlotPrev3,
	kPBSaveSlotPrev4,
	kPBSaveSlotPrev5,
	kPBSaveSlotPrev6,

	kKICurrPlayerText	= 30,
	kKIPlayerList		= 31,
	kKIChatModeBtn		= 32,

	kBlackBarDlg		= 33,
	kBlackBarKIButtons	= 34,
	kKILogoutButton		= 35,
};

// Step 2: Add the string to the .cpp file

#endif 
