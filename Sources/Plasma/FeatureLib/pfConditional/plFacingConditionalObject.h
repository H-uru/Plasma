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

#ifndef plFacingConditionalObject_inc
#define plFacingConditionalObject_inc

#include "../../NucleusLib/pnModifier/plConditionalObject.h"

class plSceneObject;

class plFacingConditionalObject : public plConditionalObject
{
protected:
	hsScalar			fTolerance;
	hsBool				fDirectional;

public:
	
	plFacingConditionalObject();
	~plFacingConditionalObject(){;}
	
	CLASSNAME_REGISTER( plFacingConditionalObject );
	GETINTERFACE_ANY( plFacingConditionalObject, plConditionalObject );
	
	hsBool MsgReceive(plMessage* msg);

	void SetTolerance(hsScalar d) { fTolerance = d; }
	void SetDirectional(hsBool d) { fDirectional = d; }

	virtual hsBool Verify(plMessage* msg);

	void Evaluate(){;}
	void Reset() { SetSatisfied(true); }

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

};


#endif // plFacingConditionalObject_inc
