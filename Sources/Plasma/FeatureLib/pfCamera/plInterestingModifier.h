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

#ifndef plInterestingModifier_inc
#define plInterestingModifier_inc

#include "../pnModifier/plSingleModifier.h"
#include "../pnMessage/plMessage.h"
#include "hsResMgr.h"
#include "hsGeometry3.h"
#include "hsStream.h"

class plInputEventMsg;

class plInterestingModifier : public plSingleModifier
{
protected:
	
	enum 
	{
		kTypeInteresting = 0,
		kTypeLookAtMod,
	};

	UInt8		fType;
	hsScalar	fView;

	static hsScalar	fInterestRadius;
	static hsScalar	fInterestWeight;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
	
public:
	plInterestingModifier(){ fType = kTypeInteresting;}
	virtual ~plInterestingModifier(){;}
	
	virtual hsBool MsgReceive(plMessage* msg) {return false;}

	CLASSNAME_REGISTER( plInterestingModifier );
	GETINTERFACE_ANY( plInterestingModifier, plSingleModifier );

	hsScalar GetInterestWeight() { return fInterestWeight; }
	hsScalar GetInterestRadius() { return fInterestRadius; }

	void SetInterestWeight(hsScalar	_InterestRadius) { fInterestWeight =_InterestRadius; }
	void SetInterestRadius(hsScalar	_InterestWeight) { fInterestRadius =_InterestWeight; }
	
	virtual void AddTarget(plSceneObject* so);
	
	void	SetType(UInt8 type) { fType = type; }
	UInt8	GetType() { return fType; }
};



#endif plInterestingModifier_inc
