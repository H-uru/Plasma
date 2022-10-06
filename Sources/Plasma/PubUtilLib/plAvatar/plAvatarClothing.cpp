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

#include "plAvatarClothing.h"

#include "plgDispatch.h"
#include "plPipeline.h"
#include "hsResMgr.h"

#include "plArmatureEffects.h"
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plClothingSDLModifier.h"

#include "pnEncryption/plRandom.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/plSDLTypes.h"

#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plInstanceDrawInterface.h"
#include "plDrawable/plMorphSequence.h"
#include "plDrawable/plMorphSequenceSDLMod.h"
#include "plDrawable/plSharedMesh.h"
#include "plDrawable/plSpaceTree.h"
#include "plGImage/plMipmap.h"
#include "plMessage/plRenderMsg.h"
#include "plResMgr/plKeyFinder.h"
#include "plSDL/plSDL.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"
#include "plVault/plVault.h"

#include "pfMessage/plClothingMsg.h"

plClothingItem::plClothingItem()
    : fGroup(), fType(), fTileset(), fSortOrder(), fThumbnail(),
      fAccessory()
{
    std::fill(std::begin(fMeshes), std::end(fMeshes), nullptr);
    std::fill(std::begin(fDefaultTint1), std::end(fDefaultTint1), 255);
    std::fill(std::begin(fDefaultTint2), std::end(fDefaultTint2), 255);
}

plClothingItem::~plClothingItem()
{
    while (!fTextures.empty()) {
        delete[] fTextures.back();
        fTextures.pop_back();
    }
}

bool plClothingItem::CanWearWith(plClothingItem *item)
{ 
    // For now, you can only wear one shirt, one pair of pants, etc.
    // Except accessories, of which you're allowed one per tileset.
    return ((item->fType != fType) || 
            (item->fType == plClothingMgr::kTypeAccessory && 
             fType == plClothingMgr::kTypeAccessory));
             //&& item->fTileset != fTileset));
}

bool plClothingItem::WearBefore(plClothingItem *item)
{
    // Accessories come last
    // Face comes first
    // Other stuff is arbitrary.

    int myPri;
    if (fType == plClothingMgr::kTypeFace)
        myPri = 0;
    else
        myPri = (fType == plClothingMgr::kTypeAccessory ? 2 : 1);

    int otherPri;
    if (item->fType == plClothingMgr::kTypeFace)
        otherPri = 0;
    else
        otherPri = (item->fType == plClothingMgr::kTypeAccessory ? 2 : 1);
    
    return myPri < otherPri;
}

bool plClothingItem::HasBaseAlpha()
{
    for (size_t i = 0; i < fElements.size(); i++)
    {
        plMipmap *tex = fTextures[i][plClothingElement::kLayerBase];
//      if (tex && (tex->GetFlags() & (plMipmap::kAlphaBitFlag | plMipmap::kAlphaChannelFlag)))
        if (tex && (tex->GetFlags() & plMipmap::kAlphaChannelFlag))
            return true;
    }
    return false;
}

bool plClothingItem::HasSameMeshes(const plClothingItem *other) const
{
    for (int i = 0; i < kMaxNumLODLevels; i++)
        if (fMeshes[i] != other->fMeshes[i])
            return false;

    return true;
}

void plClothingItem::Read(hsStream *s, hsResMgr *mgr)
{
    hsKeyedObject::Read(s, mgr);

    fName = s->ReadSafeString();
    fGroup = s->ReadByte();
    fType = s->ReadByte();
    fTileset = s->ReadByte();
    fSortOrder = s->ReadByte();

    fCustomText = s->ReadSafeString();
    fDescription = s->ReadSafeString();
    if (s->ReadBool())
        mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // thumbnail

    uint32_t tileCount = s->ReadLE32();
    for (uint32_t i = 0; i < tileCount; i++)
    {
        fElementNames.emplace_back(s->ReadSafeString());

        uint8_t layerCount = s->ReadByte();
        for (uint8_t j = 0; j < layerCount; j++)
        {
            int layer = s->ReadByte();
            mgr->ReadKeyNotifyMe(s, new plElementRefMsg(GetKey(), plRefMsg::kOnCreate, i, -1, ST::string(), layer), plRefFlags::kActiveRef); // texture
        }
    }

    for (int i = 0; i < kMaxNumLODLevels; i++)
    {
        if (s->ReadBool())
            mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, -1), plRefFlags::kActiveRef); // shared mesh
    }

    fElements.assign(tileCount, nullptr);
    if (plClothingMgr::GetClothingMgr())
    {
        plGenRefMsg *msg = new plGenRefMsg(plClothingMgr::GetClothingMgr()->GetKey(), plRefMsg::kOnCreate, -1, -1);
        hsgResMgr::ResMgr()->AddViaNotify(GetKey(), msg, plRefFlags::kActiveRef); 
    }
    mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // forced accessory    

    for (int i = 0; i < 3; i++)
    {
        fDefaultTint1[i] = s->ReadByte();
        fDefaultTint2[i] = s->ReadByte();
    }   
}

void plClothingItem::Write(hsStream *s, hsResMgr *mgr)
{
    hsKeyedObject::Write(s, mgr);

    s->WriteSafeString(fName);
    s->WriteByte(fGroup);
    s->WriteByte(fType);
    s->WriteByte(fTileset);
    s->WriteByte(fSortOrder);

    s->WriteSafeString(fCustomText);
    s->WriteSafeString(fDescription);
    s->WriteBool(fThumbnail != nullptr);
    if (fThumbnail != nullptr)
        mgr->WriteKey(s, fThumbnail->GetKey());

    size_t texSkip = 0;
    for (plMipmap** texList : fTextures)
        if (texList == nullptr)
            texSkip++;

    s->WriteLE32((uint32_t)(fTextures.size() - texSkip));
    for (size_t i = 0; i < fTextures.size(); i++)
    {
        if (fTextures[i] == nullptr)
            continue;

        s->WriteSafeString(fElementNames[i]);

        uint8_t layerCount = 0;
        for (int j = 0; j < plClothingElement::kLayerMax; j++)
        {
            // Run through once to get the count of valid layers
            if (fTextures[i][j] != nullptr)
                layerCount++;
        }

        s->WriteByte(layerCount);
        for (uint8_t j = 0; j < plClothingElement::kLayerMax; j++)
        {
            if (fTextures[i][j] != nullptr)
            {
                s->WriteByte(j);
                mgr->WriteKey(s, fTextures[i][j]->GetKey());
            }
        }
    }

    for (int i = 0; i < kMaxNumLODLevels; i++)
    {
        s->WriteBool(fMeshes[i] != nullptr);
        if (fMeshes[i] != nullptr)
            mgr->WriteKey(s, fMeshes[i]->GetKey());
    }

    // EXPORT ONLY
    plKey accessoryKey;
    if (!fAccessoryName.empty())
    {
        ST::string strBuf = ST::format("CItm_{}", fAccessoryName);
        accessoryKey = plKeyFinder::Instance().StupidSearch("GlobalClothing", "", plClothingItem::Index(), strBuf);
        if (accessoryKey == nullptr)
        {
            strBuf = ST::format("Couldn't find accessory \"{}\". It won't show at runtime.", fAccessoryName);
            hsMessageBox(strBuf.c_str(), GetKeyName().c_str(), hsMessageBoxNormal);
        }
    }
    mgr->WriteKey(s, accessoryKey);
    
    for (int i = 0; i < 3; i++)
    {
        s->WriteByte(fDefaultTint1[i]);
        s->WriteByte(fDefaultTint2[i]);
    }
}

