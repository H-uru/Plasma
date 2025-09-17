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

#include "plNetServerSessionInfo.h"
#include "plNetCommon.h"

#include <utility>
#include <string_theory/string_stream>

#include "hsStream.h"

#include "pnMessage/plMessage.h"
#include "pnNetProtocol/pnNpCommon.h"

#define SAFE(s) ((s).empty() ? "(nil)" : (s))
#define kComma  ","
#define kEmpty  ""
#define kSemicolon  ";"


////////////////////////////////////////////////////////////////////

void plAgeInfoStruct::Read( hsStream * s, hsResMgr* )
{
    s->ReadByte(&fFlags);
    if (IsFlagSet(kHasAgeFilename))
        plMsgStdStringHelper::Peek(fAgeFilename,s);
    if (IsFlagSet(kHasAgeInstanceName))
        plMsgStdStringHelper::Peek(fAgeInstanceName,s);
    if (IsFlagSet(kHasAgeInstanceGuid))
        fAgeInstanceGuid.Read( s );
    if (IsFlagSet(kHasAgeUserDefinedName))
        plMsgStdStringHelper::Peek(fAgeUserDefinedName,s);
    if (IsFlagSet(kHasAgeSequenceNumber))
        s->ReadLE32(&fAgeSequenceNumber);
    if (IsFlagSet(kHasAgeDescription))
        plMsgStdStringHelper::Peek(fAgeDescription,s);
    if (IsFlagSet(kHasAgeLanguage))
        s->ReadLE32(&fAgeLanguage);
    UpdateFlags();
}

void plAgeInfoStruct::Write( hsStream * s, hsResMgr* )
{
    UpdateFlags();
    s->WriteByte(fFlags);
    if ( IsFlagSet( kHasAgeFilename ) )
        plMsgStdStringHelper::Poke(fAgeFilename,s);
    if ( IsFlagSet( kHasAgeInstanceName ) )
        plMsgStdStringHelper::Poke(fAgeInstanceName,s);
    if ( IsFlagSet( kHasAgeInstanceGuid ) )
        fAgeInstanceGuid.Write( s );
    if ( IsFlagSet( kHasAgeUserDefinedName ) )
        plMsgStdStringHelper::Poke(fAgeUserDefinedName,s);
    if ( IsFlagSet( kHasAgeSequenceNumber ) )
        s->WriteLE32(fAgeSequenceNumber);
    if ( IsFlagSet( kHasAgeDescription ) )
        plMsgStdStringHelper::Poke(fAgeDescription,s);
    if ( IsFlagSet( kHasAgeLanguage ) )
        s->WriteLE32(fAgeLanguage);
}

bool plAgeInfoStruct::IsEqualTo( const plAgeInfoStruct * other ) const
{
    UpdateFlags();
    other->UpdateFlags();

    // if we both have guids, just compare them.
    if ( HasAgeInstanceGuid() && other->HasAgeInstanceGuid() )
        return fAgeInstanceGuid.IsEqualTo( other->GetAgeInstanceGuid() );
    // otherwise compare everything.
    bool match = true;
    if (match && HasAgeFilename() && other->HasAgeFilename())
        match = match && ( GetAgeFilename().compare_i( other->GetAgeFilename() )==0 );
    if (match && HasAgeInstanceName() && other->HasAgeInstanceName())
        match = match && ( GetAgeInstanceName().compare_i( other->GetAgeInstanceName() )==0 );
    if (match && HasAgeUserDefinedName() && other->HasAgeUserDefinedName())
        match = match && ( GetAgeUserDefinedName().compare_i( other->GetAgeUserDefinedName() )==0 );
    if (match && HasAgeSequenceNumber() && other->HasAgeSequenceNumber())
        match = match && fAgeSequenceNumber==other->GetAgeSequenceNumber();
    if (match && HasAgeLanguage() && other->HasAgeLanguage())
        match = match && fAgeLanguage==other->GetAgeLanguage();
    // don't compare description fields
    return match;
}

