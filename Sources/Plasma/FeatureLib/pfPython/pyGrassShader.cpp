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

#include "pyGrassShader.h"

#include "plSurface/plGrassShaderMod.h"

#include "pyGlueHelpers.h"
#include "pyKey.h"

pyGrassShader::pyGrassShader()
{
}

pyGrassShader::pyGrassShader(plKey key)
{
    fShaderKey = std::move(key);
}

pyGrassShader::pyGrassShader(pyKey& key)
{
    fShaderKey = key.getKey();
}

void pyGrassShader::SetKey(plKey key)
{
    fShaderKey = std::move(key);
}

//////////////////////////////////////////////////////////////////////
// Setter functions
//////////////////////////////////////////////////////////////////////

void pyGrassShader::SetWaveDistortion(int waveNum, const std::vector<float> & distortion)
{
    if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
    {
        PyErr_Format(PyExc_ValueError, "setWaveDistortion expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
        return;
    }
    if (distortion.size() != 3)
    {
        PyErr_SetString(PyExc_TypeError, "setWaveDistortion expects the direction to be a three-element tuple only");
        return;
    }
    if (fShaderKey)
    {
        plGrassShaderMod* shader = plGrassShaderMod::ConvertNoRef(fShaderKey->ObjectIsLoaded());
        if (shader)
        {
            shader->fWaves[waveNum].fDistX = distortion[0];
            shader->fWaves[waveNum].fDistY = distortion[1];
            shader->fWaves[waveNum].fDistZ = distortion[2];
            shader->RefreshWaves();
        }
    }
}

void pyGrassShader::SetWaveDirection(int waveNum, const std::vector<float> & direction)
{
    if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
    {
        PyErr_Format(PyExc_ValueError, "setWaveDirection expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
        return;
    }
    if (direction.size() != 2)
    {
        PyErr_SetString(PyExc_TypeError, "setWaveDirection expects the direction to be a two-element tuple only");
        return;
    }
    if (fShaderKey)
    {
        plGrassShaderMod* shader = plGrassShaderMod::ConvertNoRef(fShaderKey->ObjectIsLoaded());
        if (shader)
        {
            shader->fWaves[waveNum].fDirX = direction[0];
            shader->fWaves[waveNum].fDirY = direction[1];
            shader->RefreshWaves();
        }
    }
}

void pyGrassShader::SetWaveSpeed(int waveNum, float speed)
{
    if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
    {
        PyErr_Format(PyExc_ValueError, "setWaveSpeed expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
        return;
    }
    if (fShaderKey)
    {
        plGrassShaderMod* shader = plGrassShaderMod::ConvertNoRef(fShaderKey->ObjectIsLoaded());
        if (shader)
        {
            shader->fWaves[waveNum].fSpeed = speed;
            shader->RefreshWaves();
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Getter functions
//////////////////////////////////////////////////////////////////////

std::vector<float> pyGrassShader::GetWaveDistortion(int waveNum) const
{
    std::vector<float> retVal;
    retVal.push_back(-1);
    retVal.push_back(-1);
    retVal.push_back(-1);
    if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
    {
        PyErr_Format(PyExc_ValueError, "getWaveDistortion expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
        return retVal;
    }
    if (fShaderKey)
    {
        plGrassShaderMod* shader = plGrassShaderMod::ConvertNoRef(fShaderKey->ObjectIsLoaded());
        if (shader)
        {
            retVal[0] = shader->fWaves[waveNum].fDistX;
            retVal[1] = shader->fWaves[waveNum].fDistY;
            retVal[2] = shader->fWaves[waveNum].fDistZ;
            return retVal;
        }
    }
    return retVal;
}

std::vector<float> pyGrassShader::GetWaveDirection(int waveNum) const
{
    std::vector<float> retVal;
    retVal.push_back(-1);
    retVal.push_back(-1);
    if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
    {
        PyErr_Format(PyExc_ValueError, "getWaveDirection expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
        return retVal;
    }
    if (fShaderKey)
    {
        plGrassShaderMod* shader = plGrassShaderMod::ConvertNoRef(fShaderKey->ObjectIsLoaded());
        if (shader)
        {
            retVal[0] = shader->fWaves[waveNum].fDirX;
            retVal[1] = shader->fWaves[waveNum].fDirY;
            return retVal;
        }
    }
    return retVal;
}

float pyGrassShader::GetWaveSpeed(int waveNum) const
{
    if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
    {
        PyErr_Format(PyExc_ValueError, "getWaveSpeed expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
        return -1;
    }
    if (fShaderKey)
    {
        plGrassShaderMod* shader = plGrassShaderMod::ConvertNoRef(fShaderKey->ObjectIsLoaded());
        if (shader)
            return shader->fWaves[waveNum].fSpeed;
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////
// Other functions
//////////////////////////////////////////////////////////////////////

void pyGrassShader::ResetWaves()
{
    if (fShaderKey)
    {
        plGrassShaderMod* shader = plGrassShaderMod::ConvertNoRef(fShaderKey->ObjectIsLoaded());
        if (shader)
            shader->ResetWaves();
    }
}
