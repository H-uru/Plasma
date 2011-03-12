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
#include "HeadSpin.h"

#include "plTempKey.h"
#include "hsKeyedObject.h"
#include "plUoid.h"



plTempKey::plTempKey(hsKeyedObject *pO,const char *nm)
{
	hsAssert(pO,"plTempKey, Need Object!");
	pO->SetKey(this);
	plLocation loc( plLocation::kGlobalFixedLoc );
	fUoid = plUoid( loc, pO->ClassIndex(), "temp");
	fUoid.SetTemporary( true );					// Must set temp flag!

#ifdef HS_DEBUGGING
	fIDName = fUoid.GetObjectName();
	fClassType = plFactory::GetNameOfClass( fUoid.GetClassType() );
#endif
}

plTempKey::~plTempKey() 
{ 
	UnRegister();
	delete fObjectPtr;
	fObjectPtr = nil;
}

void plTempKey::VerifyLoaded() const
{
	// Can't do a whole lot in this case for tempKeys :)
}

