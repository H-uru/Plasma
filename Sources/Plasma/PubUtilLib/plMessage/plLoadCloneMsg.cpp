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

#include "plLoadCloneMsg.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "pnNetCommon/plNetApp.h"

// CTOR / default
plLoadCloneMsg::plLoadCloneMsg()
: fValidMsg(),
  fOriginatingPlayerID(),
  fTriggerMsg()
{
    SetBCastFlag(plMessage::kNetPropagate);
};

// CTOR uoidToClone, requestorKey, userData, isLoading
// this form is for creating new clones
plLoadCloneMsg::plLoadCloneMsg(const plUoid &uoidToClone, plKey requestorKey, uint32_t userData)
    : fRequestorKey(std::move(requestorKey)),
      fUserData(userData),
      fValidMsg(),
      fTriggerMsg(),
      fIsLoading(true)      // this constructor form is only used for loading
{
    SetBCastFlag(plMessage::kNetPropagate);
    
    hsKeyedObject * koRequestor = fRequestorKey->ObjectIsLoaded();
    if(koRequestor)
    {
        plNetClientApp *app = plNetClientApp::GetInstance();
        plKey originalKey = hsgResMgr::ResMgr()->FindKey(uoidToClone);

        if(originalKey)
        {
            // all is well. finish it off.
            fCloneKey = hsgResMgr::ResMgr()->CloneKey(originalKey);
            fOriginatingPlayerID = app->GetPlayerID();
            fValidMsg = true;
            this->AddReceiver(plNetApp::GetInstance()->GetKey());
        } else {
            char buffer[128];
            sprintf(buffer, "Can't find key named %s", uoidToClone.GetObjectName().c_str());
            hsAssert(0, buffer);
        }
    } else {
        hsStatusMessage("Clone requestor is not loaded.");
    }
}

// CTOR existing, requestor, userData, isLoading
// this form is for unloading or other operations on existing clones
plLoadCloneMsg::plLoadCloneMsg(plKey existing, plKey requestor, uint32_t userData, bool isLoading)
: fCloneKey(std::move(existing)),
  fRequestorKey(std::move(requestor)),
  fUserData(userData),
  fValidMsg(true),
  fTriggerMsg(),
  fIsLoading(isLoading)
{
    if (plNetApp::GetInstance())
    {
        SetBCastFlag(plMessage::kNetPropagate);
        AddReceiver(plNetApp::GetInstance()->GetKey());
    }
    if (plNetClientApp::GetInstance())
        fOriginatingPlayerID = plNetClientApp::GetInstance()->GetPlayerID();
    hsAssert(fRequestorKey->ObjectIsLoaded(), "Clone (unloading) requestor is not loaded.");
}

plLoadCloneMsg::~plLoadCloneMsg()
{
    hsRefCnt_SafeUnRef(fTriggerMsg);
}

// READ
void plLoadCloneMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);
    fCloneKey = mgr->ReadKey(stream);
    fRequestorKey = mgr->ReadKey(stream);
    fOriginatingPlayerID = stream->ReadLE32();
    fUserData = stream->ReadLE32();
    fValidMsg = stream->ReadBool();
    fIsLoading = stream->ReadBool();
    fTriggerMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
}

// WRITE
void plLoadCloneMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream,mgr);
    mgr->WriteKey(stream, fCloneKey);
    mgr->WriteKey(stream, fRequestorKey);
    stream->WriteLE32(fOriginatingPlayerID);
    stream->WriteLE32(fUserData);
    stream->WriteBool(fValidMsg);
    stream->WriteBool(fIsLoading);
    mgr->WriteCreatable(stream, fTriggerMsg);
}

enum LoadCloneMsgFlags
{
    kLoadCloneMsgCloneKey,
    kLoadCloneMsgRequestorKey,
    kLoadCloneMsgOrigPlayerID,
    kLoadCloneMsgUserData,
    kLoadCloneMsgValidMsg,
    kLoadCloneMsgIsLoading,
    kLoadCloneMsgTriggerMsg,
};

void plLoadCloneMsg::ReadVersion(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgReadVersion(stream, mgr);

    hsBitVector contentFlags;
    contentFlags.Read(stream);

    if (contentFlags.IsBitSet(kLoadCloneMsgCloneKey))
        fCloneKey = mgr->ReadKey(stream);

    if (contentFlags.IsBitSet(kLoadCloneMsgRequestorKey))
        fRequestorKey = mgr->ReadKey(stream);

    if (contentFlags.IsBitSet(kLoadCloneMsgOrigPlayerID))
        fOriginatingPlayerID = stream->ReadLE32();

    if (contentFlags.IsBitSet(kLoadCloneMsgUserData))
        fUserData = stream->ReadLE32();

    if (contentFlags.IsBitSet(kLoadCloneMsgValidMsg))
        fValidMsg = stream->ReadBool();

    if (contentFlags.IsBitSet(kLoadCloneMsgIsLoading))
        fIsLoading = stream->ReadBool();

    if (contentFlags.IsBitSet(kLoadCloneMsgTriggerMsg))
        fTriggerMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
}

void plLoadCloneMsg::WriteVersion(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWriteVersion(stream,mgr);

    hsBitVector contentFlags;
    contentFlags.SetBit(kLoadCloneMsgCloneKey);
    contentFlags.SetBit(kLoadCloneMsgRequestorKey);
    contentFlags.SetBit(kLoadCloneMsgOrigPlayerID);
    contentFlags.SetBit(kLoadCloneMsgUserData);
    contentFlags.SetBit(kLoadCloneMsgValidMsg);
    contentFlags.SetBit(kLoadCloneMsgIsLoading);
    contentFlags.SetBit(kLoadCloneMsgTriggerMsg);
    contentFlags.Write(stream);

    // kLoadCloneMsgCloneKey
    mgr->WriteKey(stream, fCloneKey);
    // kLoadCloneMsgRequestorKey
    mgr->WriteKey(stream, fRequestorKey);
    // kLoadCloneMsgOrigPlayerID
    stream->WriteLE32(fOriginatingPlayerID);
    // kLoadCloneMsgUserData
    stream->WriteLE32(fUserData);
    // kLoadCloneMsgValidMsg
    stream->WriteBool(fValidMsg);
    // kLoadCloneMsgIsLoading
    stream->WriteBool(fIsLoading);
    // kLoadCloneMsgTriggerMSg
    mgr->WriteCreatable(stream, fTriggerMsg);
}

plKey plLoadCloneMsg::GetCloneKey() const
{
    return fCloneKey;
}

plKey plLoadCloneMsg::GetRequestorKey() const
{
    return fRequestorKey;
}

bool plLoadCloneMsg::IsValidMessage() const
{
    return fValidMsg;
}

uint32_t plLoadCloneMsg::GetUserData() const
{
    return fUserData;
}

uint32_t plLoadCloneMsg::GetOriginatingPlayerID() const
{
    return fOriginatingPlayerID;
}

void plLoadCloneMsg::SetOriginatingPlayerID(uint32_t playerId)
{
    fOriginatingPlayerID = playerId;
}

bool plLoadCloneMsg::GetIsLoading() const
{
    return fIsLoading;
}

void plLoadCloneMsg::SetTriggerMsg(plMessage *msg)
{
    if (fTriggerMsg != nullptr)
        hsRefCnt_SafeUnRef(fTriggerMsg);

    hsRefCnt_SafeRef(msg);
    fTriggerMsg = msg;
}

plMessage *plLoadCloneMsg::GetTriggerMsg() const
{
    return fTriggerMsg;
}
