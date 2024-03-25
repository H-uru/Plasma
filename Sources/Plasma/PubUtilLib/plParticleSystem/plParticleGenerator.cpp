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

#include "plParticleGenerator.h"
#include "plParticle.h"
#include "plParticleEmitter.h"
#include "plParticleSystem.h"

#include "hsColorRGBA.h"
#include "hsFastMath.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "pnEncryption/plRandom.h"

#include "plInterp/plController.h"
#include "plMessage/plParticleUpdateMsg.h"

static const float DEFAULT_INVERSE_MASS = 1.f;

static plRandom sRandom;

void plParticleGenerator::ComputeDirection(float pitch, float yaw, hsVector3 &direction)
{
    float cosPitch, sinPitch;
    float cosYaw, sinYaw;
    hsFastMath::SinCos(pitch, sinPitch, cosPitch);
    hsFastMath::SinCos(yaw, sinYaw, cosYaw);        

    direction.Set(-sinYaw * cosPitch, sinPitch, cosPitch * cosYaw);
}

// Inverse function of ComputeDirection. Give it a normalized vector, and it will tell you a
// pitch and yaw (angles for the unit Z vector) to get there.
void plParticleGenerator::ComputePitchYaw(float &pitch, float &yaw, const hsVector3 &dir)
{
    pitch = asin(dir.fY);
    float cos_pitch = cos(pitch);
    if (cos_pitch == 0)
    {
        yaw = 0;
        return;
    }
    float inv = -dir.fX / cos_pitch;
    if (inv > 1.0f)
        inv = 1.0f;
    if (inv < -1.0f)
        inv = -1.0f;
    yaw = asin(inv);
    if (dir.fZ < 0)
        yaw = hsConstants::pi<float> - yaw;
}


plSimpleParticleGenerator::~plSimpleParticleGenerator()
{
    delete [] fInitPos;
    delete [] fInitPitch;
    delete [] fInitYaw;
}

void plSimpleParticleGenerator::Init(float genLife, float partLifeMin, float partLifeMax,
                                     float particlesPerSecond, uint32_t numSources, hsPoint3 *initPos,
                                     float *initPitch, float *initYaw, float angleRange, 
                                     float initVelMin, float initVelMax,
                                     float xSize, float ySize, 
                                     float scaleMin, float scaleMax,
                                     float massRange, float radsPerSecRange)
{
    fGenLife = genLife;
    fPartLifeMin = partLifeMin;
    fPartLifeMax = partLifeMax;
    fParticlesPerSecond = particlesPerSecond;
    fNumSources = numSources;
    fInitPos = initPos;
    fInitPitch = initPitch;
    fInitYaw = initYaw;
    fAngleRange = angleRange;
    fVelMin = initVelMin;
    fVelMax = initVelMax;
    fXSize = xSize;
    fYSize = ySize;
    fScaleMin = scaleMin;
    fScaleMax = scaleMax;

    fPartInvMassMin = 1.f / (DEFAULT_INVERSE_MASS + massRange);
    fPartInvMassRange = 1.f / DEFAULT_INVERSE_MASS - fPartInvMassMin;

    fPartRadsPerSecRange = radsPerSecRange;

    fParticleSum = 0;
    fMiscFlags = 0;
    if (fGenLife < 0) fMiscFlags |= kImmortal;
}