bool plClothingItem::MsgReceive(plMessage* msg)
{
    plElementRefMsg *eMsg = plElementRefMsg::ConvertNoRef(msg);
    if (eMsg)
    {
        plMipmap *tex = plMipmap::ConvertNoRef(eMsg->GetRef());
        if (tex)
        {
            if( eMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                if ((int32_t)fTextures.size() <= eMsg->fWhich)
                    fTextures.resize(eMsg->fWhich + 1);
                if ((int32_t)fElementNames.size() <= eMsg->fWhich)
                    fElementNames.resize(eMsg->fWhich + 1);
                
                if (fElementNames[eMsg->fWhich].empty())
                    fElementNames[eMsg->fWhich] = eMsg->fElementName;
                if (fTextures[eMsg->fWhich] == nullptr)
                {
                    plMipmap **layers = new plMipmap*[plClothingElement::kLayerMax];
                    std::fill(layers, layers + plClothingElement::kLayerMax, nullptr);
                    fTextures[eMsg->fWhich] = layers;
                }

                fTextures[eMsg->fWhich][eMsg->fLayer] = tex;
            }
            else if( eMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                fTextures[eMsg->fWhich][eMsg->fLayer] = nullptr;
            }
            return true;
        }
    }
    
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plSharedMesh *mesh = plSharedMesh::ConvertNoRef(refMsg->GetRef());
        if (mesh)
        {
            if (refMsg->fWhich >= 0 && refMsg->fWhich < kMaxNumLODLevels)
            {
                if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                    fMeshes[refMsg->fWhich] = mesh;
                else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                    fMeshes[refMsg->fWhich] = nullptr;
            }
            return true;
        }

        plMipmap *thumbnail = plMipmap::ConvertNoRef(refMsg->GetRef());
        if (thumbnail)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fThumbnail = thumbnail;
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                fThumbnail = nullptr;
            return true;
        }

        plClothingItem *accessory = plClothingItem::ConvertNoRef(refMsg->GetRef());
        if (accessory)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fAccessory = accessory;
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                fAccessory = nullptr;
            return true;
        }       
    }

    return hsKeyedObject::MsgReceive(msg);
}

/////////////////////////////////////////////////////////////////////////////

bool plClosetItem::IsMatch(const plClosetItem *other) const
{
    return (fItem == other->fItem && fOptions.IsMatch(&other->fOptions));
}

/////////////////////////////////////////////////////////////////////////////

plClothingBase::plClothingBase() : fBaseTexture() { }

void plClothingBase::Read(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Read(s, mgr);

    fName = s->ReadSafeString();
    if (s->ReadBool())
        mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef);
    fLayoutName = s->ReadSafeString();
}

void plClothingBase::Write(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Write(s, mgr);

    s->WriteSafeString(fName);
    s->WriteBool(fBaseTexture != nullptr);
    if (fBaseTexture != nullptr)
        mgr->WriteKey(s, fBaseTexture->GetKey());
    s->WriteSafeString(fLayoutName);
}

bool plClothingBase::MsgReceive(plMessage* msg)
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
        {
            fBaseTexture = plMipmap::ConvertNoRef(refMsg->GetRef());
        }
        else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
        {
            fBaseTexture = nullptr;
        }
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}


/////////////////////////////////////////////////////////////////////////////

plClothingOutfit::plClothingOutfit() : 
    fTargetLayer(nullptr), fBase(nullptr), fGroup(0), fAvatar(nullptr), fSynchClients(false), fMaterial(nullptr),
    fVaultSaveEnabled(true), fMorphsInitDone(false)
{
    fSkinTint.Set(1.f, 0.84f, 0.71f, 1.f);
    fItems.clear();
    for (int i = 0; i < plClothingLayout::kMaxTileset; i++)
        fDirtyItems.SetBit(i);

    for (int i = 0; i < plClothingElement::kLayerSkinLast - plClothingElement::kLayerSkinFirst; i++)
        fSkinBlends[i] = 0.f;
}

plClothingOutfit::~plClothingOutfit()
{
    fItems.clear();
    while (!fOptions.empty()) {
        delete fOptions.back();
        fOptions.pop_back();
    }
    plgDispatch::Dispatch()->UnRegisterForExactType(plPreResourceMsg::Index(), GetKey());
}

void plClothingOutfit::AddItem(plClothingItem *item, bool update /* = true */, bool broadcast /* = true */, bool netForce /* = false */)
{
    if (std::find(fItems.cbegin(), fItems.cend(), item) != fItems.cend())
        return;

    plClothingMsg *msg = new plClothingMsg();
    msg->AddReceiver(GetKey());
    msg->AddCommand(plClothingMsg::kAddItem);
    msg->fItemKey = item->GetKey();
    if (broadcast)
        msg->SetBCastFlag(plMessage::kNetPropagate);
    if ( netForce )
    {
        // also doesn't make much sense to not net propagate netForced messages
        msg->SetBCastFlag(plMessage::kNetPropagate);
        msg->SetBCastFlag(plMessage::kNetForce);
    }

    if (update)
        msg->AddCommand(plClothingMsg::kUpdateTexture);

    plgDispatch::MsgSend(msg);
}

void plClothingOutfit::ForceUpdate(bool retry)
{
    plClothingMsg *msg = new plClothingMsg();   
    msg->AddCommand(plClothingMsg::kUpdateTexture);
    if (retry)
        msg->AddCommand(plClothingMsg::kRetry);     // force a resend
    msg->Send(GetKey());
}

void plClothingOutfit::RemoveItem(plClothingItem *item, bool update /* = true */, bool netForce /* = false */)
{
    if (std::find(fItems.cbegin(), fItems.cend(), item) == fItems.cend())
        return;

    plClothingMsg *msg = new plClothingMsg();
    msg->AddReceiver(GetKey());
    msg->AddCommand(plClothingMsg::kRemoveItem);
    msg->SetBCastFlag(plMessage::kNetPropagate);
    if ( netForce )
        msg->SetBCastFlag(plMessage::kNetForce);
    msg->fItemKey = item->GetKey();
    if (update)
        msg->AddCommand(plClothingMsg::kUpdateTexture);

    plgDispatch::MsgSend(msg);
}

void plClothingOutfit::TintItem(plClothingItem *item, float red, float green, float blue,
                                bool update /* = true */, bool broadcast /* = true */, bool netForce /* = false */,
                                bool retry /* = true */, uint8_t layer /* = kLayerTint1 */)
{
    plClothingMsg *msg = new plClothingMsg();
    msg->AddReceiver(GetKey());
    msg->AddCommand(plClothingMsg::kTintItem);
    msg->fItemKey = item->GetKey();
    msg->fColor.Set(red, green, blue, 1.f);
    msg->fLayer = layer;
    if (broadcast)
        msg->SetBCastFlag(plMessage::kNetPropagate);
    if (netForce)
    {
        msg->SetBCastFlag(plMessage::kNetPropagate);
        msg->SetBCastFlag(plMessage::kNetForce);
    }
    if (update)
        msg->AddCommand(plClothingMsg::kUpdateTexture);
    if (retry)
        msg->AddCommand(plClothingMsg::kRetry);
    plgDispatch::MsgSend(msg);
}

void plClothingOutfit::TintSkin(float red, float green, float blue,
                                bool update /* = true */, bool broadcast /* = true */)
{
    plClothingMsg *msg = new plClothingMsg();
    msg->AddReceiver(GetKey());
    msg->AddCommand(plClothingMsg::kTintSkin);
    msg->fColor.Set(red, green, blue, 1.f);
    if (broadcast)
        msg->SetBCastFlag(plMessage::kNetPropagate);
    if (update)
        msg->AddCommand(plClothingMsg::kUpdateTexture);
    
    plgDispatch::MsgSend(msg);
}

