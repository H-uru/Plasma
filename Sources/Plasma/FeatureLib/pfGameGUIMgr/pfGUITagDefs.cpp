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
//  pfGUITagDefs.cpp                                                        //
//  List of Tag IDs for the GameGUIMgr                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUITagDefs.h"
#include "pfGameGUIMgr.h"

using namespace ST::literals;

//// Tag List ////////////////////////////////////////////////////////////////
//  Here's the actual list of tags. It's basically a list of konstants, but
//  they get translated into two things:
//          1. An enum, to send as a uint32_t to the GetDialogFromTag() and
//             GetControlFromTag() functions.
//          2. A string, which gets put in a dropdown box in the appropriate
//             MAX component, which sets the given control's tag ID to the
//             right konstant.

// Step 1: add your konstant to the end of the .h file list

// Step 2: Add the string here

pfGUITag    gGUITags[] = {
    { kKIMainDialog, "KI Main Dialog"_st },
    { kKITestEditBox, "KI Test Control"_st },
    { kKIEntryDlg, "KI Entry Dlg"_st },
    { kKICloseButton, "KI Close Dlg Button"_st },
    { kKITestControl2, "KI Test Control 2"_st },
    { kKIAddButton, "KI Add Button"_st },
    { kKIEditButton, "KI Edit Button"_st },
    { kKIRemoveButton, "KI Remove Button"_st },
    { kKIYesNoDlg, "KI Yes/No Dialog"_st },
    { kKIYesBtn, "KI Yes Button"_st },
    { kKINoBtn, "KI No Button"_st },
    { kKIStaticText, "KI Static Text"_st },
    { kKITestControl3, "KI Test Control 3"_st },
    { kKIMiniDialog, "KI Mini Dialog"_st },
    { kPlayerBook, "PB Dialog"_st },
    { kPBLinkToBtn, "PB Link To Button"_st },
    { kPBSaveLinkBtn, "PB Save Link Button"_st },
    { kPBSaveSlotRadio, "PB Save Slot Radio"_st },
    { kPBSaveSlotPrev1, "PB Save Slot Preview 1"_st },
    { kPBSaveSlotPrev2, "PB Save Slot Preview 2"_st },
    { kPBSaveSlotPrev3, "PB Save Slot Preview 3"_st },
    { kPBSaveSlotPrev4, "PB Save Slot Preview 4"_st },
    { kPBSaveSlotPrev5, "PB Save Slot Preview 5"_st },
    { kPBSaveSlotPrev6, "PB Save Slot Preview 6"_st },
    { kKICurrPlayerText, "KI Current Player Label"_st },
    { kKIPlayerList,    "KI Mini Friends List"_st },
    { kKIChatModeBtn,   "KI Toggle Chat Mode Btn"_st },
    { kBlackBarDlg,     "Black Bar Dialog"_st },
    { kBlackBarKIButtons, "Black Bar KI Radio Group"_st },
    { kKILogoutButton, "KI Logout Button"_st },

    { 0, ""_st }       // Ending tag, MUST ALWAYS BE HERE
};
