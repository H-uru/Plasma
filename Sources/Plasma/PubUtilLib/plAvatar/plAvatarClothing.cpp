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
#include "hsTypes.h"
#include "hsTemplates.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plGDispatch.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plDrawable/plInstanceDrawInterface.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plPipeResMakeMsg.h"
#include "../pfMessage/plClothingMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../plGImage/plMipmap.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "../plPipeline/plRenderTarget.h"
#include "plPipeline.h"
#include "plClothingLayout.h"
#include "plAvatarClothing.h"
#include "plClothingSDLModifier.h"
#include "../plGImage/hsCodecManager.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureEffects.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../plMessage/plReplaceGeometryMsg.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plSharedMesh.h"
#include "../plDrawable/plMorphSequence.h"
#include "../plDrawable/plMorphSequenceSDLMod.h"
#include "../plDrawable/plSpaceTree.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayer.h"
#include "../plMath/plRandom.h"
#include "../plSDL/plSDL.h"
#include "../plVault/plVault.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plNetClientComm/plNetClientComm.h"


plClothingItem::plClothingItem() : fName(nil), fGroup(0), fTileset(0), fType(0), fSortOrder(0),
								   fDescription(nil), fCustomText(nil), fThumbnail(nil), 
								   fAccessory(nil), fAccessoryName(nil)
{	
	int i;
	fTextures.Reset();
	fElementNames.Reset();
	fElements.Reset();
	
	for (i = 0; i < 3; i++)
	{
		fDefaultTint1[i] = fDefaultTint2[i] = 255;
	}
	for (i = 0; i < kMaxNumLODLevels; i++)
		fMeshes[i] = nil;
}

plClothingItem::~plClothingItem()
{
	while (fElementNames.GetCount() > 0)
		delete [] fElementNames.Pop();

	while (fTextures.GetCount() > 0)
		delete [] fTextures.Pop();
	
	delete [] fName;
	delete [] fDescription;
	delete [] fCustomText;
	delete [] fAccessoryName;
}

hsBool plClothingItem::CanWearWith(plClothingItem *item)
{ 
	// For now, you can only wear one shirt, one pair of pants, etc.
	// Except accessories, of which you're allowed one per tileset.
	return ((item->fType != fType) || 
			(item->fType == plClothingMgr::kTypeAccessory && 
			 fType == plClothingMgr::kTypeAccessory));
			 //&& item->fTileset != fTileset));
}

hsBool plClothingItem::WearBefore(plClothingItem *item)
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

hsBool plClothingItem::HasBaseAlpha()
{
	int i;
	for (i = 0; i < fElements.GetCount(); i++)
	{
		plMipmap *tex = fTextures[i][plClothingElement::kLayerBase];
//		if (tex && (tex->GetFlags() & (plMipmap::kAlphaBitFlag | plMipmap::kAlphaChannelFlag)))
		if (tex && (tex->GetFlags() & plMipmap::kAlphaChannelFlag))
			return true;
	}
	return false;
}

hsBool plClothingItem::HasSameMeshes(plClothingItem *other)
{
	int i;
	for (i = 0; i < kMaxNumLODLevels; i++)
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
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // thumbnail

	int tileCount = s->ReadSwap32();
	int i, j;
	for (i = 0; i < tileCount; i++)
	{
		fElementNames.Append(s->ReadSafeString());

		int layerCount = s->ReadByte();
		for (j = 0; j < layerCount; j++)
		{
			int layer = s->ReadByte();
			mgr->ReadKeyNotifyMe(s, TRACKED_NEW plElementRefMsg(GetKey(), plRefMsg::kOnCreate, i, -1, nil, layer), plRefFlags::kActiveRef); // texture
		}
	}

	for (i = 0; i < kMaxNumLODLevels; i++)
	{
		if (s->ReadBool())
			mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, i, -1), plRefFlags::kActiveRef); // shared mesh
	}

	fElements.SetCountAndZero(tileCount);
	if (plClothingMgr::GetClothingMgr())
	{
		plGenRefMsg *msg = TRACKED_NEW plGenRefMsg(plClothingMgr::GetClothingMgr()->GetKey(), plRefMsg::kOnCreate, -1, -1);
		hsgResMgr::ResMgr()->AddViaNotify(GetKey(), msg, plRefFlags::kActiveRef); 
	}
	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // forced accessory	

	for (i = 0; i < 3; i++)
	{
		fDefaultTint1[i] = s->ReadByte();
		fDefaultTint2[i] = s->ReadByte();
	}	
}

