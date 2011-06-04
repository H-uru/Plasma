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
#include "hsStream.h"
#include "../pnMessage/plMessage.h"
#include "plNetServerSessionInfo.h"
#include "hsStlUtils.h"
#include "plNetCommon.h"
#include "../plVault/plVault.h"

#include <sstream>

#define SAFE(s)	((s)?(s):"(nil)")
#define kComma  ","
#define kEmpty	""
#define kSemicolon	";"


////////////////////////////////////////////////////////////////////

void plAgeInfoStruct::Read( hsStream * s, hsResMgr* )
{
	s->LogSubStreamStart("push me");
	s->LogReadSwap( &fFlags ,"AgeInfoStruct Flags");
	if ( IsFlagSet( kHasAgeFilename ) ) {
		s->LogSubStreamPushDesc("AgeFilename");
		plMsgStdStringHelper::Peek(fAgeFilename,s);
	}
	if ( IsFlagSet( kHasAgeInstanceName ) ) {
		s->LogSubStreamPushDesc("AgeInstanceName");
		plMsgStdStringHelper::Peek(fAgeInstanceName,s);
	}
	if ( IsFlagSet( kHasAgeInstanceGuid ) ) {
		s->LogSubStreamPushDesc("AgeInstanceGuid");
		fAgeInstanceGuid.Read( s );
	}
	if ( IsFlagSet( kHasAgeUserDefinedName ) ){
		s->LogSubStreamPushDesc("UserDefinedName");
		plMsgStdStringHelper::Peek(fAgeUserDefinedName,s);
	}
	if ( IsFlagSet( kHasAgeSequenceNumber ) ) {
		s->LogReadSwap( &fAgeSequenceNumber ,"AgeSequenceNumber");
	}
	if ( IsFlagSet( kHasAgeDescription ) ) {
		s->LogSubStreamPushDesc("AgeDescription");
		plMsgStdStringHelper::Peek(fAgeDescription,s);
	}
	if ( IsFlagSet( kHasAgeLanguage ) ) {
		s->LogReadSwap( &fAgeLanguage ,"AgeLanguage");
	}
	UpdateFlags();
	s->LogSubStreamEnd();
}

void plAgeInfoStruct::Write( hsStream * s, hsResMgr* )
{
	UpdateFlags();
	s->WriteSwap( fFlags );
	if ( IsFlagSet( kHasAgeFilename ) )
		plMsgStdStringHelper::Poke(fAgeFilename,s);
	if ( IsFlagSet( kHasAgeInstanceName ) )
		plMsgStdStringHelper::Poke(fAgeInstanceName,s);
	if ( IsFlagSet( kHasAgeInstanceGuid ) )
		fAgeInstanceGuid.Write( s );
	if ( IsFlagSet( kHasAgeUserDefinedName ) )
		plMsgStdStringHelper::Poke(fAgeUserDefinedName,s);
	if ( IsFlagSet( kHasAgeSequenceNumber ) )
		s->WriteSwap( fAgeSequenceNumber );
	if ( IsFlagSet( kHasAgeDescription ) )
		plMsgStdStringHelper::Poke(fAgeDescription,s);
	if ( IsFlagSet( kHasAgeLanguage ) )
		s->WriteSwap( fAgeLanguage );
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
		match = match && ( stricmp( GetAgeFilename(), other->GetAgeFilename() )==0 );
	if (match && HasAgeInstanceName() && other->HasAgeInstanceName())
		match = match && ( stricmp( GetAgeInstanceName(), other->GetAgeInstanceName() )==0 );
	if (match && HasAgeUserDefinedName() && other->HasAgeUserDefinedName())
		match = match && ( stricmp( GetAgeUserDefinedName(), other->GetAgeUserDefinedName() )==0 );
	if (match && HasAgeSequenceNumber() && other->HasAgeSequenceNumber())
		match = match && fAgeSequenceNumber==other->GetAgeSequenceNumber();
	if (match && HasAgeLanguage() && other->HasAgeLanguage())
		match = match && fAgeLanguage==other->GetAgeLanguage();
	// don't compare description fields
	return match;
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
	char tmp[MAX_PATH];

	// Filename	
	StrToAnsi(tmp, info.ageFilename, arrsize(tmp));
	SetAgeFilename(tmp);
	// InstanceName	
	StrToAnsi(tmp, info.ageInstName, arrsize(tmp));
	SetAgeInstanceName(tmp);
	// UserDefinedName	
	StrToAnsi(tmp, info.ageUserName, arrsize(tmp));
	SetAgeUserDefinedName(tmp);
	// Description	
	StrToAnsi(tmp, info.ageDesc, arrsize(tmp));
	SetAgeDescription(tmp);

	SetAgeInstanceGuid(&plUUID(info.ageInstId));
	SetAgeSequenceNumber(info.ageSequenceNumber);
	SetAgeLanguage(info.ageLanguage);
}

