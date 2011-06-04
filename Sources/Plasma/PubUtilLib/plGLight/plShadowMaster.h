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

#ifndef plShadowMaster_inc
#define plShadowMaster_inc

#include "../pnSceneObject/plObjInterface.h"

class plShadowCaster;
class plShadowSlave;

struct hsMatrix44;
class hsBounds3Ext;
class hsStream;
class hsResMgr;
class plMessage;
class plLightInfo;
class plShadowCastMsg;


class plShadowMaster : public plObjInterface
{
public:
	// Props inc by 1 (bit shift in bitvector).
	enum plDrawProperties {
		kDisable				= 0,
		kSelfShadow,

		kNumProps				// last in the list
	};


protected:
	// Global clamp on shadow map size and stuff
	static UInt32					fGlobalMaxSize;
	static hsScalar					fGlobalMaxDist;
	static hsScalar					fGlobalVisParm;

	// Constant parameter(s) for this master.
	hsScalar						fAttenDist;
	hsScalar						fMaxDist;
	hsScalar						fMinDist;

	UInt32							fMaxSize;
	UInt32							fMinSize;

	hsScalar						fPower;

	// Temp data used for one frame and recycled.
	hsTArray<plShadowSlave*>		fSlavePool;
	plLightInfo*					fLightInfo;

	// These are specific to the projection type (perspective or orthogonal), so have to
	// be implemented by the derived class.
	virtual void IComputeWorldToLight(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
	virtual void IComputeProjections(plShadowCastMsg* castMsg, plShadowSlave* slave) const = 0;
	virtual void IComputeISect(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
	virtual void IComputeBounds(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;

	// Base class implementations of the rest. These might need to be overridden later, especially
	// on computing the width and height. That's really specific to the projection type, but
	// to get started I'll probably just always return 256x256.
	virtual void IComputeCasterBounds(const plShadowCaster* caster, hsBounds3Ext& casterBnd);
	virtual void IComputeWidthAndHeight(plShadowCastMsg* castMsg, plShadowSlave* slave) const;
	virtual void IComputeLUT(plShadowCastMsg* castMsg, plShadowSlave* slave) const;
	virtual hsScalar IComputePower(const plShadowCaster* caster, const hsBounds3Ext& casterBnd) const;

	virtual plShadowSlave* ILastChanceToBail(plShadowCastMsg* castMsg, plShadowSlave* slave);

	virtual plShadowSlave* ICreateShadowSlave(plShadowCastMsg* castMsg, const hsBounds3Ext& casterBnd, hsScalar power);

	virtual plShadowSlave* INewSlave(const plShadowCaster* caster) = 0;
	virtual plShadowSlave* INextSlave(const plShadowCaster* caster);

	virtual plShadowSlave* IRecycleSlave(plShadowSlave* slave);
	plLightInfo* ISetLightInfo();

	virtual void IBeginRender();
	virtual hsBool IOnCastMsg(plShadowCastMsg* castMsg);

public:
	plShadowMaster();
	virtual ~plShadowMaster();

	CLASSNAME_REGISTER( plShadowMaster );
	GETINTERFACE_ANY( plShadowMaster, plObjInterface );

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) {}

	Int32		GetNumProperties() const { return kNumProps; }

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	// These are usually handled internally, activating on read and deactivating
	// on destruct. Made public in case they need to be manually handled, like
	// on dynamic construction and use.
	void Deactivate() const;
	void Activate() const;

	// These should only be useful on scene conversion.
	hsScalar GetAttenDist() const { return fAttenDist; }
	void SetAttenDist(hsScalar d) { fAttenDist = d; }

	hsScalar GetMaxDist() const { return fMaxDist; }
	hsScalar GetMinDist() const { return fMinDist; }
	void SetMaxDist(hsScalar m);

	UInt32 GetMaxSize() const { return fMaxSize; }
	UInt32 GetMinSize() const { return fMinSize; }
	void SetMaxSize(UInt32 s) { fMaxSize = s; }
	void SetMinSize(UInt32 s) { fMinSize = s; }

	hsScalar GetPower() const { return fPower; }
	void SetPower(hsScalar f) { fPower = f; }

	static void SetGlobalMaxSize(UInt32 s) ;
	static UInt32 GetGlobalMaxSize() { return fGlobalMaxSize; }

	static void SetGlobalMaxDist(hsScalar s) { fGlobalMaxDist = s; }
	static hsScalar GetGlobalMaxDist() { return fGlobalMaxDist; }

	static void SetGlobalShadowQuality(hsScalar s);
	static hsScalar GetGlobalShadowQuality() { return fGlobalVisParm; }
};



#endif // plShadowMaster_inc
