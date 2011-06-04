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

#ifndef plActivatorConditionalObject_inc
#define plActivatorConditionalObject_inc

#include "../../NucleusLib/pnModifier/plConditionalObject.h"
#include "hsTemplates.h"

class plKey;

class plActivatorConditionalObject : public plConditionalObject
{
protected:

	hsTArray<plKey>	fActivators;

public:
	
	plActivatorConditionalObject();
	~plActivatorConditionalObject(){;}
	
	CLASSNAME_REGISTER( plActivatorConditionalObject );
	GETINTERFACE_ANY( plActivatorConditionalObject, plConditionalObject );
	
	virtual hsBool MsgReceive(plMessage* msg);
	
	void Evaluate(){;}
	void SetActivatorKey(plKey k);
	void Reset() { SetSatisfied(false); }

	virtual void Read(hsStream* stream, hsResMgr* mgr); 
	virtual void Write(hsStream* stream, hsResMgr* mgr);

};

class plActivatorActivatorConditionalObject : public plActivatorConditionalObject
{
public:
	
	plActivatorActivatorConditionalObject(){;}
	~plActivatorActivatorConditionalObject(){;}
	
	CLASSNAME_REGISTER( plActivatorActivatorConditionalObject );
	GETINTERFACE_ANY( plActivatorActivatorConditionalObject, plActivatorConditionalObject );
	
	virtual hsBool MsgReceive(plMessage* msg);
	

};

class plVolActivatorConditionalObject : public plActivatorConditionalObject
{
public:
	
	plVolActivatorConditionalObject(){;}
	~plVolActivatorConditionalObject(){;}
	
	CLASSNAME_REGISTER( plVolActivatorConditionalObject );
	GETINTERFACE_ANY( plVolActivatorConditionalObject, plActivatorConditionalObject );
	
	virtual hsBool MsgReceive(plMessage* msg);
	

};

#endif // plActivatorConditionalObject_inc
