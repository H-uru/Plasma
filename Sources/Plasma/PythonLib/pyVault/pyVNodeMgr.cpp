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
#include "pyVNodeMgr.h"
#include "../plVault/plVaultCallback.h"
#include "../plVault/plVaultInitTasks.h"
#include "../pfPython/pyVaultNode.h"
#include "../pfPython/pyVaultFolderNode.h"
#include "../pyNetClientComm/pyNetClientComm.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plStatusLog/plStatusLog.h"

////////////////////////////////////////////////////////////////////

class pyVaultOperationCallback : public plVaultOperationCallback
{
public:
	PyObject * fPyObject;
	pyVaultOperationCallback( PyObject * pyObject )
		: fPyObject( pyObject )
	{
		Py_XINCREF( fPyObject );
	}
	~pyVaultOperationCallback()
	{
		Py_XDECREF( fPyObject );
	}
	void VaultOperationStarted( UInt32 context )
	{
		if ( fPyObject )
		{
			// Do callback
			PyObject* func = PyObject_GetAttrString( fPyObject, "operationStarted" );
			if ( func )
			{
				if ( PyCallable_Check(func)>0 )
				{
					PyObject* retVal = PyObject_CallMethod(fPyObject, "operationStarted", "l", context);
					Py_XDECREF(retVal);
				}
			}
		}
	}
	void VaultOperationComplete( UInt32 context, int resultCode )
	{
		if ( fPyObject )
		{
			// Pass args.
//			PyObject* pyArgs = PyObject_GetAttrString( fPyObject, "fCbArgs" );
//			if ( pyArgs )
//			{
//				dict pyDict = dict();
//				std::map<UInt16,plCreatable*>	args;
//				fCbArgs.GetItems( args );
//				for ( std::map<UInt16,plCreatable*>::iterator ii=args.begin(); ii!=args.end(); ++ii )
//				{
//					UInt16 key = ii->first;
//					plCreatable* arg = ii->second;
//					plCreatableGenericValue * genValue = plCreatableGenericValue::ConvertNoRef( arg );
//					if ( genValue )
//					{
//						plGenericType & value = genValue->Value();
//						switch ( value.GetType() )
//						{
//						case plGenericType::kInt:
//							pyDict[key] = (int)value;
//							break;
//						case plGenericType::kUInt:
//							pyDict[key] = (unsigned int)value;
//							break;
//						case plGenericType::kFloat:
//							pyDict[key] = (float)value;
//							break;
//						case plGenericType::kDouble:
//							pyDict[key] = (double)value;
//							break;
//						case plGenericType::kBool:
//							pyDict[key] = (bool)value;
//							break;
//						case plGenericType::kChar:
//							pyDict[key] = (char)value;
//							break;
//						case plGenericType::kString:
//							pyDict[key] = (const char *)value;
//							break;
//						case plGenericType::kAny:
//							break;
//						case plGenericType::kNone:
//							break;
//						}
//					}
//				}
//				PyObject_SetAttrString( fPyObject, "fCbArgs", pyDict.ptr() );
//			}
			// Do callback
			PyObject* func = PyObject_GetAttrString( fPyObject, "operationComplete" );
			if ( func )
			{
				if ( PyCallable_Check(func)>0 )
				{
					PyObject* retVal = PyObject_CallMethod(fPyObject, "operationComplete", "li", context, resultCode);
					Py_XDECREF(retVal);
				}
			}
		}
		delete this;
	}
};


class pyVaultCallback : public plVaultStubbedCallback
{
public:
	PyObject * fPyObject;
	pyVaultCallback( PyObject * pyObject )
		: fPyObject( pyObject )
	{
		Py_XINCREF( fPyObject );
	}
	~pyVaultCallback()
	{
		Py_XDECREF( fPyObject );
	}
};


////////////////////////////////////////////////////////////////////

int pyVNodeMgr::VaultMsgHandler::HandleMessage( plNetMessage* msg )
{
	plNetMsgVault * vaultMsg = plNetMsgVault::ConvertNoRef( msg );

	if ( vaultMsg )
	{
		plNetCoreMessage * ncmsg = (plNetCoreMessage*)msg->GetNetCoreMsg();
		msg->PeekBuffer( ncmsg->GetData(), ncmsg->GetLen() );
		fMyVNodeMgr->GetStatusLog()->AddLineF( "\t%s", msg->AsStdString().c_str() );
		plVault::ProcessMsg( vaultMsg );
		return plNetClientComm::kOK_MsgConsumed;
	}

	return hsFail;
}


