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
#ifndef hsGEnviron_inc
#define hsGEnviron_inc

#include "hsGeometry3.h" // hsPoint3
#include "../plInterp/hsTimedValue.h"
#include "../plResMgr/hsKeyedObject.h"
#include "hsColorRGBA.h"
#include "hsTemplates.h"

//
//-----------------------------------
// Environment
//-----------------------------------
//

class hsGMaterial;
class hsGDevEnvCache;
class hsFogControl;
class hsSceneNode;
class hsScene;
class hsGRenderProcs;
class hsG3DDevice;
class plKey;
class hsResMgr;

class hsGEnvironment : public hsKeyedObject {
private:
    static const UInt16 kSaveMagicNumber;
    static const UInt16 kSaveVersion;
    
public:
    struct FogState {
    private:
        static const UInt16 kSaveMagicNumber;
        static const UInt16 kSaveVersion;
        
    public:
        FogState() : fFlags(0) {};
		enum {
			kColorSet		= 0x1,
			kDistanceSet	= 0x2,
			kDepthSet		= 0x4,
			kDensitySet		= 0x8,
			kYonSet			= 0x10,
			kClearSet		= 0x20,

			kLinear			= 0x10000000,
			kExp			= 0x20000000,
			kExp2			= 0x40000000,
			kTypeMask		= kLinear | kExp | kExp2
		};
		UInt32							fFlags;
		hsTimedValue<hsColorRGBA>		fColor;
		hsTimedValue<hsColorRGBA>		fClear;
		hsTimedValue<hsScalar>			fDistance;
		hsTimedValue<hsScalar>			fDensity;
		hsTimedValue<hsScalar>			fDepth;
		hsTimedValue<hsScalar>			fYon;

		void ValidateEnv(class hsGEnvironment* env);
		void SetFromEnv(class hsGEnvironment* env);
		void SetToEnv(class hsGEnvironment* env);
        void Save(hsStream *stream, hsResMgr* mgr);
        void Load(hsStream *stream, hsResMgr* mgr);
	};

	enum {
		kMapSet				= 0x1,
		kCenterSet			= 0x2,
		kRadiusSet			= 0x4,
		kEnvironMapSet		= kMapSet | kCenterSet | kRadiusSet,
		kFogDepthSet		= 0x8,
		kFogColorSet		= 0x10,
		kFogDensitySet		= 0x20,
		kYonSet				= 0x40,
		kOverride			= 0x80,
		kFarOut				= 0x100,
		kFogDistanceSet		= 0x200,
		kCacheInvalid		= 0x400,
		kClearColorSet		= 0x800,
		kCurrentDepthSet	= 0x1000,
		kFogControl			= 0x2000,
		kSortObjects		= 0x4000,
		kHasRenderProcs		= 0x8000,
		kFogLinear			= 0x10000,
		kFogExp				= 0x20000,
		kFogExp2			= 0x40000,
		kFogTypeMask		= kFogLinear | kFogExp | kFogExp2,
		kClearColorAmbient	= 0x80000,
		kFogColorAmbient	= 0x100000,
		kFogColorDiffuse	= 0x200000
	};
protected:
	static hsScalar		fYonScale;

	UInt32				fFlags;
	hsGMaterial*		fMap;
	char				fMapName[256];
	hsPoint3			fPos;
	hsScalar			fRadius;
	hsScalar			fValidScale;
	hsTArray<FogState*>				fFogStateStack;
	hsTimedValue<hsScalar>			fFogDistance;
	hsTimedValue<hsScalar>			fFogDepth; // value 0..1, as fraction of yon, 0 is no fog
	hsTimedValue<hsScalar>			fFogDensity;
	hsTimedValue<hsColorRGBA>		fFogColor;
	hsTimedValue<hsColorRGBA>		fClearColor;
	hsTimedValue<hsScalar>			fYon;
	hsScalar						fCurrentDepth; // function of Depth and Distance

	FogState						fResetState;
	hsGDevEnvCache*					fDevCache;

	hsDynamicArray<hsGRenderProcs*>		fRenderProcs;
	hsTArray<plKey*>          fNodeKeys;

	hsFogControl*					fFogControl;

	void				IReadFogControl(hsStream* s, hsResMgr* mgr);
	void				IWriteFogControl(hsStream* s, hsResMgr* mgr);

public:
	hsGEnvironment();
	virtual ~hsGEnvironment();

	hsBool32 AddNode(hsSceneNode *node);
    hsBool32 AddNodeKey(plKey *key);
    Int32 GetNumNodes() { return fNodeKeys.GetCount(); }
    hsSceneNode* GetNode(Int32 i);
    plKey* GetNodeKey(Int32 i) { return fNodeKeys[i]; }

    char*				GetMapName() { return fMapName; }
	hsGMaterial*		GetMap() const { return fMap; }
	hsPoint3			GetCenter() const { return fPos; }
	hsScalar			GetRadius() const { return fRadius; }
	hsScalar			GetFogDistance() const { return fFogDistance.GetValue(); }
	hsScalar			GetFogDepth() const { return fFogDepth.GetValue(); }
	hsScalar			GetCurrentDepth() const { return fCurrentDepth; }
	hsScalar			GetFogDensity() const { return fFogDensity.GetValue(); }
	hsColorRGBA			GetFogColor() const { return fFogColor.GetValue(); }
	hsColorRGBA			GetClearColor() const { return fClearColor.GetValue(); }
	hsScalar			GetYon() const { return fYonScale * fYon.GetValue(); }
	hsScalar			GetUnscaledYon() const { return fYon.GetValue(); }
	hsBool32			GetOverride() const { return 0 != (fFlags & kOverride); }
	UInt32				GetFlags() const { return fFlags; }

