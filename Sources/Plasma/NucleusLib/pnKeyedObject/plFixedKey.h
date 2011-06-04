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
#ifndef PLFIXEDKEY_H
#define PLFIXEDKEY_H


	// Using Fixed Key feature:
	// Add a new fixedkey to the enum list below.
	// Then add, to the Seed list (in plFixedKey.cpp) in the Corresponding 
	// position, (don't screw up, it will be validated...)
	// The "Full Address" data for you Fixed key (see rules in plFixedKey.cpp)

enum plFixedKeyId
{
	kFirst_Fixed_KEY,

	kLOSObject_KEY,
	kTimerCallbackManager_KEY,
	kConsoleObject_KEY,	
	kAudioSystem_KEY,
	kInput_KEY,
	kClient_KEY,
	kNetClientMgr_KEY,
	kListenerMod_KEY,
	kTransitionMgr_KEY,
	kLinkEffectsMgr_KEY,
	kGameGUIMgr_KEY,
	kGameGUIDynamicDlg_KEY,
	kVirtualCamera1_KEY,
	kDefaultCameraMod1_KEY,
	kKIGUIGlue_KEY,
	kClothingMgr_KEY,
	kInputInterfaceMgr_KEY,
	kAVIWriter_KEY,
	kResManagerHelper_KEY,
	kAvatarMgr_KEY,
	kSimulationMgr_KEY,
	kTransitionCamera_KEY,
	kCCRMgr_KEY,
	kNetClientCloneRoom_KEY,
	kMarkerMgr_KEY,
	kAutoProfile_KEY,
	kGlobalVisMgr_KEY,
	kFontCache_KEY,
	kRelevanceMgr_KEY,
	kJournalBookMgr_KEY,
	kAgeLoader_KEY,
	kBuiltIn3rdPersonCamera_KEY,
	kSecurePreloader_KEY,

	kLast_Fixed_KEY
};


#endif
