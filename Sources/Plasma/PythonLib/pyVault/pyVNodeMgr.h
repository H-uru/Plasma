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
////////////////////////////////////////////////////////////////////
// pyVNodeMgr - python wrapper for plVNodeMgr class.

#ifndef pyVNodeMgr_h_inc
#define pyVNodeMgr_h_inc

#include "../plVault/plVaultClient.h"
#include "../plNetClientComm/plNetClientComm.h"

#include "../FeatureLib/pfPython/pyGlueHelpers.h"
#include <python.h>

////////////////////////////////////////////////////////////////////

class pyVaultNode;
class pyVaultCallback;
class pyStatusLog;
class pyNetClientComm;
class pyVaultFolderNode;

////////////////////////////////////////////////////////////////////

class pyVNodeMgr : public plVNodeMgr
{
private:
	typedef std::vector<pyVaultCallback*> PyCallbackVec;
	PyCallbackVec	fPyCallbacks;

	class VaultMsgHandler : public plNetClientComm::MsgHandler
	{
	private:
		pyVNodeMgr *	fMyVNodeMgr;
		int HandleMessage( plNetMessage* msg );
	public:
		VaultMsgHandler(): fMyVNodeMgr(nil) {}
		void setMgr(pyVNodeMgr * thaNodeMgr) {fMyVNodeMgr = thaNodeMgr;}
	};
	friend class VaultMsgHandler;
	VaultMsgHandler	fMsgHandler;

protected:
	PyObject*			fMyCommObj;
	pyNetClientComm*	fMyComm; // pointer to object stored in fMyCommObj

	bool IAmOnline() const;
	bool IIsThisMe( plVaultPlayerInfoNode* node ) const;
	bool IIsThisMe( plVaultPlayerNode * node ) const;
	int ISendNetMsg( plNetMsgVault* msg, UInt32 sendFlags=0 );
	UInt32 IGetPlayerID() const;

	pyVNodeMgr(): fMyComm(nil) {fMsgHandler.setMgr(this);} // for python glue only, do NOT call
	pyVNodeMgr( PyObject* thaComm );

public:
	~pyVNodeMgr();

	void setMyComm(PyObject* thaComm); // for python glue only, do NOT call

	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE;
	PYTHON_CLASS_NEW_FRIEND(ptVNodeMgr);
	static PyObject* New(PyObject* thaComm);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVNodeMgr object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVNodeMgr); // converts a PyObject to a pyVNodeMgr (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	PyObject * GetNetClient() const { Py_INCREF(fMyCommObj); return fMyCommObj; } // returns pyNetClientComm

	/////////////////////////////////////////////////
	// Vault Client API

	int Update( double secs );

	void Startup();
	void Shutdown();

	// connect/disconnect
	bool IsConnected();
	void Disconnect(
		PyObject* cb=nil,
		UInt32 cbContext=0 );
	void Connect(
		int childFetchLevel=plVault::kFetchAllChildren,
		PyObject* cb=nil,
		UInt32 cbContext=0 );
	// TODO: Glue this.
	// Fetch matching node from server and hold onto it.
	// Note: You won't receive notifications about the fetched node or
	// it's children until it has been added to your root node or any
	// of it's children.
//	bool FetchNode(
//		pyVaultNode* templateNode,
//		int childFetchLevel=plVault::kFetchAllChildren,
//		bool allowCreate = false,
//		PyObject* cb=nil,
//		UInt32 cbContext=0 );
	bool FetchNode( UInt32 nodeID,
		int childFetchLevel=plVault::kFetchAllChildren,
		PyObject* cb=nil,
		UInt32 cbContext=0 );

	// get our root node
	PyObject* GetRootNode() const; // returns pyVaultNode
	// get the client node ID returned to us by the server ( if we didn't
	//	fetch when we connected then we have to use this to identify ourselves ).
	UInt32	GetClientID() const;
	// search all nodes in client locally
	PyObject* GetNode( UInt32 id ) const; // returns pyVaultNode
	// TODO: Glue these.
	PyObject* FindNode( pyVaultNode* templateNode ) const; // returns pyVaultNode
//	bool	FindNodes( const pyVaultNode* templateNode, PyObject * out ) const;
	// callback management
	bool	EnableCallbacks( bool b );	// returns previous enabled setting.
	void	AddCallback( PyObject* cb );
	void	RemoveCallback( PyObject* cb );

	// create a node of the given type.
	PyObject* CreateNode( int nodeType, bool persistent ); // returns pyVaultNode

	// dump contents to log
	void	Dump() const;
};

////////////////////////////////////////////////////////////////////

class pyPlayerVNodeMgr : public pyVNodeMgr
{
protected:
	bool IAmSuperUser( void ) const;
	void IFillOutConnectFields( plNetMsgVault* msg ) const;
	plVNodeInitTask * IGetNodeInitializationTask( plVaultNode * node );

	pyPlayerVNodeMgr(): pyVNodeMgr() {} // for python glue only, do NOT call
	pyPlayerVNodeMgr( PyObject* thaComm );

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptPlayerVNodeMgr);
	static PyObject* New(PyObject* thaComm);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyPlayerVNodeMgr object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyPlayerVNodeMgr); // converts a PyObject to a pyPlayerVNodeMgr (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
};

////////////////////////////////////////////////////////////////////

class pyAgeVNodeMgr : public pyVNodeMgr
{
	std::string		fAgeFilename;
	plServerGuid	fAgeInstanceGuid;

protected:
	bool IAmSuperUser( void ) const;
	void IFillOutConnectFields( plNetMsgVault* msg ) const;
	plVNodeInitTask * IGetNodeInitializationTask( plVaultNode * node );

	pyAgeVNodeMgr(): pyVNodeMgr() {} // for python glue only, do NOT call
	pyAgeVNodeMgr( PyObject* thaComm );

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAgeVNodeMgr);
	static PyObject* New(PyObject* thaComm);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeVNodeMgr object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeVNodeMgr); // converts a PyObject to a pyAgeVNodeMgr (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	void SetAgeInfo( const char * ageFilename, const char * ageInstanceGuid );
};

////////////////////////////////////////////////////////////////////

class pyAdminVNodeMgr : public pyVNodeMgr
{
private:
	bool	fWantGlobalSDL;
	bool	fWantAllPlayers;

protected:
	bool IAmSuperUser( void ) const;
	void IFillOutConnectFields( plNetMsgVault* msg ) const;
	plVNodeInitTask * IGetNodeInitializationTask( plVaultNode * node );

	pyAdminVNodeMgr(): pyVNodeMgr(), fWantGlobalSDL(true), fWantAllPlayers(false) {} // for python glue only, do NOT call
	pyAdminVNodeMgr( PyObject* thaComm );

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAdminVNodeMgr);
	static PyObject* New(PyObject* thaComm);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAdminVNodeMgr object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAdminVNodeMgr); // converts a PyObject to a pyAdminVNodeMgr (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	void SetWantGlobalSDL( bool v ) { fWantGlobalSDL=v; }
	void SetWantAllPlayers( bool v ) { fWantAllPlayers=v; }

	PyObject * GetGlobalInbox() const; // returns pyVaultFolderNode
};

////////////////////////////////////////////////////////////////////
#endif // pyVNodeMgr_h_inc