void plClothingOutfit::MorphItem(plClothingItem *item, uint8_t layer, uint8_t delta, float weight,
                                 bool retry /* = true */)
{
    plClothingMsg *msg = new plClothingMsg();
    msg->AddReceiver(GetKey());
    msg->AddCommand(plClothingMsg::kMorphItem);
    msg->fItemKey = item->GetKey();
    msg->fLayer = layer;
    msg->fDelta = delta;
    msg->fWeight = weight;
    if (retry)
        msg->AddCommand(plClothingMsg::kRetry);
    plgDispatch::MsgSend(msg);
}

void plClothingOutfit::SetAge(float age, bool update /* = true */, bool broadcast /* = true */)
{
    SetSkinBlend(age, plClothingElement::kLayerSkinBlend1, update, broadcast);
}

void plClothingOutfit::SetSkinBlend(float blend, uint8_t layer, bool update /* = true */, bool broadcast /* = true */)
{
    plClothingMsg *msg = new plClothingMsg();
    msg->AddReceiver(GetKey());
    msg->AddCommand(plClothingMsg::kBlendSkin);
    msg->fLayer = layer;
    msg->fColor.Set(1.f, 1.f, 1.f, blend);
    if (broadcast)
        msg->SetBCastFlag(plMessage::kNetPropagate);
    if (update)
        msg->AddCommand(plClothingMsg::kUpdateTexture);
    
    plgDispatch::MsgSend(msg);
}

float plClothingOutfit::GetSkinBlend(uint8_t layer)
{
    if (layer >= plClothingElement::kLayerSkinBlend1 && layer <= plClothingElement::kLayerSkinLast)
        return fSkinBlends[layer - plClothingElement::kLayerSkinBlend1];

    return 0;
}

void plClothingOutfit::IAddItem(plClothingItem *item)
{
    if (item->fGroup != fGroup)
    {
        // Trying to wear clothing from the wrong group, remove the ref.
        GetKey()->Release(item->GetKey());
        return;
    }

    if (std::find(fItems.cbegin(), fItems.cend(), item) == fItems.cend())
    {
        // Remove any other item we have that can't be worn with this
        for (hsSsize_t i = fItems.size() - 1; i >= 0; i--)
        {
            if (!item->CanWearWith(fItems[i]))
            {
                plClothingItem *goner = fItems[i];
                if (goner->fAccessory)
                {
                    GetKey()->Release(goner->fAccessory->GetKey());
                    IRemoveItem(goner->fAccessory);
                }
                GetKey()->Release(goner->GetKey());
                IRemoveItem(goner); // Can't wait for the ref message to process                
            }
        }

        size_t iItem;
        for (iItem = 0; iItem < fItems.size(); iItem++)
        {
            if (item->WearBefore(fItems[iItem]))
                break;
        }
        fItems.emplace(fItems.begin() + iItem, item);
        plClothingItemOptions *op = new plClothingItemOptions;
        fOptions.emplace(fOptions.begin() + iItem, op);
        IInstanceSharedMeshes(item);
        fDirtyItems.SetBit(item->fTileset);
        
        // A bit of hackage for bare feet sound effects.
        if (item->fType == plClothingMgr::kTypeLeftFoot)
        {
            plArmatureEffectsMgr *mgr = fAvatar->GetArmatureEffects();
            plArmatureEffectFootSound *soundEffect = nullptr;
            size_t num = mgr->GetNumEffects();

            for (size_t i = 0; i < num; i++)
            {
                soundEffect = plArmatureEffectFootSound::ConvertNoRef(mgr->GetEffect(i));
                if (soundEffect)
                    break;
            }

            if (soundEffect)
            {
                if (item->fName == "03_MLFoot04_01" || item->fName == "03_FLFoot04_01")
                    soundEffect->SetFootType(plArmatureEffectFootSound::kFootTypeBare);
                else
                    soundEffect->SetFootType(plArmatureEffectFootSound::kFootTypeShoe);
            }
        }
    }   
}

void plClothingOutfit::IRemoveItem(plClothingItem *item)
{
    // We may just be removing the ref...
    auto iter = std::find(fItems.cbegin(), fItems.cend(), item);
    if (iter != fItems.cend())
    {
        auto index = std::distance(fItems.cbegin(), iter);
        fItems.erase(fItems.begin() + index);
        delete fOptions[index];
        fOptions.erase(fOptions.begin() + index);
        IRemoveSharedMeshes(item);
        fDirtyItems.SetBit(item->fTileset);
    }
}

bool plClothingOutfit::ITintItem(plClothingItem *item, hsColorRGBA color, uint8_t layer)
{
    auto iter = std::find(fItems.cbegin(), fItems.cend(), item);
    if (iter != fItems.cend())
    {
        auto index = std::distance(fItems.cbegin(), iter);
        if (layer == plClothingElement::kLayerTint1)
            fOptions[index]->fTint1 = color;
        if (layer == plClothingElement::kLayerTint2)
            fOptions[index]->fTint2 = color;
        fDirtyItems.SetBit(item->fTileset);

        if (fItems[index]->fAccessory)
        {
            plClothingItem *acc = fItems[index]->fAccessory;
            auto accIter = std::find(fItems.cbegin(), fItems.cend(), acc);
            if (accIter != fItems.cend())
            {
                auto accIndex = std::distance(fItems.cbegin(), accIter);
                if (layer == plClothingElement::kLayerTint1)
                    fOptions[accIndex]->fTint1 = color;
                if (layer == plClothingElement::kLayerTint2)
                    fOptions[accIndex]->fTint2 = color;
                fDirtyItems.SetBit(acc->fTileset);
            }
        }
        return true;
    }
    
    return false;
}

hsColorRGBA plClothingOutfit::GetItemTint(plClothingItem *item, uint8_t layer /* = kLayerTint1 */) const
{
    if (layer >= plClothingElement::kLayerSkinFirst &&
        layer <= plClothingElement::kLayerSkinLast)
        return fSkinTint;
    
    auto iter = std::find(fItems.cbegin(), fItems.cend(), item);
    if (iter != fItems.cend())
    {
        auto index = std::distance(fItems.cbegin(), iter);
        if (layer == plClothingElement::kLayerTint1)
            return fOptions[index]->fTint1;
        if (layer == plClothingElement::kLayerTint2)
            return fOptions[index]->fTint2;
    }
    
    hsColorRGBA color;
    color.Set(1.f, 1.f, 1.f, 1.f);
    return color;
}

bool plClothingOutfit::IMorphItem(plClothingItem *item, uint8_t layer, uint8_t delta, float weight)
{
    if (fAvatar && std::find(fItems.cbegin(), fItems.cend(), item) != fItems.cend())
    {
        for (uint8_t i = 0; i < fAvatar->GetNumLOD(); i++)
        {
            if (item->fMeshes[i]->fMorphSet == nullptr)
                continue;

            const plSceneObject *so = fAvatar->GetClothingSO(i);
            if (!so)
                continue;

            plMorphSequence *seq = const_cast<plMorphSequence*>(plMorphSequence::ConvertNoRef(so->GetModifierByType(plMorphSequence::Index())));
            plKey meshKey = item->fMeshes[i]->GetKey();

            // Lower LOD objects don't have all the morphs, so check if this one is in range.
            if (seq && layer < seq->GetNumLayers(meshKey) && delta < seq->GetNumDeltas(layer, meshKey))
                seq->SetWeight(layer, delta, weight, meshKey);
        }
        return true;
    }
    
    return false;
}

