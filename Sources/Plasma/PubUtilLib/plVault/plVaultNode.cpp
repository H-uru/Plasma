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
#include "hsTemplates.h"
#include "hsStlUtils.h"
#include "plVaultNode.h"
#include "plVaultNodeIterator.h"
#include "../pnMessage/plMessage.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plNetCommon/plNetCommon.h"
#include "../plStatusLog/plStatusLog.h"
#include "../plSDL/plSDL.h"
#include <sstream>
#include "hsGeometry3.h"
#include "hsStringTokenizer.h"

////////////////////////////////////////////////////////////////////

#define plSafeGetLog()	if (fMyNodeMgr) fMyNodeMgr->GetStatusLog()


////////////////////////////////////////////////////////////////////


bool plVaultNode::MatchesNode::operator()( const plVaultNode * node ) const
{
	if ( !node )
		return false;

	bool matches = true;

	if ( fNode->IIsFieldFlagSet( plVaultNode::kID ) )
	{
		matches &= ( fNode->GetID()==node->GetID() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kType ) )
	{
		matches &= ( fNode->GetType()==node->GetType() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kPermissions ) )
	{
		matches &= ( fNode->GetPermissions()==node->GetPermissions() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kOwnerNodeID ) )
	{
		matches &= ( fNode->GetOwnerNodeID()==node->GetOwnerNodeID() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kGroupNodeID ) )
	{
		matches &= ( fNode->GetGroupNodeID()==node->GetGroupNodeID() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kModifyTime ) )
	{
		matches &= ( *fNode->GetModifyTime()==*node->GetModifyTime() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kCreatorNodeID ) )
	{
		matches &= ( fNode->GetCreatorNodeID()==node->GetCreatorNodeID() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kCreateTime ) )
	{
		matches &= ( *fNode->GetCreateTime()==*node->GetCreateTime() );
	}
//	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kCreateAgeCoords ) )
//	{
//		hsAssert( false, "Compare age coords not supported" );
//	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kCreateAgeTime ) )
	{
		matches &= ( *fNode->GetCreateAgeTime()==*node->GetCreateAgeTime() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kCreateAgeName ) )
	{
		matches &= ( strcmp( fNode->GetCreateAgeName(), node->GetCreateAgeName() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kCreateAgeGuid ) )
	{
		matches &= ( fNode->GetCreateAgeGuid()->IsEqualTo( node->GetCreateAgeGuid() ) );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kInt32_1 ) )
	{
		matches &= ( fNode->IGetInt32_1()==node->IGetInt32_1() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kInt32_2 ) )
	{
		matches &= ( fNode->IGetInt32_2()==node->IGetInt32_2() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kInt32_3 ) )
	{
		matches &= ( fNode->IGetInt32_3()==node->IGetInt32_3() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kInt32_4 ) )
	{
		matches &= ( fNode->IGetInt32_4()==node->IGetInt32_4() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kUInt32_1 ) )
	{
		matches &= ( fNode->IGetUInt32_1()==node->IGetUInt32_1() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kUInt32_2 ) )
	{
		matches &= ( fNode->IGetUInt32_2()==node->IGetUInt32_2() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kUInt32_3 ) )
	{
		matches &= ( fNode->IGetUInt32_3()==node->IGetUInt32_3() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kUInt32_4 ) )
	{
		matches &= ( fNode->IGetUInt32_4()==node->IGetUInt32_4() );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kString64_1 ) )
	{
		matches &= ( strcmp( fNode->IGetString64_1(), node->IGetString64_1() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kString64_2 ) )
	{
		matches &= ( strcmp( fNode->IGetString64_2(), node->IGetString64_2() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kString64_3 ) )
	{
		matches &= ( strcmp( fNode->IGetString64_3(), node->IGetString64_3() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kString64_4 ) )
	{
		matches &= ( strcmp( fNode->IGetString64_4(), node->IGetString64_4() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kString64_5 ) )
	{
		matches &= ( strcmp( fNode->IGetString64_5(), node->IGetString64_5() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kString64_6 ) )
	{
		matches &= ( strcmp( fNode->IGetString64_6(), node->IGetString64_6() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kIString64_1 ) )
	{
		matches &= ( stricmp( fNode->IGetIString64_1(), node->IGetIString64_1() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kIString64_2 ) )
	{
		matches &= ( stricmp( fNode->IGetIString64_2(), node->IGetIString64_2() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kText_1 ) )
	{
		matches &= ( strcmp( fNode->IGetText_1(), node->IGetText_1() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kText_2 ) )
	{
		matches &= ( strcmp( fNode->IGetText_2(), node->IGetText_2() )==0 );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kBlob_1 ) )
	{
//		hsAssert( false, "Compare blobs not supported" );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kBlob_2 ) )
	{
//		hsAssert( false, "Compare blobs not supported" );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kBlob_1_Guid ) )
	{
		matches &= ( fNode->IGetBlob_1_Guid()->IsEqualTo( node->IGetBlob_1_Guid() ) );
	}
	if ( matches && fNode->IIsFieldFlagSet( plVaultNode::kBlob_2_Guid ) )
	{
		matches &= ( fNode->IGetBlob_2_Guid()->IsEqualTo( node->IGetBlob_2_Guid() ) );
	}

	return matches;
}

bool plVaultNode::MatchesNode::operator()( const plVaultNodeRef * nodeRef ) const
{
	return operator()( nodeRef->GetChild() );
}

plVaultNode::plVaultNode()
: fID( 0 )
, fType( 0 )
, fPermissions( 0 )
, fOwnerNodeID( 0 )
, fGroupNodeID( 0 )
, fCreatorNodeID( 0 )
, fInt32_1( 0 )
, fInt32_2( 0 )
, fInt32_3( 0 )
, fInt32_4( 0 )
, fUInt32_1( 0 )
, fUInt32_2( 0 )
, fUInt32_3( 0 )
, fUInt32_4( 0 )
, fOwnerNode( nil )
, fGroupNode( nil )
, fCreatorNode( nil )
, fPersistent( true )
, fSaving( false )
, fInitialized( false )
, fUserData( 0 )
, fMyNodeMgr( nil )
{
}

plVaultNode::~plVaultNode()
{
	IFailPendingSaves();
	std::for_each( fChildNodes.begin(), fChildNodes.end(), xtl::delete_ptr() );
	fChildNodes.clear();
}

bool plVaultNode::IIsFieldFlagSet( int b ) const
{
	return fFieldFlags.IsBitSet( b )?true:false;
}

void plVaultNode::IGotUpdated( void )
{
	hsLogEntry( plSafeGetLog()->AddLineF( "Updated remotely: %s", AsStdString().c_str() ) );
}


UInt32 plVaultNode::GetID( void ) const
{
	return fID;
}

void plVaultNode::ISetID( UInt32 v )
{
	ISetFieldFlag( kID );
	fID = v;
}


const plVaultPlayerInfoNode * plVaultNode::GetOwnerNode( void ) const
{
	if (!fMyNodeMgr)
		return nil;
		
	plVaultPlayerInfoNode templateNode;
	templateNode.SetPlayerID( fOwnerNodeID );
	plVaultNode * result;
	fMyNodeMgr->FindNode( &templateNode, result );
	return plVaultPlayerInfoNode::ConvertNoRef( result );
}


const plVaultPlayerInfoNode * plVaultNode::GetCreatorNode( void ) const
{
	if (!fMyNodeMgr)
		return nil;

	plVaultPlayerInfoNode templateNode;
	templateNode.SetPlayerID( fCreatorNodeID );
	plVaultNode * result;
	fMyNodeMgr->FindNode( &templateNode, result );
	return plVaultPlayerInfoNode::ConvertNoRef( result );
}

const plVaultNode * plVaultNode::GetGroupNode( void ) const
{
	return nil;
}


bool plVaultNode::IHasFieldFlagsSet( void )
{
	fFieldFlags.Compact();
	return fFieldFlags.GetSize()>0;
}

bool plVaultNode::HasValidID( void ) const
{
	return ( fID>0 ) && ( !fPersistent || ( fPersistent && ( fID>=plVault::kFirstStoredNodeID ) ) );
}

bool plVaultNode::IsStored( void ) const
{
	if( !fMyNodeMgr )
	{
		return fID>plVault::kFirstStoredNodeID;
	}
	else
	{
		return ( fMyNodeMgr->IAmOnline() && ( ( fID>plVault::kFirstStoredNodeID ) || fSaving ) )
			|| ( !fMyNodeMgr->IAmOnline() );	// if not online then, sure... it's stored.. yeah.
	}
}

bool plVaultNode::IsMgrNode( void ) const
{
	return ( IsStored() && fType>plVault::kNodeType_VNodeMgrLow && fType<plVault::kNodeType_VNodeMgrHigh );
}

void plVaultNode::IDoPendingSave( void )
{
	if ( !fPendingSaves.empty() )
	{
		plVaultSaveNodeTask * task = fPendingSaves.front();
		fPendingSaves.erase( fPendingSaves.begin() );
		hsLogEntry(plSafeGetLog()->AddLineF( "Starting pending save operation on node %lu", GetID() ));
		task->Start();
	}
}

void plVaultNode::IFailPendingSaves()
{
	while ( !fPendingSaves.empty() )
	{
		plVaultSaveNodeTask * task = fPendingSaves.front();
		fPendingSaves.erase( fPendingSaves.begin() );
		hsLogEntry(plSafeGetLog()->AddLineF( "Aborting pending save operation on node %lu", GetID() ));
		task->TaskComplete( false );
	}
}

void plVaultNode::IAddPendingSave( plVaultSaveNodeTask * task )
{
	hsLogEntry(plSafeGetLog()->AddLineF( "Adding pending save operation to node %lu", GetID() ));
	fPendingSaves.push_back( task );
}


static bool CompareNodeRefCreateTimes( const plVaultNodeRef * A, const plVaultNodeRef * B )
{
	return ( A->GetCreateTime()->GetSecs()<B->GetCreateTime()->GetSecs() );
}

plVaultNodeRef * plVaultNode::ILocalAddNode( plVaultNode * node, bool notify )
{
	plVaultNodeRef nodeRef;
	nodeRef.ISetChildNode( node );
	return ILocalAddNode( &nodeRef, notify );
}

plVaultNodeRef * plVaultNode::ILocalAddNode( const plVaultNodeRef * in, bool notify )
{
	hsAssert( in->GetChildID()!=0, "ILocalAddNode: Invalid child ID" );

	if ( in->fChildID<plVault::kFirstStoredNodeID )
	{
		int xx=0;
	}

	if ( IIsAnUpstreamNode( in->GetChildID() ) )
	{
//		plSafeGetLog()->AddLine( "ILocalAddNode: node being added as a child exists above me in family tree!" );
		hsAssert( false, "ILocalAddNode: node being added as a child exists above me in family tree!" );
		return nil;
	}

	plVaultNodeRef * nodeRef;
	bool found = GetNode( in->GetChildID(), nodeRef );
	if ( !found && fMyNodeMgr)
	{
		nodeRef = fMyNodeMgr->CreateNodeRef();
		nodeRef->CopyFrom( in );
		nodeRef->ISetParentID( GetID() );
		if ( in->GetChild() )
			nodeRef->ISetChildNode( in->GetChild() );
		// add node
		fChildNodes.push_back( nodeRef );
		// sort nodes by create time
		std::sort( fChildNodes.begin(), fChildNodes.end(), CompareNodeRefCreateTimes );
		// call internal notify
		ILocalNodeAdded( nodeRef );
		hsLogEntry(plSafeGetLog()->AddLineF( "Added child %lu to parent %lu", nodeRef->GetChildID(), nodeRef->GetParentID() ));
		// do callbacks
		if ( notify )
			fMyNodeMgr->INotifyNodeRefAdded( nodeRef );
	}
	return nodeRef;
}

bool plVaultNode::ILocalRemoveNode( UInt32 nodeID, bool notify )
{
	plVaultNode tmp;
	tmp.ISetID( nodeID );
	plVault::NodeRefVec::iterator it =
		std::find_if( fChildNodes.begin(), fChildNodes.end(),
		plVaultNode::MatchesNode( &tmp ) );
	bool found = ( it!=fChildNodes.end() );
	if ( found && fMyNodeMgr)
	{
		plVaultNodeRef * nodeRef = *it;
		if ( notify )
			fMyNodeMgr->INotifyRemovingNodeRef( nodeRef );
		delete *it;
		fChildNodes.erase( it );
		if ( notify )
			fMyNodeMgr->INotifyNodeRefRemoved( nodeID, GetID() );
	}
	return found;
}

bool plVaultNode::IIsAnUpstreamNode( UInt32 nodeID )
{
	plVaultNode * node;
	if ( fMyNodeMgr && fMyNodeMgr->GetNode( nodeID, node ) )
		return node->IIsADownstreamNode( GetID() );
	return false;
}

bool plVaultNode::IIsADownstreamNode( UInt32 nodeID )
{
	for ( int i=0; i<fChildNodes.size(); i++ )
	{
		if ( fChildNodes[i]->GetChildID()==nodeID )
			return true;
		if ( fChildNodes[i]->GetChild()->IIsADownstreamNode( nodeID ) )
			return true;
	}
	return false;
}

plVaultNodeRef * plVaultNode::AddNode( plVaultNode * node, plVaultOperationCallback * cb, UInt32 cbContext )
{
	if ( IsPersistent() && !IsStored() && node->IsPersistent() )
	{
		std::string msg;
		xtl::format( msg, "Tried to add persistent child to non-saved persistent parent. parent fSaving=%s", fSaving?"yep":"nope" );
		hsLogEntry(plSafeGetLog()->AddLineF( plStatusLog::kRed, "%s", msg.c_str() ));
//		hsAssert( false, msg.c_str() );
		plVaultOperationCallbackHolder cbHolder( fMyNodeMgr, cb, cbContext );
		cbHolder.OperationComplete( hsFail );
		return false;
	}
	plVaultNodeRef * nodeRef;
	if ( FindNode( node, nodeRef ) )
	{
		// we already have this child, short circuit.
		plVaultOperationCallbackHolder cbHolder( fMyNodeMgr, cb, cbContext );
		cbHolder.SetCbFields( nodeRef->GetChild(), nodeRef );
		cbHolder.OperationComplete( hsOK );
		return nodeRef;
	}

	return fMyNodeMgr ? fMyNodeMgr->IAddNodeRef( node, GetID(), cb, cbContext ) : nil;		// starts network operation
}

bool plVaultNode::LinkToNode( const plVaultNode * templateNode, int childFetchLevel, bool allowCreate, plVaultOperationCallback * cb, UInt32 cbContext )
{
	if ( IsPersistent() && !IsStored() )
	{
		std::string msg;
		xtl::format( msg, "Tried to link a child to non-saved persistent parent. parent fSaving=%s", fSaving?"yep":"nope" );
		hsLogEntry(plSafeGetLog()->AddLineF( plStatusLog::kRed, "%s", msg.c_str() ));
//		hsAssert( false, msg.c_str() );
		plVaultOperationCallbackHolder cbHolder( fMyNodeMgr, cb, cbContext );
		cbHolder.OperationComplete( hsFail );
		return false;
	}
	plVaultNodeRef * nodeRef;
	if ( FindNode( templateNode, nodeRef ) )
	{
		// we already have this child, short circuit.
		plVaultOperationCallbackHolder cbHolder( fMyNodeMgr, cb, cbContext );
		cbHolder.SetCbFields( nodeRef->GetChild(), nodeRef );
		cbHolder.OperationComplete( hsOK );
		return true;
	}

	// Can't allowCreate if template specified an ID, unless we aren't online.
	allowCreate &= ( fMyNodeMgr && !templateNode->IIsFieldFlagSet( plVaultNode::kID ) || !fMyNodeMgr->IAmOnline() );

	fMyNodeMgr ? fMyNodeMgr->ILinkToNode( templateNode, childFetchLevel, allowCreate, GetID(), cb, cbContext ) : NULL_STMT;

	return true;
}

bool plVaultNode::LinkToNode( UInt32 nodeID, int childFetchLevel, plVaultOperationCallback * cb, UInt32 cbContext )
{
	plVaultNode templateNode;
	templateNode.ISetID( nodeID );
	return LinkToNode( &templateNode, childFetchLevel, false, cb,cbContext );
}

bool plVaultNode::RemoveNode( const plVaultNode * node, plVaultOperationCallback * cb/*=nil*/, UInt32 cbContext/*=0 */)
{
	// remove by id
	if ( node->GetID()!=0 )
		return RemoveNode( node->GetID(), cb, cbContext );
	// remove by template match
	plVaultNodeRef * nodeRef;
	if ( FindNode( node, nodeRef ) )
		return RemoveNode( nodeRef->GetChildID(), cb, cbContext );
	plVaultOperationCallbackHolder cbHolder( fMyNodeMgr, cb, cbContext );
	cbHolder.OperationComplete( hsFail );
	return false;
}

bool plVaultNode::RemoveNode( UInt32 nodeID, plVaultOperationCallback * cb/*=nil*/, UInt32 cbContext/*=0 */)
{
	fMyNodeMgr ? fMyNodeMgr->IRemoveNodeRef( nodeID, GetID(), cb, cbContext ) : NULL_STMT;// starts network operation
	return true;
}

void plVaultNode::GetNodes( plVault::NodeRefVec & out )
{
	std::copy( fChildNodes.begin(), fChildNodes.end(), std::back_inserter( out ) );
}

bool plVaultNode::GetNode( UInt32 nodeID, plVaultNodeRef *& out ) const
{
	plVaultNode tmp;
	tmp.ISetID( nodeID );
	return FindNode( &tmp, out );
}

bool plVaultNode::FindNode( const plVaultNode * templateNode, plVaultNodeRef *& out ) const
{
	plVaultNode tmp;
	if ( templateNode->IsStored() )
		tmp.ISetID( templateNode->GetID() );
	else
		tmp.CopyFrom( templateNode );

	out = nil;
	plVault::NodeRefVec::const_iterator it =
		std::find_if( fChildNodes.begin(), fChildNodes.end(),
		plVaultNode::MatchesNode( &tmp ) );
	bool found = ( it!=fChildNodes.end() );
	if ( found )
		out = *it;
	return found;
}

bool plVaultNode::FindNodes( const plVaultNode * templateNode, plVault::NodeRefVec & out ) const
{
	plVaultNode tmp;
	if ( templateNode->IsStored() )
		tmp.ISetID( templateNode->GetID() );
	else
		tmp.CopyFrom( templateNode );

	int n = out.size();
	MatchesNode matcher( &tmp );
	for ( int i=0; i<fChildNodes.size(); i++ )
		if ( matcher( fChildNodes[i] ) )
			out.push_back( fChildNodes[i] );
	return ( out.size()-n > 0 );
}

bool plVaultNode::FindNodeRecurse( const plVaultNode * templateNode, plVaultNodeRef *& out ) const
{
	if ( !FindNode( templateNode, out ) )
	{
		for ( plVault::NodeRefVec::const_iterator it=fChildNodes.begin(); it!=fChildNodes.end(); ++it )
		{
			if ( (*it)->GetChild()->FindNodeRecurse( templateNode, out ) )
				return true;
		}
	}
	return false;
}

bool plVaultNode::HasNode( UInt32 nodeID )
{
	plVaultNode templateNode;
	templateNode.SetID( nodeID );
	return HasNode( &templateNode );
}

bool plVaultNode::HasNode( const plVaultNode * templateNode )
{
	plVaultNodeRef * nodeRef;
	return FindNode( templateNode, nodeRef );
}

bool plVaultNode::Modified( void ) const
{
	fFieldFlags.Compact();
	return ( fFieldFlags.GetSize()>0 );
}

void plVaultNode::SetModified( bool b )
{
	if ( !b )
	{
		IClearAllFieldFlags();
	}
	else
	{
		ISetAllFieldFlags();
		SetModifyTime( &plUnifiedTime::GetCurrentTime() );
	}
}



UInt32 plVaultNode::GetClientID( void ) const
{
	if ( !fMyNodeMgr )
		return 0;
	return fMyNodeMgr->GetClientID();
}


void plVaultNode::Save( plVaultOperationCallback * cb, UInt32 cbContext )
{
	if (Modified() && fMyNodeMgr)
	{
		fMyNodeMgr->ISaveNode( this, cb, cbContext );
	}
	else
	{
		plVaultOperationCallbackHolder cbHolder( fMyNodeMgr, cb, cbContext );
		cbHolder.OperationComplete( hsOK );
	}
}

void plVaultNode::SaveAll( plVaultOperationCallback * cb, UInt32 cbContext )
{
	for ( int i=0; i<fChildNodes.size(); i++ )
	{
		plVaultNodeRef * nodeRef = fChildNodes[i];
		plVaultNode * child = nodeRef->GetChild();
		child->SaveAll( cb, cbContext );
	}
	Save( cb, cbContext );
}


void plVaultNode::SendTo( UInt32 destClientNodeID, plVaultOperationCallback * cb, UInt32 cbContext )
{
	if (!fMyNodeMgr)
		return;
	fMyNodeMgr->ISendNode( this, destClientNodeID, cb, cbContext );
}


void plVaultNode::RemoveAllNodes( void )
{
	plVault::IDVec ids;
	for ( plVaultNodeIterator nodeIt=GetIterator(); nodeIt; ++nodeIt )
	{
		ids.push_back( (*nodeIt)->GetChildID() );
	}
	for ( int i=0; i<ids.size(); i++ )
	{
		RemoveNode( ids[i] );
	}
}


plVaultNodeIterator plVaultNode::GetIterator( void )
{
	return plVaultNodeIterator( this );
}


// NOTE: IF YOU CHANGE plVaultNode STREAM FORMAT, BE SURE TO
// INCREMENT VaultNodeVersion (ABOVE).
void plVaultNode::Read( hsStream * s, hsResMgr * mgr )
{
	hsAssert( s, "nil stream" );

	fFieldFlags.Read( s );

	// these fields are always streamed
	s->ReadSwap( &fID );
	s->ReadSwap( &fType );
	s->ReadSwap( &fPermissions );
	s->ReadSwap( &fOwnerNodeID );
	s->ReadSwap( &fGroupNodeID );
	fModifyTime.Read( s );
	// set these to nil in case their corresponding ID changed above.
	fOwnerNode = nil;
	fGroupNode = nil;

	// fields relating to creation of this node
	if ( IIsFieldFlagSet( kCreatorNodeID ) )
	{
		s->ReadSwap( &fCreatorNodeID );
		fCreatorNode = nil;
	}
	if ( IIsFieldFlagSet( kCreateTime ) )
	{
		fCreateTime.Read( s );
	}
//	if ( IIsFieldFlagSet( kCreateAgeCoords ) )
//	{
//		fCreateAgeCoords.Read( s, mgr );
//	}
	if ( IIsFieldFlagSet( kCreateAgeTime ) )
	{
		fCreateAgeTime.Read( s );
	}
	if ( IIsFieldFlagSet( kCreateAgeName ) )
	{
		plMsgStdStringHelper::Peek( fCreateAgeName, s );
	}
	if ( IIsFieldFlagSet( kCreateAgeGuid ) )
	{
		fCreateAgeGuid.Read( s );
	}
	// generic fields
	if ( IIsFieldFlagSet( kInt32_1 ) )
	{
		s->ReadSwap( &fInt32_1 );
	}
	if ( IIsFieldFlagSet( kInt32_2 ) )
	{
		s->ReadSwap( &fInt32_2 );
	}
	if ( IIsFieldFlagSet( kInt32_3 ) )
	{
		s->ReadSwap( &fInt32_3 );
	}
	if ( IIsFieldFlagSet( kInt32_4 ) )
	{
		s->ReadSwap( &fInt32_4 );
	}
	if ( IIsFieldFlagSet( kUInt32_1 ) )
	{
		s->ReadSwap( &fUInt32_1 );
	}
	if ( IIsFieldFlagSet( kUInt32_2 ) )
	{
		s->ReadSwap( &fUInt32_2 );
	}
	if ( IIsFieldFlagSet( kUInt32_3 ) )
	{
		s->ReadSwap( &fUInt32_3 );
	}
	if ( IIsFieldFlagSet( kUInt32_4 ) )
	{
		s->ReadSwap( &fUInt32_4 );
	}
	if ( IIsFieldFlagSet( kString64_1 ) )
	{
		plMsgStdStringHelper::Peek( fString64_1, s );
	}
	if ( IIsFieldFlagSet( kString64_2 ) )
	{
		plMsgStdStringHelper::Peek( fString64_2, s );
	}
	if ( IIsFieldFlagSet( kString64_3 ) )
	{
		plMsgStdStringHelper::Peek( fString64_3, s );
	}
	if ( IIsFieldFlagSet( kString64_4 ) )
	{
		plMsgStdStringHelper::Peek( fString64_4, s );
	}
	if ( IIsFieldFlagSet( kString64_5 ) )
	{
		plMsgStdStringHelper::Peek( fString64_5, s );
	}
	if ( IIsFieldFlagSet( kString64_6 ) )
	{
		plMsgStdStringHelper::Peek( fString64_6, s );
	}
	if ( IIsFieldFlagSet( kIString64_1 ) )
	{
		plMsgStdStringHelper::Peek( fIString64_1, s );
	}
	if ( IIsFieldFlagSet( kIString64_2 ) )
	{
		plMsgStdStringHelper::Peek( fIString64_2, s );
	}
	if ( IIsFieldFlagSet( kText_1 ) )
	{
		plMsgStdStringHelper::Peek( fText_1, s );
	}
	if ( IIsFieldFlagSet( kText_2 ) )
	{
		plMsgStdStringHelper::Peek( fText_2, s );
	}
	if ( IIsFieldFlagSet( kBlob_1 ) )
	{
		UInt32 buflen;
		s->ReadSwap( &buflen );
		IAllocBufferBlob_1( buflen );
		if (buflen)
			s->Read( buflen,(void*)IGetBufferBlob_1() );
	}
	if ( IIsFieldFlagSet( kBlob_2 ) )
	{
		UInt32 buflen;
		s->ReadSwap( &buflen );
		IAllocBufferBlob_2( buflen );
		if (buflen)
			s->Read( buflen,(void*)IGetBufferBlob_2() );
	}
	if ( IIsFieldFlagSet( kBlob_1_Guid ) )
	{
		fBlob_1_Guid.Read( s );
	}
	if ( IIsFieldFlagSet( kBlob_2_Guid ) )
	{
		fBlob_2_Guid.Read( s );
	}
}

// NOTE: IF YOU CHANGE plVaultNode STREAM FORMAT, BE SURE TO
// INCREMENT VaultNodeVersion (ABOVE).
void plVaultNode::Write( hsStream * s, hsResMgr * mgr )
{
	hsAssert( s, "nil stream" );
#ifdef HS_BUILD_FOR_UNIX
	plOperationTimer t( "plVaultNode::Write" );
	t.Start( "write operation", 2 );
#endif


	fFieldFlags.Write( s );

	// these fields are always streamed
	s->WriteSwap( fID );
	s->WriteSwap( fType );
	s->WriteSwap( fPermissions );
	s->WriteSwap( fOwnerNodeID );
	s->WriteSwap( fGroupNodeID );
	fModifyTime.Write( s );

	// fields relating to creation of this node
	if ( IIsFieldFlagSet( kCreatorNodeID ) )
	{
		s->WriteSwap( fCreatorNodeID );
	}
	if ( IIsFieldFlagSet( kCreateTime ) )
	{
		fCreateTime.Write( s );
	}
//	if ( IIsFieldFlagSet( kCreateAgeCoords ) )
//	{
//		fCreateAgeCoords.Write( s, mgr );
//	}
	if ( IIsFieldFlagSet( kCreateAgeTime ) )
	{
		fCreateAgeTime.Write( s );
	}
	if ( IIsFieldFlagSet( kCreateAgeName ) )
	{
		plMsgStdStringHelper::Poke( fCreateAgeName, s );
	}
	if ( IIsFieldFlagSet( kCreateAgeGuid ) )
	{
		fCreateAgeGuid.Write( s );
	}
	// generic fields
	if ( IIsFieldFlagSet( kInt32_1 ) )
	{
		s->WriteSwap( fInt32_1 );
	}
	if ( IIsFieldFlagSet( kInt32_2 ) )
	{
		s->WriteSwap( fInt32_2 );
	}
	if ( IIsFieldFlagSet( kInt32_3 ) )
	{
		s->WriteSwap( fInt32_3 );
	}
	if ( IIsFieldFlagSet( kInt32_4 ) )
	{
		s->WriteSwap( fInt32_4 );
	}
	if ( IIsFieldFlagSet( kUInt32_1 ) )
	{
		s->WriteSwap( fUInt32_1 );
	}
	if ( IIsFieldFlagSet( kUInt32_2 ) )
	{
		s->WriteSwap( fUInt32_2 );
	}
	if ( IIsFieldFlagSet( kUInt32_3 ) )
	{
		s->WriteSwap( fUInt32_3 );
	}
	if ( IIsFieldFlagSet( kUInt32_4 ) )
	{
		s->WriteSwap( fUInt32_4 );
	}
	if ( IIsFieldFlagSet( kString64_1 ) )
	{
		plMsgStdStringHelper::Poke( fString64_1, s );
	}
	if ( IIsFieldFlagSet( kString64_2 ) )
	{
		plMsgStdStringHelper::Poke( fString64_2, s );
	}
	if ( IIsFieldFlagSet( kString64_3 ) )
	{
		plMsgStdStringHelper::Poke( fString64_3, s );
	}
	if ( IIsFieldFlagSet( kString64_4 ) )
	{
		plMsgStdStringHelper::Poke( fString64_4, s );
	}
	if ( IIsFieldFlagSet( kString64_5 ) )
	{
		plMsgStdStringHelper::Poke( fString64_5, s );
	}
	if ( IIsFieldFlagSet( kString64_6 ) )
	{
		plMsgStdStringHelper::Poke( fString64_6, s );
	}
	if ( IIsFieldFlagSet( kIString64_1 ) )
	{
		plMsgStdStringHelper::Poke( fIString64_1, s );
	}
	if ( IIsFieldFlagSet( kIString64_2 ) )
	{
		plMsgStdStringHelper::Poke( fIString64_2, s );
	}
	if ( IIsFieldFlagSet( kText_1 ) )
	{
		plMsgStdStringHelper::Poke( fText_1, s );
	}
	if ( IIsFieldFlagSet( kText_2 ) )
	{
		plMsgStdStringHelper::Poke( fText_2, s );
	}
	if ( IIsFieldFlagSet( kBlob_1 ) )
	{
		UInt32 buflen = IGetBufferSizeBlob_1();
		s->WriteSwap( buflen );
		if ( buflen )
			s->Write( buflen, IGetBufferBlob_1() );
	}
	if ( IIsFieldFlagSet( kBlob_2 ) )
	{
		UInt32 buflen = IGetBufferSizeBlob_2();
		s->WriteSwap( buflen );
		if ( buflen )
			s->Write( buflen, IGetBufferBlob_2() );
	}
	if ( IIsFieldFlagSet( kBlob_1_Guid ) )
	{
		fBlob_1_Guid.Write( s );
	}
	if ( IIsFieldFlagSet( kBlob_2_Guid ) )
	{
		fBlob_2_Guid.Write( s );
	}
}

void plVaultNode::CopyFrom( const plVaultNode * other, bool forceCopyAllFields )
{
	hsRAMStream ram;
	plVaultNode * nonConstOther = const_cast<plVaultNode*>( other );

	hsBitVector saveFlags;
	if ( forceCopyAllFields )
	{
		saveFlags = nonConstOther->fFieldFlags;
		nonConstOther->ISetAllFieldFlags();
	}

	nonConstOther->Write( &ram, nil );
	ram.Rewind();
	this->Read( &ram, nil );

	if ( forceCopyAllFields )
	{
		nonConstOther->fFieldFlags = saveFlags;
	}
}

std::string	plVaultNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%sNode[ID:%lu,Owner:%lu,Creator:%lu,Mod:%s]",
		space.c_str(), GetID(), GetOwnerNodeID(), GetCreatorNodeID(),
		fModifyTime.Format("%m/%d/%y %H:%M:%S GMT").c_str() );
	return result;
}

std::string plVaultNode::FieldsAsStdString( void ) const
{
	std::stringstream stm;

	stm << "ID=" << GetID();

	if ( IIsFieldFlagSet( plVaultNode::kPermissions ) )
	{
		stm << ", Permissions=" << GetPermissions();
	}
	if ( IIsFieldFlagSet( plVaultNode::kOwnerNodeID ) )
	{
		stm << ", OwnerIdx=" <<GetOwnerNodeID();
	}
	if ( IIsFieldFlagSet( plVaultNode::kGroupNodeID ) )
	{
		stm << ", GroupIdx=" << GetGroupNodeID();
	}
	if ( IIsFieldFlagSet( plVaultNode::kModifyTime ) )
	{
		stm << ", ModifyTime='" << GetModifyTime()->Format("%m/%d/%y %H:%M:%S").c_str() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kCreateAgeName ) )
	{
		stm << ", CreateAgeName='" << GetCreateAgeName() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kCreateAgeGuid ) )
	{
		stm << ", CreateAgeGuid='" << GetCreateAgeGuid()->AsStdString().c_str() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kInt32_1 ) )
	{
		stm << ", Int32_1=" << IGetInt32_1();
	}
	if ( IIsFieldFlagSet( plVaultNode::kInt32_2 ) )
	{
		stm << ", Int32_2=" << IGetInt32_2();
	}
	if ( IIsFieldFlagSet( plVaultNode::kInt32_3 ) )
	{
		stm << ", Int32_3=" << IGetInt32_3();
	}
	if ( IIsFieldFlagSet( plVaultNode::kUInt32_1 ) )
	{
		stm << ", UInt32_1=" << IGetUInt32_1();
	}
	if ( IIsFieldFlagSet( plVaultNode::kUInt32_2 ) )
	{
		stm << ", UInt32_2=" << IGetUInt32_2();
	}
	if ( IIsFieldFlagSet( plVaultNode::kUInt32_3 ) )
	{
		stm << ", UInt32_3=" << IGetUInt32_3();
	}
	if ( IIsFieldFlagSet( plVaultNode::kString64_1 ) )
	{
		stm << ", String64_1='" << IGetString64_1() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kString64_2 ) )
	{
		stm << ", String64_2='" << IGetString64_2() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kString64_3 ) )
	{
		stm << ", String64_3='" << IGetString64_3() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kString64_4 ) )
	{
		stm << ", String64_4='" << IGetString64_4() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kString64_5 ) )
	{
		stm << ", String64_5='" << IGetString64_5() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kString64_6 ) )
	{
		stm << ", String64_6='" << IGetString64_6() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kIString64_1 ) )
	{
		stm << ", IString64_1='" << IGetIString64_1() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kIString64_2 ) )
	{
		stm << ", IString64_2='" << IGetIString64_2() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kText_1 ) )
	{
		stm << ", Text_1='" << IGetText_1() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kText_2 ) )
	{
		stm << ", Text_2='" << IGetText_2() << "'";
	}
	if ( IIsFieldFlagSet( plVaultNode::kBlob_1 ) )
	{
		stm << ", Blob_1";
	}
	if ( IIsFieldFlagSet( plVaultNode::kBlob_2 ) )
	{
		stm << ", Blob_2";
	}

	return stm.str();
}


