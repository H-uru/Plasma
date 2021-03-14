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

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plLinkToAgeMsg.h"
#include "plNetCommon/plNetServerSessionInfo.h"
#include "plNetCommon/plNetCommon.h"
#include "pnNetCommon/plNetApp.h"

/////////////////////////////////////////////////////////////////////////
//
// plLinkToAgeMsg

plLinkToAgeMsg::plLinkToAgeMsg() : fFlags(0)
{
}

plLinkToAgeMsg::plLinkToAgeMsg( const plAgeLinkStruct * link ) : fFlags(0)
{
    fAgeLink.CopyFrom( link );
}

void plLinkToAgeMsg::PlayLinkSfx(bool linkIn, bool linkOut)
{
    if (linkIn)
        fFlags &= ~kMuteLinkInSfx;
    else
        fFlags |= kMuteLinkInSfx;
    if (linkOut)
        fFlags &= ~kMuteLinkOutSfx;
    else
        fFlags |= kMuteLinkOutSfx;
}

void plLinkToAgeMsg::Read(hsStream* stream, hsResMgr* mgr)  
{
    plMessage::IMsgRead( stream, mgr );
    fFlags = stream->ReadByte();
    fAgeLink.Read( stream, mgr );
    fLinkInAnimName = stream->ReadSafeString();
}

void plLinkToAgeMsg::Write(hsStream* stream, hsResMgr* mgr) 
{
    plMessage::IMsgWrite( stream, mgr );
    stream->WriteByte( fFlags );
    fAgeLink.Write( stream, mgr );
    stream->WriteSafeString(fLinkInAnimName);
}

enum LinkToAgeFlags
{
    kLinkToAgeAgeLinkStruct,
    kLinkToAgeLinkAnimName,
};

void plLinkToAgeMsg::ReadVersion(hsStream* s, hsResMgr* mgr)    
{
    plMessage::IMsgReadVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.Read(s);

    if ( contentFlags.IsBitSet( kLinkToAgeAgeLinkStruct ) )
        fAgeLink.Read( s, mgr );
    if ( contentFlags.IsBitSet( kLinkToAgeLinkAnimName ) )
        fLinkInAnimName = s->ReadSafeString();
}

void plLinkToAgeMsg::WriteVersion(hsStream* s, hsResMgr* mgr)   
{
    plMessage::IMsgWriteVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.SetBit(kLinkToAgeAgeLinkStruct);
    contentFlags.SetBit(kLinkToAgeLinkAnimName);
    contentFlags.Write(s);

    // write kLinkToAgeAgeLinkStruct
    fAgeLink.Write( s, mgr );
    s->WriteSafeString(fLinkInAnimName);
}


/////////////////////////////////////////////////////////////////////////////
//
//  plLinkingMgrMsg

plLinkingMgrMsg::plLinkingMgrMsg()
: fLinkingMgrCmd( 0 /*plNetLinkingMgr::kNilCmd*/ )
{
}

plLinkingMgrMsg::~plLinkingMgrMsg()
{
}

enum LinkingMgrMsgFlags
{
    kLinkingMgrCmd,
    kLinkingMgrArgs,
};

void plLinkingMgrMsg::Read( hsStream* stream, hsResMgr* mgr )
{
    plMessage::IMsgRead( stream, mgr );

    hsBitVector contentFlags;
    contentFlags.Read( stream );

    if ( contentFlags.IsBitSet( kLinkingMgrCmd ) )
        stream->ReadByte(&fLinkingMgrCmd);
    if ( contentFlags.IsBitSet( kLinkingMgrArgs ) )
        fArgs.Read( stream, mgr );
}

void plLinkingMgrMsg::Write( hsStream* stream, hsResMgr* mgr )
{
    plMessage::IMsgWrite( stream, mgr );

    hsBitVector contentFlags;
    contentFlags.SetBit( kLinkingMgrCmd );
    contentFlags.SetBit( kLinkingMgrArgs );
    contentFlags.Write( stream );

    stream->WriteByte(fLinkingMgrCmd);
    fArgs.Write( stream, mgr );
}

