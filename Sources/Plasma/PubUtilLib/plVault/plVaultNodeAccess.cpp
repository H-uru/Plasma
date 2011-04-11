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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultNodeAccess.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop

//============================================================================
// Volatile Vault Node Fields - be very careful when adding to this
//============================================================================
struct NodeTypeToVolatileField {
	unsigned	nodeType;
	qword		volatileFields;
};

NodeTypeToVolatileField volatileFieldList[] = {
	{plVault::kNodeType_PlayerInfo,	VaultPlayerInfoNode::kOnline | VaultPlayerInfoNode::kAgeInstName | VaultPlayerInfoNode::kAgeInstUuid},
	{0, 0}
};

//============================================================================
qword GetNodeVolatileFields(NetVaultNode* node) {
	qword		volatileFields	= 0;
	unsigned	index			= 0;

	while (volatileFieldList[index].nodeType != 0) {
		if (node->nodeType == volatileFieldList[index].nodeType) {
			volatileFields |= volatileFieldList[index].volatileFields;
			break;
		}

		++index;
	}

	return volatileFields;
}

/*****************************************************************************
*
*   NetVaultNodeAccess
*
***/

//============================================================================
NetVaultNodeAccess::NetVaultNodeAccess (NetVaultNode * node)
:	base(node)
,	fieldFlags(node->fieldFlags)
,	dirtyFlags(node->dirtyFlags)
{ }


/*****************************************************************************
*
*   VaultPlayerNode
*
***/

//============================================================================
VaultPlayerNode::VaultPlayerNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	playerName(node->istring64_1)
,	avatarShapeName(node->string64_1)
,	disabled(node->int32_1)
,	explorer(node->int32_2)
,	onlineTime(node->uint32_1)
,	accountUuid(node->uuid_1)
,	inviteUuid(node->uuid_2)
{
}

