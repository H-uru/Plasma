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
#ifndef cyDraw_h
#define cyDraw_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyDraw
//
// PURPOSE: Class wrapper to map draw functions to plasma2 message
//

#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

class cyDraw
{
protected:
	plKey			fSender;
	hsTArray<plKey>	fRecvr;
	hsBool			fNetForce;

	cyDraw(plKey sender=nil,const plKey recvr=nil);
public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptDraw);
	static PyObject *New(PyObject *sender = NULL, PyObject* recvr = NULL);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyDraw object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyDraw); // converts a PyObject to a cyDraw (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// setters
	void SetSender(plKey &sender);
	void AddRecvr(plKey &recvr);
	virtual void SetNetForce(hsBool state);

	// Enable draw
	virtual void EnableT(hsBool state);
	virtual void Enable();
	virtual void Disable();
};

#endif  // cyDraw_h