void plAgeInfoStruct::CopyFrom(const plAgeInfoStruct * other)
{
    hsAssert(other, "CopyFrom called with null struct");

    fFlags = other->fFlags;
    fAgeFilename = other->fAgeFilename;
    fAgeInstanceName = other->fAgeInstanceName;
    fAgeInstanceGuid = other->fAgeInstanceGuid;
    fAgeUserDefinedName = other->fAgeUserDefinedName;
    fAgeDescription = other->fAgeDescription;
    fAgeSequenceNumber = other->fAgeSequenceNumber;
    fAgeLanguage = other->fAgeLanguage;
}

void plAgeInfoStruct::CopyFrom( const plVaultAgeInfoNode * node )
{
    hsAssert(false, "eric, port me");
    Clear();
/*
    if ( node )
    {
        SetAgeFilename( node->GetAgeFilename() );
        SetAgeInstanceName( node->GetAgeInstanceName() );
        SetAgeInstanceGuid( node->GetAgeInstanceGuid() );
        SetAgeUserDefinedName( node->GetAgeUserDefinedName() );
        SetAgeSequenceNumber( node->GetSequenceNumber() );
        SetAgeDescription( node->GetAgeDescription() );
        SetAgeLanguage( node->GetAgeLanguage() );
        UpdateFlags();
    }
*/
}

//============================================================================
void plAgeInfoStruct::CopyFrom(const NetAgeInfo & info) {
    // Filename
    SetAgeFilename(ST::string::from_utf16(info.ageFilename));
    // InstanceName
    SetAgeInstanceName(ST::string::from_utf16(info.ageInstName));
    // UserDefinedName
    SetAgeUserDefinedName(ST::string::from_utf16(info.ageUserName));
    // Description
    SetAgeDescription(ST::string::from_utf16(info.ageDesc));

    plUUID inst(info.ageInstId);
    SetAgeInstanceGuid(&inst);
    SetAgeSequenceNumber(info.ageSequenceNumber);
    SetAgeLanguage(info.ageLanguage);
}

//============================================================================
ST::string plAgeInfoStruct::AsString() const
{
    const char * spacer = kEmpty;

    ST::string_stream ss;

    ss << "[";

    if (HasAgeFilename())
    {
        ss  << spacer
            << "FName:"
            << SAFE(GetAgeFilename());
        spacer = kComma;
    }
    if (HasAgeInstanceName())
    {
        ss  << spacer
            << "IName:"
            << SAFE(GetAgeInstanceName());
        spacer = kComma;
    }
    if (HasAgeInstanceGuid())
    {
        ss  << spacer
            << "Guid:"
            << fAgeInstanceGuid.AsString();
        spacer = kComma;
    }
    if (HasAgeUserDefinedName())
    {
        ss  << spacer
            << "UName:"
            << SAFE(GetAgeUserDefinedName());
        spacer = kComma;
    }
    if (HasAgeSequenceNumber())
    {
        ss  << spacer
            << "Seq:"
            << GetAgeSequenceNumber();
        spacer = kComma;
    }
    if (HasAgeDescription())
    {
        ss  << spacer
            << "Desc:"
            << SAFE(GetAgeDescription());
        spacer = kComma;
    }
    if (HasAgeLanguage())
    {
        ss  << spacer
            << "Lang:"
            << GetAgeLanguage();
        spacer = kComma;
    }
    ss  << "]";

    return ss.to_string();
}


void plAgeInfoStruct::SetAgeFilename( const ST::string & v )
{
    if (!v.empty())
    {
        SetFlag( kHasAgeFilename );
        fAgeFilename=v;
    }
    else
    {
        ClearFlag( kHasAgeFilename );
    }
}

void plAgeInfoStruct::SetAgeInstanceName( const ST::string & v )
{
    if (!v.empty())
    {
        SetFlag( kHasAgeInstanceName );
        fAgeInstanceName=v;
    }
    else
    {
        ClearFlag( kHasAgeInstanceName );
    }
}