void plVaultNode::Dump( int level, bool displaySeen, bool beenSeen, plStatusLog * log ) const
{
	if (!fMyNodeMgr)
		return;
	if ( !log )
		log = fMyNodeMgr->GetStatusLog();
	std::string str;
	if ( displaySeen )
		xtl::format( str, "%s, seen:%s", AsStdString( level ).c_str(), beenSeen?"yes":"no" );
	else
		xtl::format( str, "%s", AsStdString( level ).c_str() );
	hsLogEntry(log->AddLine( str.c_str() ));
	for ( plVault::NodeRefVec::const_iterator it=fChildNodes.begin(); it!=fChildNodes.end(); ++it )
	{
		plVaultNodeRef * nodeRef = *it;
		plVaultNode * node = nodeRef->GetChild();
		if ( node )
			node->Dump( level+2, true, nodeRef->BeenSeen() );
	}
}

void plVaultNode::SetInitialized( bool b )
{
	fInitialized = b;
	if (!fMyNodeMgr)
		return;
	if ( fInitialized )
		fMyNodeMgr->INotifyNodeInitialized( this );
}

bool plVaultNode::StartInitializing( plVaultOperationCallback * cb, UInt32 cbContext )
{
	plVNodeInitTask * initTask = fMyNodeMgr ? fMyNodeMgr->IGetNodeInitializationTask( this ) : nil;
	if ( initTask )
	{
		initTask->SetCbObject( cb, cbContext );
		initTask->Start();
		return true;	// initTask started
	}
	else
	{
		SetInitialized( true );
		plVaultOperationCallbackHolder cbHolder( fMyNodeMgr, cb, cbContext );
		cbHolder.OperationComplete( hsOK );
		return false;	// no initTask started
	}
}



