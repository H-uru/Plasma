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
#ifndef pyWaveSet_h
#define pyWaveSet_h

#include "pyGlueHelpers.h"
#include "pnKeyedObject/plKey.h"

class pyColor;
class pyKey;
class pyPoint3;
class pyVector3;

//////////////////////////////////////////////////////////////////////
//
// pyWaveSet   - a wrapper class to provide interface to wave sets
//
//////////////////////////////////////////////////////////////////////
class pyWaveSet
{
private:
    plKey fWaterKey;

protected:
    pyWaveSet() = default; // for python glue only, do NOT call
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
    
    void SetGeoMaxLength(float s, float secs=0);
    void SetGeoMinLength(float s, float secs=0);
    void SetGeoAmpOverLen(float s, float secs=0);
    void SetGeoChop(float s, float secs=0);
    void SetGeoAngleDev(float s, float secs=0);

    // Texture wave parameters. Safe to twiddle any time or speed.
    // The new settings take effect as new waves are spawned.

    void SetTexMaxLength(float s, float secs=0);
    void SetTexMinLength(float s, float secs=0);
    void SetTexAmpOverLen(float s, float secs=0);
    void SetTexChop(float s, float secs=0);
    void SetTexAngleDev(float s, float secs=0);

    // The size in feet of one tile of the ripple texture. If you change this (I don't 
    // recommend it), you need to change it very slowly or it will look very stupid.
    void SetRippleScale(float s, float secs=0);

    // The direction the wind is blowing (waves will be more or less perpindicular to wind dir).
    // Change somewhat slowly, like over 30 seconds.
    void SetWindDir(const pyVector3& s, float secs=0);

    // Change these gently, effect is immediate.
    void SetSpecularNoise(float s, float secs=0);
    void SetSpecularStart(float s, float secs=0);
    void SetSpecularEnd(float s, float secs=0);

    // Water Height is overriden if the ref object is animated.
    void SetWaterHeight(float s, float secs=0);

    // Water Offset and DepthFalloff are complicated, and not immediately interesting to animate.
    void SetWaterOffset(const pyVector3& s, float secs=0);
        void SetOpacOffset(float s, float secs=0);
        void SetReflOffset(float s, float secs=0);
        void SetWaveOffset(float s, float secs=0);
    void SetDepthFalloff(const pyVector3& s, float secs=0);
        void SetOpacFalloff(float s, float secs=0);
        void SetReflFalloff(float s, float secs=0);
        void SetWaveFalloff(float s, float secs=0);

    // Max and Min Atten aren't very interesting, and will probably go away.
    void SetMaxAtten(const pyVector3& s, float secs=0);
    void SetMinAtten(const pyVector3& s, float secs=0);

    // Water colors, adjust slowly, effect is immediate.
    void SetWaterTint(pyColor& s, float secs=0);
    void SetWaterOpacity(float s, float secs=0);
    void SetSpecularTint(pyColor& s, float secs=0);
    void SetSpecularMute(float s, float secs=0);

    // The environment map is essentially projected onto a sphere. Moving the center of
    // the sphere north will move the reflections north, changing the radius of the
    // sphere effects parallax in the obvious way.
    void SetEnvCenter(const pyPoint3& s, float secs=0);
    void SetEnvRadius(float s, float secs=0);

    // ==============================================================================
    // Get functions
    // ==============================================================================

    float GetGeoMaxLength() const;
    float GetGeoMinLength() const;
    float GetGeoAmpOverLen() const;
    float GetGeoChop() const;
    float GetGeoAngleDev() const;

    float GetTexMaxLength() const;
    float GetTexMinLength() const;
    float GetTexAmpOverLen() const;
    float GetTexChop() const;
    float GetTexAngleDev() const;

    float GetRippleScale() const;

    PyObject* GetWindDir() const; // returns pyVector3

    float GetSpecularNoise() const;
    float GetSpecularStart() const;
    float GetSpecularEnd() const;

    float GetWaterHeight() const;

    PyObject* GetWaterOffset() const; // returns pyVector3
        float GetOpacOffset() const;
        float GetReflOffset() const;
        float GetWaveOffset() const;
    PyObject* GetDepthFalloff() const; // returns pyVector3
        float GetOpacFalloff() const;
        float GetReflFalloff() const;
        float GetWaveFalloff() const;

    PyObject* GetMaxAtten() const; // returns pyVector3
    PyObject* GetMinAtten() const; // returns pyVector3

    PyObject* GetWaterTint() const; // returns pyColor
    float GetWaterOpacity() const;
    PyObject* GetSpecularTint() const; // returns pyColor
    float GetSpecularMute() const;

    PyObject* GetEnvCenter() const; // returns pyPoint3
    float GetEnvRadius() const;

    // ==============================================================================
    // Buoy functions
    // ==============================================================================

    void AddBuoy(const pyKey& soKey) const;
    void RemoveBuoy(const pyKey& soKey) const;
};


#endif // pyWaveSet_h