void plAgeInfoStruct::SetAgeInstanceGuid( const plUUID * v )
{
    if ( v )
    {
        SetFlag( kHasAgeInstanceGuid );
        fAgeInstanceGuid.CopyFrom( v );
    }
    else
    {
        ClearFlag( kHasAgeInstanceGuid );
        fAgeInstanceGuid.Clear();
    }
}

void plAgeInfoStruct::SetAgeUserDefinedName( const ST::string & v )
{
    if (!v.empty())
    {
        SetFlag( kHasAgeUserDefinedName );
        fAgeUserDefinedName=v;
    }
    else
    {
        ClearFlag( kHasAgeUserDefinedName );
    }
}

void plAgeInfoStruct::SetAgeSequenceNumber( uint32_t v )
{
    if ( v )
    {
        SetFlag( kHasAgeSequenceNumber );
        fAgeSequenceNumber=v;
    }
    else
    {
        ClearFlag( kHasAgeSequenceNumber );
    }
}

void plAgeInfoStruct::SetAgeDescription( const ST::string & v )
{
    if (!v.empty())
    {
        SetFlag( kHasAgeDescription );
        fAgeDescription=v;
    }
    else
    {
        ClearFlag( kHasAgeDescription );
    }
}

void plAgeInfoStruct::SetAgeLanguage( uint32_t v )
{
    if (v != static_cast<uint32_t>(-1))
    {
        SetFlag( kHasAgeLanguage );
        fAgeLanguage = v;
    }
    else
    {
        ClearFlag( kHasAgeLanguage );
    }
}

void plAgeInfoStruct::UpdateFlags() const
{
    SetFlag( kHasAgeFilename, !fAgeFilename.empty() );
    SetFlag( kHasAgeInstanceName, !fAgeInstanceName.empty() );
    SetFlag( kHasAgeUserDefinedName, !fAgeUserDefinedName.empty() );
    SetFlag( kHasAgeInstanceGuid, fAgeInstanceGuid.IsSet() );
    SetFlag( kHasAgeSequenceNumber, fAgeSequenceNumber!=0 );
    SetFlag( kHasAgeDescription, !fAgeDescription.empty() );
    SetFlag( kHasAgeLanguage, fAgeLanguage>=0 );
}

void plAgeInfoStruct::Clear()
{
    fFlags = 0;
    fAgeFilename = "";
    fAgeInstanceName = "";
    fAgeUserDefinedName = "";
    fAgeInstanceGuid.Clear();
    fAgeSequenceNumber = 0;
    fAgeDescription = "";
    fAgeLanguage = -1;
}


////////////////////////////////////////////////////////////////////

plAgeLinkStruct::plAgeLinkStruct()
: fFlags( kHasAgeInfo|kHasLinkingRules|kHasSpawnPt )
, fLinkingRules( plNetCommon::LinkingRules::kBasicLink )
, fSpawnPoint( kDefaultSpawnPoint )
, fAmCCR( 0 )
{
}

void plAgeLinkStruct::Read( hsStream * s, hsResMgr* m)
{
    s->ReadLE16(&fFlags);
    if (IsFlagSet(kHasAgeInfo))
        fAgeInfo.Read( s,m );
    if ( IsFlagSet( kHasLinkingRules ) )
        s->ReadByte(&fLinkingRules);
    if ( IsFlagSet( kHasSpawnPt_DEAD ) )
    {
        ST::string str;
        plMsgStdStringHelper::Peek(str,s);
        fSpawnPoint.SetName( str );
        if ( fSpawnPoint.GetName() == kDefaultSpawnPtName )
            fSpawnPoint.SetTitle( kDefaultSpawnPtTitle );
        else
            fSpawnPoint.SetTitle(std::move(str));
        ClearFlag( kHasSpawnPt_DEAD );
        SetFlag( kHasSpawnPt );
    }
    if ( IsFlagSet( kHasSpawnPt_DEAD2 ) )
    {
        fSpawnPoint.ReadOld( s );
        ClearFlag( kHasSpawnPt_DEAD2 );
        SetFlag( kHasSpawnPt );
    }
    else if ( IsFlagSet( kHasSpawnPt ) )
    {
        fSpawnPoint.Read( s );
    }
    if ( IsFlagSet( kHasAmCCR ) )
        s->ReadByte(&fAmCCR);

    if ( IsFlagSet( kHasParentAgeFilename ) )
        plMsgStdStringHelper::Peek(fParentAgeFilename,s);
}