void plClothingOutfit::Read(hsStream* s, hsResMgr* mgr)
{
    plSynchedObject::Read(s, mgr);

    fGroup = s->ReadByte();
    mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // plClothingBase  

    if (fGroup != plClothingMgr::kClothingBaseNoOptions)
    {
        mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // target layer
        mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // material    
    }
    plgDispatch::Dispatch()->RegisterForExactType(plPreResourceMsg::Index(), GetKey());
    //ReadItems(s, mgr, false);
}

void plClothingOutfit::Write(hsStream* s, hsResMgr* mgr)
{
    plSynchedObject::Write(s, mgr);

    s->WriteByte(fGroup);
    mgr->WriteKey(s, fBase->GetKey());
    
    if (fGroup != plClothingMgr::kClothingBaseNoOptions)
    {
        mgr->WriteKey(s, fTargetLayer->GetKey());
        mgr->WriteKey(s, fMaterial->GetKey());
    }
    //WriteItems(s, mgr);
}

void plClothingOutfit::StripAccessories()
{
    for (hsSsize_t i = fItems.size() - 1; i >= 0; i--)
    {
        if (fItems[i]->fType == plClothingMgr::kTypeAccessory)
        {
            GetKey()->Release(fItems[i]->GetKey());
            IRemoveItem(fItems[i]);
        }
    }
}

void plClothingOutfit::IHandleMorphSDR(plStateDataRecord *sdr)
{
    plSimpleStateVariable *lodVar = sdr->FindVar(plMorphSequenceSDLMod::kStrTargetID);
    if (!lodVar)
        return;

    uint8_t lod;
    lodVar->Get(&lod);

    const plSceneObject *so = fAvatar->GetClothingSO(lod);
    if (!so)
        return;

    plMorphSequenceSDLMod *morph = const_cast<plMorphSequenceSDLMod*>(plMorphSequenceSDLMod::ConvertNoRef(so->GetModifierByType(plMorphSequenceSDLMod::Index())));
    if (morph)
        morph->SetCurrentStateFrom(sdr);
}

bool plClothingOutfit::ReadClothing()
{
    // Have we set a clothing file? If that's the case, load from there.
    if (fClothingFile.IsValid())
        return IReadFromFile(fClothingFile);
    else
        return IReadFromVault();
}

bool plClothingOutfit::IReadFromVault()
{
    SetupMorphSDL();

    WearDefaultClothing();

    hsRef<RelVaultNode> rvn = VaultGetAvatarOutfitFolder();
    if (!rvn)
        return false;

    RelVaultNode::RefList nodes;
    rvn->GetChildNodes(plVault::kNodeType_SDL, 1, &nodes);

    for (const hsRef<RelVaultNode> &node : nodes) {
        VaultSDLNode sdl(node);
        if (!sdl.GetSDLData().empty()) {
            hsRAMStream ram;
            ram.Write(sdl.GetSDLData().size(), sdl.GetSDLData().data());
            ram.Rewind();

            ST::string sdlRecName;
            int sdlRecVersion;
            plStateDataRecord::ReadStreamHeader(&ram, &sdlRecName, &sdlRecVersion);
            plStateDescriptor * desc = plSDLMgr::GetInstance()->FindDescriptor(sdlRecName, sdlRecVersion);
            if (desc) {
                plStateDataRecord * sdlDataRec = new plStateDataRecord(desc);
                if (sdlDataRec->Read(&ram, 0)) {
                    if (sdlRecName == kSDLMorphSequence)
                        IHandleMorphSDR(sdlDataRec);
                    else
                        plClothingSDLModifier::HandleSingleSDR(sdlDataRec, this);
                }
                delete sdlDataRec;              
            }
        }
    }
    
    fSynchClients = true; // set true if the next synch should be bcast
    ForceUpdate(true);
    
    return true;
}

void plClothingOutfit::SaveCustomizations(bool retry /* = true */)
{
    plClothingMsg *msg = new plClothingMsg();
    msg->AddReceiver(GetKey());
    msg->AddCommand(plClothingMsg::kSaveCustomizations);
    if (retry)
        msg->AddCommand(plClothingMsg::kRetry);

    msg->Send();
}

void plClothingOutfit::WriteToVault()
{
    if (!fVaultSaveEnabled)
        return;

    hsRef<RelVaultNode> rvn = VaultGetAvatarOutfitFolder();
    if (!rvn)
        return;

    std::vector<plStateDataRecord*> SDRs;
    
    plStateDataRecord clothingSDR(kSDLClothing);
    fAvatar->GetClothingSDLMod()->PutCurrentStateIn(&clothingSDR);
    plSDStateVariable * clothesStateDesc = clothingSDR.FindSDVar(plClothingSDLModifier::kStrWardrobe);

    for (unsigned i = 0; i < clothesStateDesc->GetCount(); ++i)
        SDRs.emplace_back(clothesStateDesc->GetStateDataRecord(i));

    plSDStateVariable * appearanceStateDesc = clothingSDR.FindSDVar(plClothingSDLModifier::kStrAppearance); // for skin tint
    SDRs.emplace_back(appearanceStateDesc->GetStateDataRecord(0));
    
    WriteToVault(SDRs);
}

void plClothingOutfit::WriteToVault(const std::vector<plStateDataRecord*> & SDRs)
{
    // We'll hit this case when the server asks us to save state for NPCs.
    if (fAvatar->GetTarget(0) != plNetClientApp::GetInstance()->GetLocalPlayer())
        return;

    hsRef<RelVaultNode> rvn = VaultGetAvatarOutfitFolder();
    if (!rvn)
        return;
        
    std::vector<plStateDataRecord*>   morphs;

    // Gather morph SDRs
    std::vector<const plMorphSequence*> morphsSDRs;
    plMorphSequence::FindMorphMods(fAvatar->GetTarget(0), morphsSDRs);
    for (const plMorphSequence* morphSeq : morphsSDRs) {
        for (unsigned j = 0; j < fAvatar->GetNumLOD(); j++) {
            if (fAvatar->GetClothingSO(j) == morphSeq->GetTarget(0)) {
                plStateDataRecord * morphSDR = new plStateDataRecord(kSDLMorphSequence);
                plSimpleStateVariable * lodVar = morphSDR->FindVar(plMorphSequenceSDLMod::kStrTargetID);
                if (lodVar)
                    lodVar->Set((int)j);

                morphSeq->GetSDLMod()->PutCurrentStateIn(morphSDR);
                morphs.emplace_back(morphSDR);
            }
        }
    }
    
    RelVaultNode::RefList templates;
    RelVaultNode::RefList actuals;
    RelVaultNode::RefList nodes;

    // Get all existing clothing SDRs
    rvn->GetChildNodes(plVault::kNodeType_SDL, 1, &nodes);    // REF: Find

    const std::vector<plStateDataRecord*> * arrs[] = {
        &SDRs,
        &morphs,
    };
    for (const auto * arr : arrs) {
        // Write all SDL to to the outfit folder, reusing existing nodes and creating new ones as necessary
        for (plStateDataRecord * rec : *arr) {
            hsRef<RelVaultNode> node;
            if (!nodes.empty()) {
                node = nodes.front();
                nodes.pop_front();
            }
            else {
                node.Steal(new RelVaultNode);
                node->SetNodeType(plVault::kNodeType_SDL);
                templates.push_back(node);
            }

            VaultSDLNode sdl(node);
            sdl.SetStateDataRecord(rec, 0);
        }
    }

    // Delete any leftover nodes
    for (const hsRef<RelVaultNode> &node : nodes)
        VaultDeleteNode(node->GetNodeId());

    // Create actual new nodes from their templates
    for (const hsRef<RelVaultNode> &temp : templates) {
        ENetError result;
        if (hsRef<RelVaultNode> actual = VaultCreateNodeAndWait(temp, &result))
            actuals.push_back(actual);
    }

    // Add new nodes to outfit folder
    for (const hsRef<RelVaultNode> &act : actuals)
        VaultAddChildNodeAndWait(rvn->GetNodeId(), act->GetNodeId(), plNetClientApp::GetInstance()->GetPlayerID());

    // Cleanup morph SDRs
    for (plStateDataRecord *morph : morphs)
        delete morph;
}

