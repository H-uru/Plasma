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
#ifndef plSimulationInterface_inc
#define plSimulationInterface_inc

#include "plObjInterface.h"

class plPhysical;
struct hsMatrix44;
class hsBounds3Ext;

class plSimulationInterface : public plObjInterface
{
public:
	// Props inc by 1 (bit shift in bitvector).
	enum plSimulationProperties {
		// prop 0 is always disable, , declared in plObjInterface
		kDisable				= 0,	// no more interaction. no interference detection
		kWeightless_DEAD,				// unaffected by gravity, but not massless
		kPinned,						// stop moving. keep colliding.
		kWarp_DEAD,						// keep moving, no colliding (a pattern is emerging here...)
		kUpright_DEAD,					// stand upright (mainly for the player)
		kPassive,						// don't push new positions to sceneobject
		kRotationForces_DEAD,			// rotate using forces 
		kCameraAvoidObject_DEAD,		// camera will try and fly around this obsticle
		kPhysAnim,						// this object is animated, and the animation can apply force
		kStartInactive,					// deactive this object at start time. will reactivate when hit
		kNoSynchronize,					// don't synchronize this physical
		kSuppressed_DEAD,				// physical still exists but is not in simulation: no forces, contact, or reports
		kNoOwnershipChange,				// Don't automatically change net ownership on this physical when it is collided with
		kAvAnimPushable,				// Something the avatar should react to and push against
		kNumProps
	};

protected:
	plPhysical* fPhysical;

	friend class plSceneObject;

	virtual void ISetSceneNode(plKey newNode);

public:
	plSimulationInterface();
	~plSimulationInterface();

	CLASSNAME_REGISTER( plSimulationInterface );
	GETINTERFACE_ANY( plSimulationInterface, plObjInterface );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
	
	void	SetProperty(int prop, hsBool on);
	Int32   GetNumProperties() const { return kNumProps; }

	// Transform settable only, if you want it get it from the coordinate interface.
	void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

	// Bounds are gettable only, they are computed on the physical.
	const hsBounds3Ext GetLocalBounds();
	const hsBounds3Ext GetWorldBounds() const;
	const hsBounds3Ext GetMaxWorldBounds();
	void ClearLinearVelocity();

	virtual hsBool MsgReceive(plMessage* msg);

	// Export only.
	void SetPhysical(plPhysical* phys);
	void ReleaseData();

	plPhysical* GetPhysical() const;
};


#endif // plSimulationInterface_inc
