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

#ifndef plSceneObject_inc
#define plSceneObject_inc

#include "hsBitVector.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "../pnNetCommon/plSynchedValue.h"
#include "../pnModifier/plModifier.h"
#include "hsStream.h"

class plObjInterface;
class plDrawInterface;
class plSimulationInterface;
class plCoordinateInterface;
class plAudioInterface;
class plMessage;
class hsStream;
class hsResMgr;
class plMessage;
class plDispatchBase;
struct hsMatrix44;

// The following two aren't dragging the Conversion side into the runtime.
// They are just to let the converter do things we don't want done at runtime.
// Nice that we can make friends with these opaque classes.
class plMaxNode;
class plMaxNodeBase;

class plSceneObject : public plSynchedObject {
	friend class plSynchedValueBase;
private:
	plDrawInterface*			GetVolatileDrawInterface() { return fDrawInterface; }
	plSimulationInterface*		GetVolatileSimulationInterface() { return fSimulationInterface; }
	plCoordinateInterface*		GetVolatileCoordinateInterface() { return fCoordinateInterface; }
	plAudioInterface*			GetVolatileAudioInterface() { return fAudioInterface; }
	plObjInterface*				GetVolatileGenericInterface(UInt16 classIdx) const;

	plModifier*					GetVolatileModifier(int i) { return fModifiers[i]; }

	plKey						fSceneNode;

	friend class plModifier;
	friend class plCoordinateInterface;
	friend class plObjInterface;
	friend class plMaxNode;
	friend class plMaxNodeBase;

	hsBool IMsgHandle(plMessage* msg);
protected:

	plDrawInterface*			fDrawInterface;
	plSimulationInterface*		fSimulationInterface;
	plCoordinateInterface*		fCoordinateInterface;
	plAudioInterface*			fAudioInterface;

	hsTArray<plModifier*>		fModifiers;

	hsTArray<plObjInterface*> fGenerics;

	void					ISetDrawInterface(plDrawInterface* di);
	void					ISetSimulationInterface(plSimulationInterface* si);
	void					ISetAudioInterface(plAudioInterface* ai);
	void					ISetCoordinateInterface(plCoordinateInterface* ci);

	void					IAddGeneric(plObjInterface* gen);
	void					IRemoveGeneric(plObjInterface* gen);
	void					IRemoveAllGenerics();
	void					IPropagateToGenerics(plMessage* msg);
	void					IPropagateToGenerics(const hsBitVector& types, plMessage* msg);

	void					IAddModifier(plModifier* mo, int i);
	void					IRemoveModifier(plModifier* mo);
	hsBool					IPropagateToModifiers(plMessage* msg);

	void					ISetInterface(plObjInterface* iface);
	void					IRemoveInterface(plObjInterface* iface);
	void					IRemoveInterface(Int16 idx, plObjInterface* iface=nil);

	void					ISetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

public:

	plSceneObject();
	virtual ~plSceneObject();

	CLASSNAME_REGISTER( plSceneObject );
	GETINTERFACE_ANY( plSceneObject, plSynchedObject );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual const plDrawInterface*				GetDrawInterface() const { return fDrawInterface; }
	virtual const plSimulationInterface*		GetSimulationInterface() const { return fSimulationInterface; }
	virtual const plCoordinateInterface*		GetCoordinateInterface() const { return fCoordinateInterface; }
	virtual const plAudioInterface*				GetAudioInterface() const { return fAudioInterface; }

	int						GetNumGenerics() const { return fGenerics.GetCount(); }
	const plObjInterface*	GetGeneric(int i) const { return fGenerics[i]; }

	plObjInterface*			GetGenericInterface(UInt16 classIdx) const { return GetVolatileGenericInterface(classIdx); }

	int						GetNumModifiers() const { return fModifiers.GetCount(); }
	const plModifier*		GetModifier(int i) const { return fModifiers[i]; }
	const plModifier*		GetModifierByType(UInt16 classIdx) const;

	virtual hsBool MsgReceive(plMessage* msg);
	virtual hsBool Eval(double secs, hsScalar del);

	void					SetSceneNode(plKey newNode);
	plKey					GetSceneNode() const;

	// Network only strange function. Do not emulate or generalize this functionality.
	virtual void SetNetGroup(plNetGroupId netGroup);

	virtual void	ReleaseData( void );

	// Force an immediate re-sync of the transforms in the hierarchy this object belongs to,
	// as opposed to waiting for the plTransformMsg to resync.
	void FlushTransform(); 
	void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
	hsMatrix44 GetLocalToWorld() const;
	hsMatrix44 GetWorldToLocal() const;
	hsMatrix44 GetLocalToParent() const;
	hsMatrix44 GetParentToLocal() const;

	hsBool	IsFinal();	// "is ready to process Loads"	

	// Export only
	virtual void SetDrawInterface(plDrawInterface* di);
	virtual void SetSimulationInterface(plSimulationInterface* si);
	virtual void SetAudioInterface(plAudioInterface* ai);
	virtual void SetCoordinateInterface(plCoordinateInterface* ci);

	virtual void AddModifier(plModifier* mo);
	virtual void RemoveModifier(plModifier* mo);
};

#endif // plSceneObject_inc