void plClothingItem::Write(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Write(s, mgr);
	int i, j;

	s->WriteSafeString(fName);
	s->WriteByte(fGroup);
	s->WriteByte(fType);
	s->WriteByte(fTileset);
	s->WriteByte(fSortOrder);

	s->WriteSafeString(fCustomText);
	s->WriteSafeString(fDescription);
	s->WriteBool(fThumbnail != nil);
	if (fThumbnail != nil)
		mgr->WriteKey(s, fThumbnail->GetKey());

	UInt32 texSkip = 0;
	for (i = 0; i < fTextures.GetCount(); i++)
		if (fTextures[i] == nil)
			texSkip++;

	s->WriteSwap32(fTextures.GetCount() - texSkip);
	for (i = 0; i < fTextures.GetCount(); i++)
	{
		if (fTextures[i] == nil)
			continue;

		s->WriteSafeString(fElementNames.Get(i));

		int layerCount = 0;
		for (j = 0; j < plClothingElement::kLayerMax; j++)
		{
			// Run through once to get the count of valid layers
			if (fTextures[i][j] != nil)
				layerCount++;
		}

		s->WriteByte(layerCount);
		for (j = 0; j < plClothingElement::kLayerMax; j++)
		{
			if (fTextures[i][j] != nil)
			{
				s->WriteByte(j);
				mgr->WriteKey(s, fTextures[i][j]->GetKey());
			}
		}
		
	}
	
	for (i = 0; i < kMaxNumLODLevels; i++)
	{
		s->WriteBool(fMeshes[i] != nil);
		if (fMeshes[i] != nil)
			mgr->WriteKey(s, fMeshes[i]->GetKey());
	}

	// EXPORT ONLY
	plKey accessoryKey = nil;
	if (fAccessoryName)
	{
		char strBuf[512];
		sprintf(strBuf, "CItm_%s", fAccessoryName);
		accessoryKey = plKeyFinder::Instance().StupidSearch("GlobalClothing", nil, plClothingItem::Index(), strBuf);
		if (accessoryKey == nil)
		{
			sprintf(strBuf, "Couldn't find accessory \"%s\". It won't show at runtime.", fAccessoryName);
			hsMessageBox(strBuf, GetKeyName(), hsMessageBoxNormal);
		}
	}
	mgr->WriteKey(s, accessoryKey);
	
	for (i = 0; i < 3; i++)
	{
		s->WriteByte(fDefaultTint1[i]);
		s->WriteByte(fDefaultTint2[i]);
	}
}

hsBool plClothingItem::MsgReceive(plMessage* msg)
{
	plElementRefMsg *eMsg = plElementRefMsg::ConvertNoRef(msg);
	if (eMsg)
	{
		plMipmap *tex = plMipmap::ConvertNoRef(eMsg->GetRef());
		if (tex)
		{
			if( eMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				if (fTextures.GetCount() <= eMsg->fWhich)
					fTextures.ExpandAndZero(eMsg->fWhich + 1);
				if (fElementNames.GetCount() <= eMsg->fWhich)
					fElementNames.ExpandAndZero(eMsg->fWhich + 1);
				
				if (fElementNames.Get(eMsg->fWhich) == nil)
					fElementNames.Set(eMsg->fWhich, hsStrcpy(eMsg->fElementName));
				if (fTextures.Get(eMsg->fWhich) == nil)
				{
					plMipmap **layers = TRACKED_NEW plMipmap*[plClothingElement::kLayerMax];
					int i;
					for (i = 0; i < plClothingElement::kLayerMax; i++)
						layers[i] = nil;
					fTextures.Set(eMsg->fWhich, layers);
				}

				fTextures.Get(eMsg->fWhich)[eMsg->fLayer] = tex;
			}
			else if( eMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
			{
				fTextures.Get(eMsg->fWhich)[eMsg->fLayer] = nil;
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
					fMeshes[refMsg->fWhich] = nil;
			}
			return true;
		}

		plMipmap *thumbnail = plMipmap::ConvertNoRef(refMsg->GetRef());
		if (thumbnail)
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				fThumbnail = thumbnail;
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
				fThumbnail = nil;
			return true;
		}

		plClothingItem *accessory = plClothingItem::ConvertNoRef(refMsg->GetRef());
		if (accessory)
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				fAccessory = accessory;
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
				fAccessory = nil;
			return true;
		}		
	}

	return hsKeyedObject::MsgReceive(msg);
}

/////////////////////////////////////////////////////////////////////////////

hsBool plClosetItem::IsMatch(plClosetItem *other)
{
	return (fItem == other->fItem && fOptions.IsMatch(&other->fOptions));
}

/////////////////////////////////////////////////////////////////////////////

plClothingBase::plClothingBase() : fName(nil), fBaseTexture(nil), fLayoutName(nil) {}

plClothingBase::~plClothingBase()
{
	delete [] fName;
	delete [] fLayoutName;
}

void plClothingBase::Read(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Read(s, mgr);

	fName = s->ReadSafeString();
	if (s->ReadBool())
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef);
	fLayoutName = s->ReadSafeString();
}

void plClothingBase::Write(hsStream* s, hsResMgr* mgr)
{
	hsKeyedObject::Write(s, mgr);

	s->WriteSafeString(fName);
	s->WriteBool(fBaseTexture != nil);
	if (fBaseTexture != nil)
		mgr->WriteKey(s, fBaseTexture->GetKey());
	s->WriteSafeString(fLayoutName);
}

hsBool plClothingBase::MsgReceive(plMessage* msg)
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
			fBaseTexture = nil;
		}
		return true;
	}

	return hsKeyedObject::MsgReceive(msg);
}


/////////////////////////////////////////////////////////////////////////////

plClothingOutfit::plClothingOutfit() : 
	fTargetLayer(nil), fBase(nil), fGroup(0), fAvatar(nil), fSynchClients(false), fMaterial(nil), fVaultSaveEnabled(true), fMorphsInitDone(false)
{
	fSkinTint.Set(1.f, 0.84, 0.71, 1.f);
	fItems.Reset();
	int i;
	for (i = 0; i < plClothingLayout::kMaxTileset; i++)
		fDirtyItems.SetBit(i);

	for (i = 0; i < plClothingElement::kLayerSkinLast - plClothingElement::kLayerSkinFirst; i++)
		fSkinBlends[i] = 0.f;
}

