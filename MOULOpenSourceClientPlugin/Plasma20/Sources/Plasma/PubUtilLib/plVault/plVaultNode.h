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
#if 0
#ifndef plVaultNode_h_inc
#define plVaultNode_h_inc

#include "hsTypes.h"
#include "hsBitVector.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "../plNetCommon/plSpawnPointInfo.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plUUID/plUUID.h"
#include "plVault.h"
#include "plVaultNodeIterator.h"
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////

class plVaultNode;
class plVaultFolderNode;
class plVaultImageNode;
class plVaultTextNoteNode;
class plVaultSDLNode;
class plVaultAgeLinkNode;
class plVaultChronicleNode;
class plVaultMgrNode;
class plVaultPlayerNode;
class plVaultPlayerInfoNode;
class plVaultPlayerInfoListNode;
class plVaultAgeNode;
class plVaultAgeInfoNode;
class plVaultAgeInfoListNode;
class plVaultSystemNode;
class plStateDataRecord;
class plURL;

struct hsPoint3;


////////////////////////////////////////////////////////////////////

typedef bool (*plNodeCompareProc)( const plVaultNode * A, const plVaultNode * B );


class plVaultNode
{
public:
	struct MatchesNode
	{
		const plVaultNode * fNode;
		MatchesNode( const plVaultNode * node ): fNode( node ) {}
		bool operator()( const plVaultNode * node ) const;
	};
	friend struct MatchesNode;

	void	SetID( UInt32 v );
	void	SetType( UInt8 v );
	void	SetPermissions( UInt32 v );
	void	SetOwnerNodeID( UInt32 v );
	void	SetGroupNodeID( UInt32 v );
	void	SetCreatorNodeID( UInt32 v );
	void	SetCreateTime( const plUnifiedTime * v );
	void	SetCreateTime( const plUnifiedTime & v );
	void	SetCreateAgeTime( const plUnifiedTime * v );
	void	SetCreateAgeTime( const plUnifiedTime & v );
	void	SetCreateAgeName( const char * v );
	void	SetCreateAgeGuid( const plUUID * v );
	void	SetCreateAgeGuid( const plUUID & v );
	void	SetInt32_1( Int32 v );
	void	SetInt32_2( Int32 v );
	void	SetInt32_3( Int32 v );
	void	SetInt32_4( Int32 v );
	void	SetUInt32_1( UInt32 v );
	void	SetUInt32_2( UInt32 v );
	void	SetUInt32_3( UInt32 v );
	void	SetUInt32_4( UInt32 v );
	void	SetString64_1( const char * v );
	void	SetString64_2( const char * v );
	void	SetString64_3( const char * v );
	void	SetString64_4( const char * v );
	void	SetString64_5( const char * v );
	void	SetString64_6( const char * v );
	void	SetIString64_1( const char * v );
	void	SetIString64_2( const char * v );
	void	SetText_1( const char * v );
	void	SetText_2( const char * v );
	void	SetBlob_1_Guid( const plUUID * v );
	void	SetBlob_2_Guid( const plUUID * v );
	void *	AllocBufferBlob_1( int size );
	void *	AllocBufferBlob_2( int size );

	UInt32							GetID () const;
	UInt8							GetType () const;
	UInt32							GetPermissions () const;
	UInt32							GetOwnerNodeID () const;
	const plVaultPlayerInfoNode *	GetOwnerNode () const;
	UInt32							GetGroupNodeID () const;
	const plVaultNode *				GetGroupNode () const;
	const plUnifiedTime *			GetModifyTime () const;
	UInt32							GetCreatorNodeID () const;
	const plVaultPlayerInfoNode *	GetCreatorNode () const;
	const plUnifiedTime *			GetCreateTime () const;
	const plUnifiedTime *			GetCreateAgeTime () const;
	const char *					GetCreateAgeName () const;
	const plUUID *					GetCreateAgeGuid () const;
	Int32							GetInt32_1 () const;
	Int32							GetInt32_2 () const;
	Int32							GetInt32_3 () const;
	Int32							GetInt32_4 () const;
	UInt32							GetUInt32_1 () const;
	UInt32							GetUInt32_2 () const;
	UInt32							GetUInt32_3 () const;
	UInt32							GetUInt32_4 () const;
	const char * 					GetString64_1 () const;
	const char * 					GetString64_2 () const;
	const char * 					GetString64_3 () const;
	const char * 					GetString64_4 () const;
	const char * 					GetString64_5 () const;
	const char * 					GetString64_6 () const;
	const char * 					GetIString64_1 () const;
	const char * 					GetIString64_2 () const;
	const char * 					GetText_1 () const;
	const char * 					GetText_2 () const;
	std::string &					GetBlob_1 ();
	const std::string & 			GetBlob_1 () const;
	void *							GetBufferBlob_1 () const;
	int								GetBufferSizeBlob_1 () const;
	const plUUID *					GetBlob_1_Guid () const;
	std::string &					GetBlob_2 ();
	const std::string & 			GetBlob_2 () const;
	void *							GetBufferBlob_2 () const;
	int								GetBufferSizeBlob_2 () const;
	const plUUID *					GetBlob_2_Guid () const;

public:
	plVaultNode();
	virtual ~plVaultNode();

