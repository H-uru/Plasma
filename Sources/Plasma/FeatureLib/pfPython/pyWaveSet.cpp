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

#include <Python.h>
#include "pyKey.h"

#include "pyColor.h"
#include "pyGeometry3.h"
#include "pyWaveSet.h"
#include "plDrawable/plWaveSet7.h"

pyWaveSet::pyWaveSet(plKey key)
{
    fWaterKey = key;
}

pyWaveSet::pyWaveSet(pyKey& key)
{
    fWaterKey = key.getKey();
}

// --------------------------------------------------------------------------------
// Geometric wave parameters. These are all safe to twiddle at any time or speed.
// The new settings take effect as new waves are spawned.

void pyWaveSet::SetGeoMaxLength(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetGeoMaxLength(s, secs);
        }
    }
}

void pyWaveSet::SetGeoMinLength(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetGeoMinLength(s, secs);
        }
    }
}

void pyWaveSet::SetGeoAmpOverLen(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetGeoAmpOverLen(s, secs);
        }
    }
}

void pyWaveSet::SetGeoChop(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetGeoChop(s, secs);
        }
    }
}

void pyWaveSet::SetGeoAngleDev(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetGeoAngleDev(s, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// Texture wave parameters. Safe to twiddle any time or speed.
// The new settings take effect as new waves are spawned.

void pyWaveSet::SetTexMaxLength(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetTexMaxLength(s, secs);
        }
    }
}

void pyWaveSet::SetTexMinLength(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetTexMinLength(s, secs);
        }
    }
}

void pyWaveSet::SetTexAmpOverLen(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetTexAmpOverLen(s, secs);
        }
    }
}

void pyWaveSet::SetTexChop(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetTexChop(s, secs);
        }
    }
}

void pyWaveSet::SetTexAngleDev(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetTexAngleDev(s, secs);
        }
    }
}
// --------------------------------------------------------------------------------
// The size in feet of one tile of the ripple texture. If you change this (I don't 
// recommend it), you need to change it very slowly or it will look very stupid.

void pyWaveSet::SetRippleScale(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetRippleScale(s, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// The direction the wind is blowing (waves will be more or less perpindicular to wind dir).
// Change somewhat slowly, like over 30 seconds.

void pyWaveSet::SetWindDir(const pyVector3& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetWindDir(s.fVector, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// Change these gently, effect is immediate.

void pyWaveSet::SetSpecularNoise(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetSpecularNoise(s, secs);
        }
    }
}

void pyWaveSet::SetSpecularStart(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetSpecularStart(s, secs);
        }
    }
}

void pyWaveSet::SetSpecularEnd(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetSpecularEnd(s, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// Water Height is overriden if the ref object is animated.

void pyWaveSet::SetWaterHeight(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetWaterHeight(s, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// Water Offset and DepthFalloff are complicated, and not immediately interesting to animate.

void pyWaveSet::SetWaterOffset(const pyVector3& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetWaterOffset(s.fVector, secs);
        }
    }
}

void pyWaveSet::SetOpacOffset(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetOpacOffset(s, secs);
        }
    }
}

void pyWaveSet::SetReflOffset(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetReflOffset(s, secs);
        }
    }
}

void pyWaveSet::SetWaveOffset(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetWaveOffset(s, secs);
        }
    }
}

void pyWaveSet::SetDepthFalloff(const pyVector3& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetDepthFalloff(s.fVector, secs);
        }
    }
}

void pyWaveSet::SetOpacFalloff(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetOpacFalloff(s, secs);
        }
    }
}

void pyWaveSet::SetReflFalloff(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetReflFalloff(s, secs);
        }
    }
}

void pyWaveSet::SetWaveFalloff(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetWaveFalloff(s, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// Max and Min Atten aren't very interesting, and will probably go away.

void pyWaveSet::SetMaxAtten(const pyVector3& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetMaxAtten(s.fVector, secs);
        }
    }
}

void pyWaveSet::SetMinAtten(const pyVector3& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetMinAtten(s.fVector, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// Water colors, adjust slowly, effect is immediate.

void pyWaveSet::SetWaterTint(pyColor& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetWaterTint(s.getColor(), secs);
        }
    }
}

void pyWaveSet::SetWaterOpacity(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetWaterOpacity(s, secs);
        }
    }
}

void pyWaveSet::SetSpecularTint(pyColor& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetSpecularTint(s.getColor(), secs);
        }
    }
}

void pyWaveSet::SetSpecularMute(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetSpecularMute(s, secs);
        }
    }
}

// --------------------------------------------------------------------------------
// The environment map is essentially projected onto a sphere. Moving the center of
// the sphere north will move the reflections north, changing the radius of the
// sphere effects parallax in the obvious way.

