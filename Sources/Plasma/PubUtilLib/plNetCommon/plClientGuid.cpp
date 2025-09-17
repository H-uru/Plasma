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

#include "plClientGuid.h"
#include "plNetCommon.h"

#include <string_theory/string_stream>

#include "hsStream.h"

#include "pnMessage/plMessage.h"

plClientGuid::plClientGuid()
:fPlayerID(0)
,fCCRLevel(0)
,fFlags(0)
,fProtectedLogin(false)
,fBuildType(plNetCommon::BuildType::kUnknown)
,fSrcAddr(0)
,fSrcPort(0)
,fReserved(false)
{
    fAccountUUID.Clear();
}

void plClientGuid::SetAccountUUID(const plUUID * v )
{
    fAccountUUID.CopyFrom( v );
    if ( !fAccountUUID.IsNull() )
        fFlags|=kAccountUUID;
    else
        fFlags&=~kAccountUUID;
}

void plClientGuid::SetAccountUUID(const plUUID & v )
{
    SetAccountUUID( &v );
}

void plClientGuid::SetBuildType(uint8_t type)
{
    fBuildType=type;
    fFlags|=kBuildType;
}

void plClientGuid::SetPlayerID(uint32_t id)
{
    fPlayerID=id;
    if ( fPlayerID )
    {
        fFlags|=kPlayerID;
        fFlags&=~kTempPlayerID;
    }
    else
        fFlags&=~kPlayerID;
}

void plClientGuid::SetTempPlayerID(uint32_t id)
{
    fPlayerID=id;
    if ( fPlayerID )
    {
        fFlags&=~kPlayerID;
        fFlags|=kTempPlayerID;
    }
    else
    {
        fFlags&=~kTempPlayerID;
    }
}

void plClientGuid::SetPlayerName(ST::string v)
{
    fPlayerName = std::move(v);
    if ( !fPlayerName.empty() )
        fFlags|=kPlayerName;
    else
        fFlags&=~kPlayerName;
}

void plClientGuid::SetCCRLevel(uint8_t v)
{
    fCCRLevel=v;
    fFlags|=kCCRLevel;
}

void plClientGuid::SetProtectedLogin(bool b)
{
    fProtectedLogin=b;
    fFlags |= kProtectedLogin;
}

void plClientGuid::SetSrcAddr( uint32_t v )
{
    fSrcAddr = v;
    if ( fSrcAddr )
        fFlags|=kSrcAddr;
    else
        fFlags&=~kSrcAddr;
}

void plClientGuid::SetSrcAddrFromStr( const char * s )
{
    hsAssert(false, "eric, port me");
}

void plClientGuid::SetSrcPort( uint16_t v )
{
    fSrcPort = v;
    if ( fSrcPort )
        fFlags|=kSrcPort;
    else
        fFlags&=~kSrcPort;
}

void plClientGuid::SetReserved(bool b)
{
    fReserved=b;
    fFlags |= kReserved;
}

void plClientGuid::SetClientKey(ST::string key)
{
    fClientKey = std::move(key);
    if ( !fClientKey.empty() )
        fFlags|=kClientKey;
    else
        fFlags&=~kClientKey;
}

const char * plClientGuid::GetSrcAddrStr() const
{
    hsAssert(false, "eric, port me");
    static const char foo[] = "";
    return foo;
}