bool plSimpleParticleGenerator::AddAutoParticles(plParticleEmitter *emitter, float dt, uint32_t numForced /* = 0 */)
{
    int32_t numNewParticles;

    if (numForced == 0)
    {
        fGenLife -= dt;
        if ((fGenLife < 0 && !(fMiscFlags & kImmortal)) || (fMiscFlags & kDisabled))
            return true; // Leave it around so that a message can bring it back to life.

        fParticleSum += fParticlesPerSecond * dt;
        numNewParticles = (int32_t)fParticleSum;
    
        if (numNewParticles <= 0 || fParticlesPerSecond == 0)
            return true;
    }
    else
    {
        numNewParticles = numForced;
    }

    uint32_t miscFlags = 0; 
    hsPoint3 currStart;
    fParticleSum -= numNewParticles;

    hsPoint3 orientation;
    hsVector3 initDirection;
    float vel = (fVelMax + fVelMin) * 0.5f;
    float velRange = vel - fVelMin;
    float initVelocity;
    float initLife;
    float life = (fPartLifeMax + fPartLifeMin) * 0.5f;
    float lifeRange = life - fPartLifeMin;
    float currSizeVar;
    float scale = (fScaleMax + fScaleMin) * 0.5f;
    float scaleRange = scale - fScaleMin;
    float radsPerSec = 0;
    uint32_t tile;
    uint32_t sourceIndex;

    const float lifeDiff = dt / numNewParticles;
    float lifeSoFar;
    int i;  
    for (i = 0, lifeSoFar = 0; i < numNewParticles; i++, lifeSoFar += lifeDiff)
    {
        initLife = life + lifeRange * sRandom.RandMinusOneToOne() - lifeSoFar;

        // Careful here... if we're supposed to generate immortal particles, we do so
        // by giving them a negative life. This is different that generating one with
        // a positive lifetime that is now negative because of "lifeSoFar". The if is
        // saying "if it's dead, but it was alive before we took away lifeSoFar, ignore it"
        if (initLife <= 0 && initLife + lifeSoFar >= 0)
            continue;

        sourceIndex = (uint32_t)(sRandom.RandZeroToOne() * fNumSources);

        ComputeDirection(fInitPitch[sourceIndex] + fAngleRange * sRandom.RandMinusOneToOne(), 
                         fInitYaw[sourceIndex] + fAngleRange * sRandom.RandMinusOneToOne(), initDirection);
        initDirection = emitter->GetLocalToWorld() * initDirection;
        initVelocity = (vel + velRange * sRandom.RandMinusOneToOne());
        
        currStart = (emitter->GetLocalToWorld() * fInitPos[sourceIndex])
                    + (initDirection * initVelocity * lifeSoFar) // Vo * t
                    + (emitter->fSystem->fAccel * lifeSoFar * lifeSoFar); // at^2
        
        if (emitter->fMiscFlags & emitter->kOrientationUp)
            orientation.Set(0.0f, -1.0f, 0.0f);
        else
            orientation.Set(&initDirection);

        tile = (uint32_t)(sRandom.RandZeroToOne() * emitter->GetNumTiles());
        currSizeVar = scale + scaleRange * sRandom.RandMinusOneToOne();

        float invMass = fPartInvMassMin;
        // Might be faster to just do the math instead of checking for zero...
        if( fPartInvMassRange > 0 )
            invMass += fPartInvMassRange * sRandom.RandZeroToOne();

        if( fPartRadsPerSecRange > 0 )
            radsPerSec = fPartRadsPerSecRange * sRandom.RandMinusOneToOne();

        hsVector3 tmp = initDirection * initVelocity;
        emitter->AddParticle(currStart, tmp, tile, fXSize, fYSize, currSizeVar, 
                         invMass, initLife, orientation, miscFlags, radsPerSec);
    }

    return true;
}

