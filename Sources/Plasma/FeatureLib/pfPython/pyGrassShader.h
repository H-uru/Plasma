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
#ifndef pyGrassShader_h
#define pyGrassShader_h

#include "hsStlUtils.h"

#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"


//////////////////////////////////////////////////////////////////////
//
// pyGrassShader   - a wrapper class to provide interface to the grass
//					 shader
//
//////////////////////////////////////////////////////////////////////

class pyGrassShader
{
private:
	plKey fShaderKey;

protected:
	pyGrassShader();
	pyGrassShader(plKey key);
	pyGrassShader(pyKey& key);
public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGrassShader);
	static PyObject *New(plKey key);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGrassShader object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGrassShader); // converts a PyObject to a pyGrassShader (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	void SetKey(plKey key);

	void SetWaveDistortion(int waveNum, const std::vector<hsScalar> & distortion);
	void SetWaveDirection(int waveNum, const std::vector<hsScalar> & direction);
	void SetWaveSpeed(int waveNum, hsScalar speed);

	std::vector<hsScalar> GetWaveDistortion(int waveNum) const;
	std::vector<hsScalar> GetWaveDirection(int waveNum) const;
	hsScalar GetWaveSpeed(int waveNum) const;

	void ResetWaves();
};


#endif // pyGrassShader_h