//============================================================================
std::string plAgeInfoStruct::AsStdString() const
{
	const char * spacer = kEmpty;

	std::stringstream ss;

	ss << "[";

	if (HasAgeFilename())
	{
		ss	<< spacer
			<< "FName:"
			<< SAFE(GetAgeFilename());
		spacer = kComma;
	}
	if (HasAgeInstanceName())
	{
		ss	<< spacer
			<< "IName:"
			<< SAFE(GetAgeInstanceName());
		spacer = kComma;
	}
	if (HasAgeInstanceGuid())
	{
		ss	<< spacer
			<< "Guid:"
			<< fAgeInstanceGuid.AsString();
		spacer = kComma;
	}
	if (HasAgeUserDefinedName())
	{
		ss	<< spacer
			<< "UName:"
			<< SAFE(GetAgeUserDefinedName());
		spacer = kComma;
	}
	if (HasAgeSequenceNumber())
	{
		ss	<< spacer
			<< "Seq:"
			<< GetAgeSequenceNumber();
		spacer = kComma;
	}
	if (HasAgeDescription())
	{
		ss	<< spacer
			<< "Desc:"
			<< SAFE(GetAgeDescription());
		spacer = kComma;
	}
	if (HasAgeLanguage())
	{
		ss	<< spacer
			<< "Lang:"
			<< GetAgeLanguage();
		spacer = kComma;
	}
	ss	<< "]";

	return ss.str().c_str();
}


void plAgeInfoStruct::SetAgeFilename( const char * v )
{
	if ( v && v[0])
	{
		SetFlag( kHasAgeFilename );
		fAgeFilename=v;
	}
	else
	{
		ClearFlag( kHasAgeFilename );
	}
}

