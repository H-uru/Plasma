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

#ifndef plLightSpace_inc
#define plLightSpace_inc

class hsStream;
class hsResMgr;

#include "../pnModifier/plMultiModifier.h"

class plLightSpace : public plMultiModifier
{
protected:

	hsKeyedObject*		fLightInfo;

	hsKeyedObject*		IGetLightInfo() const { return fLightInfo; }
public:
	plLightSpace() : fLightInfo(nil) {}
	virtual ~plLightSpace() {}

	CLASSNAME_REGISTER( plLightSpace );
	GETINTERFACE_ANY( plLightSpace, plMultiModifier );

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);


};

#endif // plLightSpace_inc