// pyVNodeMgr ----------------------------------------------
pyVNodeMgr::pyVNodeMgr( PyObject* thaComm )
{
	if (!pyNetClientComm::Check(thaComm))
	{
		fMyCommObj = NULL;
		return; // screwed!
	}

	fMsgHandler.setMgr(this);

	fMyCommObj = thaComm;
	Py_INCREF(fMyCommObj);
	fMyComm = pyNetClientComm::ConvertFrom(fMyCommObj);
	fMyComm->GetNetClientComm()->AddMsgHandlerForType( plNetMsgVault::Index(), &fMsgHandler );
	plVNodeMgr::SetStatusLog( fMyComm->GetNetClientComm()->GetLog(), false );
}

// ~pyVNodeMgr ----------------------------------------------
pyVNodeMgr::~pyVNodeMgr()
{
	fMyComm->GetNetClientComm()->RemoveMsgHandler( &fMsgHandler );
	Py_DECREF(fMyCommObj);
}

void pyVNodeMgr::setMyComm(PyObject* thaComm)
{
	if (fMyComm)
	{
		fMyComm->GetNetClientComm()->RemoveMsgHandler(&fMsgHandler);
		Py_DECREF(fMyCommObj);
		fMyCommObj = NULL;
		fMyComm = NULL;
	}
	if (!pyNetClientComm::Check(thaComm))
		return; // screwed!

	fMyCommObj = thaComm;
	Py_INCREF(fMyCommObj);
	fMyComm = pyNetClientComm::ConvertFrom(fMyCommObj);
	fMyComm->GetNetClientComm()->AddMsgHandlerForType(plNetMsgVault::Index(), &fMsgHandler);
	plVNodeMgr::SetStatusLog(fMyComm->GetNetClientComm()->GetLog(), false);
}

// IAmOnline ----------------------------------------------
bool pyVNodeMgr::IAmOnline() const
{
	return true;
}

// IIsThisMe ----------------------------------------------
bool pyVNodeMgr::IIsThisMe( plVaultPlayerInfoNode* node ) const
{
	return ( fMyComm->GetNetClientComm()->GetPlayerID()==node->GetPlayerID() );
}

// IIsThisMe ----------------------------------------------
bool pyVNodeMgr::IIsThisMe( plVaultPlayerNode * node ) const
{
	return ( fMyComm->GetNetClientComm()->GetPlayerID()==node->GetID() );
}

// ISendNetMsg ----------------------------------------------
int pyVNodeMgr::ISendNetMsg( plNetMsgVault* msg, UInt32 sendFlags/*=0 */)
{
	return fMyComm->GetNetClientComm()->SendMsg( msg, sendFlags );
}

// IGetPlayerID ----------------------------------------------
UInt32 pyVNodeMgr::IGetPlayerID() const
{
	return fMyComm->GetNetClientComm()->GetPlayerID();
}

// Update ----------------------------------------------
int pyVNodeMgr::Update( double secs )
{
	return plVNodeMgr::Update( secs );
}

// Startup ----------------------------------------------
void pyVNodeMgr::Startup()
{
	plVNodeMgr::Startup();
}

// Shutdown ----------------------------------------------
void pyVNodeMgr::Shutdown()
{
	plVNodeMgr::Shutdown();
}


// IsConnected ----------------------------------------------
bool pyVNodeMgr::IsConnected()
{
	return plVNodeMgr::IsConnected();
}

// Disconnect ----------------------------------------------
void pyVNodeMgr::Disconnect( PyObject* cb/*=nil*/, UInt32 cbContext/*=0 */)
{
	// disconnect from allplayers and globalsdl folders
	plVaultNodeRef * out;
	plVaultNode * root = plVNodeMgr::GetRootNode();
	if ( root )
	{
		plVaultFolderNode tmpGlobalSDL;
		tmpGlobalSDL.SetFolderType( plVault::kAllAgeGlobalSDLNodesFolder );
		if ( root->FindNode( &tmpGlobalSDL, out ) )
			root->RemoveNode( out->GetChildID() );
		plVaultFolderNode tmpAllPlayers;
		tmpAllPlayers.SetFolderType( plVault::kAllPlayersFolder );
		if ( root->FindNode( &tmpAllPlayers, out ) )
			root->RemoveNode( out->GetChildID() );
	}
	plVNodeMgr::Disconnect( new pyVaultOperationCallback( cb ), cbContext );
}