plClothingOutfit::~plClothingOutfit()
{
	fItems.Reset();
	while (fOptions.GetCount() > 0)
		delete fOptions.Pop();
	plgDispatch::Dispatch()->UnRegisterForExactType(plPreResourceMsg::Index(), GetKey());
}

void plClothingOutfit::AddItem(plClothingItem *item, hsBool update /* = true */, hsBool broadcast /* = true */, hsBool netForce /* = false */)
{
	if (fItems.Find(item) != fItems.kMissingIndex)
		return;

	plClothingMsg *msg = TRACKED_NEW plClothingMsg();
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
	plClothingMsg *msg = TRACKED_NEW plClothingMsg();	
	msg->AddCommand(plClothingMsg::kUpdateTexture);
	if (retry)
		msg->AddCommand(plClothingMsg::kRetry);		// force a resend
	msg->Send(GetKey());
}

void plClothingOutfit::RemoveItem(plClothingItem *item, hsBool update /* = true */, hsBool netForce /* = false */)
{
	if (fItems.Find(item) == fItems.kMissingIndex)
		return;

	plClothingMsg *msg = TRACKED_NEW plClothingMsg();
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

void plClothingOutfit::TintItem(plClothingItem *item, hsScalar red, hsScalar green, hsScalar blue,
								hsBool update /* = true */, hsBool broadcast /* = true */, hsBool netForce /* = false */,
								hsBool retry /* = true */, UInt8 layer /* = kLayerTint1 */)
{
	plClothingMsg *msg = TRACKED_NEW plClothingMsg();
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

void plClothingOutfit::TintSkin(hsScalar red, hsScalar green, hsScalar blue,
								hsBool update /* = true */, hsBool broadcast /* = true */)
{
	plClothingMsg *msg = TRACKED_NEW plClothingMsg();
	msg->AddReceiver(GetKey());
	msg->AddCommand(plClothingMsg::kTintSkin);
	msg->fColor.Set(red, green, blue, 1.f);
	if (broadcast)
		msg->SetBCastFlag(plMessage::kNetPropagate);
	if (update)
		msg->AddCommand(plClothingMsg::kUpdateTexture);
	
	plgDispatch::MsgSend(msg);
}

void plClothingOutfit::MorphItem(plClothingItem *item, UInt8 layer, UInt8 delta, hsScalar weight,
								 hsBool retry /* = true */)
{
	plClothingMsg *msg = TRACKED_NEW plClothingMsg();
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

void plClothingOutfit::SetAge(hsScalar age, hsBool update /* = true */, hsBool broadcast /* = true */)
{
	SetSkinBlend(age, plClothingElement::kLayerSkinBlend1, update, broadcast);
}

void plClothingOutfit::SetSkinBlend(hsScalar blend, UInt8 layer, hsBool update /* = true */, hsBool broadcast /* = true */)
{
	plClothingMsg *msg = TRACKED_NEW plClothingMsg();
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

hsScalar plClothingOutfit::GetSkinBlend(UInt8 layer)
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

	if (fItems.Find(item) == fItems.kMissingIndex)
	{
		// Remove any other item we have that can't be worn with this
		int i;
		for (i = fItems.GetCount() - 1; i >= 0; i--)
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
	
		for (i = 0; i < fItems.GetCount(); i++)
		{
			if (item->WearBefore(fItems[i]))
				break;
		}
		fItems.Insert(i, item);
		plClothingItemOptions *op = TRACKED_NEW plClothingItemOptions;
		fOptions.Insert(i, op);
		IInstanceSharedMeshes(item);
		fDirtyItems.SetBit(item->fTileset);
		
		// A bit of hackage for bare feet sound effects.
		if (item->fType == plClothingMgr::kTypeLeftFoot)
		{
			plArmatureEffectsMgr *mgr = fAvatar->GetArmatureEffects();
			plArmatureEffectFootSound *soundEffect = nil;
			int num = mgr->GetNumEffects();
			int i;

			for (i = 0; i < num; i++)
			{
				if (soundEffect = plArmatureEffectFootSound::ConvertNoRef(mgr->GetEffect(i)))
					break;
			}
			
			if (soundEffect)
			{
				if (!strcmp(item->fName, "03_MLFoot04_01") || !strcmp(item->fName, "03_FLFoot04_01"))
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
	UInt32 index = fItems.Find(item);
	if (index != fItems.kMissingIndex)
	{
		fItems.Remove(index);
		delete fOptions.Get(index);
		fOptions.Remove(index);
		IRemoveSharedMeshes(item);
		fDirtyItems.SetBit(item->fTileset);
	}
}

hsBool plClothingOutfit::ITintItem(plClothingItem *item, hsColorRGBA color, UInt8 layer)
{
	UInt32 index = fItems.Find(item);
	if (index != fItems.kMissingIndex)
	{
		if (layer == plClothingElement::kLayerTint1)
			fOptions[index]->fTint1 = color;
		if (layer == plClothingElement::kLayerTint2)
			fOptions[index]->fTint2 = color;
		fDirtyItems.SetBit(item->fTileset);

		if (fItems[index]->fAccessory)
		{
			plClothingItem *acc = fItems[index]->fAccessory;
			UInt32 accIndex = fItems.Find(acc);
			if (accIndex != fItems.kMissingIndex)
			{
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

hsColorRGBA plClothingOutfit::GetItemTint(plClothingItem *item, UInt8 layer /* = kLayerTint1 */) const
{
	if (layer >= plClothingElement::kLayerSkinFirst &&
		layer <= plClothingElement::kLayerSkinLast)
		return fSkinTint;
	
	UInt32 index = fItems.Find(item);
	if (index != fItems.kMissingIndex)
	{
		if (layer == plClothingElement::kLayerTint1)
			return fOptions[index]->fTint1;
		if (layer == plClothingElement::kLayerTint2)
			return fOptions[index]->fTint2;
	}
	
	hsColorRGBA color;
	color.Set(1.f, 1.f, 1.f, 1.f);
	return color;
}

hsBool plClothingOutfit::IMorphItem(plClothingItem *item, UInt8 layer, UInt8 delta, hsScalar weight)
{
	UInt32 index = fItems.Find(item);
	if (index != fItems.kMissingIndex)
	{
		int i;
		for (i = 0; i < fAvatar->GetNumLOD(); i++)
		{
			if (item->fMeshes[i]->fMorphSet == nil)
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
	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // plClothingBase	

	if (fGroup != plClothingMgr::kClothingBaseNoOptions)
	{
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // target layer
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // material	
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
	int i;
	for (i = fItems.GetCount() - 1; i >= 0; i--)
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

	UInt8 lod;
	lodVar->Get(&lod);

	const plSceneObject *so = fAvatar->GetClothingSO(lod);
	if (!so)
		return;

	plMorphSequenceSDLMod *morph = const_cast<plMorphSequenceSDLMod*>(plMorphSequenceSDLMod::ConvertNoRef(so->GetModifierByType(plMorphSequenceSDLMod::Index())));
	if (morph)
		morph->SetCurrentStateFrom(sdr);
}

void plClothingOutfit::ReadFromVault()
{
	SetupMorphSDL();

	WearDefaultClothing();

	RelVaultNode * rvn;
	if (nil == (rvn = VaultGetAvatarOutfitFolderIncRef()))
		return;

	ARRAY(RelVaultNode*) nodes;
	rvn->GetChildNodesIncRef(plVault::kNodeType_SDL, 1, &nodes);	

	for (unsigned i = 0; i < nodes.Count(); ++i) {
		VaultSDLNode sdl(nodes[i]);
		if (sdl.sdlDataLen) {
			hsRAMStream ram;
			ram.Write(sdl.sdlDataLen, sdl.sdlData);
			ram.Rewind();
			
			char *	sdlRecName = nil;
			int		sdlRecVersion;
			plStateDataRecord::ReadStreamHeader(&ram, &sdlRecName, &sdlRecVersion);
			plStateDescriptor * desc = plSDLMgr::GetInstance()->FindDescriptor(sdlRecName, sdlRecVersion);
			if (desc) {
				plStateDataRecord * sdlDataRec = TRACKED_NEW plStateDataRecord(desc);
				if (sdlDataRec->Read(&ram, 0)) {
					if (!strcmp(sdlRecName, kSDLMorphSequence))
						IHandleMorphSDR(sdlDataRec);
					else
						plClothingSDLModifier::HandleSingleSDR(sdlDataRec, this);
				}
				delete sdlDataRec;				
			}
			delete [] sdlRecName;
		}
		nodes[i]->DecRef();
	}
	
	fSynchClients = true; // set true if the next synch should be bcast
	ForceUpdate(true);
	
	rvn->DecRef();
}

void plClothingOutfit::SaveCustomizations(hsBool retry /* = true */)
{
	plClothingMsg *msg = TRACKED_NEW plClothingMsg();
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

	RelVaultNode * rvn;
	if (nil == (rvn = VaultGetAvatarOutfitFolderIncRef()))
		return;

	ARRAY(plStateDataRecord*) SDRs;
	
	plStateDataRecord clothingSDR(kSDLClothing);
	fAvatar->GetClothingSDLMod()->PutCurrentStateIn(&clothingSDR);
	plSDStateVariable * clothesStateDesc = clothingSDR.FindSDVar(plClothingSDLModifier::kStrWardrobe);

	for (unsigned i = 0; i < clothesStateDesc->GetCount(); ++i)
		SDRs.Add(clothesStateDesc->GetStateDataRecord(i));

	plSDStateVariable * appearanceStateDesc = clothingSDR.FindSDVar(plClothingSDLModifier::kStrAppearance); // for skin tint
	SDRs.Add(appearanceStateDesc->GetStateDataRecord(0));
	
	WriteToVault(SDRs);
	rvn->DecRef();
}

void plClothingOutfit::WriteToVault(const ARRAY(plStateDataRecord*) & SDRs)
{
	// We'll hit this case when the server asks us to save state for NPCs.
	if (fAvatar->GetTarget(0) != plNetClientApp::GetInstance()->GetLocalPlayer())
		return;

	RelVaultNode * rvn;
	if (nil == (rvn = VaultGetAvatarOutfitFolderIncRef()))
		return;
		
	ARRAY(plStateDataRecord*)	morphs;

	// Gather morph SDRs	
	hsTArray<const plMorphSequence*> morphsSDRs;
	plMorphSequence::FindMorphMods(fAvatar->GetTarget(0), morphsSDRs);
	for (unsigned i = 0; i < morphsSDRs.GetCount(); ++i) {
		for (unsigned j = 0; j < fAvatar->GetNumLOD(); j++) {
			if (fAvatar->GetClothingSO(j) == morphsSDRs[i]->GetTarget(0)) {
				plStateDataRecord * morphSDR = TRACKED_NEW plStateDataRecord(kSDLMorphSequence);
				plSimpleStateVariable * lodVar = morphSDR->FindVar(plMorphSequenceSDLMod::kStrTargetID);
				if (lodVar)
					lodVar->Set((int)j);

				morphsSDRs[i]->GetSDLMod()->PutCurrentStateIn(morphSDR);
				morphs.Add(morphSDR);
			}
		}
	}
	
	ARRAY(RelVaultNode*) templates;
	ARRAY(RelVaultNode*) actuals;
	ARRAY(RelVaultNode*) nodes;

	// Get all existing clothing SDRs
	rvn->GetChildNodesIncRef(plVault::kNodeType_SDL, 1, &nodes);	// REF: Find

	const ARRAY(plStateDataRecord*) * arrs[] = {
		&SDRs,
		&morphs,
	};
	for (unsigned arrIdx = 0; arrIdx < arrsize(arrs); ++arrIdx) {
		const ARRAY(plStateDataRecord*) * arr = arrs[arrIdx];
		
		// Write all SDL to to the outfit folder, reusing existing nodes and creating new ones as necessary
		for (unsigned i = 0; i < arr->Count(); ++i) {
			RelVaultNode * node;
			if (nodes.Count()) {
				node = nodes[0];
				nodes.DeleteUnordered(0);
				node->IncRef();	// REF: Work
				node->DecRef();	// REF: Find
			}
			else {
				RelVaultNode * templateNode = NEWZERO(RelVaultNode);
				templateNode->SetNodeType(plVault::kNodeType_SDL);
				templates.Add(templateNode);
				node = templateNode;
				node->IncRef();	// REF: Create
				node->IncRef();	// REF: Work
			}

			VaultSDLNode sdl(node);
			sdl.SetStateDataRecord((*arr)[i], 0);
			node->DecRef();		// REF: Work
		}
	}
		
	// Delete any leftover nodes
	{ for (unsigned i = 0; i < nodes.Count(); ++i) {
		VaultDeleteNode(nodes[i]->nodeId);
		nodes[i]->DecRef();	// REF: Array
	}}
	
	// Create actual new nodes from their templates
	{ for (unsigned i = 0; i < templates.Count(); ++i) {
		ENetError result;
		if (RelVaultNode * actual = VaultCreateNodeAndWaitIncRef(templates[i], &result)) {
			actuals.Add(actual);
		}
		templates[i]->DecRef();	// REF: Create
	}}

	// Add new nodes to outfit folder
	{ for (unsigned i = 0; i < actuals.Count(); ++i) {
		VaultAddChildNodeAndWait(rvn->nodeId, actuals[i]->nodeId, NetCommGetPlayer()->playerInt);
		actuals[i]->DecRef();	// REF: Create
	}}

	// Cleanup morph SDRs
	{for (unsigned i = 0; i < morphs.Count(); ++i) {
		DEL(morphs[i]);
	}}
	
	rvn->DecRef();
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
		plClothingUpdateBCMsg *BCMsg = TRACKED_NEW plClothingUpdateBCMsg();
		BCMsg->SetSender(GetKey());
		plgDispatch::MsgSend(BCMsg);
	}
}

void plClothingOutfit::WearDefaultClothing()
{
	StripAccessories();
	
	plClothingMgr *cMgr = plClothingMgr::GetClothingMgr();
	hsTArray<plClothingItem *>items;
	cMgr->GetItemsByGroup(fGroup, items);

	// Wear one thing of each type
	UInt32 i, j;
	for (i = 0; i < plClothingMgr::kMaxType; i++)
	{
		if (i == plClothingMgr::kTypeAccessory)
			continue;

		for (j = 0; j < items.GetCount(); j++)
		{
			if (items[j]->fType == i)
			{
				AddItem(items[j], false, false);
				if (i == plClothingMgr::kTypeHair || i == plClothingMgr::kTypeFace)
				{	
					// Hair tint color
					TintItem(items[j], 0.5, 0.3, 0.2, false, false);
				}
				else
				{
					TintItem(items[j], items[j]->fDefaultTint1[0] / 255.f, items[j]->fDefaultTint1[1] / 255.f,
							 items[j]->fDefaultTint1[2] / 255.f, false, false);
				}

				// Everyone can tint layer 2. Go nuts!
				TintItem(items[j], items[j]->fDefaultTint2[0] / 255.f, items[j]->fDefaultTint2[1] / 255.f,
						 items[j]->fDefaultTint2[2] / 255.f, false, false, false, true, plClothingElement::kLayerTint2);					
				break;
			}
		}
	}
}

void plClothingOutfit::WearDefaultClothingType(UInt32 clothingType)
{
	plClothingMgr *cMgr = plClothingMgr::GetClothingMgr();
	hsTArray<plClothingItem *> items;
	cMgr->GetItemsByGroup(fGroup, items);

	UInt32 i;
	for (i=0; i<items.GetCount(); i++)
	{
		if (items[i]->fType == clothingType)
		{
			AddItem(items[i], false, false);
			if (clothingType == plClothingMgr::kTypeHair || clothingType == plClothingMgr::kTypeFace)
			{
				// Hair tint color
				TintItem(items[i], 0.5, 0.3, 0.2, false, false);
			}
			else
			{
				TintItem(items[i], items[i]->fDefaultTint1[0] / 255.f, items[i]->fDefaultTint1[1] / 255.f,
						 items[i]->fDefaultTint1[2] / 255.f, false, false);
			}

			// Everyone can tint layer 2. Go nuts!
			TintItem(items[i], items[i]->fDefaultTint2[0] / 255.f, items[i]->fDefaultTint2[1] / 255.f,
					 items[i]->fDefaultTint2[2] / 255.f, false, false, false, true, plClothingElement::kLayerTint2);
			break;
		}
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
	ReadFromVault();

	fVaultSaveEnabled = true;
}

static plRandom sRandom;

void plClothingOutfit::WearRandomOutfit()
{
	plClothingMgr *cMgr = plClothingMgr::GetClothingMgr();
	hsTArray<plClothingItem *>items;

	// Wear one thing of each type
	UInt32 i, j;
	for (i = 0; i < plClothingMgr::kMaxType; i++)
	{
		if (i == plClothingMgr::kTypeAccessory)
			continue;

		items.Reset();
		cMgr->GetItemsByGroupAndType(fGroup, (UInt8)i, items);
		j = (UInt32)(sRandom.RandZeroToOne() * items.GetCount());

		hsScalar r1 = sRandom.RandZeroToOne();
		hsScalar g1 = sRandom.RandZeroToOne();
		hsScalar b1 = sRandom.RandZeroToOne();
		hsScalar r2 = sRandom.RandZeroToOne();
		hsScalar g2 = sRandom.RandZeroToOne();
		hsScalar b2 = sRandom.RandZeroToOne();

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

hsBool plClothingOutfit::ReadItems(hsStream* s, hsResMgr* mgr, hsBool broadcast /* = true */)
{
	hsBool result = true;
	UInt32 numItems = s->ReadSwap32();
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
		if( key == nil )
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
	s->WriteSwap32(fItems.GetCount());
	int i;
	for (i = 0; i < fItems.GetCount(); i++)
	{
		mgr->WriteKey(s, fItems.Get(i)->GetKey());
		fOptions.Get(i)->fTint1.Write(s);
		fOptions.Get(i)->fTint2.Write(s);
	}
}

hsBool plClothingOutfit::MsgReceive(plMessage* msg)
{
	plPreResourceMsg *preMsg = plPreResourceMsg::ConvertNoRef(msg);
	if (preMsg)
	{
		if (fAvatar && fGroup != plClothingMgr::kClothingBaseNoOptions)
		{
			plDrawable *spans = fAvatar->FindDrawable();
			const hsBounds3Ext &bnds = spans->GetWorldBounds();
			if (bnds.GetType() == kBoundsNormal)
			{
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
				if (preMsg->Pipeline()->TestVisibleWorld(spans->GetSpaceTree()->GetWorldBounds()))
					preMsg->Pipeline()->SubmitClothingOutfit(this);				
			}
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
				fTargetLayer = nil;
			
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
				fBase = nil;
			
			return true;		
		}
		
		hsGMaterial *mat = hsGMaterial::ConvertNoRef(refMsg->GetRef());
		if (mat)
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				fMaterial = mat;
			else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
				fMaterial = nil;
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
			plGenRefMsg *msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
			hsgResMgr::ResMgr()->AddViaNotify(cMsg->fItemKey, msg, plRefFlags::kActiveRef);
			plClothingItem *accessory = plClothingItem::ConvertNoRef(cMsg->fItemKey->GetObjectPtr())->fAccessory;
			if (accessory)
			{
				plGenRefMsg *msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1);
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
			int i, j;
			for (i = 0; i < fItems.GetCount(); i++)
				for (j = 0; j < fItems[i]->fElements.GetCount(); j++)
					if (fItems[i]->fTextures[j][plClothingElement::kLayerSkin] != nil)
						fDirtyItems.SetBit(fItems[i]->fTileset);
		}

		if (cMsg->GetCommand(plClothingMsg::kBlendSkin))
		{
			hsScalar blend = cMsg->fColor.a;
			if (blend > 1.f)
				blend = 1.f;
			if (blend < 0.f)
				blend = 0.f;
			if (cMsg->fLayer >= plClothingElement::kLayerSkinBlend1 &&
				cMsg->fLayer <= plClothingElement::kLayerSkinBlend6)
			{
				fSkinBlends[cMsg->fLayer - plClothingElement::kLayerSkinBlend1] = blend;
			
				int i, j;
				for (i = 0; i < fItems.GetCount(); i++)
					for (j = 0; j < fItems[i]->fElements.GetCount(); j++)
						if (fItems[i]->fTextures[j][cMsg->fLayer] != nil)
							fDirtyItems.SetBit(fItems[i]->fTileset);
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
				plClothingMsg *update = TRACKED_NEW plClothingMsg();
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
hsBool plClothingOutfit::DirtySynchState(const char* SDLStateName, UInt32 synchFlags)
{
	plSynchEnabler ps(true);	// make sure synching is enabled, since this happens during load
	synchFlags |= plSynchedObject::kForceFullSend;	// TEMP
	if (fSynchClients)
		synchFlags |= plSynchedObject::kBCastToClients;
	fSynchClients=false;
	hsAssert(fAvatar, "nil fAvatar");
	return fAvatar->GetTarget(0)->DirtySynchState(SDLStateName, synchFlags);
}

// Note: Currently the word "instance" is a lie. We just copy. In the future
// we'll be good about this, but I wanted to get it working first.
void plClothingOutfit::IInstanceSharedMeshes(plClothingItem *item)
{
	if (fAvatar)
		fAvatar->ValidateMesh();

	hsBool partialSort = item->fCustomText && strstr(item->fCustomText, "NeedsSort");
	int i;
	for (i = 0; i < plClothingItem::kMaxNumLODLevels; i++)
	{
		const plSceneObject *so = fAvatar->GetClothingSO(i);
		if (so != nil && item->fMeshes[i] != nil)
		{
			plInstanceDrawInterface *idi = const_cast<plInstanceDrawInterface*>(plInstanceDrawInterface::ConvertNoRef(so->GetDrawInterface()));
			if (idi)
				idi->AddSharedMesh(item->fMeshes[i], fMaterial, !item->HasBaseAlpha(), i, partialSort);
		}
	}
}

void plClothingOutfit::IRemoveSharedMeshes(plClothingItem *item)
{	
	if (fAvatar == nil)
		return;

	int i;
	for (i = 0; i < plClothingItem::kMaxNumLODLevels; i++)
	{
		const plSceneObject *so = fAvatar->GetClothingSO(i);
		if (so != nil && item->fMeshes[i] != nil)
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
		hsTArray<const plMorphSequence*> morphs;
		plMorphSequence::FindMorphMods(fAvatar->GetTarget(0), morphs);
		for (unsigned i = 0; i < morphs.GetCount(); ++i)
		{
			for (unsigned j = 0; j < fAvatar->GetNumLOD(); j++)
			{
				if (fAvatar->GetClothingSO(j) == morphs[i]->GetTarget(0))
				{
					plMorphSequenceSDLMod* morph = morphs[i]->GetSDLMod();
					if (morph)
						morph->SetIsAvatar(true);
				}
			}
		}

		fMorphsInitDone = true;
	}
}


/////////////////////////////////////////////////////////////////////////////

const char *plClothingMgr::GroupStrings[] = 
{
	"Male Clothing",
	"Female Clothing",
	"(No Clothing Options)"
};

const char *plClothingMgr::TypeStrings[] =
{
	"Pants",
	"Shirt",
	"LeftHand",
	"RightHand",
	"Face",
	"Hair",
	"LeftFoot",
	"RightFoot",
	"Accessory"
};

plClothingMgr *plClothingMgr::fInstance = nil;

plClothingMgr::plClothingMgr()
{
	fLayouts.Reset();
	fItems.Reset();
}

plClothingMgr::~plClothingMgr()
{
	while (fElements.GetCount() > 0)
		delete fElements.Pop();
	while (fLayouts.GetCount() > 0)
		delete fLayouts.Pop();
	while (fItems.GetCount() > 0)
		delete fItems.Pop();
}

plClothingLayout *plClothingMgr::GetLayout(char *name)
{
	int i;
	for (i = 0; i < fLayouts.GetCount(); i++)
	{
		if (!strcmp(fLayouts.Get(i)->fName, name))
			return fLayouts.Get(i);
	}
	return nil;
}

plClothingElement *plClothingMgr::FindElementByName(char *name)
{
	int i;
	for (i = 0; i < fElements.GetCount(); i++)
	{
		if (!strcmp(fElements.Get(i)->fName, name))
			return fElements.Get(i);
	}
	return nil;	
}

void plClothingMgr::AddItemsToCloset(hsTArray<plClosetItem> &items)
{
	RelVaultNode * rvn = VaultGetAvatarClosetFolderIncRef();
	if (!rvn)
		return;
		
	hsTArray<plClosetItem> closet;
	GetClosetItems(closet);
	
	ARRAY(RelVaultNode*)	templates;
	
	for (unsigned i = 0; i < items.GetCount(); ++i) {
		bool match = false;
		for (unsigned j = 0; j < closet.GetCount(); ++j) {
			if (closet[j].IsMatch(&items[i]))
			{
				match = true;
				break;
			}
		}
		
		if (match)
			continue;

		plStateDataRecord rec(plClothingSDLModifier::GetClothingItemSDRName());
		plClothingSDLModifier::PutSingleItemIntoSDR(&items[i], &rec);
		
		RelVaultNode * templateNode = NEWZERO(RelVaultNode);
		templateNode->IncRef();
		templateNode->SetNodeType(plVault::kNodeType_SDL);
		
		VaultSDLNode sdl(templateNode);
		sdl.SetStateDataRecord(&rec);

		templates.Add(templateNode);
	}
	
	for (unsigned i = 0; i < templates.Count(); ++i) {
		ENetError result;
		if (RelVaultNode * actual = VaultCreateNodeAndWaitIncRef(templates[i], &result)) {
			VaultAddChildNodeAndWait(
				rvn->nodeId,
				actual->nodeId,
				NetCommGetPlayer()->playerInt
			);
			actual->DecRef(); // REF: Create
		}
		templates[i]->DecRef(); // REF: Create
	}
	
	rvn->DecRef();
}

void plClothingMgr::GetClosetItems(hsTArray<plClosetItem> &out)
{
	RelVaultNode * rvn = VaultGetAvatarClosetFolderIncRef();
	if (!rvn)
		return;

	ARRAY(RelVaultNode*)	nodes;
	rvn->GetChildNodesIncRef(plVault::kNodeType_SDL, 1, &nodes);
	out.SetCount(nodes.Count());
	
	for (unsigned i = 0; i < nodes.Count(); ++i) {
		VaultSDLNode sdl(nodes[i]);
		plStateDataRecord * rec = NEWZERO(plStateDataRecord);
		if (sdl.GetStateDataRecord(rec, 0))
			plClothingSDLModifier::HandleSingleSDR(rec, nil, &out[i]);
		DEL(rec);
	}

	if (out.GetCount()) {
		for (int i = out.GetCount() - 1; i >= 0; i--) {
			if (out[i].fItem == nil)
				out.Remove(i);
		}
	}
	
	rvn->DecRef();
}	

void plClothingMgr::GetAllWithSameMesh(plClothingItem *item, hsTArray<plClothingItem*> &out)
{
	int i;
	for (i = 0; i < fItems.GetCount(); i++)
	{
		if (item->HasSameMeshes(fItems[i]))
			out.Append(fItems[i]);
	}
}

// Yes, it's a lame n^2 function. Show me that we have enough items for it
// to matter and I'll speed it up.
void plClothingMgr::FilterUniqueMeshes(hsTArray<plClothingItem*> &items)
{
	int i, j;
	for (i = items.GetCount() - 1; i >= 1; i--)
	{
		for (j = i - 1; j >= 0; j--)
		{
			if (items[i]->HasSameMeshes(items[j]))
			{
				items.Remove(i);
				break;
			}
		}
	}
}

plClothingItem *plClothingMgr::FindItemByName(const char *name)
{
	if (!name)
		return nil;
	
	int i;
	for (i = 0; i < fItems.GetCount(); i++)
	{
		plClothingItem* item = fItems.Get(i);
		if (!strcmp(item->fName, name))
			return item;
	}
	return nil;
}

void plClothingMgr::GetItemsByGroup(UInt8 group, hsTArray<plClothingItem*> &out)
{
	int i;
	for (i = 0; i < fItems.GetCount(); i++)
	{
		if (fItems.Get(i)->fGroup == group)
			out.Append(fItems.Get(i));
	}
}

void plClothingMgr::GetItemsByGroupAndType(UInt8 group, UInt8 type, hsTArray<plClothingItem*> &out)
{
	int i;
	for (i = 0; i < fItems.GetCount(); i++)
	{
		if (fItems.Get(i)->fGroup == group && fItems.Get(i)->fType == type)
			out.Append(fItems.Get(i));
	}
}

plClothingItem *plClothingMgr::GetLRMatch(plClothingItem *item)
{
	int i;
	for (i = 0; i < fItems.GetCount(); i++)
	{
		if (IsLRMatch(item, fItems[i]))
			return fItems[i];
	}

	// Couldn't find one.
	return nil;
}

hsBool plClothingMgr::IsLRMatch(plClothingItem *item1, plClothingItem *item2)
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
	if (item1->fTextures.GetCount() != item2->fTextures.GetCount()) return false;

	int i, j;
	for (i = 0; i < item1->fTextures.GetCount(); i++)
	{
		for (j = 0; j < plClothingElement::kLayerMax; j++)
			if (item1->fTextures[i][j] != item2->fTextures[i][j])
				return false;
	}

	// Finally... we're not our own match
	return item1 != item2;
}

void plClothingMgr::Init()
{
	fInstance = TRACKED_NEW plClothingMgr;
	fInstance->RegisterAs(kClothingMgr_KEY);
	fInstance->IInit();
}

void plClothingMgr::IInit()
{
	plClothingElement::GetElements(fElements);
	plClothingLayout *layout = TRACKED_NEW plClothingLayout("BasicHuman", 1024);
	layout->fElements.Append(FindElementByName("shirt-chest"));
	layout->fElements.Append(FindElementByName("shirt-sleeve"));
	layout->fElements.Append(FindElementByName("face"));
	layout->fElements.Append(FindElementByName("eyeball"));
	layout->fElements.Append(FindElementByName("shoe-top"));
	layout->fElements.Append(FindElementByName("shoe-bottom"));
	layout->fElements.Append(FindElementByName("pants"));
	layout->fElements.Append(FindElementByName("hand-LOD"));
	layout->fElements.Append(FindElementByName("hand-square"));
	layout->fElements.Append(FindElementByName("hand-wide"));
	
	fLayouts.Append(layout);
}

void plClothingMgr::DeInit()
{
	if (fInstance)
	{
		fInstance->UnRegisterAs(kClothingMgr_KEY);
		fInstance = nil;
	}
}	

hsBool plClothingMgr::MsgReceive(plMessage* msg)
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
				fItems.RemoveItem(item);
			}
			return true;
		}
	}

	return hsKeyedObject::MsgReceive(msg);
}

void plClothingMgr::IAddItem(plClothingItem *item)
{
	hsBool allFound = true;
	int i, j;
	for (i = 0; i < item->fElementNames.GetCount(); i++)
	{
		for (j = 0; j < fElements.GetCount(); j++)
		{	
			if (!strcmp(item->fElementNames.Get(i), fElements.Get(j)->fName))
			{
				item->fElements.Set(i, fElements.Get(j));
				break;
			}
		}
		if (j >= fElements.GetCount())
		{
			allFound = false;
			break;
		}
	}

	if (allFound)
	{
		for (i = 0; i < fItems.GetCount(); i++)
		{
			if (fItems[i]->fSortOrder >= item->fSortOrder)
				break;
		}
		fItems.InsertAtIndex(i, item);
	}
	else
		hsAssert(false, "Couldn't match all elements of added clothing item.");
}

void plClothingMgr::ChangeAvatar(char *name)
{
	plAvatarMgr::GetInstance()->UnLoadLocalPlayer();
	plAvatarMgr::GetInstance()->LoadPlayer(name, nil);
}

