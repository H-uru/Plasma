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
#ifndef plTempKey_inc
#define plTempKey_inc

#include "plKeyImp.h"	

class hsKeyedObject;

	//------------------------------------
	// plTempKey is a handle to a keyedObject, which is not registered
	// The key may be handed to others to send messages.
	// when done with a plTempKey call ReleaseTemporary (from Base class plKey)
	//------------------------------------

class plTempKey : public plKeyImp
{

public:
		plTempKey(hsKeyedObject *pO,const char *nm=nil);
		plTempKey(plUoid u) { fUoid=u; }	// used server side
		~plTempKey();		// USE plKey->ReleaseTemporary to delete this...

		virtual void        VerifyLoaded() const;
};


#endif 