ST::string plClientGuid::AsString() const
{
#define kComma  ","
#define kEmpty  ""
    const char * spacer = kEmpty;

    ST::string_stream ss;

    ss << "[";

    if (IsFlagSet(kPlayerID))
    {
        ss << spacer << "Pid:" << fPlayerID;
        spacer = kComma;
    }
    else if (IsFlagSet(kTempPlayerID))
    {
        ss << spacer << "tPd:" << fPlayerID;
        spacer = kComma;
    }
    if (IsFlagSet(kPlayerName))
    {
        ss << spacer << "Plr:" << fPlayerName;
    }
    if (IsFlagSet(kCCRLevel))
    {
        ss << spacer << "CCR:" << (int)fCCRLevel;
        spacer = kComma;
    }
    if (IsFlagSet(kProtectedLogin))
    {
        ss << spacer << "Pro:" << (int)fProtectedLogin;
        spacer = kComma;
    }
    if (IsFlagSet(kBuildType))
    {
        ss << spacer << "Bld:" << plNetCommon::BuildType::BuildTypeStr(fBuildType);
        spacer = kComma;
    }
    if ( IsFlagSet(kSrcAddr) )
    {
        ss << spacer << "Addr:" << GetSrcAddrStr();
        spacer = kComma;
    }
    if ( IsFlagSet(kSrcPort) )
    {
        ss << spacer << "Port:" << (int)fSrcPort;
        spacer = kComma;
    }
    if (IsFlagSet(kAccountUUID))
    {
        ss << spacer << "plUUID:" << fAccountUUID.AsString();
        spacer = kComma;
    }
    if ( IsFlagSet(kReserved))
    {
        ss << spacer << "Res:" << (int)fReserved;
        spacer = kComma;
    }
    if (IsFlagSet(kClientKey))
    {
        ss << spacer << "ClientKey:" << fClientKey;
        spacer = kComma;
    }
    ss  << "]";

    return ss.to_string();
}

ST::string plClientGuid::AsLogString() const
{
#define kSemicolon  ";"
    const char* spacer = kSemicolon;

    ST::string_stream ss;

    if (IsFlagSet(kAccountUUID))
    {
        ss << "AcctUUID=" << fAccountUUID.AsString();
        ss << spacer;
    }
    if (IsFlagSet(kPlayerID))
    {
        ss << "PlayerID=" << fPlayerID;
        ss << spacer;
    }
//  else if (IsFlagSet(kTempPlayerID))
//  {
//      ss << "tempPlayerID:" << fPlayerID;
//      ss << spacer;
//  }
    if ( IsFlagSet(kSrcAddr) )
    {
        ss << "SrcAddr=" << GetSrcAddrStr();
        ss << spacer;
    }
    if ( IsFlagSet(kSrcPort) )
    {
        ss << "SrcPort=" << (int)fSrcPort;
        ss << spacer;
    }
    if (IsFlagSet(kCCRLevel))
    {
        ss << "CCRLevel=" << (int)fCCRLevel;
        ss << spacer;
    }
    if (IsFlagSet(kProtectedLogin))
    {
        ss << "Protected=" << (int)fProtectedLogin;
        ss << spacer;
    }
    if (IsFlagSet(kBuildType))
    {
        ss << "Build=" << plNetCommon::BuildType::BuildTypeStr(fBuildType);
        ss << spacer;
    }
    if (IsFlagSet(kReserved))
    {
        ss << "Reserved=" << (int)fReserved;
        ss << spacer;
    }
    if (IsFlagSet(kClientKey))
    {
        ss << "ClientKey=" << fClientKey;
        ss << spacer;
    }

    return ss.to_string();
}

void plClientGuid::Read(hsStream * s, hsResMgr* mgr)
{
    s->ReadLE16(&fFlags);
    if (IsFlagSet(kAccountUUID))
        fAccountUUID.Read( s );
    if (IsFlagSet(kPlayerID))
        s->ReadLE32(&fPlayerID);
    else if (IsFlagSet(kTempPlayerID))
        s->ReadLE32(&fPlayerID);
    if (IsFlagSet(kPlayerName))
        plMsgStdStringHelper::Peek( fPlayerName, s );
    if (IsFlagSet(kCCRLevel))
        s->ReadByte(&fCCRLevel);
    if (IsFlagSet(kProtectedLogin))
        fProtectedLogin = s->ReadBool();
    if (IsFlagSet(kBuildType))
        s->ReadByte(&fBuildType);
    if (IsFlagSet(kSrcAddr))
        s->ReadLE32(&fSrcAddr);
    if (IsFlagSet(kSrcPort))
        s->ReadLE16(&fSrcPort);
    if (IsFlagSet(kReserved))
        fReserved = s->ReadBool();
    if (IsFlagSet(kClientKey))
        plMsgStdStringHelper::Peek( fClientKey, s );
}

