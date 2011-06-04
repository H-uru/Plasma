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
#include "pyGrassShader.h"
#include "../plSurface/plGrassShaderMod.h"

pyGrassShader::pyGrassShader()
{
	fShaderKey = nil;
}

pyGrassShader::pyGrassShader(plKey key)
{
	fShaderKey = key;
}

pyGrassShader::pyGrassShader(pyKey& key)
{
	fShaderKey = key.getKey();
}

void pyGrassShader::SetKey(plKey key)
{
	fShaderKey = key;
}

//////////////////////////////////////////////////////////////////////
// Setter functions
//////////////////////////////////////////////////////////////////////

void pyGrassShader::SetWaveDistortion(int waveNum, const std::vector<hsScalar> & distortion)
{
	if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
	{
		char errmsg[256];
		sprintf(errmsg,"setWaveDistortion expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
		PyErr_SetString(PyExc_ValueError, errmsg);
		return;
	}
	if (distortion.size() != 3)
	{
		char errmsg[256];
		sprintf(errmsg,"setWaveDistortion expects the direction to be a three-element tuple only");
		PyErr_SetString(PyExc_TypeError, errmsg);
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

void pyGrassShader::SetWaveDirection(int waveNum, const std::vector<hsScalar> & direction)
{
	if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
	{
		char errmsg[256];
		sprintf(errmsg,"setWaveDirection expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
		PyErr_SetString(PyExc_ValueError, errmsg);
		return;
	}
	if (direction.size() != 2)
	{
		char errmsg[256];
		sprintf(errmsg,"setWaveDirection expects the direction to be a two-element tuple only");
		PyErr_SetString(PyExc_TypeError, errmsg);
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

void pyGrassShader::SetWaveSpeed(int waveNum, hsScalar speed)
{
	if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
	{
		char errmsg[256];
		sprintf(errmsg,"setWaveSpeed expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
		PyErr_SetString(PyExc_ValueError, errmsg);
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

std::vector<hsScalar> pyGrassShader::GetWaveDistortion(int waveNum) const
{
	std::vector<hsScalar> retVal;
	retVal.push_back(-1);
	retVal.push_back(-1);
	retVal.push_back(-1);
	if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
	{
		char errmsg[256];
		sprintf(errmsg,"getWaveDistortion expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
		PyErr_SetString(PyExc_ValueError, errmsg);
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

std::vector<hsScalar> pyGrassShader::GetWaveDirection(int waveNum) const
{
	std::vector<hsScalar> retVal;
	retVal.push_back(-1);
	retVal.push_back(-1);
	if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
	{
		char errmsg[256];
		sprintf(errmsg,"getWaveDirection expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
		PyErr_SetString(PyExc_ValueError, errmsg);
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

hsScalar pyGrassShader::GetWaveSpeed(int waveNum) const
{
	if ((waveNum < 0)||(waveNum >= plGrassShaderMod::kNumWaves))
	{
		char errmsg[256];
		sprintf(errmsg,"getWaveSpeed expects the waveNum to be between 0 and %d (inclusive)", plGrassShaderMod::kNumWaves - 1);
		PyErr_SetString(PyExc_ValueError, errmsg);
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
