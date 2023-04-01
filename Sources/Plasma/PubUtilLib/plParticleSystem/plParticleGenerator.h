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
#ifndef plParticleGenerator_inc
#define plParticleGenerator_inc

#include "hsGeometry3.h"

#include "pnFactory/plCreatable.h"

class plParticleEmitter;
class plScalarController;

// This class is responsible for all the details of automatically generating particles for a plParticleEmitter.
// It gets a change in time, and must do whatever necessary to generate the appropriate number of particles
// for that timespan

class plParticleGenerator : public plCreatable
{

public:
    // returns false if it's done generating particles and is safe to delete.
    virtual bool AddAutoParticles(plParticleEmitter *emitter, float dt, uint32_t numForced = 0) = 0;

    virtual void UpdateParam(uint32_t paramID, float paramValue) = 0;

    CLASSNAME_REGISTER( plParticleGenerator );
    GETINTERFACE_ANY( plParticleGenerator, plCreatable );

    static void ComputeDirection(float pitch, float yaw, hsVector3 &direction);
    static void ComputePitchYaw(float &pitch, float &yaw, const hsVector3 &dir);
    static inline float GetRandomVar() { return 2.0f * rand() / float(RAND_MAX) - 1.0f; } // returns a num between +/- 1.0
};

class plSimpleParticleGenerator : public plParticleGenerator
{
public:
    plSimpleParticleGenerator()
        : fParticlesPerSecond(), fNumSources(), fInitPos(), fInitPitch(),
          fInitYaw(), fAngleRange(), fVelMin(), fVelMax(), fXSize(), fYSize(),
          fScaleMin(), fScaleMax(), fGenLife(), fPartLifeMin(), fPartLifeMax(),
          fPartInvMassMin(), fPartInvMassRange(), fPartRadsPerSecRange(), fParticleSum(),
          fMiscFlags()
    { }
    ~plSimpleParticleGenerator();
    void Init(float genLife, float partLifeMin, float partLifeMax, float particlesPerSecond, 
              uint32_t numSources, hsPoint3 *pos, float *initPitch, float *initYaw, float angleRange,
              float initVelMin, float initVelMax, float xSize, float ySize, 
              float scaleMin, float scaleMax,
              float massRange, float radsPerSecRange);

    CLASSNAME_REGISTER( plSimpleParticleGenerator );
    GETINTERFACE_ANY( plSimpleParticleGenerator, plParticleGenerator);
    
    bool AddAutoParticles(plParticleEmitter *emitter, float dt, uint32_t numForced) override;
    void UpdateParam(uint32_t paramID, float paramValue) override;

    void Read(hsStream* s, hsResMgr *mgr) override;
    void Write(hsStream* s, hsResMgr *mgr) override;

protected:
    float fParticlesPerSecond;
    uint32_t fNumSources;
    hsPoint3 *fInitPos;
    float *fInitPitch, *fInitYaw;
    float fAngleRange;
    float fVelMin, fVelMax;
    float fXSize, fYSize, fScaleMin, fScaleMax;
    float fGenLife;  // How long shall we spit out particles from this location? When this time runs out, we stop
                        // spitting particles, but we don't actually die until all of our particles die naturally.
                        // (Even the ones that we feel are suffering needlessly.)

    float fPartLifeMin; // lifespan for the particles we generate
    float fPartLifeMax;

    float fPartInvMassMin;   // Doing a uniform variant over the inverse mass range (instead of over the mass range
    float fPartInvMassRange; // and then inverting) will favor the low end of the mass range, but then again,
                                // it's just a freaking game. Note though that fPartInvMassMin == 1.f / massMAX.

    float fPartRadsPerSecRange; // Zero means no rot, otherwise uniform random between [-range..range]

    float fParticleSum;

    enum
    {
        kImmortal   = 0x1,
        kDisabled   = 0x2,
    };
    uint32_t fMiscFlags;
};

class plOneTimeParticleGenerator : public plParticleGenerator
{
public:

    plOneTimeParticleGenerator()
        : fCount(), fPosition(), fDirection(), fXSize(), fYSize(),
          fScaleMin(), fScaleMax(), fPartRadsPerSecRange()
    { }
    ~plOneTimeParticleGenerator();
    void Init(float count, hsPoint3 *pointArray, hsVector3 *dirArray, 
              float xSize, float ySize, float scaleMin, float scaleMax, float radsPerSec);

    CLASSNAME_REGISTER( plOneTimeParticleGenerator );
    GETINTERFACE_ANY( plOneTimeParticleGenerator, plParticleGenerator);

    bool AddAutoParticles(plParticleEmitter *emitter, float dt, uint32_t numForced = 0) override;
    void UpdateParam(uint32_t paramID, float paramValue) override { }

    void Read(hsStream* s, hsResMgr *mgr) override;
    void Write(hsStream* s, hsResMgr *mgr) override;

protected:
    float fCount;
    hsPoint3 *fPosition;
    hsVector3 *fDirection;
    float fXSize, fYSize, fScaleMin, fScaleMax;
    float fPartRadsPerSecRange; // Zero means no rot, otherwise uniform random between [-range..range]
};

#endif