////////////////////////////////////////////////////////////////////

plVaultFolderNode::plVaultFolderNode()
{
	ISetType( plVault::kNodeType_Folder );
}

plVaultFolderNode::~plVaultFolderNode()
{
}

std::string	plVaultFolderNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Type:%s,Name:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ), plVault::StandardNodeStr( GetFolderType() ),
		GetFolderName(), plVaultNode::AsStdString().c_str() );
	return result;
}

std::string plVaultFolderNode::AsHtmlForm( const char * action )
{
	std::stringstream ss;
	ss << "<form name='VaultNode' method='post' action='" << action << "'>" << std::endl;
	ss << "</form>" << std::endl;
	return ss.str();
}


////////////////////////////////////////////////////////////////////

plVaultImageNode::plVaultImageNode()
: fMipmap( nil )
{
	ISetType( plVault::kNodeType_Image );
}

plVaultImageNode::~plVaultImageNode()
{
}

void plVaultImageNode::CopyFrom( const plVaultNode * other, bool forceCopyAllFields )
{
	const plVaultImageNode * IMG = plVaultImageNode::ConvertNoRef( other );
	if ( IMG )
		fMipmap = IMG->fMipmap;
	plVaultNode::CopyFrom( other, forceCopyAllFields );
}

std::string	plVaultImageNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Size:%d] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ), GetBufSize(), plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plVaultTextNoteNode::plVaultTextNoteNode()
{
	ISetType( plVault::kNodeType_TextNote );
}

