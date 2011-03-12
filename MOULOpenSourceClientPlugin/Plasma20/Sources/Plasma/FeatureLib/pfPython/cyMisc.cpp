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
#include "cyMisc.h"

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "../plResMgr/plKeyFinder.h"

#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plConsoleMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plMessage/plExcludeRegionMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../pnMessage/plCmdIfaceModMsg.h"
#include "../pnMessage/plAttachMsg.h"
#include "../plMessage/plTimerCallbackMsg.h"
#include "../plMessage/plNetVoiceListMsg.h"
#include "../pnMessage/plClientMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnTimer/plTimerCallbackManager.h"
#include "../plVault/plVault.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plNetTransport/plNetTransport.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plMultistageBehMod.h"
#include "../plAvatar/plAvBrainCritter.h"
#include "pyCritterBrain.h"
#include "cyPythonInterface.h"
#include "pyKey.h"
#include "pySceneObject.h"
#include "pyPlayer.h"
#include "pyImage.h"
#include "pyDniCoordinates.h"
#include "pyDniInfoSource.h"
#include "pyColor.h"
#include "pyNetLinkingMgr.h"
#include "pyAgeInfoStruct.h"
#include "pyAgeLinkStruct.h"
#include "pyAlarm.h"
#include "../pfMessage/pfKIMsg.h"
#include "../plNetMessage/plNetMessage.h"
#include "../pfCamera/plVirtualCamNeu.h"
#include "../plPipeline/plDynamicEnvMap.h"

#include "../pfGameGUIMgr/pfGameGUIMgr.h"
#include "../pfGameGUIMgr/pfGUIDialogMod.h"
#include "pyGUIDialog.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#include "../plMessage/plCCRMsg.h"
#include "../plAgeLoader/plAgeLoader.h"

#include "../plResMgr/plLocalization.h"
#include "../plGLight/plLightInfo.h"

#include "../plInputCore/plAvatarInputInterface.h"
#include "../plInputCore/plInputDevice.h"

#include "../plVault/plAgeInfoSource.h"

#include "../pfLocalizationMgr/pfLocalizationMgr.h"

//// Static Class Stuff //////////////////////////////////////////////////////
plPipeline* cyMisc::fPipeline = nil;
UInt32	cyMisc::fUniqueNumber = 0;

#ifdef PLASMA_EXTERNAL_RELEASE
UInt32	cyMisc::fPythonLoggingLevel = cyMisc::kErrorLevel;
#else
UInt32	cyMisc::fPythonLoggingLevel = cyMisc::kWarningLevel;
#endif

