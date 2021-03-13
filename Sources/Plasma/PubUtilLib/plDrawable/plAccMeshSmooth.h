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

#ifndef plAccMeshSmooth_inc
#define plAccMeshSmooth_inc

#include <vector>

#include "plAccessGeometry.h"
#include "plAccessSpan.h"

struct hsPoint3;
struct hsVector3;
class plGeometrySpan;

class plAccMeshSmooth
{
public:
    enum {
        kNone               = 0x0,
        kSmoothNorm         = 0x1,
        kSmoothPos          = 0x2,
        kSmoothDiffuse      = 0x4
    };

protected:
    struct VtxAccum
    {
        hsPoint3        fPos;
        hsVector3       fNorm;
        hsColorRGBA     fDiffuse;
    };

    uint32_t          fFlags;

    float        fMinNormDot;
    float        fDistTolSq;

    plAccessGeometry            fAccGeom;
    std::vector<plAccessSpan>   fSpans;

    hsPoint3        IPositionToWorld(plAccessSpan& span, int i) const;
    hsVector3       INormalToWorld(plAccessSpan& span, int i) const;
    hsPoint3        IPositionToLocal(plAccessSpan& span, const hsPoint3& wPos) const;
    hsVector3       INormalToLocal(plAccessSpan& span, const hsVector3& wNorm) const;

    void            FindEdges(uint32_t maxVtxIdx, uint32_t nTris, uint16_t* idxList, std::vector<uint16_t>& edgeVerts);
    void            FindEdges(std::vector<plGeometrySpan*>& sets, std::vector<uint16_t>* edgeVerts);
    void            FindSharedVerts(plAccessSpan& span, size_t numEdgeVerts, std::vector<uint16_t>& edgeVerts, std::vector<uint16_t>& shareVtx, VtxAccum& accum);
    void            SetNormals(plAccessSpan& span, std::vector<uint16_t>& shareVtx, const hsVector3& norm) const;
    void            SetPositions(plAccessSpan& span, std::vector<uint16_t>& shareVtx, const hsPoint3& pos) const;
    void            SetDiffuse(plAccessSpan& span, std::vector<uint16_t>& shareVtx, const hsColorRGBA& diff) const;

public:
    plAccMeshSmooth() : fFlags(kSmoothNorm), fMinNormDot(0.25f), fDistTolSq(1.e-4f), fAccGeom() {}

    void        SetAngle(float degs);
    float    GetAngle() const; // returns degrees

    void        SetDistTol(float dist);
    float    GetDistTol() const;

    void        Smooth(std::vector<plGeometrySpan*>& sets);

    void        SetFlags(uint32_t f) { fFlags = f; }
    uint32_t      GetFlags() const { return fFlags; }
};

#endif // plAccMeshSmooth_inc