plVaultTextNoteNode::~plVaultTextNoteNode()
{
}

void plVaultTextNoteNode::SetText( const char * text )
{
	std::string work=text; memcpy( IAllocBufferBlob_1( work.size() ), work.c_str(), work.size() );
}

std::string	plVaultTextNoteNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Title:%s,Text:%s,Type:%d,Sub:%d] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ), GetTitle(),
		GetText(), GetNoteType(), GetNoteSubType(), plVaultNode::AsStdString().c_str() );
	return result;
}

plVaultFolderNode * plVaultTextNoteNode::GetDeviceInbox() const
{
	hsAssert( GetNoteType()==plVault::kNoteType_Device, "This text note is not a device." );
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kDeviceInboxFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		return plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	return nil;
}

void plVaultTextNoteNode::SetDeviceInbox( const char * inboxName,
	plVaultOperationCallback * cb/*=nil*/, UInt32 cbContext/*=0*/ )
{
	// Remove old inbox.
	plVaultFolderNode * inbox = GetDeviceInbox();
	if ( inbox )
		RemoveNode( inbox->GetID() );
	// Link to new inbox.
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kDeviceInboxFolder );
	tmp.SetFolderName( inboxName );
	// if the inbox name matches our device title, then
	// connect to our own personal version of the inbox,
	// else connect to the shared copy of the inbox.
	if ( stricmp( inboxName, GetTitle() )==0 )
		tmp.SetOwnerNodeID( GetID() );
	LinkToNode( &tmp, plVault::kFetchAllChildren, true, cb, cbContext );
}


////////////////////////////////////////////////////////////////////

plVaultSDLNode::plVaultSDLNode()
: fSDLDataRec( nil )
{
	ISetType( plVault::kNodeType_SDL );
}

plVaultSDLNode::~plVaultSDLNode()
{
	delete fSDLDataRec;
}

void plVaultSDLNode::Save(
	plVaultOperationCallback * cb/*=nil*/,
	UInt32 cbContext/*=0 */)
{
#ifdef HS_DEBUGGING
	hsLogEntry( DumpStateDataRecord("Saving plVaultSDLNode", false ) );
#endif
	plVaultNode::Save( cb, cbContext );
}

plStateDataRecord * plVaultSDLNode::GetStateDataRecord( UInt32 readOptions/*=0 */) const
{
	delete fSDLDataRec;
	fSDLDataRec = nil;

	if ( GetBufSize()==0 || GetBuffer()==nil )
		return nil;

	hsRAMStream ram;
	ram.Write( GetBufSize(), GetBuffer() );
	ram.Rewind();
	char * sdlRecName=nil;
	int sdlRecVersion;
	plStateDataRecord::ReadStreamHeader( &ram, &sdlRecName, &sdlRecVersion );

	hsLogEntry(plSafeGetLog()->AddLineF( "SDLNode: SDL: name %s, version %d.", sdlRecName, sdlRecVersion ));

	plStateDescriptor* des = plSDLMgr::GetInstance()->FindDescriptor(sdlRecName, sdlRecVersion );
	if (!des)
	{
		hsLogEntry(plSafeGetLog()->AddLineF( "SDLNode: Can't find SDL desc, name:%s ver:%d. Attempting to read with latest descriptor.", sdlRecName, sdlRecVersion ));
		des = plSDLMgr::GetInstance()->FindDescriptor(sdlRecName, plSDL::kLatestVersion );
		if (!des)
		{
			hsLogEntry(plSafeGetLog()->AddLineF( "SDLNode: Can't find SDL desc, name:%s ver:kLatestVersion. Cannot read sdl record.", sdlRecName ));
			return nil;
		}
	}

	fSDLDataRec = TRACKED_NEW plStateDataRecord( des );
	if ( !fSDLDataRec->Read( &ram, 0, readOptions ) )
	{
		hsLogEntry(plSafeGetLog()->AddLineF( "SDLNode: Failed to read SDL desc, name:%s ver:%d", sdlRecName, sdlRecVersion ));
		delete fSDLDataRec;
		fSDLDataRec = nil;
	}
	else
	{
		hsLogEntry(plSafeGetLog()->AddLineF( "SDLNode: After reading SDL: name %s, version %d.",
			fSDLDataRec->GetDescriptor()->GetName(), fSDLDataRec->GetDescriptor()->GetVersion() ));
	}

	delete [] sdlRecName;
	return fSDLDataRec;
}