void plAgeInfoStruct::SetAgeInstanceName( const char * v )
{
	if ( v && v[0])
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

void plAgeInfoStruct::SetAgeUserDefinedName( const char * v )
{
	if ( v && v[0])
	{
		SetFlag( kHasAgeUserDefinedName );
		fAgeUserDefinedName=v;
	}
	else
	{
		ClearFlag( kHasAgeUserDefinedName );
	}
}

void plAgeInfoStruct::SetAgeSequenceNumber( UInt32 v )
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

void plAgeInfoStruct::SetAgeDescription( const char * v )
{
	if ( v && v[0])
	{
		SetFlag( kHasAgeDescription );
		fAgeDescription=v;
	}
	else
	{
		ClearFlag( kHasAgeDescription );
	}
}

void plAgeInfoStruct::SetAgeLanguage( UInt32 v )
{
	if ( v >= 0 )
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
	SetFlag( kHasAgeFilename, fAgeFilename.size()!=0 );
	SetFlag( kHasAgeInstanceName, fAgeInstanceName.size()!=0 );
	SetFlag( kHasAgeUserDefinedName, fAgeUserDefinedName.size()!=0 );
	SetFlag( kHasAgeInstanceGuid, fAgeInstanceGuid.IsSet() );
	SetFlag( kHasAgeSequenceNumber, fAgeSequenceNumber!=0 );
	SetFlag( kHasAgeDescription, fAgeDescription.size()!=0 );
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

const char * plAgeInfoStruct::GetDisplayName() const
{
	int seq = GetAgeSequenceNumber();
	if ( seq>0 )
		xtl::format( fDisplayName, "%s(%d) %s", GetAgeUserDefinedName(), seq, GetAgeInstanceName() );
	else
		xtl::format( fDisplayName, "%s %s", GetAgeUserDefinedName(), GetAgeInstanceName() );
	return fDisplayName.c_str();
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
	s->LogSubStreamStart("push me");
	s->LogReadSwap( &fFlags ,"AgeLinkStruct Flags");
	if ( IsFlagSet( kHasAgeInfo ) ) {
		s->LogSubStreamPushDesc("AgeInfo");
		fAgeInfo.Read( s,m );
	}
	if ( IsFlagSet( kHasLinkingRules ) )
		s->LogReadSwap( &fLinkingRules ,"LinkingRules");
	if ( IsFlagSet( kHasSpawnPt_DEAD ) )
	{
		std::string str;
		s->LogSubStreamPushDesc("SpawnPt_DEAD");
		plMsgStdStringHelper::Peek(str,s);
		fSpawnPoint.SetName( str.c_str() );
		if ( strcmp( fSpawnPoint.GetName(), kDefaultSpawnPtName )==0 )
			fSpawnPoint.SetTitle( kDefaultSpawnPtTitle );
		else
			fSpawnPoint.SetTitle( str.c_str() );
		ClearFlag( kHasSpawnPt_DEAD );
		SetFlag( kHasSpawnPt );
	}
	if ( IsFlagSet( kHasSpawnPt_DEAD2 ) )
	{
		s->LogSubStreamPushDesc("SpawnPt_DEAD2");
		fSpawnPoint.ReadOld( s );
		ClearFlag( kHasSpawnPt_DEAD2 );
		SetFlag( kHasSpawnPt );
	}
	else if ( IsFlagSet( kHasSpawnPt ) )
	{
		s->LogSubStreamPushDesc("SpawnPt");
		fSpawnPoint.Read( s );
	}
	if ( IsFlagSet( kHasAmCCR ) )
		s->LogReadSwap( &fAmCCR ,"AmCCR");

	if ( IsFlagSet( kHasParentAgeFilename ) )
	{
		s->LogSubStreamPushDesc("ParentAgeFilename");
		plMsgStdStringHelper::Peek(fParentAgeFilename,s);
	}
}

void plAgeLinkStruct::Write( hsStream * s, hsResMgr* m)
{
	s->WriteSwap( fFlags );
	if ( IsFlagSet( kHasAgeInfo ) )
		fAgeInfo.Write( s,m );
	if ( IsFlagSet( kHasLinkingRules ) )
		s->WriteSwap( fLinkingRules );
	if ( IsFlagSet( kHasSpawnPt ) )
		fSpawnPoint.Write( s );
	if ( IsFlagSet( kHasAmCCR ) )
		s->WriteSwap( fAmCCR );
	if ( IsFlagSet( kHasParentAgeFilename ) )
		plMsgStdStringHelper::Poke(fParentAgeFilename,s);
}

void plAgeLinkStruct::SetParentAgeFilename( const char * v )
{
	if ( v )
	{
		SetFlag( kHasParentAgeFilename );
		fParentAgeFilename=v;
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
		*this=*other;
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
//		fAgeInfo.CopyFrom( node->GetAgeInfo() );
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

std::string plAgeLinkStruct::AsStdString() const
{
	const char * spacer = kEmpty;

	std::stringstream ss;

	ss << "[";

	if (HasAgeInfo())
	{
		ss	<< spacer
			<< "Nfo:"
			<< fAgeInfo.AsStdString();
		spacer = kComma;
	}
	if (HasLinkingRules())
	{
		ss	<< spacer
			<< "Rule:"
			<< plNetCommon::LinkingRules::LinkingRuleStr( GetLinkingRules() );
		spacer = kComma;
	}
	if (HasSpawnPt())
	{
		ss	<< spacer
			<< "Spwn:"
			<< fSpawnPoint.AsStdString().c_str();
		spacer = kComma;
	}
	if (HasAmCCR())
	{
		ss	<< spacer
			<< "CCR:"
			<< ( GetAmCCR()?"yes":"no" );
		spacer = kComma;
	}
	ss	<< "]";

	return ss.str().c_str();
}


////////////////////////////////////////////////////////////////////

void plNetServerSessionInfo::Read(hsStream* s, hsResMgr*)
{
	Clear();
	s->LogSubStreamStart("push me");
	s->LogReadSwap(&fFlags,"ServerSessionInfo Flags");
	if (IsFlagSet(kHasServerName)){
		s->LogSubStreamPushDesc("ServerName");
		plMsgStdStringHelper::Peek(fServerName,s);
	}
	if (IsFlagSet(kHasServerType))
		s->LogReadSwap(&fServerType,"ServerType");
	if (IsFlagSet(kHasServerAddr)){
		s->LogSubStreamPushDesc("ServerAddr");
		plMsgStdStringHelper::Peek(fServerAddr,s);
	}
	if (IsFlagSet(kHasServerPort))
		s->LogReadSwap(&fServerPort,"ServerPort");
	if (IsFlagSet(kHasServerGuid)){
		s->LogSubStreamPushDesc("ServerGuid");
		fServerGuid.Read(s);
	}
}

void plNetServerSessionInfo::Write(hsStream* s, hsResMgr*)
{
	s->WriteSwap(fFlags);
	if (IsFlagSet(kHasServerName))
		plMsgStdStringHelper::Poke(fServerName,s);
	if (IsFlagSet(kHasServerType))
		s->WriteSwap(fServerType);
	if (IsFlagSet(kHasServerAddr))
		plMsgStdStringHelper::Poke(fServerAddr,s);
	if (IsFlagSet(kHasServerPort))
		s->WriteSwap(fServerPort);
	if (IsFlagSet(kHasServerGuid))
		fServerGuid.Write(s);
}

// Read and Write already have their own flags, so I'll just leave those for now. -Colin
void plNetServerSessionInfo::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	Read(s, mgr);
}

void plNetServerSessionInfo::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	Write(s, mgr);
}

void plNetServerSessionInfo::Clear()
{
	fFlags = 0;
	fServerName = "";
	fServerType = plNetServerConstants::kInvalidLo;
	fServerAddr = "";
	fServerPort = 0;
	fServerGuid = plUUID();
}

void plNetServerSessionInfo::CopyFrom(const plNetServerSessionInfo * other)
{
	if ( other )
	{
		fFlags = other->fFlags;
		fServerName = other->fServerName;
		fServerType = other->fServerType;
		fServerAddr = other->fServerAddr;
		fServerPort = other->fServerPort;
		fServerGuid.CopyFrom(other->fServerGuid);
	}
	else
	{
		Clear();
	}
}

std::string plNetServerSessionInfo::AsStdString() const
{
	const char * spacer = kEmpty;

	std::stringstream ss;

	ss << "[";

	if (HasServerType())
	{
		ss	<< spacer
			<< "T:"
			<< plNetServerConstants::GetServerTypeStr(fServerType);
		spacer = kComma;
	}
	if (HasServerName())
	{
		ss	<< spacer
			<< "N:"
			<< SAFE(fServerName.c_str());
		spacer = kComma;
	}
	if (HasServerGuid())
	{
		ss	<< spacer
			<< "G:"
			<< fServerGuid.AsString();
		spacer = kComma;
	}
	if (HasServerAddr() || HasServerPort())
	{
		ss	<< spacer
			<< "A:["
			<< SAFE(fServerAddr.c_str())
			<< ":"
			<< fServerPort
			<< "]";
		spacer = kComma;
	}
	ss	<< "]";

	return ss.str().c_str();
}

std::string plNetServerSessionInfo::AsLogString() const
{
	const char* spacer = kSemicolon;

	std::stringstream ss;
	std::string typeName = "";

	if (HasServerType())
	{
		typeName = plNetServerConstants::GetServerTypeStr(fServerType);
	}

	if (HasServerName())
	{
		ss << typeName << "Name" << "=";
		ss << fServerName.c_str();
		ss << spacer;
	}

	if (HasServerAddr())
	{
		ss << typeName << "Addr" << "=";
		ss << fServerAddr.c_str();
		ss << spacer;
	}

	if (HasServerPort())
	{
		ss << typeName << "Port" << "=";
		ss << fServerPort;
		ss << spacer;
	}

	if (HasServerGuid())
	{
		ss << typeName << "Guid" << "=";
		ss << fServerGuid.AsString();
		ss << spacer;
	}

	return ss.str().c_str();
}

bool plNetServerSessionInfo::IsEqualTo(const plNetServerSessionInfo * other) const
{
	bool match = true;
	if (match && IsFlagSet(kHasServerGuid) && other->IsFlagSet(kHasServerGuid))
		match = match && fServerGuid.IsEqualTo(other->GetServerGuid());
	if (match && IsFlagSet(kHasServerName) && other->IsFlagSet(kHasServerName))
		match = match && (_stricmp(fServerName.c_str(),other->fServerName.c_str())==0);
	if (match && IsFlagSet(kHasServerType) && other->IsFlagSet(kHasServerType))
		match = match && fServerType==other->fServerType;
	if (match && IsFlagSet(kHasServerAddr) && other->IsFlagSet(kHasServerAddr))
		match = match && (_stricmp(fServerAddr.c_str(),other->fServerAddr.c_str())==0);
	if (match && IsFlagSet(kHasServerPort) && other->IsFlagSet(kHasServerPort))
		match = match && fServerPort==other->fServerPort;
	return match;
}


void plNetServerSessionInfo::SetServerName(const char * val)
{
	if (val)
	{
		fServerName=val;
		SetFlag(kHasServerName);
	}
	else
	{
		fServerName="";
		ClearFlag(kHasServerName);
	}
}

void plNetServerSessionInfo::SetServerType(UInt8 val)
{
	if (val>0)
	{
		fServerType=val;
		SetFlag(kHasServerType);
	}
	else
	{
		fServerType=0;
		ClearFlag(kHasServerType);
	}
}

void plNetServerSessionInfo::SetServerAddr(const char * val)
{
	if (val)
	{
		fServerAddr = val;
		SetFlag(kHasServerAddr);
	}
	else
	{
		fServerAddr = "";
		ClearFlag(kHasServerAddr);
	}
}

void plNetServerSessionInfo::SetServerPort(UInt16 val)
{
	if (val>0)
	{
		fServerPort=val;
		SetFlag(kHasServerPort);
	}
	else
	{
		fServerPort=0;
		ClearFlag(kHasServerPort);
	}
}

void plNetServerSessionInfo::SetServerGuid(const plUUID * val)
{
	if (val && val->IsSet())
	{
		fServerGuid.CopyFrom(val);
		SetFlag(kHasServerGuid);
	}
	else
	{
		fServerGuid.Clear();
		ClearFlag(kHasServerGuid);
	}
}

void plNetServerSessionInfo::CopyServerGuid(const plUUID & val)
{
	if (val.IsSet())
	{
		fServerGuid.CopyFrom(val);
		SetFlag(kHasServerGuid);
	}
	else
	{
		fServerGuid.Clear();
		ClearFlag(kHasServerGuid);
	}
}


///////////////////////////////////////////////////////////////////
// End.
