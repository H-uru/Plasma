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
#ifndef plClothingSDLModifier_inc
#define plClothingSDLModifier_inc

#include "../plModifier/plSDLModifier.h"

#include "hsColorRGBA.h"
#include "hsTemplates.h"
#include "hsTypes.h"

//
// This modifier is responsible for sending and recving 
// an avatar's clothing saveState.
//
class plClothingOutfit;
class plClothingItem;
class plClothingItemOptions;
class plClosetItem;
class plStateDataRecord;
class plKey;
class plClothingSDLModifier : public plSDLModifier
{
protected:
	plClothingOutfit* fClothingOutfit;
		
	void IPutCurrentStateIn(plStateDataRecord* dstState);
	void ISetCurrentStateFrom(const plStateDataRecord* srcState);
	
	UInt32 IApplyModFlags(UInt32 sendFlags) { return (sendFlags | plSynchedObject::kDontPersistOnServer | plSynchedObject::kIsAvatarState); }

public:
	// var labels 
	static char kStrItem[];	
	static char kStrTint[];	
	static char kStrTint2[];	
	static char kStrWardrobe[];	
	static char kStrSkinTint[];
	static char kStrFaceBlends[];
	static char kStrAppearance[];
	static char kStrClothingDescName[];
	static char kStrAppearanceDescName[];
	static char	kStrLinkInAnim[];
	
	CLASSNAME_REGISTER( plClothingSDLModifier );
	GETINTERFACE_ANY( plClothingSDLModifier, plSDLModifier);
	
	plClothingSDLModifier();
	
	plClothingOutfit* GetClothingOutfit();
	void SetClothingOutfit(plClothingOutfit* c) { fClothingOutfit=c; }
	
	void PutCurrentStateIn(plStateDataRecord* dstState);
	const char* GetSDLName() const { return kSDLClothing; }
	static const char* GetClothingItemSDRName() { return kStrClothingDescName; }

	// Pass in a clothing outfit if you want to apply the clothing item. Pass in a closet item if you're just
	// looking to parse the SDL info.
	// Aw heck. Go crazy if you want and pass them BOTH in! Muahahaha!
	static void HandleSingleSDR(const plStateDataRecord *sdr, plClothingOutfit *clothing = nil, plClosetItem *closetItem = nil);
	static void PutSingleItemIntoSDR(plClosetItem *item, plStateDataRecord *sdr);
		
	static const plClothingSDLModifier *plClothingSDLModifier::FindClothingSDLModifier(const plSceneObject *obj);
};

#endif	// plClothingSDLModifier_inc