void plLinkingMgrMsg::ReadVersion( hsStream* s, hsResMgr* mgr )
{
    // Read() does the contentFlags thing.
    Read( s, mgr );
}

void plLinkingMgrMsg::WriteVersion( hsStream* s, hsResMgr* mgr )
{
    // Write() does the contentFlags thing.
    Write( s, mgr );
}


/////////////////////////////////////////////////////////////////////////////
//
//  plLinkEffectsTriggerMsg

plLinkEffectsTriggerMsg::~plLinkEffectsTriggerMsg()
{
}

void plLinkEffectsTriggerMsg::MuteLinkSfx(bool mute)
{
    if (mute)
        fFlags |= kMuteLinkSfx;
    else
        fFlags &= ~kMuteLinkSfx;
}

void plLinkEffectsTriggerMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead( stream, mgr );
    
    fInvisLevel = stream->ReadLE32();
    fLeavingAge = stream->ReadBool();
    fLinkKey = mgr->ReadKey(stream);
    fFlags = stream->ReadLE32();
    fLinkInAnimKey = mgr->ReadKey(stream);
}

void plLinkEffectsTriggerMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite( stream, mgr );
    
    stream->WriteLE32(fInvisLevel);
    stream->WriteBool(fLeavingAge);
    mgr->WriteKey(stream, fLinkKey);
    stream->WriteLE32(fFlags);
    mgr->WriteKey(stream, fLinkInAnimKey);
}

enum LinkEffectsFlags
{
    kLinkEffectsLeavingAge,
    kLinkEffectsLinkKey,
    kLinkEffectsEffects,
    kLinkEffectsLinkInAnimKey,
};

void plLinkEffectsTriggerMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgReadVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kLinkEffectsLeavingAge))
        fLeavingAge = s->ReadBool();
    if (contentFlags.IsBitSet(kLinkEffectsLinkKey))
        fLinkKey = mgr->ReadKey(s);
    if (contentFlags.IsBitSet(kLinkEffectsEffects))
        fEffects = s->ReadLE32();
    if (contentFlags.IsBitSet(kLinkEffectsLinkInAnimKey))
        fLinkInAnimKey = mgr->ReadKey(s);

    if (plNetClientApp::GetInstance() && fLinkKey == plNetClientApp::GetInstance()->GetLocalPlayerKey())
    {
        SetBCastFlag(plMessage::kNetStartCascade, true);
        SetBCastFlag(plMessage::kNetNonLocal | plMessage::kNetPropagate, false);
    }
}

void plLinkEffectsTriggerMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgWriteVersion(s, mgr);
    
    hsBitVector contentFlags;
    contentFlags.SetBit(kLinkEffectsLeavingAge);
    contentFlags.SetBit(kLinkEffectsLinkKey);
    contentFlags.SetBit(kLinkEffectsEffects);
    contentFlags.SetBit(kLinkEffectsLinkInAnimKey);
    contentFlags.Write(s);

    // kLinkEffectsLeavingAge
    s->WriteBool(fLeavingAge);
    // kLinkEffectsLinkKey
    mgr->WriteKey(s, fLinkKey);
    // kLinkEffectsEffects
    s->WriteLE32(fEffects);
    // kLinkEffectsLinkInAnimKey
    mgr->WriteKey(s, fLinkInAnimKey);
}

void plLinkEffectsTriggerMsg::SetLinkKey(const plKey &key)
{
    fLinkKey = key; 
}

void plLinkEffectsTriggerMsg::SetLinkInAnimKey(plKey &key)
{
    fLinkInAnimKey = key;
}

/////////////////////////////////////////////////////////////////////////////
//
//  plLinkEffectsPrepMsg

