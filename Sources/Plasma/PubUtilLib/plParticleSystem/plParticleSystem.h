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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef plParticleSystem_inc
#define plParticleSystem_inc

#include <vector>

#include "hsGeometry3.h"
#include "pnModifier/plModifier.h"
#include "hsColorRGBA.h"
#include "hsMatrix44.h"
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
    friend class plParticleEmitter;
    friend class plSimpleParticleGenerator;

protected:
    static const float GRAVITY_ACCEL_FEET_PER_SEC2;
    plSceneObject *fTarget;

    hsGMaterial *fTexture;          // One texture per system (Tiling is your friend!)  
    uint32_t fXTiles, fYTiles;        // Width/height of the texture (in tiles) for determining a particle's UVs

    double fCurrTime;
    double fLastTime;
    hsVector3 fAccel;
    float fPreSim;
    float fDrag;
    float fWindMult;
    bool    fAttachedToAvatar;

    uint32_t fMaxTotalParticles;
    uint32_t fMaxTotalParticlesLeft;
    uint32_t fNumValidEmitters;
    uint32_t fMaxEmitters;
    uint32_t fNextEmitterToGo;

    plParticleEmitter **fEmitters;  // Various locations we're emitting particles from (the first one is
                                    // reserved for particles added explicitly (to keep all the bookkeeping
                                    // in the hands of plParticleEmitter).
    
    std::vector<plParticleEffect *> fForces;       // Global forces (wind/gravity/etc) that affect acceleration.
    std::vector<plParticleEffect *> fEffects;      // Any other non-constraint effects.
    std::vector<plParticleEffect *> fConstraints;  // Rigid body, collision, connectivity, etc.
    plParticleContext   fContext; // Rendering context passed to forces/effects/constraints.

    std::vector<plKey>  fPermaLights; // Runtime lights assigned to this system. Currently don't support projected lights on particles.

    // Material related animations, mapped over the course of a particle's life
    plController *fAmbientCtl;
    plController *fDiffuseCtl;
    plController *fOpacityCtl;  
    plController *fWidthCtl;
    plController *fHeightCtl;

    plParticleSDLMod *fParticleSDLMod;

    bool IShouldUpdate(plPipeline* pipe) const;
    bool IEval(double secs, float del, uint32_t dirty) override; // required by plModifier
    void IHandleRenderMsg(plPipeline* pipe);
    plDrawInterface* ICheckDrawInterface();
    void IAddEffect(plParticleEffect *effect, uint32_t type);
    void IReadEffectsArray(std::vector<plParticleEffect *> &effects, uint32_t type, hsStream *s, hsResMgr *mgr);
    void IPreSim();

public:
    plParticleSystem()
        : fParticleSDLMod(), fAttachedToAvatar(), fTarget(), fTexture(),
          fXTiles(), fYTiles(), fCurrTime(), fLastTime(), fPreSim(), fDrag(),
          fWindMult(), fMaxTotalParticles(), fMaxTotalParticlesLeft(),
          fNumValidEmitters(), fMaxEmitters(), fNextEmitterToGo(), fEmitters(),
          fContext(), fAmbientCtl(), fDiffuseCtl(), fOpacityCtl(),
          fWidthCtl(), fHeightCtl(), fMiscFlags()      
    { }
    virtual ~plParticleSystem();
    void Init(uint32_t xTiles, uint32_t yTiles, uint32_t maxTotalParticles, uint32_t numEmitters, 
              plController *ambientCtl, plController *diffuseCtl, plController *opacityCtl, 
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
    uint8_t fMiscFlags; // Not read/written (could be, but it's not needed yet.)
    
    // There might not be enough particles available. this function returns the number of maxParticles that
    // the emitter actually received.
    uint32_t AddEmitter(uint32_t maxParticles, plParticleGenerator *gen, uint32_t emitterFlags);

    plParticleEmitter* GetAvailEmitter();
    
    CLASSNAME_REGISTER( plParticleSystem );
    GETINTERFACE_ANY( plParticleSystem, plModifier);
    
    // These are just public wrappers to the equivalent plParticleEmitter functions, provided for the purpose
    // of adding particles to a system explicitly.
    void AddParticle(hsPoint3 &pos, hsVector3 &velocity, uint32_t tileIndex, 
        float hSize, float vSize, float scale, float invMass, float life, 
        hsPoint3 &orientation, uint32_t miscFlags, float radsPerSec=0.f);
    void GenerateParticles(uint32_t num, float dt = 0.f);
    void WipeExistingParticles(); // Instant nuke
    void KillParticles(float num, float timeToDie, uint8_t flags); // Sets a death timer. They'll get removed on the next update (if time has run out)
    uint16_t StealParticlesFrom(plParticleSystem *victim, uint16_t num); // Returns the number of particles actually stolen     
    void TranslateAllParticles(hsPoint3 &amount); // Used to recenter the system when linking between ages. 
    
    void DisableGenerators();
    uint32_t GetNumTiles() const { return fXTiles * fYTiles; }
    void SetTileIndex(plParticle &particle, uint32_t index); // Sets the UV coordinates appropriate for the current texture
    uint32_t GetNumValidParticles(bool immortalOnly = false) const; // Takes a bit longer if we want a count of immortal particles...
    uint32_t GetMaxTotalParticles() const { return fMaxTotalParticles; }
    
    const hsMatrix44 &GetLocalToWorld() const;
    void SetAccel(const hsVector3& a) { fAccel = GRAVITY_ACCEL_FEET_PER_SEC2 * a; }
    void SetGravity(float pct) { fAccel.Set(0, 0, -GRAVITY_ACCEL_FEET_PER_SEC2 * pct); }
    void SetDrag(float d) { fDrag = -d; }
    void SetWindMult(float m) { fWindMult = m; }
    void SetPreSim(float time) { fPreSim = time; }
    void UpdateGenerator(uint32_t paramID, float value);
    plParticleGenerator *GetExportedGenerator() const;
    
    const hsVector3& GetAccel() const { return fAccel; }
    float GetDrag() const { return fDrag; }
    float GetWindMult() const { return fWindMult; }
    plParticleEffect *GetEffect(uint16_t type) const;
    
    plParticleSDLMod* GetSDLMod() {return fParticleSDLMod;}
    // Functions related to/required by plModifier
    size_t GetNumTargets() const override { return fTarget ? 1 : 0; }
    plSceneObject* GetTarget(size_t w) const override { hsAssert(w < 1, "Bad target"); return fTarget; }
    void AddTarget(plSceneObject* so) override;
    void RemoveTarget(plSceneObject* so) override;
    
    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;
    bool MsgReceive(plMessage* msg) override;
    
    void SetAttachedToAvatar(bool attached);
    
    // Export only functions for building the system. Not supported at runtime.
    // AddLight allows the particle system to remain in ignorant bliss about runtime lights
    void AddLight(plKey liKey);
};

#endif
