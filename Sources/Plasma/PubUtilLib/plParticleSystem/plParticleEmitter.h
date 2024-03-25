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
#ifndef plParticleEmitter_inc
#define plParticleEmitter_inc

#include "hsGeometry3.h"
#include "hsBounds.h"
#include "hsColorRGBA.h"

#include "plEffectTargetInfo.h"

#include "pnFactory/plCreatable.h"

class hsBounds3Ext;
class plParticleSystem;
class plParticleCore;
class plParticleExt;
class plParticleGenerator;
class plSimpleParticleGenerator;
class hsResMgr;

// This just holds a bunch of parameters for an emission location. A particle system can have several of these

class plParticleEmitter : public plCreatable
{
    friend class plParticleSystem;
    friend class plSimpleParticleGenerator;

public:
    plParticleEmitter();
    ~plParticleEmitter();
    void Init(plParticleSystem *system, uint32_t maxParticles, uint32_t spanIndex, uint32_t miscFlags,
              plParticleGenerator *gen = nullptr);
    void Clone(plParticleEmitter* src, uint32_t spanIndex);

    plParticleCore *GetParticleArray() const { return fParticleCores; }
    uint32_t GetParticleCount() const { return fNumValidParticles; }
    uint32_t GetNumTiles() const;
    const hsBounds3Ext &GetBoundingBox() const { return fBoundBox; }
    uint32_t GetSpanIndex() const { return fSpanIndex; }
    const hsMatrix44 &GetLocalToWorld() const;

    void AddParticle(hsPoint3 &pos, hsVector3 &velocity, uint32_t tileIndex, 
                     float hSize, float vSize, float scale, float invMass, float life,
                     hsPoint3 &orientation, uint32_t miscFlags, float radsPerSec=0);
    void WipeExistingParticles();
    void KillParticles(float num, float timeToDie, uint8_t flags);  
    uint16_t StealParticlesFrom(plParticleEmitter *victim, uint16_t num); // returns the number actually stolen
    void TranslateAllParticles(hsPoint3 &amount); // Used to recenter the system when linking between ages. 
    void UpdateGenerator(uint32_t paramID, float paramValue);

    static uint32_t CreateHexColor(const hsColorRGBA &color);
    static uint32_t CreateHexColor(const float r, const float g, const float b, const float a);

    void OverrideLocalToWorld(const hsMatrix44& l2w);
    void UnOverrideLocalToWorld() { fMiscFlags &= ~kOverrideLocalToWorld; }
    bool LocalToWorldOverridden() const { return 0 != (fMiscFlags & kOverrideLocalToWorld); }
    void SetTimeToLive(float dt) { fTimeToLive = dt; }
    float GetTimeToLive() const { return fTimeToLive; } // 0 time to live is never turn off.

    CLASSNAME_REGISTER( plParticleEmitter );
    GETINTERFACE_ANY( plParticleEmitter, plCreatable);

    void Read(hsStream* s, hsResMgr *mgr) override;
    void Write(hsStream* s, hsResMgr *mgr) override;

    enum // Miscellaneous flags
    {
        kMatIsEmissive              = 0x00000001,

        kNormalUp                   = 0x00000010,
        kNormalVelUpVel             = 0x00000020,
        kNormalFromCenter           = 0x00000040,
        kNormalDynamicMask          = kNormalVelUpVel | kNormalFromCenter, // precalc methods that need updating each frame
        kNormalPrecalcMask          = kNormalDynamicMask | kNormalUp, // All types where emitter precalculates the normal

        kNormalViewFacing           = 0x00000100,
        kNormalNearestLight         = 0x00000200,

        kNeedsUpdate                = 0x01000000,
        kBorrowedGenerator          = 0x02000000,
        kOverrideLocalToWorld       = 0x04000000,
        kOnReserve                  = 0x08000000,

        kOrientationUp              = 0x10000000,
        kOrientationVelocityBased   = 0x20000000,
        kOrientationVelocityStretch = 0x40000000,
        kOrientationVelocityFlow    = 0x80000000,
        kOrientationVelocityMask    = kOrientationVelocityBased | kOrientationVelocityStretch | kOrientationVelocityFlow, // Velocity dependent
        kOrientationMask            = kOrientationUp | kOrientationVelocityMask,
    };
    uint32_t fMiscFlags;

protected:

    plParticleSystem *fSystem;          // The particle system this belongs to.
    plParticleCore *fParticleCores;     // The particle pool, created on init, initialized as needed, and recycled. 
    plParticleExt *fParticleExts;       // Same mapping as the Core pool. Contains extra info the render pipeline
                                        // doesn't need.

    plParticleGenerator *fGenerator;    // Optional auto generator (have this be nil if you don't want auto-generation)
    uint32_t fSpanIndex;                  // Index of the span that this emitter uses.
    uint32_t fNumValidParticles;          
    uint32_t fMaxParticles;
    hsBounds3Ext fBoundBox;
    plEffectTargetInfo fTargetInfo;     // A collection of pointers and strides that plParticleEffects will manipulate.
    hsColorRGBA fColor;

    hsMatrix44 fLocalToWorld;
    float fTimeToLive;

    void IClear();
    void ISetupParticleMem();
    void ISetSystem(plParticleSystem *sys) { fSystem = sys; }
    bool IUpdate(float delta);
    void IUpdateParticles(float delta);
    void IUpdateBoundsAndNormals(float delta);
    void IRemoveParticle(uint32_t index);
};

#endif
