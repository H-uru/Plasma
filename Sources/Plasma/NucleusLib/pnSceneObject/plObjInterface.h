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

#ifndef plInterface_inc
#define plInterface_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnMessage/plRefMsg.h"
#include "plSceneObject.h"
#include "hsStream.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "../pnNetCommon/plSynchedValue.h"
#include "hsBitVector.h"

class hsResMgr;
class plDrawInterface;
class plSimulationInterface;
class plCoordinateInterface;
class plAudioInterface;

//
// Since interfaces are keyed objects with valid uoids and
// may have dynamic data (coordinate interface), they are 
// synched (saved) objects
//
class plObjInterface : public plSynchedObject
{
protected:
	enum {
		kDisable		= 0x0 // Derived interfaces duplicate this, so if you add more here, they need to know.
	};
	friend class plSynchedValueBase;
	friend class plSceneObject;
protected:
	plSceneObject*				fOwner;
	hsBitVector					fProps;

	// SetSceneNode just called by owner. If we're an interface to external data,
	// we need to pass the change on. Otherwise, do nothing.
	virtual void	ISetSceneNode(plKey node) {} 
	plSceneObject*	IGetOwner() const { return fOwner; }
	virtual void	ISetOwner(plSceneObject* owner);
	void			ISetAllProperties(const hsBitVector& b);	

	plDrawInterface* IGetOwnerDrawInterface() { return fOwner ? fOwner->GetVolatileDrawInterface() : nil; }
	plSimulationInterface* IGetOwnerSimulationInterface() { return fOwner ? fOwner->GetVolatileSimulationInterface() : nil; }
	plCoordinateInterface* IGetOwnerCoordinateInterface() { return fOwner ? fOwner->GetVolatileCoordinateInterface() : nil; }
	plAudioInterface* IGetOwnerAudioInterface() { return fOwner ? fOwner->GetVolatileAudioInterface() : nil; }
public:

	plObjInterface();
	~plObjInterface();

	CLASSNAME_REGISTER( plObjInterface );
	GETINTERFACE_ANY( plObjInterface, plSynchedObject );

	virtual hsBool MsgReceive(plMessage* msg);

	const plSceneObject* GetOwner() const { return IGetOwner(); }
	plKey GetOwnerKey() const { return IGetOwner() ? IGetOwner()->GetKey() : nil; }
	
	virtual plKey GetSceneNode() const { return IGetOwner() ? IGetOwner()->GetSceneNode() : nil; } 

	// override SetProperty to pass the prop down to the pool objects 
	virtual void	SetProperty(int prop, hsBool on) { fProps.SetBit(prop, on); }
	
	// shouldn't need to override GetProperty()
	hsBool	GetProperty(int prop) const { return fProps.IsBitSet(prop); }
	virtual Int32   GetNumProperties() const = 0;

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) = 0;

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual void	ReleaseData( void ) { }
};


#endif // plInterface_inc
