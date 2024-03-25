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
#include "plClothingSDLModifier.h"

#include "plArmatureMod.h"
#include "plAvatarClothing.h"
#include "plClothingLayout.h"

#include "pnKeyedObject/plKeyImp.h"
#include "pnMessage/plSDLModifierMsg.h"
#include "pnNetCommon/plSDLTypes.h"
#include "pnSceneObject/plSceneObject.h"

#include "plSDL/plSDL.h"


// static vars
char plClothingSDLModifier::kStrItem[]="item";
char plClothingSDLModifier::kStrTint[]="tint";
char plClothingSDLModifier::kStrTint2[]="tint2";
char plClothingSDLModifier::kStrWardrobe[]="wardrobe";
char plClothingSDLModifier::kStrSkinTint[]="skinTint";
char plClothingSDLModifier::kStrFaceBlends[]="faceBlends";
char plClothingSDLModifier::kStrAppearance[]="appearance";
char plClothingSDLModifier::kStrClothingDescName[]="clothingItem";
char plClothingSDLModifier::kStrAppearanceDescName[]="appearanceOptions";
char plClothingSDLModifier::kStrLinkInAnim[] = "linkInAnim";

//
// init latest data
//
plClothingSDLModifier::plClothingSDLModifier() :
    fClothingOutfit()
{
}

//
// find armature mod and cache it's clothingOutfit  
//
plClothingOutfit* plClothingSDLModifier::GetClothingOutfit()
{
    if (fClothingOutfit)
        return fClothingOutfit;

    plSceneObject* target = GetTarget();
    hsAssert(target, "plClothingSDLModifier: nil target");
    for (size_t i = 0; i < target->GetNumModifiers(); i++)
    {
        const plArmatureMod* am = plArmatureMod::ConvertNoRef(target->GetModifier(i));
        if (am)
        {
            fClothingOutfit=am->GetClothingOutfit();
            break;
        }
    }

    return fClothingOutfit;
}

void plClothingSDLModifier::PutCurrentStateIn(plStateDataRecord* dstState)
{
    IPutCurrentStateIn(dstState);
}

//
// get current state from clothingOutfit and put it in dstState
//
void plClothingSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
    plSceneObject* sobj=GetTarget();
    hsAssert(sobj, "plClothingSDLModifier, nil target");
    
    plClothingOutfit* clothing = GetClothingOutfit();
    hsAssert(clothing, "nil clothingOutfit");
    
    std::vector<plStateDataRecord*> SDRs;
    const std::vector<plClothingItem*>& items = clothing->GetItemList();
    const std::vector<plClothingItemOptions*>& options = clothing->GetOptionList();
    
    plSDStateVariable* clothesStateDesc = dstState->FindSDVar(kStrWardrobe);    // find clothes list
    size_t itemCount = items.size();
    size_t optionsCount = options.size();
    hsAssert(optionsCount==itemCount, "number of items should match number of options according to clothing.sdl"); 
    
    if (clothesStateDesc->GetCount() != itemCount)
        clothesStateDesc->Alloc(itemCount);     // set appropriate list size

    size_t lowerCount = std::min(itemCount, optionsCount);
    for (size_t i = 0; i < lowerCount; i++)
    {
        plClosetItem closetItem;
        closetItem.fItem = items[i];
        closetItem.fOptions = *options[i];

        PutSingleItemIntoSDR(&closetItem, clothesStateDesc->GetStateDataRecord(i));
        SDRs.emplace_back(clothesStateDesc->GetStateDataRecord(i));
    }

    // skin tint
    plSDStateVariable* appearanceStateDesc = dstState->FindSDVar(kStrAppearance); // for skin tint
    uint8_t skinTint[3];
    skinTint[0] = (uint8_t)(clothing->fSkinTint.r * 255);
    skinTint[1] = (uint8_t)(clothing->fSkinTint.g * 255);
    skinTint[2] = (uint8_t)(clothing->fSkinTint.b * 255);
    appearanceStateDesc->GetStateDataRecord(0)->FindVar(kStrSkinTint)->Set(skinTint);

    plSimpleStateVariable* faceBlends = appearanceStateDesc->GetStateDataRecord(0)->FindVar(kStrFaceBlends);
    int numBlends = plClothingElement::kLayerSkinLast - plClothingElement::kLayerSkinFirst;
    if (faceBlends->GetCount() != numBlends)
        faceBlends->Alloc(numBlends);
    for (int i = 0; i < numBlends; i++)
        faceBlends->Set((uint8_t)(clothing->fSkinBlends[i] * 255), i);

    SDRs.emplace_back(appearanceStateDesc->GetStateDataRecord(0));

    // This logically belongs in the avatar.sdl file, but clothing.sdl is already broadcast to
    // other players when you join an age, and I don't want to broadcast all of avatar.sdl for
    // just this one value.
    plSimpleStateVariable *var = dstState->FindVar(kStrLinkInAnim);
    if (var)
        var->Set(clothing->fAvatar->fLinkInAnimKey);    
}

