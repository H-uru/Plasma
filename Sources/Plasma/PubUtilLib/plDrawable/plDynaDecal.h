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

#ifndef plDynaDecal_inc
#define plDynaDecal_inc

#include "hsGeometry3.h"

class plAuxSpan;

class plDynaDecalBin;

class plDecalVtxFormat
{
public:
    hsPoint3    fPos;
    hsVector3   fNorm;
    uint32_t      fDiffuse;
    uint32_t      fSpecular; // Not used anywhere, carried around everywhere.
    hsPoint3    fUVW[2];
};

// UVW[0] is the currently used UVW.
// UVW[1] is for the alpha hack (when necessary).
const uint8_t kDecalVtxFormat = 0x2; // Two UVW's, otherwise vanilla.


class plDynaDecal
{
public:
    enum
    {
        kFresh          = 0x1,
        kAttenColor     = 0x2,
        kVertexShader   = 0x4
    };
protected:

    // StartVtx and StartIdx are relative to the start of the data
    // owned by this decal's span, not relative to the start of the
    // underlying buffers.
    uint16_t      fStartVtx;
    uint16_t      fNumVerts;

    uint16_t      fStartIdx;
    uint16_t      fNumIdx;

    double      fBirth;
    float       fInitAtten;
    uint32_t    fFlags;

    plDecalVtxFormat*   fVtxBase; // Safe pointer, the buffer data will outlive this decal
    
    plAuxSpan*          fAuxSpan;

    friend class plDynaDecalMgr;
public:
    virtual ~plDynaDecal() { }

    virtual bool        Age(double t, float ramp, float decay, float life) = 0;
};

// No expansion
class plDynaSplot : public plDynaDecal
{
protected:

public:

    bool        Age(double t, float ramp, float decay, float life) override;
};

// Expands radially from center
class plDynaRipple : public plDynaDecal
{
public:

    bool        Age(double t, float ramp, float decay, float life) override;

    float fC1U;
    float fC2U;

    float fC1V;
    float fC2V;

};

// Expands in V from top (V=0), expands in U from center (U=0.5)
class plDynaWake : public plDynaDecal
{
public:

    bool        Age(double t, float ramp, float decay, float life) override;

    float fC1U;
    float fC2U;

    float fC1V;
    float fC2V;

};

// Scrolls in V, no change in U
class plDynaWave : public plDynaDecal
{
public:

    bool        Age(double t, float ramp, float decay, float life) override;

    float fScrollRate;
};

// About the same as a DynaRipple, but implemented with vertex/pixel shaders.
// Only useful with plWaveSet.
class plDynaRippleVS : public plDynaRipple
{
public:

    bool        Age(double t, float ramp, float decay, float life) override;
};

#endif // plDynaDecal_inc