// XXX HACK. DON'T USE (this function exists for the temp console command Clothing.SwapClothTexHACK)
void plClothingOutfit::DirtyTileset(int tileset)
{
    fDirtyItems.SetBit(tileset);
    ForceUpdate(true);
}

void plClothingOutfit::IUpdate()
{
    //GenerateTexture();
    fAvatar->RefreshTree();
    DirtySynchState(kSDLClothing, 0);
    
    plArmatureMod * avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    hsAssert(avMod,"plArmatureMod not found for local player");
    if (avMod->GetClothingOutfit()==this)
    {
        // Let the GUI know we changed clothes
        plClothingUpdateBCMsg *BCMsg = new plClothingUpdateBCMsg();
        BCMsg->SetSender(GetKey());
        plgDispatch::MsgSend(BCMsg);
    }
}

void plClothingOutfit::WearDefaultClothing(bool broadcast)
{
    StripAccessories();
    
    plClothingMgr *cMgr = plClothingMgr::GetClothingMgr();
    std::vector<plClothingItem *> items;
    cMgr->GetItemsByGroup(fGroup, items);

    // Wear one thing of each type
    for (int i = 0; i < plClothingMgr::kMaxType; i++)
    {
        if (i == plClothingMgr::kTypeAccessory)
            continue;

        for (plClothingItem* item : items)
        {
            if (item->fType == i)
            {
                AddItem(item, false, false);
                if (i == plClothingMgr::kTypeHair || i == plClothingMgr::kTypeFace)
                {   
                    // Hair tint color
                    TintItem(item, 0.5f, 0.3f, 0.2f, false, false);
                }
                else
                {
                    TintItem(item, item->fDefaultTint1[0] / 255.f, item->fDefaultTint1[1] / 255.f,
                             item->fDefaultTint1[2] / 255.f, false, false);
                }

                // Everyone can tint layer 2. Go nuts!
                TintItem(item, item->fDefaultTint2[0] / 255.f, item->fDefaultTint2[1] / 255.f,
                         item->fDefaultTint2[2] / 255.f, false, false, false, true, plClothingElement::kLayerTint2);
                break;
            }
        }
    }

    if (broadcast) {
        fSynchClients = true;
        ForceUpdate(true);
    }
}

void plClothingOutfit::WearDefaultClothingType(uint32_t clothingType, bool broadcast)
{
    plClothingMgr *cMgr = plClothingMgr::GetClothingMgr();
    std::vector<plClothingItem *> items;
    cMgr->GetItemsByGroup(fGroup, items);

    for (plClothingItem* item : items)
    {
        if (item->fType == clothingType)
        {
            AddItem(item, false, false);
            if (clothingType == plClothingMgr::kTypeHair || clothingType == plClothingMgr::kTypeFace)
            {
                // Hair tint color
                TintItem(item, 0.5f, 0.3f, 0.2f, false, false);
            }
            else
            {
                TintItem(item, item->fDefaultTint1[0] / 255.f, item->fDefaultTint1[1] / 255.f,
                         item->fDefaultTint1[2] / 255.f, false, false);
            }

            // Everyone can tint layer 2. Go nuts!
            TintItem(item, item->fDefaultTint2[0] / 255.f, item->fDefaultTint2[1] / 255.f,
                     item->fDefaultTint2[2] / 255.f, false, false, false, true, plClothingElement::kLayerTint2);
            break;
        }
    }

    if (broadcast) {
        fSynchClients = true;
        ForceUpdate(true);
    }
}

void plClothingOutfit::WearMaintainerOutfit()
{
    fVaultSaveEnabled = false;

    WearDefaultClothing();
    plClothingItem *item;
    
    if (fGroup == plClothingMgr::kClothingBaseMale)
    {
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_MHAcc_SuitHelmet");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_MLHand_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_MRHand_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_MTorso_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_MLegs_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_MLFoot_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_MRFoot_Suit");
        if (item)
            AddItem(item, false, false);
    }
    else if (fGroup == plClothingMgr::kClothingBaseFemale)
    {
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_FHair_SuitHelmet");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_FLHand_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_FRHand_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_FTorso_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_FLegs_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_FLFoot_Suit");
        if (item)
            AddItem(item, false, false);
        item = plClothingMgr::GetClothingMgr()->FindItemByName("03_FRFoot_Suit");
        if (item)
            AddItem(item, false, false);
    }
    
    fSynchClients = true;
    ForceUpdate(true);
}

void plClothingOutfit::RemoveMaintainerOutfit()
{
    ReadClothing();

    fVaultSaveEnabled = true;
}

void plClothingOutfit::WearRandomOutfit()
{
    static plRandom sRandom;

    plClothingMgr *cMgr = plClothingMgr::GetClothingMgr();
    std::vector<plClothingItem *> items;

    // Wear one thing of each type
    for (uint8_t i = 0; i < plClothingMgr::kMaxType; i++)
    {
        if (i == plClothingMgr::kTypeAccessory)
            continue;

        items.clear();
        cMgr->GetItemsByGroupAndType(fGroup, i, items);
        size_t j = (size_t)(sRandom.RandZeroToOne() * items.size());

        float r1 = sRandom.RandZeroToOne();
        float g1 = sRandom.RandZeroToOne();
        float b1 = sRandom.RandZeroToOne();
        float r2 = sRandom.RandZeroToOne();
        float g2 = sRandom.RandZeroToOne();
        float b2 = sRandom.RandZeroToOne();

        AddItem(items[j], false, false);
        TintItem(items[j], r1, g1, b1, false, false, false, true, 1);
        TintItem(items[j], r2, g2, b2, false, false, false, true, 2);

        plClothingItem *match = cMgr->GetLRMatch(items[j]);
        if (match)
        {
            AddItem(match, false, false);
            TintItem(match, r1, g1, b1, false, false, false, true, 1);
            TintItem(match, r2, g2, b2, false, false, false, true, 2);
        }
    }
    TintSkin(sRandom.RandZeroToOne(), sRandom.RandZeroToOne(), sRandom.RandZeroToOne());
}

bool plClothingOutfit::ReadItems(hsStream* s, hsResMgr* mgr, bool broadcast /* = true */)
{
    bool result = true;
    uint32_t numItems = s->ReadLE32();
    int i;
    for (i = 0; i < numItems; i++)
    {
        plKey key = mgr->ReadKey( s );
        hsColorRGBA color;
        hsColorRGBA color2;
        color.Read(s);
        color2.Read(s);
        
        // Make sure to read everything in before hitting this and possibly skipping to
        // the next item, lest we disrupt the stream.
        if (key == nullptr)
        {
            hsAssert( false, "Nil item in plClothingOutfit::ReadItems(). The vault probably contains a key with a plLocation that's moved since then. Tsk, tsk." );
            result = false;
            continue;
        }

        plClothingItem *item = plClothingItem::ConvertNoRef( key->GetObjectPtr() );
        AddItem(item, false, broadcast); 
        TintItem(item, color.r, color.g, color.b, false, broadcast, false, true, plClothingElement::kLayerTint1);
        TintItem(item, color2.r, color2.g, color2.b, false, broadcast, false, true, plClothingElement::kLayerTint2);
    }

    return result;
}