void plVaultSDLNode::InitStateDataRecord( const char * sdlRecName, UInt32 writeOptions/*=0 */)
{
	if ( !GetStateDataRecord() )
	{
		delete fSDLDataRec;
		fSDLDataRec = nil;

		plStateDescriptor* des = plSDLMgr::GetInstance()->FindDescriptor(sdlRecName, plSDL::kLatestVersion);
		if ( !des )
		{
			hsLogEntry( plSafeGetLog()->AddLineF( "SDLNode: Can't find SDL desc, name:%s", sdlRecName ) );
			return;
		}

		fSDLDataRec = TRACKED_NEW plStateDataRecord( des );
		fSDLDataRec->SetFromDefaults( false );
		SetStateDataRecord( fSDLDataRec, writeOptions|plSDL::kDontWriteDirtyFlag );
		Save();
	}
}


void plVaultSDLNode::SetStateDataRecord( const plStateDataRecord * rec, UInt32 writeOptions/*=0 */)
{
	hsRAMStream ram;
	rec->WriteStreamHeader( &ram );
	rec->Write( &ram, 0, writeOptions );
	ram.Rewind();
	ram.CopyToMem( AllocBuffer( ram.GetEOF() ) );
}

void plVaultSDLNode::DumpStateDataRecord( const char * msg/*=nil */, bool dirtyOnly/*=false*/ ) const
{
	if ( !fSDLDataRec )
		GetStateDataRecord();

	if ( !fSDLDataRec )
		return;

	if ( !msg )
		msg = "VaultSDLNode";

	fSDLDataRec->DumpToObjectDebugger( msg, dirtyOnly );
}

std::string	plVaultSDLNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Size:%d,Ident:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ),
		GetBufSize(), plVault::StandardNodeStr( GetIdent() ),
		plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plVaultAgeInfoNode::plVaultAgeInfoNode()
: fCanVisitFolder( nil )
, fAgeOwnersFolder( nil )
, fChildAgesFolder( nil )
, fAgeSDL( nil )
{
	ISetType( plVault::kNodeType_AgeInfo );
}

plVaultAgeInfoNode::~plVaultAgeInfoNode()
{
}

plVaultAgeLinkNode * plVaultAgeInfoNode::IGetLink( plVaultFolderNode * folder, const plAgeInfoStruct * info ) const
{
	plVaultNodeIterator it = folder->GetIterator();

	for ( it; it; ++it )
	{
		plVaultAgeLinkNode * linkNode = plVaultAgeLinkNode::ConvertNoRef( it->GetChild() );
		if ( !linkNode )
			continue;
		plVaultAgeInfoNode * infoNode = linkNode->GetAgeInfo();
		if ( !infoNode )
			continue;
		if ( info->HasAgeInstanceGuid() && infoNode->GetAgeInstanceGuid()->IsEqualTo( info->GetAgeInstanceGuid() ) )
			return linkNode;
		if ( info->HasAgeInstanceName() && stricmp( infoNode->GetAgeInstanceName(), info->GetAgeInstanceName() )==0 )
			return linkNode;
		if ( info->HasAgeFilename() && stricmp( infoNode->GetAgeFilename(), info->GetAgeFilename() )==0 )
			return linkNode;
	}

	return nil;
}

const plNetServerSessionInfo * plVaultAgeInfoNode::AsServerInfo() const
{
	fServerInfo.Clear();
	fServerInfo.SetServerName( GetAgeInstanceName() );
	fServerInfo.SetServerGuid( GetAgeInstanceGuid() );
	return &fServerInfo;
}

const plAgeInfoStruct * plVaultAgeInfoNode::AsAgeInfoStruct() const
{
	fAgeInfoStruct.SetAgeFilename( GetAgeFilename() );
	fAgeInfoStruct.SetAgeInstanceName( GetAgeInstanceName() );
	fAgeInfoStruct.SetAgeInstanceGuid( GetAgeInstanceGuid() );
	fAgeInfoStruct.SetAgeUserDefinedName( GetAgeUserDefinedName() );
	fAgeInfoStruct.SetAgeSequenceNumber( GetSequenceNumber() );
	fAgeInfoStruct.SetAgeDescription( GetAgeDescription() );
	return &fAgeInfoStruct;
}


void plVaultAgeInfoNode::FromAgeInfoStruct( const plAgeInfoStruct * ageInfo )
{
	if ( ageInfo->HasAgeFilename() )
		SetAgeFilename( ageInfo->GetAgeFilename() );
	if ( ageInfo->HasAgeInstanceName() )
		SetAgeInstanceName( ageInfo->GetAgeInstanceName() );
	if ( ageInfo->HasAgeInstanceGuid() )
		SetAgeInstanceGuid( ageInfo->GetAgeInstanceGuid() );
	if ( ageInfo->HasAgeUserDefinedName() )
		SetAgeUserDefinedName( 	ageInfo->GetAgeUserDefinedName() );
	if ( ageInfo->HasAgeSequenceNumber() )
		SetSequenceNumber( ageInfo->GetAgeSequenceNumber() );
	if ( ageInfo->HasAgeDescription() )
		SetAgeDescription( ageInfo->GetAgeDescription() );
}

plVaultPlayerInfoListNode * plVaultAgeInfoNode::GetCanVisitFolder( void ) const
{
	if ( fCanVisitFolder )
		return fCanVisitFolder;
	plVaultPlayerInfoListNode tmp;
	tmp.SetFolderType( plVault::kCanVisitFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fCanVisitFolder = plVaultPlayerInfoListNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fCanVisitFolder )
		hsLogEntry(plSafeGetLog()->AddLineF( "fCanVisitFolder node not found" ));
	return fCanVisitFolder;
}

plVaultPlayerInfoListNode * plVaultAgeInfoNode::GetAgeOwnersFolder( void ) const
{
	if ( fAgeOwnersFolder )
		return fAgeOwnersFolder;
	plVaultPlayerInfoListNode tmp;
	tmp.SetFolderType( plVault::kAgeOwnersFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgeOwnersFolder = plVaultPlayerInfoListNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAgeOwnersFolder )
		hsLogEntry(plSafeGetLog()->AddLineF( "fAgeOwnersFolder node not found" ));
	return fAgeOwnersFolder;
}

plVaultFolderNode * plVaultAgeInfoNode::GetChildAgesFolder( void ) const
{
	if ( fChildAgesFolder )
		return fChildAgesFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kChildAgesFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fChildAgesFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fChildAgesFolder )
		hsLogEntry(plSafeGetLog()->AddLineF( "fChildAgesFolder node not found" ));
	return fChildAgesFolder;
}

plVaultAgeLinkNode * plVaultAgeInfoNode::GetChildAgeLink( const plAgeInfoStruct * info ) const
{
	GetChildAgesFolder();
	hsAssert( fChildAgesFolder, "Missing ChildAges folder" );
	if (fChildAgesFolder)
		return IGetLink( fChildAgesFolder, info );
	return nil;
}

plVaultSDLNode * plVaultAgeInfoNode::IGetAgeSDL() const
{
	if ( fAgeSDL )
		return fAgeSDL;
	plVaultSDLNode tmp;
	tmp.SetIdent( plVault::kAgeInstanceSDLNode );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgeSDL = plVaultSDLNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAgeSDL )
		hsLogEntry(plSafeGetLog()->AddLineF( "fAgeSDL node not found" ));
	return fAgeSDL;
}

plVaultPlayerInfoNode * plVaultAgeInfoNode::GetCzar() const
{
	if ( !GetAgeOwnersFolder() )
		return nil;
	GetAgeOwnersFolder()->Sort();
	plVaultNodeIterator it = GetAgeOwnersFolder()->GetIterator();
	if ( it.GetCount()==0 )
		return nil;
	return plVaultPlayerInfoNode::ConvertNoRef( it.GetAt( 0 )->GetChild() );
}


const char * plVaultAgeInfoNode::GetDisplayName() const
{
	return AsAgeInfoStruct()->GetDisplayName();
}

std::string	plVaultAgeInfoNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Age:%lu,FName:%s,IName:%s,Guid:%s,UName:%s,Seq:%d,Pub:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ),
		GetAgeID(), GetAgeFilename(), GetAgeInstanceName(),
		GetAgeInstanceGuid()->AsStdString().c_str(), GetAgeUserDefinedName(),
		GetSequenceNumber(), IsPublic()?"yes":"no", plVaultNode::AsStdString().c_str() );
	return result;
}



////////////////////////////////////////////////////////////////////

plVaultAgeLinkNode::plVaultAgeLinkNode()
: fAgeInfo( nil )
{
	ISetType( plVault::kNodeType_AgeLink );
}

plVaultAgeLinkNode::~plVaultAgeLinkNode()
{
}

plVaultAgeInfoNode * plVaultAgeLinkNode::GetAgeInfo() const
{
	if ( fAgeInfo )
		return fAgeInfo;
	plVaultAgeInfoNode tmp;
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgeInfo = plVaultAgeInfoNode::ConvertNoRef( nodeRef->GetChild() );
	return fAgeInfo;
}

void plVaultAgeLinkNode::SetAgeInfo( plVaultAgeInfoNode * node )
{
	plVaultAgeInfoNode * curr = GetAgeInfo();
	if ( curr )
	{
		if ( curr->GetID()==node->GetID() )
			return;

		RemoveNode( curr->GetID() );
	}
	fAgeInfo = nil;
	AddNode( node );
}

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

void plVaultAgeLinkNode::AddSpawnPoint( const plSpawnPointInfo & point )
{
	plSpawnPointVec points;
	GetSpawnPoints( points );
	if ( std::find_if( points.begin(), points.end(), MatchesSpawnPointTitle( point.fTitle.c_str() ) )!=points.end() )
		return;

	// only check to see if the titles are the same... 
	//... so we can add the same spawnpoint as long as they have different titles
		//if ( std::find_if( points.begin(), points.end(), MatchesSpawnPointName( point.fSpawnPt.c_str() ) )!=points.end() )
		//	return;

	points.push_back( point );
	SetSpawnPoints( points );
}

void plVaultAgeLinkNode::RemoveSpawnPoint( const char * spawnPtName )
{
	plSpawnPointVec points;
	GetSpawnPoints( points );												    
	plSpawnPointVec::iterator it = std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) );
	if ( it!=points.end() )
	{
		points.erase( it );
		SetSpawnPoints( points );
	}
}

bool plVaultAgeLinkNode::HasSpawnPoint( const plSpawnPointInfo & point ) const
{
	return HasSpawnPoint( point.GetName() );
}

bool plVaultAgeLinkNode::HasSpawnPoint( const char * spawnPtName ) const
{
	plSpawnPointVec points;
	GetSpawnPoints( points );												    
	return ( std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) )!=points.end() );
}

void plVaultAgeLinkNode::GetSpawnPoints( plSpawnPointVec & out ) const
{
	char token1[ 1024 ];
	hsStringTokenizer izer1( (const char *)IGetBufferBlob_1(), ";" );
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
		out.push_back( point );
	}
}

void plVaultAgeLinkNode::SetSpawnPoints( const plSpawnPointVec & in )
{
	std::stringstream ss;
	for ( int i=0; i<in.size(); i++ )
		ss
		<< in[i].fTitle << ":"
		<< in[i].fSpawnPt << ":"
		<< in[i].fCameraStack << ";"
		;
	char * buf = (char *)IAllocBufferBlob_1( ss.str().size() );
	strcpy( buf, ss.str().c_str() );
}

