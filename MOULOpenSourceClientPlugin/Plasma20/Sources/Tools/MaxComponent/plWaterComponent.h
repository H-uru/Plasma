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


#ifndef plWaterComponent_inc
#define plWaterComponent_inc

#include "plQuality.h"

const Class_ID WATER_COMP_CID(0x1d7c20cc, 0x68106c16);
const Class_ID SHORE_COMP_CID(0x74613f05, 0x326d74ac);
const Class_ID WDECAL_COMP_CID(0x1fe01c50, 0x3d16276f);
const Class_ID ENVMAP_COMP_CID(0x59b7e1c, 0x22780bb7);

class plWaveSetBase;
class plFixedWaterState7;
class plRenderTarget;
class plDynamicEnvMap;
class plDynamicCamMap;

class plWaveSet7;

class plWaterComponent : public plComponent
{
public:
	enum {
		kRefObject,

		kWindSpeed,
		kWaterTint,		// Color
		kWaterOpac,	// Scalar
		kSpecularTint,	// Color
		kRippleScale, // determines ripple scale
		
		kDepthOpac,	// Depth at which overall opacity reaches 100% in feet
		kDepthRefl,	// Depth at which reflection strength reaches 100% in feet
		kDepthWave,	// Depth at which geometric wave height reaches 100% in feet
		kZeroOpac, // Offset, positive pulls full-on in
		kZeroRefl, // Offset, positive pulls full-on in
		kZeroWave, // Offset, positive pulls full-on in

		kShoreTint,		// Color => MinColor.rgb
		kShoreOpac,
		kWispiness,

		kPeriod, // ???
		kFinger,	// Finger length [0..100]%

		kDispersion,		// How rough (non-viscous) the surface ripples look.

		kEnvObject,
		kEnvSize,
		kEnvRadius,

		kEdgeOpac,
		kEdgeRadius,
		
		kEnvRefresh,

		kGeoAngleDev,
		kTexAngleDev,
		
		kGeoMinLen,
		kGeoMaxLen,
		kGeoAmpOverLen,

		kTexMinLen,
		kTexMaxLen,
		kTexAmpOverLen,

		kNoise,

		kSpecStart,
		kSpecEnd,

		kGeoChop,
		kTexChop,

		kSpecularMute,

		kNumParams
	};
	enum {
		kRef,
//		kBasicWater,
		kAdvWater,
		kBasicShore,
		kAdvShore,
		kEnvMap,
		kVtxHelp,
		kGeoWater,
		kTexWater,
		kNumRollups
	};
protected:
	plWaveSet7*		fWaveSet;

	hsBool			IGetRefObject(plMaxNode* node);
	hsBool			IReadEnvObject(plMaxNode* node, plErrorMsg* pErrMsg, plFixedWaterState7& ws);
	hsBool			IReadRefObject(plMaxNodeBase* node, plFixedWaterState7& ws);
	hsBool			IMakeWaveSet(plMaxNode* node, plErrorMsg* pErrMsg);

	plWaveSetBase*	IGetWaveSet() const { return (plWaveSetBase*)fWaveSet; } // fWaveSet set in SetupProperties pass.

	hsScalar		IGetWaterHeight();

public:
	plWaterComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool DeInit(plMaxNode* node, plErrorMsg* pErrMsg);

	virtual int GetMinCap() { return plQuality::kPS_1_1; }

	// This works anytime.
	static hsScalar GetWaterHeight(INode* node); // node is component node.

	// These only work after PreConvert pass
	static plWaveSetBase* GetWaveSet(INode* node); // Node is the component node
	static plWaveSetBase* GetWaveSetFromNode(plMaxNode* node); // node is the component's target

	// These just deal with old data with obsolete parameters.
	void		CheckForObsoleteParams();
	IOResult	Load(ILoad* iLoad);
};

class plShoreComponent : public plComponent
{
public:
	enum {
		kWaveSet
	};
protected:
public:
	plShoreComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);

	virtual int GetMinCap() { return plQuality::kPS_1_1; }
};

class plWDecalComponent : public plComponent
{
public:
	enum {
		kWaveSet,
		kEnv
	};
protected:
public:
	plWDecalComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);

	virtual int GetMinCap() { return plQuality::kPS_1_1; }
};

class plEnvMapComponent : public plComponent
{
public:
	enum {
		kVisSets,
		kHither,
		kYon,
		kFogEnable,
		kFogStart,
		kFogColor,
		kRefreshRate,
		kEnvSize,
		kIncChars,
		kMapType,
		kVisSetNames,
	};

	// Map types
	enum
	{
		kMapCubic,
		kMapSingle,
	};

protected:
	plRenderTarget* fMap; // Will be a plDynamicEnvMap or plDynamicCamMap

	plRenderTarget* IGetMap();

public:
	plEnvMapComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	virtual hsBool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg);
	virtual hsBool Convert(plMaxNode* node, plErrorMsg* pErrMsg);

	plDynamicEnvMap* GetEnvMap();
	plDynamicCamMap* GetCamMap();
	static plDynamicEnvMap* GetEnvMap(plMaxNode* node);
	static plDynamicCamMap* GetCamMap(plMaxNode* node);
	static plEnvMapComponent* GetEnvMapComponent(plMaxNode* node);
};

#endif // plBlowComponent_inc