void plSimpleParticleGenerator::UpdateParam(uint32_t paramID, float paramValue)
{
    switch (paramID)
    {
    case plParticleUpdateMsg::kParamParticlesPerSecond:
        fParticlesPerSecond = paramValue;
        break;
    case plParticleUpdateMsg::kParamInitPitchRange:
    case plParticleUpdateMsg::kParamInitYawRange:
        fAngleRange = paramValue;
        break;
//  case plParticleUpdateMsg::kParamInitVel:
//      fInitVel = paramValue;
//      break;
//  case plParticleUpdateMsg::kParamInitVelRange:
//      fInitVelRange = paramValue;
//      break;
    case plParticleUpdateMsg::kParamVelMin:
        fVelMin = paramValue;
        break;
    case plParticleUpdateMsg::kParamVelMax:
        fVelMax = paramValue;
        break;
    case plParticleUpdateMsg::kParamXSize:
        fXSize = paramValue;
        break;
    case plParticleUpdateMsg::kParamYSize:
        fYSize = paramValue;
        break;
//  case plParticleUpdateMsg::kParamSizeRange:
//      fSizeRange = paramValue;
//      break;
    case plParticleUpdateMsg::kParamScaleMin:
        fScaleMin = paramValue;
        break;
    case plParticleUpdateMsg::kParamScaleMax:
        fScaleMax = paramValue;
        break;
    case plParticleUpdateMsg::kParamGenLife:
        fGenLife = paramValue;
        if (fGenLife < 0)
            fMiscFlags |= kImmortal;
        else
            fMiscFlags &= ~kImmortal;
        break;
//  case plParticleUpdateMsg::kParamPartLife:
//      fPartLife = paramValue;
//      if (fPartLife < 0)
//          fPartLifeRange = 0;
//      break;
//  case plParticleUpdateMsg::kParamPartLifeRange:
//      fPartLifeRange = paramValue;
//      break;
    case plParticleUpdateMsg::kParamPartLifeMin:
        fPartLifeMin = paramValue;
        break;
    case plParticleUpdateMsg::kParamPartLifeMax:
        fPartLifeMax = paramValue;
        break;
    case plParticleUpdateMsg::kParamEnabled:
        if (paramValue == 0.f)
            fMiscFlags |= kDisabled;
        else
            fMiscFlags &= ~kDisabled;
        break;
    default:
        break;
    }
}

void plSimpleParticleGenerator::Read(hsStream* s, hsResMgr *mgr)
{
    float genLife = s->ReadLEFloat();
    float partLifeMin = s->ReadLEFloat();
    float partLifeMax = s->ReadLEFloat();
    float pps = s->ReadLEFloat();
    uint32_t numSources = s->ReadLE32();
    hsPoint3 *pos = new hsPoint3[numSources];
    float *pitch = new float[numSources];
    float *yaw = new float[numSources];
    int i;
    for (i = 0; i < numSources; i++)
    {
        pos[i].Read(s);
        pitch[i] = s->ReadLEFloat();
        yaw[i] = s->ReadLEFloat();
    }
    float angleRange = s->ReadLEFloat();
    float velMin = s->ReadLEFloat();
    float velMax = s->ReadLEFloat();
    float xSize = s->ReadLEFloat();
    float ySize = s->ReadLEFloat();
    float scaleMin = s->ReadLEFloat();
    float scaleMax = s->ReadLEFloat();
    float massRange = s->ReadLEFloat();
    float radsPerSec = s->ReadLEFloat();

    Init(genLife, partLifeMin, partLifeMax, pps, numSources, pos, pitch, yaw, angleRange, velMin, velMax,
         xSize, ySize, scaleMin, scaleMax, massRange, radsPerSec);
}

