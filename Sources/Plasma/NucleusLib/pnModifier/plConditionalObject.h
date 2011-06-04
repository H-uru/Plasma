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

#ifndef plConditionalObject_inc
#define plConditionalObject_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsBitVector.h"
#include "../pnNetCommon/plSynchedValue.h"

class plLogicModBase;

class plConditionalObject : public hsKeyedObject
{
private:
	// since 'this' is not derived from synchedObject, its synched values must be associated
	// with it's logicModifier (which is a synchedObject).  Thus it's a synched value 'friend'.
	hsBool							bSatisfied;		
	hsBool							fToggle;
public:
	enum
	{
		kLocalElement	= 0,
		kNOT,
	};
protected:
	plLogicModBase*					fLogicMod;
	hsBitVector						fFlags;
	hsBool							fReset;
public:
	plConditionalObject();
	virtual ~plConditionalObject();

	CLASSNAME_REGISTER( plConditionalObject );
	GETINTERFACE_ANY( plConditionalObject, hsKeyedObject );

	virtual void Read(hsStream* stream, hsResMgr* mgr) { hsKeyedObject::Read(stream, mgr); bSatisfied = stream->ReadBool(); fToggle = stream->ReadBool();}
	virtual void Write(hsStream* stream, hsResMgr* mgr){ hsKeyedObject::Write(stream, mgr); stream->WriteBool( bSatisfied ); stream->WriteBool(fToggle);}

	virtual void SetLogicMod(plLogicModBase* pMod) { fLogicMod = pMod; }

//	virtual hsBool MsgReceive(plMessage* msg) = 0;

	virtual hsBool Satisfied() { if(HasFlag(kNOT)) return !bSatisfied; else return bSatisfied; }
	void SetSatisfied(hsBool b) { bSatisfied=b; }
	hsBool IsToggle() { return fToggle; }
	void SetToggle(hsBool b) { fToggle = b; }

	// this is used if condtiton 1 is dependent on another condition's state at the
	// time of a message coming into condition 1;
	virtual hsBool Verify(plMessage* msg) { return true; }

	virtual void Evaluate() = 0;
	
	virtual void Reset() = 0;
	virtual hsBool ResetOnTrigger() { return fReset; }

	hsBool	HasFlag(int f) const { return fFlags.IsBitSet(f); }
	void	SetFlag(int f) { fFlags.SetBit(f); }
	void	ClearFlag(int which) { fFlags.ClearBit( which ); }


};


#endif // plConditionalObject_inc
