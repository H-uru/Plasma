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

#include "pfGameGUIMgr.h"
#include "pfGUITagDefs.h"

//// Tag List ////////////////////////////////////////////////////////////////
//	Here's the actual list of tags. It's basically a list of konstants, but
//	they get translated into two things:
//			1. An enum, to send as a UInt32 to the GetDialogFromTag() and
//			   GetControlFromTag() functions.
//			2. A string, which gets put in a dropdown box in the appropriate
//			   MAX component, which sets the given control's tag ID to the
//			   right konstant.

// Step 1: add your konstant to the end of the .h file list

// Step 2: Add the string here

pfGUITag	gGUITags[] = {
	{ kKIMainDialog, "KI Main Dialog" },
	{ kKITestEditBox, "KI Test Control" },
	{ kKIEntryDlg, "KI Entry Dlg" },
	{ kKICloseButton, "KI Close Dlg Button" },
	{ kKITestControl2, "KI Test Control 2" },
	{ kKIAddButton, "KI Add Button" },
	{ kKIEditButton, "KI Edit Button" },
	{ kKIRemoveButton, "KI Remove Button" },
	{ kKIYesNoDlg, "KI Yes/No Dialog" },
	{ kKIYesBtn, "KI Yes Button" },
	{ kKINoBtn, "KI No Button" },
	{ kKIStaticText, "KI Static Text" },
	{ kKITestControl3, "KI Test Control 3" },
	{ kKIMiniDialog, "KI Mini Dialog" },
	{ kPlayerBook, "PB Dialog" },
	{ kPBLinkToBtn, "PB Link To Button" },
	{ kPBSaveLinkBtn, "PB Save Link Button" },
	{ kPBSaveSlotRadio, "PB Save Slot Radio" },
	{ kPBSaveSlotPrev1, "PB Save Slot Preview 1" },
	{ kPBSaveSlotPrev2, "PB Save Slot Preview 2" },
	{ kPBSaveSlotPrev3, "PB Save Slot Preview 3" },
	{ kPBSaveSlotPrev4, "PB Save Slot Preview 4" },
	{ kPBSaveSlotPrev5, "PB Save Slot Preview 5" },
	{ kPBSaveSlotPrev6, "PB Save Slot Preview 6" },
	{ kKICurrPlayerText, "KI Current Player Label" },
	{ kKIPlayerList,	"KI Mini Friends List" },
	{ kKIChatModeBtn,	"KI Toggle Chat Mode Btn" },
	{ kBlackBarDlg,		"Black Bar Dialog" },
	{ kBlackBarKIButtons, "Black Bar KI Radio Group" },
	{ kKILogoutButton, "KI Logout Button" },

	{ 0, "" }		// Ending tag, MUST ALWAYS BE HERE
};
