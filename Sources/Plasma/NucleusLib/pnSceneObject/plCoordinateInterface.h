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


#ifndef plCoordinateInterface_inc
#define plCoordinateInterface_inc

#include "plObjInterface.h"
#include "hsTemplates.h"
#include "hsMatrix44.h"
#include "../pnNetCommon/plSynchedValue.h"

class hsStream;
class hsResMgr;


class plCoordinateInterface : public plObjInterface
{
public:
	enum plCoordinateProperties {
		kDisable				= 0, // prop 0 is always disable, declared in plObjInterface
		kCanEverDelayTransform	= 1, // we can sometimes delay our transform eval (i.e. we're on a physics object)
		kDelayedTransformEval	= 2, // we're currently registering for the DelayedTransformMsg (we, and all our
									 // descendants, have the kCanEverDelayTransform prop)

		kNumProps					 // last in the list
	};

	enum plCoordinateTransformPhases
	{
		kTransformPhaseNormal,
		kTransformPhaseDelayed,
	};
	
protected:
	enum {
		kTransformDirty		= 0x1,
		kWarp				= 0x2,

		kMaxState			= 0xffff
	};
	enum plAttachFlags {
		kMaintainWorldPosition			= 0x1,
		kMaintainSceneNode				= 0x2,
		kAboutToAttach					= 0x4
	};
	enum Reason {
		kReasonUnknown	= 0x1,			// somebody moved us
		kReasonPhysics	= 0x2,			// physics moved us
		kMaxReasons	= 0xffff			// sixteen bits
	};

	// Set by the client in IUpdate(). This tells us where we are in the update loop so that we know
	// which transform message to register for when our transform is dirtied.
	static UInt8							fTransformPhase;
	
	// Temp debugging tool, so we can quickly (dis/en)able delayed transforms at runtime.
	static hsBool							fDelayedTransformsEnabled;

	UInt16									fState;
	UInt16									fReason;		// why we've changed position (if we have)

	hsTArray<plSceneObject*>				fChildren;
	plCoordinateInterface*					fParent;	// if this changes, marks us as dirty

	hsMatrix44								fLocalToParent;
	hsMatrix44								fParentToLocal;

	hsMatrix44								fLocalToWorld;
	hsMatrix44								fWorldToLocal;

	virtual void ISetOwner(plSceneObject* so);

	virtual void ISetParent(plCoordinateInterface* par); // don't use, use AddChild on parent
	virtual void ISetSceneNode(plKey newNode);
	// objectToo moves the sceneObject to the new room, else just move the data and remove
	// the object from whatever room he's in.

	// Network only strange functions. Do not emulate or generalize this functionality.
	void					ISetNetGroupRecur(plNetGroupId netGroup);

	virtual void ISetChild(plSceneObject* child, int which); // sets parent on child
	virtual void IAddChild(plSceneObject* child); // sets parent on child
	virtual void IRemoveChild(plSceneObject* child); // removes this as parent of child
	virtual void IRemoveChild(int i); // removes this as parent of child
	virtual void IAttachChild(plSceneObject* child, UInt8 flags); // physically attaches child to us
	virtual void IDetachChild(plSceneObject* child, UInt8 flags); // physically detach this child from us
	virtual void IUpdateDelayProp(); // Called whenever a child is added/removed

	virtual void IRecalcTransforms(); // Called by ITransformChanged when we need to re-examine our relationship with our parent.
	virtual void ITransformChanged(hsBool force, UInt16 reasons, hsBool checkForDelay); // called by SceneObject on TransformChanged messsage

	void					IDirtyTransform();
	void					IRegisterForTransformMessage(hsBool delayed);
	void					IUnRegisterForTransformMessage();
	plCoordinateInterface*	IGetRoot();

	friend class plSceneObject;

public:
	plCoordinateInterface();
	~plCoordinateInterface();

	CLASSNAME_REGISTER( plCoordinateInterface );
	GETINTERFACE_ANY( plCoordinateInterface, plObjInterface );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
	virtual void SetLocalToParent(const hsMatrix44& l2p, const hsMatrix44& p2l);
	// special version for setting transform from physics.
	// separate to keep from changing interface to add "reason" parameter
	virtual void SetTransformPhysical(const hsMatrix44& l2w, const hsMatrix44& w2l);

	virtual void MultTransformLocal(const hsMatrix44& move, const hsMatrix44& invMove);
	virtual void WarpToWorld(const hsMatrix44& l2w, const hsMatrix44& w2l);
	virtual void WarpToLocal(const hsMatrix44& l2p, const hsMatrix44& p2l);

	// Force an immediate re-sync of the transforms in the hierarchy this object belongs to,
	// as opposed to waiting for the plTransformMsg to resync.
	// There are two uses for this:
	//		a) You need the transforms for this object to be valid NOW, can't wait till after a TransforMsg
	//			In this case, you want to flush from the root, because something higher up the hierarchy
	//			may be dirty, which invalidates this object's transforms
	//		b) You've just dirtied this object's transform, and need it to propagate downward from here NOW
	//			In this case, fromRoot should be false, because you haven't dirtied anything higher up
	//			the hierarchy.
	// Another way to look at it is, if the transforms for the tree were correct before you messed with
	// this object, you only need to flush the transforms for this object and its recursive children,
	// so fromRoot=false.
	// If the entire tree is potentially dirty and you need to read from it, you want the entire tree
	// synced up, so fromRoot=true.
	// fromRoot=true is always safe, just potentially wasteful, so if you don't know, use fromRoot=true or
	// preferably, don't use this function.
	void FlushTransform(hsBool fromRoot=true); 

	virtual const hsMatrix44& GetLocalToParent() const { return fLocalToParent; }
	virtual const hsMatrix44& GetParentToLocal() const { return fParentToLocal; }
	
	virtual const hsMatrix44& GetLocalToWorld() const { return fLocalToWorld; }
	virtual const hsMatrix44& GetWorldToLocal() const { return fWorldToLocal; }

	virtual const hsPoint3 GetWorldPos() const { return fLocalToWorld.GetTranslate(); }

	virtual int GetNumChildren() const { return fChildren.GetCount(); }
	virtual plCoordinateInterface* GetChild(int i) const;
	virtual plCoordinateInterface* GetParent() const { return fParent; }

	virtual hsBool MsgReceive(plMessage* msg);

	UInt16 GetReasons();
	void ClearReasons();

	Int32   GetNumProperties() const { return kNumProps; }
	static UInt8	GetTransformPhase() { return fTransformPhase; }
	static void		SetTransformPhase(UInt8 phase) { fTransformPhase = phase; }

	static hsBool	GetDelayedTransformsEnabled() { return fDelayedTransformsEnabled; }
	static void		SetDelayedTransformsEnabled(hsBool val) { fDelayedTransformsEnabled = val; }
};


#endif // plCoordinateInterface_inc
