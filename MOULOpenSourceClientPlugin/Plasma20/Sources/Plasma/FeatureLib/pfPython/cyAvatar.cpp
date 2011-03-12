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
#include "cyAvatar.h"


#include "plgDispatch.h"
#include "../plAvatar/plAvatarMgr.h"

#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plOneShotCallbacks.h"
#include "../plMessage/plOneShotMsg.h"
#include "../plMessage/plMultistageMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../plGImage/plMipmap.h"
#include "pyKey.h"
#include "pySceneObject.h"
#include "pyColor.h"
#include "pyImage.h"
#include "cyPythonInterface.h"
#include "cyMisc.h"

#include "../plAvatar/plOneShotMod.h"
#include "../plAvatar/plMultistageBehMod.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plClothingLayout.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvBrainHuman.h"		// needed to call the emote
#include "../plAvatar/plAGAnim.h"			// to get the BodyUsage enum
#include "../plInputCore/plAvatarInputInterface.h"
#include "plPhysical.h"
#include "../plMessage/plSimStateMsg.h"

#include "../pnNetCommon/plNetApp.h"
#include "../plVault/plVault.h"

#include "../plDrawable/plSharedMesh.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plDrawable/plMorphSequence.h"


///////////////////////////////////////////////////////////////////////////
//
// LOCAL FORWARD DECLs
//
///////////////////////////////////////////////////////////////////////////
bool IEnterGenericMode(const char *enterAnim, const char *idleAnim, const char *exitAnim, bool autoExit, plAGAnim::BodyUsage bodyUsage,
					   plAvBrainGeneric::BrainType = plAvBrainGeneric::kGeneric);
bool IExitTopmostGenericMode();


cyAvatar::cyAvatar(plKey sender, plKey recvr)
{
	SetSender(sender);
	AddRecvr(recvr);
	fNetForce = false;
}

// setters
void cyAvatar::SetSender(plKey &sender)
{
	fSender = sender;
}

void cyAvatar::AddRecvr(plKey &recvr)
{
	if ( recvr != nil )
		fRecvr.Append(recvr);
}

void cyAvatar::SetNetForce(hsBool state)
{
	// set our flag
	fNetForce = state;
}

