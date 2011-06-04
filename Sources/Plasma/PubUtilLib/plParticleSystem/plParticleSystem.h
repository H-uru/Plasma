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
#ifndef plParticleSystem_inc
#define plParticleSystem_inc

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "../../NucleusLib/pnModifier/plModifier.h"
#include "../pnNetCommon/plSynchedValue.h"
#include "../CoreLib/hsColorRGBA.h"
#include "../CoreLib/hsMatrix44.h"
#include "plEffectTargetInfo.h"

class plPipeline;
class plParticle;
class plParticleGenerator;
class plSimpleParticleGenerator;
class plParticleEmitter;
class plParticleEffect;
class plParticleSDLMod;
class plController;
class hsGMaterial;
class plDrawableSpans;
class plDrawInterface;
class Mtl;

// This is responsible for creating and maintaining (allocation and update cycle) a related set of particles. 
// Automatic particle generation is handled by the plParticleEmitters (one for each spawning location).

class plParticleSystem : public plModifier
{
	friend plParticleEmitter;
	friend plSimpleParticleGenerator;

protected:
	static const hsScalar GRAVITY_ACCEL_FEET_PER_SEC2;
	plSceneObject *fTarget;

	hsGMaterial *fTexture;			// One texture per system (Tiling is your friend!)	
	UInt32 fXTiles, fYTiles;		// Width/height of the texture (in tiles) for determining a particle's UVs

	double fCurrTime;
	double fLastTime;
	hsVector3 fAccel;
	hsScalar fPreSim;
	hsScalar fDrag;
	hsScalar fWindMult;
	bool	fAttachedToAvatar;

	UInt32 fMaxTotalParticles;
	UInt32 fMaxTotalParticlesLeft;
	UInt32 fNumValidEmitters;
	UInt32 fMaxEmitters;
	UInt32 fNextEmitterToGo;

	plParticleEmitter **fEmitters;	// Various locations we're emitting particles from (the first one is
									// reserved for particles added explicitly (to keep all the bookkeeping
									// in the hands of plParticleEmitter).
	
	hsTArray<plParticleEffect *> fForces;		// Global forces (wind/gravity/etc) that affect accelleration.
	hsTArray<plParticleEffect *> fEffects;		// Any other non-constraint effects.
	hsTArray<plParticleEffect *> fConstraints;	// Rigid body, collision, connectivity, etc.
	plParticleContext	fContext; // Rendering context passed to forces/effects/constraints.

	hsTArray<plKey>		fPermaLights; // Runtime lights assigned to this system. Currently don't support projected lights on particles.

	// Material related animations, mapped over the course of a particle's life
	plController *fAmbientCtl;
	plController *fDiffuseCtl;
	plController *fOpacityCtl;	
	plController *fWidthCtl;
	plController *fHeightCtl;

	plParticleSDLMod *fParticleSDLMod;

	hsBool IShouldUpdate(plPipeline* pipe) const;
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty); // required by plModifier
	void IHandleRenderMsg(plPipeline* pipe);
	plDrawInterface* ICheckDrawInterface();
	void IAddEffect(plParticleEffect *effect, UInt32 type);
	void IReadEffectsArray(hsTArray<plParticleEffect *> &effects, UInt32 type, hsStream *s, hsResMgr *mgr);
	void IPreSim();

public:
	plParticleSystem();
	virtual ~plParticleSystem();
	void Init(UInt32 xTiles, UInt32 yTiles, UInt32 maxTotalParticles, UInt32 numEmitters, 
			  plController *ambientCtl,	plController *diffuseCtl, plController *opacityCtl,	
			  plController *widthCtl, plController *heightCtl);

	enum effectType
	{
		kEffectForce = 0x1,
		kEffectMisc = 0x2,
		kEffectConstraint = 0x4,
	};
	
	enum miscFlags
	{
		kParticleSystemAlwaysUpdate = 0x1
	};
	UInt8 fMiscFlags; // Not read/written (could be, but it's not needed yet.)
	
	// There might not be enough particles available. this function returns the number of maxParticles that
	// the emitter actually received.
	UInt32 AddEmitter(UInt32 maxParticles, plParticleGenerator *gen, UInt32 emitterFlags);

	plParticleEmitter* GetAvailEmitter();
	
	CLASSNAME_REGISTER( plParticleSystem );
	GETINTERFACE_ANY( plParticleSystem, plModifier);
	
	// These are just public wrappers to the equivalent plParticleEmitter functions, provided for the purpose
	// of adding particles to a system explicitly.
	void AddParticle(hsPoint3 &pos, hsVector3 &velocity, UInt32 tileIndex, 
		hsScalar hSize, hsScalar vSize, hsScalar scale, hsScalar invMass, hsScalar life, 
		hsPoint3 &orientation, UInt32 miscFlags, hsScalar radsPerSec=0.f);
	void GenerateParticles(UInt32 num, hsScalar dt = 0.f);
	void WipeExistingParticles(); // Instant nuke
	void KillParticles(hsScalar num, hsScalar timeToDie, UInt8 flags); // Sets a death timer. They'll get removed on the next update (if time has run out)
	UInt16 StealParticlesFrom(plParticleSystem *victim, UInt16 num); // Returns the number of particles actually stolen		
	void TranslateAllParticles(hsPoint3 &amount); // Used to recenter the system when linking between ages.	
	
	void DisableGenerators();
	UInt32 GetNumTiles() const { return fXTiles * fYTiles; }
	void SetTileIndex(plParticle &particle, UInt32 index); // Sets the UV coordinates appropriate for the current texture
	UInt32 GetNumValidParticles(hsBool immortalOnly = false) const; // Takes a bit longer if we want a count of immortal particles...
	UInt32 GetMaxTotalParticles() const { return fMaxTotalParticles; }
	
	const hsMatrix44 &GetLocalToWorld() const;
	void SetAccel(const hsVector3& a) { fAccel = GRAVITY_ACCEL_FEET_PER_SEC2 * a; }
	void SetGravity(hsScalar pct) { fAccel.Set(0, 0, -GRAVITY_ACCEL_FEET_PER_SEC2 * pct); }
	void SetDrag(hsScalar d) { fDrag = -d; }
	void SetWindMult(hsScalar m) { fWindMult = m; }
	void SetPreSim(hsScalar time) { fPreSim = time; }
	void UpdateGenerator(UInt32 paramID, hsScalar value);
	plParticleGenerator *GetExportedGenerator() const;
	
	const hsVector3& GetAccel() const { return fAccel; }
	hsScalar GetDrag() const { return fDrag; }
	hsScalar GetWindMult() const { return fWindMult; }
	plParticleEffect *GetEffect(UInt16 type) const;
	
	plParticleSDLMod* GetSDLMod() {return fParticleSDLMod;}
	// Functions related to/required by plModifier
	virtual int GetNumTargets() const { return fTarget ? 1 : 0; }
	virtual plSceneObject* GetTarget(int w) const { hsAssert(w < 1, "Bad target"); return fTarget; }
	virtual void AddTarget(plSceneObject* so);
	virtual void RemoveTarget(plSceneObject* so);
	
	virtual void Read(hsStream* s, hsResMgr* mgr); 
	virtual void Write(hsStream* s, hsResMgr* mgr);
	virtual hsBool MsgReceive(plMessage* msg);
	
	void SetAttachedToAvatar(bool attached);
	
	// Export only functions for building the system. Not supported at runtime.
	// AddLight allows the particle system to remain in ignorant bliss about runtime lights
	void AddLight(plKey liKey);
};

#endif
