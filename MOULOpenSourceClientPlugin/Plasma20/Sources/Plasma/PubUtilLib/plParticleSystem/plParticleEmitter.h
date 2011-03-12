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
#ifndef plParticleEmitter_inc
#define plParticleEmitter_inc

#include "hsGeometry3.h"
#include "hsBounds.h"
#include "../pnNetCommon/plSynchedValue.h"
#include "../CoreLib/hsColorRGBA.h"

class hsBounds3Ext;
class plParticleSystem;
class plParticleCore;
class plParticleExt;
class plParticleGenerator;
class plSimpleParticleGenerator;
class hsResMgr;

#include "plEffectTargetInfo.h"

// This just holds a bunch of parameters for an emission location. A particle system can have several of these

class plParticleEmitter : public plCreatable
{
	friend plParticleSystem;
	friend plSimpleParticleGenerator;

public:
	plParticleEmitter();
	~plParticleEmitter();
	void Init(plParticleSystem *system, UInt32 maxParticles, UInt32 spanIndex, UInt32 miscFlags,
			  plParticleGenerator *gen = nil);
	void Clone(plParticleEmitter* src, UInt32 spanIndex);

	plParticleCore *GetParticleArray() const { return fParticleCores; }
	UInt32 GetParticleCount() const { return fNumValidParticles; }
	UInt32 GetNumTiles() const;
	const hsBounds3Ext &GetBoundingBox() const { return fBoundBox; }
	UInt32 GetSpanIndex() const { return fSpanIndex; }
	const hsMatrix44 &GetLocalToWorld() const;

	void AddParticle(hsPoint3 &pos, hsVector3 &velocity, UInt32 tileIndex, 
					 hsScalar hSize, hsScalar vSize, hsScalar scale, hsScalar invMass, hsScalar life,
					 hsPoint3 &orientation, UInt32 miscFlags, hsScalar radsPerSec=0);
	void WipeExistingParticles();
	void KillParticles(hsScalar num, hsScalar timeToDie, UInt8 flags);	
	UInt16 StealParticlesFrom(plParticleEmitter *victim, UInt16 num); // returns the number actually stolen
	void TranslateAllParticles(hsPoint3 &amount); // Used to recenter the system when linking between ages.	
	void UpdateGenerator(UInt32 paramID, hsScalar paramValue);

	static UInt32 CreateHexColor(const hsColorRGBA &color);
	static UInt32 CreateHexColor(const hsScalar r, const hsScalar g, const hsScalar b, const hsScalar a);

	void OverrideLocalToWorld(const hsMatrix44& l2w);
	void UnOverrideLocalToWorld() { fMiscFlags &= ~kOverrideLocalToWorld; }
	hsBool LocalToWorldOverridden() const { return 0 != (fMiscFlags & kOverrideLocalToWorld); }
	void SetTimeToLive(hsScalar dt) { fTimeToLive = dt; }
	hsScalar GetTimeToLive() const { return fTimeToLive; } // 0 time to live is never turn off.

	CLASSNAME_REGISTER( plParticleEmitter );
	GETINTERFACE_ANY( plParticleEmitter, plCreatable);

	virtual void Read(hsStream* s, hsResMgr *mgr); 
	virtual void Write(hsStream* s, hsResMgr *mgr);

	enum // Miscellaneous flags
	{
		kMatIsEmissive				= 0x00000001,

		kNormalUp					= 0x00000010,
		kNormalVelUpVel				= 0x00000020,
		kNormalFromCenter			= 0x00000040,
		kNormalDynamicMask			= kNormalVelUpVel | kNormalFromCenter, // precalc methods that need updating each frame
		kNormalPrecalcMask			= kNormalDynamicMask | kNormalUp, // All types where emitter precalculates the normal

		kNormalViewFacing			= 0x00000100,
		kNormalNearestLight			= 0x00000200,

		kNeedsUpdate				= 0x01000000,
		kBorrowedGenerator			= 0x02000000,
		kOverrideLocalToWorld		= 0x04000000,
		kOnReserve					= 0x08000000,

		kOrientationUp				= 0x10000000,
		kOrientationVelocityBased	= 0x20000000,
		kOrientationVelocityStretch	= 0x40000000,
		kOrientationVelocityFlow	= 0x80000000,
		kOrientationVelocityMask	= kOrientationVelocityBased | kOrientationVelocityStretch | kOrientationVelocityFlow, // Velocity dependent
		kOrientationMask			= kOrientationUp | kOrientationVelocityMask,
	};
	UInt32 fMiscFlags;

protected:

	plParticleSystem *fSystem;			// The particle system this belongs to.
	plParticleCore *fParticleCores;		// The particle pool, created on init, initialized as needed, and recycled. 
	plParticleExt *fParticleExts;		// Same mapping as the Core pool. Contains extra info the render pipeline
										// doesn't need.

	plParticleGenerator *fGenerator;	// Optional auto generator (have this be nil if you don't want auto-generation)
	UInt32 fSpanIndex;					// Index of the span that this emitter uses.
	UInt32 fNumValidParticles;			
	UInt32 fMaxParticles;
	hsBounds3Ext fBoundBox;
	plEffectTargetInfo fTargetInfo;		// A collection of pointers and strides that plParticleEffects will manipulate.
	hsColorRGBA fColor;

	hsMatrix44 fLocalToWorld;
	hsScalar fTimeToLive;

	void IClear();
	void ISetupParticleMem();
	void ISetSystem(plParticleSystem *sys) { fSystem = sys; }
	hsBool IUpdate(hsScalar delta);
	void IUpdateParticles(hsScalar delta);
	void IUpdateBoundsAndNormals(hsScalar delta);
	void IRemoveParticle(UInt32 index);
};

#endif