std::string	plVaultAgeLinkNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	const char * spawnPts = (const char *)IGetBufferBlob_1();
	xtl::format( result, "%s%s[Locked:%d Volatile:%d,SpnPts:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ),
		GetLocked(), GetVolatile(), spawnPts?spawnPts:"(nil)",
		plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plVaultChronicleNode::plVaultChronicleNode()
{
	ISetType( plVault::kNodeType_Chronicle );
}

plVaultChronicleNode::~plVaultChronicleNode()
{
}

std::string	plVaultChronicleNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Name:%s,Type:%lu,Value:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ), GetName(),
		GetEntryType(), GetValue(),
		plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plVaultPlayerInfoNode::plVaultPlayerInfoNode()
{
	ISetType( plVault::kNodeType_PlayerInfo );
}

plVaultPlayerInfoNode::~plVaultPlayerInfoNode()
{
}

void plVaultPlayerInfoNode::SetPlayerName( const char * text )
{
	if ( fMyNodeMgr )
		CheckIAmSuperUserOr_VoidReturn( fMyNodeMgr, ( !IsStored()||fMyNodeMgr->IIsThisMe( this ) ) );
	ISetIString64_1( text );
}

void plVaultPlayerInfoNode::SetPlayerID( UInt32 v )
{
	if ( fMyNodeMgr )
		CheckIAmSuperUserOr_VoidReturn( fMyNodeMgr, ( !IsStored()||fMyNodeMgr->IIsThisMe( this ) ) );
	ISetUInt32_1( v );
}

void plVaultPlayerInfoNode::SetAgeInstanceName( const char * v )
{
	ISetString64_1( v );
}

const char * plVaultPlayerInfoNode::GetAgeInstanceName( void ) const
{
	return ( IsOnline() ) ? IGetString64_1() : "";
}

void plVaultPlayerInfoNode::SetAgeGuid( const plUUID * v)
{
	ISetString64_2( v->AsStdString().c_str() );
}

const plUUID * plVaultPlayerInfoNode::GetAgeGuid( void ) const
{
	if ( IsOnline() )
		fGuid.FromString( IGetString64_2() );
	else
		fGuid.Clear();
	return &fGuid;
}

void plVaultPlayerInfoNode::SetOnline( bool b )
{
	ISetInt32_1( b );
}


std::string	plVaultPlayerInfoNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[PlayerID:%lu,PlayerName:%s,IsOnline:%s,InAge:%s,InAgeGuid:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ), GetPlayerID(),
		GetPlayerName(), (IsOnline())?"yes":"no",
		GetAgeInstanceName(), GetAgeGuid()->AsStdString().c_str(),
		plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

// true if A is less than B
bool PlayerInfoNodeCompare( const plVaultNode * a, const plVaultNode * b )
{
	const plVaultPlayerInfoNode * A = plVaultPlayerInfoNode::ConvertNoRef( a );
	const plVaultPlayerInfoNode * B = plVaultPlayerInfoNode::ConvertNoRef( b );

	if ( !A )
		return false;
	if ( !B )
		return true;
	if ( A->IsOnline() && !B->IsOnline() )
		return true;
	if ( B->IsOnline() && !A->IsOnline() )
		return false;
	return ( stricmp( A->GetPlayerName(), B->GetPlayerName() )<0 );
}

bool PlayerInfoNodeRefCompare( const plVaultNodeRef * nodeRefA, const plVaultNodeRef * nodeRefB )
{
	if ( !nodeRefA || !nodeRefB )
		return false;

	return PlayerInfoNodeCompare( nodeRefA->GetChild(), nodeRefB->GetChild() );
};

plVaultPlayerInfoListNode::plVaultPlayerInfoListNode()
{
	ISetType( plVault::kNodeType_PlayerInfoList );
}

plVaultPlayerInfoListNode::~plVaultPlayerInfoListNode()
{
}

bool plVaultPlayerInfoListNode::HasPlayer( UInt32 playerID )
{
	plVaultPlayerInfoNode templateNode;
	templateNode.SetPlayerID( playerID );
	plVaultNodeRef * tmp;
	return FindNode( &templateNode, tmp );
}

bool plVaultPlayerInfoListNode::AddPlayer( UInt32 playerID )
{
	plVaultPlayerInfoNode templateNode;
	templateNode.SetPlayerID( playerID );
	plVaultNodeRef * tmp;
	if ( !FindNode( &templateNode, tmp ) )
		return LinkToNode( &templateNode );
	return true;
}

void plVaultPlayerInfoListNode::RemovePlayer( UInt32 playerID )
{
	plVaultPlayerInfoNode templateNode;
	templateNode.SetPlayerID( playerID );
	plVaultNodeRef * tmp;
	if ( FindNode( &templateNode, tmp ) )
		RemoveNode( tmp->GetChildID() );
}

plVaultPlayerInfoNode * plVaultPlayerInfoListNode::GetPlayer( UInt32 playerID )
{
	plVaultPlayerInfoNode templateNode;
	templateNode.SetPlayerID( playerID );
	plVaultNodeRef * tmp;
	if ( FindNode( &templateNode, tmp ) )
		return plVaultPlayerInfoNode::ConvertNoRef( tmp->GetChild() );
	return nil;
}


void plVaultPlayerInfoListNode::SetPlayers( const plVault::IDVec & playerIDs )
{
	RemoveAllNodes();
	for ( int i=0; i<playerIDs.size(); i++ )
		AddPlayer( playerIDs[i] );
}

void plVaultPlayerInfoListNode::ISortPlayers( plNodeRefCompareProc compare )
{
	std::sort( fChildNodes.begin(), fChildNodes.end(), compare );
}

void plVaultPlayerInfoListNode::Sort()
{
	ISortPlayers( PlayerInfoNodeRefCompare );
}

void plVaultPlayerInfoListNode::ILocalNodeAdded( plVaultNodeRef * nodeRef )
{
	Sort();
}


////////////////////////////////////////////////////////////////////

plVaultAgeInfoListNode::plVaultAgeInfoListNode()
{
	ISetType( plVault::kNodeType_AgeInfoList );
}

plVaultAgeInfoListNode::~plVaultAgeInfoListNode()
{
}

bool plVaultAgeInfoListNode::HasAge( UInt32 ageID )
{
	plVaultAgeInfoNode templateNode;
	templateNode.SetAgeID( ageID );
	plVaultNodeRef * tmp;
	return FindNode( &templateNode, tmp );
}

bool plVaultAgeInfoListNode::AddAge( UInt32 ageID )
{
	plVaultAgeInfoNode templateNode;
	templateNode.SetAgeID( ageID );
	plVaultNodeRef * tmp;
	if ( !FindNode( &templateNode, tmp ) )
		return LinkToNode( &templateNode );
	return true;
}

void plVaultAgeInfoListNode::RemoveAge( UInt32 ageID )
{
	plVaultAgeInfoNode templateNode;
	templateNode.SetAgeID( ageID );
	plVaultNodeRef * tmp;
	if ( FindNode( &templateNode, tmp ) )
		RemoveNode( tmp->GetChildID() );
}

////////////////////////////////////////////////////////////////////
plVaultMgrNode::plVaultMgrNode()
: fMyInboxFolder( nil ), fSystemNode(nil)
{
}

plVaultMgrNode::~plVaultMgrNode()
{
}

plVaultFolderNode * plVaultMgrNode::GetInbox( void ) const
{
	if ( fMyInboxFolder )
		return fMyInboxFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kInboxFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fMyInboxFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	return fMyInboxFolder;
}

plVaultSystemNode * plVaultMgrNode::GetSystemNode( ) const
{
	if ( fSystemNode )
		return fSystemNode;
	plVaultSystemNode tmp;
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fSystemNode = plVaultSystemNode::ConvertNoRef( nodeRef->GetChild() );
	return fSystemNode;
}

////////////////////////////////////////////////////////////////////

plVaultPlayerNode::plVaultPlayerNode()
: fPlayerInfo( nil )
, fAvatarOutfitFolder( nil )
, fAvatarClosetFolder( nil )
, fChronicleFolder( nil )
, fIgnoreListFolder( nil )
, fBuddyListFolder( nil )
, fPeopleIKnowAboutFolder( nil )
, fAgeJournalsFolder( nil )
, fAgesICanVisitFolder( nil )
, fAgesIOwnFolder( nil )
, fInviteFolder( nil )
{
	ISetType( plVault::kNodeType_VNodeMgrPlayer );
}

plVaultPlayerNode::~plVaultPlayerNode()
{
}

plVaultPlayerInfoNode * plVaultPlayerNode::GetPlayerInfo() const
{
	static plVaultPlayerInfoNode s_nilNode;
	if ( fPlayerInfo && fPlayerInfo != &s_nilNode)
		return fPlayerInfo;
	fPlayerInfo = nil;
	plVaultPlayerInfoNode tmp;
	tmp.SetPlayerID( GetID() );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fPlayerInfo = plVaultPlayerInfoNode::ConvertNoRef( nodeRef->GetChild() );
	if (!fPlayerInfo) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fPlayerInfo node not found" ));
		fPlayerInfo = &s_nilNode;
	}
	return fPlayerInfo;
}

plVaultFolderNode * plVaultPlayerNode::GetAvatarOutfitFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fAvatarOutfitFolder && fAvatarOutfitFolder != &s_nilNode)
		return fAvatarOutfitFolder;
	fAvatarOutfitFolder = nil;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kAvatarOutfitFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAvatarOutfitFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAvatarOutfitFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fAvatarOutfitFolder node not found" ));
		fAvatarOutfitFolder = &s_nilNode;
	}
	return fAvatarOutfitFolder;
}

plVaultFolderNode * plVaultPlayerNode::GetAvatarClosetFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fAvatarClosetFolder && fAvatarClosetFolder != &s_nilNode)
		return fAvatarClosetFolder;
	fAvatarClosetFolder = nil;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kAvatarClosetFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAvatarClosetFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAvatarClosetFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fAvatarClosetFolder node not found" ));
		fAvatarClosetFolder = &s_nilNode;
	}
	return fAvatarClosetFolder;
}

plVaultFolderNode * plVaultPlayerNode::GetChronicleFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fChronicleFolder && fChronicleFolder != &s_nilNode)
		return fChronicleFolder;
	fChronicleFolder = nil;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kChronicleFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fChronicleFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fChronicleFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fChronicleFolder node not found" ));
		fChronicleFolder = &s_nilNode;
	}	
	return fChronicleFolder;
}

plVaultFolderNode * plVaultPlayerNode::GetAgeJournalsFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fAgeJournalsFolder && fAgeJournalsFolder != &s_nilNode)
		return fAgeJournalsFolder;
	fAgeJournalsFolder = nil;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kAgeJournalsFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgeJournalsFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAgeJournalsFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fAgeJournalsFolder node not found" ));
		fAgeJournalsFolder = &s_nilNode;
	}	
	return fAgeJournalsFolder;
}

