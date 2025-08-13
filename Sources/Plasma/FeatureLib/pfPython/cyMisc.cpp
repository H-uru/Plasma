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

#include "cyMisc.h"

#include <Python.h>
#include <utility>
#include <vector>

#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plTimerCallbackManager.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plKeyImp.h"
#include "pnMessage/plAttachMsg.h"
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plClientMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetClient/plNetLinkingMgr.h"
#include "pnNetCommon/pnNetCommon.h"
#include "plNetTransport/plNetTransport.h"
#include "plNetTransport/plNetTransportMember.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plAvBrainCritter.h"
#include "plAvatar/plMultistageBehMod.h"
#include "plGLight/plLightInfo.h"
#include "plInputCore/plAvatarInputInterface.h"
#include "plInputCore/plInputDevice.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plCCRMsg.h"
#include "plMessage/plExcludeRegionMsg.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plLoadCloneMsg.h"
#include "plMessage/plNetVoiceListMsg.h"
#include "plMessage/plTimerCallbackMsg.h"
#include "plPipeline/plDynamicEnvMap.h"
#include "plResMgr/plKeyFinder.h"
#include "plResMgr/plLocalization.h"
#include "plStatusLog/plStatusLog.h"
#include "plVault/plVault.h"

#include "pfCamera/plVirtualCamNeu.h"
#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "pfGameGUIMgr/pfGUIDialogMod.h"
#include "pfLocalizationMgr/pfLocalizationMgr.h"
#include "pfMessage/pfKIMsg.h"

#include "pyAgeInfoStruct.h"
#include "pyAlarm.h"
#include "pyColor.h"
#include "pyCritterBrain.h"
#include "pyGeometry3.h"
#include "pyGlueHelpers.h"
#include "pyGUIDialog.h"
#include "pyKey.h"
#include "pyPlayer.h"
#include "pySceneObject.h"

//// Static Class Stuff //////////////////////////////////////////////////////
plPipeline* cyMisc::fPipeline = nullptr;
uint32_t  cyMisc::fUniqueNumber = 0;

#ifdef PLASMA_EXTERNAL_RELEASE
uint32_t  cyMisc::fPythonLoggingLevel = cyMisc::kErrorLevel;
#else
uint32_t  cyMisc::fPythonLoggingLevel = cyMisc::kWarningLevel;
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
uint32_t cyMisc::GetPythonLoggingLevel()
{
    return fPythonLoggingLevel;
}
void cyMisc::SetPythonLoggingLevel(uint32_t new_level)
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
void cyMisc::Console(ST::string command)
{
    // create message to send to the console
    plControlEventMsg* pMsg = new plControlEventMsg;
    pMsg->SetBCastFlag(plMessage::kBCastByType);
    pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
    pMsg->SetControlActivated(true);
    pMsg->SetCmdString(std::move(command));
    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
}