void plClothingOutfit::WriteItems(hsStream *s, hsResMgr *mgr)
{
    s->WriteLE32((uint32_t)fItems.size());
    for (size_t i = 0; i < fItems.size(); i++)
    {
        mgr->WriteKey(s, fItems[i]->GetKey());
        fOptions[i]->fTint1.Write(s);
        fOptions[i]->fTint2.Write(s);
    }
}

bool plClothingOutfit::MsgReceive(plMessage* msg)
{
    plPreResourceMsg *preMsg = plPreResourceMsg::ConvertNoRef(msg);
    if (preMsg)
    {
        if (fAvatar && fGroup != plClothingMgr::kClothingBaseNoOptions)
        {
            plDrawable *spans = fAvatar->FindDrawable();
            // This is a bit hacky... The drawable code has just run through and updated
            // each span's bounds (see plDrawableSpans::IUpdateMatrixPaletteBoundsHack())
            // but not the world bounds for the entire drawable. So we tell the space tree
            // to refresh. However, the pageTreeMgr would then get confused because the
            // space tree is no longer dirty (see plPageTreeMgr::IRefreshTree()), 
            // causing the avatar to only draw if the origin is in view. 
            // So we just force it dirty, and everyone's happy.
            spans->GetSpaceTree()->Refresh();
            spans->GetSpaceTree()->MakeDirty();

            // Where were we? Oh yeah... if this avatar is in view it needs a texture. Tell
            // the pipeline.
            const hsBounds3Ext &bnds = spans->GetSpaceTree()->GetWorldBounds();
            if ((bnds.GetType() == kBoundsNormal) && preMsg->Pipeline()->TestVisibleWorld(bnds))
                preMsg->Pipeline()->SubmitClothingOutfit(this);
        }
    }
 
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plLayer *layer = plLayer::ConvertNoRef(refMsg->GetRef());
        if (layer)
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fTargetLayer = layer;
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                fTargetLayer = nullptr;
            
            return true;
        }
        plClothingItem *item = plClothingItem::ConvertNoRef(refMsg->GetRef());
        if (item)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest) )
            {
                IAddItem(item);
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                IRemoveItem(item);
            }
            return true;
        }
        plClothingBase *base = plClothingBase::ConvertNoRef(refMsg->GetRef());
        if (base)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fBase = base;
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                fBase = nullptr;
            
            return true;        
        }
        
        hsGMaterial *mat = hsGMaterial::ConvertNoRef(refMsg->GetRef());
        if (mat)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                fMaterial = mat;
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                fMaterial = nullptr;
        }
    }

    // For now: Adding an item fires off a notify, and the item won't get added until the ref message
    // is processed, BUT we need to make sure that task doesn't require any more messages (like the
    // trickling of chains of AddViaNotifies), so that other messages received after it (like tint
    // commands) just have to check a kRetry flag, resend the message, and be confident that the item
    // will be present by the time their resent message is handled.
    // 
    // Should we ever handle AddViaNotify in a separate thread, this will blow up. (Ok, we'll just have
    // bugs about the characters not having the right clothings options visible. No explosions.)

    plClothingMsg *cMsg = plClothingMsg::ConvertNoRef(msg);
    if (cMsg)
    {
        if (cMsg->GetCommand(plClothingMsg::kAddItem))
        {
            plGenRefMsg *msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
            hsgResMgr::ResMgr()->AddViaNotify(cMsg->fItemKey, msg, plRefFlags::kActiveRef);
            plClothingItem *accessory = plClothingItem::ConvertNoRef(cMsg->fItemKey->GetObjectPtr())->fAccessory;
            if (accessory)
            {
                plGenRefMsg *msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
                hsgResMgr::ResMgr()->AddViaNotify(accessory->GetKey(), msg, plRefFlags::kActiveRef);
            }               
        }
        if (cMsg->GetCommand(plClothingMsg::kRemoveItem))
        {
            plClothingItem *item = plClothingItem::ConvertNoRef(cMsg->fItemKey->GetObjectPtr());
            if (item && item->fType == plClothingMgr::kTypeAccessory)
                GetKey()->Release(cMsg->fItemKey);
        }
        if (cMsg->GetCommand(plClothingMsg::kTintItem))
        {   
            if (!ITintItem(plClothingItem::ConvertNoRef(cMsg->fItemKey->GetObjectPtr()), cMsg->fColor, cMsg->fLayer) &&
                cMsg->GetCommand(plClothingMsg::kRetry))
            {
                // We failed to tint because we're not yet wearing the item.
                // However, the kRetry flag is set, so we fire another tint command off

                TintItem(plClothingItem::ConvertNoRef(cMsg->fItemKey->GetObjectPtr()), 
                         cMsg->fColor.r, cMsg->fColor.g, cMsg->fColor.b,
                         cMsg->GetCommand(plClothingMsg::kUpdateTexture), false, false, false, cMsg->fLayer);
                return true;
            }
        }
        if (cMsg->GetCommand(plClothingMsg::kMorphItem))
        {
            if (cMsg->GetCommand(plClothingMsg::kRetry))
                MorphItem(plClothingItem::ConvertNoRef(cMsg->fItemKey->GetObjectPtr()), cMsg->fLayer, cMsg->fDelta, cMsg->fWeight, false);
            else
                IMorphItem(plClothingItem::ConvertNoRef(cMsg->fItemKey->GetObjectPtr()), cMsg->fLayer, cMsg->fDelta, cMsg->fWeight);
        }

        if (cMsg->GetCommand(plClothingMsg::kTintSkin))
        {
            fSkinTint = cMsg->fColor;
            for (plClothingItem* item : fItems)
                for (size_t j = 0; j < item->fElements.size(); j++)
                    if (item->fTextures[j][plClothingElement::kLayerSkin] != nullptr)
                        fDirtyItems.SetBit(item->fTileset);
        }

        if (cMsg->GetCommand(plClothingMsg::kBlendSkin))
        {
            float blend = cMsg->fColor.a;
            if (blend > 1.f)
                blend = 1.f;
            if (blend < 0.f)
                blend = 0.f;
            if (cMsg->fLayer >= plClothingElement::kLayerSkinBlend1 &&
                cMsg->fLayer <= plClothingElement::kLayerSkinBlend6)
            {
                fSkinBlends[cMsg->fLayer - plClothingElement::kLayerSkinBlend1] = blend;

                for (plClothingItem* item : fItems)
                    for (size_t j = 0; j < item->fElements.size(); j++)
                        if (item->fTextures[j][cMsg->fLayer] != nullptr)
                            fDirtyItems.SetBit(item->fTileset);
            }
        }
        if (cMsg->GetCommand(plClothingMsg::kSaveCustomizations))
        {
            if (cMsg->GetCommand(plClothingMsg::kRetry))
                SaveCustomizations(false);
            else
                WriteToVault();
        }

        // Make sure to check for an update last
        if (cMsg->GetCommand(plClothingMsg::kUpdateTexture))
        {
            // If kUpdateTexture was not the only command, we need to resend the update
            // as a solo command, so that it happens after any other AddViaNotify messages
            if (cMsg->ResendUpdate())
            {
                plClothingMsg *update = new plClothingMsg();
                update->AddReceiver(GetKey());
                update->AddCommand(plClothingMsg::kUpdateTexture);
                plgDispatch::MsgSend(update);
            }
            else
                IUpdate();
        }
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

//
// TESTING SDL
// Send clothing sendState msg to object's plClothingSDLModifier
//
bool plClothingOutfit::DirtySynchState(const ST::string& SDLStateName, uint32_t synchFlags)
{
    plSynchEnabler ps(true);    // make sure synching is enabled, since this happens during load
    synchFlags |= plSynchedObject::kForceFullSend;  // TEMP
    if (fSynchClients)
        synchFlags |= plSynchedObject::kBCastToClients;
    fSynchClients=false;
    hsAssert(fAvatar, "nil fAvatar");
    return fAvatar->GetTarget(0)->DirtySynchState(SDLStateName, synchFlags);
}

// Note: Currently the uint16_t "instance" is a lie. We just copy. In the future
// we'll be good about this, but I wanted to get it working first.
void plClothingOutfit::IInstanceSharedMeshes(plClothingItem *item)
{
    if (!fAvatar)
        return;

    fAvatar->ValidateMesh();

    bool partialSort = item->fCustomText.contains("NeedsSort");
    for (int i = 0; i < plClothingItem::kMaxNumLODLevels; i++)
    {
        const plSceneObject *so = fAvatar->GetClothingSO(i);
        if (so && item->fMeshes[i])
        {
            plInstanceDrawInterface *idi = const_cast<plInstanceDrawInterface*>(plInstanceDrawInterface::ConvertNoRef(so->GetDrawInterface()));
            if (idi)
                idi->AddSharedMesh(item->fMeshes[i], fMaterial, !item->HasBaseAlpha(), i, partialSort);
        }
    }
}

void plClothingOutfit::IRemoveSharedMeshes(plClothingItem *item)
{   
    if (fAvatar == nullptr)
        return;

    int i;
    for (i = 0; i < plClothingItem::kMaxNumLODLevels; i++)
    {
        const plSceneObject *so = fAvatar->GetClothingSO(i);
        if (so != nullptr && item->fMeshes[i] != nullptr)
        {
            plInstanceDrawInterface *idi = const_cast<plInstanceDrawInterface*>(plInstanceDrawInterface::ConvertNoRef(so->GetDrawInterface()));
            if (idi)
                idi->RemoveSharedMesh(item->fMeshes[i]);            
        }
    }
}

void plClothingOutfit::SetupMorphSDL()
{
    if (!fMorphsInitDone)
    {
        std::vector<const plMorphSequence*> morphs;
        plMorphSequence::FindMorphMods(fAvatar->GetTarget(0), morphs);
        for (const plMorphSequence* morphSeq : morphs)
        {
            for (unsigned j = 0; j < fAvatar->GetNumLOD(); j++)
            {
                if (fAvatar->GetClothingSO(j) == morphSeq->GetTarget(0))
                {
                    plMorphSequenceSDLMod* morph = morphSeq->GetSDLMod();
                    if (morph)
                        morph->SetIsAvatar(true);
                }
            }
        }

        fMorphsInitDone = true;
    }
}

bool plClothingOutfit::WriteToFile(const plFileName &filename)
{
    if (!filename.IsValid())
        return false;

    hsRef<RelVaultNode> rvn = VaultGetAvatarOutfitFolder();
    if (!rvn)
        return false;

    hsUNIXStream S;
    if (!S.Open(filename, "wb"))
        return false;

    S.WriteByte(fGroup);

    RelVaultNode::RefList nodes;
    rvn->GetChildNodes(plVault::kNodeType_SDL, 1, &nodes);
    S.WriteLE32((uint32_t)nodes.size());
    for (const hsRef<RelVaultNode> &node : nodes) {
        VaultSDLNode sdl(node);
        S.WriteLE32((uint32_t)sdl.GetSDLData().size());
        if (!sdl.GetSDLData().empty())
            S.Write(sdl.GetSDLData().size(), sdl.GetSDLData().data());
    }

    S.Close();
    return true;
}

bool plClothingOutfit::IReadFromFile(const plFileName &filename)
{
    if (!filename.IsValid())
        return false;

    hsUNIXStream S;
    if (!S.Open(filename))
        return false;

    bool isLocalAvatar = plAvatarMgr::GetInstance()->GetLocalAvatar()->GetClothingOutfit() == this;

    uint8_t gender = S.ReadByte();
    if (gender != fGroup) {
        if (isLocalAvatar) {
            if (gender == plClothingMgr::kClothingBaseMale)
                plClothingMgr::ChangeAvatar("Male", filename);
            else if (gender == plClothingMgr::kClothingBaseFemale)
                plClothingMgr::ChangeAvatar("Female", filename);
        }
        S.Close();
        return true;
    }

    StripAccessories();

    uint32_t nodeCount = S.ReadLE32();
    for (size_t i = 0; i < nodeCount; i++) {
        uint32_t dataLen = S.ReadLE32();
        if (dataLen) {
            ST::string sdlRecName;
            int sdlRecVersion;
            plStateDataRecord::ReadStreamHeader(&S, &sdlRecName, &sdlRecVersion);
            plStateDescriptor* desc = plSDLMgr::GetInstance()->FindDescriptor(sdlRecName, sdlRecVersion);
            if (desc) {
                plStateDataRecord sdlDataRec(desc);
                if (sdlDataRec.Read(&S, 0)) {
                    if (sdlRecName == kSDLMorphSequence)
                        IHandleMorphSDR(&sdlDataRec);
                    else
                        plClothingSDLModifier::HandleSingleSDR(&sdlDataRec, this);
                }
            }
        }
    }

    S.Close();
    fSynchClients = true;
    ForceUpdate(true);
    SaveCustomizations(); // Sync with the vault
    return true;
}


/////////////////////////////////////////////////////////////////////////////

using namespace ST::literals;

const ST::string plClothingMgr::GroupStrings[] = 
{
    "Male Clothing"_st,
    "Female Clothing"_st,
    "(No Clothing Options)"_st
};

const ST::string plClothingMgr::TypeStrings[] =
{
    "Pants"_st,
    "Shirt"_st,
    "LeftHand"_st,
    "RightHand"_st,
    "Face"_st,
    "Hair"_st,
    "LeftFoot"_st,
    "RightFoot"_st,
    "Accessory"_st
};

plClothingMgr *plClothingMgr::fInstance = nullptr;

plClothingMgr::~plClothingMgr()
{
    while (!fElements.empty()) {
        delete fElements.back();
        fElements.pop_back();
    }
    while (!fLayouts.empty()) {
        delete fLayouts.back();
        fLayouts.pop_back();
    }
    while (!fItems.empty()) {
        delete fItems.back();
        fItems.pop_back();
    }
}

plClothingLayout *plClothingMgr::GetLayout(const ST::string &name) const
{
    for (plClothingLayout* layout : fLayouts)
    {
        if (layout->fName == name)
            return layout;
    }
    return nullptr;
}

plClothingElement *plClothingMgr::FindElementByName(const ST::string &name) const
{
    for (plClothingElement* element : fElements)
    {
        if (element->fName == name)
            return element;
    }
    return nullptr;
}

void plClothingMgr::AddItemsToCloset(const std::vector<plClosetItem> &items)
{
    hsRef<RelVaultNode> rvn = VaultGetAvatarClosetFolder();
    if (!rvn)
        return;
        
    std::vector<plClosetItem> closet;
    GetClosetItems(closet);
    
    RelVaultNode::RefList templates;

    for (const plClosetItem& item : items) {
        bool match = std::any_of(closet.cbegin(), closet.cend(),
                                 [&item](const plClosetItem& closetItem) {
                                     return closetItem.IsMatch(&item);
                                 });
        if (match)
            continue;

        plStateDataRecord rec(plClothingSDLModifier::GetClothingItemSDRName());
        plClothingSDLModifier::PutSingleItemIntoSDR(&item, &rec);
        
        hsRef<RelVaultNode> templateNode(new RelVaultNode, hsStealRef);
        templateNode->SetNodeType(plVault::kNodeType_SDL);
        
        VaultSDLNode sdl(templateNode);
        sdl.SetStateDataRecord(&rec);

        templates.push_back(templateNode);
    }
    
    for (const hsRef<RelVaultNode> &temp : templates) {
        ENetError result;
        if (hsRef<RelVaultNode> actual = VaultCreateNodeAndWait(temp, &result)) {
            VaultAddChildNodeAndWait(
                rvn->GetNodeId(),
                actual->GetNodeId(),
                plNetClientApp::GetInstance()->GetPlayerID()
            );
        }
    }
}

void plClothingMgr::GetClosetItems(std::vector<plClosetItem> &out)
{
    hsRef<RelVaultNode> rvn = VaultGetAvatarClosetFolder();
    if (!rvn)
        return;

    RelVaultNode::RefList nodes;
    rvn->GetChildNodes(plVault::kNodeType_SDL, 1, &nodes);
    out.resize(nodes.size());
    
    auto iter = nodes.begin();
    for (unsigned i = 0; i < nodes.size(); ++i, ++iter) {
        VaultSDLNode sdl(*iter);
        plStateDataRecord * rec = new plStateDataRecord;
        if (sdl.GetStateDataRecord(rec, 0))
            plClothingSDLModifier::HandleSingleSDR(rec, nullptr, &out[i]);
        delete rec;
    }

    for (auto iter = out.cbegin(); iter != out.cend(); ) {
        if (iter->fItem == nullptr)
            iter = out.erase(iter);
        else
            ++iter;
    }
}

void plClothingMgr::GetAllWithSameMesh(plClothingItem *item, std::vector<plClothingItem*> &out)
{
    for (plClothingItem* myItem : fItems)
    {
        if (item->HasSameMeshes(myItem))
            out.emplace_back(myItem);
    }
}

// Yes, it's a lame n^2 function. Show me that we have enough items for it
// to matter and I'll speed it up.
void plClothingMgr::FilterUniqueMeshes(std::vector<plClothingItem*> &items)
{
    for (auto i = items.cbegin(); i != items.cend(); ++i)
    {
        for (auto j = std::next(i); j != items.cend(); )
        {
            if ((*i)->HasSameMeshes(*j))
                j = items.erase(j);
            else
                ++j;
        }
    }
}

plClothingItem *plClothingMgr::FindItemByName(const ST::string &name) const
{
    if (name.empty())
        return nullptr;

    for (plClothingItem* item : fItems)
    {
        if (item->fName == name)
            return item;
    }
    return nullptr;
}

void plClothingMgr::GetItemsByGroup(uint8_t group, std::vector<plClothingItem*> &out)
{
    for (plClothingItem* item : fItems)
    {
        if (item->fGroup == group)
            out.emplace_back(item);
    }
}

void plClothingMgr::GetItemsByGroupAndType(uint8_t group, uint8_t type, std::vector<plClothingItem*> &out)
{
    for (plClothingItem* item : fItems)
    {
        if (item->fGroup == group && item->fType == type)
            out.emplace_back(item);
    }
}

plClothingItem *plClothingMgr::GetLRMatch(plClothingItem *item)
{
    for (plClothingItem* myItem : fItems)
    {
        if (IsLRMatch(item, myItem))
            return myItem;
    }

    // Couldn't find one.
    return nullptr;
}

bool plClothingMgr::IsLRMatch(plClothingItem *item1, plClothingItem *item2)
{
    if (!item1 || !item2)
        return false;

    switch (item1->fType)
    {
    case kTypeLeftHand:
        if (item2->fType != kTypeRightHand) return false;
        break;
    case kTypeRightHand:
        if (item2->fType != kTypeLeftHand) return false;
        break;
    case kTypeLeftFoot:
        if (item2->fType != kTypeRightFoot) return false;
        break;
    case kTypeRightFoot:
        if (item2->fType != kTypeLeftFoot) return false;
        break;
    default:
        // if its not a matching kinda item, then there can be no match
        return false;
    }

    // Types check out fine, now compare textures
    if (item1->fTextures.size() != item2->fTextures.size())
        return false;

    for (size_t i = 0; i < item1->fTextures.size(); i++)
    {
        for (int j = 0; j < plClothingElement::kLayerMax; j++)
            if (item1->fTextures[i][j] != item2->fTextures[i][j])
                return false;
    }

    // Finally... we're not our own match
    return item1 != item2;
}

void plClothingMgr::Init()
{
    fInstance = new plClothingMgr;
    fInstance->RegisterAs(kClothingMgr_KEY);
    fInstance->IInit();
}

void plClothingMgr::IInit()
{
    plClothingElement::GetElements(fElements);
    plClothingLayout *layout = new plClothingLayout("BasicHuman", 1024);
    layout->fElements.emplace_back(FindElementByName("shirt-chest"));
    layout->fElements.emplace_back(FindElementByName("shirt-sleeve"));
    layout->fElements.emplace_back(FindElementByName("face"));
    layout->fElements.emplace_back(FindElementByName("eyeball"));
    layout->fElements.emplace_back(FindElementByName("shoe-top"));
    layout->fElements.emplace_back(FindElementByName("shoe-bottom"));
    layout->fElements.emplace_back(FindElementByName("pants"));
    layout->fElements.emplace_back(FindElementByName("hand-LOD"));
    layout->fElements.emplace_back(FindElementByName("hand-square"));
    layout->fElements.emplace_back(FindElementByName("hand-wide"));
    
    fLayouts.emplace_back(layout);
}

void plClothingMgr::DeInit()
{
    if (fInstance)
    {
        fInstance->UnRegisterAs(kClothingMgr_KEY);
        fInstance = nullptr;
    }
}   

bool plClothingMgr::MsgReceive(plMessage* msg)
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plClothingItem *item = plClothingItem::ConvertNoRef(refMsg->GetRef());
        if (item)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate) )
            {
                IAddItem(item);
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                auto idx = std::find(fItems.cbegin(), fItems.cend(), item);
                if (idx != fItems.end())
                    fItems.erase(idx);
            }
            return true;
        }
    }

    return hsKeyedObject::MsgReceive(msg);
}

void plClothingMgr::IAddItem(plClothingItem *item)
{
    bool allFound = true;
    for (size_t i = 0; i < item->fElementNames.size(); i++)
    {
        size_t j;
        for (j = 0; j < fElements.size(); j++)
        {   
            if (item->fElementNames[i] == fElements[j]->fName)
            {
                item->fElements[i] = fElements[j];
                break;
            }
        }
        if (j >= fElements.size())
        {
            allFound = false;
            break;
        }
    }

    if (allFound)
    {
        auto iter =  fItems.cbegin();
        for (; iter != fItems.cend(); ++iter)
        {
            if ((*iter)->fSortOrder >= item->fSortOrder)
                break;
        }
        fItems.insert(iter, item);
    }
    else
        hsAssert(false, "Couldn't match all elements of added clothing item.");
}

void plClothingMgr::ChangeAvatar(const ST::string& name, const plFileName &clothingFile)
{
    plAvatarMgr::GetInstance()->UnLoadLocalPlayer();
    plAvatarMgr::GetInstance()->LoadPlayerFromFile(name, "", clothingFile);
}