void pyWaveSet::SetEnvCenter(const pyPoint3& s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetEnvCenter(s.fPoint, secs);
        }
    }
}

void pyWaveSet::SetEnvRadius(float s, float secs)
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            waveset->SetEnvRadius(s, secs);
        }
    }
}

// --------------------------------------------------------------------------------

// ================================================================================
// Get Functions
// ================================================================================

// --------------------------------------------------------------------------------
float pyWaveSet::GetGeoMaxLength() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetGeoMaxLength();
        }
    }

    return -1;
}

float pyWaveSet::GetGeoMinLength() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetGeoMinLength();
        }
    }

    return -1;
}

float pyWaveSet::GetGeoAmpOverLen() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetGeoAmpOverLen();
        }
    }

    return -1;
}

float pyWaveSet::GetGeoChop() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetGeoChop();
        }
    }

    return -1;
}

float pyWaveSet::GetGeoAngleDev() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetGeoAngleDev();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

float pyWaveSet::GetTexMaxLength() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetTexMaxLength();
        }
    }

    return -1;
}

float pyWaveSet::GetTexMinLength() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetTexMinLength();
        }
    }

    return -1;
}

float pyWaveSet::GetTexAmpOverLen() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetTexAmpOverLen();
        }
    }

    return -1;
}

float pyWaveSet::GetTexChop() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetTexChop();
        }
    }

    return -1;
}

float pyWaveSet::GetTexAngleDev() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetTexAngleDev();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

float pyWaveSet::GetRippleScale() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetRippleScale();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

PyObject* pyWaveSet::GetWindDir() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyVector3::New(waveset->GetWindDir());
        }
    }

    PYTHON_RETURN_NONE;
}

// --------------------------------------------------------------------------------

float pyWaveSet::GetSpecularNoise() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetSpecularNoise();
        }
    }

    return -1;
}

float pyWaveSet::GetSpecularStart() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetSpecularStart();
        }
    }

    return -1;
}

float pyWaveSet::GetSpecularEnd() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetSpecularEnd();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

float pyWaveSet::GetWaterHeight() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetWaterHeight();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

PyObject* pyWaveSet::GetWaterOffset() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyVector3::New(waveset->GetWaterOffset());
        }
    }

    PYTHON_RETURN_NONE;
}

float pyWaveSet::GetOpacOffset() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetOpacOffset();
        }
    }

    return -1;
}

float pyWaveSet::GetReflOffset() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetReflOffset();
        }
    }

    return -1;
}

float pyWaveSet::GetWaveOffset() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetWaveOffset();
        }
    }

    return -1;
}

PyObject* pyWaveSet::GetDepthFalloff() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyVector3::New(waveset->GetDepthFalloff());
        }
    }

    PYTHON_RETURN_NONE;
}

float pyWaveSet::GetOpacFalloff() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetOpacFalloff();
        }
    }

    return -1;
}

float pyWaveSet::GetReflFalloff() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetReflFalloff();
        }
    }

    return -1;
}

float pyWaveSet::GetWaveFalloff() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetWaveFalloff();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

PyObject* pyWaveSet::GetMaxAtten() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyVector3::New(waveset->GetMaxAtten());
        }
    }

    PYTHON_RETURN_NONE;
}

PyObject* pyWaveSet::GetMinAtten() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyVector3::New(waveset->GetMinAtten());
        }
    }

    PYTHON_RETURN_NONE;
}

// --------------------------------------------------------------------------------

PyObject* pyWaveSet::GetWaterTint() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyColor::New(waveset->GetWaterTint());
        }
    }

    PYTHON_RETURN_NONE;
}

float pyWaveSet::GetWaterOpacity() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetWaterOpacity();
        }
    }

    return -1;
}

PyObject* pyWaveSet::GetSpecularTint() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyColor::New(waveset->GetSpecularTint());
        }
    }

    PYTHON_RETURN_NONE;
}

float pyWaveSet::GetSpecularMute() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetSpecularMute();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

PyObject* pyWaveSet::GetEnvCenter() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return pyPoint3::New(waveset->GetEnvCenter());
        }
    }

    PYTHON_RETURN_NONE;
}

float pyWaveSet::GetEnvRadius() const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->GetEnvRadius();
        }
    }

    return -1;
}

// --------------------------------------------------------------------------------

void pyWaveSet::AddBuoy(const pyKey& soKey) const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->AddBuoy(soKey.getKey());
        }
    }
}

void pyWaveSet::RemoveBuoy(const pyKey& soKey) const
{
    if (fWaterKey)
    {
        plWaveSet7* waveset = plWaveSet7::ConvertNoRef(fWaterKey->ObjectIsLoaded());
        if (waveset)
        {
            return waveset->RemoveBuoy(soKey.getKey());
        }
    }
}
