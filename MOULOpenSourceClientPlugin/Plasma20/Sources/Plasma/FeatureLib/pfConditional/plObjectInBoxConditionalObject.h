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

#ifndef plObjectInBoxConditionalObject_inc
#define plObjectInBoxConditionalObject_inc

#include "../../NucleusLib/pnModifier/plConditionalObject.h"
#include "hsTemplates.h"

class plKey;

class plObjectInBoxConditionalObject : public plConditionalObject
{
protected:

	hsTArray<plKey>		fInside;
	plKey				fCurrentTrigger;

public:
	
	plObjectInBoxConditionalObject();
	~plObjectInBoxConditionalObject(){;}
	
	CLASSNAME_REGISTER( plObjectInBoxConditionalObject );
	GETINTERFACE_ANY( plObjectInBoxConditionalObject, plConditionalObject );
	
	hsBool MsgReceive(plMessage* msg);

	void Evaluate(){;}
	void Reset() { SetSatisfied(false); }
	virtual hsBool Satisfied() { return true; }
	virtual hsBool Verify(plMessage* msg);

};

class plVolumeSensorConditionalObject : public plConditionalObject
{

protected:

	hsTArray<plKey>	fInside;
	int					fTrigNum;
	int					fType;
	hsBool				fFirst;
	hsBool				fTriggered;
	hsBool				fIgnoreExtraEnters;
public:

	static bool makeBriceHappyVar;


	enum
	{
		kTypeEnter	= 1,
		kTypeExit,
	};
	plVolumeSensorConditionalObject();
	~plVolumeSensorConditionalObject(){;}
	
	CLASSNAME_REGISTER( plVolumeSensorConditionalObject );
	GETINTERFACE_ANY( plVolumeSensorConditionalObject, plConditionalObject );
	
	virtual hsBool MsgReceive(plMessage* msg);

	void Evaluate(){;}
	void Reset() { SetSatisfied(false); }
	virtual hsBool Satisfied();
	void	SetType(int i) { fType = i; }

	void SetTrigNum(int i) { fTrigNum = i; }
	void SetFirst(hsBool b) { fFirst = b; }

	void IgnoreExtraEnters(hsBool ignore = true) {fIgnoreExtraEnters = ignore;}

	virtual void Read(hsStream* stream, hsResMgr* mgr); 
	virtual void Write(hsStream* stream, hsResMgr* mgr);

};
class plVolumeSensorConditionalObjectNoArbitration : public plVolumeSensorConditionalObject
{
public:
	plVolumeSensorConditionalObjectNoArbitration ():plVolumeSensorConditionalObject(){;}
	~plVolumeSensorConditionalObjectNoArbitration (){;}
	CLASSNAME_REGISTER( plVolumeSensorConditionalObjectNoArbitration );
	GETINTERFACE_ANY( plVolumeSensorConditionalObjectNoArbitration, plConditionalObject );
	virtual hsBool MsgReceive(plMessage* msg);
	virtual void Read(hsStream* stream, hsResMgr* mgr); 
protected:
	plKey fHittee;
};


#endif // plObjectInBoxConditionalObject_inc