	hsScalar			GoalFogDistance() const { return fFogDistance.GetGoal(); }
	hsScalar			GoalFogDepth() const { return fFogDepth.GetGoal(); }
	hsScalar			GoalFogDensity() const { return fFogDensity.GetGoal(); }
	hsColorRGBA			GoalFogColor() const { return fFogColor.GetGoal(); }
	hsColorRGBA			GoalClearColor() const { return fClearColor.GetGoal(); }
	hsScalar			GoalYon() const { return fYon.GetGoal(); }

	const hsTimedValue<hsScalar>&		FogDistanceState() const { return fFogDistance; }
	const hsTimedValue<hsScalar>&		FogDepthState() const { return fFogDepth; }
	const hsTimedValue<hsScalar>&		FogDensityState() const { return fFogDensity; }
	const hsTimedValue<hsColorRGBA>&	FogColorState() const { return fFogColor; }
	const hsTimedValue<hsColorRGBA>&	ClearColorState() const { return fClearColor; }
	const hsTimedValue<hsScalar>&		YonState() const { return fYon; }

	void SetFogDistance(const hsTimedValue<hsScalar>& v);
	void SetFogDepth(const hsTimedValue<hsScalar>& v);
	void SetFogDensity(const hsTimedValue<hsScalar>& v);
	void SetFogColor(const hsTimedValue<hsColorRGBA>& v);
	void SetClearColor(const hsTimedValue<hsColorRGBA>& v);
	void SetYon(const hsTimedValue<hsScalar>& v);

	void SetMapName(const char *name);
	void SetMap(hsGMaterial *m); // refs
	void SetCenter(const hsPoint3 &p);
	void SetRadius(hsScalar r);
	void SetFogDistance(hsScalar d);
	void SetFogDepth(hsScalar f);
	void SetCurrentDepth(hsScalar f);
	void SetFogDensity(hsScalar f);
	void SetFogColor(const hsColorRGBA &c);
	void SetClearColor(const hsColorRGBA &c);
	void SetYon(hsScalar f);
	void SetOverride(hsBool32 on);
	void SetIsFar(hsBool32 on=true);
	void SetHasFogControl(hsBool32 on=true);
	void SetSortObjects(hsBool32 on=true);
	void SetFogType(UInt32 t);
	void SetFogColorAmbient(hsBool32 on=true);
	void SetFogColorDiffuse(hsBool32 on=true);
	void SetClearColorAmbient(hsBool32 on=true);

	void SetTimedFogDistance(const hsScalar g, const hsScalar s);
	void SetTimedFogDepth(const hsScalar g, const hsScalar s);
	void SetTimedFogDensity(const hsScalar g, const hsScalar s);
	void SetTimedFogColor(const hsColorRGBA& g, const hsScalar s);
	void SetTimedClearColor(const hsColorRGBA& g, const hsScalar s);
	void SetTimedYon(const hsScalar g, const hsScalar s);

	void UnSetMapName() { *fMapName = 0; }
	void UnSetEnvironMap() { fFlags &= ~kEnvironMapSet; }
	void UnSetFogDistance() { fFlags &= ~kFogDistanceSet; }
	void UnSetFogDepth() { fFlags &= ~kFogDepthSet; }
	void UnSetFogDensity() { fFlags &= ~kFogDensitySet; }
	void UnSetFogColor() { fFlags &= ~kFogColorSet; }
	void UnSetClearColor() { fFlags &= ~kClearColorSet; }
	void UnSetYon() { fFlags &= ~kYonSet; }

	hsGEnvironment*		Copy(hsGEnvironment* env); // returns this

	void				MixEnvirons(hsGEnvironment* env, hsGEnvironment* def);

	void				Push(hsG3DDevice* d);
	void				Pop(hsG3DDevice* d);

	void				Blend();
	void				Restore();
	void				Init(hsSceneNode* node);

	void				SetDeviceCache(hsGDevEnvCache* p);
	hsGDevEnvCache*		GetDeviceCache(){ return fDevCache; }

	void				SetFogControl(hsFogControl* fc);
	hsFogControl*		GetFogControl() { return fFogControl; }

	void				SaveFogState();			// push
	hsGEnvironment::FogState* PopFogState() { return fFogStateStack.GetCount() ? fFogStateStack.Pop() : nil; }	// doesn't restore
	void				RestoreFogState();		// pop and restore

	void				AddRenderProc(hsGRenderProcs* rp); // refs
	hsGRenderProcs*		GetRenderProc(int i); // no ref
	UInt32				GetNumRenderProcs();

	virtual void		SetResetState();
	virtual void		Reset();
	virtual void		ValidateInResetState();
    virtual void        Save(hsStream *stream, hsResMgr* mgr);
    virtual void        Load(hsStream *stream, hsResMgr* mgr);
	virtual void		Update(hsScalar secs, const hsPoint3& vPos);

	virtual void		Read(hsStream* s);
	virtual void		Write(hsStream* s);

    virtual void        Write(hsStream *stream, hsResMgr *group);
    virtual void        Read(hsStream *stream, hsResMgr *group);

	static hsScalar		GetYonScale() { return fYonScale; }
	static hsScalar		SetYonScale(hsScalar s);

	virtual hsBool MsgReceive(plMessage* msg);
};

#endif // hsGEnviron_inc
