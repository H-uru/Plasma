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
#include "plFixedWaterState7.h"

#include "hsStream.h"

void plFixedWaterState7::WaveState::Set(const plFixedWaterState7::WaveState& w, float secs)
{
    fMaxLength.Set(w.fMaxLength, secs);
    fMinLength.Set(w.fMinLength, secs);
    fAmpOverLen.Set(w.fAmpOverLen, secs);
    fChop.Set(w.fChop, secs);
    fAngleDev.Set(w.fAngleDev, secs);
}

void plFixedWaterState7::Set(const plFixedWaterState7& t, float secs)
{
    fWindDir.Set(t.fWindDir, secs);

    fGeoState.Set(t.fGeoState, secs);

    fTexState.Set(t.fTexState, secs);
    fRippleScale.Set(t.fRippleScale, secs);

    fSpecVec.Set(t.fSpecVec, secs);

    fWaterHeight.Set(t.fWaterHeight, secs);
    fWaterOffset.Set(t.fWaterOffset, secs);
    fMaxAtten.Set(t.fMaxAtten, secs);
    fMinAtten.Set(t.fMinAtten, secs);
    fDepthFalloff.Set(t.fDepthFalloff, secs);

    fWispiness.Set(t.fWispiness, secs);
    fShoreTint.Set(t.fShoreTint, secs);
    fMaxColor.Set(t.fMaxColor, secs);
    fMinColor.Set(t.fMinColor, secs);
    fEdgeOpac.Set(t.fEdgeOpac, secs);
    fEdgeRadius.Set(t.fEdgeRadius, secs);

    fPeriod.Set(t.fPeriod, secs);
    fFingerLength.Set(t.fFingerLength, secs);


    fWaterTint.Set(t.fWaterTint, secs);
    fSpecularTint.Set(t.fSpecularTint, secs);

    fEnvCenter.Set(t.fEnvCenter, secs);
    fEnvRefresh.Set(t.fEnvRefresh, secs);
    fEnvRadius.Set(t.fEnvRadius, secs);

    
}

void plFixedWaterState7::WaveState::Read(hsStream* s)
{
    fMaxLength.Read(s);
    fMinLength.Read(s);
    fAmpOverLen.Read(s);
    fChop.Read(s);
    fAngleDev.Read(s);
}

void plFixedWaterState7::WaveState::Write(hsStream* s) const
{
    fMaxLength.Write(s);
    fMinLength.Write(s);
    fAmpOverLen.Write(s);
    fChop.Write(s);
    fAngleDev.Write(s);
}

void plFixedWaterState7::Read(hsStream* s)
{
    // Geometric waves
    fGeoState.Read(s);

    // Texture waves
    fTexState.Read(s);
    fRippleScale.Read(s);

    // Geometric and Texture share wind direction
    fWindDir.Read(s);

    // Level of noise added during summation of texture waves
    fSpecVec.Read(s);

    // Depth parameters. Affect how the depth of
    // the water vertex is interpreted into water
    // surface properties.
    fWaterHeight.Read(s);
    fWaterOffset.Read(s);
    fMaxAtten.Read(s);
    fMinAtten.Read(s);
    fDepthFalloff.Read(s);

    // Shore parameters

    // Appearance
    fWispiness.Read(s);
    fShoreTint.Read(s);
    // Next two only used in generation of bubble layer
    fMaxColor.Read(s);
    fMinColor.Read(s);
    fEdgeOpac.Read(s);
    fEdgeRadius.Read(s);

    // Simulation
    fPeriod.Read(s);
    fFingerLength.Read(s);


    // Water appearance.
    fWaterTint.Read(s);
    fSpecularTint.Read(s);

    fEnvCenter.Read(s);
    fEnvRefresh.Read(s);
    fEnvRadius.Read(s);
}

void plFixedWaterState7::Write(hsStream* s) const
{
    // Geometric waves
    fGeoState.Write(s);

    // Texture waves
    fTexState.Write(s);
    fRippleScale.Write(s);

    // Geometric and Texture share wind direction
    fWindDir.Write(s);

    // Level of noise added during summation of texture waves
    fSpecVec.Write(s);

    // Depth parameters. Affect how the depth of
    // the water vertex is interpreted into water
    // surface properties.
    fWaterHeight.Write(s);
    fWaterOffset.Write(s);
    fMaxAtten.Write(s);
    fMinAtten.Write(s);
    fDepthFalloff.Write(s);

    // Shore parameters

    // Appearance
    fWispiness.Write(s);
    fShoreTint.Write(s);
    // Next two only used in generation of bubble layer
    fMaxColor.Write(s);
    fMinColor.Write(s);
    fEdgeOpac.Write(s);
    fEdgeRadius.Write(s);

    // Simulation
    fPeriod.Write(s);
    fFingerLength.Write(s);


    // Water appearance.
    fWaterTint.Write(s);
    fSpecularTint.Write(s);

    fEnvCenter.Write(s);
    fEnvRefresh.Write(s);
    fEnvRadius.Write(s);
}