// Connect ----------------------------------------------
void pyVNodeMgr::Connect( int childFetchLevel/*=plVault::kFetchAllChildren*/, PyObject* cb/*=nil*/, UInt32 cbContext/*=0 */)
{
	plVNodeMgr::Connect( childFetchLevel, new pyVaultOperationCallback( cb ), cbContext );
}

// FetchNode ----------------------------------------------
bool pyVNodeMgr::FetchNode( UInt32 nodeID,
	int childFetchLevel/*=plVault::kFetchAllChildren*/,
	PyObject* cb/*=nil*/,
	UInt32 cbContext/*=0 */)
{
	return plVNodeMgr::FetchNode( nodeID, childFetchLevel, new pyVaultOperationCallback( cb ), cbContext );
}

// GetRootNode ----------------------------------------------
PyObject* pyVNodeMgr::GetRootNode() const
{
	return pyVaultNode::New( plVNodeMgr::GetRootNode() );
}

// GetClientID ----------------------------------------------
UInt32 pyVNodeMgr::GetClientID() const
{
	return plVNodeMgr::GetClientID();
}

// GetNode ----------------------------------------------
PyObject* pyVNodeMgr::GetNode( UInt32 id ) const
{
	plVaultNode * tmp;
	if ( plVNodeMgr::GetNode( id, tmp ) )
		return pyVaultNode::New( tmp );
	PYTHON_RETURN_NONE;
}

// FindNode ----------------------------------------------
PyObject* pyVNodeMgr::FindNode( pyVaultNode* templateNode ) const
{
	plVaultNode * node;
	if ( plVNodeMgr::FindNode( templateNode->GetNode(), node ) )
		return pyVaultNode::New( node );
	PYTHON_RETURN_NONE;
}

// EnableCallbacks ----------------------------------------------
bool pyVNodeMgr::EnableCallbacks( bool b )
{
	return plVNodeMgr::EnableCallbacks( b );
}

// AddCallback ----------------------------------------------
void pyVNodeMgr::AddCallback( PyObject* cb )
{
	pyVaultCallback * pycb = new pyVaultCallback( cb );
	fPyCallbacks.push_back( pycb );
	plVNodeMgr::AddCallback( pycb );
}

// RemoveCallback ----------------------------------------------
void pyVNodeMgr::RemoveCallback( PyObject* cb )
{
	PyCallbackVec	tmp;
	for ( int i=0; i<fPyCallbacks.size(); i++ )
	{
		if ( fPyCallbacks[i]->fPyObject==cb )
			tmp.push_back( fPyCallbacks[i] );
	}
	for ( int i=0; i<tmp.size(); i++ )
	{
		PyCallbackVec::iterator it = std::find( fPyCallbacks.begin(), fPyCallbacks.end(), tmp[i] );
		if ( it!=fPyCallbacks.end() )
			fPyCallbacks.erase( it );
		plVNodeMgr::RemoveCallback( tmp[i] );
		delete tmp[i];
	}
}

// CreateNode ----------------------------------------------
PyObject* pyVNodeMgr::CreateNode( int nodeType, bool persistent )
{
	plVaultNode * node = plVNodeMgr::CreateNode( nodeType, persistent?kNewPersistentNodeOptions( this ):kNilNodeOptions() );
	if ( node )
		return pyVaultNode::New( node );
	PYTHON_RETURN_NONE;
}

// Dump ----------------------------------------------
void pyVNodeMgr::Dump() const
{
	plVNodeMgr::Dump();
}

////////////////////////////////////////////////////////////////////////

pyPlayerVNodeMgr::pyPlayerVNodeMgr( PyObject* thaComm )
: pyVNodeMgr( thaComm )
{}

// IAmSuperUser ----------------------------------------------
bool pyPlayerVNodeMgr::IAmSuperUser( void ) const
{
	return false;
}

