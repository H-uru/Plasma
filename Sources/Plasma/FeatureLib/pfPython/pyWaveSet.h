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
#ifndef pyWaveSet_h
#define pyWaveSet_h

#include "pyKey.h"
#include "pyGeometry3.h"
#include "pyColor.h"

#include <python.h>
#include "pyGlueHelpers.h"

//////////////////////////////////////////////////////////////////////
//
// pyWaveSet   - a wrapper class to provide interface to wave sets
//
//////////////////////////////////////////////////////////////////////

/*
// Getters and Setters for Python twiddling
	//
	// First a way to set new values. The secs parameter says how long to take
	// blending to the new value from the current value.
	//
	

	
	
	// Skipping the shore parameters, because they are never used.

	


	// Now a way to get current values. See the accompanying Setter for notes on
	// what the parameter means.
	//
	
	
	

	
	
	
 */

class pyWaveSet
{
private:
	plKey fWaterKey;

protected:
	pyWaveSet(): fWaterKey(nil) {} // for python glue only, do NOT call
	pyWaveSet(plKey key);
	pyWaveSet(pyKey& key);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptWaveSet);
	static PyObject *New(plKey key);
	static PyObject *New(pyKey& key);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyWaveSet object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyWaveSet); // converts a PyObject to a pyWaveSet (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	void setKey(pyKey& key) {fWaterKey = key.getKey();} // for python glue only, do NOT call

	// ==============================================================================
	// Set functions
	// ==============================================================================

	// Geometric wave parameters. These are all safe to twiddle at any time or speed.
	// The new settings take effect as new waves are spawned.
	
	void SetGeoMaxLength(hsScalar s, hsScalar secs=0);
	void SetGeoMinLength(hsScalar s, hsScalar secs=0);
	void SetGeoAmpOverLen(hsScalar s, hsScalar secs=0);
	void SetGeoChop(hsScalar s, hsScalar secs=0);
	void SetGeoAngleDev(hsScalar s, hsScalar secs=0);

	// Texture wave parameters. Safe to twiddle any time or speed.
	// The new settings take effect as new waves are spawned.

	void SetTexMaxLength(hsScalar s, hsScalar secs=0);
	void SetTexMinLength(hsScalar s, hsScalar secs=0);
	void SetTexAmpOverLen(hsScalar s, hsScalar secs=0);
	void SetTexChop(hsScalar s, hsScalar secs=0);
	void SetTexAngleDev(hsScalar s, hsScalar secs=0);

	// The size in feet of one tile of the ripple texture. If you change this (I don't 
	// recommend it), you need to change it very slowly or it will look very stupid.
	void SetRippleScale(hsScalar s, hsScalar secs=0);

	// The direction the wind is blowing (waves will be more or less perpindicular to wind dir).
	// Change somewhat slowly, like over 30 seconds.
	void SetWindDir(const pyVector3& s, hsScalar secs=0);

	// Change these gently, effect is immediate.
	void SetSpecularNoise(hsScalar s, hsScalar secs=0);
	void SetSpecularStart(hsScalar s, hsScalar secs=0);
	void SetSpecularEnd(hsScalar s, hsScalar secs=0);

	// Water Height is overriden if the ref object is animated.
	void SetWaterHeight(hsScalar s, hsScalar secs=0);

	// Water Offset and DepthFalloff are complicated, and not immediately interesting to animate.
	void SetWaterOffset(const pyVector3& s, hsScalar secs=0);
		void SetOpacOffset(hsScalar s, hsScalar secs=0);
		void SetReflOffset(hsScalar s, hsScalar secs=0);
		void SetWaveOffset(hsScalar s, hsScalar secs=0);
	void SetDepthFalloff(const pyVector3& s, hsScalar secs=0);
		void SetOpacFalloff(hsScalar s, hsScalar secs=0);
		void SetReflFalloff(hsScalar s, hsScalar secs=0);
		void SetWaveFalloff(hsScalar s, hsScalar secs=0);

	// Max and Min Atten aren't very interesting, and will probably go away.
	void SetMaxAtten(const pyVector3& s, hsScalar secs=0);
	void SetMinAtten(const pyVector3& s, hsScalar secs=0);

	// Water colors, adjust slowly, effect is immediate.
	void SetWaterTint(pyColor& s, hsScalar secs=0);
	void SetWaterOpacity(hsScalar s, hsScalar secs=0);
	void SetSpecularTint(pyColor& s, hsScalar secs=0);
	void SetSpecularMute(hsScalar s, hsScalar secs=0);

	// The environment map is essentially projected onto a sphere. Moving the center of
	// the sphere north will move the reflections north, changing the radius of the
	// sphere effects parallax in the obvious way.
	void SetEnvCenter(const pyPoint3& s, hsScalar secs=0);
	void SetEnvRadius(hsScalar s, hsScalar secs=0);

	// ==============================================================================
	// Get functions
	// ==============================================================================

	hsScalar GetGeoMaxLength() const;
	hsScalar GetGeoMinLength() const;
	hsScalar GetGeoAmpOverLen() const;
	hsScalar GetGeoChop() const;
	hsScalar GetGeoAngleDev() const;

	hsScalar GetTexMaxLength() const;
	hsScalar GetTexMinLength() const;
	hsScalar GetTexAmpOverLen() const;
	hsScalar GetTexChop() const;
	hsScalar GetTexAngleDev() const;

	hsScalar GetRippleScale() const;

	PyObject* GetWindDir() const; // returns pyVector3

	hsScalar GetSpecularNoise() const;
	hsScalar GetSpecularStart() const;
	hsScalar GetSpecularEnd() const;

	hsScalar GetWaterHeight() const;

	PyObject* GetWaterOffset() const; // returns pyVector3
		hsScalar GetOpacOffset() const;
		hsScalar GetReflOffset() const;
		hsScalar GetWaveOffset() const;
	PyObject* GetDepthFalloff() const; // returns pyVector3
		hsScalar GetOpacFalloff() const;
		hsScalar GetReflFalloff() const;
		hsScalar GetWaveFalloff() const;

	PyObject* GetMaxAtten() const; // returns pyVector3
	PyObject* GetMinAtten() const; // returns pyVector3

	PyObject* GetWaterTint() const; // returns pyColor
	hsScalar GetWaterOpacity() const;
	PyObject* GetSpecularTint() const; // returns pyColor
	hsScalar GetSpecularMute() const;

	PyObject* GetEnvCenter() const; // returns pyPoint3
	hsScalar GetEnvRadius() const;
};


#endif // pyWaveSet_h