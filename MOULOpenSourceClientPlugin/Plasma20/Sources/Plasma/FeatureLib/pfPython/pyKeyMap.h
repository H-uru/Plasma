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
#ifndef _pyKeyMap_h_
#define _pyKeyMap_h_

//////////////////////////////////////////////////////////////////////
//
// pyKeyMap   - a wrapper class all the Key mapping functions
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "../pnInputCore/plKeyMap.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyKeyMap
{
protected:
	pyKeyMap() {};

private:
	plKeyCombo IBindKeyToVKey( const char *keyStr );


public:
	enum
	{
		kShift	= 0x01,
		kCtrl	= 0x02
	};

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptKeyMap);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyKeyMap object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyKeyMap); // converts a PyObject to a pyKeyMap (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// conversion functions
	const char* ConvertVKeyToChar( UInt32 vk, UInt32 flags );
	UInt32 ConvertCharToVKey( const char *charVKey );
	UInt32 ConvertCharToFlags( const char *charVKey );

	UInt32 ConvertCharToControlCode(const char* charCode);
	const char* ConvertControlCodeToString( UInt32 code );


	// bind a key to an action
	void BindKey( const char* keyStr1, const char* keyStr2, const char* act);
	void BindKeyToConsoleCommand( const char* keyStr1, const char* command);

	UInt32 GetBindingKey1(UInt32 code);
	UInt32 GetBindingFlags1(UInt32 code);
	UInt32 GetBindingKey2(UInt32 code);
	UInt32 GetBindingFlags2(UInt32 code);

	UInt32 GetBindingKeyConsole(const char* command);
	UInt32 GetBindingFlagsConsole(const char* command);

	void WriteKeyMap();

};

#endif // _pyKeyMap_h_
