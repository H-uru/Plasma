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
#ifndef plPXPhysical_h_inc
#define plPXPhysical_h_inc

#include "plPhysical.h"	
#include "hsMatrix44.h"
#include "../plPhysical/plSimDefs.h"
#include "hsBitVector.h"
#include "hsUtils.h"

class NxActor;
class NxConvexMesh;
class NxTriangleMesh;

struct hsPoint3;
class hsQuat;
class plPhysicalProxy;
class plDrawableSpans;
class hsGMaterial;
struct hsPlane3;

class plMessage;
class plLOSHit;
class plSimulationMsg;
class plSDLModifier;
class plPhysicalSndGroup;
class plGenRefMsg;
class plSceneObject;
class hsVectorStream;
class NxCapsule;

class PhysRecipe
{
public:
	PhysRecipe();

	hsScalar mass;
	hsScalar friction;
	hsScalar restitution;
	plSimDefs::Bounds bounds;
	plSimDefs::Group group;
	UInt32 reportsOn;
	plKey objectKey;
	plKey sceneNode;
	plKey worldKey;

	// The local to subworld matrix (or local to world if worldKey is nil)
	hsMatrix44 l2s;

	NxConvexMesh* convexMesh;
	NxTriangleMesh* triMesh;

	// For spheres only
	hsScalar radius;
	hsPoint3 offset;

	// For Boxes
	hsPoint3 bDimensions;
	hsPoint3 bOffset;

	// For export time only.  The original data used to create the mesh
	hsVectorStream* meshStream;
};

class plPXPhysical : public plPhysical
{
public:
	friend class plSimulationMgr;

	enum PhysRefType
	{
		kPhysRefWorld,
		kPhysRefSndGroup
	};

	plPXPhysical();
	virtual ~plPXPhysical();

	CLASSNAME_REGISTER(plPXPhysical);
	GETINTERFACE_ANY(plPXPhysical, plPhysical);

	// Export time and internal use only
	hsBool Init(PhysRecipe& recipe);

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	//
	// From plPhysical
	//
	virtual plPhysical& SetProperty(int prop, hsBool b);
	virtual hsBool GetProperty(int prop) const { return fProps.IsBitSet(prop) != 0; }

	virtual void SetObjectKey(plKey key) { fObjectKey = key; }
	virtual plKey GetObjectKey() const { return fObjectKey; }

	virtual void SetSceneNode(plKey node);
	virtual plKey GetSceneNode() const;

	virtual hsBool GetLinearVelocitySim(hsVector3& vel) const;
	virtual void SetLinearVelocitySim(const hsVector3& vel);
	virtual void ClearLinearVelocity();

	virtual hsBool GetAngularVelocitySim(hsVector3& vel) const;
	virtual void SetAngularVelocitySim(const hsVector3& vel);

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, hsBool force=false);
	virtual void GetTransform(hsMatrix44& l2w, hsMatrix44& w2l);

	virtual int GetGroup() const { return fGroup; }

	virtual void	AddLOSDB(UInt16 flag) { hsSetBits(fLOSDBs, flag); }
	virtual void	RemoveLOSDB(UInt16 flag) { hsClearBits(fLOSDBs, flag); }
	virtual UInt16	GetAllLOSDBs() { return fLOSDBs; }
	virtual hsBool	IsInLOSDB(UInt16 flag) { return hsCheckBits(fLOSDBs, flag); }

	virtual hsBool    DoDetectorHullWorkaround() { return fSaveTriangles ? true : false;	}
	virtual hsBool	Should_I_Trigger(hsBool enter, hsPoint3& pos);
	virtual hsBool	IsObjectInsideHull(const hsPoint3& pos);
	virtual void	SetInsideConvexHull(hsBool inside) { fInsideConvexHull = inside;	}

	virtual plKey GetWorldKey() const { return fWorldKey; }

	virtual plPhysicalSndGroup* GetSoundGroup() const { return fSndGroup; }

	virtual void GetPositionSim(hsPoint3& pos) const { IGetPositionSim(pos); }

	virtual void SendNewLocation(hsBool synchTransform = false, hsBool isSynchUpdate = false);

	virtual void SetHitForce(const hsVector3& force, const hsPoint3& pos) { fWeWereHit=true; fHitForce = force; fHitPos = pos; }
	virtual void ApplyHitForce();
	virtual void ResetHitForce() { fWeWereHit=false; fHitForce.Set(0,0,0); fHitPos.Set(0,0,0); }

	virtual void GetSyncState(hsPoint3& pos, hsQuat& rot, hsVector3& linV, hsVector3& angV);
	virtual void SetSyncState(hsPoint3* pos, hsQuat* rot, hsVector3* linV, hsVector3* angV);

	virtual void ExcludeRegionHack(hsBool cleared);

	virtual plDrawableSpans* CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo);

	hsBool DoReportOn(plSimDefs::Group group) const { return hsCheckBits(fReportsOn, 1<<group); }

	// Returns true if this object is *really* dynamic.  We can have physicals
	// that are in the dynamic group but are actually animated or something.
	// This weeds those out.
	hsBool IsDynamic() const;
	
	//Hack to check if there is an overlap with the capsule
	//this partially for exclude regions vs avatar capsule
	virtual hsBool OverlapWithCapsule(NxCapsule& cap);

	virtual hsScalar GetMass() {return fMass;}