//============================================================================
void VaultPlayerNode::SetPlayerName (const wchar v[]) {
	IVaultNodeSetString(kPlayerName, base, &playerName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultPlayerNode::SetAvatarShapeName (const wchar v[]) {
	IVaultNodeSetString(kAvatarShapeName, base, &avatarShapeName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultPlayerNode::SetDisabled (int v) {
	IVaultNodeSetValue(kDisabled, base, &disabled, v);
}

//============================================================================
void VaultPlayerNode::SetOnlineTime (unsigned v) {
	IVaultNodeSetValue(kOnlineTime, base, &onlineTime, v);
}

//============================================================================
void VaultPlayerNode::SetAccountUuid (const Uuid & v) {
	IVaultNodeSetValue(kAccountUuid, base, &accountUuid, v);
}

//============================================================================
void VaultPlayerNode::SetInviteUuid (const Uuid & v) {
	IVaultNodeSetValue(kInviteUuid, base, &inviteUuid, v);
}

//============================================================================
void VaultPlayerNode::SetExplorer (int v) {
	IVaultNodeSetValue(kExplorer, base, &explorer, v);
}


/*****************************************************************************
*
*   VaultPlayerInfoNode
*
***/

//============================================================================
VaultPlayerInfoNode::VaultPlayerInfoNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	playerId(node->uint32_1)
,	playerName(node->istring64_1)
,	ageInstName(node->string64_1)
,	ageInstUuid(node->uuid_1)
,	online(node->int32_1)
,	ccrLevel(node->int32_2)
{
}

//============================================================================
void VaultPlayerInfoNode::SetPlayerId (unsigned v) {
	IVaultNodeSetValue(kPlayerId, base, &playerId, v);
}

//============================================================================
void VaultPlayerInfoNode::SetPlayerName (const wchar v[]) {
	IVaultNodeSetString(kPlayerName, base, &playerName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultPlayerInfoNode::SetAgeInstName (const wchar v[]) {
	IVaultNodeSetString(kAgeInstName, base, &ageInstName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultPlayerInfoNode::SetAgeInstUuid (const Uuid & v) {
	IVaultNodeSetValue(kAgeInstUuid, base, &ageInstUuid, v);
}

//============================================================================
void VaultPlayerInfoNode::SetOnline (int v) {
	IVaultNodeSetValue(kOnline, base, &online, v);
}

//============================================================================
void VaultPlayerInfoNode::SetCCRLevel (int v) {
	IVaultNodeSetValue(kCCRLevel, base, &ccrLevel, v);
}

/*****************************************************************************
*
*   VaultFolderNode
*
***/

//============================================================================
VaultFolderNode::VaultFolderNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	folderType(node->int32_1)
,	folderName(node->string64_1)
{
}

//============================================================================
void VaultFolderNode::SetFolderName (const wchar v[]) {
	IVaultNodeSetString(kFolderName, base, &folderName, v, kMaxVaultNodeStringLength);
}
	
//============================================================================
void VaultFolderNode::SetFolderType (int v) {
	IVaultNodeSetValue(kFolderType, base, &folderType, v);
}


/*****************************************************************************
*
*   VaultPlayerInfoListNode
*
***/

//============================================================================
VaultPlayerInfoListNode::VaultPlayerInfoListNode (NetVaultNode * node)
:	VaultFolderNode(node)
{
}


/*****************************************************************************
*
*   VaultAgeInfoListNode
*
***/

//============================================================================
VaultAgeInfoListNode::VaultAgeInfoListNode (NetVaultNode * node)
:	VaultFolderNode(node)
{
}


/*****************************************************************************
*
*   VaultChronicleNode
*
***/

//============================================================================
VaultChronicleNode::VaultChronicleNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	entryType(node->int32_1)
,	entryName(node->string64_1)
,	entryValue(node->text_1)
{
}

//============================================================================
void VaultChronicleNode::SetEntryType (int v) {
	IVaultNodeSetValue(kEntryType, base, &entryType, v);
}

//============================================================================
void VaultChronicleNode::SetEntryName (const wchar v[]) {
	IVaultNodeSetString(kEntryName, base, &entryName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultChronicleNode::SetEntryValue (const wchar v[]) {
	IVaultNodeSetString(kEntryValue, base, &entryValue, v, (unsigned)-1);
}


/*****************************************************************************
*
*   VaultTextNoteNode
*
***/

//============================================================================
VaultTextNoteNode::VaultTextNoteNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	noteType(node->int32_1)
,	noteSubType(node->int32_2)
,	noteTitle(node->string64_1)
,	noteText(node->text_1)
{
}

//============================================================================
void VaultTextNoteNode::SetNoteType (int v) {
	IVaultNodeSetValue(kNoteType, base, &noteType, v);
}

//============================================================================
void VaultTextNoteNode::SetNoteSubType (int v) {
	IVaultNodeSetValue(kNoteSubType, base, &noteSubType, v);
}

//============================================================================
void VaultTextNoteNode::SetNoteTitle (const wchar v[]) {
	IVaultNodeSetString(kNoteTitle, base, &noteTitle, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultTextNoteNode::SetNoteText (const wchar v[]) {
	IVaultNodeSetString(kNoteText, base, &noteText, v, (unsigned)-1);
}

//============================================================================
enum EAgeInfoFields {
	kAgeFilename,
	kAgeInstName,
	kAgeUserName,
	kAgeDesc,
	kAgeInstGuid,
	kAgeLanguage,
	kAgeSequence,
	kNumAgeInfoFields
};

#ifdef CLIENT
void VaultTextNoteNode::SetVisitInfo (const plAgeInfoStruct & info) {
	
	ARRAY(wchar) buf;
	
	for (unsigned i = 0; i < kNumAgeInfoFields; ++i) {
		switch (i) {
			case kAgeFilename: {
				wchar src[128];
				StrToUnicode(src, info.GetAgeFilename(), arrsize(src));
				unsigned len = StrLen(src);
				wchar * dst = buf.New(len);
				MemCopy(dst, src, len * sizeof(src[0]));
			}
			break;
			
			case kAgeInstName: {
				wchar src[128];
				StrToUnicode(src, info.GetAgeInstanceName(), arrsize(src));
				unsigned len = StrLen(src);
				wchar * dst = buf.New(len);
				MemCopy(dst, src, len * sizeof(src[0]));
			}
			break;
			
			case kAgeUserName: {
				wchar src[128];
				StrToUnicode(src, info.GetAgeUserDefinedName(), arrsize(src));
				unsigned len = StrLen(src);
				wchar * dst = buf.New(len);
				MemCopy(dst, src, len * sizeof(src[0]));
			}
			break;
			
			case kAgeDesc: {
				wchar src[128];
				StrToUnicode(src, info.GetAgeDescription(), arrsize(src));
				unsigned len = StrLen(src);
				wchar * dst = buf.New(len);
				MemCopy(dst, src, len * sizeof(src[0]));
			}
			break;
			
			case kAgeInstGuid: {
				Uuid guid = (Uuid)*info.GetAgeInstanceGuid();
				wchar src[64];
				GuidToString(guid, src, arrsize(src));
				unsigned len = StrLen(src);
				wchar * dst = buf.New(len);
				MemCopy(dst, src, len * sizeof(src[0]));
			}
			break;
			
			case kAgeLanguage: {
				wchar src[32];
				StrPrintf(src, arrsize(src), L"%u", info.GetAgeLanguage());
				unsigned len = StrLen(src);
				wchar * dst = buf.New(len);
				MemCopy(dst, src, len * sizeof(src[0]));
			}
			break;
			
			case kAgeSequence: {
				wchar src[32];
				StrPrintf(src, arrsize(src), L"%u", info.GetAgeSequenceNumber());
				unsigned len = StrLen(src);
				wchar * dst = buf.New(len);
				MemCopy(dst, src, len * sizeof(src[0]));
			}
			break;

			DEFAULT_FATAL(i);
		}
		
		wchar * sep = buf.New(1);
		*sep = L'|';			
	}
	
	wchar * term = buf.New(1);
	*term = 0;
	
	SetNoteText(buf.Ptr());
}
#endif

//============================================================================
#ifdef CLIENT
bool VaultTextNoteNode::GetVisitInfo (plAgeInfoStruct * info) {

	wchar * mem;
	const wchar * str = mem = StrDup(noteText);
	
	for (unsigned i = 0; i < kNumAgeInfoFields; ++i) {
		
		wchar token[1024];
		switch (i) {
			case kAgeFilename: {
				StrTokenize(&str, token, arrsize(token), L"|", 1);
				if (StrLen(token) > 0) {
					char ansi[1024];
					StrToAnsi(ansi, token, arrsize(ansi));
					info->SetAgeFilename(ansi);
				}
			}
			break;
			
			case kAgeInstName: {
				StrTokenize(&str, token, arrsize(token), L"|", 1);
				if (StrLen(token) > 0) {
					char ansi[1024];
					StrToAnsi(ansi, token, arrsize(ansi));
					info->SetAgeInstanceName(ansi);
				}
			}
			break;
			
			case kAgeUserName: {
				StrTokenize(&str, token, arrsize(token), L"|", 1);
				if (StrLen(token) > 0) {
					char ansi[1024];
					StrToAnsi(ansi, token, arrsize(ansi));
					info->SetAgeUserDefinedName(ansi);
				}
			}
			break;
			
			case kAgeDesc: {
				StrTokenize(&str, token, arrsize(token), L"|", 1);
				if (StrLen(token) > 0) {
					char ansi[1024];
					StrToAnsi(ansi, token, arrsize(ansi));
					info->SetAgeDescription(ansi);
				}
			}
			break;
			
			case kAgeInstGuid: {
				StrTokenize(&str, token, arrsize(token), L"|", 1);
				if (StrLen(token) > 0) {
					Uuid guid;
					GuidFromString(token, &guid);
					info->SetAgeInstanceGuid(&plUUID(guid));
				}
			}
			break;
			
			case kAgeLanguage: {
				StrTokenize(&str, token, arrsize(token), L"|", 1);
				if (StrLen(token) > 0) {
					info->SetAgeLanguage(StrToUnsigned(token, nil, 10));
				}
			}
			break;
			
			case kAgeSequence: {
				StrTokenize(&str, token, arrsize(token), L"|", 1);
				if (StrLen(token) > 0) {
					info->SetAgeSequenceNumber(StrToUnsigned(token, nil, 10));
				}
			}
			break;

			DEFAULT_FATAL(i);
		}			
	}
	
	FREE(mem);
	return true;
}
#endif


/*****************************************************************************
*
*   VaultSDLNode
*
***/
	
//============================================================================
VaultSDLNode::VaultSDLNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	sdlIdent(node->int32_1)
,	sdlName(node->string64_1)
,	sdlData(node->blob_1)
,	sdlDataLen(node->blob_1Length)
{
}

//============================================================================
void VaultSDLNode::SetSdlIdent (int v) {
	IVaultNodeSetValue(kSDLIdent, base, &sdlIdent, v);
}

//============================================================================
void VaultSDLNode::SetSdlName (const wchar v[]) {
	IVaultNodeSetString(kSDLName, base, &sdlName, v, kMaxVaultNodeStringLength);
}

//============================================================================
#ifdef CLIENT
bool VaultSDLNode::GetStateDataRecord (plStateDataRecord * rec, unsigned readOptions) {
	if (!sdlDataLen || !sdlData)
		return false;

	hsRAMStream ram;
	ram.Write(sdlDataLen, sdlData);
	ram.Rewind();
	
	char *	sdlRecName = nil;
	int		sdlRecVersion;
	if (!plStateDataRecord::ReadStreamHeader(&ram, &sdlRecName, &sdlRecVersion))
		return false;
		
	rec->SetDescriptor(sdlRecName, sdlRecVersion);
	FREE(sdlRecName);

	// Note: Setting from default here results in a bug causing age SDL to
	// be incorrectly shown when immediately linking back to an age you linked
	// out of (relto door will be closed, window shut, etc).	
	// rec->SetFromDefaults(false);

	if (!rec->Read( &ram, 0, readOptions))
		return false;
		
	// If we converted the record to a newer version, re-save it.		
	if (rec->GetDescriptor()->GetVersion() != sdlRecVersion)
		SetStateDataRecord(rec, readOptions);
	
	return true;
}
#endif // def CLIENT

//============================================================================
#ifdef CLIENT
void VaultSDLNode::SetStateDataRecord (const plStateDataRecord * rec, unsigned writeOptions) {
	hsRAMStream ram;
	rec->WriteStreamHeader(&ram);
	rec->Write(&ram, 0, writeOptions);
	ram.Rewind();
	
	unsigned bytes = ram.GetEOF();
	byte * buf, * heap = nil;
	if (bytes <= 2048)
		buf = ALLOCA(byte, bytes);
	else
		buf = (byte *)ALLOC(bytes);

	ram.CopyToMem(buf);		
	
	IVaultNodeSetBlob(kSDLData, base, &sdlData, &sdlDataLen, buf, bytes);
	
	FREE(heap);
}
#endif // def CLIENT

//============================================================================
#ifdef CLIENT
void VaultSDLNode::InitStateDataRecord (const wchar sdlRecName[], unsigned writeOptions) {
	{
		plStateDataRecord * rec = NEWZERO(plStateDataRecord);
		bool exists = GetStateDataRecord(rec, 0);
		DEL(rec);
		if (exists)
			return;
	}
	
	char aStr[MAX_PATH];
	StrToAnsi(aStr, sdlRecName, arrsize(aStr));
	if (plStateDescriptor * des = plSDLMgr::GetInstance()->FindDescriptor(aStr, plSDL::kLatestVersion)) {
		plStateDataRecord rec(des);
		rec.SetFromDefaults(false);
		SetStateDataRecord(&rec, writeOptions|plSDL::kDontWriteDirtyFlag);
	}
}
#endif // def CLIENT


/*****************************************************************************
*
*   VaultImageNode
*
***/

//============================================================================
VaultImageNode::VaultImageNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	title(node->string64_1)
,	imgType(node->int32_1)
,	imgData(node->blob_1)
,	imgDataLen(node->blob_1Length)
{
}

//============================================================================
void VaultImageNode::SetImageTitle (const wchar v[]) {
	IVaultNodeSetString(kImageTitle, base, &title, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultImageNode::SetImageType (int v) {
	IVaultNodeSetValue(kImageType, base, &imgType, v);
}

//============================================================================
void VaultImageNode::SetImageData (const byte buffer[], unsigned bytes) {
	IVaultNodeSetBlob(kImageData, base, &imgData, &imgDataLen, buffer, bytes);
}

//============================================================================
#ifdef CLIENT
void VaultImageNode::StuffImage (plMipmap * src) {
	hsRAMStream	ramStream;
	plJPEG::Instance().SetWriteQuality(30/*percent*/);
	if (plJPEG::Instance().WriteToStream(&ramStream, src)) {
		unsigned bytes = ramStream.GetEOF();		
		byte * buffer = (byte *)ALLOC(bytes);
		ramStream.CopyToMem(buffer);
		IVaultNodeSetBlob(kImageData, base, &imgData, &imgDataLen, buffer, bytes);
		SetImageType(kJPEG);
		FREE(buffer);
	}
	else {
		IVaultNodeSetBlob(kImageData, base, &imgData, &imgDataLen, nil, 0);
		SetImageType(kNone);
	}
}
#endif

//============================================================================
#ifdef CLIENT
bool VaultImageNode::ExtractImage (plMipmap ** dst) {
	
	switch (imgType) {
		case kNone:
			(*dst) = nil;
		return false;
		
		case kJPEG: {
			hsRAMStream	ramStream;
			ramStream.Write(imgDataLen, imgData);
			ramStream.Rewind();
			(*dst) = plJPEG::Instance().ReadFromStream(&ramStream);
		}
		return ((*dst) != nil);
		
		DEFAULT_FATAL(imgType);
	}
}
#endif


/*****************************************************************************
*
*   VaultAgeLinkNode
*
***/

#ifdef CLIENT
struct MatchesSpawnPointTitle
{
	std::string fTitle;
	MatchesSpawnPointTitle( const char * title ):fTitle( title ){}
	bool operator ()( const plSpawnPointInfo & p ) const { return ( p.fTitle==fTitle ); }
};
struct MatchesSpawnPointName
{
	std::string fName;
	MatchesSpawnPointName( const char * name ):fName( name ){}
	bool operator ()( const plSpawnPointInfo & p ) const { return ( p.fSpawnPt==fName ); }
};
#endif

//============================================================================
VaultAgeLinkNode::VaultAgeLinkNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	unlocked(node->int32_1)
,	volat(node->int32_2)
,	spawnPoints(node->blob_1)
,	spawnPointsLen(node->blob_1Length)
{
}

//============================================================================
void VaultAgeLinkNode::SetUnlocked (int v) {
	IVaultNodeSetValue(kUnlocked, base, &unlocked, v);
}

//============================================================================
void VaultAgeLinkNode::SetVolatile (int v) {
	IVaultNodeSetValue(kVolatile, base, &volat, v);
}

//============================================================================
#ifdef CLIENT
bool VaultAgeLinkNode::CopyTo (plAgeLinkStruct * link) {
	if (RelVaultNode * me = VaultGetNodeIncRef(base->nodeId)) {
		if (RelVaultNode * info = me->GetChildNodeIncRef(plVault::kNodeType_AgeInfo, 1)) {
			VaultAgeInfoNode access(info);
			access.CopyTo(link->GetAgeInfo());
			me->DecRef();
			return true;
		}
		me->DecRef();
	}
	link->Clear();
	return false;
}
#endif

//============================================================================
#ifdef CLIENT
void VaultAgeLinkNode::AddSpawnPoint (const plSpawnPointInfo & point) {

	plSpawnPointVec points;
	GetSpawnPoints( &points );
	if ( std::find_if( points.begin(), points.end(), MatchesSpawnPointTitle( point.fTitle.c_str() ) )!=points.end() )
		return;

	// only check to see if the titles are the same... 
	//... so we can add the same spawnpoint as long as they have different titles
		//if ( std::find_if( points.begin(), points.end(), MatchesSpawnPointName( point.fSpawnPt.c_str() ) )!=points.end() )
		//	return;

	points.push_back( point );
	SetSpawnPoints( points );
}
#endif

//============================================================================
#ifdef CLIENT
void VaultAgeLinkNode::RemoveSpawnPoint (const char spawnPtName[]) {

	plSpawnPointVec points;
	GetSpawnPoints( &points );												    
	plSpawnPointVec::iterator it = std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) );
	while ( it!=points.end() )
	{
		points.erase( it );
		SetSpawnPoints( points );
		it = std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) );
	}
}
#endif

//============================================================================
#ifdef CLIENT
bool VaultAgeLinkNode::HasSpawnPoint (const char spawnPtName[]) const {

	plSpawnPointVec points;
	GetSpawnPoints( &points );												    
	return ( std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) )!=points.end() );
}
#endif

//============================================================================
#ifdef CLIENT
bool VaultAgeLinkNode::HasSpawnPoint (const plSpawnPointInfo & point) const {

	return HasSpawnPoint(point.GetName());
}
#endif

//============================================================================
#ifdef CLIENT
void VaultAgeLinkNode::GetSpawnPoints (plSpawnPointVec * out) const {
	
	char str[2048];
	ZERO(str);
	MemCopy(str, spawnPoints, min(spawnPointsLen, arrsize(str) - 1));
	
	char token1[ 1024 ];
	hsStringTokenizer izer1( str, ";" );
	while ( izer1.Next( token1, sizeof( token1 ) ) )
	{
		plSpawnPointInfo point;
		char token2[ 1024 ];
		hsStringTokenizer izer2( token1, ":" );
		if ( izer2.Next( token2, sizeof( token2 ) ) )
			point.fTitle = token2;
		if ( izer2.Next( token2, sizeof( token2 ) ) )
			point.fSpawnPt = token2;
		if ( izer2.Next( token2, sizeof( token2 ) ) )
			point.fCameraStack = token2;

		out->push_back(point);
	}
}
#endif

//============================================================================
#ifdef CLIENT
void VaultAgeLinkNode::SetSpawnPoints (const plSpawnPointVec & in) {

	std::stringstream ss;
	for ( unsigned i=0; i<in.size(); i++ ) {
		ss
			<< in[i].fTitle << ":"
			<< in[i].fSpawnPt << ":"
			<< in[i].fCameraStack << ";";
	}
	IVaultNodeSetBlob(
		kSpawnPoints,
		base,
		&spawnPoints,
		&spawnPointsLen,
		(const byte *)ss.str().c_str(),
		ss.str().size()
	);
}
#endif

/*****************************************************************************
*
*   VaultAgeNode
*
***/

//============================================================================
VaultAgeNode::VaultAgeNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	ageInstUuid(node->uuid_1)
,	parentAgeInstUuid(node->uuid_2)
,	ageName(node->string64_1)
{
}

//============================================================================
void VaultAgeNode::SetAgeInstGuid (const Uuid & v) {
	IVaultNodeSetValue(kAgeInstanceGuid, base, &ageInstUuid, v);
}

//============================================================================
void VaultAgeNode::SetParentAgeInstGuid (const Uuid & v) {
	IVaultNodeSetValue(kParentAgeInstanceGuid, base, &parentAgeInstUuid, v);
}

//============================================================================
void VaultAgeNode::SetAgeName (const wchar v[]) {
	IVaultNodeSetString(kAgeName, base, &ageName, v, kMaxVaultNodeStringLength);
}


/*****************************************************************************
*
*   VaultAgeInfoNode
*
***/

//============================================================================
VaultAgeInfoNode::VaultAgeInfoNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	ageFilename(node->string64_2)
,	ageInstName(node->string64_3)
,	ageUserDefinedName(node->string64_4)
,	ageInstUuid(node->uuid_1)
,	parentAgeInstUuid(node->uuid_2)
,	ageSequenceNumber(node->int32_1)
,	ageIsPublic(node->int32_2)
,	ageLanguage(node->int32_3)
,	ageId(node->uint32_1)
,	ageCzarId(node->uint32_2)
,	ageInfoFlags(node->uint32_3)
,	ageDescription(node->text_1)
{
}

//============================================================================
void VaultAgeInfoNode::SetAgeFilename (const wchar v[]) {
	IVaultNodeSetString(kAgeFilename, base, &ageFilename, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultAgeInfoNode::SetAgeInstName (const wchar v[]) {
	IVaultNodeSetString(kAgeInstanceName, base, &ageInstName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultAgeInfoNode::SetAgeUserDefinedName (const wchar v[]) {
	IVaultNodeSetString(kAgeUserDefinedName, base, &ageUserDefinedName, v, kMaxVaultNodeStringLength);
}

//============================================================================
void VaultAgeInfoNode::SetAgeInstGuid (const Uuid & v) {
	IVaultNodeSetValue(kAgeInstanceGuid, base, &ageInstUuid, v);
}

//============================================================================
void VaultAgeInfoNode::SetParentAgeInstGuid (const Uuid & v) {
	IVaultNodeSetValue(kParentAgeInstanceGuid, base, &parentAgeInstUuid, v);
}

//============================================================================
void VaultAgeInfoNode::SetAgeSequenceNumber (int v) {
	IVaultNodeSetValue(kAgeSequenceNumber, base, &ageSequenceNumber, v);
}

//============================================================================
void VaultAgeInfoNode::_SetAgeIsPublic (int v) {
	IVaultNodeSetValue(kIsPublic, base, &ageIsPublic, v);
}

//============================================================================
void VaultAgeInfoNode::SetAgeLanguage (int v) {
	IVaultNodeSetValue(kAgeLanguage, base, &ageLanguage, v);
}

//============================================================================
void VaultAgeInfoNode::SetAgeId (unsigned v) {
	IVaultNodeSetValue(kAgeId, base, &ageId, v);
}

//============================================================================
void VaultAgeInfoNode::SetAgeCzarId (unsigned v) {
	IVaultNodeSetValue(kAgeCzarId, base, &ageCzarId, v);
}

//============================================================================
void VaultAgeInfoNode::SetAgeInfoFlags (unsigned v) {
	IVaultNodeSetValue(kAgeInfoFlags, base, &ageInfoFlags, v);
}

//============================================================================
void VaultAgeInfoNode::SetAgeDescription (const wchar v[]) {
	IVaultNodeSetString(kAgeDescription, base, &ageDescription, v, (unsigned)-1);
}

//============================================================================
#ifdef CLIENT
const class plUnifiedTime * VaultAgeInfoNode::GetAgeTime () const {
	hsAssert(false, "eric, implement me.");
	return nil;
}
#endif // def CLIENT

//============================================================================
#ifdef CLIENT
void VaultAgeInfoNode::CopyFrom (const plAgeInfoStruct * info) {
	wchar str[MAX_PATH];

	// age filename
	if (info->HasAgeFilename())	{
		StrToUnicode(str, info->GetAgeFilename(), arrsize(str));
		SetAgeFilename(str);
	}
	else {
		SetAgeFilename(nil);
	}

	// age instance name	
	if (info->HasAgeInstanceName())	{
		StrToUnicode(str, info->GetAgeInstanceName(), arrsize(str));
		SetAgeInstName(str);
	}
	else {
		SetAgeInstName(nil);
	}
	
	// age user-defined name
	if (info->HasAgeUserDefinedName())	{
		StrToUnicode(str, info->GetAgeUserDefinedName(), arrsize(str));
		SetAgeUserDefinedName(str);
	}
	else {
		SetAgeUserDefinedName(nil);
	}

	// age description
	// TODO
	if (info->HasAgeDescription())	{
//		StrToUnicode(str, info->GetAgeDescription(), arrsize(str));
//		SetAgeDescription(str);
	}
	else {
//		SetAgeDescription(nil);
	}

	// age sequence number
	SetAgeSequenceNumber(info->GetAgeSequenceNumber());

	// age instance guid
	SetAgeInstGuid((Uuid)*info->GetAgeInstanceGuid());

	// age language	
	SetAgeLanguage(info->GetAgeLanguage());
}
#endif // def CLIENT

//============================================================================
#ifdef CLIENT
void VaultAgeInfoNode::CopyTo (plAgeInfoStruct * info) const {
	char str[MAX_PATH];

	// age filename 	
	StrToAnsi(str, ageFilename, arrsize(str));
	info->SetAgeFilename(str);
	
	// age instance name
	StrToAnsi(str, ageInstName, arrsize(str));
	info->SetAgeInstanceName(str);
	
	// age user-defined name
	StrToAnsi(str, ageUserDefinedName, arrsize(str));
	info->SetAgeUserDefinedName(str);

	// age description
	// TODO
	
	// age sequence number
	info->SetAgeSequenceNumber(ageSequenceNumber);
	
	// age instance guid
	plUUID uuid(ageInstUuid);
	info->SetAgeInstanceGuid(&uuid);
	
	// age language
	info->SetAgeLanguage(ageLanguage);
}
#endif // def CLIENT


/*****************************************************************************
*
*   VaultSystemNode
*
***/

//============================================================================
VaultSystemNode::VaultSystemNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	ccrStatus(node->int32_1)
{
}

//============================================================================
void VaultSystemNode::SetCCRStatus (int v) {
	IVaultNodeSetValue(kCCRStatus, base, &ccrStatus, v);
}


/*****************************************************************************
*
*   VaultMarkerGameNode
*
***/

//============================================================================
VaultMarkerGameNode::VaultMarkerGameNode (NetVaultNode * node)
:	NetVaultNodeAccess(node)
,	gameName(node->text_1)
,	gameGuid(node->uuid_1)
{
}

//============================================================================
void VaultMarkerGameNode::SetGameName (const wchar v[]) {
	IVaultNodeSetString(kGameName, base, &gameName, v, (unsigned)-1);
}

//============================================================================
void VaultMarkerGameNode::SetGameGuid (const Uuid & v) {
	IVaultNodeSetValue(kGameGuid, base, &gameGuid, v);
}
