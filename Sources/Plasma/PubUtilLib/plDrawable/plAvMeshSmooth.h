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

#ifndef plAvMeshSmooth_inc
#define plAvMeshSmooth_inc

#include <vector>

#include "plAccessGeometry.h"
#include "plAccessSpan.h"

class plGeometrySpan;
struct hsPoint3;
struct hsVector3;

class plAvMeshSmooth
{
public:
    enum {
        kNone               = 0x0,
        kSmoothNorm         = 0x1,
        kSmoothPos          = 0x2,
        kSmoothDiffuse      = 0x4
    };

    class XfmSpan
    {
    public:
        plGeometrySpan*     fSpan;
        hsMatrix44          fSpanToNeutral;
        hsMatrix44          fNormSpanToNeutral; // == Transpose(Inverse(fSpanToNeutral)) == Transpose(fNeutralToSpan)

        hsMatrix44          fNeutralToSpan;
        hsMatrix44          fNormNeutralToSpan; // == Transpose(Inverse(fNeutralToSpan)) == Transpose(fSpanToNeutral)

        plAccessSpan        fAccSpan;
    };

protected:
    uint32_t          fFlags;

    float        fMinNormDot;
    float        fDistTolSq;

    plAccessGeometry            fAccGeom;

    hsPoint3        IPositionToNeutral(XfmSpan& span, int i) const;
    hsVector3       INormalToNeutral(XfmSpan& span, int i) const;
    hsPoint3        IPositionToSpan(XfmSpan& span, const hsPoint3& wPos) const;
    hsVector3       INormalToSpan(XfmSpan& span, const hsVector3& wNorm) const;

    void            FindEdges(uint32_t maxVtxIdx, uint32_t nTris, uint16_t* idxList, std::vector<uint16_t>& edgeVerts);
    void            FindEdges(std::vector<XfmSpan>& spans, std::vector<uint16_t>* edgeVerts);

public:
    plAvMeshSmooth() : fFlags(kSmoothNorm), fMinNormDot(0.25f), fDistTolSq(1.e-4f), fAccGeom() {}

    void        SetAngle(float degs);
    float    GetAngle() const; // returns degrees

    void        SetDistTol(float dist);
    float    GetDistTol() const;

    void        Smooth(std::vector<XfmSpan>& srcSpans, std::vector<XfmSpan>& dstSpans);

    void        SetFlags(uint32_t f) { fFlags = f; }
    uint32_t      GetFlags() const { return fFlags; }
};

#endif // plAvMeshSmooth_inc
