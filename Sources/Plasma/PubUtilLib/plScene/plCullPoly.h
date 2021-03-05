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

#ifndef plCullPoly_inc
#define plCullPoly_inc

#include <vector>

#include "hsBitVector.h"
#include "hsGeometry3.h"

struct hsMatrix44;
class hsStream;
class hsResMgr;

const float          kCullPolyDegen = 1.e-4f;

class plCullPoly
{
public:
    enum {
        kNone       = 0x0,
        kHole       = 0x1,
        kTwoSided   = 0x2
    };

    uint32_t                  fFlags;
    mutable hsBitVector     fClipped; // fClipped[i] => edge(fVerts[i], fVerts[(i+1)%n])

    std::vector<hsPoint3>   fVerts;
    hsVector3               fNorm;
    float                fDist;
    hsPoint3                fCenter;
    float                fRadius;

    const hsPoint3&         GetCenter() const { return fCenter; }
    float                GetRadius() const { return fRadius; }

    void                    SetHole(bool on) { if( on )fFlags |= kHole; else fFlags &= ~kHole; }
    void                    SetTwoSided(bool on) { if( on )fFlags |= kTwoSided; else fFlags &= ~kTwoSided; }

    bool                    IsHole() const { return fFlags & kHole; } // Assumes kHole is 0x1
    bool                    IsTwoSided() const { return 0 != (fFlags & kTwoSided); }

    plCullPoly& Init(const plCullPoly& p)
    {
        fClipped.Clear();
        fVerts.clear();
        fFlags = p.fFlags;
        fNorm = p.fNorm;
        fDist = p.fDist;
        fCenter = p.fCenter;
        return *this;
    }

    plCullPoly&             Flip(const plCullPoly& p);
    plCullPoly&             InitFromVerts(uint32_t f=kNone);
    float                ICalcRadius() const;

    plCullPoly&             Transform(const hsMatrix44& l2w, const hsMatrix44& w2l, plCullPoly& dst) const;

    void                    Read(hsStream* s, hsResMgr* mgr);
    void                    Write(hsStream* s, hsResMgr* mgr) const;

    bool                    DegenerateVert(const hsPoint3& p) const
    {
        if (!fVerts.empty())
            return (kCullPolyDegen > hsVector3(&p, &fVerts.back()).MagnitudeSquared());
        return false;
    }

    bool                    Validate() const; // no-op, except for special debugging circumstances.
};

#endif // plCullPoly_inc