protected:
	void IGetPositionSim(hsPoint3& pos) const;
	void IGetRotationSim(hsQuat& rot) const;
	void ISetPositionSim(const hsPoint3& pos);
	void ISetRotationSim(const hsQuat& rot);

	/** Handle messages about our references. */
	hsBool HandleRefMsg(plGenRefMsg * refM);

	/////////////////////////////////////////////////////////////
	//
	// WORLDS, SUBWORLDS && CONTEXTS
	//
	/////////////////////////////////////////////////////////////

	void IConvertGroups(UInt32 memberOf, UInt32 reportsOn, UInt32 collideWith);

	/** See if the object is in a valid, non-overlapping position.
		A valid overlap is one which is approved by the collision
		masking code, i.e. my memberOf has no intersection with your
		bounceOff and vice-versa
		*/
	// Set overlapText to get a string naming all the overlapping physicals (that you must delete)
	hsBool CheckValidPosition(char** overlapText=nil);

	/////////////////////////////////////////////////////////////
	//
	// NETWORK SYNCHRONIZATION
	//
	/////////////////////////////////////////////////////////////

	/** Remember that we need to do a synch soon. */
	hsBool DirtySynchState(const char* SDLStateName, UInt32 synchFlags );

	double GetLastSyncTime() { return fLastSyncTime; }

	/** Get the simulation transform of the physical, in world
	coordinates (factoring in the subworld if necessary */
	void IGetTransformGlobal(hsMatrix44 &l2w) const;
	void ISetTransformGlobal(const hsMatrix44& l2w);

	// Enable/disable collisions and dynamic movement
	void IEnable(hsBool enable);

	void IMakeHull(NxConvexMesh* convexMesh, hsMatrix44 l2w);

	NxActor* fActor;
	plKey fWorldKey;	// either a subworld or nil

	plSimDefs::Bounds fBoundsType;
	plSimDefs::Group fGroup;
	UInt32 fReportsOn;			// bit vector for groups we report interactions with
	UInt16 fLOSDBs;				// Which LOS databases we get put into
	hsBitVector fProps;			// plSimulationInterface::plSimulationProperties kept here
	float	fMass;

	plKey fObjectKey;			// the key to our scene object
	plKey fSceneNode;			// the room we're in

	// PHYSX FIXME - need to create a plasma hull so that we can determine if inside
	hsPlane3* fWorldHull;
	UInt32	  fHullNumberPlanes;
	hsPoint3* fSaveTriangles;
	hsBool		fInsideConvexHull;
	void ISetHullToWorldWTriangles();
	inline hsBool ITestPlane(const hsPoint3 &pos, const hsPlane3 &plane)
	{
		hsScalar dis = plane.fN.InnerProduct(pos);
		dis += plane.fD;
		if (dis == 0.f)
			return false;
		if( dis >= 0.f )	
			return false;	

		return true;
	}

	// we need to remember the last matrices we sent to the coordinate interface
	// so that we can recognize them when we send them back and not reapply them,
	// which would reactivate our body. inelegant but effective
	hsMatrix44 fCachedLocal2World;

	// Syncronization
	double			fLastSyncTime;
	plSDLModifier*	fSDLMod;

	plPhysicalSndGroup*	fSndGroup;

	hsBool		fWeWereHit;
	hsVector3	fHitForce;
	hsPoint3	fHitPos;

	plPhysicalProxy* fProxyGen;				// visual proxy for debugging

	static int	fNumberAnimatedPhysicals;
	static int	fNumberAnimatedActivators;
};

#endif
