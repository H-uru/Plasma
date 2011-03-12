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
#ifndef pyStatusLog_h_inc
#define pyStatusLog_h_inc

//////////////////////////////////////////////////////////////////////
//
// pyStatusLog   - a wrapper class to provide interface to the plStatusLog stuff
//
//  and interface to the ChatLog (ptChatStatusLog)
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyColor.h"
class plStatusLog;


class pyStatusLog
{
private:
	plStatusLog*	fLog;
	bool	fICreatedLog;

protected:
	pyStatusLog( plStatusLog* log=nil );

public:
	~pyStatusLog();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptStatusLog);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(plStatusLog* log);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyStatusLog object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyStatusLog); // converts a PyObject to a pyStatusLog (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void AddPlasmaConstantsClasses(PyObject *m);

	virtual hsBool Open(const char* logName, UInt32 numLines, UInt32 flags);
	virtual hsBool Write(const char* text);
	virtual hsBool WriteColor(const char* text, pyColor& color);
	virtual void Close();

	virtual hsBool IsOpen();
};


#endif // pyStatusLog_h_inc