// IFillOutConnectFields ----------------------------------------------
void pyPlayerVNodeMgr::IFillOutConnectFields( plNetMsgVault* msg ) const
{
	msg->GetArgs()->AddInt( plVault::kArg_NodeMgrType, plVault::kNodeType_VNodeMgrPlayer );
	msg->GetArgs()->AddInt( plVault::kArg_NodeMgrID, fMyComm->GetNetClientComm()->GetPlayerID() );
}

// IGetNodeInitializationTask ----------------------------------------------
plVNodeInitTask * pyPlayerVNodeMgr::IGetNodeInitializationTask( plVaultNode * node )
{
	if ( plVaultPlayerNode::ConvertNoRef( node ) )
		return new plVaultPlayerInitializationTask( this, node, true );
	return nil;
}

////////////////////////////////////////////////////////////////////////

pyAgeVNodeMgr::pyAgeVNodeMgr( PyObject* thaComm )
: pyVNodeMgr( thaComm )
{}

// IAmSuperUser ----------------------------------------------
bool pyAgeVNodeMgr::IAmSuperUser( void ) const
{
	return false;
}

// IFillOutConnectFields ----------------------------------------------
void pyAgeVNodeMgr::IFillOutConnectFields( plNetMsgVault* msg ) const
{
	msg->GetArgs()->AddInt( plVault::kArg_NodeMgrType, plVault::kNodeType_VNodeMgrAge );
	msg->GetArgs()->AddString( plVault::kArg_NodeMgrAgeInstanceName, fAgeFilename.c_str() );
	msg->GetArgs()->AddItem( plVault::kArg_NodeMgrAgeGuid, &fAgeInstanceGuid );
}

// IGetNodeInitializationTask ----------------------------------------------
plVNodeInitTask * pyAgeVNodeMgr::IGetNodeInitializationTask( plVaultNode * node )
{
	if ( plVaultAgeNode::ConvertNoRef( node ) )
		return new plVaultAgeInitializationTask( this, node, nil, true );
	if ( plVaultAgeInfoNode::ConvertNoRef( node ) )
		return new plVaultAgeInfoInitializationTask( this, node );
	return nil;
}

// SetAgeInfo ----------------------------------------------
void pyAgeVNodeMgr::SetAgeInfo( const char * ageFilename, const char * ageInstanceGuid )
{
	fAgeFilename = ageFilename;
	fAgeInstanceGuid.FromString( ageInstanceGuid );
}

////////////////////////////////////////////////////////////////////////

pyAdminVNodeMgr::pyAdminVNodeMgr( PyObject* thaComm )
: pyVNodeMgr( thaComm )
, fWantGlobalSDL( true )
, fWantAllPlayers( false )
{}

// IAmSuperUser ----------------------------------------------
bool pyAdminVNodeMgr::IAmSuperUser( void ) const
{
	return true;
}

// IFillOutConnectFields ----------------------------------------------
void pyAdminVNodeMgr::IFillOutConnectFields( plNetMsgVault* msg ) const
{
	msg->GetArgs()->AddInt( plVault::kArg_NodeMgrType, plVault::kNodeType_VNodeMgrAdmin );
	msg->GetArgs()->AddInt( plVault::kArg_NodeMgrID, fMyComm->GetNetClientComm()->GetPlayerID() );
}

// IGetNodeInitializationTask ----------------------------------------------
plVNodeInitTask * pyAdminVNodeMgr::IGetNodeInitializationTask( plVaultNode * node )
{
	if ( plVaultAdminNode::ConvertNoRef( node ) )
		return new plVaultAdminInitializationTask( this, node, fWantGlobalSDL, fWantAllPlayers );
	return nil;
}

// GetGlobalInbox ----------------------------------------------
PyObject * pyAdminVNodeMgr::GetGlobalInbox() const
{
	plVaultFolderNode tmp;
	tmp.SetFolderType( plVault::kGlobalInboxFolder );
	plVaultNode * node;
	if ( plVNodeMgr::FindNode( &tmp, node ) )
	{
		return pyVaultFolderNode::New( plVaultFolderNode::ConvertNoRef( node ) );
	}
	PYTHON_RETURN_NONE;
}

////////////////////////////////////////////////////////////////////
// End.