	bool	IsADownstreamNode( UInt32 nodeID ) const;


	/////////////////////////////////////////////////
	// Vault Node API

#if 0
	// Add child node
	plVaultNodeRef * AddNode(
		plVaultNode * node,
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	// Find matching node on server and add it to this node.
	// Optionally don't fetch children of this node.
	bool LinkToNode(
		const plVaultNode * templateNode,
		int childFetchLevel=plVault::kFetchAllChildren,
		bool allowCreate=true,
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	bool LinkToNode(
		UInt32 nodeID,
		int childFetchLevel=plVault::kFetchAllChildren,
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	// Remove child node
	bool RemoveNode( const plVaultNode * node,
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	bool RemoveNode( UInt32 nodeID,
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	// Remove all child nodes
	void RemoveAllNodes( void );
	// Add/Save this node to vault
	virtual void Save(
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	// Save this node and all child nodes that need saving.
	// NOTE: Currently, the cb object is called back for
	// each node saved.
	void SaveAll(
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	// Send this node to the destination client node. will be received in it's inbox folder.
	void SendTo(
		UInt32 destClientNodeID,
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
#endif

	// Get an iterator to this node's children
	plVaultNodeIterator GetIterator( void );


#if 0
	// Get child node count
	int		GetNodeCount( void ) const { return fChildNodes.size(); }
	// Get all child nodes.
	void	GetNodes( plVault::NodeRefVec & out );
	// Get child node by ID
	bool	GetNode( UInt32 nodeID, plVaultNodeRef *& out ) const;
	// Get child node matching template node
	bool	FindNode( const plVaultNode * templateNode, plVaultNodeRef *& out ) const;
	// Get all matching child nodes
	bool	FindNodes( const plVaultNode * templateNode, plVault::NodeRefVec & out ) const;
	// Get first matching node recursively among children
	bool	FindNodeRecurse( const plVaultNode * templateNode, plVaultNodeRef *& out ) const;
	// Returns true if node is a child of this node
	bool	HasNode( UInt32 nodeID );
	bool	HasNode( const plVaultNode * templateNode );

	// true if node needs saving
	bool	Modified( void ) const;
	void	SetModified( bool b );

	// Get the client ID from my Vault client.
	UInt32	GetClientID( void ) const;

	// application data keyed to this node. not stored.
	void	SetUserData( UInt32 v ) { fUserData=v; }
	UInt32	GetUserData( void ) const { return fUserData; }
#endif

	/////////////////////////////////////////////////

#if 0
	void	Read( hsStream * s, hsResMgr * mgr );
	void	Write( hsStream * s, hsResMgr * mgr );
	virtual void CopyFrom( const plVaultNode * other, bool forceCopyAllFields=false );

	virtual std::string AsStdString( int level=0 ) const;
	std::string			FieldsAsStdString( void ) const;
	void				Dump( int level=0, bool displaySeen=false, bool beenSeen=true, plStatusLog * log=nil ) const;
	void				GetChildNodes( const plVault::NodeRefVec *& out ) const { out=&fChildNodes; }

	virtual std::string AsHtmlForm( const char * action ) { return ""; }
	bool				FromHtmlPost( plURL & url ) { return false; }
#endif

private:
	plVaultNode( const plVaultNode & );
	plVaultNode & operator =( const plVaultNode & );
};


//============================================================================
//============================================================================
//============================================================================
//============================================================================
#if 0

////////////////////////////////////////////////////////////////////

class plVaultFolderNode : public plVaultNode
{
public:
	enum FieldMap
	{
		kFolderType		= kInt32_1,
		kFolderName		= kString64_1,
	};

	plVaultFolderNode();
	~plVaultFolderNode();
	CLASSNAME_REGISTER( plVaultFolderNode );
	GETINTERFACE_ANY( plVaultFolderNode, plVaultNode );

	void	SetFolderType( int type )		{ ISetInt32_1( type ); }
	int		GetFolderType( void ) const		{ return IGetInt32_1();}
	void	SetFolderName( const char * v )	{ ISetString64_1( v ); }
	const char * GetFolderName( void ) const{ return IGetString64_1();}

	std::string	AsStdString( int level=0 ) const;

	std::string AsHtmlForm( const char * action );
};

////////////////////////////////////////////////////////////////////

class plMipmap;
class plVaultImageNode : public plVaultNode
{
	friend class plNetClientVNodeMgr;

	plMipmap *		fMipmap;
	void	ISetMipmap( plMipmap * v ) { fMipmap=v; }
	plMipmap * IGetMipmap( void ) const { return fMipmap; }

public:
	enum FieldMap
	{
		kImageType		= kInt32_1,
		kImageTitle		= kString64_1,
		kImageData		= kBlob_1,
	};

	enum ImageTypes	{ kJPEG=1 };

	plVaultImageNode();
	~plVaultImageNode();
	CLASSNAME_REGISTER( plVaultImageNode );
	GETINTERFACE_ANY( plVaultImageNode, plVaultNode );

	const plMipmap * GetMipmap( void ) const { return fMipmap; }
	// this override copies the mipmap ptr too.
	void	CopyFrom( const plVaultNode * other, bool forceCopyAllFields=false );

	void	SetImageType( int type )	{ ISetInt32_1( type ); }
	int		GetImageType( void ) const	{ return IGetInt32_1(); }
	void *	AllocBuffer( int size )		{ return IAllocBufferBlob_1( size ); }
	void *	GetBuffer( void ) const		{ return IGetBufferBlob_1(); }
	int		GetBufSize() const			{ return IGetBufferSizeBlob_1(); }

	void SetTitle( const char * text )	{ ISetString64_1( text ); }
	const char * GetTitle( void ) const	{ return IGetString64_1(); }
	
	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultTextNoteNode : public plVaultNode
{
public:
	enum FieldMap
	{
		kNoteType		= kInt32_1,
		kNoteSubType	= kInt32_2,
		kNoteTitle		= kString64_1,
		kNoteText		= kBlob_1,
	};

	plVaultTextNoteNode();
	~plVaultTextNoteNode();
	CLASSNAME_REGISTER( plVaultTextNoteNode );
	GETINTERFACE_ANY( plVaultTextNoteNode, plVaultNode );

	void SetText( const char * text );
	const char * GetText( void ) const	{ return IGetBlob_1().c_str(); }

	void SetTitle( const char * text )	{ ISetString64_1( text ); }
	const char * GetTitle( void ) const	{ return IGetString64_1(); }

	void SetNoteType( Int32 type )		{ ISetInt32_1( type ); }
	Int32 GetNoteType( void ) const		{ return IGetInt32_1(); }

	void SetNoteSubType( Int32 type )	{ ISetInt32_2( type ); }
	Int32 GetNoteSubType( void ) const	{ return IGetInt32_2(); }

	// Device-specific:
	plVaultFolderNode * GetDeviceInbox() const;
	void SetDeviceInbox( const char * inboxName,
		plVaultOperationCallback * cb=nil, UInt32 cbContext=0 );

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultSDLNode : public plVaultNode
{
	mutable plStateDataRecord * fSDLDataRec;	// for GetStateDataRecord()

public:
	enum FieldMap
	{
		kName		= kString64_1,
		kIdent		= kInt32_1,		// pgmr-defined, from plVault::StandardNodes enum.
		kSDLData	= kBlob_1,
	};

	plVaultSDLNode();
	~plVaultSDLNode();
	CLASSNAME_REGISTER( plVaultSDLNode );
	GETINTERFACE_ANY( plVaultSDLNode, plVaultNode );

	void	SetIdent( int v )			{ ISetInt32_1( v ); }
	int		GetIdent() const			{ return IGetInt32_1(); }

	void *	AllocBuffer( int size )		{ return IAllocBufferBlob_1( size ); }
	void *	GetBuffer( void ) const		{ return IGetBufferBlob_1(); }
	int		GetBufSize() const			{ return IGetBufferSizeBlob_1(); }

	plStateDataRecord * GetStateDataRecord( UInt32 readOptions=0 ) const;	// returned pointer will be valid until next call to this fxn, or until this node instance goes away.
	void SetStateDataRecord( const plStateDataRecord * rec, UInt32 writeOptions=0 );
	void InitStateDataRecord( const char * sdlRecName, UInt32 writeOptions=0 );
	void DumpStateDataRecord( const char * msg=nil, bool dirtyOnly=false ) const;

	// override to dump debug info.
	void Save(
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultAgeInfoNode : public plVaultNode
{
	mutable plVaultPlayerInfoListNode *		fCanVisitFolder;
	mutable plVaultPlayerInfoListNode *		fAgeOwnersFolder;
	mutable plVaultSDLNode *				fAgeSDL;
	mutable plVaultFolderNode *				fChildAgesFolder;

	mutable	plUUID	fAgeInstanceGuid;
	mutable plNetServerSessionInfo	fServerInfo;
	mutable plAgeInfoStruct			fAgeInfoStruct;

	/////////////////////////////////
	friend class pyVault;
	friend class pyAgeVault;
	friend class plVaultAgeInfoInitializationTask;
	plVaultSDLNode *	IGetAgeSDL() const;
	plVaultAgeLinkNode * IGetLink( plVaultFolderNode * folder, const plAgeInfoStruct * info ) const;
	/////////////////////////////////

public:
	enum FieldMap
	{
		kAgeFilename		= kString64_1,		// "Garden"
		kAgeInstanceName	= kString64_2,		// "Eder Kemo"
		kAgeUserDefinedName	= kString64_3,		// "Joe's"
		kAgeInstanceGuid	= kString64_4,
		kAgeDescription		= kText_1,			// "Stay out!"
		kAgeSequenceNumber	= kInt32_1,
		kIsPublic			= kInt32_2,
		kAgeLanguage		= kInt32_3,			// The language of the client that made this age
		kAgeID				= kUInt32_1,
		kAgeCzarID			= kUInt32_2,
		kAgeInfoFlags		= kUInt32_3,
	};

	enum AgeFlagBits	// 32 bits max
	{
		kIsStartupNeighborhood	= 1<<0,
	};

	plVaultAgeInfoNode();
	~plVaultAgeInfoNode();

	CLASSNAME_REGISTER( plVaultAgeInfoNode );
	GETINTERFACE_ANY( plVaultAgeInfoNode, plVaultNode );

	plVaultPlayerInfoListNode *	GetCanVisitFolder() const;
	plVaultPlayerInfoListNode * GetAgeOwnersFolder() const;
	plVaultFolderNode * GetChildAgesFolder( void ) const;
	plVaultAgeLinkNode * GetChildAgeLink( const plAgeInfoStruct * info ) const;
	const plVaultSDLNode *	GetAgeSDL() const { return IGetAgeSDL(); }
	plVaultSDLNode *	GetAgeSDL() { return IGetAgeSDL(); }
	plVaultPlayerInfoNode * GetCzar() const;

	const char * GetAgeFilename() const { return IGetString64_1(); }
	void	SetAgeFilename( const char * v ) { ISetString64_1( v ); }

	const char * GetAgeInstanceName() const { return IGetString64_2(); }
	void	SetAgeInstanceName( const char * v ) { ISetString64_2( v ); }

	const char * GetAgeUserDefinedName() const { return IGetString64_3(); }
	void	SetAgeUserDefinedName( const char * v ) { ISetString64_3( v ); }

	const plUUID * GetAgeInstanceGuid() const { fAgeInstanceGuid.FromString( IGetString64_4() ); return &fAgeInstanceGuid; }
	void	SetAgeInstanceGuid( const plUUID * guid ) { fAgeInstanceGuid.CopyFrom( guid ); ISetString64_4( fAgeInstanceGuid.AsString() ); }

	const char * GetAgeDescription() const { return IGetText_1(); }
	void	SetAgeDescription( const char * v ) { ISetText_1( v ); }

	Int32	GetSequenceNumber() const { return IGetInt32_1(); }
	void	SetSequenceNumber( Int32 v ) { ISetInt32_1( v ); }

	UInt32	GetAgeID() const { return IGetUInt32_1(); }
	void	SetAgeID( UInt32 v ) { ISetUInt32_1( v ); }

	// aka mayor
	UInt32	GetCzarID() const { return IGetUInt32_2(); }
	void	SetCzarID( UInt32 v ) { ISetUInt32_2( v ); }

	bool	IsPublic() const { return ( IGetInt32_2()!=0 ); }
	// no glue for this one. Use plVaultAgeMgr::SetAgePublic() instead.
	void	ISetPublic( bool v ) { ISetInt32_2( v ); }

	Int32	GetAgeLanguage() const { return IGetInt32_3(); }
	void	SetAgeLanguage( Int32 v ) { ISetInt32_3( v ); }

	bool	GetAgeFlag( UInt32 bit ) const { return ( IGetUInt32_3()&bit )!=0; }
	void	SetAgeFlag( UInt32 bit, bool on=true )
	{	if ( on ) ISetUInt32_3( IGetUInt32_3()|bit );
		else ISetUInt32_3( IGetUInt32_3()&~bit );
	}

	// helpers for linking.
	const plNetServerSessionInfo * AsServerInfo() const;
	const plAgeInfoStruct * AsAgeInfoStruct() const;
	// helpers for init
	void	FromAgeInfoStruct( const plAgeInfoStruct * ageInfo );
	// other helpers
	const char * GetDisplayName() const;

	// debug
	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultAgeInfoListNode : public plVaultFolderNode
{
public:
	enum FieldMap
	{
	};

	plVaultAgeInfoListNode();
	~plVaultAgeInfoListNode();
	CLASSNAME_REGISTER( plVaultAgeInfoListNode );
	GETINTERFACE_ANY( plVaultAgeInfoListNode, plVaultFolderNode );
	bool	HasAge( UInt32 AgeID );
	bool	AddAge( UInt32 AgeID );
	void	RemoveAge( UInt32 AgeID );
};

////////////////////////////////////////////////////////////////////

class plVaultAgeLinkNode : public plVaultNode
{
	mutable plVaultAgeInfoNode *	fAgeInfo;

public:
	enum FieldMap
	{
		kLocked				= kInt32_1,		// locked on psnl bookshelf
		kVolatile			= kInt32_2,		// volatile on psnl bookshelf
		kSpawnPoints		= kBlob_1,		// aka treasure stones.
	};

	plVaultAgeLinkNode();
	~plVaultAgeLinkNode();
	CLASSNAME_REGISTER( plVaultAgeLinkNode );
	GETINTERFACE_ANY( plVaultAgeLinkNode, plVaultNode );

	plVaultAgeInfoNode *	GetAgeInfo() const;
	void	SetAgeInfo( plVaultAgeInfoNode * v );
	// locked on psnl age bookshelf
	void	SetLocked( bool v ) { ISetInt32_1( v?1:0 ); }
	bool	GetLocked() const { return ( IGetInt32_1()!=0 ); }
	// volatile on psnl age bookshelf
	void	SetVolatile( bool v ) { ISetInt32_2( v?1:0 ); }
	bool	GetVolatile() const { return ( IGetInt32_2()!=0 ); }
	// spawn points
	void	AddSpawnPoint( const plSpawnPointInfo & point ); // will only add if not there already.
	void	RemoveSpawnPoint( const char * spawnPtName );
	bool	HasSpawnPoint( const char * spawnPtName ) const;
	bool	HasSpawnPoint( const plSpawnPointInfo & point ) const;	// compares spawn name only, not title.
	void	GetSpawnPoints( plSpawnPointVec & out ) const;
	void	SetSpawnPoints( const plSpawnPointVec & in );

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultChronicleNode : public plVaultNode
{
public:
	enum FieldMap
	{
		kEntryType		= kInt32_1,
		kEntryName		= kString64_1,
		kEntryValue		= kText_1,
	};

	plVaultChronicleNode();
	~plVaultChronicleNode();
	CLASSNAME_REGISTER( plVaultChronicleNode );
	GETINTERFACE_ANY( plVaultChronicleNode, plVaultNode );

	void SetName( const char * text )	{ ISetString64_1( text ); }
	const char * GetName( void ) const	{ return IGetString64_1(); }
	void SetValue( const char * text )	{ ISetText_1( text ); }
	const char * GetValue( void ) const	{ return IGetText_1(); }
	void SetEntryType( UInt32 type )	{ ISetInt32_1( type ); }
	UInt32 GetEntryType( void ) const	{ return IGetInt32_1(); }

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultPlayerInfoNode : public plVaultNode
{
	mutable plUUID	fGuid;	// used with GetAgeGuid() only.

public:
	enum FieldMap
	{
		kPlayerID			= kUInt32_1,
		kPlayerName			= kIString64_1,
		kAgeInstanceName	= kString64_1,	// name of age player is currently in
		kAgeInstanceGuid	= kString64_2,	// guid of age player is currently in
		kOnline				= kInt32_1,		// whether or not player is online
	};

	plVaultPlayerInfoNode();
	~plVaultPlayerInfoNode();
	CLASSNAME_REGISTER( plVaultPlayerInfoNode );
	GETINTERFACE_ANY( plVaultPlayerInfoNode, plVaultNode );

	// player  ID
	void	SetPlayerID( UInt32 v );
	UInt32	GetPlayerID( void ) const			{ return IGetUInt32_1(); }
	// player name
	void	SetPlayerName( const char * text );
	const char * GetPlayerName( void ) const	{ return IGetIString64_1(); }
	// age the player is currently in, if any.
	void	SetAgeInstanceName( const char * v );
	const char * GetAgeInstanceName( void ) const;
	void	SetAgeGuid( const plUUID * v);
	const plUUID * GetAgeGuid( void ) const;
	// online status
	void	SetOnline( bool b );
	bool	IsOnline( void ) const				{ return IGetInt32_1()!=0; }

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultPlayerInfoListNode : public plVaultFolderNode
{
	void	ISortPlayers( plNodeRefCompareProc proc );
	void	ILocalNodeAdded( plVaultNodeRef * nodeRef );

public:
	enum FieldMap
	{
	};

	plVaultPlayerInfoListNode();
	~plVaultPlayerInfoListNode();
	CLASSNAME_REGISTER( plVaultPlayerInfoListNode );
	GETINTERFACE_ANY( plVaultPlayerInfoListNode, plVaultFolderNode );
	bool	HasPlayer( UInt32 playerID );
	bool	AddPlayer( UInt32 playerID );
	void	RemovePlayer( UInt32 playerID );
	plVaultPlayerInfoNode * GetPlayer( UInt32 playerID );
	void	SetPlayers( const plVault::IDVec & playerIDs );
	void	SortBy( plNodeRefCompareProc proc ) { ISortPlayers( proc ); }
	void	Sort();
};

////////////////////////////////////////////////////////////////////

class plVaultMgrNode : public plVaultNode
{
	friend class plVaultServer;
	friend class plVaultConnectTask;
	friend class plVNodeMgrInitializationTask;

	mutable plVaultFolderNode *	fMyInboxFolder;
	mutable plVaultSystemNode *	fSystemNode;
public:
	enum FieldMap
	{
	};

	plVaultMgrNode();
	~plVaultMgrNode();
	CLASSNAME_REGISTER( plVaultMgrNode );
	GETINTERFACE_ANY( plVaultMgrNode, plVaultNode );
	plVaultFolderNode *	GetInbox( void ) const;
	plVaultSystemNode * GetSystemNode() const;
};

////////////////////////////////////////////////////////////////////

class plVaultPlayerNode : public plVaultMgrNode
{
	mutable plVaultPlayerInfoNode *	fPlayerInfo;
	mutable plVaultFolderNode *		fAvatarOutfitFolder;
	mutable plVaultFolderNode *		fAvatarClosetFolder;
	mutable plVaultFolderNode *		fChronicleFolder;
	mutable plVaultFolderNode *		fAgeJournalsFolder;
	mutable plVaultPlayerInfoListNode * fIgnoreListFolder;
	mutable plVaultPlayerInfoListNode * fBuddyListFolder;
	mutable plVaultPlayerInfoListNode * fPeopleIKnowAboutFolder;
	mutable plVaultFolderNode *		fAgesICanVisitFolder;
	mutable plVaultFolderNode *		fAgesIOwnFolder;
	mutable plVaultFolderNode *		fInviteFolder;
	mutable plUUID	fAcctUUID;

	plVaultAgeLinkNode * IFindLink( plVaultFolderNode * folder, const plAgeInfoStruct * info ) const;
	void	IRemoveLink( plVaultFolderNode * folder, const plUUID * guid );

public:
	enum FieldMap
	{
		kPlayerName			= kIString64_1,
		kAvatarShapeName	= kString64_1,
		kDisabled			= kInt32_2,
		kOnlineTime			= kUInt32_2,		// seconds
		kAccountUUID		= kIString64_2,
	};

	plVaultPlayerNode();
	~plVaultPlayerNode();
	CLASSNAME_REGISTER( plVaultPlayerNode );
	GETINTERFACE_ANY( plVaultPlayerNode, plVaultMgrNode );

	plVaultPlayerInfoNode *	GetPlayerInfo( void ) const;
	plVaultFolderNode *		GetAvatarOutfitFolder( void ) const;
	plVaultFolderNode *		GetAvatarClosetFolder( void ) const;
	plVaultFolderNode *		GetChronicleFolder( void ) const;
	plVaultFolderNode *		GetAgeJournalsFolder( void ) const;
	plVaultPlayerInfoListNode * GetIgnoreListFolder( void ) const;
	plVaultPlayerInfoListNode * GetBuddyListFolder( void ) const;
	plVaultPlayerInfoListNode * GetPeopleIKnowAboutFolder( void ) const;
	plVaultFolderNode *		GetAgesICanVisitFolder( void ) const;
	plVaultFolderNode *		GetAgesIOwnFolder( void ) const;
	plVaultFolderNode *		GetInviteFolder( void ) const;

	plVaultAgeLinkNode *	GetLinkToMyNeighborhood() const;
	plVaultAgeLinkNode *	GetLinkToCity() const;

	///////////////
	// Owned ages
	plVaultAgeLinkNode * GetOwnedAgeLink( const plAgeInfoStruct * info, bool skipVolatile=false ) const;
	void	RemoveOwnedAgeLink( const plUUID * guid );
	// Visit ages
	plVaultAgeLinkNode * GetVisitAgeLink( const plAgeInfoStruct * info ) const;
	void	RemoveVisitAgeLink( const plUUID * guid );
	///////////////
	// Chronicle
	plVaultChronicleNode * FindChronicleEntry( const char * entryName );

	// player name
	void	SetPlayerName( const char * v );
	const char * GetPlayerName( void ) const	{ return IGetIString64_1(); }
	// avatar shape name
	void	SetAvatarShapeName( const char * v );
	const char * GetAvatarShapeName( void ) const{return IGetString64_1(); }
	// disabled?
	void	SetDisabled( bool v )				{ ISetInt32_2( v ); }
	bool	IsDisabled( void ) const			{ return IGetInt32_2()!=0; }
	// account ID
	void	SetAccountUUID( const plUUID * v );
	const plUUID * GetAccountUUID( void ) const;
	// online time accumulator (plUnifiedTime.GetSecs())
	void	SetOnlineTime( UInt32 v )			{ ISetUInt32_2( v ); }
	UInt32	GetOnlineTime( void ) const			{ return IGetUInt32_2(); }
	void	IncOnlineTime( UInt32 v )			{ SetOnlineTime( GetOnlineTime()+v ); }

	// override to also save player info node.
	void Save(
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultAgeNode : public plVaultMgrNode
{
	mutable	plUUID			fAgeGuid;

	mutable plVaultAgeInfoNode *		fAgeInfo;
	mutable plVaultFolderNode *			fAgeDevicesFolder;
	mutable plVaultFolderNode *			fSubAgesFolder;
	mutable plVaultPlayerInfoListNode * fPeopleIKnowAboutFolder;
	mutable plVaultFolderNode *			fChronicleFolder;
	// used only with personal ages
	mutable plVaultFolderNode *			fAgesIOwnFolder;
	// used only with nexus age
	mutable plVaultFolderNode *			fPublicAgesFolder;

	plVaultAgeLinkNode * IGetLink( plVaultFolderNode * folder, const plAgeInfoStruct * info ) const;

public:
	enum FieldMap
	{
		kAgeGuid		= kString64_1,
	};

	plVaultAgeNode();
	~plVaultAgeNode();
	CLASSNAME_REGISTER( plVaultAgeNode );
	GETINTERFACE_ANY( plVaultAgeNode, plVaultMgrNode );

	plVaultAgeInfoNode *		GetAgeInfo() const;

	plVaultFolderNode *			GetAgeDevicesFolder( void ) const;
	plVaultFolderNode *			GetSubAgesFolder( void ) const;
	// age chronicle
	plVaultFolderNode *			GetChronicleFolder( void ) const;
	// People who have published to devices in this age
	plVaultPlayerInfoListNode *	GetPeopleIKnowAboutFolder( void ) const;

	// PERSONAL AGE SPECIFIC
	plVaultFolderNode *			GetAgesIOwnFolder( void ) const;

	// NEXUS AGE SPECIFIC
	plVaultFolderNode  *		GetPublicAgesFolder( void ) const;

	// To publish a node to a device, get its device inbox ID, then
	// call node->SendTo( deviceInboxID );
	plVaultAgeLinkNode *		GetSubAgeLink( const plAgeInfoStruct * info ) const;

	// AGE DEVICES. AKA IMAGERS, WHATEVER.
	// Add a new device.
	void AddDevice( const char * deviceName,
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );
	// Remove a device.
	void RemoveDevice( const char * deviceName );
	// True if device exists in age.
	bool HasDevice( const char * deviceName );
	// Get a device.
	plVaultTextNoteNode * GetDevice( const char * deviceName );

	// age guid
	const plUUID * GetAgeGuid() const { fAgeGuid.FromString( IGetString64_1() ); return &fAgeGuid; }
	void	SetAgeGuid( const plUUID * guid );
	
	// Age chronicle
	plVaultChronicleNode * FindChronicleEntry( const char * entryName );

	// override to also save age info node.
	void Save(
		plVaultOperationCallback * cb=nil,
		UInt32 cbContext=0 );

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////

class plVaultAdminNode : public plVaultMgrNode
{
	mutable plVaultFolderNode *	fAllAgeSDLEventInboxesFolder;

public:
	enum FieldMap
	{
	};

	plVaultAdminNode();
	~plVaultAdminNode();
	CLASSNAME_REGISTER( plVaultAdminNode );
	GETINTERFACE_ANY( plVaultAdminNode, plVaultMgrNode );

	plVaultFolderNode *	GetAllAgeSDLEventInboxesFolder( void ) const;

	std::string	AsStdString( int level=0 ) const;
};


////////////////////////////////////////////////////////////////////

class plVaultMarkerNode : public plVaultNode
{
public:
	enum FieldMap
	{
		kAgeName			= kCreateAgeName,
		kMarkerText			= kText_1,
		kGPSTorans			= kInt32_1,
		kGPSHSpans			= kInt32_2,
		kGPSVSpans			= kInt32_3,
		kMarkerPosX			= kUInt32_1,
		kMarkerPosY			= kUInt32_2,
		kMarkerPosZ			= kUInt32_3,
	};

	plVaultMarkerNode();
	~plVaultMarkerNode();
	CLASSNAME_REGISTER(plVaultMarkerNode);
	GETINTERFACE_ANY(plVaultMarkerNode, plVaultNode);

	void SetAge(const char* ageName) { ISetCreateAgeName(ageName); }
	const char* GetAge() const { return IGetCreateAgeName(); }

	void SetPosition(const hsPoint3& pos);
	hsPoint3 GetPosition() const;

	void SetText(const char* text) { ISetText_1(text); }
	const char* GetText() const { return IGetText_1(); }

	void SetGPS(Int32 t, Int32 h, Int32 v) { ISetInt32_1(t);ISetInt32_2(h);ISetInt32_3(v); }
	Int32 GetGPSTorans() const { return IGetInt32_1(); }
	Int32 GetGPSHSpans() const { return IGetInt32_2(); }
	Int32 GetGPSVSpans() const { return IGetInt32_3(); }

	std::string	AsStdString(int level=0) const;
};

////////////////////////////////////////////////////////////////////

class plVaultMarkerListNode : public plVaultFolderNode
{
public:
	enum FieldMap
	{
		kOwnerID			= kUInt32_1,
		kOwnerName			= kString64_2,
		kGameType			= kInt32_1,
		kRoundLength		= kInt32_2,
	};

	plVaultMarkerListNode();
	~plVaultMarkerListNode();

	CLASSNAME_REGISTER(plVaultMarkerListNode);
	GETINTERFACE_ANY(plVaultMarkerListNode, plVaultFolderNode);

	void	SetOwnerName( const char * v )	{ ISetString64_2( v ); }
	const char * GetOwnerName( void ) const{ return IGetString64_2();}

	Int32	GetGameType() const { return IGetInt32_1(); }
	void	SetGameType( Int32 v ) { ISetInt32_1( v ); }

	Int32	GetRoundLength() const { return IGetInt32_2(); }
	void	SetRoundLength( Int32 v ) { ISetInt32_2( v ); }

	std::string	AsStdString(int level=0) const;

};

////////////////////////////////////////////////////////////////////

//
// A node that ALL vault mgrs get - for system wide info
//
class plVaultSystemNode : public plVaultNode
{
	mutable plVaultFolderNode *	fGlobalInboxFolder;

public:
	enum FieldMap
	{
		kCCRAwayStatus		= kInt32_1
	};

	plVaultSystemNode();
	CLASSNAME_REGISTER( plVaultSystemNode);
	GETINTERFACE_ANY( plVaultSystemNode, plVaultNode );

	void	SetCCRAwayStatus( bool b)			{ ISetInt32_1( b); }
	int		GetCCRAwayStatus( void ) const		{ return IGetInt32_1();}

	plVaultFolderNode *	GetGlobalInbox( void ) const;

	std::string	AsStdString( int level=0 ) const;
};

////////////////////////////////////////////////////////////////////
// Vault Server VNode
// Makes sure nodes like AllPlayers exist.

class plNetVaultServerNode : public plVaultMgrNode
{
	friend class plNetVaultServerInitializationTask;

	mutable plVaultAgeInfoNode * fCityInfo;
	UInt32		fAllPlayersFolderID;
	UInt32		fAllAgeGlobalSDLNodesFolderID;
	UInt32		fPublicAgesFolderID;

public:
	plNetVaultServerNode();
	~plNetVaultServerNode();
	CLASSNAME_REGISTER( plNetVaultServerNode );
	GETINTERFACE_ANY( plNetVaultServerNode, plVaultMgrNode );

	UInt32		GetAllPlayersFolderID() const { return fAllPlayersFolderID; }
	UInt32		GetAllAgeGlobalSDLNodesFolderID() const { return fAllAgeGlobalSDLNodesFolderID; }
	UInt32		GetPublicAgesFolderID() const { return fPublicAgesFolderID; }

	std::string	AsStdString( int level=0 ) const;
};

#endif // 0
//============================================================================
//============================================================================
//============================================================================
//============================================================================


#endif // plVaultNode_h_inc

#endif