void plAgeLinkStruct::Write( hsStream * s, hsResMgr* m)
{
    s->WriteLE16(fFlags);
    if ( IsFlagSet( kHasAgeInfo ) )
        fAgeInfo.Write( s,m );
    if ( IsFlagSet( kHasLinkingRules ) )
        s->WriteByte(fLinkingRules);
    if ( IsFlagSet( kHasSpawnPt ) )
        fSpawnPoint.Write( s );
    if ( IsFlagSet( kHasAmCCR ) )
        s->WriteByte(fAmCCR);
    if ( IsFlagSet( kHasParentAgeFilename ) )
        plMsgStdStringHelper::Poke(fParentAgeFilename,s);
}

void plAgeLinkStruct::SetParentAgeFilename(ST::string v)
{
    if (!v.empty())
    {
        SetFlag( kHasParentAgeFilename );
        fParentAgeFilename = std::move(v);
    }
    else
    {
        ClearFlag( kHasParentAgeFilename );
    }
}

void plAgeLinkStruct::CopyFrom( const plAgeLinkStruct * other )
{
    if ( other )
    {
        fFlags = other->fFlags;
        fAgeInfo.CopyFrom(&other->fAgeInfo);
        fLinkingRules = other->fLinkingRules;
        fSpawnPoint = other->fSpawnPoint;
        fAmCCR = other->fAmCCR;
        fParentAgeFilename = other->fParentAgeFilename;
    }
    else
    {
        Clear();
    }
}

void plAgeLinkStruct::CopyFrom( const plVaultAgeLinkNode * node )
{
    // don't clear ourselves, copy age info from node. leave spawn point alone.
    if ( node )
    {
        hsAssert(false, "eric, port me");
//      fAgeInfo.CopyFrom( node->GetAgeInfo() );
    }
    else
    {
        Clear();
    }
}

bool plAgeLinkStruct::IsEqualTo( const plAgeLinkStruct * other ) const
{
    bool match = true;
    if (match && IsFlagSet(kHasAgeInfo) && other->IsFlagSet(kHasAgeInfo))
        match = match && fAgeInfo.IsEqualTo( other->GetAgeInfo() );
    // don't compare linking rules.
    // don't compare SpawnPt
    // don't compare AmCCR
    return match;
}

void plAgeLinkStruct::Clear()
{
    fFlags = 0;
    fAgeInfo.Clear();
    fLinkingRules = plNetCommon::LinkingRules::kBasicLink;
    fSpawnPoint = kDefaultSpawnPoint;
    fAmCCR = false;
}

ST::string plAgeLinkStruct::AsString() const
{
    const char * spacer = kEmpty;

    ST::string_stream ss;

    ss << "[";

    if (HasAgeInfo())
    {
        ss  << spacer
            << "Nfo:"
            << fAgeInfo.AsString();
        spacer = kComma;
    }
    if (HasLinkingRules())
    {
        ss  << spacer
            << "Rule:"
            << plNetCommon::LinkingRules::LinkingRuleStr( GetLinkingRules() );
        spacer = kComma;
    }
    if (HasSpawnPt())
    {
        ss  << spacer
            << "Spwn:"
            << fSpawnPoint.AsString();
        spacer = kComma;
    }
    if (HasAmCCR())
    {
        ss  << spacer
            << "CCR:"
            << ( GetAmCCR()?"yes":"no" );
        spacer = kComma;
    }
    ss  << "]";

    return ss.to_string();
}

///////////////////////////////////////////////////////////////////
// End.