void plClothingSDLModifier::PutSingleItemIntoSDR(const plClosetItem *item, plStateDataRecord *sdr)
{
    plKey key = item->fItem->GetKey();
    sdr->FindVar(kStrItem)->Set(key);           

    //hsColorRGBA c = item->fOptions.fTint1;
    //hsColorRGBA c2 = item->fOptions.fTint2;
    uint8_t c[3];
    uint8_t c2[3];
    c[0] = (uint8_t)(item->fOptions.fTint1.r * 255);
    c[1] = (uint8_t)(item->fOptions.fTint1.g * 255);
    c[2] = (uint8_t)(item->fOptions.fTint1.b * 255);
    c2[0] = (uint8_t)(item->fOptions.fTint2.r * 255);
    c2[1] = (uint8_t)(item->fOptions.fTint2.g * 255);
    c2[2] = (uint8_t)(item->fOptions.fTint2.b * 255);
    
    sdr->FindVar(kStrTint)->Set(c);
    sdr->FindVar(kStrTint2)->Set(c2);
}

//
// Apply the SDL state that is passed in to the current clothing state.
//
void plClothingSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
    plSceneObject* sobj=GetTarget();
    hsAssert(sobj, "plClothingSDLModifier, nil target");

    plClothingOutfit* clothing = GetClothingOutfit();
    if (clothing == nullptr)
    {
        hsAssert(clothing, "nil clothingOutfit");
        return;
    }

    plSDStateVariable* clothesStateDesc = srcState->FindSDVar(kStrWardrobe);    // find clothes list
    int num=clothesStateDesc->GetCount();   // size of clothes list
    bool update=true;

    // We just remove the accessories. Any regular items will be replaced
    // when we try to wear something else in the same category. (And in
    // case the player lies, this guarantees they'll at least be wearing something.)
    clothing->StripAccessories();

    int i;
    for(i=0;i<num;i++)
    {
        // get clothes item from clothes list
        plStateDataRecord* clothingItemState=clothesStateDesc->GetStateDataRecord(i);
        HandleSingleSDR(clothingItemState, clothing);
    }

    plSDStateVariable* appearanceStateDesc = srcState->FindSDVar(kStrAppearance); // for skin tint
    plStateDataRecord* appearanceState = appearanceStateDesc->GetStateDataRecord(0);
    HandleSingleSDR(appearanceState, clothing);

    if (update)
        clothing->ForceUpdate(true /* retry */);

    plSimpleStateVariable *var;
    var = srcState->FindVar(kStrLinkInAnim);
    if (var)
        var->Get(&clothing->fAvatar->fLinkInAnimKey);   
}