void cyAvatar::SetSenderKey(pyKey& pKey)
{
	SetSender(pKey.getKey());
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IFindArmatureModKey
//  PARAMETERS : avObj  - avatar sceneobject
//
//  PURPOSE    : find the armature mod for this sceneoabject (if its an avatar)
//
const plArmatureMod* cyAvatar::IFindArmatureMod(plKey avKey)
{
	plSceneObject* avObj = plSceneObject::ConvertNoRef(avKey->ObjectIsLoaded());
	if ( avObj )
	{
		// search through its modifiers to see if one of them is an avatar modifier
		int i;
		for ( i=0; i<avObj->GetNumModifiers(); i++ )
		{
			const plModifier* mod = avObj->GetModifier(i);
			// see if it is an avatar mod base class
			const plArmatureMod* avmod = plArmatureMod::ConvertNoRef(mod);
			if ( avmod )
				return avmod;
		}
	}
	// otherwise we didn't find anything
	return nil;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IFindArmatureModKey
//  PARAMETERS : avObj  - avatar sceneobject
//
//  PURPOSE    : find the armature mod for this sceneoabject (if its an avatar)
//
plKey cyAvatar::IFindArmatureModKey(plKey avKey)
{
	const plArmatureMod* avatar = IFindArmatureMod(avKey);
	if ( avatar )
		return avatar->GetKey();
	// otherwise we didn't find anything
	return nil;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : oneShot
//  PARAMETERS : 
//
//  PURPOSE    : oneShot Avatar (must already be there)
//
void cyAvatar::OneShot(pyKey &seekKey, float duration, hsBool usePhysics,
			   const char *animName, hsBool drivable, hsBool reversible)
{
	if ( fRecvr.Count() > 0 )
	{
		// create message
		plAvOneShotMsg* pMsg = TRACKED_NEW plAvOneShotMsg(
			(plKey )fSender,
			nil,
			seekKey.getKey(),	// Mark D told me to do it ...paulg
			duration,  
			usePhysics,  
			animName, // Constructor will do a copy. -mf- hsStrcpy(animName),
			drivable, 
			reversible);

		// check if this needs to be network forced to all clients
		if (fNetForce )
		{
			// set the network propagate flag to make sure it gets to the other clients
			pMsg->SetBCastFlag(plMessage::kNetPropagate);
			pMsg->SetBCastFlag(plMessage::kNetForce);
		}
		// must have a receiver!
		pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		// add all our receivers to the message receiver list
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RunBehavior
//  PARAMETERS : 
//
//  PURPOSE    : Run Behavior, could be single or multi-stage shot
//
void cyAvatar::RunBehavior(pyKey &behKey, hsBool netForce, hsBool netProp)
{
	// first there is someone to send to and make sure that we an avatar to send this to
	if ( behKey.getKey() != nil && fRecvr.Count() > 0)
	{
		// must determine if the behKey is pointing to Single or Multi Shot behavior
		if ( plOneShotMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nil )
		{
			// create a message OneShotMessage
			plOneShotMsg* pMsg = TRACKED_NEW plOneShotMsg;
			// check if this needs to be network forced to all clients
			if (netProp)
			{
				pMsg->SetBCastFlag(plMessage::kNetPropagate);
			}
			else
			{
				pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
			}

			if (netForce)
			{
				// set the network propagate flag to make sure it gets to the other clients
				pMsg->SetBCastFlag(plMessage::kNetPropagate);
				pMsg->SetBCastFlag(plMessage::kNetForce);
			}
			else
			{
				pMsg->SetBCastFlag(plMessage::kNetForce, false);
			}

			pMsg->SetSender(fSender);
			pMsg->AddReceiver(behKey.getKey());
			int i;
			for ( i=0; i<fRecvr.Count(); i++ )
			{
				// make sure there is an avatar to set
				if ( fRecvr[i] != nil )
				{
					pMsg->fPlayerKey = (plKey)fRecvr[i];
					plgDispatch::MsgSend( pMsg );	// send off command for each valid avatar we find
													// ... really, should only be one... though
				}
			}
		}
		// else if it is a Multistage guy
		else if ( plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nil )
		{
			// its a multistage thingy... need to send it a plNotifyMsg
			// create new notify message to do the actual send with
			plNotifyMsg* pNMsg = TRACKED_NEW plNotifyMsg;

			// set whether this should be forced over the network (ignoring net-cascading)
			if (netProp)
			{
				pNMsg->SetBCastFlag(plMessage::kNetPropagate);
			}
			else
			{
				pNMsg->SetBCastFlag(plMessage::kNetPropagate, false);
			}

			if ( netForce )
			{
				pNMsg->SetBCastFlag(plMessage::kNetPropagate);
				pNMsg->SetBCastFlag(plMessage::kNetForce);
			}
			else
			{
				pNMsg->SetBCastFlag(plMessage::kNetForce, false);
			}

			// copy data and event records to new NotifyMsg
			pNMsg->fState = 1.0;
			// need to recreate all the events in the new message by Adding them
			if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
			{
				pNMsg->AddPickEvent( (plKey)fRecvr[0], nil, true, hsPoint3(0,0,0) );
			}

			// add receivers
			// loop though adding the ones that want to be notified of the change
			pNMsg->AddReceiver(behKey.getKey());
			pNMsg->SetSender(fSender);
			plgDispatch::MsgSend( pNMsg );
		}

	}
}
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RunBehaviorAndReply
//  PARAMETERS : 
//
//  PURPOSE    : Run Behavior, multistage only, reply to specified key'd object
//
void cyAvatar::RunBehaviorAndReply(pyKey& behKey, pyKey& replyKey, hsBool netForce, hsBool netProp)
{
	plMultistageBehMod* pMod = plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr());	
	if ( pMod )
	{
		// its a multistage thingy... need to send it a plNotifyMsg
		// create new notify message to do the actual send with
		plNotifyMsg* pNMsg = TRACKED_NEW plNotifyMsg;

		// set whether this should be forced over the network (ignoring net-cascading)
		if (netProp)
		{
			pNMsg->SetBCastFlag(plMessage::kNetPropagate);
		}
		else
		{
			pNMsg->SetBCastFlag(plMessage::kNetPropagate, false);
		}

		if (netForce)
		{
			// set the network propagate flag to make sure it gets to the other clients
			pNMsg->SetBCastFlag(plMessage::kNetPropagate);
			pNMsg->SetBCastFlag(plMessage::kNetForce);
		}
		else
		{
			pNMsg->SetBCastFlag(plMessage::kNetForce, false);
		}

		// copy data and event records to new NotifyMsg
		pNMsg->fState = 1.0;
		// need to recreate all the events in the new message by Adding them
		if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
		{
			pNMsg->AddPickEvent( (plKey)fRecvr[0], nil, true, hsPoint3(0,0,0) );
		}

		// add receivers
		// loop though adding the ones that want to be notified of the change
		pNMsg->AddReceiver(behKey.getKey());
		pNMsg->SetSender(replyKey.getKey());
		plgDispatch::MsgSend( pNMsg );
	}

}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : NextStage
//  PARAMETERS : behKey  - behavior pyKey
//             : transTime  - the transition time to the next stage
//   (optional): rewind   - whether to rewind to the front of the next stage
//
//  PURPOSE    : Go to the next stage in a multi-stage behavior
//
// NOTE: only works with multi-stage behaviors
//
void cyAvatar::NextStage(pyKey &behKey, hsScalar transTime, hsBool setTime, hsScalar newTime,
						hsBool setDirection, bool isForward, hsBool netForce)
{
	// first there is someone to send to and make sure that we an avatar to send this to
	if ( behKey.getKey() != nil && fRecvr.Count() > 0)
	{
		// if it is a Multistage guy
		if ( plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nil )
		{
			plKey avKey = IFindArmatureModKey( (plKey)fRecvr[0] );
			if ( avKey )
			{
				// create the message
				plAvBrainGenericMsg* pMsg = TRACKED_NEW plAvBrainGenericMsg((plKey)fSender, avKey,
					plAvBrainGenericMsg::kNextStage, 0, setTime, newTime,
					setDirection, (bool)isForward, transTime);

				if ( netForce )
					pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

				plgDispatch::MsgSend( pMsg );
			}
		}

	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PreviousStage
//  PARAMETERS : behKey  - behavior pyKey
//             : transTime  - the transition time to the next stage
//   (optional): rewind   - whether to rewind to the front of the next stage
//
//  PURPOSE    : Go to the previous stage in a multi-stage behavior
//
// NOTE: only works with multi-stage behaviors
//
void cyAvatar::PreviousStage(pyKey &behKey, hsScalar transTime, hsBool setTime, hsScalar newTime,
						hsBool setDirection, bool isForward, hsBool netForce)
{
	// first there is someone to send to and make sure that we an avatar to send this to
	if ( behKey.getKey() != nil && fRecvr.Count() > 0)
	{
		// if it is a Multistage guy
		if ( plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nil )
		{
			plKey avKey = IFindArmatureModKey( (plKey)fRecvr[0] );
			if ( avKey )
			{
				// create the message
				plAvBrainGenericMsg* pMsg = TRACKED_NEW plAvBrainGenericMsg((plKey)fSender, avKey,
					plAvBrainGenericMsg::kPrevStage, 0, setTime, newTime,
					setDirection, (bool)isForward, transTime);

				if ( netForce )
					pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

				plgDispatch::MsgSend( pMsg );
			}
		}

	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GotoStage
//  PARAMETERS : behKey  - behavior pyKey
//             : stage   - stage number to go to
//             : transTime  - the transition time to the next stage
//   (optional): rewind   - whether to rewind to the front of the next stage
//
//  PURPOSE    : Go to a particular stage in a multi-stage behavior
//
// NOTE: only works with multi-stage behaviors
//
void cyAvatar::GoToStage(pyKey &behKey, Int32 stage, hsScalar transTime, hsBool setTime, hsScalar newTime,
						hsBool setDirection, bool isForward, hsBool netForce)
{
	// first there is someone to send to and make sure that we an avatar to send this to
	if ( behKey.getKey() != nil && fRecvr.Count() > 0)
	{
		// if it is a Multistage guy
		if ( plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nil )
		{
			plKey avKey = IFindArmatureModKey( (plKey)fRecvr[0] );
			if ( avKey )
			{
				// create the message
				plAvBrainGenericMsg* pMsg = TRACKED_NEW plAvBrainGenericMsg((plKey)fSender, avKey,
					plAvBrainGenericMsg::kGotoStage, stage, setTime, newTime,
					setDirection, isForward, transTime);

				if ( netForce )
					pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

				plgDispatch::MsgSend( pMsg );
			}
		}

	}
}


void cyAvatar::SetLoopCount(pyKey &behKey, Int32 stage, Int32 loopCount, hsBool netForce)
{
	// if it is a Multistage guy
	if ( plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nil )
	{
		plMultistageModMsg* pMsg = TRACKED_NEW plMultistageModMsg((plKey)nil, behKey.getKey());
		pMsg->SetCommand(plMultistageModMsg::kSetLoopCount);
		pMsg->fStageNum = (UInt8)stage;
		pMsg->fNumLoops = (UInt8)loopCount;

		if ( netForce )
			pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

		plgDispatch::MsgSend( pMsg );
	}
} 


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : seek
//  PARAMETERS : 
//
//  PURPOSE    : seek Avatar (must already be there)
//

/* Unsupported. Ask Bob if you want it back.

void cyAvatar::Seek(pyKey &seekKey, float duration, hsBool usePhysics)
{
	// must have a receiver!
	if ( fRecvr.Count() > 0 )
	{
		// create message
		plAvSeekMsg* pMsg = TRACKED_NEW plAvSeekMsg(
			(plKey)fSender,nil, seekKey.getKey(),duration,usePhysics);

		// check if this needs to be network forced to all clients
		if (fNetForce )
		{
			// set the network propagate flag to make sure it gets to the other clients
			pMsg->SetBCastFlag(plMessage::kNetPropagate);
			pMsg->SetBCastFlag(plMessage::kNetForce);
		}
		pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		// add all our receivers to the message receiver list
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}
*/

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetAvatarClothingGroup
//  PARAMETERS : 
//
//  PURPOSE    : Return what clothing group the avatar is in
//
Int32 cyAvatar::GetAvatarClothingGroup()
{
	// find the avatar's armature modifier
	const plArmatureMod *avMod = nil;

	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			if ( avMod )
			{
				return avMod->GetClothingOutfit()->fGroup;
			}
		}
	}
	return -1;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClosetClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
//
std::vector<std::string> cyAvatar::GetEntireClothingList(Int32 clothing_type)
{
	// Currently, just all the clothing available will be returned
	hsTArray<plClothingItem*> clothingList = plClothingMgr::GetClothingMgr()->GetItemList();
	int numItems = clothingList.GetCount();

	// create the string list to send to python...
	std::vector<std::string> retVal;
	for (int i = 0; i < numItems; i++)
		retVal.push_back(clothingList[i]->GetName());

	return retVal;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClosetClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
//
std::vector<PyObject*> cyAvatar::GetClosetClothingList(Int32 clothing_type)
{
	std::vector<PyObject*> retVal;

	// find the avatar's armature modifier
	const plArmatureMod *avMod = nil;

	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			if ( avMod )
			{
				// Get all the clothes that we can wear
				hsTArray<plClothingItem*> clothingList;
				plClothingMgr::GetClothingMgr()->GetItemsByGroup(avMod->GetClothingOutfit()->fGroup, clothingList);
				int numItems = clothingList.GetCount();
				// create the string list to send to python... as a python object
				int i;
				for ( i=0; i<numItems; i++ )
				{
					plClothingItem* item = clothingList[i];
					if ( clothing_type == -1 || item->fType == clothing_type )
					{
						// add this event record to the main event list (lists within a list)
						// create list
						PyObject* clothingItem = PyList_New(5);

						// [0] = clothing name
						PyList_SetItem(clothingItem, 0, PyString_FromString(item->GetName()));
						
						// [1] = clothing type
						PyList_SetItem(clothingItem, 1, PyInt_FromLong(item->fType));

						// [2] = description
						const char* description = "";		// assume an empty string
						if ( item->fDescription != nil )
							description = item->fDescription;
						PyList_SetItem(clothingItem, 2, PyString_FromString(description));

						// [3] = ptImage of icon
						if ( item->fThumbnail != nil )
							// create a ptImage
							PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
						else
							PyList_SetItem(clothingItem, 3, PyInt_FromLong(0));

						// [4] = fCustomText
						const char* custom = "";			// assume an empty string
						if ( item->fCustomText != nil )
							custom = item->fCustomText;
						PyList_SetItem(clothingItem, 4, PyString_FromString(custom));
						
						retVal.push_back(clothingItem);
					}
				}
			}
		}
	}
	return retVal;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetAvatarClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
//
std::vector<PyObject*> cyAvatar::GetAvatarClothingList()
{
	std::vector<PyObject*> retVal;
	// find the avatar's armature modifier
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			if ( avMod )
			{
				// Currently, just all the clothing available will be returned
				hsTArray<plClothingItem*> clothingList = avMod->GetClothingOutfit()->GetItemList();
				int numItems = clothingList.GetCount();
				// create the string list to send to python... as a python object
				int i;
				for ( i=0; i<numItems; i++ )
				{
					// add this event record to the main event list (lists within a list)
					// create list
					PyObject* clothingItem = PyList_New(5);
					plClothingItem* item = clothingList[i];

					// [0] = clothing name
					PyList_SetItem(clothingItem, 0, PyString_FromString(item->GetName()));

					// [1] = clothing type
					PyList_SetItem(clothingItem, 1, PyInt_FromLong(item->fType));

					// [2] = description
					const char* description = "";		// assume an empty string
					if ( item->fDescription != nil )
						description = item->fDescription;
					PyList_SetItem(clothingItem, 2, PyString_FromString(description));

					// [3] = ptImage of icon
					if ( item->fThumbnail != nil )
						// create a ptImage
						PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
					else
						PyList_SetItem(clothingItem, 3, PyInt_FromLong(0));

					// [4] = fCustomText
					const char* custom = "";			// assume an empty string
					if ( item->fCustomText != nil )
						custom = item->fCustomText;
					PyList_SetItem(clothingItem, 4, PyString_FromString(custom));

					retVal.push_back(clothingItem);
				}
			}
		}
	}
	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetWardrobeClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of items that are in the avatars closet
//
std::vector<PyObject*> cyAvatar::GetWardrobeClothingList()
{
	std::vector<PyObject*> retVal;
	hsTArray<plClosetItem> closetList;
	plClothingMgr::GetClothingMgr()->GetClosetItems(closetList);
	int numItems = closetList.GetCount();
	// create the string list to send to python... as a python object
	int i;
	for ( i=0; i<numItems; i++ )
	{
		// add this event record to the main event list (lists within a list)
		// create list
		PyObject* closetItem = PyList_New(7);

		// [0] = clothing name
		PyList_SetItem(closetItem, 0, PyString_FromString(closetList[i].fItem->GetName()));
		
		// [1] = clothing type
		PyList_SetItem(closetItem, 1, PyInt_FromLong(closetList[i].fItem->fType));
		
		// [2] = description
		const char* description = "";		// assume an empty string
		if ( closetList[i].fItem->fDescription != nil )
			description = closetList[i].fItem->fDescription;
		PyList_SetItem(closetItem, 2, PyString_FromString(description));

		// [3] = ptImage of icon
		if ( closetList[i].fItem->fThumbnail != nil )
			// create a ptImage
			PyList_SetItem(closetItem, 3, pyImage::New(closetList[i].fItem->fThumbnail->GetKey()));
		else
			PyList_SetItem(closetItem, 3, PyInt_FromLong(0));

		// [4] = fCustomText
		const char* custom = "";			// assume an empty string
		if ( closetList[i].fItem->fCustomText != nil )
			custom = closetList[i].fItem->fCustomText;
		PyList_SetItem(closetItem, 4, PyString_FromString(custom));

		// [5] = fTint1
		PyList_SetItem(closetItem, 5, pyColor::New(closetList[i].fOptions.fTint1));
		
		// [6] = fTint2
		PyList_SetItem(closetItem, 6, pyColor::New(closetList[i].fOptions.fTint2));

		retVal.push_back(closetItem);
	}
	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddWardrobeClothingItem
//  PARAMETERS : clothing_name - the name of the clothing item to add to your wardrobe
//             : tint1 - layer one color
//             : tint2 - layer two color
//
//  PURPOSE    : To add a clothing item to the avatar's wardrobe (closet)
//
void cyAvatar::AddWardrobeClothingItem(const char* clothing_name,pyColor& tint1,pyColor& tint2)
{
	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name);
	if ( item )
	{
		hsTArray<plClosetItem> items;
		items.SetCount(1);
		items[0].fItem = item;
		items[0].fOptions.fTint1.Set(tint1.getRed(), tint1.getGreen(), tint1.getBlue(), 1.f);
		items[0].fOptions.fTint2.Set(tint2.getRed(), tint2.getGreen(), tint2.getBlue(), 1.f);

		plClothingMgr::GetClothingMgr()->AddItemsToCloset(items);
	}
}	


/////////////////////////////////////////////////////////////////////////////
//
//	Function   : GetUniqueMeshList
//	PARAMETERS : clothing_type - the type of clothing to get
//
//	PURPOSE    : Return a list of unique clothing items (each has a different mesh)
//			   : that belong to the specific type
//
std::vector<PyObject*> cyAvatar::GetUniqueMeshList(Int32 clothing_type)
{
	std::vector<PyObject*> retVal;

	// find the avatar's armature modifier
	const plArmatureMod *avMod = nil;

	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			if ( avMod )
			{
				// Get all the clothes that we can wear
				hsTArray<plClothingItem*> clothingList;
				plClothingMgr::GetClothingMgr()->GetItemsByGroup(avMod->GetClothingOutfit()->fGroup, clothingList);
				plClothingMgr::GetClothingMgr()->FilterUniqueMeshes(clothingList); // filter all redundant meshes
				int numItems = clothingList.GetCount();
				// create the string list to send to python... as a python object
				int i;
				for ( i=0; i<numItems; i++ )
				{
					plClothingItem* item = clothingList[i];
					if ( clothing_type == -1 || item->fType == clothing_type )
					{
						// add this event record to the main event list (lists within a list)
						// create list
						PyObject* clothingItem = PyList_New(5);

						// [0] = clothing name
						PyList_SetItem(clothingItem, 0, PyString_FromString(item->GetName()));

						// [1] = clothing type
						PyList_SetItem(clothingItem, 1, PyInt_FromLong(item->fType));

						// [2] = description
						const char* description = "";		// assume an empty string
						if ( item->fDescription != nil )
							description = item->fDescription;
						PyList_SetItem(clothingItem, 2, PyString_FromString(description));

						// [3] = ptImage of icon
						if ( item->fThumbnail != nil )
							// create a ptImage
							PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
						else
							PyList_SetItem(clothingItem, 3, PyInt_FromLong(0));

						// [4] = fCustomText
						const char* custom = "";			// assume an empty string
						if ( item->fCustomText != nil )
							custom = item->fCustomText;
						PyList_SetItem(clothingItem, 4, PyString_FromString(custom));

						retVal.push_back(clothingItem);
					}
				}
			}
		}
	}
	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//	Function   : GetAllWithSameMesh
//	PARAMETERS : clothing_name - the name of the mesh to get the textures of
//
//	PURPOSE	   : Return a list of clothing items that have the same mesh as
//			   : the item passed in
//
std::vector<PyObject*> cyAvatar::GetAllWithSameMesh(const char* clothing_name)
{
	std::vector<PyObject*> retVal;

	// find the avatar's armature modifier
	const plArmatureMod *avMod = nil;

	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			if ( avMod )
			{
				// Get all clothes with the same mesh as the one passed in
				hsTArray<plClothingItem*> clothingList;
				plClothingMgr::GetClothingMgr()->GetAllWithSameMesh(plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name), clothingList);
				int numItems = clothingList.GetCount();
				// create the string list to send to python... as a python object
				int i;
				for ( i=0; i<numItems; i++ )
				{
					// add this event record to the main event list (lists within a list)
					// create list
					PyObject* clothingItem = PyList_New(5);
					plClothingItem* item = clothingList[i];

					// [0] = clothing name
					PyList_SetItem(clothingItem, 0, PyString_FromString(item->GetName()));

					// [1] = clothing type
					PyList_SetItem(clothingItem, 1, PyInt_FromLong(item->fType));

					// [2] = description
					const char* description = "";		// assume an empty string
					if ( item->fDescription != nil )
						description = item->fDescription;
					PyList_SetItem(clothingItem, 2, PyString_FromString(description));

					// [3] = ptImage of icon
					if ( item->fThumbnail != nil )
						// create a ptImage
						PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
					else
						PyList_SetItem(clothingItem, 3, PyInt_FromLong(0));

					// [4] = fCustomText
					const char* custom = "";			// assume an empty string
					if ( item->fCustomText != nil )
						custom = item->fCustomText;
					PyList_SetItem(clothingItem, 4, PyString_FromString(custom));

					retVal.push_back(clothingItem);
				}
			}
		}
	}
	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetMatchingClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Return the clothing item that matches this one
//             : If no match then returns the number 0
//
PyObject* cyAvatar::GetMatchingClothingItem(const char* clothing_name)
{
	// Get all the clothes that we can wear
	hsTArray<plClothingItem*> clothingList;
	plClothingItem* match = plClothingMgr::GetClothingMgr()->GetLRMatch(plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name));
	if ( match )
	{
		// create list
		PyObject* clothingItem = PyList_New(5);
		
		// [0] = clothing name
		PyList_SetItem(clothingItem, 0, PyString_FromString(match->GetName()));

		// [1] = clothing type
		PyList_SetItem(clothingItem, 1, PyInt_FromLong(match->fType));

		// [2] = description
		const char* description = "";		// assume an empty string
		if ( match->fDescription != nil )
			description = match->fDescription;
		PyList_SetItem(clothingItem, 2, PyString_FromString(description));

		// [3] = ptImage of icon
		if ( match->fThumbnail != nil )
			// create a ptImage
			PyList_SetItem(clothingItem, 3, pyImage::New(match->fThumbnail->GetKey()));
		else
			PyList_SetItem(clothingItem, 3, PyInt_FromLong(0));

		// [4] = fCustomText
		const char* custom = "";			// assume an empty string
		if ( match->fCustomText != nil )
			custom = match->fCustomText;
		PyList_SetItem(clothingItem, 4, PyString_FromString(custom));

		return clothingItem;
	}
	else
		return PyInt_FromLong(0);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : WearClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns 0, if clothing item was not found
//
hsBool cyAvatar::WearClothingItem(const char* clothing_name)
{
	return WearClothingItemU(clothing_name,true);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RemoveClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns false, if clothing item was not found
//
hsBool cyAvatar::RemoveClothingItem(const char* clothing_name)
{
	return RemoveClothingItemU(clothing_name,true);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
hsBool cyAvatar::TintClothingItem(const char* clothing_name, pyColor& tint)
{
	return TintClothingItemU(clothing_name,tint,true);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItemLayer
//  PARAMETERS : clothing_name   - name of the clothing item to change the color of
//             : tint   - what color to change it to
//             : layer  - which layer to change (1 or 2)
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
hsBool cyAvatar::TintClothingItemLayer(const char* clothing_name, pyColor& tint, UInt8 layer)
{
	return TintClothingItemLayerU(clothing_name,tint,layer,true);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : WearClothingItem
//  PARAMETERS : --- with update flag
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns 0, if clothing item was not found
//
hsBool cyAvatar::WearClothingItemU(const char* clothing_name, hsBool update)
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name);

			if (avMod && item)
			{
				if ( fNetForce )
					avMod->GetClothingOutfit()->AddItem(item, update, true, true);
				else
					avMod->GetClothingOutfit()->AddItem(item, update);
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RemoveClothingItemU
//  PARAMETERS : --- with update flag
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns false, if clothing item was not found
//
hsBool cyAvatar::RemoveClothingItemU(const char* clothing_name, hsBool update)
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

			plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name);

			if (avMod && item)
			{
				if ( fNetForce )
					avMod->GetClothingOutfit()->RemoveItem(item,update,true);
				else
					avMod->GetClothingOutfit()->RemoveItem(item,update);
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItemU
//  PARAMETERS : --- with update flag
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
hsBool cyAvatar::TintClothingItemU(const char* clothing_name, pyColor& tint, hsBool update)
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

			plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name);

			if (avMod && item)
			{
				avMod->GetClothingOutfit()->TintItem(item, tint.getRed(),tint.getGreen(),tint.getBlue(),update,true,fNetForce,true,plClothingElement::kLayerTint1);
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItemLayer
//  PARAMETERS : clothing_name   - name of the clothing item to change the color of
//             : tint   - what color to change it to
//             : layer  - which layer to change (1 or 2)
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
hsBool cyAvatar::TintClothingItemLayerU(const char* clothing_name, pyColor& tint, UInt8 layer, hsBool update)
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

			plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name);

			if (avMod && item)
			{
				// Convert Python layer number to clothing code...
				if (layer == 2)
					layer = plClothingElement::kLayerTint2;
				else 
					layer = plClothingElement::kLayerTint1;
				avMod->GetClothingOutfit()->TintItem(item, tint.getRed(),tint.getGreen(),tint.getBlue(),update,true,fNetForce,true,layer);
				return true;
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClothingItemParameterString
//  PARAMETERS : 
//
//  PURPOSE    : Get the custom parameter string for a clothing item
//
const char* cyAvatar::GetClothingItemParameterString(const char* clothing_name)
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

			plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name);

			if (avMod && item)
			{
				if ( item->fCustomText != nil )
					return item->fCustomText;
				else
					return "";
			}
		}
	}

	return "";
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetTintClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
//
PyObject* cyAvatar::GetTintClothingItem(const char* clothing_name)
{
	return GetTintClothingItemL(clothing_name,1);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetTintClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
//
PyObject* cyAvatar::GetTintClothingItemL(const char* clothing_name, UInt8 layer)
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

			plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName((char*)clothing_name);

			if (avMod && item)
			{
				// Convert Python layer number to clothing code...
				if (layer == 2)
					layer = plClothingElement::kLayerTint2;
				else 
					layer = plClothingElement::kLayerTint1;
				hsColorRGBA tint = avMod->GetClothingOutfit()->GetItemTint(item,layer);
				return pyColor::New(tint);
			}
		}
	}

	char errmsg[256];
	sprintf(errmsg,"Cannot find clothing item %d to find out what tint it is",clothing_name);
	PyErr_SetString(PyExc_KeyError, errmsg);
	// returning nil means an error occurred
	return nil;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintSkin
//  PARAMETERS : 
//
//  PURPOSE    : Tint the skin of the player's avatar
//
void cyAvatar::TintSkin(pyColor& tint)
{
	TintSkinU(tint,true);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintSkinU
//  PARAMETERS : 
//
//  PURPOSE    : Tint the skin of the player's avatar with optional update flag
//
void cyAvatar::TintSkinU(pyColor& tint, hsBool update)
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			avMod->GetClothingOutfit()->TintSkin(tint.getRed(),tint.getGreen(),tint.getBlue(),update,true);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetTintSkin
//  PARAMETERS : 
//
//  PURPOSE    : Get the tint of the skin of the player's avatar
//
PyObject* cyAvatar::GetTintSkin()
{
	const plArmatureMod *avMod = nil;
	// we can really only talk to one avatar, so just get the first one (which is probably the only one)
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			hsColorRGBA tint = avMod->GetClothingOutfit()->fSkinTint;
			// now create the ptColor Python object
			return pyColor::New(tint);
		}
	}
	
	char errmsg[256];
	sprintf(errmsg,"Cannot find the skin of the player. Whatever that means!");
	PyErr_SetString(PyExc_KeyError, errmsg);
	// returning nil means an error occurred
	return nil;
}

plMorphSequence* cyAvatar::LocalMorphSequence()
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if (!avMod)
		return nil;
	

	const plSceneObject *so = avMod->GetClothingSO(0); // grabbing the high LOD node
	if (!so)
		return nil;

	const plModifier* constSeq = nil;
	int i;
	for (i = 0; i < so->GetNumModifiers(); i++)
	{
		constSeq = so->GetModifier(i);
		if (constSeq && plMorphSequence::ConvertNoRef(constSeq))
		{
			return (plMorphSequence*)constSeq; // safe cast, we've already checked type (plus we're const_cast'ing).
		}
	}

	return nil;
}

/////////////////////////////////////////////////////////////////////////////
//
//	Function   : SetMorph
//	PARAMETERS : clothing_name - the name of the clothing to morph
//			   : layer - the layer to affect
//			   : value - what the new value should be (clipped between -1 and 1)
//
//	PURPOSE	   : Set the morph value of a specific layer of clothing
//
void cyAvatar::SetMorph(const char* clothing_name, UInt8 layer, float value)
{
	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);
	if( !item )
	{
		PyErr_SetString(PyExc_KeyError, "Item not found");
		return;
	}

	float wgtPlus;
	float wgtMinus;
	
	if(value > 1.0) value = 1.0;
	if(value < -1.0) value = -1.0;
	
	if (value > 0)
	{
		wgtPlus = value;
		wgtMinus = 0;
	}
	else
	{
		wgtMinus = -value;
		wgtPlus = 0;
	}
	
	if ( fRecvr.Count() > 0 && fRecvr[0] != nil )
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
		if (so != nil)
		{
			const plArmatureMod *avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
			if (avMod && avMod->GetClothingOutfit())
			{
				avMod->GetClothingOutfit()->MorphItem(item, layer, 0, wgtPlus, true);
				avMod->GetClothingOutfit()->MorphItem(item, layer, 1, wgtMinus, true);
			}
		}
	}
			

}