void plClientGuid::Write(hsStream * s, hsResMgr* mgr)
{
    s->WriteLE16(fFlags);
    if (IsFlagSet(kAccountUUID))
        fAccountUUID.Write( s );
    if (IsFlagSet(kPlayerID))
        s->WriteLE32(fPlayerID);
    else if (IsFlagSet(kTempPlayerID))
        s->WriteLE32(fPlayerID);
    if (IsFlagSet(kPlayerName))
        plMsgStdStringHelper::Poke( fPlayerName, s );
    if (IsFlagSet(kCCRLevel))
        s->WriteByte(fCCRLevel);
    if (IsFlagSet(kProtectedLogin))
        s->WriteBool(fProtectedLogin);
    if (IsFlagSet(kBuildType))
        s->WriteByte(fBuildType);
    if (IsFlagSet(kSrcAddr))
        s->WriteLE32(fSrcAddr);
    if (IsFlagSet(kSrcPort))
        s->WriteLE16(fSrcPort);
    if (IsFlagSet(kReserved))
        s->WriteBool(fReserved);
    if (IsFlagSet(kClientKey))
        plMsgStdStringHelper::Poke( fClientKey, s );
}

void plClientGuid::CopyFrom(const plClientGuid * other)
{
    fFlags = other->fFlags;
    fAccountUUID.CopyFrom( &other->fAccountUUID );
    fPlayerID = other->fPlayerID;
    fPlayerName = other->fPlayerName;
    fCCRLevel = other->fCCRLevel;
    fProtectedLogin = other->fProtectedLogin;
    fBuildType = other->fBuildType;
    fSrcAddr = other->fSrcAddr;
    fSrcPort = other->fSrcPort;
    fReserved = other->fReserved;
    fClientKey = other->fClientKey;
}

void plClientGuid::UpdateFrom(const plClientGuid * other)
{
    if ( !HasAccountUUID() && other->HasAccountUUID() )
        SetAccountUUID( other->GetAccountUUID() );
    if ( !HasPlayerID() && other->HasPlayerID() )
        SetPlayerID( other->GetPlayerID() );
    if ( !HasPlayerName() && other->HasPlayerName() )
        SetPlayerName( other->GetPlayerName() );
    if ( !HasProtectedLogin() && other->HasProtectedLogin() )
        SetProtectedLogin( other->GetProtectedLogin() );
    if ( !HasCCRLevel() && other->HasCCRLevel() )
        SetCCRLevel( other->GetCCRLevel() );
    if ( !HasBuildType() && other->HasBuildType() )
        SetBuildType( other->GetBuildType() );
    if ( !HasSrcAddr() && other->HasSrcAddr() )
        SetSrcAddr( other->GetSrcAddr() );
    if ( !HasSrcPort() && other->HasSrcPort() )
        SetSrcPort( other->GetSrcPort() );
    if ( !HasReservedBit() && other->HasReservedBit() )
        SetReserved( other->IsReserved() );
    if ( !HasClientKey() && other->HasClientKey() )
        SetClientKey( other->GetClientKey() );
}

void plClientGuid::Clear()
{
    plClientGuid tmp;
    CopyFrom( &tmp );
}

bool plClientGuid::IsEqualTo(const plClientGuid * other) const
{
    return
        fFlags == other->fFlags &&
        fAccountUUID.IsEqualTo( &other->fAccountUUID ) &&
        fPlayerID == other->fPlayerID &&
        fPlayerName == other->fPlayerName &&
        fCCRLevel == other->fCCRLevel &&
        fProtectedLogin == other->fProtectedLogin &&
        fBuildType == other->fBuildType &&
        fReserved == other->fReserved &&
        fClientKey == other->fClientKey;
}

bool operator==(const plClientGuid & X, const plClientGuid & Y)
{
    return ( X.fAccountUUID.IsEqualTo( &Y.fAccountUUID )&&X.fPlayerID==Y.fPlayerID&&X.fFlags==Y.fFlags);
}
bool operator!=(const plClientGuid & X, const plClientGuid & Y)
{
    return ( !X.fAccountUUID.IsEqualTo( &Y.fAccountUUID )||X.fPlayerID!=Y.fPlayerID||X.fFlags!=Y.fFlags);
}
bool operator<(const plClientGuid & X, const plClientGuid & Y)
{
    return ( X.fAccountUUID.CompareTo( &Y.fAccountUUID )<0||X.fPlayerID<Y.fPlayerID);
}

