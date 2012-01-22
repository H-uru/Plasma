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

#include "HeadSpin.h"

#include "plShadowSlave.h"
#include "plTweak.h"

#include <float.h>
#include <math.h>

#ifndef isnan
#define isnan _isnan
#endif

static const float kMinMinZ = 1.f; // totally random arbitrary number (has to be > 0).

bool plShadowSlave::ISetupOrthoViewTransform()
{
    hsBounds3Ext bnd = fCasterWorldBounds;

    bnd.Transform(&fWorldToLight);

    float minZ = bnd.GetMins().fZ;
    float maxZ = bnd.GetCenter().fZ + fAttenDist;

    float minX = bnd.GetMins().fX;
    float maxX = bnd.GetMaxs().fX;

    float minY = bnd.GetMins().fY;
    float maxY = bnd.GetMaxs().fY;


    hsMatrix44 proj;
    proj.Reset();
    proj.NotIdentity();

    // First the LightToTexture, which uses the above pretty much as is.
    // Note the remapping to range [0.5..width-0.5] etc. 
    // About this kAdjustBias. According to the docs, it should be 0.5,
    // and on the perspective projection 0.5 works great. But on the
    // directional (ortho) projection, it just makes the mapping wrong,
    // and an offset of zero works great. This could be a driver bug, or
    // hardware "dependency" (read IHV bug), but whatever, zero is working
    // now. Might need to adjust for new drivers or other hardware.
    const float kAdjustBias = 0.0f;
    proj.fMap[0][0] = 1.f / (maxX - minX);
    proj.fMap[0][3] = -minX / (maxX - minX) + kAdjustBias / fWidth;
    proj.fMap[1][1] = -1.f / (maxY - minY);
    proj.fMap[1][3] = -minY / (maxY - minY) + kAdjustBias / fHeight;
    proj.fMap[2][2] = 1.f;
    proj.fMap[3][3] = 1.f;

    fWorldToTexture = proj * fWorldToLight;

    // Now the LightToNDC. This one's a little trickier, because we want to compensate for
    // having brought in the viewport to keep our border constant, so we can clamp the 
    // projected texture and not have the edges smear off to infinity.
    // Like the adjust bias above, this part is correct in theory, but only
    // screws things up (increases Z-acne).
#if 0
    float delX = maxX - minX;
    minX += delX / (fWidth * 0.5f);
    maxX -= delX / (fWidth * 0.5f);
    float delY = maxY - minY;
    minY += delY / (fHeight * 0.5f);
    maxY -= delY / (fHeight * 0.5f);
#endif 


    fView.SetView(hsPoint3(minX, minY, minZ), hsPoint3(maxX, maxY, maxZ));
    fView.SetScreenSize((uint16_t)fWidth, (uint16_t)fHeight);
    fView.SetCameraTransform(fWorldToLight, fLightToWorld);
    fView.SetPerspective(false);
    fView.SetViewPort(0, 0, float(fWidth), float(fHeight), false);

    fLightDir = fLightToWorld.GetAxis(hsMatrix44::kUp);
    SetFlag(kPositional, false);
    SetFlag(kReverseCull, true);
    
    return true;
}

bool plShadowSlave::ISetupPerspViewTransform()
{
    hsBounds3Ext bnd = fCasterWorldBounds;

    bnd.Transform(&fWorldToLight);

    float minZ = bnd.GetMins().fZ;
    float maxZ = bnd.GetCenter().fZ + fAttenDist;

    if( minZ < kMinMinZ )
        minZ = kMinMinZ;

    // EAP
    // This is my hack to get the Nexus age working.  The real problem
    // is probably data-side.  I take full responsibility for this
    // hack-around breaking the entire system, loosing data, causing
    // unauthorized credit card transactions, etc.      
    if (isnan(bnd.GetMins().fX) || isnan(bnd.GetMins().fY))
        return false;
    if (isnan(bnd.GetMaxs().fX) || isnan(bnd.GetMaxs().fY))
        return false;

    float cotX, cotY;
    if( -bnd.GetMins().fX > bnd.GetMaxs().fX )
    {
        hsAssert(bnd.GetMins().fX < 0, "Empty shadow caster bounds?");
        cotX = -minZ / bnd.GetMins().fX;
    }
    else
    {
        hsAssert(bnd.GetMaxs().fX > 0, "Empty shadow caster bounds?");
        cotX = minZ / bnd.GetMaxs().fX;
    }

    if( -bnd.GetMins().fY > bnd.GetMaxs().fY )
    {
        hsAssert(bnd.GetMins().fY < 0, "Empty shadow caster bounds?");
        cotY = -minZ / bnd.GetMins().fY;
    }
    else
    {
        hsAssert(bnd.GetMaxs().fY > 0, "Empty shadow caster bounds?");
        cotY = minZ / bnd.GetMaxs().fY;
    }

    hsMatrix44 proj;
    proj.Reset();
    proj.NotIdentity();

    // First the LightToTexture, which uses the above pretty much as is.
    // Note the remapping to range [0.5..width-0.5] etc. Also, the perspective
    // divide is by the 3rd output (not the fourth), so we make the 3rd
    // output be W (instead of Z).
    // This also means that our translate goes into [i][2] instead of [i][3].
#if 0
    proj.fMap[0][0] = cotX * 0.5f;
    proj.fMap[0][2] = -0.5f * (1.f + 0.5f/fWidth);
    proj.fMap[1][1] = cotY * 0.5f;
    proj.fMap[1][2] = -0.5f * (1.f + 0.5f/fHeight);
#else
    plConst(float) kBiasScale(1.f);
    plConst(float) kBiasTrans(1.f);
    proj.fMap[0][0] = cotX * 0.5f * ( float(fWidth-2.f) / float(fWidth) ) * kBiasScale;
    proj.fMap[0][2] = 0.5f * (1.f - kBiasTrans * 0.5f/fWidth);
    proj.fMap[1][1] = -cotY * 0.5f * ( float(fHeight-2.f) / float(fHeight) ) * kBiasScale;
    proj.fMap[1][2] = 0.5f * (1.f - kBiasTrans * 0.5f/fHeight);
#endif

#if 0 // This computes correct Z, but we really just want W in 3rd component. HACKFISH
    proj.fMap[2][2] = maxZ / (maxZ - minZ);
    proj.fMap[2][3] = -minZ * maxZ / (maxZ - minZ);
#elif 1
    proj.fMap[2][2] = 1.f;
    proj.fMap[2][3] = 0;
#endif
    proj.fMap[3][2] = 1.f;
    proj.fMap[3][3] = 0;

    fWorldToTexture = proj * fWorldToLight;

    // Now the LightToNDC. This one's a little trickier, because we want to compensate for
    // having brought in the viewport to keep our border constant, so we can clamp the 
    // projected texture and not have the edges smear off to infinity.
    cotX -= cotX / (fWidth * 0.5f);
    cotY -= cotY / (fHeight * 0.5f);

    float tanX = 1.f / cotX;
    float tanY = 1.f / cotY;
    fView.SetView(hsPoint3(-tanX, -tanY, minZ), hsPoint3(tanX, tanY, maxZ));
    fView.SetScreenSize((uint16_t)fWidth, (uint16_t)fHeight);
    fView.SetCameraTransform(fWorldToLight, fLightToWorld);
    fView.SetPerspective(true);
    fView.SetViewPort(0, 0, float(fWidth), float(fHeight), false);

    fLightPos = fLightToWorld.GetTranslate();
    SetFlag(kPositional, true);

    return true;
}