/////////////////////////////////////////////////////////////////////////////
//
//	Function   : GetMorph
//	PARAMETERS : clothing_name - the name of the clothing to get the value from
//			   : layer - the layer to get the value from
//
//	PURPOSE    : Returns the current morph value of the specific layer of clothing
//
float cyAvatar::GetMorph(const char* clothing_name, UInt8 layer)
{
	plMorphSequence* seq = LocalMorphSequence();
	if( !seq )
	{
		PyErr_SetString(PyExc_KeyError, "Sequence not found");
		return 0;
	}

	plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);
	if( !item )
	{
		PyErr_SetString(PyExc_KeyError, "Item not found");
		return 0;
	}

	plKey meshKey = item->fMeshes[0]->GetKey();

	if (layer >= seq->GetNumLayers(meshKey))
	{
		PyErr_SetString(PyExc_KeyError, "Layer index too high");
		return 0;
	}	

	float wgtPlus;
	float wgtMinus;

	wgtPlus = seq->GetWeight(layer,0,meshKey);
	wgtMinus = seq->GetWeight(layer,1,meshKey);

	if (wgtPlus > 0)
		return wgtPlus;
	else
		return -wgtMinus;
}

/////////////////////////////////////////////////////////////////////////////
//
//	Function   : SetSkinBlend
//	PARAMETERS : layer - the layer to affect
//			   : value - what the new value should be (clipped between 0 and 1)
//
//	PURPOSE	   : Set the skin blend for the specified layer
//
void cyAvatar::SetSkinBlend(UInt8 layer, float value)
{
	if (value < 0.0) value = 0.0;
	if (value > 1.0) value = 1.0;

	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avMod)
	{
		avMod->GetClothingOutfit()->SetSkinBlend(value, (int)layer + plClothingElement::kLayerSkinBlend1 - 1);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//	Function   : GetSkinBlend
//	PARAMETERS : layer - the layer to get the blend for
//
//	PURPOSE	   : Returns the current layer's skin blend
//
float cyAvatar::GetSkinBlend(UInt8 layer)
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avMod)
	{
		return avMod->GetClothingOutfit()->GetSkinBlend((int)layer + plClothingElement::kLayerSkinBlend1 - 1);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SaveClothing
//  PARAMETERS :
//
//  PURPOSE	   : Saves the current clothing to the vault (including morphs)
//
void cyAvatar::SaveClothing()
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avMod)
		avMod->GetClothingOutfit()->SaveCustomizations();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterSubWorld
//  PARAMETERS : object  - a sceneobject that is in the subworld
//
//  PURPOSE    : Place the Avatar into the subworld of the sceneobject specified
//
void cyAvatar::EnterSubWorld(pySceneObject& object)
{
	// make sure that there is atleast one avatar scene object attached (should be)
	if ( fRecvr.Count() > 0)
	{
		// find the armature modifier
		plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
		if(avatar)
		{
			// get the sceneobject that we will use to find the subworld
			plKey SOkey = object.getObjKey();
			if ( SOkey )
			{
				plSceneObject *SO = plSceneObject::ConvertNoRef(SOkey->ObjectIsLoaded());
				if(SO)
				{
					plKey subWorldKey = SOkey;
					plKey physKey = avatar->GetKey();
					plKey nilKey;	// sorry
					plSubWorldMsg *swMsg = TRACKED_NEW plSubWorldMsg(nilKey, physKey, subWorldKey);
					swMsg->Send();
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitSubWorld
//  PARAMETERS : (none)
//
//  PURPOSE    : Exit the avatar from the subworld, back into the ... <whatever> world
//
void cyAvatar::ExitSubWorld()
{
	// make sure that there is atleast one avatar scene object attached (should be)
	if ( fRecvr.Count() > 0)
	{
		// find the armature modifier
		plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
		if(avatar)
		{
			plKey subWorldKey;		// we're going to the nil subworld
			plKey physKey = avatar->GetKey();
			plKey nilKey;	// sorry
			plSubWorldMsg *swMsg = TRACKED_NEW plSubWorldMsg(nilKey, physKey, subWorldKey);
			swMsg->Send();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PlaySimpleAnimation
//  PARAMETERS : object  - a sceneobject that is in the subworld
//
//  PURPOSE    : Place the Avatar into the subworld of the sceneobject specified
//
void cyAvatar::PlaySimpleAnimation(const char* animName)
{
	// make sure that there is atleast one avatar scene object attached (should be)
	if ( fRecvr.Count() > 0)
	{
		// find the armature modifier
		plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
		if(avatar)
		{
			avatar->PlaySimpleAnim(animName);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ChangeAvatar
//  PARAMETERS : gender name  - is a string of the name of the gender to go to
//
//  PURPOSE    : Change the local avatar's gender.
//
//  Valid genders:
//    Male
//    Female
//
void cyAvatar::ChangeAvatar(const char* genderName)
{
#ifndef PLASMA_EXTERNAL_RELEASE
	plClothingMgr::ChangeAvatar((char*)genderName);
	
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, genderName, arrsize(wStr));
	
	RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef();
	if (rvnPlr) {
		VaultPlayerNode plr(rvnPlr);
		plr.SetAvatarShapeName(wStr);
		rvnPlr->DecRef();
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ChangePlayerName
//  PARAMETERS : name  - is a string of the new name for the player
//
//  PURPOSE    : Change the local player's avatar name
//
void cyAvatar::ChangePlayerName(const char* playerName)
{
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, playerName, arrsize(wStr));
	
	RelVaultNode * rvnPlr = VaultGetPlayerNodeIncRef();
	if (rvnPlr) {
		VaultPlayerNode plr(rvnPlr);
		plr.SetPlayerName(wStr);
		rvnPlr->DecRef();
	} 
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Emote
//  PARAMETERS : emoteName - name of the emote to play on the avatar
//
//  PURPOSE    : plays an emote on a the local avatar (net propagated)
//
bool cyAvatar::Emote(const char* emoteName)
{
	// can we find an emote of this name?
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	return AvatarEmote(avatar, emoteName);
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Sit
//  PARAMETERS : none
//
//  PURPOSE    : Makes the avatar sit down on the ground where they are.
//				 The avatar will automatically stand when the user tries to move.
//
bool cyAvatar::Sit()
{
	return IEnterGenericMode("SitDownGround", "SitIdleGround", "SitStandGround", true, plAGAnim::kBodyLower, plAvBrainGeneric::kSitOnGround);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterKiMode
//  PARAMETERS : none
//
//  PURPOSE    : Makes the avatar appear to be using the ki.
//
bool cyAvatar::EnterKiMode()
{
	return IEnterGenericMode("KiBegin", "KiUse", "KiEnd", false, plAGAnim::kBodyFull);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitKiMode
//  PARAMETERS : none
//
//  PURPOSE    : Makes the avatar stop appearing to use the ki.
//				 May cause problems if EnterKiMode() was not called earlier.
//
bool cyAvatar::ExitKiMode()
{
	return IExitTopmostGenericMode();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterAFKMode
//  PARAMETERS : none
//
//  PURPOSE    : Tell the avatar to enter the AFK mode (sitting, head down)
//
bool cyAvatar::EnterAFKMode()
{
	return IEnterGenericMode("AFKEnter", "AFKIdle", "AFKExit", true, plAGAnim::kBodyFull, plAvBrainGeneric::kAFK);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitAFKMode
//  PARAMETERS : none
//
//  PURPOSE    : Tell the avatar to exit the AFK mode
//				 May cause problems if EnterKiMode() was not called earlier.
//
bool cyAvatar::ExitAFKMode()
{
	return IExitTopmostGenericMode();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterPBMode
//  PARAMETERS : none
//
//  PURPOSE    : Enter the personal book mode...stay until further notice.
//
bool cyAvatar::EnterPBMode()
{
	return IEnterGenericMode("PersonalBookEnter", "PersonalBookIdle", "PersonalBookExit", false, plAGAnim::kBodyFull);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitPBMode
//  PARAMETERS : none
//
//  PURPOSE    : Leave the personal book mode. Currently leaves any mode; will become
//			   : more specific in future version
//
bool cyAvatar::ExitPBMode()
{
	return IExitTopmostGenericMode();
}


int cyAvatar::GetCurrentMode()
{
	// make sure that there is atleast one avatar scene object attached (should be)
	if ( fRecvr.Count() > 0)
	{
		// find the armature modifier
		plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
		if(avatar)
		{
			return avatar->GetCurrentGenericType();
		}
	}
	return plAvBrainGeneric::kNonGeneric;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : disable movement controls
//  PARAMETERS : 
//
//  PURPOSE    : something tells me python shouldn't do this this way
//


void cyAvatar::DisableMovementControls()
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if (avatar)
	{	
		if (!avatar->IsInputSuspended())
			avatar->SuspendInput();
	}
}

void cyAvatar::EnableMovementControls()
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();	
	if (avatar)
	{
		if (avatar->IsInputSuspended())
			avatar->ResumeInput();
	}
}

void cyAvatar::DisableMouseMovement()
{
	plAvatarInputInterface::GetInstance()->SuspendMouseMovement();
}

void cyAvatar::EnableMouseMovement()
{
	plAvatarInputInterface::GetInstance()->EnableMouseMovement();
}

void cyAvatar::EnableAvatarJump()
{
	plAvatarInputInterface::GetInstance()->EnableJump(true);
}

void cyAvatar::DisableAvatarJump()
{
	plAvatarInputInterface::GetInstance()->EnableJump(false);
}

void cyAvatar::EnableForwardMovement()
{
	plAvatarInputInterface::GetInstance()->EnableForwardMovement(true);
}

void cyAvatar::DisableForwardMovement()
{
	plAvatarInputInterface::GetInstance()->EnableForwardMovement(false);
}

bool cyAvatar::LocalAvatarRunKeyDown()
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if (avMod)
		return avMod->FastKeyDown();
	return false;
}

bool cyAvatar::LocalAvatarIsMoving()
{
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if (avMod)
		return avMod->ForwardKeyDown() || avMod->BackwardKeyDown() || avMod->StrafeRightKeyDown() ||
			avMod->StrafeLeftKeyDown() || avMod->TurnRightKeyDown() || avMod->TurnLeftKeyDown() ||
			avMod->JumpKeyDown();
	return false;
}

void cyAvatar::SetMouseTurnSensitivity(hsScalar val)
{
	plArmatureMod::SetMouseTurnSensitivity(val);
}

hsScalar cyAvatar::GetMouseTurnSensitivity()
{
	return plArmatureMod::GetMouseTurnSensitivity();
}

void cyAvatar::SpawnNext()
{
	static int whichSpawn = 0;
	plAvatarMgr *mgr = plAvatarMgr::GetInstance();
	int max = mgr->NumSpawnPoints();

	whichSpawn = ++whichSpawn < max ? whichSpawn : 0;
	
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if(avatar)
	{
		double fakeTime = 0.0f;
		avatar->SpawnAt(whichSpawn, fakeTime);
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RegisterForBehaviorNotify()
//  PARAMETERS : none
//
//  PURPOSE    : To register for notifies from the avatar for any kind of behavior notify
//
void cyAvatar::RegisterForBehaviorNotify(pyKey &selfKey)
{
	// make sure that there is atleast one avatar scene object attached (should be)
	if ( fRecvr.Count() > 0)
	{
		// find the armature modifier
		plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
		if(avatar)
		{
			avatar->RegisterForBehaviorNotify(selfKey.getKey());
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : UnRegisterForBehaviorNotify()
//  PARAMETERS : none
//
//  PURPOSE    : To remove the registeration for notifies from the avatar
//
void cyAvatar::UnRegisterForBehaviorNotify(pyKey &selfKey)
{
	// make sure that there is atleast one avatar scene object attached (should be)
	if ( fRecvr.Count() > 0)
	{
		// find the armature modifier
		plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
		if(avatar)
		{
			avatar->UnRegisterForBehaviorNotify(selfKey.getKey());
		}
	}
}






/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IEnterGenericMode
//  PARAMETERS : none
//
//  PURPOSE    : Three-stage multistage animations (sit down, sit, get up) are really common.
//			   : This does the basic setup.
//
bool IEnterGenericMode(const char *enterAnim, const char *idleAnim, const char *exitAnim, bool autoExit, plAGAnim::BodyUsage bodyUsage, 
					   plAvBrainGeneric::BrainType type /* = kGeneric */)
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	return PushSimpleMultiStage(avatar, enterAnim, idleAnim, exitAnim, true, autoExit, bodyUsage, type);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IExitTopmostGenericMode
//  PARAMETERS : none
//
//  PURPOSE    : Exits whatever multistage animation you're in. We currently don't discriminate;
//			   : that will be added later.
//
bool IExitTopmostGenericMode()
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

	plAvBrainGenericMsg* pMsg = TRACKED_NEW plAvBrainGenericMsg(nil, avatar->GetKey(),
		plAvBrainGenericMsg::kGotoStage, 2, false, 0.0,
		false, false, 0.0);

	pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

	plgDispatch::MsgSend( pMsg );
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IsCurrentBrainHuman
//  PARAMETERS : none
//
//  PURPOSE    : Returns whether the top most brain is a human brain
//
hsBool cyAvatar::IsCurrentBrainHuman()
{
	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if (avatar)
	{
		plArmatureBrain *brain = avatar->GetCurrentBrain();
		plAvBrainHuman *human = plAvBrainHuman::ConvertNoRef(brain);
		if (human)
			return true;
	}
	return false;
}