plVaultFolderNode * plVaultPlayerNode::GetAgesICanVisitFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fAgesICanVisitFolder && fAgesICanVisitFolder != &s_nilNode)
		return fAgesICanVisitFolder;
	fAgesICanVisitFolder = nil;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kAgesICanVisitFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgesICanVisitFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAgesICanVisitFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fAgesICanVisitFolder node not found" ));
		fAgesICanVisitFolder = &s_nilNode;
	}
	return fAgesICanVisitFolder;
}

plVaultFolderNode * plVaultPlayerNode::GetAgesIOwnFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fAgesIOwnFolder && fAgesIOwnFolder != &s_nilNode)
		return fAgesIOwnFolder;
	fAgesIOwnFolder = nil;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kAgesIOwnFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgesIOwnFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAgesIOwnFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fAgesIOwnFolder node not found" ));
		fAgesIOwnFolder = &s_nilNode;
	}
	return fAgesIOwnFolder;
}

plVaultPlayerInfoListNode * plVaultPlayerNode::GetIgnoreListFolder( void ) const
{
	static plVaultPlayerInfoListNode s_nilNode;
	if ( fIgnoreListFolder && fIgnoreListFolder != &s_nilNode)
		return fIgnoreListFolder;
	fIgnoreListFolder = nil;
	plVaultPlayerInfoListNode tmp;
	tmp.SetFolderType( plVault::kIgnoreListFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fIgnoreListFolder = plVaultPlayerInfoListNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fIgnoreListFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fIgnoreListFolder node not found" ));
		fIgnoreListFolder = &s_nilNode;
	}
	return fIgnoreListFolder;
}

plVaultPlayerInfoListNode * plVaultPlayerNode::GetBuddyListFolder( void ) const
{
	static plVaultPlayerInfoListNode s_nilNode;
	if ( fBuddyListFolder && fBuddyListFolder != &s_nilNode)
		return fBuddyListFolder;
	fBuddyListFolder = nil;
	plVaultPlayerInfoListNode tmp;
	tmp.SetFolderType( plVault::kBuddyListFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fBuddyListFolder = plVaultPlayerInfoListNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fBuddyListFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fBuddyListFolder node not found" ));
		fBuddyListFolder = &s_nilNode;
	}
	return fBuddyListFolder;
}

plVaultPlayerInfoListNode * plVaultPlayerNode::GetPeopleIKnowAboutFolder( void ) const
{
	static plVaultPlayerInfoListNode s_nilNode;
	if ( fPeopleIKnowAboutFolder && fPeopleIKnowAboutFolder != &s_nilNode)
		return fPeopleIKnowAboutFolder;
	fPeopleIKnowAboutFolder = nil;
	plVaultPlayerInfoListNode tmp;
	tmp.SetFolderType( plVault::kPeopleIKnowAboutFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fPeopleIKnowAboutFolder = plVaultPlayerInfoListNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fPeopleIKnowAboutFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fPeopleIKnowAboutFolder node not found" ));
		fPeopleIKnowAboutFolder = &s_nilNode;
	}
	return fPeopleIKnowAboutFolder;
}

plVaultFolderNode * plVaultPlayerNode::GetInviteFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fInviteFolder && fInviteFolder != &s_nilNode)
		return fInviteFolder;
	fInviteFolder = nil;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kPlayerInviteFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fInviteFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fInviteFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fInviteFolder node not found" ));
		fInviteFolder = &s_nilNode;
	}
	return fInviteFolder;
}

//////

plVaultAgeLinkNode * plVaultPlayerNode::GetLinkToMyNeighborhood() const
{
	plAgeInfoStruct info;
	info.SetAgeFilename( kNeighborhoodAgeFilename );
	return GetOwnedAgeLink( &info );
}


plVaultAgeLinkNode * plVaultPlayerNode::GetLinkToCity() const
{
	plAgeInfoStruct info;
	info.SetAgeFilename( kCityAgeFilename );
	return GetOwnedAgeLink( &info );
}

//////

plVaultAgeLinkNode * plVaultPlayerNode::IFindLink( plVaultFolderNode * folder, const plAgeInfoStruct * info ) const
{
	plVaultNodeIterator it = folder->GetIterator();

	for ( it; it; ++it )
	{
		plVaultAgeLinkNode * linkNode = plVaultAgeLinkNode::ConvertNoRef( it->GetChild() );
		if ( !linkNode )
			continue;
		plVaultAgeInfoNode * infoNode = linkNode->GetAgeInfo();
		if ( !infoNode )
			continue;
		if ( info->IsEqualTo( infoNode->AsAgeInfoStruct() ) )
			return linkNode;
	}

	return nil;
}

void plVaultPlayerNode::IRemoveLink( plVaultFolderNode * folder, const plUUID * guid )
{
	plVaultNodeIterator it = folder->GetIterator();

	for ( it; it; ++it )
	{
		plVaultAgeLinkNode * linkNode = plVaultAgeLinkNode::ConvertNoRef( it->GetChild() );
		if ( !linkNode )
			continue;
		plVaultAgeInfoNode * infoNode = linkNode->GetAgeInfo();
		if ( !infoNode )
			continue;
		if ( infoNode->GetAgeInstanceGuid()->IsEqualTo( guid ) )
		{
			folder->RemoveNode( linkNode->GetID() );
		}
	}
}

///////////

plVaultAgeLinkNode * plVaultPlayerNode::GetOwnedAgeLink( const plAgeInfoStruct * info, bool skipVolatile/*=false */) const
{
	GetAgesIOwnFolder();
	hsAssert( fAgesIOwnFolder, "Missing AgesIOwn folder" );
	plAgeInfoStruct tmp;
	if ( info->HasAgeFilename() )
		tmp.SetAgeFilename( info->GetAgeFilename() );
	else if ( info->HasAgeInstanceGuid() )
		tmp.SetAgeInstanceGuid( info->GetAgeInstanceGuid() );
	else
		tmp.CopyFrom( info );
	plVaultAgeLinkNode * result = IFindLink( fAgesIOwnFolder, &tmp );

	if ( result && result->GetVolatile() && skipVolatile )
		return nil;

	return result;
}


void plVaultPlayerNode::RemoveOwnedAgeLink( const plUUID * guid )
{
	GetAgesIOwnFolder();
	hsAssert( fAgesIOwnFolder, "Missing AgesIOwn folder" );
	IRemoveLink( fAgesIOwnFolder, guid );
}

///////////

plVaultAgeLinkNode * plVaultPlayerNode::GetVisitAgeLink( const plAgeInfoStruct * info ) const
{
	GetAgesICanVisitFolder();
	hsAssert( fAgesICanVisitFolder, "Missing AgesICanVisit folder" );
	plAgeInfoStruct tmp;
	if ( info->HasAgeInstanceGuid() )
		tmp.SetAgeInstanceGuid( info->GetAgeInstanceGuid() );
	else
		tmp.CopyFrom( info );
	return IFindLink( fAgesICanVisitFolder, &tmp );
}

void plVaultPlayerNode::RemoveVisitAgeLink( const plUUID * guid )
{
	GetAgesICanVisitFolder();
	hsAssert( fAgesICanVisitFolder, "Missing AgesICanVisit folder" );
	IRemoveLink( fAgesICanVisitFolder, guid );
}

///////////

plVaultChronicleNode * plVaultPlayerNode::FindChronicleEntry( const char * entryName )
{
	GetChronicleFolder();
	hsAssert( fChronicleFolder, "Missing Chronicle folder" );
	plVaultChronicleNode templateNode;
	templateNode.SetName( entryName );
	plVaultNodeRef * tmp;
	if ( fChronicleFolder->FindNode( &templateNode, tmp ) )
		return plVaultChronicleNode::ConvertNoRef( tmp->GetChild() );
	return nil;
}


///////////

void plVaultPlayerNode::SetAccountUUID( const plUUID * v )
{
	if ( fMyNodeMgr )
		CheckIAmSuperUserOr_VoidReturn( fMyNodeMgr, ( !IsStored()||fMyNodeMgr->IIsThisMe( this ) ) );
	fAcctUUID.CopyFrom( v );
	ISetIString64_2( fAcctUUID.AsStdString().c_str() );
}

const plUUID * plVaultPlayerNode::GetAccountUUID( void ) const
{
	if ( fAcctUUID.IsNull() )
		fAcctUUID.FromString( IGetIString64_2() );
	return &fAcctUUID;
}

void plVaultPlayerNode::SetPlayerName( const char * v )
{
	if ( fMyNodeMgr )
		CheckIAmSuperUserOr_VoidReturn( fMyNodeMgr, ( !IsStored()||fMyNodeMgr->IIsThisMe( this ) ) );
	ISetIString64_1( v );
	plVaultPlayerInfoNode * playerInfo = GetPlayerInfo();
	if ( playerInfo )
		playerInfo->SetPlayerName( v );
}

void plVaultPlayerNode::SetAvatarShapeName( const char * v )
{
	if ( fMyNodeMgr )
		CheckIAmSuperUserOr_VoidReturn( fMyNodeMgr, ( !IsStored()||fMyNodeMgr->IIsThisMe( this ) ) );
	ISetString64_1( v );
}

void plVaultPlayerNode::Save( plVaultOperationCallback * cb, UInt32 cbContext )
{
	if ( GetPlayerInfo() )
		GetPlayerInfo()->Save();
	plVaultNode::Save( cb, cbContext );
}


std::string	plVaultPlayerNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[ID:%lu,Nam:%s,Av:%s:Acct:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ), GetID(),
		GetPlayerName(), GetAvatarShapeName(), GetAccountUUID()->AsStdString().c_str(),
		plVaultMgrNode::AsStdString().c_str() );
	return result;
}


////////////////////////////////////////////////////////////////////

plVaultAgeNode::plVaultAgeNode()
: fAgeInfo( nil )
, fAgesIOwnFolder( nil )
, fAgeDevicesFolder( nil )
, fSubAgesFolder( nil )
, fPeopleIKnowAboutFolder( nil )
, fPublicAgesFolder( nil )
, fChronicleFolder( nil )
{
	ISetType( plVault::kNodeType_VNodeMgrAge );
}

plVaultAgeNode::~plVaultAgeNode()
{
}

plVaultAgeLinkNode * plVaultAgeNode::IGetLink( plVaultFolderNode * folder, const plAgeInfoStruct * info ) const
{
	plVaultNodeIterator it = folder->GetIterator();

	for ( it; it; ++it )
	{
		plVaultAgeLinkNode * linkNode = plVaultAgeLinkNode::ConvertNoRef( it->GetChild() );
		if ( !linkNode )
			continue;
		plVaultAgeInfoNode * infoNode = linkNode->GetAgeInfo();
		if ( !infoNode )
			continue;
		if ( info->HasAgeInstanceGuid() && infoNode->GetAgeInstanceGuid()->IsEqualTo( info->GetAgeInstanceGuid() ) )
			return linkNode;
		if ( info->HasAgeInstanceName() && stricmp( infoNode->GetAgeInstanceName(), info->GetAgeInstanceName() )==0 )
			return linkNode;
		if ( info->HasAgeFilename() && stricmp( infoNode->GetAgeFilename(), info->GetAgeFilename() )==0 )
			return linkNode;
	}

	return nil;
}


