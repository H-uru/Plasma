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
#ifndef plSDLModifier_inc
#define plSDLModifier_inc

#include "../pnModifier/plSingleModifier.h"
#include "../pnNetCommon/plSDLTypes.h"

//
// Base class for modifiers which send/recv State Desc Language (SDL) messages
//
class plStateDataRecord;
class plSimpleStateVariable;
class plSDLModifier : public plSingleModifier
{
protected:
	plStateDataRecord* fStateCache;
	bool	fSentOrRecvdState;
	
	void ISendNetMsg(plStateDataRecord*& state, plKey senderKey, UInt32 sendFlags);		// transmit net msg	
	virtual void IPutCurrentStateIn(plStateDataRecord* dstState) = 0;
	virtual void ISetCurrentStateFrom(const plStateDataRecord* srcState) = 0;
	virtual void ISentState(const plStateDataRecord* sentState) {}
	hsBool IEval(double secs, hsScalar del, UInt32 dirty) {return false;}
	
	virtual UInt32 IApplyModFlags(UInt32 sendFlags);
    
public:
	CLASSNAME_REGISTER( plSDLModifier );
	GETINTERFACE_ANY( plSDLModifier, plSingleModifier);

	plSDLModifier();
	virtual ~plSDLModifier();

	hsBool MsgReceive(plMessage* msg);
	void SendState(UInt32 sendFlags);		// send a state update
	void ReceiveState(const plStateDataRecord* srcState);	// recv a state update
	virtual const char* GetSDLName() const = 0;	// return the string name of the type of state descriptor you handle
	virtual plKey GetStateOwnerKey() const;
	
	plStateDataRecord* GetStateCache() const { return fStateCache; }
	virtual void AddTarget(plSceneObject* so);
	
	void AddNotifyForVar(plKey key, const char* varName, float tolerance) const;
};

#endif	// plSDLModifier_inc
