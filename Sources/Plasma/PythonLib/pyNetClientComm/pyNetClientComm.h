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
// pyNetClientComm - python wrapper for plNetClientComm class.

#ifndef pyNetClientComm_h_inc
#define pyNetClientComm_h_inc

#include "../plNetClientComm/plNetClientComm.h"
#include "../plStatusLog/plStatusLog.h"

#include "../FeatureLib/pfPython/pyGlueHelpers.h"
#include <python.h>

////////////////////////////////////////////////////////////////////

class pyAgeLinkStruct;
class pyNetServerSessionInfo;
class pyStatusLog;
class pyNetCore;

////////////////////////////////////////////////////////////////
// plNetClientComm Callback Wrappers

// Message handler for unsolicited msgs or registered
// for specific msg types.
class pyNetClientCommMsgHandler : public plNetClientComm::MsgHandler
{
public:
	PyObject*	fPyObject;
	pyNetClientCommMsgHandler( PyObject* pyObject ): fPyObject( pyObject ) {}
	int HandleMessage( plNetMessage* msg );
};

// Receipt handler for changed msg receipts.
class pyNetClientCommRcptHandler : public plNetClientComm::RcptHandler
{
public:
	PyObject*	fPyObject;
	pyNetClientCommRcptHandler( PyObject* pyObject ): fPyObject( pyObject ) {}
	void HandleMsgReceipt( plNetCoreMsgReceipt* rcpt );
};

////////////////////////////////////////////////////////////////////

class pyNetClientComm
{
	// We contain the plNetClientComm we are wrapping.
	plNetClientComm		fNetClient;

protected:

	////////////////////////////////////////////////////////////////

	pyNetClientComm();

public:
	~pyNetClientComm();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptNetClientComm);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyNetClientComm object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyNetClientComm); // converts a PyObject to a pyNetClientComm (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	plNetClientComm * GetNetClientComm() { return &fNetClient; }

	////////////////////////////////////////////////////////////////
	// NETWORK OPERATIONS

	// Auth with active server using auth info set earlier.
	// Will timeout after maxAuthSecs elapsed.
	int		NetAuthenticate( double maxAuthSecs, PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Leave the active server.
	int		NetLeave( UInt8 reason, PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Ping the specified server.
	int		NetPing( int serverType, int timeoutSecs=0, PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Spawn a game for us.
	int		NetFindAge( const pyAgeLinkStruct* linkInfo, PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Get player list.
	int		NetGetPlayerList( PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Set the active player.
	int		NetSetActivePlayer( UInt32 playerID, const char* playerName, PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Create a player
	int		NetCreatePlayer( const char* playerName, const char* avatarShape, UInt32 createFlags, PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Join age
	int		NetJoinAge( PyObject* cbClass=nil, UInt32 cbContext=0 );
	// Set server-side timeout
	int		NetSetTimeout( float timeoutSecs, PyObject* cbClass=nil, UInt32 cbContext=0 );

	////////////////////////////////////////////////////////////////

	// Calls ErrorHandler, if set. Returns value of result
	// that was passed in (for use in return/compound statements).
	int		ReportError( int err, int result );

	////////////////////////////////////////////////////////////////

	// Get/Set Log object
	void	SetLog( pyStatusLog* log );
	void	SetLogByName( const char * name, UInt32 flags=0 );
	PyObject* GetLog() const; // return pyStatusLog

	// NetCore log level
	void	SetLogLevel( int logLevel );

	// Startup/Shutdown this object
	int		Init( bool threaded=true, int logLevel=0 );
	// flushMsgsSecs: time to spend flushing net msgs queued in net core.
	//	<0 means no time limit. spin until all msgs are flushed.
	int		Fini( float flushMsgsSecs=0.f );

	// Call this in your update loop.
	int		Update();

	// Access to the NetCore object.
	pyNetCore*	GetNetCore() const;

	// Get/Set Authentication info
	int		SetAuthInfo( const char* acctName, const char* password );
	const char* GetAcctName() const;
	const char* GetPassword() const;

	// Sets the server we want to communicate with.
	int		SetActiveServer( pyNetServerSessionInfo* nfo );
	int		SetActiveServer2( const char * addr, int port );
	const pyNetServerSessionInfo* GetActiveServer() const;

	// Sets/clears receipt tracking for given message class.
	void	SetReceiptTrackingForType( UInt16 msgClassIdx, bool on );

	// Adds a msg handler for a msg that is convertable to specified type.
	void	AddMsgHandlerForType( UInt16 msgClassIdx, pyNetClientCommMsgHandler* handler );

	// Adds a msg handler for a specific msg type.
	void	AddMsgHandlerForExactType( UInt16 msgClassIdx, pyNetClientCommMsgHandler* handler );

	void	RemoveMsgHandler( pyNetClientCommMsgHandler* handler );

	// Msgs not part of a task controlled by this
	// object, and doesn't have a handler set for its type
	// are sent to this handler (if set).
	void	SetDefaultHandler( pyNetClientCommMsgHandler* msgHandler );

	// Changed message rcpts are sent to this handler if set.
	void	SetMsgReceiptHandler( pyNetClientCommRcptHandler* rcptHandler );

	// Send a message to the server.
	int		SendMsg( plNetMessage* msg, UInt32 sendFlags=0 );
	// Send a message to specified peer
	int		SendMsg( plNetMessage* msg, plNetCore::PeerID peerID, UInt32 sendFlags=0 );

	// Set the alive message send frequency. 0 means don't send periodic alive msgs.
	void	SetAliveFreq( float secs );
	float	GetAliveFreq() const;

	// Set the amount of time before we declare server-silence.
	void	SetServerSilenceTime( float secs );
	float	GetServerSilenceTime() const;

	// Set the maximum amount of time to spend processing
	// incoming msgs per call to Update().
	void	SetMaxMsgProcessingTime( float secs );

};

////////////////////////////////////////////////////////////////////
#endif // pyNetClientComm_h_inc