plLinkEffectsTriggerPrepMsg::~plLinkEffectsTriggerPrepMsg()
{
    if (fTrigger)
        hsRefCnt_SafeUnRef(fTrigger);
}

void plLinkEffectsTriggerPrepMsg::SetTrigger(plLinkEffectsTriggerMsg *msg)
{
    if (fTrigger)
        hsRefCnt_SafeUnRef(fTrigger);
    
    hsRefCnt_SafeRef(msg);
    fTrigger = msg;
}

/////////////////////////////////////////////////////////////////////////////
//
// plLinkEffectBCMsg

plLinkEffectBCMsg::plLinkEffectBCMsg() : fLinkFlags() { SetBCastFlag(plMessage::kBCastByExactType); }

void plLinkEffectBCMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);

    fLinkKey = mgr->ReadKey(stream);
    fLinkFlags = stream->ReadLE32();
}

void plLinkEffectBCMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);

    mgr->WriteKey(stream, fLinkKey);
    stream->WriteLE32(fLinkFlags);
}

void plLinkEffectBCMsg::SetLinkFlag(uint32_t flag, bool on /* = true */)
{
    if (on)
        fLinkFlags |= flag;
    else
        fLinkFlags &= ~flag;
}

bool plLinkEffectBCMsg::HasLinkFlag(uint32_t flag)
{
    return fLinkFlags & flag;
}

/////////////////////////////////////////////////////////////////////////////
//
// plLinkEffectPrepBCMsg

plLinkEffectPrepBCMsg::plLinkEffectPrepBCMsg() : fLeavingAge() { SetBCastFlag(plMessage::kBCastByExactType); }

/////////////////////////////////////////////////////////////////////////////
//
// plLinkCallbacks

void plLinkCallbackMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plEventCallbackMsg::Read(stream, mgr);

    fLinkKey = mgr->ReadKey(stream);
}

void plLinkCallbackMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plEventCallbackMsg::Write(stream, mgr);

    mgr->WriteKey(stream, fLinkKey);
}

/////////////////////////////////////////////////////////////////////////////////////
////
//// plPseudoLinkEffectMsg

plPseudoLinkEffectMsg::plPseudoLinkEffectMsg()
{
    SetBCastFlag(plMessage::kNetPropagate | plMessage::kBCastByExactType);
}

void plPseudoLinkEffectMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);
    fLinkObjKey = mgr->ReadKey(stream);
    fAvatarKey = mgr->ReadKey(stream);
}

void plPseudoLinkEffectMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);
    mgr->WriteKey(stream, fLinkObjKey);
    mgr->WriteKey(stream, fAvatarKey);
}

/////////////////////////////////////////////////////////////////////////////////////
////
//// plPseudoLinkAnimTriggerMsg

plPseudoLinkAnimTriggerMsg::plPseudoLinkAnimTriggerMsg() : fForward(false)
{
    SetBCastFlag(plMessage::kBCastByExactType);
}


plPseudoLinkAnimTriggerMsg::plPseudoLinkAnimTriggerMsg(bool forward, plKey avatarKey)
{
    fForward = forward;
    fAvatarKey = avatarKey;
    SetBCastFlag(plMessage::kBCastByExactType);
}

void plPseudoLinkAnimTriggerMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream,mgr);
    fForward = stream->ReadBool();
    fAvatarKey = mgr->ReadKey(stream);
}

void plPseudoLinkAnimTriggerMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream,mgr);
    stream->WriteBool(fForward);
    mgr->WriteKey(stream, fAvatarKey);
}

/////////////////////////////////////////////////////////////////////////////////////
////
//// plPseudoLinkAnimCallbackMsg


void plPseudoLinkAnimCallbackMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream,mgr);
    mgr->WriteKey(stream, fAvatarKey);
}

void plPseudoLinkAnimCallbackMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream,mgr);
    mgr->WriteKey(stream, fAvatarKey);
}
