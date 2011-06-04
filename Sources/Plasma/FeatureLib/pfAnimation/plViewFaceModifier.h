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
#ifndef plViewFaceModifier_inc
#define plViewFaceModifier_inc

#include "hsMatrix44.h"
#include "hsBounds.h"
#include "../pnModifier/plSingleModifier.h"

class plGenRefMsg;
class plPipeline;

class plViewFaceModifier : public plSingleModifier
{
public:
	enum plVFFlags {
		kPivotFace			= 0,
		kPivotFavorY,
		kPivotY,
		kPivotTumble,
		kScale,
		kFaceCam,
		kFaceList,
		kFacePlay,
		kFaceObj,
		kOffset,
		kOffsetLocal,
		kMaxBounds
	};
protected:

	hsVector3				fLastDirY;

	hsVector3				fScale;

	hsMatrix44				fOrigLocalToParent;
	hsMatrix44				fOrigParentToLocal;

	hsPoint3				fFacePoint;

	plSceneObject*			fFaceObj;

	hsVector3				fOffset;

	hsBounds3Ext			fMaxBounds;

	virtual hsBool IFacePoint(plPipeline* pipe, const hsPoint3& at);
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);

	enum RefType
	{
		kRefFaceObj
	};
	void			IOnReceive(plGenRefMsg* refMsg);
	void			IOnRemove(plGenRefMsg* refMsg);
	void			ISetObject(plKey soKey);

public:
	plViewFaceModifier();
	virtual ~plViewFaceModifier();

	CLASSNAME_REGISTER( plViewFaceModifier );
	GETINTERFACE_ANY( plViewFaceModifier, plSingleModifier );
	
	virtual void SetTarget(plSceneObject* so);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	// ViewFace specific
	void SetScale(const hsVector3& s) { fScale = s; }
	const hsVector3& GetScale() const { return fScale; }

	void SetOrigTransform(const hsMatrix44& l2p, const hsMatrix44& p2l);

	void SetMaxBounds(const hsBounds3Ext& bnd);
	const hsBounds3Ext& GetMaxBounds() const { return fMaxBounds; }
	hsBool HaveMaxBounds() const { return HasFlag(kMaxBounds); }

	enum FollowMode
	{
		kFollowCamera			= 0, // Follow the camera
		kFollowListener,
		kFollowPlayer,
		kFollowObject
	};
	void			SetFollowMode(FollowMode m, plKey soKey=nil); // For follow object, set obj, else it's ignored.
	FollowMode		GetFollowMode() const;
	plSceneObject*	GetFollowObject() const { return fFaceObj; }

	void				SetOffsetActive(hsBool on) { if(on) SetFlag(kOffset); else ClearFlag(kOffset); }
	hsBool				GetOffsetActive() const { return HasFlag(kOffset); }

	void				SetOffset(const hsVector3& off, hsBool local=true);
	const hsVector3&	GetOffset() const { return fOffset; }
	hsBool				GetOffsetLocal() const { return HasFlag(kOffsetLocal); }
};

#endif // plViewFaceModifier_inc