void plVaultAgeNode::SetAgeGuid( const plUUID * guid )
{
	fAgeGuid.CopyFrom( guid );
	ISetString64_1( guid->AsStdString().c_str() );
	if ( GetAgeInfo() )
		GetAgeInfo()->SetAgeInstanceGuid( guid );
}

void plVaultAgeNode::Save( plVaultOperationCallback * cb/* =nil */, UInt32 cbContext/* =0  */)
{
	if ( GetAgeInfo() )
		GetAgeInfo()->Save();
	plVaultNode::Save( cb, cbContext );
}

plVaultAgeInfoNode * plVaultAgeNode::GetAgeInfo() const
{
	if ( fAgeInfo )
		return fAgeInfo;
	plVaultAgeInfoNode tmp;
	tmp.SetAgeID( GetID() );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgeInfo = plVaultAgeInfoNode::ConvertNoRef( nodeRef->GetChild() );
	return fAgeInfo;
}

plVaultFolderNode * plVaultAgeNode::GetAgeDevicesFolder( void ) const
{
	if ( fAgeDevicesFolder )
		return fAgeDevicesFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kAgeDevicesFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgeDevicesFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAgeDevicesFolder )
		hsLogEntry(plSafeGetLog()->AddLineF( "fAgeDevicesFolder node not found" ));
	return fAgeDevicesFolder;
}

plVaultFolderNode * plVaultAgeNode::GetSubAgesFolder( void ) const
{
	if ( fSubAgesFolder )
		return fSubAgesFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kSubAgesFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fSubAgesFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fSubAgesFolder )
		hsLogEntry(plSafeGetLog()->AddLineF( "fSubAgesFolder node not found" ));
	return fSubAgesFolder;
}

plVaultPlayerInfoListNode * plVaultAgeNode::GetPeopleIKnowAboutFolder( void ) const
{
	if ( fPeopleIKnowAboutFolder )
		return fPeopleIKnowAboutFolder;
	plVaultPlayerInfoListNode tmp;
	tmp.SetFolderType( plVault::kPeopleIKnowAboutFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fPeopleIKnowAboutFolder = plVaultPlayerInfoListNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fPeopleIKnowAboutFolder )
		hsLogEntry(plSafeGetLog()->AddLineF( "fPeopleIKnowAboutFolder node not found" ));
	return fPeopleIKnowAboutFolder;
}

plVaultFolderNode * plVaultAgeNode::GetChronicleFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fChronicleFolder && fChronicleFolder != &s_nilNode)
		return fChronicleFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kChronicleFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fChronicleFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fChronicleFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fChronicleFolder node not found" ));
		fChronicleFolder = &s_nilNode;
	}	
	return fChronicleFolder;
}

plVaultFolderNode * plVaultAgeNode::GetAgesIOwnFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fAgesIOwnFolder && fAgesIOwnFolder != &s_nilNode)
		return fAgesIOwnFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kAgesIOwnFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fAgesIOwnFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fAgesIOwnFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fAgesIOwnFolder node not found" ));
		fAgesIOwnFolder = &s_nilNode;
	}	
	return fAgesIOwnFolder;
}

plVaultFolderNode * plVaultAgeNode::GetPublicAgesFolder( void ) const
{
	static plVaultFolderNode s_nilNode;
	if ( fPublicAgesFolder && fPublicAgesFolder != &s_nilNode)
		return fPublicAgesFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kPublicAgesFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fPublicAgesFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	if ( !fPublicAgesFolder ) {
		hsLogEntry(plSafeGetLog()->AddLineF( "fPublicAgesFolder node not found" ));
		fPublicAgesFolder = &s_nilNode;
	}	
	return fPublicAgesFolder;
}

// Add a new device.
void plVaultAgeNode::AddDevice( const char * deviceName,
	plVaultOperationCallback * cb /*=nil*/, UInt32 cbContext /*=0*/ )
{
	plVaultTextNoteNode * device = GetDevice( deviceName );
	if ( !device && fMyNodeMgr)
	{
		device = plVaultTextNoteNode::ConvertNoRef( fMyNodeMgr->CreateNode( plVault::kNodeType_TextNote, kNewPersistentNodeOptions( fMyNodeMgr ) ) );
		device->SetTitle( deviceName );
		device->SetNoteType( plVault::kNoteType_Device );
		GetAgeDevicesFolder()->AddNode( device, cb, cbContext );
	}
	else
	{
		if ( cb )
		{
			cb->fNode = device;
			cb->VaultOperationStarted( cbContext );
			cb->VaultOperationComplete( cbContext, device->IsStored()?hsOK:hsFail );
		}
	}
}

// Remove a device.
void plVaultAgeNode::RemoveDevice( const char * deviceName )
{
	plVaultTextNoteNode * device = GetDevice( deviceName );
	if ( device )
		GetAgeDevicesFolder()->RemoveNode( device->GetID() );
}

// True if device exists in age.
bool plVaultAgeNode::HasDevice( const char * deviceName )
{
	return ( GetDevice( deviceName )!=nil );
}

plVaultTextNoteNode * plVaultAgeNode::GetDevice( const char * deviceName )
{
	GetAgeDevicesFolder();
	hsAssert( fAgeDevicesFolder, "Missing AgeDevicesFolder" );
	plVaultTextNoteNode tmp;
	tmp.SetTitle( deviceName );
	tmp.SetNoteType( plVault::kNoteType_Device );
	plVaultNodeRef * nodeRef;
	if ( fAgeDevicesFolder->FindNode( &tmp, nodeRef ) )
		return plVaultTextNoteNode::ConvertNoRef( nodeRef->GetChild() );
	hsLogEntry(plSafeGetLog()->AddLineF( "Device %s not found", deviceName ));
	return nil;
}


plVaultAgeLinkNode * plVaultAgeNode::GetSubAgeLink( const plAgeInfoStruct * info ) const
{
	GetSubAgesFolder();
	hsAssert( fSubAgesFolder, "Missing SubAges folder" );
	return IGetLink( fSubAgesFolder, info );
}

plVaultChronicleNode * plVaultAgeNode::FindChronicleEntry( const char * entryName )
{
	GetChronicleFolder();
	hsAssert( fChronicleFolder, "Missing Chronicle folder" );
	plVaultChronicleNode templateNode;
	templateNode.SetName( entryName );
	plVaultNodeRef * tmp;
	if ( fChronicleFolder->FindNode( &templateNode, tmp ) )
		return plVaultChronicleNode::ConvertNoRef( tmp->GetChild() );
	return nil;
}

std::string	plVaultAgeNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Guid:%s] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ),
		GetAgeGuid()->AsStdString().c_str(),
		plVaultNode::AsStdString().c_str() );
	return result;
}


////////////////////////////////////////////////////////////////////

plVaultAdminNode::plVaultAdminNode()
: fAllAgeSDLEventInboxesFolder(nil)
{
	ISetType( plVault::kNodeType_VNodeMgrAdmin );
}

plVaultAdminNode::~plVaultAdminNode()
{
}

std::string	plVaultAdminNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ),
		plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plVaultMarkerNode::plVaultMarkerNode() 
{
	ISetType(plVault::kNodeType_Marker);
}

plVaultMarkerNode::~plVaultMarkerNode()
{
}

void plVaultMarkerNode::SetPosition(const hsPoint3& pos)
{
	// To be efficient, write our floats as UInt32's, so we don't have to allocate
	// a blob or some crap like that.
	UInt32 x = *((UInt32*)&pos.fX);
	UInt32 y = *((UInt32*)&pos.fY);
	UInt32 z = *((UInt32*)&pos.fZ);

	ISetUInt32_1(x);
	ISetUInt32_2(y);
	ISetUInt32_3(z);
}

hsPoint3 plVaultMarkerNode::GetPosition() const
{
	UInt32 uX = IGetUInt32_1();
	UInt32 uY = IGetUInt32_2();
	UInt32 uZ = IGetUInt32_3();

	float x = *((float*)&uX);
	float y = *((float*)&uY);
	float z = *((float*)&uZ);

	return hsPoint3(x, y, z);
}

std::string	plVaultMarkerNode::AsStdString(int level) const
{
	std::string result;
	std::string space( level, ' ' );
	hsPoint3 pos = GetPosition();
	xtl::format( result, "%s%s[Text:%s Pos: %.1f,%.1f,%.1f GPS: %d,%d,%d] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ),
		GetText(), pos.fX, pos.fY, pos.fZ, GetGPSTorans(),GetGPSHSpans(),GetGPSVSpans(), plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plVaultMarkerListNode::plVaultMarkerListNode()
{
	ISetType(plVault::kNodeType_MarkerList);
}

plVaultMarkerListNode::~plVaultMarkerListNode()
{
}

std::string	plVaultMarkerListNode::AsStdString(int level) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[Owner:%s[%d], GameType: %d, RoundLength: %d] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ),
		GetOwnerName(),GetOwnerID(),GetGameType(),GetRoundLength(), plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plVaultSystemNode::plVaultSystemNode()
: fGlobalInboxFolder( nil )
{
	ISetType( plVault::kNodeType_System );
}

plVaultFolderNode * plVaultSystemNode::GetGlobalInbox( void ) const
{
	if ( fGlobalInboxFolder )
		return fGlobalInboxFolder;
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kGlobalInboxFolder );
	plVaultNodeRef * nodeRef;
	if ( FindNode( &tmp, nodeRef ) )
		fGlobalInboxFolder = plVaultFolderNode::ConvertNoRef( nodeRef->GetChild() );
	return fGlobalInboxFolder;
}


std::string	plVaultSystemNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%s%s[CCRAway:%d] %s",
		space.c_str(), plVault::NodeTypeStr( GetType() ), 
		GetCCRAwayStatus(),
		plVaultNode::AsStdString().c_str() );
	return result;
}

////////////////////////////////////////////////////////////////////

plNetVaultServerNode::plNetVaultServerNode()
: fCityInfo( nil )
, fAllPlayersFolderID( 0 )
, fAllAgeGlobalSDLNodesFolderID( 0 )
, fPublicAgesFolderID( 0 )
{
	ISetType( plVault::kNodeType_VNodeMgrServer );
}

plNetVaultServerNode::~plNetVaultServerNode()
{
}

std::string	plNetVaultServerNode::AsStdString( int level ) const
{
	std::string result;
	std::string space( level, ' ' );
	xtl::format( result, "%sVAULT[] %s",
		space.c_str(), plVaultNode::AsStdString().c_str() );
	return result;
}


// End.


#endif