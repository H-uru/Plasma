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
#ifndef hsGCompMatDefined
#define hsGCompMatDefined

#include "hsTemplates.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "hsGMatState.h"
#include "hsColorRGBA.h"

class hsScene;
class hsResMgr;
class hsG3DDevice;
class plLayerInterface;
class plLayer;

// inlines for Texture and Material after class declarations

class hsGMaterial : public plSynchedObject
{
public:
	// Things we have to know that some layer has
	enum hsGCompFlags {
		kCompShaded							= 0x1,
		kCompEnvironMap						= 0x2,
		kCompProjectOnto					= 0x4,
		kCompSoftShadow						= 0x8,
		kCompSpecular						= 0x10,
		kCompTwoSided						= 0x20,
		kCompDrawAsSplats					= 0x40,
		kCompAdjusted						= 0x80,
		kCompNoSoftShadow					= 0x100,
		kCompDynamic						= 0x200,
		kCompDecal							= 0x400,
		kCompIsEmissive_OBSOLETE			= 0x800,
		kCompIsLightMapped					= 0x1000,
		kCompNeedsBlendChannel				= 0x2000	// For materials that have extra layers to simulate vtx alpha
	};
	enum UpdateFlags
	{
		kUpdateAgain		= 0x01
	};

protected:
	UInt32					fLOD;
	hsTArray<plLayerInterface*>		fLayers;
	hsTArray<plLayerInterface*>		fPiggyBacks;

	UInt32					fCompFlags;
	UInt32					fLoadFlags;

	hsScalar				fLastUpdateTime;

	void				IClearLayers();
	UInt32				IMakeExtraLayer();

	void					InsertLayer(plLayerInterface* lay, Int32 which = 0, hsBool piggyBack = false);
	void					SetLayer(plLayerInterface* lay, Int32 which = 0, hsBool insert=false, hsBool piggyBack=false);
	void					ReplaceLayer(plLayerInterface* oldLay, plLayerInterface* newLay, hsBool piggyBack = false);
	void					RemoveLayer(plLayerInterface* oldLay, hsBool piggyBack = false);
public:
	hsGMaterial();
	~hsGMaterial();

	virtual hsGMaterial*	Clone();
	virtual hsGMaterial*	CloneNoLayers(); // For things like blending copies, that manipulate layers directly.
											 // copies no keyed objects.
	plLayer*				MakeBaseLayer();
	plLayerInterface*		GetLayer(UInt32 which);
	plLayerInterface*		GetPiggyBack(UInt32 which);
	UInt32					AddLayerViaNotify(plLayerInterface* lay);
	UInt32					GetNumLayers() const		{ return fLayers.GetCount(); }
	void					SetNumLayers(int cnt);
	UInt32					GetNumPiggyBacks() const	{ return fPiggyBacks.GetCount(); }
	void					SetNumPiggyBacks();

	void					SetLOD(UInt32 l)			{ fLOD = l; }
	UInt32					GetLOD() const				{ return fLOD; }

	void					SetCompositeFlags(UInt32 f) { fCompFlags = f; } // normally composite flags are calculated internally, not set.
	UInt32					GetCompositeFlags()	const	{ return fCompFlags; }
	UInt32					GetLoadFlags() const		{ return fLoadFlags; }

	hsScalar				GetLastUpdateTime()	const	{ return fLastUpdateTime; }
	void					SetLastUpdateTime(hsScalar f) { fLastUpdateTime = f; }
	hsBool					IShouldUpdate(hsScalar secs, UInt32 flags) { return GetLastUpdateTime() != secs || (flags & kUpdateAgain); }

	hsBool					IsDynamic() const			{ return (fCompFlags & kCompDynamic); }
	hsBool					IsDecal() const				{ return (fCompFlags & kCompDecal); }
	hsBool					NeedsBlendChannel()			{ return (fCompFlags & kCompNeedsBlendChannel); }

	virtual void		Read(hsStream* s);
	virtual void		Write(hsStream* s);
	virtual void		Read(hsStream* s, hsResMgr *group);
	virtual void		Write(hsStream* s, hsResMgr *group);

	virtual void Eval(double secs, UInt32 frame);
	virtual void Reset();
	virtual void Init();

	CLASSNAME_REGISTER( hsGMaterial );
	GETINTERFACE_ANY( hsGMaterial, hsKeyedObject );
	
	virtual hsBool MsgReceive(plMessage* msg);
};

#endif // hsGCompMatDefined
