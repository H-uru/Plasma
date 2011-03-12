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
#ifndef _pyNotify_h_
#define _pyNotify_h_

//////////////////////////////////////////////////////////////////////
//
// pyNotify   - a wrapper class to provide interface to send a NotifyMsg
//
//////////////////////////////////////////////////////////////////////

#include "../pnMessage/plNotifyMsg.h"

#include "pyKey.h"
#include "pyGeometry3.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyNotify
{
private:
	plKey			fSenderKey;		// the holder of the who (the modifier) we are
	// the list of receivers that want to be notified
	hsTArray<plKey> fReceivers;

	hsBool			fNetPropagate;
	hsBool			fNetForce;

	// notify message build area
	plNotifyMsg		fBuildMsg;

protected:
	pyNotify(); // only used by python glue, do NOT call
	pyNotify(pyKey& selfkey);

public:
	~pyNotify();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptNotify);
	static PyObject *New(pyKey& selfkey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyNotify object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyNotify); // converts a PyObject to a pyNotify (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void AddPlasmaConstantsClasses(PyObject *m);

	void SetSender(pyKey& selfkey); // only used by python glue, do NOT call

	// methods that will be exposed to Python
	virtual void ClearReceivers();
	virtual void AddReceiver(pyKey* key);
	virtual void SetNetPropagate(hsBool propagate);
	virtual void SetNetForce(hsBool state);
	virtual void SetActivateState(hsScalar state);
	virtual void SetType(Int32 type);

	// add event record helpers
	virtual void AddCollisionEvent( hsBool enter, pyKey* other, pyKey* self );
	virtual void AddPickEvent(hsBool enabled, pyKey* other, pyKey* self, pyPoint3 hitPoint);
	virtual void AddControlKeyEvent( Int32 key, hsBool down );
	virtual void AddVarNumber(const char* name, hsScalar number);
	virtual void AddVarKey(const char* name, pyKey* key);
	virtual void AddFacingEvent( hsBool enabled, pyKey* other, pyKey* self, hsScalar dot);
	virtual void AddContainerEvent( hsBool entering, pyKey* container, pyKey* contained);
	virtual void AddActivateEvent( hsBool active, hsBool activate );
	virtual void AddCallbackEvent( Int32 event );
	virtual void AddResponderState(Int32 state);

	virtual void Send();

};

#endif // _pyNotify_h_