void cyMisc::ConsoleNet(ST::string command, bool netForce)
{
    // create message to send to the console
    plControlEventMsg* pMsg = new plControlEventMsg;
    pMsg->SetBCastFlag(plMessage::kBCastByType);
    pMsg->SetBCastFlag(plMessage::kNetPropagate);
    if ( netForce )
        pMsg->SetBCastFlag(plMessage::kNetForce);
    pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
    pMsg->SetControlActivated(true);
    pMsg->SetCmdString(std::move(command));
    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : FindSceneObject
//  PARAMETERS : name   - string of name of the sceneobject
//             : ageName - string of the name of the age to look in
//
//  PURPOSE    : Execute a console command from a python script,
//                  optionally propagate over the net
//
PyObject* cyMisc::FindSceneObject(const ST::string& name, const ST::string& ageName)
{
    // assume that we won't find the sceneobject (key is equal to nil)
    plKey key;

    if ( !name.empty() )
    {
        key=plKeyFinder::Instance().StupidSearch(ageName, {}, plSceneObject::Index(), name, false);
    }

    if (key == nullptr)
    {
        ST::string errmsg = ST::format("Sceneobject {} not found", name);
        PyErr_SetString(PyExc_NameError, errmsg.c_str());
        return nullptr; // return nullptr cause we errored
    }
    return pySceneObject::New(key);
}

PyObject* cyMisc::FindSceneObjects(const ST::string& name)
{
    // assume that we won't find the sceneobject (key is equal to nil)
    std::vector<plKey> keys;

    if ( !name.empty() )
        plKeyFinder::Instance().ReallyStupidSubstringSearch(name, plSceneObject::Index(), keys);

    PyObject* result = PyList_New(keys.size());
    for (size_t i=0; i < keys.size(); i++)
        PyList_SET_ITEM(result, i, pySceneObject::New(keys[i]));

    return result;
}



PyObject* cyMisc::FindActivator(const ST::string& name)
{
    plKey key;
    if (!name.empty())
    {
        std::vector<plKey> keylist;
        plKeyFinder::Instance().ReallyStupidActivatorSearch(name, keylist);

        if (keylist.size() == 1)
            key = keylist[0];
    }

    if (key == nullptr)
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
void cyMisc::PopUpConsole(ST::string command)
{
    // create message to send to the console
    plControlEventMsg* pMsg1 = new plControlEventMsg;
    pMsg1->SetBCastFlag(plMessage::kBCastByType);
    pMsg1->SetControlCode(B_SET_CONSOLE_MODE);
    pMsg1->SetControlActivated(true);
    plgDispatch::MsgSend( pMsg1 );  // whoosh... off it goes
    // create message to send to the console
    plControlEventMsg* pMsg2 = new plControlEventMsg;
    pMsg2->SetBCastFlag(plMessage::kBCastByType);
    pMsg2->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
    pMsg2->SetControlActivated(true);
    pMsg2->SetCmdString(std::move(command));
    plgDispatch::MsgSend( pMsg2 );  // whoosh... off it goes
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TimerCallback
//  PARAMETERS : command   - string of console commmand to execute
//
//  PURPOSE    : Execute a console command from a python script
//
void cyMisc::TimerCallback(pyKey& selfkey, float time, int32_t id)
{
    // setup the message to come back to whoever the pyKey is pointing to
    plTimerCallbackMsg* pTimerMsg = new plTimerCallbackMsg(selfkey.getKey(),id);
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
void cyMisc::AttachObject(pyKey& ckey, pyKey& pkey, bool netForce)
{
    plKey childKey = ckey.getKey();
    plKey parentKey = pkey.getKey();

    // make sure that there was a child ket
    if ( childKey )
    {
        // create the attach message to attach the child
        plAttachMsg* pMsg = new plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRequest);
        
        if (netForce) {
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        
        plgDispatch::MsgSend( pMsg );
    }
}
void cyMisc::AttachObjectSO(pySceneObject& cobj, pySceneObject& pobj, bool netForce)
{
    plKey childKey = cobj.getObjKey();
    plKey parentKey = pobj.getObjKey();

    // make sure that there was a child ket
    if ( childKey )
    {
        // create the attach message to attach the child
        plAttachMsg* pMsg = new plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRequest);

        if (netForce) {
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }

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
void cyMisc::DetachObject(pyKey& ckey, pyKey& pkey, bool netForce)
{
    plKey childKey = ckey.getKey();
    plKey parentKey = pkey.getKey();

    // make sure that there was a child ket
    if ( childKey )
    {
        // create the attach message to detach the child
        plAttachMsg* pMsg = new plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRemove);

        if (netForce) {
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }

        plgDispatch::MsgSend( pMsg );
    }
}
void cyMisc::DetachObjectSO(pySceneObject& cobj, pySceneObject& pobj, bool netForce)
{
    plKey childKey = cobj.getObjKey();
    plKey parentKey = pobj.getObjKey();

    // make sure that there was a child ket
    if ( childKey )
    {
        // create the attach message to detach the child
        plAttachMsg* pMsg = new plAttachMsg(parentKey, childKey->GetObjectPtr(), plRefMsg::kOnRemove);

        if (netForce) {
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }

        plgDispatch::MsgSend( pMsg );
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetDirtySyncStateServer
//  PARAMETERS : 
//
//  PURPOSE    : set the Python modifier to be dirty and asked to be saved out
//
void cyMisc::SetDirtySyncState(pyKey &selfkey, const ST::string& SDLStateName, uint32_t sendFlags)
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
void cyMisc::SetDirtySyncStateWithClients(pyKey &selfkey, const ST::string& SDLStateName, uint32_t sendFlags)
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
//              key is provided.
//
bool cyMisc::WasLocallyNotified(pyKey &selfkey)
{
    return selfkey.WasLocalNotify();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClientName
//  PARAMETERS : avatar key
//
//  PURPOSE    : Return the net client (account) name of the player whose avatar
//              key is provided.
//
ST::string cyMisc::GetClientName(pyKey &avKey)
{
    return plNetClientMgr::GetInstance()->GetPlayerName(avKey.getKey());
}

PyObject* cyMisc::GetAvatarKeyFromClientID(int clientID)
{
    if (clientID == plNetClientMgr::GetInstance()->GetPlayerID())
    {
        return pyKey::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
    }
    else
    {
        std::vector<plNetTransportMember*> members = plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted();

        for (plNetTransportMember* mbr : members)
        {
            if (mbr != nullptr && mbr->GetAvatarKey() != nullptr && mbr->GetPlayerID() == clientID)
            {
                return pyKey::New(mbr->GetAvatarKey());
            }
        }
    }

    PYTHON_RETURN_NONE;
}


int cyMisc::GetClientIDFromAvatarKey(pyKey& avatar)
{
    if (plNetClientMgr::GetInstance()->GetLocalPlayerKey() == avatar.getKey())
    {
        return (plNetClientMgr::GetInstance()->GetPlayerID());
    }

    for (plNetTransportMember* mbr : plNetClientMgr::GetInstance()->TransportMgr().GetMemberList())
    {
        if (mbr != nullptr && mbr->GetAvatarKey() == avatar.getKey())
        {
            return mbr->GetPlayerID();
        }
    }

    return -1;
}

int cyMisc::GetLocalClientID()
{
    return (plNetClientMgr::GetInstance()->GetPlayerID());
}

bool cyMisc::ValidateKey(pyKey& key)
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
ST::string cyMisc::GetLocalClientName()
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

ST::string cyMisc::GetAgeName()
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
    PYTHON_RETURN_NONE; // return none, not nullptr (cause it isn't really an error... or is it?)
}


ST::string cyMisc::GetPrevAgeName()
{
    plNetLinkingMgr* nmgr = plNetLinkingMgr::GetInstance();
    if (nmgr)
    {
        plAgeLinkStruct* als = nmgr->GetPrevAgeLink();
        if (als)
            return als->GetAgeInfo()->GetAgeFilename();
    }
    return ST::string();
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
    PYTHON_RETURN_NONE; // return none, not nullptr (cause it isn't really an error... or is it?)
}

time_t cyMisc::GetDniTime()
{
    const plUnifiedTime utime = plNetClientMgr::GetInstance()->GetServerTime();
    if ( utime.GetSecs() != 0)
        return ConvertGMTtoDni(utime.GetSecs());
    else
        return 0;
}

time_t cyMisc::GetServerTime()
{
    const plUnifiedTime utime = plNetClientMgr::GetInstance()->GetServerTime();
    return utime.GetSecs();
}

float cyMisc::GetAgeTimeOfDayPercent()
{
    return plNetClientMgr::GetInstance()->GetCurrentAgeTimeOfDayPercent();
}

#define kMST (uint32_t)25200
#define kOneHour (uint32_t)3600
#define kOneDay (uint32_t)86400

time_t cyMisc::ConvertGMTtoDni(time_t gtime)
{
    // convert to mountain time
    time_t dtime = gtime - kMST;
    plUnifiedTime utime = plUnifiedTime();
    utime.SetSecs(dtime);
    // check for daylight savings time in New Mexico and adjust
    if ( utime.GetMonth() >= 3 && utime.GetMonth() <= 11 )
    {
        plUnifiedTime dstStart = plUnifiedTime();
        dstStart.SetGMTime(utime.GetYear(),3,8,2,0,0);
        // find first Sunday after (including) 3/8 (second Sunday of March)
        int days_to_go = 7 - dstStart.GetDayOfWeek();
        if (days_to_go == 7)
            days_to_go = 0;
        time_t dstStartSecs = dstStart.GetSecs() + days_to_go * kOneDay;

        plUnifiedTime dstEnd = plUnifiedTime();
        dstEnd.SetGMTime(utime.GetYear(),11,1,1,0,0);
        // find first sunday after (including) 11/1 (first Sunday of November)
        days_to_go = 7 - dstEnd.GetDayOfWeek();
        if (days_to_go == 7)
            days_to_go = 0;
        time_t dstEndSecs = dstEnd.GetSecs() + days_to_go * kOneDay;

        if ( dtime >= dstStartSecs && dtime < dstEndSecs )
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
void cyMisc::ExcludeRegionSet(pyKey& sender, pyKey& exKey, uint16_t state)
{
    plExcludeRegionMsg *msg = new plExcludeRegionMsg;

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
    plgDispatch::MsgSend( msg );    // whoosh... off it goes
}

void cyMisc::ExcludeRegionSetNow(pyKey& sender, pyKey& exKey, uint16_t state)
{
    plExcludeRegionMsg *msg = new plExcludeRegionMsg;

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
    msg->fSynchFlags |= plSynchedObject::kSendImmediately;
    plgDispatch::MsgSend( msg );    // whoosh... off it goes
}

void cyMisc::FlashWindow()
{
    plKey clientKey   = hsgResMgr::ResMgr()->FindKey(kClient_KEY);
    plClientMsg* pMsg = new plClientMsg(plClientMsg::kFlashWindow);
    pMsg->Send(clientKey);          // whoosh... off it goes
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
float cyMisc::GetDelSysSeconds()
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
void cyMisc::LoadDialog(const ST::string& name)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
    if ( mgr )
    {
        if ( !mgr->IsDialogLoaded(name) )
            mgr->LoadDialog( name );
    }
}

// Load dialog and set the GUINotifyMsg receiver key
void cyMisc::LoadDialogKA(const ST::string& name, pyKey& rKey, const ST::string& ageName)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
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
void cyMisc::UnloadDialog(const ST::string& name)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
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
bool cyMisc::IsDialogLoaded(const ST::string& name)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
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
void cyMisc::ShowDialog(const ST::string& name)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
    if ( mgr )
        mgr->ShowDialog(name);
}
void cyMisc::HideDialog(const ST::string& name)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
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
PyObject* cyMisc::GetDialogFromTagID(uint32_t tag)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
    if ( mgr )
    {
        // get the owner dialog modifier pointer
        pfGUIDialogMod* pdialog = mgr->GetDialogFromTag(tag);
        if ( pdialog )
            return pyGUIDialog::New(pdialog->GetKey());
    }

    ST::string errmsg = ST::format("GUIDialog TagID {} not found", tag);
    PyErr_SetString(PyExc_KeyError, errmsg.c_str());
    return nullptr; // return nullptr, cause we threw an error
}

PyObject* cyMisc::GetDialogFromString(const ST::string& name)
{
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
    if ( mgr )
    {
        // get the owner dialog modifier pointer
        pfGUIDialogMod* pdialog = mgr->GetDialogFromString(name);
        if ( pdialog )
            return pyGUIDialog::New(pdialog->GetKey());
    }

    ST::string errmsg = ST::format("GUIDialog {} not found", name);
    PyErr_SetString(PyExc_KeyError, errmsg.c_str());
    return nullptr; // return nullptr, cause we threw an error
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

    PyErr_SetString(PyExc_NameError, "Local avatar not found");
    return nullptr; // returns nullptr, cause we threw an error
}

PyObject* cyMisc::GetLocalPlayer()
{
    return pyPlayer::New(plNetClientMgr::GetInstance()->GetLocalPlayerKey(),
                         plNetClientMgr::GetInstance()->GetPlayerName(),
                         plNetClientMgr::GetInstance()->GetPlayerID(),
                         0.0 );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetNPC
//  PARAMETERS : npcID  - is the ID of an NPC
//
//  PURPOSE    : Returns a pySceneobject of an NPC
//
PyObject* cyMisc::GetNPC(int npcID)
{
    plSceneObject *so = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetNPC(npcID));
    if ( so )
        return pySceneObject::New(so->GetKey());

    PyErr_SetString(PyExc_NameError, "NPC not found");
    PYTHON_RETURN_ERROR;
}

PyObject* cyMisc::GetNPCCount()
{
    return PyLong_FromLong(plNetClientMgr::GetInstance()->NPCKeys().size());
}

#include "plPipeline.h"
#include "plGImage/plMipmap.h"

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IsSolo
//  PARAMETERS : 
//
//  PURPOSE    : Return whether we are the only player in the Age
//
bool cyMisc::IsSolo()
{
    plNetClientMgr* nc = plNetClientMgr::GetInstance();
    if (nc)
        return nc->TransportMgr().GetMemberList().empty();
    return true;
}

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

    pyPL.reserve(nc->TransportMgr().GetNumMembers());
    for (size_t i = 0; i < nc->TransportMgr().GetNumMembers(); i++)
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

    plNetClientMgr* nc = plNetClientMgr::GetInstance();

    if (!nc) // only ever really happens if they try to call us in max... I hope
        return pyPL;

    pyPL.reserve(nc->TransportMgr().GetNumMembers());

    // get the sorted member list from the Net transport manager
    std::vector<plNetTransportMember*> members = nc->TransportMgr().GetMemberListDistSorted();
    for (plNetTransportMember* mbr : members)
    {
        plKey avkey = mbr->GetAvatarKey();
        if (avkey)
        {
            // only non-ignored people in list and not in ignore list
            if (!VaultAmIgnoringPlayer(mbr->GetPlayerID()))
            {
                PyObject* playerObj = pyPlayer::New(avkey, mbr->GetPlayerName(), mbr->GetPlayerID(), mbr->GetDistSq());
                pyPlayer* player = pyPlayer::ConvertFrom(playerObj); // accesses internal pyPlayer object

                // modifies playerObj
                if (mbr->IsCCR())
                    player->SetCCRFlag(true);
                if (mbr->IsServer())
                    player->SetServerFlag(true);

                pyPL.push_back(playerObj);
            }
        }
    }
    return pyPL;
}

uint32_t cyMisc::GetMaxListenListSize()
{
    return plNetListenList::kMaxListenListSize;
}

float cyMisc::GetMaxListenDistSq()
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
uint32_t cyMisc::SendRTChat(const pyPlayer& from, const std::vector<pyPlayer*> & tolist, const ST::string& message, uint32_t flags)
{
    // create the messge that will contain the chat message
    pfKIMsg *msg = new pfKIMsg( pfKIMsg::kHACKChatMsg );
    msg->SetString( message );
    msg->SetUser( from.GetPlayerName(), from.GetPlayerID() );
    msg->SetFlags( flags );
    msg->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce);
    msg->SetBCastFlag(plMessage::kLocalPropagate, 0);

    // this goes to everybody on the shard
    if (flags & pfKIMsg::kGlobalMsg)
        msg->SetBCastFlag(plMessage::kCCRSendToAllPlayers);
    // allow inter-age routing of this msg
    if (flags & pfKIMsg::kInterAgeMsg)
        msg->SetBCastFlag( plMessage::kNetAllowInterAge );

    // add net rcvrs to msg
    for ( auto it = tolist.begin(); it != tolist.end(); ++it )
    {
        pyPlayer* to = *it;
        if ( !VaultAmIgnoringPlayer( to->GetPlayerID() ) )
            msg->AddNetReceiver(to->GetPlayerID());
    }

    msg->Send();
    return flags;
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
void cyMisc::SendKIMessage(uint32_t command, float value)
{
    // create the mesage to send
    pfKIMsg *msg = new pfKIMsg( (uint8_t)command );

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
void cyMisc::SendKIMessageS(uint32_t command, const ST::string& value)
{
    // create the mesage to send
    pfKIMsg *msg = new pfKIMsg( (uint8_t)command );

    msg->SetString(value);

    // send it off
    plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendKIMessageI
//  PARAMETERS : command   - the command type
//             : value     - extra value as an int32_t
//
//  PURPOSE    : Send message to the KI, to tell it things to do
//
//  RETURNS    : nothing
//
void  cyMisc::SendKIMessageI(uint32_t command, int32_t value)
{
    // create the mesage to send
    pfKIMsg *msg = new pfKIMsg( (uint8_t)command );

    msg->SetIntValue(value);

    // send it off
    plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SendKIMessageIReply
//  PARAMETERS : command   - the command type
//             : value     - extra value as an int32_t
//
//  PURPOSE    : Send message to the KI, to tell it things to do
//
//  RETURNS    : nothing
//
void  cyMisc::SendKIGZMarkerMsg(int32_t markerNumber, pyKey& sender)
{
    // create the mesage to send
    pfKIMsg *msg = new pfKIMsg( pfKIMsg::kGZInRange );

    msg->SetIntValue(markerNumber);
    msg->SetSender(sender.getKey());

    // send it off
    plgDispatch::MsgSend( msg );
}

void cyMisc::SendKIRegisterImagerMsg(const ST::string& imagerName, pyKey& sender)
{
    pfKIMsg *msg = new pfKIMsg(pfKIMsg::kRegisterImager);

    msg->SetString(imagerName);
    msg->SetSender(sender.getKey());

    plgDispatch::MsgSend( msg );
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
void cyMisc::RateIt(const ST::string& chronicleName, const ST::string& thestring, bool onceFlag)
{
    // create the mesage to send
    pfKIMsg *msg = new pfKIMsg( pfKIMsg::kRateIt );

    msg->SetUser(chronicleName,0);
    msg->SetString(thestring);
    msg->SetIntValue(onceFlag);
    // send it off
    plgDispatch::MsgSend( msg );
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetPrivateChatList
//  PARAMETERS : key            - who's joining
//
//  PURPOSE    : Lock the local avatar into private vox messaging, and / or add new memebers to his chat list
//
//  RETURNS    : nothing
//

void cyMisc::SetPrivateChatList(const std::vector<pyPlayer*> & tolist)
{
    if (tolist.size() > 0)
    {
        plNetVoiceListMsg* pMsg = new plNetVoiceListMsg(plNetVoiceListMsg::kForcedListenerMode);
        for (pyPlayer* toPlayer : tolist)
            pMsg->GetClientList().emplace_back(toPlayer->GetPlayerID());
        
        plKey k = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
        pMsg->SetRemovedKey(k);
        pMsg->Send();
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ClearPrivateChatList
//  PARAMETERS : key            - who's leaving
//
//  PURPOSE    : Remove the local avatar from private vox messaging, and / or clear memebers from his chat list
//
//  RETURNS    : nothing
//
void cyMisc::ClearPrivateChatList(pyKey& member)
{
    plNetVoiceListMsg* pMsg = new plNetVoiceListMsg(plNetVoiceListMsg::kDistanceMode);
    plKey k = member.getKey();
    pMsg->SetRemovedKey(k);
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
void cyMisc::SendPetitionToCCR(ST::string message, uint8_t reason, ST::string title)
{
    // create the mesage to send
    plCCRPetitionMsg *msg = new plCCRPetitionMsg();
    msg->SetNote(std::move(message));
    msg->SetType(reason);
    msg->SetTitle(std::move(title));
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
void cyMisc::SendChatToCCR(ST::string message, int32_t CCRPlayerID)
{
    // create the mesage to send
    plCCRCommunicationMsg *msg = new plCCRCommunicationMsg();
    msg->SetMessageText(std::move(message));
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

void cyMisc::PageInNodes(const std::vector<ST::string>& nodeNames, const ST::string& age, bool netForce)
{
    if (hsgResMgr::ResMgr())
    {
        plSynchEnabler ps(false);   // disable dirty tracking while paging in
        plClientMsg* msg = new plClientMsg(plClientMsg::kLoadRoom);
        plKey clientKey = hsgResMgr::ResMgr()->FindKey(kClient_KEY);
        msg->AddReceiver(clientKey);

        if (netForce) {
            msg->SetBCastFlag(plMessage::kNetPropagate);
            msg->SetBCastFlag(plMessage::kNetForce);
        }

        for (const auto& nodeName : nodeNames)
            msg->AddRoomLoc(plKeyFinder::Instance().FindLocation(!age.empty() ? age : NetCommGetAge()->ageDatasetName, nodeName));

        msg->Send();
    }
}

void cyMisc::PageOutNode(const ST::string& nodeName, bool netForce)
{
    if ( hsgResMgr::ResMgr() )
    {
        plSynchEnabler ps(false);   // disable dirty tracking while paging out
        plClientMsg* pMsg1 = new plClientMsg(plClientMsg::kUnloadRoom);
        plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
        pMsg1->AddReceiver( clientKey );
        pMsg1->AddRoomLoc(plKeyFinder::Instance().FindLocation("", nodeName));

        if (netForce) {
            pMsg1->SetBCastFlag(plMessage::kNetPropagate);
            pMsg1->SetBCastFlag(plMessage::kNetForce);
        }
        plgDispatch::MsgSend(pMsg1);
    }
}


#include "plAvatar/plArmatureMod.h"
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



#include "plPipeline/plFogEnvironment.h"

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
        hcolor.a = 1.0f;        // make sure that alpha is 1
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
    ST::string command = ST::format("Graphics.Renderer.SetClearColor {f} {f} {f}", red, green, blue);

    // create message to send to the console
    plControlEventMsg* pMsg = new plControlEventMsg;
    pMsg->SetBCastFlag(plMessage::kBCastByType);
    pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
    pMsg->SetControlActivated(true);
    pMsg->SetCmdString(command);
    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
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
        plIfaceFadeAvatarMsg* iMsg = new plIfaceFadeAvatarMsg;
        iMsg->SetSubjectKey(nmgr->GetLocalPlayerKey());
        iMsg->SetBCastFlag(plMessage::kBCastByExactType);
        iMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        iMsg->Enable();
        iMsg->Send();
    }
}

void cyMisc::DisableAvatarCursorFade()
{
    plNetClientMgr* nmgr = plNetClientMgr::GetInstance();
    if (nmgr)
    {
        plIfaceFadeAvatarMsg* iMsg = new plIfaceFadeAvatarMsg;
        iMsg->SetSubjectKey(nmgr->GetLocalPlayerKey());
        iMsg->SetBCastFlag(plMessage::kBCastByExactType);
        iMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        iMsg->Disable();
        iMsg->Send();
    }
}

void cyMisc::FadeLocalPlayer(bool fade)
{
    plNetClientMgr* nmgr = plNetClientMgr::GetInstance();
    if (nmgr)
    {
        plCameraTargetFadeMsg* pMsg = new plCameraTargetFadeMsg;
        pMsg->SetFadeOut(fade);
        pMsg->SetSubjectKey(nmgr->GetLocalPlayerKey());
        pMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
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

void cyMisc::EnableOfferBookMode(pyKey& selfkey, const ST::string& ageFilename, const ST::string& ageInstanceName)
{
    plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kSetOfferBookMode);
    pMsg->SetSender(selfkey.getKey());
    pMsg->SetAgeFileName(ageFilename);
    pMsg->SetAgeName(ageInstanceName);  
    pMsg->Send();
}

void cyMisc::DisableOfferBookMode()
{
    plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kClearOfferBookMode);
    pMsg->Send();
}

void cyMisc::NotifyOffererPublicLinkCompleted(uint32_t offerer)
{
    plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kNotifyOfferCompleted, plNetClientMgr::GetInstance()->GetPlayerID());
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

void cyMisc::NotifyOffererPublicLinkRejected(uint32_t offerer)
{
    plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kNotifyOfferRejected);
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

void cyMisc::NotifyOffererPublicLinkAccepted(uint32_t offerer)
{
    plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kNotifyOfferAccepted);
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

void cyMisc::ToggleAvatarClickability(bool on)
{
    plInputIfaceMgrMsg* pMsg = nullptr;
    if (on)
        pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIEnableAvatarClickable);
    else
        pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIDisableAvatarClickable);
    plKey k = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    pMsg->SetAvKey(k);
    pMsg->SetBCastFlag(plMessage::kNetPropagate);
    pMsg->SetBCastFlag(plMessage::kNetForce);
    pMsg->Send();

}

void cyMisc::SetShareSpawnPoint(const ST::string& spawnPoint)
{
    plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kSetShareSpawnPoint);
    plKey k = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    pMsg->SetSender(k);
    pMsg->SetSpawnPoint(spawnPoint);
    pMsg->Send();
}

void cyMisc::SetShareAgeInstanceGuid(const plUUID& guid)
{
    plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kSetShareAgeInstanceGuid);
    plKey k = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    pMsg->SetSender(k);
    pMsg->SetAgeInstanceGuid(guid);
    pMsg->Send();
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : GetCCRAwayStatus
// PARAMETERS :
//
// PURPOSE    : Returns current status of CCR dept
//
bool cyMisc::IsCCRAwayStatus()
{
    return !VaultGetCCRStatus();
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : AmCCR
// PARAMETERS :
//
// PURPOSE    : Returns true if local player is a CCR
//
bool cyMisc::AmCCR()
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
// PURPOSE    : Send's a VaultTask to the server to perform the invite
//
void cyMisc::AcceptInviteInGame(const ST::string& friendName, const ST::string& inviteKey)
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
//
//  particle system management
//
//
//
#include "plMessage/plParticleUpdateMsg.h"
#include "plParticleSystem/plParticleSystem.h"
#include "plParticleSystem/plParticleEffect.h"
void cyMisc::TransferParticlesToKey(pyKey& fromKey, pyKey& toKey, int numParticles)
{
    plKey frKey = fromKey.getKey();
    plSceneObject* so = plSceneObject::ConvertNoRef(toKey.getKey()->ObjectIsLoaded());
    if (so == nullptr)
        return;
    
    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    
    plParticleTransferMsg* pMsg = new plParticleTransferMsg(nullptr, avMod->GetKey(), nullptr, frKey, numParticles);
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
    if (sys == nullptr)
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
        if (sys == nullptr)
            return;
        
    }
    plParticleEffect *flock = sys->GetEffect(plParticleFlockEffect::Index());
    if (flock)
    {
        (new plParticleFlockMsg(nullptr, flock->GetKey(), nullptr, plParticleFlockMsg::kFlockCmdSetDissentPoint, x, y, z))->Send();
    }
}

void cyMisc::SetParticleOffset(float x, float y, float z, pyKey& particles)
{
    plKey frKey = particles.getKey();
    plSceneObject* pObj = plSceneObject::ConvertNoRef(particles.getKey()->ObjectIsLoaded());
    if (!pObj)
        return;
    const plParticleSystem *sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));
    if (sys == nullptr)
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
        if (sys == nullptr)
            return;
        
    }

    plParticleEffect *flock = sys->GetEffect(plParticleFlockEffect::Index());
    if (flock)
    {
        (new plParticleFlockMsg(nullptr, flock->GetKey(), nullptr, plParticleFlockMsg::kFlockCmdSetOffset, x, y, z))->Send();
    }
}

void cyMisc::KillParticles(float time, float pct, pyKey& particles)
{
    plKey frKey = particles.getKey();
    plSceneObject* pObj = plSceneObject::ConvertNoRef(particles.getKey()->ObjectIsLoaded());
    if (!pObj)
        return;
    const plParticleSystem *sys = plParticleSystem::ConvertNoRef(pObj->GetModifierByType(plParticleSystem::Index()));
    if (sys == nullptr)
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
        if (sys == nullptr)
            return;
        
    }
    plParticleEffect *flock = sys->GetEffect(plParticleFlockEffect::Index());
    if (flock)
    {
        plParticleKillMsg* pMsg = new plParticleKillMsg(nullptr, frKey, nullptr, pct, time,
                                            plParticleKillMsg::kParticleKillPercentage | plParticleKillMsg::kParticleKillImmortalOnly);
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


void cyMisc::SetLightColorValue(pyKey& light, const ST::string& lightName, float r, float g, float b, float a)
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
            pIface = nullptr; // not the one they want, check our children
    }

    if (!pIface)
    {
        // recurse through our children...
        for (size_t i = 0; i < pObj->GetCoordinateInterface()->GetNumChildren(); i++)
        {
            const plSceneObject* child = pObj->GetCoordinateInterface()->GetChild(i)->GetOwner();
            if (child)
            {
                pIface = child->GetGenericInterface(plLightInfo::Index());
                if (pIface)
                {
                    // found a child... is it the one we want?
                    if (lightName != child->GetKeyName())
                        pIface = nullptr; // not the child we want, keep looking
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

#include "pnMessage/plEnableMsg.h"
void cyMisc::SetLightAnimationOn(pyKey& light, const ST::string& lightName, bool start)
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
            pIface = nullptr; // not the one they want, check our children
    }

    if (!pIface)
    {
        // recurse through our children...
        for (size_t i = 0; i < pObj->GetCoordinateInterface()->GetNumChildren(); i++)
        {
            const plSceneObject* child = pObj->GetCoordinateInterface()->GetChild(i)->GetOwner();
            if (child)
            {
                pIface = child->GetGenericInterface(plLightInfo::Index());
                if (pIface)
                {
                    // found a child... is it the one we want?
                    if (lightName != child->GetKeyName())
                        pIface = nullptr; // not the child we want, keep looking
                }
            }
            if (pIface)
                break;
        }
    }
    if (pIface)
    {
        plEnableMsg* enableMsg = new plEnableMsg;
        enableMsg->AddReceiver(pIface->GetKey());
        enableMsg->SetBCastFlag(plMessage::kNetPropagate);
        enableMsg->SetBCastFlag(plMessage::kNetForce);

        plAnimCmdMsg* animMsg = new plAnimCmdMsg;
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
// PURPOSE    : let you get control event messages at will (for pseudo-GUI's like the psnl bookshelf or clft imager)

void cyMisc::RegisterForControlEventMessages(bool on, pyKey& k)
{
    plCmdIfaceModMsg* pMsg = new plCmdIfaceModMsg;
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
#include "plMessage/plLOSRequestMsg.h"
bool cyMisc::RequestLOSScreen(pyKey &selfkey, int32_t ID, float xPos, float yPos, float distance, int what, int reportType)
{
    plPipeline* pipe = selfkey.GetPipeline();
    if (pipe)
    {
        int32_t x=(int32_t) ( xPos * pipe->Width() );
        int32_t y=(int32_t) ( yPos * pipe->Height() );

        hsPoint3 endPos;
        
        pipe->ScreenToWorldPoint( 1,0, &x, &y, distance, 0, &endPos );
        hsPoint3 startPos = pipe->GetViewPositionWorld();

        // move the start pos out a little to avoid backing up against physical objects...
        hsVector3 view(endPos - startPos);
        view.Normalize();
        startPos = startPos + (view * 1.5f);

        plLOSRequestMsg* pMsg = nullptr;
        switch (what)
        {
            case kClickables:
                pMsg = new plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBUIItems, plLOSRequestMsg::kTestClosest );
                pMsg->SetRequestName(ST::format("Python [{}]: Clickables", selfkey.getName()));
                pMsg->SetCullDB(plSimDefs::kLOSDBUIBlockers);
                break;
            case kCameraBlockers:
                pMsg = new plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBCameraBlockers, plLOSRequestMsg::kTestClosest );
                pMsg->SetRequestName(ST::format("Python [{}]: Camera Blockers", selfkey.getName()));
                break;
            case kCustom:
                pMsg = new plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBCustom, plLOSRequestMsg::kTestClosest );
                pMsg->SetRequestName(ST::format("Python [{}]: Custom", selfkey.getName()));
                break;
            case kShootable:
                pMsg = new plLOSRequestMsg( selfkey.getKey(), startPos, endPos, plSimDefs::kLOSDBShootableItems, plLOSRequestMsg::kTestClosest );
                pMsg->SetRequestName(ST::format("Python [{}]: Shootables", selfkey.getName()));
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
#include "plDrawable/plVisLOSMgr.h"
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
#include "plMessage/plBulletMsg.h"
void cyMisc::ShootBulletFromScreen(pyKey &selfkey, float xPos, float yPos, float radius, float range)
{
    plPipeline* pipe = selfkey.GetPipeline();
    if (pipe)
    {
        int32_t x=(int32_t) ( xPos * pipe->Width() );
        int32_t y=(int32_t) ( yPos * pipe->Height() );

        hsPoint3 endPos;
        
        pipe->ScreenToWorldPoint( 1,0, &x, &y, range, 0, &endPos );
        hsPoint3 startPos = pipe->GetViewPositionWorld();

        // move the start pos out a little to avoid backing up against physical objects...
        hsVector3 view(endPos - startPos);
        view.Normalize();
        startPos = startPos + (view * 1.5f);

        plBulletMsg* bull = new plBulletMsg(selfkey.getKey(), nullptr, nullptr);
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
void cyMisc::ShootBulletFromObject(pyKey &selfkey, pySceneObject* sobj, float radius, float range)
{
    plSceneObject* so = plSceneObject::ConvertNoRef(sobj->getObjKey()->ObjectIsLoaded());
    if( so )
    {
        // find the direction

        hsMatrix44 l2w = so->GetLocalToWorld();
        hsVector3 dir(-l2w.fMap[0][0], -l2w.fMap[1][0], -l2w.fMap[2][0]);
        dir.Normalize();
        hsPoint3 pos = l2w.GetTranslate();

        plBulletMsg* bull = new plBulletMsg(selfkey.getKey(), nullptr, nullptr);
        bull->FireShot(pos, dir, radius, range);

        bull->Send();
    }
}


//////////////////////////////////////////////////////////////////////////////
//
// Function   : GetPublicAgeList
// PARAMETERS : ageName
//
// PURPOSE    : Get the list of public ages for the given age name.
//
void cyMisc::GetPublicAgeList(const ST::string& ageName)
{
    NetCommGetPublicAgeList(ageName, nullptr);
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : CreatePublicAge
// PARAMETERS : ageInfo
//
// PURPOSE    : Add a public age to the list of available ones.
//
void cyMisc::CreatePublicAge(pyAgeInfoStruct* ageInfo)
{
    VaultSetOwnedAgePublic(ageInfo->GetAgeInfo(), true);
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : RemovePublicAge
// PARAMETERS : ageInstanceGuid
//
// PURPOSE    : Remove a public age from the list of available ones.
//
void cyMisc::RemovePublicAge(const ST::string& ageInstanceGuid)
{
    plAgeInfoStruct info;
    plUUID uuid(ageInstanceGuid);
    info.SetAgeInstanceGuid(&uuid);
    VaultSetOwnedAgePublic(&info, false);
}

int cyMisc::GetKILevel()
{
    int result = pfKIMsg::kNanoKI;

    if (hsRef<RelVaultNode> rvn = VaultFindChronicleEntry(pfKIMsg::kChronicleKILevel)) {
        VaultChronicleNode chron(rvn);
        result = chron.GetEntryValue().to_int();
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////////
//
// the following are for recording and rebuilding the camera stack

#include "pfCamera/plCameraModifier.h"

int cyMisc::GetNumCameras()
{
    return (plVirtualCam1::Instance()->GetNumCameras());
}

ST::string cyMisc::GetCameraNumber(int number)
{
    plCameraModifier1* pCam = plVirtualCam1::Instance()->GetCameraNumber(number-1);
    if (pCam && pCam->GetTarget())
    {
        ST::string ret = pCam->GetTarget()->GetKeyName();
        ST::string log = ST::format("saving camera named {} to chronicle\n", ret);
        plVirtualCam1::Instance()->AddMsgToLog(log.c_str());
        return ret;
    }
    plVirtualCam1::Instance()->AddMsgToLog("sending empty to camera chronicle\n");
    return "empty";
}

void cyMisc::RebuildCameraStack(const ST::string& name, const ST::string& ageName)
{
    plKey key;
    ST::string str = ST::format("attempting to restore camera named {} from chronicle\n", name);
    plVirtualCam1::Instance()->AddMsgToLog(str.c_str());

    if (name.compare("empty") == 0)
        return;

    if ( !name.empty() )
    {
        key=plKeyFinder::Instance().StupidSearch("", "", plSceneObject::Index(), name, false);
    }
    if (key == nullptr)
    {
        // try and use this new hack method to find it
        if (!plVirtualCam1::Instance()->RestoreFromName(name))
        {
            // give up and force built in 3rd person
            plVirtualCam1::Instance()->PushThirdPerson();
            ST::string errmsg = ST::format("Sceneobject {} not found", name);
            PyErr_SetString(PyExc_NameError, errmsg.c_str());
        }
    }
    else
    {
        // now we have the scene object, look for it's camera modifier
        const plCameraModifier1* pMod = nullptr;
        plSceneObject* pObj = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
        if (pObj)
        {
            for (size_t i = 1; i < pObj->GetNumModifiers(); i++)
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
        ST::string errmsg = ST::format("Sceneobject {} has no camera modifier", name);
        PyErr_SetString(PyExc_NameError, errmsg.c_str());
    }
    
}

void cyMisc::PyClearCameraStack()
{
    plVirtualCam1::Instance()->ClearStack();
}

void cyMisc::RecenterCamera()
{
    plCameraMsg* pCam = new plCameraMsg;
    pCam->SetBCastFlag(plMessage::kBCastByExactType);
    pCam->SetCmd(plCameraMsg::kResetPanning);
    pCam->Send();
}

#include "plMessage/plTransitionMsg.h"

void cyMisc::FadeIn(float lenTime, bool holdFlag, bool noSound)
{
    plTransitionMsg *msg = new plTransitionMsg( noSound ? plTransitionMsg::kFadeInNoSound : plTransitionMsg::kFadeIn, lenTime, holdFlag );
    plgDispatch::MsgSend( msg );
}

void cyMisc::FadeOut(float lenTime, bool holdFlag, bool noSound)
{
    plTransitionMsg *msg = new plTransitionMsg( noSound ? plTransitionMsg::kFadeOutNoSound : plTransitionMsg::kFadeOut, lenTime, holdFlag );
    plgDispatch::MsgSend( msg );
}

void cyMisc::SetClickability(bool b)
{
    plInputIfaceMgrMsg* msg = new plInputIfaceMgrMsg(b ? plInputIfaceMgrMsg::kEnableClickables : plInputIfaceMgrMsg::kDisableClickables );
    plgDispatch::MsgSend(msg);
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Debug build only: Assert if condition is false.
//
// PURPOSE    : debugging
//
void cyMisc::DebugAssert(bool cond, const ST::string& msg)
{
    hsAssert(cond, msg.c_str());
}

void cyMisc::DebugPrint(const ST::string& msg, uint32_t level)
{
    if (level < fPythonLoggingLevel)
        return;
    plStatusLog* log = plStatusLogMgr::GetInstance().FindLog("python.log", false);
    if (!log)
        return;

    switch (level) {
    case kDebugDump:
        log->AddLine(plStatusLog::kGreen, msg);
        break;
    case kWarningLevel:
        log->AddLine(plStatusLog::kYellow, msg);
        break;
    case kAssertLevel:
        hsAssert(false, msg.c_str());
        // ... fall thru to the actual log-print
    case kErrorLevel:
        log->AddLine(plStatusLog::kRed, msg);
        break;
    default:
        log->AddLine(plStatusLog::kWhite, msg);
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Set a python object to be called back after a certain amount of time.
//
// PURPOSE    : script can trigger itself over time w/o having to specify it in the dataset.
//
void cyMisc::SetAlarm( float secs, PyObject * cb, uint32_t cbContext )
{
    pyAlarmMgr::GetInstance()->SetAlarm( secs, cb, cbContext );
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Save Screen Shot
//
// PURPOSE    : captures the screen and saves it as a jpeg
//
#include "plGImage/plJPEG.h"
void cyMisc::SaveScreenShot(const plFileName& fileName, int x, int y, int quality)
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
#include "plPipeline/plCaptureRender.h"
void cyMisc::StartScreenCapture(pyKey& selfkey)
{
    cyMisc::StartScreenCaptureWH(selfkey, 800, 600);
}

void cyMisc::StartScreenCaptureWH(pyKey& selfkey, uint16_t width, uint16_t height)
{
    plCaptureRender::Capture(selfkey.getKey(), width, height);
}


#include "plAvatar/plAvatarClothing.h"
void cyMisc::WearMaintainerSuit(pyKey& key, bool wear)
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

void cyMisc::WearDefaultClothing(pyKey& key, bool broadcast)
{
    if (key.getKey() != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
        return;

    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    
    if (avMod)
    {
        avMod->GetClothingOutfit()->WearDefaultClothing(broadcast);
    }
}

void cyMisc::WearDefaultClothingType(pyKey& key, uint32_t type, bool broadcast)
{
    if (key.getKey() != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
        return;

    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();

    if (avMod)
    {
        avMod->GetClothingOutfit()->WearDefaultClothingType(type, broadcast);
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Function   : Fake link to object
//
// PURPOSE    : takes an avatar key and an object key and fake-links the avatar
//              to that object's position.  appears to be a link to other players
//

void cyMisc::FakeLinkToObject(pyKey& avatar, pyKey& object)
{
    plPseudoLinkEffectMsg* msg = new plPseudoLinkEffectMsg;
    msg->fAvatarKey = avatar.getKey();
    msg->fLinkObjKey = object.getKey();
    plgDispatch::MsgSend(msg);
}

void cyMisc::FakeLinkToObjectNamed(const ST::string& name)
{
    plKey key;
    if ( !name.empty() )
    {
        key = plKeyFinder::Instance().StupidSearch("", "", plSceneObject::Index(), name, false);
    }

    if (!key)
        return;
    plPseudoLinkEffectMsg* msg = new plPseudoLinkEffectMsg;
    msg->fAvatarKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    msg->fLinkObjKey = key;
    plgDispatch::MsgSend(msg);
}

PyObject* cyMisc::LoadAvatarModel(ST::string modelName, pyKey& spawnPoint, const ST::string& userStr)
{
    plKey SpawnedKey = plAvatarMgr::GetInstance()->LoadAvatar(std::move(modelName), "", false, spawnPoint.getKey(), nullptr, userStr);
    return pyKey::New(SpawnedKey);
}

void cyMisc::UnLoadAvatarModel(pyKey& avatar)
{
    plAvatarMgr::GetInstance()->UnLoadAvatar(avatar.getKey(), false, true);
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
// Function   : GetLocalizedString
//
// PURPOSE    : Returns the specified localized string with the parameters
//              properly replaced (the list is a list of strings).
//              Name is in "Age.Set.Name" format
//
ST::string cyMisc::GetLocalizedString(const ST::string& name, const std::vector<ST::string> & arguments)
{
    if (pfLocalizationMgr::InstanceValid())
        return pfLocalizationMgr::Instance().GetString(name, arguments);
    return "";
}

void cyMisc::EnablePlanarReflections(bool enable)
{
    plDynamicCamMap::SetEnabled(enable);
}

bool cyMisc::ArePlanarReflectionsSupported()
{
    return plDynamicCamMap::GetCapable();
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

void cyMisc::SetGraphicsOptions(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool VSync)
{
    // This has to send a message to plClient because python is loaded in the max plugins

    plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
    plClientMsg* clientMsg = new plClientMsg(plClientMsg::kResetGraphicsDevice);
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

bool cyMisc::DumpLogs(const ST::string& folder)
{
    return plStatusLogMgr::GetInstance().DumpLogs(folder);
}

bool cyMisc::FileExists(const plFileName & filename)
{
    return plFileInfo(filename).Exists();
}

bool cyMisc::CreateDir(const plFileName & directory)
{
    return plFileSystem::CreateDir(directory, true);
}

plFileName cyMisc::GetUserPath()
{
    return plFileSystem::GetUserDataPath();
}

plFileName cyMisc::GetInitPath()
{
    return plFileSystem::GetInitPath();
}

void cyMisc::SetBehaviorNetFlags(pyKey & behKey, bool netForce, bool netProp)
{
    if (plMultistageBehMod * behMod = plMultistageBehMod::ConvertNoRef(behKey.getKey()->ObjectIsLoaded()))
    {
        behMod->SetNetForce(netForce);
        behMod->SetNetProp(netProp);
    }
}

void cyMisc::SendFriendInvite(const ST::string& email, const ST::string& toName)
{
    if (hsRef<RelVaultNode> pNode = VaultGetPlayerNode())
    {
        VaultPlayerNode player(pNode);
        plUUID inviteUuid = player.GetInviteUuid();

        // If we don't have an invite UUID set then make a new one
        if (inviteUuid.IsNull())
        {
            inviteUuid = plUUID::Generate();
            player.SetInviteUuid(inviteUuid);
        }

        NetCommSendFriendInvite(email, toName, inviteUuid);
    }
}

PyObject* cyMisc::PyGuidGenerate()
{
    plUUID newGuid = plUUID::Generate();

    return PyUnicode_FromSTString(newGuid.AsString());
}

PyObject* cyMisc::GetAIAvatarsByModelName(const ST::string& name)
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
            PyTuple_SetItem(tuple, 1, PyUnicode_FromSTString(armMod->GetUserStr()));

            PyList_Append(avList, tuple);
            Py_DECREF(tuple);
        }
    }
    return avList;
}

void cyMisc::ForceVaultNodeUpdate(unsigned nodeId)
{
    VaultFetchNodesAndWait(&nodeId, 1, true);
}

void cyMisc::VaultDownload(unsigned nodeId)
{
    VaultDownloadAndWait("PyVaultDownload", nodeId, nullptr);
}

PyObject* cyMisc::CloneKey(pyKey* object, bool loading) {

    plKey obj = object->getKey();
    plUoid uoid = obj->GetUoid();

    plLoadCloneMsg* cloneMsg;
    if (uoid.IsClone())
        cloneMsg = new plLoadCloneMsg(obj, plNetClientMgr::GetInstance()->GetKey(), 0, loading);
    else 
        cloneMsg = new plLoadCloneMsg(uoid, plNetClientMgr::GetInstance()->GetKey(), 0);

    cloneMsg->SetBCastFlag(plMessage::kNetPropagate);
    cloneMsg->SetBCastFlag(plMessage::kNetForce);
    cloneMsg->Send();

    return pyKey::New(cloneMsg->GetCloneKey());
}

PyObject* cyMisc::FindClones(pyKey* object) {
    plKey obj = object->getKey();
    plUoid uoid = obj->GetUoid();

    plKeyImp* imp = plKeyImp::GetFromKey(obj);
    size_t cloneNum = imp->GetNumClones();
    PyObject* keyList = PyList_New(cloneNum);

    for (size_t i = 0; i < cloneNum; i++) {
        PyObject* key = pyKey::New(imp->GetCloneByIdx(i));
        PyList_SET_ITEM(keyList, i, key);
    }

    return keyList;
}