void plSimpleParticleGenerator::Write(hsStream* s, hsResMgr *mgr)
{
    s->WriteLEFloat(fGenLife);
    s->WriteLEFloat(fPartLifeMin);
    s->WriteLEFloat(fPartLifeMax);
    s->WriteLEFloat(fParticlesPerSecond);
    s->WriteLE32(fNumSources);
    int i;
    for (i = 0; i < fNumSources; i++)
    {
        fInitPos[i].Write(s);
        s->WriteLEFloat(fInitPitch[i]);
        s->WriteLEFloat(fInitYaw[i]);
    }
    s->WriteLEFloat(fAngleRange);
    s->WriteLEFloat(fVelMin);
    s->WriteLEFloat(fVelMax);
    s->WriteLEFloat(fXSize);
    s->WriteLEFloat(fYSize);
    s->WriteLEFloat(fScaleMin);
    s->WriteLEFloat(fScaleMax);

    float massRange = 1.f / fPartInvMassMin - DEFAULT_INVERSE_MASS;
    s->WriteLEFloat(massRange);
    s->WriteLEFloat(fPartRadsPerSecRange);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

plOneTimeParticleGenerator::~plOneTimeParticleGenerator()
{
    delete [] fPosition;
    delete [] fDirection;
}

void plOneTimeParticleGenerator::Init(float count, hsPoint3 *pointArray, hsVector3 *dirArray, 
                                      float xSize, float ySize, float scaleMin, float scaleMax, float radsPerSecRange)
{
    fCount = count;
    fPosition = pointArray;
    fDirection = dirArray;
    fXSize = xSize;
    fYSize = ySize;
    fScaleMin = scaleMin;
    fScaleMax = scaleMax;
    fPartRadsPerSecRange = radsPerSecRange;
}

// The numForced param is required by the parent class, but ignored by this particular generator
bool plOneTimeParticleGenerator::AddAutoParticles(plParticleEmitter *emitter, float dt, uint32_t numForced /* = 0 */)
{
    float currSizeVar;
    float scale = (fScaleMax + fScaleMin) / 2;
    float scaleRange = scale - fScaleMin;

    float tile;
    hsPoint3 currStart;
    hsPoint3 orientation;
    hsVector3 initDirection;
    hsVector3 zeroVel;
    float radsPerSec = 0;

    int i;
    for (i = 0; i < fCount; i++)
    {
        currStart = emitter->GetLocalToWorld() * fPosition[i];
        initDirection = emitter->GetLocalToWorld() * fDirection[i];

        if (emitter->fMiscFlags & emitter->kOrientationUp)
            orientation.Set(0.0f, -1.0f, 0.0f);
        else
            orientation.Set(&initDirection);

        tile = (float)(sRandom.Rand() % emitter->GetNumTiles());
        currSizeVar = scale + scaleRange * sRandom.RandMinusOneToOne();

        if( fPartRadsPerSecRange > 0 )
            radsPerSec = fPartRadsPerSecRange * sRandom.RandMinusOneToOne();

        emitter->AddParticle(currStart, zeroVel, (uint32_t)tile, fXSize, fYSize, currSizeVar, 
                             DEFAULT_INVERSE_MASS, -1, orientation, 0, radsPerSec);
    }
    emitter->fMiscFlags &= ~plParticleEmitter::kNeedsUpdate;
    return false; // We've done our one-time job. Let the emitter know to delete us.
}

void plOneTimeParticleGenerator::Read(hsStream* s, hsResMgr *mgr)
{
    uint32_t count = s->ReadLE32();
    float xSize = s->ReadLEFloat();
    float ySize = s->ReadLEFloat();
    float scaleMin = s->ReadLEFloat();
    float scaleMax = s->ReadLEFloat();
    float radsPerSecRange = s->ReadLEFloat();

    hsPoint3 *pos = new hsPoint3[count];
    hsVector3 *dir = new hsVector3[count];

    int i;
    for (i = 0; i < count; i++)
    {
        pos[i].Read(s);
        dir[i].Read(s);
    }

    Init((float)count, pos, dir, xSize, ySize, scaleMin, scaleMax, radsPerSecRange);
}

void plOneTimeParticleGenerator::Write(hsStream* s, hsResMgr *mgr)
{
    s->WriteLE32((uint32_t)fCount);
    s->WriteLEFloat(fXSize);
    s->WriteLEFloat(fYSize);
    s->WriteLEFloat(fScaleMin);
    s->WriteLEFloat(fScaleMax);
    s->WriteLEFloat(fPartRadsPerSecRange);

    int i;
    for (i = 0; i < fCount; i++)
    {
        fPosition[i].Write(s);
        fDirection[i].Write(s);
    }
}
