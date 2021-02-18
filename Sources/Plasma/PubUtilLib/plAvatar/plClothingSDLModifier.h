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
#ifndef plClothingSDLModifier_inc
#define plClothingSDLModifier_inc

#include "HeadSpin.h"
#include "hsColorRGBA.h"

#include "pnNetCommon/plSDLTypes.h"

#include "plModifier/plSDLModifier.h"

//
// This modifier is responsible for sending and recving 
// an avatar's clothing saveState.
//
class plClothingOutfit;
class plClothingItem;
struct plClothingItemOptions;
struct plClosetItem;
class plStateDataRecord;
class plKey;
class plClothingSDLModifier : public plSDLModifier
{
protected:
    plClothingOutfit* fClothingOutfit;
        
    void IPutCurrentStateIn(plStateDataRecord* dstState) override;
    void ISetCurrentStateFrom(const plStateDataRecord* srcState) override;
    
    uint32_t IApplyModFlags(uint32_t sendFlags) override { return (sendFlags | plSynchedObject::kDontPersistOnServer | plSynchedObject::kIsAvatarState); }

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
    static char kStrLinkInAnim[];
    
    CLASSNAME_REGISTER( plClothingSDLModifier );
    GETINTERFACE_ANY( plClothingSDLModifier, plSDLModifier);
    
    plClothingSDLModifier();
    
    plClothingOutfit* GetClothingOutfit();
    void SetClothingOutfit(plClothingOutfit* c) { fClothingOutfit=c; }
    
    void PutCurrentStateIn(plStateDataRecord* dstState);
    const char* GetSDLName() const override { return kSDLClothing; }
    static const char* GetClothingItemSDRName() { return kStrClothingDescName; }

    // Pass in a clothing outfit if you want to apply the clothing item. Pass in a closet item if you're just
    // looking to parse the SDL info.
    // Aw heck. Go crazy if you want and pass them BOTH in! Muahahaha!
    static void HandleSingleSDR(const plStateDataRecord *sdr, plClothingOutfit *clothing = nullptr, plClosetItem *closetItem = nullptr);
    static void PutSingleItemIntoSDR(const plClosetItem *item, plStateDataRecord *sdr);
        
    static const plClothingSDLModifier *FindClothingSDLModifier(const plSceneObject *obj);
};

#endif  // plClothingSDLModifier_inc