/////////////////////////////////////////////////////////////////////////////
// static
void cyMisc::Update( double secs )
{
	// only update once per 1/2 sec
	static double lastUpdateTime = 0.0;
	if ( secs-lastUpdateTime>=0.5 )
	{
		lastUpdateTime = secs;
		pyAlarmMgr::GetInstance()->Update( secs );
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Get/SetDebugPrintLevel
//
//  PURPOSE    : gets and sets the python debug print level
//
UInt32 cyMisc::GetPythonLoggingLevel()
{
	return fPythonLoggingLevel;
}
void cyMisc::SetPythonLoggingLevel(UInt32 new_level)
{
	fPythonLoggingLevel = new_level;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Console
//  PARAMETERS : command   - string of console commmand to execute
//
//  PURPOSE    : Execute a console command from a python script
//
void cyMisc::Console(const char* command)
{
	if ( command != nil )
	{
		// create message to send to the console
		plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
		pMsg->SetBCastFlag(plMessage::kBCastByType);
		pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
		pMsg->SetControlActivated(true);
		pMsg->SetCmdString(command);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void cyMisc::ConsoleNet(const char* command, hsBool netForce)
{
	if ( command != nil )
	{
		// create message to send to the console
		plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
		pMsg->SetBCastFlag(plMessage::kBCastByType);
		pMsg->SetBCastFlag(plMessage::kNetPropagate);
		if ( netForce )
			pMsg->SetBCastFlag(plMessage::kNetForce);
		pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
		pMsg->SetControlActivated(true);
		pMsg->SetCmdString(command);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : FindSceneObject
//  PARAMETERS : name   - string of name of the sceneobject
//             : ageName - string of the name of the age to look in
//
//  PURPOSE    : Execute a console command from a python script,
//					optionally propagate over the net
//
PyObject* cyMisc::FindSceneObject(const char* name, const char* ageName)
{
	// assume that we won't find the sceneobject (key is equal to nil)
	plKey key=nil;

	if ( name || name[0] != 0)
	{
		const char* theAge = ageName;
		if ( ageName[0] == 0 )
			theAge = nil;
		key=plKeyFinder::Instance().StupidSearch(theAge,nil,plSceneObject::Index(), name, false);
	}

	if ( key == nil )
	{
		char errmsg[256];
		sprintf(errmsg,"Sceneobject %s not found",name);
		PyErr_SetString(PyExc_NameError, errmsg);
		return nil; // return nil cause we errored
	}
	return pySceneObject::New(key);
}

PyObject* cyMisc::FindActivator(const char* name)
{
	plKey key = nil;
	if (name && strlen(name) > 0)
	{
		std::vector<plKey> keylist;
		plKeyFinder::Instance().ReallyStupidActivatorSearch(name, keylist);

		if (keylist.size() == 1)
			key = keylist[0];
	}

	if (key == nil)
	{
		PYTHON_RETURN_NONE;
	}
	else
		return pyKey::New(key);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PopUpConsole
//  PARAMETERS : command   - string of console commmand to execute
//
//  PURPOSE    : Execute a console command from a python script
//
void cyMisc::PopUpConsole(const char* command)
{
	if ( command != nil )
	{
		// create message to send to the console
		plControlEventMsg* pMsg1 = TRACKED_NEW plControlEventMsg;
		pMsg1->SetBCastFlag(plMessage::kBCastByType);
		pMsg1->SetControlCode(B_SET_CONSOLE_MODE);
		pMsg1->SetControlActivated(true);
		plgDispatch::MsgSend( pMsg1 );	// whoosh... off it goes
		// create message to send to the console
		plControlEventMsg* pMsg2 = TRACKED_NEW plControlEventMsg;
		pMsg2->SetBCastFlag(plMessage::kBCastByType);
		pMsg2->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
		pMsg2->SetControlActivated(true);
		pMsg2->SetCmdString(command);
		plgDispatch::MsgSend( pMsg2 );	// whoosh... off it goes
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TimerCallback
//  PARAMETERS : command   - string of console commmand to execute
//
//  PURPOSE    : Execute a console command from a python script
//
void cyMisc::TimerCallback(pyKey& selfkey, hsScalar time, UInt32 id)
{
	// setup the message to come back to whoever the pyKey is pointing to
	plTimerCallbackMsg* pTimerMsg = TRACKED_NEW plTimerCallbackMsg(selfkey.getKey(),id);
	plgTimerCallbackMgr::NewTimer( time, pTimerMsg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ClearTimerCallbacks
//  PARAMETERS : key of object to clear callbacks to
//
//  PURPOSE    : clear timer callbacks to a certain key
//
void cyMisc::ClearTimerCallbacks(pyKey& selfkey)
{
	plgTimerCallbackMgr::CancelCallbacksToKey(selfkey.getKey());
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AttachObject
//  PARAMETERS : child object
//             :   to be attached to parent object
//
//  PURPOSE    : Attach an object to another object, knowing only their pyKeys
//
void cyMisc::AttachObject(pyKey& ckey, pyKey& pkey)
{
	plKey childKey = ckey.getKey();
	plKey parentKey = pkey.getKey();

	// make sure that there was a child ket
	if ( childKey )
	{
		// create the attach message to attach the child
		plAttachMsg* pMsg = TRACKED_NEW plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRequest);
		plgDispatch::MsgSend( pMsg );
	}
}
void cyMisc::AttachObjectSO(pySceneObject& cobj, pySceneObject& pobj)
{
	plKey childKey = cobj.getObjKey();
	plKey parentKey = pobj.getObjKey();

	// make sure that there was a child ket
	if ( childKey )
	{
		// create the attach message to attach the child
		plAttachMsg* pMsg = TRACKED_NEW plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRequest);
		plgDispatch::MsgSend( pMsg );
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : DetachObject
//  PARAMETERS : child object
//             :   to be attached to parent object
//
//  PURPOSE    : Attach an object to another object, knowing only their pyKeys
//
void cyMisc::DetachObject(pyKey& ckey, pyKey& pkey)
{
	plKey childKey = ckey.getKey();
	plKey parentKey = pkey.getKey();

	// make sure that there was a child ket
	if ( childKey )
	{
		// create the attach message to detach the child
		plAttachMsg* pMsg = TRACKED_NEW plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRemove);
		plgDispatch::MsgSend( pMsg );
	}
}
void cyMisc::DetachObjectSO(pySceneObject& cobj, pySceneObject& pobj)
{
	plKey childKey = cobj.getObjKey();
	plKey parentKey = pobj.getObjKey();

	// make sure that there was a child ket
	if ( childKey )
	{
		// create the attach message to detach the child
		plAttachMsg* pMsg = TRACKED_NEW plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRemove);
		plgDispatch::MsgSend( pMsg );
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : LinkToAge
//  PARAMETERS : 
//
//  PURPOSE    : LinkToAge
//
//  STATUS     : Depreciated. Use plNetLinkingMgr or pyNetLinkingMgr instead.
//

//void cyMisc::LinkToAge(pyKey &selfkey, const char *AgeName,const char *SpawnPointName)
//{
//	// find the Modifier that called us
//	hsStatusMessage("PY: LinkToAge\n");
//		// Ask the Modifier if it was Local or Network
//	if (selfkey.WasLocalNotify())
//	{
//		hsStatusMessage("PY:LOCAL NOTIFY\n");
//		plNetLinkingMgr::GetInstance()->LinkToPublicAge( AgeName, SpawnPointName );
//	}
//	else
//	{
//		hsStatusMessage("PY:REMOTE NOTIFY\n");
//	}
//}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetDirtySyncStateServer
//  PARAMETERS : 
//
//  PURPOSE    : set the Python modifier to be dirty and asked to be saved out
//
void cyMisc::SetDirtySyncState(pyKey &selfkey, const char* SDLStateName, UInt32 sendFlags)
{
	selfkey.DirtySynchState(SDLStateName, sendFlags);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetDirtySyncStateClients
//  PARAMETERS : 
//
//  PURPOSE    : set the Python modifier to be dirty and asked to be saved out
//
void cyMisc::SetDirtySyncStateWithClients(pyKey &selfkey, const char* SDLStateName, UInt32 sendFlags)
{
	selfkey.DirtySynchState(SDLStateName, sendFlags|plSynchedObject::kBCastToClients);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnableControlKeyEvents & DisableControlKeyEvents
//  PARAMETERS : none
//
//  PURPOSE    : register and unregister for control key events
//
void cyMisc::EnableControlKeyEvents(pyKey &selfkey)
{
	selfkey.EnableControlKeyEvents();
}

void cyMisc::DisableControlKeyEvents(pyKey &selfkey)
{
	selfkey.DisableControlKeyEvents();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClientName
//  PARAMETERS : avatar key
//
//  PURPOSE    : Return the net client (account) name of the player whose avatar
//				key is provided.
//
hsBool cyMisc::WasLocallyNotified(pyKey &selfkey)
{
	return selfkey.WasLocalNotify();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClientName
//  PARAMETERS : avatar key
//
//  PURPOSE    : Return the net client (account) name of the player whose avatar
//				key is provided.
//
const char* cyMisc::GetClientName(pyKey &avKey)
{
	const char* ret=plNetClientMgr::GetInstance()->GetPlayerName(avKey.getKey());
	return (ret==nil) ? "" : ret;
}

PyObject* cyMisc::GetAvatarKeyFromClientID(int clientID)
{
	PyObject* keyObj = NULL;

	if (clientID == plNetClientMgr::GetInstance()->GetPlayerID())
	{
		keyObj = pyKey::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	}
	else
	{
		plNetTransportMember **members = nil;
		plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted( members );

		if( members != nil)
		{
			for(int i = 0; i < plNetClientMgr::GetInstance()->TransportMgr().GetNumMembers(); i++ )
			{
				plNetTransportMember *mbr = members[ i ];
				if( mbr != nil && mbr->GetAvatarKey() != nil && mbr->GetPlayerID() == clientID)
				{	
					keyObj = pyKey::New(mbr->GetAvatarKey());
					break;
				}
			}
		}

		delete [] members;
	}

	if (keyObj)
		return keyObj;
	else
		PYTHON_RETURN_NONE;
}


int cyMisc::GetClientIDFromAvatarKey(pyKey& avatar)
{
	int ret = -1;

	if (plNetClientMgr::GetInstance()->GetLocalPlayerKey() == avatar.getKey())
	{
		return (plNetClientMgr::GetInstance()->GetPlayerID());
	}
	plNetTransportMember **members = nil;
	plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted( members );
	if( members != nil)
	{
		for(int i = 0; i < plNetClientMgr::GetInstance()->TransportMgr().GetNumMembers(); i++ )
		{
			plNetTransportMember *mbr = members[ i ];

			if( mbr != nil && mbr->GetAvatarKey() == avatar.getKey())
			{	
				ret = mbr->GetPlayerID();
				break;
			}
		}
	}

	delete [] members;
	return ret;
}

int cyMisc::GetLocalClientID()
{
	return (plNetClientMgr::GetInstance()->GetPlayerID());
}

hsBool cyMisc::ValidateKey(pyKey& key)
{
	plKey pKey = key.getKey();
	
	if (pKey && pKey->ObjectIsLoaded())
		return true;
		
	return false;
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClientName
//  PARAMETERS : 
//
//  PURPOSE    : Return the local net client (account) name
//
const char* cyMisc::GetLocalClientName()
{
	return plNetClientMgr::GetInstance()->GetPlayerName();
}


//
// Get Current age information - DEPRECIATED. Use ptDniInfoSource() object instead
//
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetAgeName
//  Function   : GetAgeTime
//  Function   : GetAgeGuid
//  PARAMETERS : 
//
//  PURPOSE    : Return the age name of the current age the local player is in
//             : Return the current coordinates of the player within this age
//             : Return the current time with the current age the player is in
//             : Return the current guid of the instance of the age the player is in
//

const char * cyMisc::GetAgeName()
{
	return NetCommGetAge()->ageDatasetName;
}

PyObject* cyMisc::GetAgeInfo()
{
	plNetLinkingMgr* nmgr = plNetLinkingMgr::GetInstance();
	if (nmgr)
	{
		plAgeLinkStruct* als = nmgr->GetAgeLink();
		if (als)
			return pyAgeInfoStruct::New(als->GetAgeInfo());
	}
	PYTHON_RETURN_NONE; // return none, not nil (cause it isn't really an error... or is it?)
}


const char* cyMisc::GetPrevAgeName()
{
	plNetLinkingMgr* nmgr = plNetLinkingMgr::GetInstance();
	if (nmgr)
	{
		plAgeLinkStruct* als = nmgr->GetPrevAgeLink();
		if (als)
			return als->GetAgeInfo()->GetAgeFilename();
	}
	return nil;
}

PyObject* cyMisc::GetPrevAgeInfo()
{
	plNetLinkingMgr* nmgr = plNetLinkingMgr::GetInstance();
	if (nmgr)
	{
		plAgeLinkStruct* als = nmgr->GetPrevAgeLink();
		if (als)
			return pyAgeInfoStruct::New(als->GetAgeInfo());
	}
	PYTHON_RETURN_NONE; // return none, not nil (cause it isn't really an error... or is it?)
}

// current time in current age
UInt32 cyMisc::GetAgeTime( void )
{
	return VaultAgeGetAgeTime();
}



UInt32 cyMisc::GetDniTime(void)
{
	const plUnifiedTime utime = plNetClientMgr::GetInstance()->GetServerTime();
	if ( utime.GetSecs() != 0)
		return ConvertGMTtoDni(utime.GetSecs());
	else
		return 0;
}

UInt32 cyMisc::GetServerTime(void)
{
	const plUnifiedTime utime = plNetClientMgr::GetInstance()->GetServerTime();
	return utime.GetSecs();
}

float cyMisc::GetAgeTimeOfDayPercent(void)
{
	return plNetClientMgr::GetInstance()->GetCurrentAgeTimeOfDayPercent();
}

#define kMST (UInt32)25200
#define kOneHour (UInt32)3600
#define kOneDay (UInt32)86400

UInt32 cyMisc::ConvertGMTtoDni(UInt32 gtime)
{
	// convert to mountain time
	UInt32 dtime = gtime - kMST;
	plUnifiedTime utime = plUnifiedTime();
	utime.SetSecs(dtime);
	// check for daylight savings time in New Mexico and adjust
	if ( utime.GetMonth() >= 4 && utime.GetMonth() < 11 )
	{
		plUnifiedTime dstStart = plUnifiedTime();
		dstStart.SetGMTime(utime.GetYear(),4,1,2,0,0);
		// find first Sunday after 4/1 (first sunday of April)
		UInt32 days_to_go = 7 - dstStart.GetDayOfWeek();
		if (days_to_go == 7)
			days_to_go = 0;
		UInt32 dstStartSecs = dstStart.GetSecs() + days_to_go * kOneDay;

		plUnifiedTime dstEnd = plUnifiedTime();
		dstEnd.SetGMTime(utime.GetYear(),10,25,1,0,0);
		// find first sunday after 10/25 (last sunday of Oct.)
		days_to_go = 7 - dstEnd.GetDayOfWeek();
		if (days_to_go == 7)
			days_to_go = 0;
		UInt32 dstEndSecs = dstEnd.GetSecs() + days_to_go * kOneDay;

		if ( dtime > dstStartSecs && dtime < dstEndSecs )
			// add hour for daylight savings time
			dtime += kOneHour;
	}

	return dtime;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExcludeRegionSet
//  PARAMETERS : key - of the exclude region, ie. where to send the message
//               state  - what state of to set at:
//                        0 = release
//                        1 = clear
//
//  PURPOSE    : Sets the state of an exclude region
//
void cyMisc::ExcludeRegionSet(pyKey& sender, pyKey& exKey, UInt16 state)
{
	plExcludeRegionMsg *msg = TRACKED_NEW plExcludeRegionMsg;

	switch (state)
	{
		case kExRegClear:
			msg->SetCmd(plExcludeRegionMsg::kClear);
			break;
		case kExRegRelease:
			msg->SetCmd(plExcludeRegionMsg::kRelease);
			break;
	}
	msg->SetSender(sender.getKey());
	msg->AddReceiver(exKey.getKey());
	plgDispatch::MsgSend( msg );	// whoosh... off it goes
}

void cyMisc::ExcludeRegionSetNow(pyKey& sender, pyKey& exKey, UInt16 state)
{
	plExcludeRegionMsg *msg = TRACKED_NEW plExcludeRegionMsg;

	switch (state)
	{
		case kExRegClear:
			msg->SetCmd(plExcludeRegionMsg::kClear);
			break;
		case kExRegRelease:
			msg->SetCmd(plExcludeRegionMsg::kRelease);
			break;
	}
	msg->SetSender(sender.getKey());
	msg->AddReceiver(exKey.getKey());
	msg->fSynchFlags = plSynchedObject::kSendImmediately;
	plgDispatch::MsgSend( msg );	// whoosh... off it goes
}

#include "hsTimer.h"
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetSeconds
//  PARAMETERS : 
//
//  PURPOSE    : Return the nunber of seconds elapsed
//
double cyMisc::GetSeconds()
{
	return hsTimer::GetSeconds();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetSysSeconds
//  PARAMETERS : 
//
//  PURPOSE    : Return the number of system seconds elapsed
//
double cyMisc::GetSysSeconds()
{
	return hsTimer::GetSysSeconds();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetDelSysSeconds
//  PARAMETERS : 
//
//  PURPOSE    : Return the frame delta seconds
//
hsScalar cyMisc::GetDelSysSeconds()
{
	return hsTimer::GetDelSysSeconds();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : LoadDialog
//  PARAMETERS : 
//
//  PURPOSE    : Return the frame delta seconds
//
void cyMisc::LoadDialog(const char* name)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
	{
		if ( !mgr->IsDialogLoaded(name) )
			mgr->LoadDialog( name );
	}
}

// Load dialog and set the GUINotifyMsg receiver key
void cyMisc::LoadDialogK(const char* name, pyKey& rKey)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
	{
		// has the dialog been loaded yet?
		if ( !mgr->IsDialogLoaded(name) )
			// no then load and set handler
			mgr->LoadDialog( name, rKey.getKey() );
		else
			// yes then just set the handler
			mgr->SetDialogToNotify(name,rKey.getKey());
	}
}

// Load dialog and set the GUINotifyMsg receiver key
void cyMisc::LoadDialogKA(const char* name, pyKey& rKey, const char* ageName)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
	{
		// has the dialog been loaded yet?
		if ( !mgr->IsDialogLoaded(name) )
			// no then load and set handler
			mgr->LoadDialog( name, rKey.getKey(), ageName );
		else
			// yes then just set the handler
			mgr->SetDialogToNotify(name,rKey.getKey());
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : UnLoadDialog
//  PARAMETERS : 
//
//  PURPOSE    : UnLoads the dialog by name
//             : optionally sets the receiver key for the GUINotifyMsg
//
void cyMisc::UnloadDialog(const char* name)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
	{
		if ( mgr->IsDialogLoaded(name) )
			mgr->UnloadDialog( name );
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IsDialogLoaded
//  PARAMETERS : 
//
//  PURPOSE    : Test to see if a dialog is loaded (according to the dialog manager)
//
hsBool cyMisc::IsDialogLoaded(const char* name)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
		return mgr->IsDialogLoaded(name);
	return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ShowDialog
//  Function   : HideDialog
//  PARAMETERS : 
//
//  PURPOSE    : Show or Hide a dialog by name
//
void cyMisc::ShowDialog(const char* name)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
		mgr->ShowDialog(name);
}
void cyMisc::HideDialog(const char* name)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
		mgr->HideDialog(name);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetDialogFromTagID
//  PARAMETERS : 
//
//  PURPOSE    : Return the frame delta seconds
//
PyObject* cyMisc::GetDialogFromTagID(UInt32 tag)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
	{
		// get the owner dialog modifier pointer
		pfGUIDialogMod* pdialog = mgr->GetDialogFromTag(tag);
		if ( pdialog )
			return pyGUIDialog::New(pdialog->GetKey());
	}

	char errmsg[256];
	sprintf(errmsg,"GUIDialog TagID %d not found",tag);
	PyErr_SetString(PyExc_KeyError, errmsg);
	return nil; // return nil, cause we threw an error
}

PyObject* cyMisc::GetDialogFromString(const char* name)
{
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if ( mgr )
	{
		// get the owner dialog modifier pointer
		pfGUIDialogMod* pdialog = mgr->GetDialogFromString(name);
		if ( pdialog )
			return pyGUIDialog::New(pdialog->GetKey());
	}

	char errmsg[256];
	sprintf(errmsg,"GUIDialog %s not found",name);
	PyErr_SetString(PyExc_KeyError, errmsg);
	return nil; // return nil, cause we threw an error
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IsGUIModal
//  PARAMETERS : 
//
//  PURPOSE    : Returns true if the GUI is currently modal (and therefore blocking input)
//
bool cyMisc::IsGUIModal()
{
	pfGameGUIMgr* mgr = pfGameGUIMgr::GetInstance();
	if (mgr)
		return mgr->IsModalBlocking();
	return false;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetLocalAvatar
//  Function   : GetLocalPlayer
//  PARAMETERS : 
//
//  PURPOSE    : Return a pySceneobject of the local Avatar
//             : Player - returns ptPlayer object
//
PyObject* cyMisc::GetLocalAvatar()
{
	plSceneObject *so = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());
	if ( so )
		return pySceneObject::New(so->GetKey());

	char errmsg[256];
	sprintf(errmsg,"Local avatar not found");
	PyErr_SetString(PyExc_NameError, errmsg);
	return nil; // returns nil, cause we threw an error
}

PyObject* cyMisc::GetLocalPlayer()
{
	return pyPlayer::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(),
						plNetClientMgr::GetInstance()->GetPlayerName(),
						plNetClientMgr::GetInstance()->GetPlayerID(),
						0.0 );
}



#if 1
#include "../plStatusLog/plStatusLog.h"
//
// TEMP SCREEN PRINT CODE FOR NON-DBG TEXT DISPLAY
//
void cyMisc::PrintToScreen(const char* msg)
{
	static plStatusLog* gStatusLog=nil;
	if (gStatusLog==nil)
	{
		gStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog( 32, "", 
			plStatusLog::kDontWriteFile | plStatusLog::kDeleteForMe | plStatusLog::kFilledBackground );
		plStatusLogMgr::GetInstance().ToggleStatusLog(gStatusLog);
	}
	gStatusLog->AddLine(msg, plStatusLog::kBlue);	
}
#endif

#include "plPipeline.h"
#include "../plGImage/plMipmap.h"

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetPlayerList
//  Function   : GetPlayerListDistanceSorted
//  PARAMETERS : 
//
//  PURPOSE    : Get a list of players (other than self) that are playing the game
//             : optionally get it sorted by distance
//
//  RETURNS    : Python object list of pyPlayer s
//
std::vector<PyObject*> cyMisc::GetPlayerList()
{
	std::vector<PyObject*> pyPL;
	plNetClientMgr* nc=plNetClientMgr::GetInstance();

	if (!nc) // only ever really happens if they try to call us in max... I hope
		return pyPL;

	int i;
	for( i = 0; i < nc->TransportMgr().GetNumMembers(); i++ )
	{
		plNetTransportMember *mbr = nc->TransportMgr().GetMember(i);
		plKey avkey = mbr->GetAvatarKey();
		if (avkey)
		{
			// only non-ignored people in list and not in ignore list
			if ( !VaultAmIgnoringPlayer ( mbr->GetPlayerID()) )
			{
				PyObject* playerObj = pyPlayer::New(avkey, mbr->GetPlayerName(), mbr->GetPlayerID(), mbr->GetDistSq());
				pyPlayer* player = pyPlayer::ConvertFrom(playerObj); // accesses internal pyPlayer object

				// modifies playerObj
				if ( mbr->IsCCR() )
					player->SetCCRFlag(true);
				if ( mbr->IsServer() )
					player->SetServerFlag(true);
				
				pyPL.push_back(playerObj);
			}
		}
	}
	return pyPL;
}

std::vector<PyObject*> cyMisc::GetPlayerListDistanceSorted()
{
	std::vector<PyObject*> pyPL;

	// get the sorted member list from the Net transport manager
	plNetTransportMember **members = nil;
	plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted( members );
	if( members != nil )
	{
		int i;
		for( i = 0; i < plNetClientMgr::GetInstance()->TransportMgr().GetNumMembers(); i++ )
		{
			plNetTransportMember *mbr = members[ i ];
			plKey avkey = mbr->GetAvatarKey();
			if (avkey)
			{
				// only non-ignored people in list and not in ignore list
				if ( !VaultAmIgnoringPlayer ( mbr->GetPlayerID()) )
				{
					PyObject* playerObj = pyPlayer::New(avkey, mbr->GetPlayerName(), mbr->GetPlayerID(), mbr->GetDistSq());
					pyPlayer* player = pyPlayer::ConvertFrom(playerObj); // accesses internal pyPlayer object

					// modifies playerObj
					if ( mbr->IsCCR() )
						player->SetCCRFlag(true);
					if ( mbr->IsServer() )
						player->SetServerFlag(true);

					pyPL.push_back(playerObj);
				}
			}
		}
		delete [] members;
	}
	return pyPL;
}

UInt32 cyMisc::GetMaxListenListSize()
{
	return plNetListenList::kMaxListenListSize;
}

hsScalar cyMisc::GetMaxListenDistSq()
{
	return plNetListenList::kMaxListenDistSq;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendRTChat
//  PARAMETERS : from   - is a pyPlayer of the person who is sending this
//             : tolist  - is a python list object, if empty then broadcast message
//             : message  - text string to send to others
//             : flags   - the flags of destiny... whatever that means
//
//  PURPOSE    : To send a real time chat message to a particualr user, a list of users
//             : or broadcast it to everyone (within hearing distance?)
//
//  RETURNS    : the flags that were sent with the message (may be modified)
//
UInt32 cyMisc::SendRTChat(pyPlayer& from, const std::vector<pyPlayer*> & tolist, const char* message, UInt32 flags)
{
	// create the messge that will contain the chat message
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kHACKChatMsg );
	msg->SetString( message );
	msg->SetUser( from.GetPlayerName(), from.GetPlayerID() );
	msg->SetFlags( flags );
	msg->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce);
	msg->SetBCastFlag(plMessage::kLocalPropagate, 0);

	if (tolist.size() > 0)
	{
		if (flags & 8/* kRTChatInterAge in PlasmaKITypes.py */)
		{
			// allow inter-age routing of this msg
			msg->SetBCastFlag( plMessage::kNetAllowInterAge );
		}
		// add net rcvrs to msg
		int i;
		for ( i=0 ; i<tolist.size() ; i++ )
		{
			if ( !VaultAmIgnoringPlayer( tolist[i]->GetPlayerID() ) )
				msg->AddNetReceiver(tolist[i]->GetPlayerID());
		}
	}

	UInt32 msgFlags = msg->GetFlags();

	if (tolist.size() == 0 || (msg->GetNetReceivers() && msg->GetNetReceivers()->size() > 0))
		msg->Send();

	return msgFlags;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendKIMessage
//  PARAMETERS : command   - the command type
//             : value     - extra value
//
//  PURPOSE    : Send message to the KI, to tell it things to do
//
//  RETURNS    : nothing
//
void cyMisc::SendKIMessage(UInt32 command, hsScalar value)
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( (UInt8)command );

	// check to see if the value makes any sense
	if ( command == pfKIMsg::kSetChatFadeDelay )
	{
		msg->SetDelay(value);
	}
	else if ( command == pfKIMsg::kSetTextChatAdminMode )
	{
		msg->SetFlags( value==1.0f ? pfKIMsg::kAdminMsg : 0 );
	}
	// send it off
	plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendKIMessageS
//  PARAMETERS : command   - the command type
//             : value     - extra value as a string
//
//  PURPOSE    : Send message to the KI, to tell it things to do
//
//  RETURNS    : nothing
//
void cyMisc::SendKIMessageS(UInt32 command, const wchar_t* value)
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( (UInt8)command );

	msg->SetString( value );

	// send it off
	plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendKIMessageI
//  PARAMETERS : command   - the command type
//             : value     - extra value as an Int32
//
//  PURPOSE    : Send message to the KI, to tell it things to do
//
//  RETURNS    : nothing
//
void  cyMisc::SendKIMessageI(UInt32 command, Int32 value)
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( (UInt8)command );

	msg->SetIntValue(value);

	// send it off
	plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendKIMessageIReply
//  PARAMETERS : command   - the command type
//             : value     - extra value as an Int32
//
//  PURPOSE    : Send message to the KI, to tell it things to do
//
//  RETURNS    : nothing
//
void  cyMisc::SendKIGZMarkerMsg(Int32 markerNumber, pyKey& sender)
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kGZInRange );

	msg->SetIntValue(markerNumber);
	msg->SetSender(sender.getKey());

	// send it off
	plgDispatch::MsgSend( msg );
}

void cyMisc::SendKIRegisterImagerMsg(const char* imagerName, pyKey& sender)
{
	pfKIMsg *msg = TRACKED_NEW pfKIMsg(pfKIMsg::kRegisterImager);

	msg->SetString(imagerName);
	msg->SetSender(sender.getKey());

	plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : YesNoDialog
//  PARAMETERS : sender    - who set this and will get the notify
//             : message   - message to put up in YesNo dialog
//
//  PURPOSE    : Put up Yes/No dialog
//
//  RETURNS    : nothing
//

void cyMisc::YesNoDialog(pyKey& sender, const char* thestring)
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kYesNoDialog );

	msg->SetSender(sender.getKey());
	msg->SetString(thestring);
	// send it off
	plgDispatch::MsgSend( msg );
}

void cyMisc::YesNoDialog(pyKey& sender, std::wstring thestring)
{
	char *temp = hsWStringToString(thestring.c_str());
	YesNoDialog(sender,temp);
	delete [] temp;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RateIt
//  PARAMETERS : chonicleName    - where to store the rating
//             : thestring       - the message in the RateIt dialog
//             : onceFlag        - flag to tell whether this is a onetime thing or ask everytime
//
//  PURPOSE    : Send message to the KI to tell it to ask the user to Rate something
//
//  RETURNS    : nothing
//
void cyMisc::RateIt(const char* chronicleName, const char* thestring, hsBool onceFlag)
{
	// create the mesage to send
	pfKIMsg *msg = TRACKED_NEW pfKIMsg( pfKIMsg::kRateIt );

	msg->SetUser(chronicleName,0);
	msg->SetString(thestring);
	msg->SetIntValue(onceFlag);
	// send it off
	plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetPrivateChatList
//  PARAMETERS : key			- who's joining
//
//  PURPOSE    : Lock the local avatar into private vox messaging, and / or add new memebers to his chat list
//
//  RETURNS    : nothing
//

void cyMisc::SetPrivateChatList(const std::vector<pyPlayer*> & tolist)
{
	if (tolist.size() > 0)
	{
		plNetVoiceListMsg* pMsg = TRACKED_NEW plNetVoiceListMsg(plNetVoiceListMsg::kForcedListenerMode);
		for (int i=0 ; i<tolist.size() ; i++ )
			pMsg->GetClientList()->Append(tolist[i]->GetPlayerID());
		
		pMsg->SetRemovedKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
		pMsg->Send();
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ClearPrivateChatList
//  PARAMETERS : key			- who's leaving
//
//  PURPOSE    : Remove the local avatar from private vox messaging, and / or clear memebers from his chat list
//
//  RETURNS    : nothing
//
void cyMisc::ClearPrivateChatList(pyKey& member)
{
	plNetVoiceListMsg* pMsg = TRACKED_NEW plNetVoiceListMsg(plNetVoiceListMsg::kDistanceMode);
	pMsg->SetRemovedKey( member.getKey() );
	pMsg->Send();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ClearCameraStack
//  PURPOSE    : clear the camera stack
//
//  RETURNS    : nothing
//

void cyMisc::ClearCameraStack()
{
	plVirtualCam1::Instance()->ClearStack();
}

bool cyMisc::IsFirstPerson()
{
	return (plVirtualCam1::Instance()->Is1stPersonCamera()!=0);
}
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendPetitionToCCR
//  PARAMETERS : message  - message to send to the CCR ("please help me!")
//
//  PURPOSE    : Send a petition to the CCR for help or questions
//
void cyMisc::SendPetitionToCCR(const char* message)
{
	SendPetitionToCCRI(message,plNetCommon::PetitionTypes::kGeneralHelp,nil);
}
void cyMisc::SendPetitionToCCRI(const char* message, UInt8 reason,const char* title)
{
	// create the mesage to send
	plCCRPetitionMsg *msg = TRACKED_NEW plCCRPetitionMsg();
	msg->SetNote(message);
	msg->SetType(reason);
	if (title)
		msg->SetTitle(title);
	// send it off
	plgDispatch::MsgSend( msg );
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendChatToCCR
//  PARAMETERS : message  - message to send to the CCR ("please help me!")
//
//  PURPOSE    : Send a chat message to the CCR for help or questions
//
void cyMisc::SendChatToCCR(const char* message,Int32 CCRPlayerID)
{
	// create the mesage to send
	plCCRCommunicationMsg *msg = TRACKED_NEW plCCRCommunicationMsg();
	msg->SetMessage(message);
	msg->SetType(plCCRCommunicationMsg::kReturnChatMsg);
	msg->SetBCastFlag(plMessage::kNetAllowInterAge);
	msg->SetBCastFlag(plMessage::kNetPropagate);
	msg->SetBCastFlag(plMessage::kNetForce);
	msg->SetBCastFlag(plMessage::kLocalPropagate,0);
	msg->AddNetReceiver( CCRPlayerID );
	msg->Send();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetNumRemotePlayers
//  
//  PURPOSE    : return the number of remote players connected
//
int cyMisc::GetNumRemotePlayers()
{
	return plNetClientMgr::GetInstance()->RemotePlayerKeys().size();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Paging functions
//  PARAMETERS : nodeName  - name of the page to load
//  
//  PURPOSE    : page in, hold or out a particular node
//

void cyMisc::PageInNodes(const std::vector<std::string> & nodeNames, const char* age)
{
	if (hsgResMgr::ResMgr())
	{
		plSynchEnabler ps(false);	// disable dirty tracking while paging in
		plClientMsg* msg = TRACKED_NEW plClientMsg(plClientMsg::kLoadRoom);
		plKey clientKey = hsgResMgr::ResMgr()->FindKey(kClient_KEY);
		msg->AddReceiver(clientKey);

		int numNames = nodeNames.size();
		for (int i = 0; i < numNames; i++)
			msg->AddRoomLoc(plKeyFinder::Instance().FindLocation(age ? age : NetCommGetAge()->ageDatasetName, nodeNames[i].c_str()));

		msg->Send();
	}
}

void cyMisc::PageOutNode(const char* nodeName)
{
	if ( hsgResMgr::ResMgr() )
	{
		plSynchEnabler ps(false);	// disable dirty tracking while paging out
		plClientMsg* pMsg1 = TRACKED_NEW plClientMsg(plClientMsg::kUnloadRoom);
		plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
		pMsg1->AddReceiver( clientKey );
		pMsg1->AddRoomLoc(plKeyFinder::Instance().FindLocation(nil, nodeName));
		plgDispatch::MsgSend(pMsg1);
	}
}


#include "../plAvatar/plArmatureMod.h"
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : LimitAvatarLOD
//  PARAMETERS : LODlimit - number of to limit the LOD to
//  
//  PURPOSE    : sets the avatar LOD limit
//
void cyMisc::LimitAvatarLOD(int LODlimit)
{
	if(LODlimit >= 0 && LODlimit <= 2)
		plArmatureLODMod::fMinLOD = LODlimit;
}



#include "../plPipeline/plFogEnvironment.h"

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Set fog default functions
//  PARAMETERS : floats  - the parameters
//  
//  PURPOSE    : sets the fog defaults
//
void cyMisc::FogSetDefColor(pyColor& color)
{
	if ( fPipeline )
	{
		hsColorRGBA hcolor = color.getColor();
		hcolor.a = 1.0f;		// make sure that alpha is 1
		plFogEnvironment env = fPipeline->GetDefaultFogEnviron();
		env.SetColor( hcolor );
		fPipeline->SetDefaultFogEnviron( &env );
	}
}

void cyMisc::FogSetDefLinear(float start, float end, float density)
{
	if ( fPipeline )
	{
		plFogEnvironment env = fPipeline->GetDefaultFogEnviron();
		env.Set( start, end, density ); 
		fPipeline->SetDefaultFogEnviron( &env );
	}
}

void cyMisc::FogSetDefExp(float end, float density)
{
	if ( fPipeline )
	{
		plFogEnvironment env = fPipeline->GetDefaultFogEnviron();
		env.SetExp( plFogEnvironment::kExpFog, end, density ); 
		fPipeline->SetDefaultFogEnviron( &env );
	}
}

void cyMisc::FogSetDefExp2(float end, float density)
{
	if ( fPipeline )
	{
		plFogEnvironment env = fPipeline->GetDefaultFogEnviron();
		env.SetExp( plFogEnvironment::kExp2Fog, end, density ); 
		fPipeline->SetDefaultFogEnviron( &env );
	}
}


void cyMisc::SetClearColor(float red, float green, float blue)
{
	// do this command via the console to keep the maxplugins from barfing
	char command[256];
	sprintf(command,"Graphics.Renderer.SetClearColor %f %f %f",red,green,blue);
	// create message to send to the console
	plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
	pMsg->SetBCastFlag(plMessage::kBCastByType);
	pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
	pMsg->SetControlActivated(true);
	pMsg->SetCmdString(command);
	plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Enable / disable cursor fade for avatar
//  PARAMETERS : 
//
//  PURPOSE    : turns avatar fade out on / off 
//

void cyMisc::EnableAvatarCursorFade()
{
	plNetClientMgr* nmgr = plNetClientMgr::GetInstance();
	if (nmgr)
	{
		plIfaceFadeAvatarMsg* iMsg = TRACKED_NEW plIfaceFadeAvatarMsg;
		iMsg->SetSubjectKey(nmgr->GetLocalPlayerKey());
		iMsg->SetBCastFlag(plMessage::kBCastByExactType);
		iMsg->SetBCastFlag(plMessage::kNetPropagate, FALSE);
		iMsg->Enable();
		iMsg->Send();
	}
}

void cyMisc::DisableAvatarCursorFade()
{
	plNetClientMgr* nmgr = plNetClientMgr::GetInstance();
	if (nmgr)
	{
		plIfaceFadeAvatarMsg* iMsg = TRACKED_NEW plIfaceFadeAvatarMsg;
		iMsg->SetSubjectKey(nmgr->GetLocalPlayerKey());
		iMsg->SetBCastFlag(plMessage::kBCastByExactType);
		iMsg->SetBCastFlag(plMessage::kNetPropagate, FALSE);
		iMsg->Disable();
		iMsg->Send();
	}
}

void cyMisc::FadeLocalPlayer(hsBool fade)
{
	plNetClientMgr* nmgr = plNetClientMgr::GetInstance();
	if (nmgr)
	{
		plCameraTargetFadeMsg* pMsg = TRACKED_NEW plCameraTargetFadeMsg;
		pMsg->SetFadeOut(fade);
		pMsg->SetSubjectKey(nmgr->GetLocalPlayerKey());
		pMsg->SetBCastFlag(plMessage::kBCastByExactType);
		pMsg->SetBCastFlag(plMessage::kNetPropagate, FALSE);
		pMsg->AddReceiver(nmgr->GetLocalPlayerKey());
		plgDispatch::MsgSend(pMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : offer linking book functions
//  PARAMETERS : 
//
//  PURPOSE    : manage offering public (pedestal) books
//

void cyMisc::EnableOfferBookMode(pyKey& selfkey, const char* ageFilename, const char* ageInstanceName)
{
	plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kSetOfferBookMode);
	pMsg->SetSender(selfkey.getKey());
	pMsg->SetAgeFileName(ageFilename);
	pMsg->SetAgeName(ageInstanceName);	
	pMsg->Send();
}

void cyMisc::DisableOfferBookMode()
{
	plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kClearOfferBookMode);
	pMsg->Send();
}

void cyMisc::NotifyOffererPublicLinkCompleted(UInt32 offerer)
{
	plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kNotifyOfferCompleted, plNetClientMgr::GetInstance()->GetPlayerID());
	pMsg->SetSender(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	if (offerer != plNetClientMgr::GetInstance()->GetPlayerID())
	{
		pMsg->SetBCastFlag( plMessage::kNetPropagate );
		pMsg->SetBCastFlag( plMessage::kNetForce );
		pMsg->SetBCastFlag( plMessage::kLocalPropagate, 0 );
		pMsg->AddNetReceiver( offerer );
	}

	pMsg->Send();
}

void cyMisc::NotifyOffererPublicLinkRejected(UInt32 offerer)
{
	plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kNotifyOfferRejected);
	pMsg->SetSender(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	if (offerer != plNetClientMgr::GetInstance()->GetPlayerID())
	{
		pMsg->SetBCastFlag( plMessage::kNetPropagate );
		pMsg->SetBCastFlag( plMessage::kNetForce );
		pMsg->SetBCastFlag( plMessage::kLocalPropagate, 0 );
		pMsg->AddNetReceiver( offerer );
	}

	pMsg->Send();
	ToggleAvatarClickability(true);
}

void cyMisc::NotifyOffererPublicLinkAccepted(UInt32 offerer)
{
	plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kNotifyOfferAccepted);
	pMsg->SetSender(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	if (offerer != plNetClientMgr::GetInstance()->GetPlayerID())
	{
		pMsg->SetBCastFlag( plMessage::kNetPropagate );
		pMsg->SetBCastFlag( plMessage::kNetForce );
		pMsg->SetBCastFlag( plMessage::kLocalPropagate, 0 );
		pMsg->AddNetReceiver( offerer );
	}

	pMsg->Send();
}

void cyMisc::ToggleAvatarClickability(hsBool on)
{
	plInputIfaceMgrMsg* pMsg = 0;
	if (on)
		pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIEnableAvatarClickable);
	else
		pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIDisableAvatarClickable);
	pMsg->SetAvKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	pMsg->SetBCastFlag(plMessage::kNetPropagate);
	pMsg->SetBCastFlag(plMessage::kNetForce);
	pMsg->Send();

}

void cyMisc::SetShareSpawnPoint(const char* spawnPoint)
{
	plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kSetShareSpawnPoint);
	pMsg->SetSender(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	pMsg->SetSpawnPoint(spawnPoint);
	pMsg->Send();
}

void cyMisc::SetShareAgeInstanceGuid(const Uuid& guid)
{
	plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kSetShareAgeInstanceGuid);
	pMsg->SetSender(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	pMsg->SetAgeInstanceGuid(guid);
	pMsg->Send();
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : GetCCRAwayStatus
// PARAMETERS :
//
// PURPOSE	  : Returns current status of CCR dept
//
hsBool cyMisc::IsCCRAwayStatus()
{
	return !VaultGetCCRStatus();
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : AmCCR
// PARAMETERS :
//
// PURPOSE	  : Returns true if local player is a CCR
//
hsBool cyMisc::AmCCR()
{
	if ( plNetClientApp::GetInstance() )
		return plNetClientApp::GetInstance()->AmCCR();
	return false;
}


//////////////////////////////////////////////////////////////////////////////
//
// Function   : AcceptInviteInGame
// PARAMETERS : Friend's Name, Invite Key
//
// PURPOSE	  : Send's a VaultTask to the server to perform the invite
//
void cyMisc::AcceptInviteInGame(const char* friendName, const char* inviteKey)
{
	hsAssert(false, "eric, implement me");
#if 0
	plNetClientMgr* nc = plNetClientMgr::GetInstance();
	if (nc)
	{
		plNetMsgVaultTask msg;
		msg.SetNetProtocol(kNetProtocolCli2Auth);
		msg.SetTopic(plNetCommon::VaultTasks::kFriendInvite);
		msg.GetArgs()->AddString(plNetCommon::VaultTaskArgs::kFriendName,friendName);
		msg.GetArgs()->AddString(plNetCommon::VaultTaskArgs::kInviteKey,inviteKey);
		nc->SendMsg(&msg);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : GetLanguage
// PARAMETERS :
//
// PURPOSE    : Returns the current language the game is in
//
int cyMisc::GetLanguage()
{
	return int(plLocalization::GetLanguage());
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : UsingUnicode
// PARAMETERS :
//
// PURPOSE    : Returns true if the current language uses Unicode (like Japanese)
//
bool cyMisc::UsingUnicode()
{
	return plLocalization::UsingUnicode();
}

//////////////////////////////////////////////////////////////////////////////
//
//
//	particle system management
//
//
//
#include "../plMessage/plParticleUpdateMsg.h"
#include "../plParticleSystem/plParticleSystem.h"
#include "../plParticleSystem/plParticleEffect.h"
void cyMisc::TransferParticlesToKey(pyKey& fromKey, pyKey& toKey, int numParticles)
{
	plKey frKey = fromKey.getKey();
	plSceneObject* so = plSceneObject::ConvertNoRef(toKey.getKey()->ObjectIsLoaded());
	if (so == nil) 
		return;
	
	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	plParticleTransferMsg* pMsg = TRACKED_NEW plParticleTransferMsg(nil, avMod->GetKey(), 0, frKey, numParticles);
	pMsg->SetBCastFlag(plMessage::kNetPropagate);
	pMsg->SetBCastFlag(plMessage::kNetForce);
	pMsg->Send();
}

void cyMisc::SetParticleDissentPoint(float x, float y, float z, pyKey& particles)
{
	plKey frKey = particles.getKey();
	plSceneObject* pObj = plSceneObject::ConvertNoRef(particles.getKey()->ObjectIsLoaded());
	if (!pObj)
		return;
	const plParticleSystem *sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));
	if (sys == nil) 
	{
		const plModifier* pArm = pObj->GetModifierByType(plArmatureMod::Index());
		if (pArm)
		{
			// it's the avatar
			const plArmatureMod* pCArm = (const plArmatureMod*)pArm;
			plArmatureMod* pNonConstArm = const_cast<plArmatureMod*>(pCArm);
			pObj = pNonConstArm->GetFollowerParticleSystemSO();
			if (!pObj)
				return;
			else
				sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));

		}
		if (sys == nil)
			return;
		
	}
	plParticleEffect *flock = sys->GetEffect(plParticleFlockEffect::Index());
	if (flock)
	{
		(TRACKED_NEW plParticleFlockMsg(nil, flock->GetKey(), 0, plParticleFlockMsg::kFlockCmdSetDissentPoint, x, y, z))->Send();
	}
}

void cyMisc::SetParticleOffset(float x, float y, float z, pyKey& particles)
{
	plKey frKey = particles.getKey();
	plSceneObject* pObj = plSceneObject::ConvertNoRef(particles.getKey()->ObjectIsLoaded());
	if (!pObj)
		return;
	const plParticleSystem *sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));
	if (sys == nil) 
	{
		const plModifier* pArm = pObj->GetModifierByType(plArmatureMod::Index());
		if (pArm)
		{
			// it's the avatar
			const plArmatureMod* pCArm = (const plArmatureMod*)pArm;
			plArmatureMod* pNonConstArm = const_cast<plArmatureMod*>(pCArm);
			pObj = pNonConstArm->GetFollowerParticleSystemSO();
			if (!pObj)
				return;
			else
				sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));

		}
		if (sys == nil)
			return;
		
	}

	plParticleEffect *flock = sys->GetEffect(plParticleFlockEffect::Index());
	if (flock)
	{
		(TRACKED_NEW plParticleFlockMsg(nil, flock->GetKey(), 0, plParticleFlockMsg::kFlockCmdSetOffset, x, y, z))->Send();
	}
}

void cyMisc::KillParticles(float time, float pct, pyKey& particles)
{
	plKey frKey = particles.getKey();
	plSceneObject* pObj = plSceneObject::ConvertNoRef(particles.getKey()->ObjectIsLoaded());
	if (!pObj)
		return;
	const plParticleSystem *sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));
	if (sys == nil) 
	{
		const plModifier* pArm = pObj->GetModifierByType(plArmatureMod::Index());
		if (pArm)
		{
			// it's the avatar
			const plArmatureMod* pCArm = (const plArmatureMod*)pArm;
			plArmatureMod* pNonConstArm = const_cast<plArmatureMod*>(pCArm);
			pObj = pNonConstArm->GetFollowerParticleSystemSO();
			if (!pObj)
				return;
			else
				sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));

		}
		if (sys == nil)
			return;
		
	}
	plParticleEffect *flock = sys->GetEffect(plParticleFlockEffect::Index());
	if (flock)
	{
		plParticleKillMsg* pMsg = TRACKED_NEW plParticleKillMsg(nil, frKey, 0, pct, time, plParticleKillMsg::kParticleKillPercentage | plParticleKillMsg::kParticleKillImmortalOnly);
		pMsg->SetBCastFlag(plMessage::kNetPropagate);
		pMsg->SetBCastFlag(plMessage::kNetForce);
		pMsg->SetBCastFlag(plMessage::kPropagateToChildren);
		pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		pMsg->Send();
	}
}

int cyMisc::GetNumParticles(pyKey& host)
{
	plSceneObject* pObj = plSceneObject::ConvertNoRef(host.getKey()->ObjectIsLoaded());
	if (!pObj)
		return 0;
	const plModifier* pArm = pObj->GetModifierByType(plArmatureMod::Index());
	if (pArm)
	{
		// it's the avatar
		const plArmatureMod* pCArm = (const plArmatureMod*)pArm;
		plArmatureMod* pNonConstArm = const_cast<plArmatureMod*>(pCArm);
		pObj = pNonConstArm->GetFollowerParticleSystemSO();
		if (!pObj)
			return 0;
	}
	const plModifier* pPart = pObj->GetModifierByType(plParticleSystem::Index());
	if (!pPart)
		return 0;
	return ((const plParticleSystem*)pPart)->GetNumValidParticles(true);
}


void cyMisc::SetLightColorValue(pyKey& light, std::string lightName, hsScalar r, hsScalar g, hsScalar b, hsScalar a)
{
	// lightName is the name of the light object attached to the light that we want to talk to
	// for the bug lights, this would be "RTOmni-BugLightTest"
	plSceneObject* pObj = plSceneObject::ConvertNoRef(light.getKey()->ObjectIsLoaded());
	if (!pObj)
		return;

	plObjInterface* pIface = pObj->GetGenericInterface(plLightInfo::Index());
	if (pIface)
	{
		// we are a light ourselves... are we the one they are looking for?
		if (lightName != pObj->GetKeyName())
			pIface = nil; // not the one they want, check our children
	}

	if (!pIface)
	{
		// recurse through our children...
		for (int i = 0; i < pObj->GetCoordinateInterface()->GetNumChildren(); i++)
		{
			const plSceneObject* child = pObj->GetCoordinateInterface()->GetChild(i)->GetOwner();
			if (child)
			{
				pIface = child->GetGenericInterface(plLightInfo::Index());
				if (pIface)
				{
					// found a child... is it the one we want?
					if (lightName != child->GetKeyName())
						pIface = nil; // not the child we want, keep looking
				}
			}
			if (pIface)
				break;
		}
	}
	if (pIface)
	{
		plLightInfo* pLight = (plLightInfo*)pIface;
		hsColorRGBA c;
		c.Set(r,g,b,a);
		pLight->SetDiffuse(c);
		pLight->SetDiffuse(c);
		pLight->SetSpecular(c);
	}
}

#include "../pnMessage/plEnableMsg.h"
void cyMisc::SetLightAnimationOn(pyKey& light, std::string lightName, hsBool start)
{
	// lightName is the name of the light object attached to the light that we want to talk to
	// for the bug lights, this would be "RTOmni-BugLightTest"
	plSceneObject* pObj = plSceneObject::ConvertNoRef(light.getKey()->ObjectIsLoaded());
	if (!pObj)
		return;

	plObjInterface* pIface = pObj->GetGenericInterface(plLightInfo::Index());
	if (pIface)
	{
		// we are a light ourselves... are we the one they are looking for?
		if (lightName != pObj->GetKeyName())
			pIface = nil; // not the one they want, check our children
	}

	if (!pIface)
	{
		// recurse through our children...
		for (int i = 0; i < pObj->GetCoordinateInterface()->GetNumChildren(); i++)
		{
			const plSceneObject* child = pObj->GetCoordinateInterface()->GetChild(i)->GetOwner();
			if (child)
			{
				pIface = child->GetGenericInterface(plLightInfo::Index());
				if (pIface)
				{
					// found a child... is it the one we want?
					if (lightName != child->GetKeyName())
						pIface = nil; // not the child we want, keep looking
				}
			}
			if (pIface)
				break;
		}
	}
	if (pIface)
	{
		plEnableMsg* enableMsg = TRACKED_NEW plEnableMsg;
		enableMsg->AddReceiver(pIface->GetKey());
		enableMsg->SetBCastFlag(plMessage::kNetPropagate);
		enableMsg->SetBCastFlag(plMessage::kNetForce);

		plAnimCmdMsg* animMsg = TRACKED_NEW plAnimCmdMsg;
		animMsg->AddReceiver(pIface->GetOwnerKey());
		animMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		animMsg->SetBCastFlag(plMessage::kNetPropagate);
		animMsg->SetBCastFlag(plMessage::kNetForce);

		if (start)
		{
			enableMsg->SetCmd(plEnableMsg::kEnable);
			animMsg->SetCmd(plAnimCmdMsg::kContinue);
		}
		else
		{
			enableMsg->SetCmd(plEnableMsg::kDisable);
			animMsg->SetCmd(plAnimCmdMsg::kStop);
			animMsg->SetCmd(plAnimCmdMsg::kGoToBegin);
		}

		enableMsg->Send();
		animMsg->Send();
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : RegisterForControlEventMessages
// PARAMETERS : switch on or off, registrant
//
// PURPOSE	  : let you get control event messages at will (for pseudo-GUI's like the psnl bookshelf or clft imager)

void cyMisc::RegisterForControlEventMessages(hsBool on, pyKey& k)
{
	plCmdIfaceModMsg* pMsg = TRACKED_NEW plCmdIfaceModMsg;
	pMsg->SetSender(k.getKey());
	if (on)
		pMsg->SetCmd(plCmdIfaceModMsg::kAdd);
	else
		pMsg->SetCmd(plCmdIfaceModMsg::kRemove);
	pMsg->SetBCastFlag(plMessage::kBCastByExactType);
	pMsg->Send();
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : RequestLOSScreen
// PARAMETERS : lots...
//
// PURPOSE    : To request an LOS from a point on the screen
//
#include "../plMessage/plLOSRequestMsg.h"
bool cyMisc::RequestLOSScreen(pyKey &selfkey, Int32 ID, hsScalar xPos, hsScalar yPos, hsScalar distance, int what, int reportType)
{
	plPipeline* pipe = selfkey.GetPipeline();
	if (pipe)
	{
		Int32 x=(Int32) ( xPos * pipe->Width() );
		Int32 y=(Int32) ( yPos * pipe->Height() );

		hsPoint3 endPos, startPos;
		
		pipe->ScreenToWorldPoint( 1,0, &x, &y, distance, 0, &endPos );
		startPos = pipe->GetViewPositionWorld();

		// move the start pos out a little to avoid backing up against physical objects...
		hsVector3 view(endPos - startPos);
		view.Normalize();
		startPos = startPos + (view * 1.5f);

		plLOSRequestMsg* pMsg = nil;
		switch (what)
		{
			case kClickables:
				pMsg = TRACKED_NEW plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBUIItems, plLOSRequestMsg::kTestClosest );
				pMsg->SetCullDB(plSimDefs::kLOSDBUIBlockers);
				break;
			case kCameraBlockers:
				pMsg = TRACKED_NEW plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBCameraBlockers, plLOSRequestMsg::kTestClosest );
				break;
			case kCustom:
				pMsg = TRACKED_NEW plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestClosest );
				break;
			case kShootable:
				pMsg = TRACKED_NEW plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBShootableItems, plLOSRequestMsg::kTestClosest );
				break;
		}

		if ( pMsg )
		{
			pMsg->SetReportType( (plLOSRequestMsg::ReportType)reportType );

			pMsg->SetRequestID( ID );

			plgDispatch::MsgSend( pMsg );
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : CheckVisLOS,CheckVisLOSFromCursor
// PARAMETERS : StartPoint, EndPoint
//
// PURPOSE    : Check is there is something visible in the path from StartPoint to EndPoint
//
#include "../plDrawable/plVisLOSMgr.h"
PyObject* cyMisc::CheckVisLOS(pyPoint3 startPoint, pyPoint3 endPoint)
{
	if (plVisLOSMgr::Instance())
	{
		plVisHit hit;
		if( plVisLOSMgr::Instance()->Check(startPoint.fPoint,endPoint.fPoint,hit) )
		{
			return pyPoint3::New(hit.fPos);
		}
	}
	PYTHON_RETURN_NONE;
}

PyObject* cyMisc::CheckVisLOSFromCursor()
{
	if (plVisLOSMgr::Instance())
	{
		plVisHit hit;
		if( plVisLOSMgr::Instance()->CursorCheck(hit) )
		{
			return pyPoint3::New(hit.fPos);
		}
	}
	PYTHON_RETURN_NONE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : IsSinglePlayerMode
// PARAMETERS :
//
// PURPOSE    : Returns whether the game is in Single Player mode
//
bool cyMisc::IsSinglePlayerMode()
{
	return false;
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : IsDemoMode
// PARAMETERS :
//
// PURPOSE    : Returns whether the game is in Single Player mode
//
bool cyMisc::IsDemoMode()
{
	plNetClientApp* nc = plNetClientApp::GetInstance();
	if (nc)
		return nc->InDemoMode();
	// if we couldn't find the net client app, maybe it was because we are single player mode
	return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : IsInternalRelease
// PARAMETERS :
//
// PURPOSE    : Returns true if we are running an internal build
//
bool cyMisc::IsInternalRelease()
{
#ifdef PLASMA_EXTERNAL_RELEASE
	return false;
#else
	return true;
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : IsEnterChatModeKeyBound
// PARAMETERS :
//
// PURPOSE    : Returns whether the EnterChatMode is bound to a key
//
bool cyMisc::IsEnterChatModeKeyBound()
{
	plAvatarInputInterface* aii = plAvatarInputInterface::GetInstance();
	if (aii)
		return aii->IsEnterChatModeBound();
	// if we couldn't find the net client app, maybe it was because we are single player mode
	return true;
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : ShootBulletFromScreen
// PARAMETERS : lots...
//
// PURPOSE    : Shoots from screen coordinates, a bullet and makes a mark on objects that know about bullet holes
//
#include "../plMessage/plBulletMsg.h"
void cyMisc::ShootBulletFromScreen(pyKey &selfkey, hsScalar xPos, hsScalar yPos, hsScalar radius, hsScalar range)
{
	plPipeline* pipe = selfkey.GetPipeline();
	if (pipe)
	{
		Int32 x=(Int32) ( xPos * pipe->Width() );
		Int32 y=(Int32) ( yPos * pipe->Height() );

		hsPoint3 endPos, startPos;
		
		pipe->ScreenToWorldPoint( 1,0, &x, &y, range, 0, &endPos );
		startPos = pipe->GetViewPositionWorld();

		// move the start pos out a little to avoid backing up against physical objects...
		hsVector3 view(endPos - startPos);
		view.Normalize();
		startPos = startPos + (view * 1.5f);

		plBulletMsg* bull = TRACKED_NEW plBulletMsg( selfkey.getKey(), nil, nil );
		bull->FireShot(startPos, view, radius, range);
		bull->Send();
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : ShootBulletFromObject
// PARAMETERS : lots...
//
// PURPOSE    : Shoots from an object, a bullet and makes a mark on objects that know about bullet holes
//
void cyMisc::ShootBulletFromObject(pyKey &selfkey, pySceneObject* sobj, hsScalar radius, hsScalar range)
{
	plSceneObject* so = plSceneObject::ConvertNoRef(sobj->getObjKey()->ObjectIsLoaded());
	if( so )
	{
		// find the direction

		hsMatrix44 l2w = so->GetLocalToWorld();
		hsVector3 dir(-l2w.fMap[0][0], -l2w.fMap[1][0], -l2w.fMap[2][0]);
		dir.Normalize();
		hsPoint3 pos = l2w.GetTranslate();

		plBulletMsg* bull = TRACKED_NEW plBulletMsg(selfkey.getKey(), nil, nil);
		bull->FireShot(pos, dir, radius, range);

		bull->Send();
	}
}


//////////////////////////////////////////////////////////////////////////////
class NetClientCommCallback : public plNetClientComm::Callback
{
public:
	enum Contexts
	{
		kInvalid,
		kGetPublicAgeList,
		kCreatePublicAge,
		kRemovePublicAge,
	};

	PyObject * fPyObject;
	NetClientCommCallback( PyObject * pyObject )
		: fPyObject( pyObject )
	{
		Py_XINCREF( fPyObject );
	}
	~NetClientCommCallback()
	{
		Py_XDECREF( fPyObject );
	}
	void OperationStarted( UInt32 context )
	{}
	void OperationComplete( UInt32 context, int resultCode )
	{
		if ( !fPyObject )
			return;



		PyObject* func = nil;

		switch ( context )
		{
		case kGetPublicAgeList:
			// Call the callback.
			func = PyObject_GetAttrString( fPyObject, "gotPublicAgeList" );
			if ( func )
			{
				if ( PyCallable_Check(func)>0 )
				{
					plCreatableStream * ageInfoStream = plCreatableStream::ConvertNoRef( fCbArgs.GetItem( 0 ) );
					plCreatableStream * nPlayersStream = plCreatableStream::ConvertNoRef( fCbArgs.GetItem( 1 ) );

					if ( ageInfoStream && nPlayersStream )
					{
						UInt16 nAgeInfoEntries;
						ageInfoStream->GetStream()->ReadSwap( &nAgeInfoEntries );

						UInt16 nPlayerCountEntries;
						nPlayersStream->GetStream()->ReadSwap( &nPlayerCountEntries );

						hsAssert( nAgeInfoEntries==nPlayerCountEntries, "huh?" );

						// convert callback args to a list of tuple(ageInfo,nPlayers)
						PyObject* pyEL = PyList_New(nAgeInfoEntries);

						for ( int i=0; i<nAgeInfoEntries; i++ )
						{
							plAgeInfoStruct ageInfo;
							UInt32 nPlayers;
							ageInfo.Read( ageInfoStream->GetStream(), nil );
							nPlayersStream->GetStream()->ReadSwap( &nPlayers );
							PyObject* t = PyTuple_New(2);
							PyTuple_SetItem(t, 0, pyAgeInfoStruct::New(&ageInfo));
							PyTuple_SetItem(t, 1, PyLong_FromUnsignedLong(nPlayers));
							PyList_SetItem(pyEL, i, t); // steals the ref
						}
						PyObject* retVal = PyObject_CallMethod(fPyObject, "gotPublicAgeList", "O", pyEL);
						Py_XDECREF(retVal);
					}
				}
			}
			break;
		case kCreatePublicAge:
			// Call the callback.
			func = PyObject_GetAttrString( fPyObject, "publicAgeCreated" );
			if ( func )
			{
				if ( PyCallable_Check(func)>0 )
				{
					plAgeInfoStruct * ageInfo = plAgeInfoStruct::ConvertNoRef( fCbArgs.GetItem( 0 ) );

					if ( ageInfo )
					{
						PyObject* ageInfoObj = pyAgeInfoStruct::New(ageInfo);
						PyObject* retVal = PyObject_CallMethod(fPyObject, "publicAgeCreated", "O", ageInfoObj);
						Py_XDECREF(retVal);
						Py_DECREF(ageInfoObj);
					}
				}
			}
			break;
		case kRemovePublicAge:
			// Call the callback.
			func = PyObject_GetAttrString( fPyObject, "publicAgeRemoved" );
			if ( func )
			{
				if ( PyCallable_Check(func)>0 )
				{
					plCreatableUuid * guid = plCreatableUuid::ConvertNoRef( fCbArgs.GetItem( 0 ) );

					if ( guid )
					{
						PyObject* retVal = PyObject_CallMethod(fPyObject, "publicAgeRemoved", "s", guid->AsString());
						Py_XDECREF(retVal);
					}
				}
			}
			break;
		}

		delete this;
	}
};


//////////////////////////////////////////////////////////////////////////////
//
// Function   : GetPublicAgeList
// PARAMETERS : ageName, callback object
//
// PURPOSE    : Get the list of public ages for the given age name.
//
void cyMisc::GetPublicAgeList( const char * ageName, PyObject * cbObject )
{
	if (cbObject)
		Py_XINCREF(cbObject);
    NetCommGetPublicAgeList(
        ageName,
        cbObject,
        plNetCommReplyMsg::kParamTypePython
    );
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : CreatePublicAge
// PARAMETERS : ageInfo, callback object
//
// PURPOSE    : Add a public age to the list of available ones.
//
void cyMisc::CreatePublicAge( pyAgeInfoStruct * ageInfo, PyObject * cbObject )
{
	VaultSetOwnedAgePublicAndWait(ageInfo->GetAgeInfo(), true);
	// TODO: make the callback here
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : RemovePublicAge
// PARAMETERS : ageInstanceGuid, callback object
//
// PURPOSE    : Remove a public age from the list of available ones.
//
void cyMisc::RemovePublicAge( const char * ageInstanceGuid, PyObject * cbObject/*=nil */)
{
	plAgeInfoStruct info;
	plUUID uuid(ageInstanceGuid);
	info.SetAgeInstanceGuid(&uuid);
	VaultSetOwnedAgePublicAndWait(&info, false);
	// TODO: make the callback here
}

int cyMisc::GetKILevel()
{
	int result = pfKIMsg::kNanoKI;
	
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, pfKIMsg::kChronicleKILevel, arrsize(wStr));
	if (RelVaultNode * rvn = VaultFindChronicleEntryIncRef(wStr)) {
		VaultChronicleNode chron(rvn);
		result = _wtoi(chron.entryValue);
		rvn->DecRef();
	}
	
	return result;
}

//////////////////////////////////////////////////////////////////////////////
//
// the following are for recording and rebuilding the camera stack

#include "../pfCamera/plCameraModifier.h"

int cyMisc::GetNumCameras()
{
	return (plVirtualCam1::Instance()->GetNumCameras());
}

const char* cyMisc::GetCameraNumber(int number)
{
	plCameraModifier1* pCam = plVirtualCam1::Instance()->GetCameraNumber(number-1);
	if (pCam->GetTarget())
	{
		const char* ret = pCam->GetTarget()->GetKeyName();
		(ret==nil) ? "empty" : ret;
		char str[256];
		sprintf(str, "saving camera named %s to chronicle\n",ret);
		plVirtualCam1::Instance()->AddMsgToLog(str);
		return ret;
	}
	plVirtualCam1::Instance()->AddMsgToLog("sending empty to camera chronicle\n");
	return "empty";
}

void cyMisc::RebuildCameraStack(const char* name, const char* ageName)
{
	plKey key=nil;
	char str[256];
	sprintf(str, "attempting to restore camera named %s from chronicle\n",name);
	plVirtualCam1::Instance()->AddMsgToLog(str);
		
	if (strcmp(name, "empty") == 0)
		return;

	if ( name || name[0] != 0)
	{
		key=plKeyFinder::Instance().StupidSearch(nil,nil,plSceneObject::Index(), name, false);
	}
	if ( key == nil )
	{
		// try and use this new hack method to find it
		if (!plVirtualCam1::Instance()->RestoreFromName(name))
		{
			// give up and force built in 3rd person
			plVirtualCam1::Instance()->PushThirdPerson();
			char errmsg[256];
			sprintf(errmsg,"Sceneobject %s not found",name);
			PyErr_SetString(PyExc_NameError, errmsg);
		}
	}
	else
	{
		// now we have the scene object, look for it's camera modifier
		const plCameraModifier1* pMod = nil;
		plSceneObject* pObj = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
		if (pObj)
		{
			for (int i = 1; i < pObj->GetNumModifiers(); i++)
			{
				pMod = plCameraModifier1::ConvertNoRef(pObj->GetModifier(i));
				if (pMod)
					break;
			}
			if (pMod)
			{	
				plVirtualCam1::Instance()->RebuildStack(pMod->GetKey());
				return;
			}
		}
		plVirtualCam1::Instance()->PushThirdPerson();
		char errmsg[256];
		sprintf(errmsg,"Sceneobject %s has no camera modifier",name);
		PyErr_SetString(PyExc_NameError, errmsg);
	}
	
}

void cyMisc::PyClearCameraStack()
{
	plVirtualCam1::Instance()->ClearStack();
}

void cyMisc::RecenterCamera()
{
	plCameraMsg* pCam = TRACKED_NEW plCameraMsg;
	pCam->SetBCastFlag(plMessage::kBCastByExactType);
	pCam->SetCmd(plCameraMsg::kResetPanning);
	pCam->Send();
}

#include "../plMessage/plTransitionMsg.h"

void cyMisc::FadeIn(float lenTime, bool holdFlag, bool noSound)
{
	plTransitionMsg	*msg = TRACKED_NEW plTransitionMsg( noSound ? plTransitionMsg::kFadeInNoSound : plTransitionMsg::kFadeIn, lenTime, holdFlag );
	plgDispatch::MsgSend( msg );
}

void cyMisc::FadeOut(float lenTime, bool holdFlag, bool noSound)
{
	plTransitionMsg	*msg = TRACKED_NEW plTransitionMsg( noSound ? plTransitionMsg::kFadeOutNoSound : plTransitionMsg::kFadeOut, lenTime, holdFlag );
	plgDispatch::MsgSend( msg );
}

void cyMisc::SetClickability(hsBool b)
{
	plInputIfaceMgrMsg* msg = TRACKED_NEW plInputIfaceMgrMsg(b ? plInputIfaceMgrMsg::kEnableClickables : plInputIfaceMgrMsg::kDisableClickables );
	plgDispatch::MsgSend(msg);
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Debug build only: Assert if condition is false.
//
// PURPOSE    : debugging
//
void cyMisc::DebugAssert( bool cond, const char * msg )
{
	hsAssert( cond, msg );
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Set a python object to be called back after a certain amount of time.
//
// PURPOSE    : script can trigger itself over time w/o having to specify it in the dataset.
//
void cyMisc::SetAlarm( float secs, PyObject * cb, UInt32 cbContext )
{
	pyAlarmMgr::GetInstance()->SetAlarm( secs, cb, cbContext );
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Save Screen Shot
//
// PURPOSE    : captures the screen and saves it as a jpeg
//
#include "../plJPEG/plJPEG.h"
void cyMisc::SaveScreenShot(const char* fileName, int x, int y, int quality)
{
	if ( cyMisc::GetPipeline() )
	{
		if (quality <= 0 || quality > 100)
			quality = 75;

		plMipmap mipmap;
		cyMisc::GetPipeline()->CaptureScreen( &mipmap, false, x, y );

		plJPEG::Instance().SetWriteQuality( quality );
		plJPEG::Instance().WriteToFile( fileName, &mipmap );
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Start a screen capture
//
// PURPOSE    : This starts a screen capture in motion. It will be capture on the next
//              update and a plCaptureRenderMsg when its ready
//
#include "../plPipeline/plCaptureRender.h"
void cyMisc::StartScreenCapture(pyKey& selfkey)
{
	cyMisc::StartScreenCaptureWH(selfkey, 800, 600);
}

void cyMisc::StartScreenCaptureWH(pyKey& selfkey, UInt16 width, UInt16 height)
{
	plCaptureRender::Capture(selfkey.getKey(), width, height);
}


#include "../plAvatar/plAvatarClothing.h"
void cyMisc::WearMaintainerSuit(pyKey& key, hsBool wear)
{
	// run on all machines, but only affects us if we call it on our local guy (who props it to others himself)
	if (key.getKey() != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
		return;

	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avMod)
	{
		if (wear)
			avMod->GetClothingOutfit()->WearMaintainerOutfit();
		else
			avMod->GetClothingOutfit()->RemoveMaintainerOutfit();
	}

}

void cyMisc::WearDefaultClothing(pyKey& key)
{
	if (key.getKey() != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
		return;

	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
	
	if (avMod)
	{
		avMod->GetClothingOutfit()->WearDefaultClothing();
	}
}

void cyMisc::WearDefaultClothingType(pyKey& key, UInt32 type)
{
	if (key.getKey() != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
		return;

	plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();

	if (avMod)
	{
		avMod->GetClothingOutfit()->WearDefaultClothingType(type);
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Fake link to object
//
// PURPOSE    : takes an avatar key and an object key and fake-links the avatar
//				to that object's position.  appears to be a link to other players
//

void cyMisc::FakeLinkToObject(pyKey& avatar, pyKey& object)
{
	plPseudoLinkEffectMsg* msg = TRACKED_NEW plPseudoLinkEffectMsg;
	msg->fAvatarKey = avatar.getKey();
	msg->fLinkObjKey = object.getKey();
	plgDispatch::MsgSend(msg);
}

void cyMisc::FakeLinkToObjectNamed(const char* name)
{
	plKey key = nil;
	if ( name || name[0] != 0)
	{
		key = plKeyFinder::Instance().StupidSearch(nil,nil,plSceneObject::Index(), name, false);
	}

	if (!key)
		return;
	plPseudoLinkEffectMsg* msg = TRACKED_NEW plPseudoLinkEffectMsg;
	msg->fAvatarKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	msg->fLinkObjKey = key;
	plgDispatch::MsgSend(msg);
}

PyObject* cyMisc::LoadAvatarModel(const char* modelName, pyKey& spawnPoint, const char* userStr)
{
	plKey SpawnedKey = plAvatarMgr::GetInstance()->LoadAvatar(modelName, "", false, spawnPoint.getKey(), nil, userStr);
	return pyKey::New(SpawnedKey);
}

void cyMisc::UnLoadAvatarModel(pyKey& avatar)
{
	plAvatarMgr::GetInstance()->UnLoadAvatar(avatar.getKey(), false);
}

void cyMisc::ForceCursorHidden()
{
	plMouseDevice::HideCursor(true);
}

void cyMisc::ForceCursorShown()
{
	plMouseDevice::ShowCursor(true);
}

///////////////////////////////////////////////////////////////////////////////
//
// Function	  : GetLocalizedString
//
// PURPOSE	  : Returns the specified localized string with the parameters
//				properly replaced (the list is a list of unicode strings) Name
//				is in "Age.Set.Name" format
//
std::wstring cyMisc::GetLocalizedString(std::wstring name, const std::vector<std::wstring> & arguments)
{
	if (pfLocalizationMgr::InstanceValid())
		return pfLocalizationMgr::Instance().GetString(name, arguments);
	return L"";
}

void cyMisc::EnablePlanarReflections(bool enable)
{
	plDynamicCamMap::SetEnabled(enable);
}

void cyMisc::GetSupportedDisplayModes(std::vector<plDisplayMode> *res)
{
	fPipeline->GetSupportedDisplayModes(res);
}

int cyMisc::GetDesktopWidth()
{
	return fPipeline->GetDesktopWidth();
}

int cyMisc::GetDesktopHeight()
{
	return fPipeline->GetDesktopHeight();
}

int cyMisc::GetDesktopColorDepth()
{
	return fPipeline->GetDesktopColorDepth();
}

PipelineParams *cyMisc::GetDefaultDisplayParams()
{
	return fPipeline->GetDefaultParams();
}

void cyMisc::SetGraphicsOptions(int Width, int Height, int ColorDepth, hsBool Windowed, int NumAASamples, int MaxAnisotropicSamples, hsBool VSync)
{
	// This has to send a message to plClient because python is loaded in the max plugins

	plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
	plClientMsg* clientMsg = TRACKED_NEW plClientMsg(plClientMsg::kResetGraphicsDevice);
	clientMsg->AddReceiver(clientKey);
	//clientMsg->SetBCastFlag(plMessage::kBCastByType);
	clientMsg->fGraphicsSettings.fWidth = Width;
	clientMsg->fGraphicsSettings.fHeight = Height;
	clientMsg->fGraphicsSettings.fColorDepth = ColorDepth;
	clientMsg->fGraphicsSettings.fWindowed = Windowed;
	clientMsg->fGraphicsSettings.fNumAASamples = NumAASamples;
	clientMsg->fGraphicsSettings.fMaxAnisoSamples = MaxAnisotropicSamples;
	clientMsg->fGraphicsSettings.fVSync = VSync;
	clientMsg->Send();

	//plClient::GetInstance()->ResetDisplayDevice(Width, Height, ColorDepth, Windowed, NumAASamples, MaxAnisotropicSamples);
}

bool cyMisc::DumpLogs(const std::wstring & folder)
{
	char* temp = hsWStringToString(folder.c_str());
	bool retVal = plStatusLogMgr::GetInstance().DumpLogs(temp);
	delete [] temp;
	return retVal;
}

bool cyMisc::FileExists(const std::wstring & filename)
{
	return PathDoesFileExist(filename.c_str());
}

bool cyMisc::CreateDir(const std::wstring & directory)
{
	return PathCreateDirectory(directory.c_str(), kPathCreateDirFlagEntireTree) == kPathCreateDirSuccess;
}

std::wstring cyMisc::GetUserPath()
{
	wchar_t path[MAX_PATH];
	PathGetUserDirectory(path, arrsize(path));
	return path;
}

std::wstring cyMisc::GetInitPath()
{
	wchar_t path[MAX_PATH];
	PathGetInitDirectory(path, arrsize(path));
	return path;
}

void cyMisc::SetBehaviorNetFlags(pyKey & behKey, hsBool netForce, hsBool netProp)
{
	if (plMultistageBehMod * behMod = plMultistageBehMod::ConvertNoRef(behKey.getKey()->ObjectIsLoaded()))
	{
		behMod->SetNetForce(netForce);
		behMod->SetNetProp(netProp);
	}
}

void cyMisc::SendFriendInvite(const wchar email[], const wchar toName[])
{
	if (RelVaultNode* pNode = VaultGetPlayerNodeIncRef())
	{
		VaultPlayerNode player(pNode);
		Uuid inviteUuid = player.inviteUuid;

		// If we don't have an invite UUID set then make a new one
		if (GuidIsNil(inviteUuid))
		{
			inviteUuid = GuidGenerate();
			player.SetInviteUuid(inviteUuid);
		}

		NetCommSendFriendInvite(email, toName, inviteUuid);
		pNode->DecRef();
	}
}

PyObject* cyMisc::PyGuidGenerate()
{
	char guidStr[64];
	Uuid newGuid = GuidGenerate();
	GuidToString(newGuid, guidStr, arrsize(guidStr));

	return PyString_FromString(guidStr);
}

PyObject* cyMisc::GetAIAvatarsByModelName(const char* name)
{
	plAvatarMgr::plArmatureModPtrVec armVec;
	plAvatarMgr::GetInstance()->FindAllAvatarsByModelName(name, armVec);

	PyObject* avList = PyList_New(0);

	for (plAvatarMgr::plArmatureModPtrVec::iterator it = armVec.begin(); it != armVec.end(); ++it)
	{
		plArmatureMod* armMod = (*it);
		plAvBrainCritter* critterBrain = plAvBrainCritter::ConvertNoRef(armMod->FindBrainByClass(plAvBrainCritter::Index()));
		if (critterBrain)
		{
			PyObject* tuple = PyTuple_New(2);
			PyTuple_SetItem(tuple, 0, pyCritterBrain::New(critterBrain));
			PyTuple_SetItem(tuple, 1, PyString_FromString(armMod->GetUserStr()));

			PyList_Append(avList, tuple);
			Py_DECREF(tuple);
		}
	}

	if (PyList_Size(avList) > 0)
		return avList;
	else
	{
		Py_DECREF(avList);
		PYTHON_RETURN_NONE;
	}
}

void cyMisc::ForceVaultNodeUpdate(unsigned nodeId)
{
	VaultFetchNodesAndWait(&nodeId, 1, true);
}

void cyMisc::VaultDownload(unsigned nodeId)
{
	VaultDownloadAndWait(
		L"PyVaultDownload",
		nodeId,
		nil,
		nil
	);
}