void plClothingSDLModifier::HandleSingleSDR(const plStateDataRecord *sdr, plClothingOutfit *clothing /* = nullptr */, plClosetItem *closetItem /* = nullptr */)
{
    if (sdr == nullptr)
        return;

    int i;
    uint8_t tint[3];
    float tintScalar[3];
    if (sdr->GetDescriptor()->GetName() == kStrClothingDescName)
    {
        // get item from clothesItem
        plSimpleStateVariable* itemVar = sdr->FindVar(kStrItem);
        plClothingItem* clothingItem = nullptr; // clothing->GetItemList()[i];
        if (itemVar->IsUsed())
        {
            plKey key;
            itemVar->Get(&key);
            clothingItem = plClothingItem::ConvertNoRef(key ? key->GetObjectPtr() : nullptr);
            if (clothingItem)
            {
                if (clothing)
                    clothing->AddItem(clothingItem, false, false /*bcast */); 
                if (closetItem)
                    closetItem->fItem = clothingItem;
            }
        }

        // tints
        if (clothingItem)
        {
            // get item from clothesItem
            plSimpleStateVariable* tintVar = sdr->FindVar(kStrTint);
            if (tintVar->IsUsed())
            {
                tintVar->Get(tint);
                tintScalar[0] = tint[0] / 255.f;
                tintScalar[1] = tint[1] / 255.f;
                tintScalar[2] = tint[2] / 255.f;
                if (clothing)
                    clothing->TintItem(clothingItem, tintScalar[0], tintScalar[1], tintScalar[2],
                                       false /*update*/, false /*broadcast*/, false /*netForce*/, true, plClothingElement::kLayerTint1);
                if (closetItem)
                    closetItem->fOptions.fTint1.Set(tintScalar[0], tintScalar[1], tintScalar[2], 1.f);
            }

            tintVar=sdr->FindVar(kStrTint2);
            if (tintVar->IsUsed())
            {
                tintVar->Get(tint);
                tintScalar[0] = tint[0] / 255.f;
                tintScalar[1] = tint[1] / 255.f;
                tintScalar[2] = tint[2] / 255.f;
                
                if (clothing)
                    clothing->TintItem(clothingItem, tintScalar[0], tintScalar[1], tintScalar[2],
                                       false /*update*/, false /*broadcast*/, false /*netForce*/, true, plClothingElement::kLayerTint2);
                if (closetItem)
                    closetItem->fOptions.fTint2.Set(tintScalar[0], tintScalar[1], tintScalar[2], 1.f);
            }
        }
    }
    else if (sdr->GetDescriptor()->GetName() == kStrAppearanceDescName)
    {
        // skin tints
        plSimpleStateVariable* skinVar = sdr->FindVar(kStrSkinTint);
        if (skinVar->IsUsed())
        {
            skinVar->Get(tint);
            if (clothing)
                clothing->TintSkin(tint[0] / 255.f, tint[1] / 255.f, tint[2] / 255.f, false /* update */, false /*broadcast*/);
        }
        plSimpleStateVariable* faceBlends = sdr->FindVar(kStrFaceBlends);
        if (faceBlends->IsUsed())
        {
            int numBlends = plClothingElement::kLayerSkinLast - plClothingElement::kLayerSkinFirst;
            for(i = 0; i < numBlends && i < faceBlends->GetCount(); i++)
            {
                uint8_t blend;
                faceBlends->Get(&blend, i);
                if (clothing)
                    clothing->fSkinBlends[i] = (float)blend / 255;
            }
        }       
    }
}

// FINDARMATUREMOD
const plClothingSDLModifier *plClothingSDLModifier::FindClothingSDLModifier(const plSceneObject *obj)
{
    size_t count = obj->GetNumModifiers();

    for (size_t i = 0; i < count; i++)
    {
        const plModifier *mod = obj->GetModifier(i);
        const plClothingSDLModifier *sdlMod = plClothingSDLModifier::ConvertNoRef(mod);
        if(sdlMod)
            return sdlMod;
    }
    return nullptr;
}
