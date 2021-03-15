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

#include "HeadSpin.h"
#include "plCutter.h"
#include "plAccessSpan.h"
#include "hsFastMath.h"
#include "plAccessGeometry.h"

#include "hsStream.h"

// Test hack
#include "plDrawableSpans.h"
#include "plDrawableGenerator.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plDrawInterface.h"
#include "plScene/plSceneNode.h"
#include "plScene/plPageTreeMgr.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

void plCutter::Read(hsStream* stream, hsResMgr* mgr)
{
    plCreatable::Read(stream, mgr);

    fLengthU = stream->ReadLEFloat();
    fLengthV = stream->ReadLEFloat();
    fLengthW = stream->ReadLEFloat();
}

void plCutter::Write(hsStream* stream, hsResMgr* mgr)
{
    plCreatable::Write(stream, mgr);

    stream->WriteLEFloat(fLengthU);
    stream->WriteLEFloat(fLengthV);
    stream->WriteLEFloat(fLengthW);
}


void plCutter::Set(const hsPoint3& pos, const hsVector3& dir, const hsVector3& out, bool flip)
{
    hsVector3 du = dir % out;
    hsVector3 dv = out % du;
    hsVector3 dw = out;

    hsFastMath::NormalizeAppr(du);
    hsFastMath::NormalizeAppr(dv);
    hsFastMath::NormalizeAppr(dw);

    fBackDir = dw;

    if( flip )
        du = -du;

    fDirU = du / fLengthU;
    fDirV = dv / -fLengthV;
    fDirW = dw / fLengthW;

    du *= fLengthU * 0.5f;
    dv *= fLengthV * 0.5f;
    dw *= fLengthW * 0.5f;

    hsPoint3 corner = pos;
    corner += -du;
    corner += dv;
    corner += -dw;

    fDistU = corner.InnerProduct(fDirU);
    fDistV = corner.InnerProduct(fDirV);
    fDistW = corner.InnerProduct(fDirW);

    hsMatrix44 l2w;
    l2w.NotIdentity();
    int i;
    for( i = 0; i < 3; i++ )
    {
        l2w.fMap[i][0] = du[i];
        l2w.fMap[i][1] = dv[i];
        l2w.fMap[i][2] = dw[i];
        l2w.fMap[i][3] = pos[i];
    }
    l2w.fMap[3][0] = l2w.fMap[3][1] = l2w.fMap[3][2] = 0;
    l2w.fMap[3][3] = 1.f;


    hsPoint3 p;
    p.Set(1.f, 1.f, 1.f);
    fWorldBounds.Reset(&p);
    p.Set(-1.f, -1.f, -1.f);
    fWorldBounds.Union(&p);
    fWorldBounds.Transform(&l2w);

    fIsect.SetBounds(fWorldBounds);
}


inline void plCutter::ISetPosNorm(float parm, const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    dst.fPos = outVtx.fPos;
    dst.fPos += parm * (inVtx.fPos - outVtx.fPos);

    dst.fNorm = outVtx.fNorm;
    dst.fNorm += parm * (inVtx.fNorm - outVtx.fNorm);

    dst.fColor = outVtx.fColor;
    dst.fColor += parm * (inVtx.fColor - outVtx.fColor);
}

// A note on where the interpolation parameter is coming from.
// 
// For the lower cases, we're looking for the point where Dot(pos, fDir) - fDist = 0.
// Starting with p = outVtx + parm * (inVtx - outVtx) and Dot(p, fDir) == fDist, we get:
// parm = (fDist - Dot(fDir,outVtx.fPos)) / (Dot(fDir, invVtx.fPos) - Dot(fDir, outVtx.fPos))
//
// UVW = Dot(fDir, fPos) - fDist
// Dot(fDir, fPos) = UVW + fDist
// So:
// parm = (fDist - (outVtx.fUVW - fDist)) / ((invVtx.fUVW - fDist) - (outVtx.fUVW - fDist))
//      = -outVtx.fUVW / (inVtx.fUVW - outVtx.fUVW)
//      = outVtx.fUVW / (outVtx.fUVW - inVtx.fUVW)

inline void plCutter::ICutoutVtxLoU(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = outVtx.fUVW.fX / (outVtx.fUVW.fX - inVtx.fUVW.fX);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fX = 0;
    dst.fUVW.fY = outVtx.fUVW.fY + parm * (inVtx.fUVW.fY - outVtx.fUVW.fY);
    dst.fUVW.fZ = outVtx.fUVW.fZ + parm * (inVtx.fUVW.fZ - outVtx.fUVW.fZ);
}

inline void plCutter::ICutoutVtxLoV(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = outVtx.fUVW.fY / (outVtx.fUVW.fY - inVtx.fUVW.fY);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fX = outVtx.fUVW.fX + parm * (inVtx.fUVW.fX - outVtx.fUVW.fX);
    dst.fUVW.fY = 0;
    dst.fUVW.fZ = outVtx.fUVW.fZ + parm * (inVtx.fUVW.fZ - outVtx.fUVW.fZ);
}

inline void plCutter::ICutoutVtxLoW(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = outVtx.fUVW.fZ / (outVtx.fUVW.fZ - inVtx.fUVW.fZ);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fX = outVtx.fUVW.fX + parm * (inVtx.fUVW.fX - outVtx.fUVW.fX);
    dst.fUVW.fY = outVtx.fUVW.fY + parm * (inVtx.fUVW.fY - outVtx.fUVW.fY);
    dst.fUVW.fZ = 0;
}

// Now for the upper cases, we start with Dot(pos, fDir) - fDist = 1.f
// So parm = (fDist + 1.f - Dot(fDir, outVtx.fPos) / (Dot(fDir, invVtx.fPos) - Dot(fDir, outVtx.fPos))
// and doing the same substitution gets
// parm = (outVtx.fUVW - 1.f) / (outVtx.fUVW - inVtx.fUVW)
inline void plCutter::ICutoutVtxHiU(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = (outVtx.fUVW.fX - 1.f) / (outVtx.fUVW.fX - inVtx.fUVW.fX);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fX = 1.f;
    dst.fUVW.fY = outVtx.fUVW.fY + parm * (inVtx.fUVW.fY - outVtx.fUVW.fY);
    dst.fUVW.fZ = outVtx.fUVW.fZ + parm * (inVtx.fUVW.fZ - outVtx.fUVW.fZ);
}

inline void plCutter::ICutoutVtxHiV(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = (outVtx.fUVW.fY - 1.f) / (outVtx.fUVW.fY - inVtx.fUVW.fY);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fX = outVtx.fUVW.fX + parm * (inVtx.fUVW.fX - outVtx.fUVW.fX);
    dst.fUVW.fY = 1.f;
    dst.fUVW.fZ = outVtx.fUVW.fZ + parm * (inVtx.fUVW.fZ - outVtx.fUVW.fZ);
}

inline void plCutter::ICutoutVtxHiW(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = (outVtx.fUVW.fZ - 1.f) / (outVtx.fUVW.fZ - inVtx.fUVW.fZ);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fX = outVtx.fUVW.fX + parm * (inVtx.fUVW.fX - outVtx.fUVW.fX);
    dst.fUVW.fY = outVtx.fUVW.fY + parm * (inVtx.fUVW.fY - outVtx.fUVW.fY);
    dst.fUVW.fZ = 1.f;
}


// Now for the split down the middle cases, we start with Dot(pos, fDir) - fDist = 0.5f
// So parm = (fDist + 0.5f - Dot(fDir, outVtx.fPos) / (Dot(fDir, invVtx.fPos) - Dot(fDir, outVtx.fPos))
// and doing the same substitution gets
// parm = (outVtx.fUVW - 0.5f) / (outVtx.fUVW - inVtx.fUVW)
inline void plCutter::ICutoutVtxMidU(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = (outVtx.fUVW.fX - 0.5f) / (outVtx.fUVW.fX - inVtx.fUVW.fX);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fX = outVtx.fUVW.fX + parm * (inVtx.fUVW.fX - outVtx.fUVW.fX); // TEST

    dst.fUVW.fX = 0.5f;
    dst.fUVW.fY = outVtx.fUVW.fY + parm * (inVtx.fUVW.fY - outVtx.fUVW.fY);
    dst.fUVW.fZ = outVtx.fUVW.fZ + parm * (inVtx.fUVW.fZ - outVtx.fUVW.fZ);
}

inline void plCutter::ICutoutVtxMidV(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = (outVtx.fUVW.fY - 0.5f) / (outVtx.fUVW.fY - inVtx.fUVW.fY);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fY = outVtx.fUVW.fY + parm * (inVtx.fUVW.fY - outVtx.fUVW.fY); // TEST

    dst.fUVW.fX = outVtx.fUVW.fX + parm * (inVtx.fUVW.fX - outVtx.fUVW.fX);
    dst.fUVW.fY = 0.5f;
    dst.fUVW.fZ = outVtx.fUVW.fZ + parm * (inVtx.fUVW.fZ - outVtx.fUVW.fZ);
}

inline void plCutter::ICutoutVtxMidW(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const
{
    float parm = (outVtx.fUVW.fZ - 0.5f) / (outVtx.fUVW.fZ - inVtx.fUVW.fZ);

    ISetPosNorm(parm, inVtx, outVtx, dst);

    dst.fUVW.fZ = outVtx.fUVW.fZ + parm * (inVtx.fUVW.fZ - outVtx.fUVW.fZ); // TEST

    dst.fUVW.fX = outVtx.fUVW.fX + parm * (inVtx.fUVW.fX - outVtx.fUVW.fX);
    dst.fUVW.fY = outVtx.fUVW.fY + parm * (inVtx.fUVW.fY - outVtx.fUVW.fY);
    dst.fUVW.fZ = 0.5f;
}

// IPolyClip
bool plCutter::IPolyClip(hsTArray<plCutoutVtx>& poly, const hsPoint3 vPos[]) const
{
    static hsTArray<plCutoutVtx> accum;
    accum.SetCount(0);

    poly[0].fUVW.fX = vPos[0].InnerProduct(fDirU) - fDistU;
    poly[0].fUVW.fY = vPos[0].InnerProduct(fDirV) - fDistV;
    poly[0].fUVW.fZ = vPos[0].InnerProduct(fDirW) - fDistW;

    poly[1].fUVW.fX = vPos[1].InnerProduct(fDirU) - fDistU;
    poly[1].fUVW.fY = vPos[1].InnerProduct(fDirV) - fDistV;
    poly[1].fUVW.fZ = vPos[1].InnerProduct(fDirW) - fDistW;

    poly[2].fUVW.fX = vPos[2].InnerProduct(fDirU) - fDistU;
    poly[2].fUVW.fY = vPos[2].InnerProduct(fDirV) - fDistV;
    poly[2].fUVW.fZ = vPos[2].InnerProduct(fDirW) - fDistW;

    // Try an early out test.
    int i;
    for( i = 0; i < 3; i++ )
    {
        int lo = 1;
        int hi = 1;
        int j;
        for( j = 0; j < 3; j++ )
        {
            lo &= poly[j].fUVW[i] <= 0;
            hi &= poly[j].fUVW[i] >= 1.f;
        }
        if( lo || hi )
        {
            poly.SetCount(0);
            return false;
        }
    }


    // First trim to lower bounds.
    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fX < 0) << 1) | (poly[j].fUVW.fX < 0);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxLoU(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxLoU(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fY < 0) << 1) | (poly[j].fUVW.fY < 0);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxLoV(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxLoV(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fZ < 0) << 1) | (poly[j].fUVW.fZ < 0);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxLoW(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxLoW(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    // Now upper bounds
    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fX > 1.f) << 1) | (poly[j].fUVW.fX > 1.f);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxHiU(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxHiU(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fY > 1.f) << 1) | (poly[j].fUVW.fY > 1.f);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxHiV(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxHiV(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fZ > 1.f) << 1) | (poly[j].fUVW.fZ > 1.f);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxHiW(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxHiW(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    return poly.GetCount() > 2;
}

// IPolyClip
bool plCutter::IFindHitPoint(const hsTArray<plCutoutVtx>& inPoly, plCutoutHit& hit) const
{
    static hsTArray<plCutoutVtx> accum;
    static hsTArray<plCutoutVtx> poly;
    accum.SetCount(0);

    poly = inPoly;

    // First trim to lower bounds.
    int i;
    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fX < 0.5f) << 1) | (poly[j].fUVW.fX < 0.5f);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxMidU(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxMidU(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fY < 0.5f) << 1) | (poly[j].fUVW.fY < 0.5f);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxMidV(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxMidV(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    // Now upper bounds
    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fX > 0.5f) << 1) | (poly[j].fUVW.fX > 0.5f);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxMidU(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxMidU(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }
    poly.Swap(accum);
    accum.SetCount(0);

    for( i = 0; i < poly.GetCount(); i++ )
    {
        int j = i ? i-1 : poly.GetCount()-1;

        int test = ((poly[i].fUVW.fY > 0.5f) << 1) | (poly[j].fUVW.fY > 0.5f);
        switch(test)
        {
        case 0:
            // Both in
            // Add this vert to outList
            accum.Append(poly[i]);
            break;
        case 1:
            // This in, last out
            // Add ClipVert(j, j-1) to outList
            // Add this vert to outList
            accum.Push();
            ICutoutVtxMidV(poly[i], poly[j], accum[accum.GetCount()-1]);
            accum.Append(poly[i]);
            break;
        case 2:
            // This out, last in
            // Add ClipVert(j-1, j) to outList
            accum.Push();
            ICutoutVtxMidV(poly[j], poly[i], accum[accum.GetCount()-1]);
            break;
        case 3:
            // Both out
            break;
        }
    }

    // At this point, if we hit, all verts should be identical, interpolated
    // into the center of the cutter.
    // No verts means no hit.
    if( !accum.GetCount() )
        return false;

    if( accum[0].fNorm.InnerProduct(fDirW) < 0 )
        return false;

    hit.fPos = accum[0].fPos;
    hit.fNorm = accum[0].fNorm;

    return true;
}


bool plCutter::FindHitPoints(const std::vector<plCutoutPoly>& src, std::vector<plCutoutHit>& hits) const
{
    hits.clear();

    for (const plCutoutPoly& poly : src)
    {
        bool loU = false;
        bool hiU = false;
        bool loV = false;
        bool hiV = false;

        int iv;
        for( iv = 0; iv < poly.fVerts.GetCount(); iv++ )
        {
            const hsPoint3& uvw = poly.fVerts[iv].fUVW;
            if( uvw.fX < 0.5f )
                loU = true;
            else
                hiU = true;
            if( uvw.fY < 0.5f )
                loV = true;
            else
                hiV = true;

            if( loU && hiU && loV && hiV )
            {
                plCutoutHit hit;
                if( IFindHitPoint(poly.fVerts, hit) )
                    hits.emplace_back(hit);
                break;
            }
        }
    }

    return !hits.empty();
}

bool plCutter::FindHitPointsConstHeight(const std::vector<plCutoutPoly>& src, std::vector<plCutoutHit>& hits, float height) const
{
    if( FindHitPoints(src, hits) )
    {
        for (plCutoutHit& hit : hits)
            hit.fPos.fZ = height;
        
        return true;
    }

    return false;
}

void plCutter::ICutoutTransformedConstHeight(const plAccessSpan& src, std::vector<plCutoutPoly>& dst) const
{
    const hsMatrix44& l2w = src.GetLocalToWorld();
    hsMatrix44 l2wNorm;
    src.GetWorldToLocal().GetTranspose(&l2wNorm);

    bool baseHasAlpha = 0 != (src.GetMaterial()->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha);

    plAccTriIterator tri(&src.AccessTri());
    // For each tri
    for( tri.Begin(); tri.More(); tri.Advance() )
    {
        // Do a polygon clip of tri to box
        static hsTArray<plCutoutVtx> poly;
        poly.SetCount(3);

        // Not sure about this, whether the constant water height should be world space or local.
        // We'll leave it in local for now.
        const hsVector3 up(0.f, 0.f, 1.f);
        hsPoint3 vPos[3];
        vPos[0] = l2w * hsPoint3(tri.Position(0).fX, tri.Position(0).fY, src.GetWaterHeight());
        vPos[1] = l2w * hsPoint3(tri.Position(1).fX, tri.Position(1).fY, src.GetWaterHeight());
        vPos[2] = l2w * hsPoint3(tri.Position(2).fX, tri.Position(2).fY, src.GetWaterHeight());

        poly[0].Init(l2w * hsPoint3(tri.Position(0).fX, tri.Position(0).fY, tri.Position(0).fZ), l2wNorm * up, tri.DiffuseRGBA(0));
        poly[1].Init(l2w * hsPoint3(tri.Position(1).fX, tri.Position(1).fY, tri.Position(1).fZ), l2wNorm * up, tri.DiffuseRGBA(1));
        poly[2].Init(l2w * hsPoint3(tri.Position(2).fX, tri.Position(2).fY, tri.Position(2).fZ), l2wNorm * up, tri.DiffuseRGBA(2));

        // If we got a polygon
        if( IPolyClip(poly, vPos) )
        {
            // tessalate the polygon into dst
            IConstruct(dst, poly, baseHasAlpha);
        }
    }
}

// We usually don't need to do any transform, because the kind of surface you
// would leave prints on tends to be static, with the transform folded into the
// verts. So it's worth having 2 separate versions of the function.
void plCutter::ICutoutTransformed(const plAccessSpan& src, std::vector<plCutoutPoly>& dst) const
{
    const hsMatrix44& l2w = src.GetLocalToWorld();
    hsMatrix44 l2wNorm;
    src.GetWorldToLocal().GetTranspose(&l2wNorm);

    bool baseHasAlpha = 0 != (src.GetMaterial()->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha);

    plAccTriIterator tri(&src.AccessTri());
    // For each tri
    for( tri.Begin(); tri.More(); tri.Advance() )
    {
        // Do a polygon clip of tri to box
        static hsTArray<plCutoutVtx> poly;
        poly.SetCount(3);

        hsPoint3 vPos[3];
        vPos[0] = l2w * tri.Position(0);
        vPos[1] = l2w * tri.Position(1);
        vPos[2] = l2w * tri.Position(2);

        poly[0].Init(vPos[0], l2wNorm * tri.Normal(0), tri.DiffuseRGBA(0));
        poly[1].Init(vPos[1], l2wNorm * tri.Normal(1), tri.DiffuseRGBA(1));
        poly[2].Init(vPos[2], l2wNorm * tri.Normal(2), tri.DiffuseRGBA(2));

        // If we got a polygon
        if( IPolyClip(poly, vPos) )
        {
            // tessalate the polygon into dst
            IConstruct(dst, poly, baseHasAlpha);
        }
    }
}

void plCutter::ICutoutConstHeight(const plAccessSpan& src, std::vector<plCutoutPoly>& dst) const
{
    if( !(src.GetLocalToWorld().fFlags & hsMatrix44::kIsIdent) )
    {
        ICutoutTransformedConstHeight(src, dst);
        return;
    }

    bool baseHasAlpha = 0 != (src.GetMaterial()->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha);

    plAccTriIterator tri(&src.AccessTri());
    // For each tri
    for( tri.Begin(); tri.More(); tri.Advance() )
    {
        // Do a polygon clip of tri to box
        static hsTArray<plCutoutVtx> poly;
        poly.SetCount(3);

        const hsVector3 up(0.f, 0.f, 1.f);

        hsPoint3 vPos[3];
        vPos[0].Set(tri.Position(0).fX, tri.Position(0).fY, src.GetWaterHeight());
        vPos[1].Set(tri.Position(1).fX, tri.Position(1).fY, src.GetWaterHeight());
        vPos[2].Set(tri.Position(2).fX, tri.Position(2).fY, src.GetWaterHeight());

        poly[0].Init(hsPoint3(tri.Position(0).fX, tri.Position(0).fY, tri.Position(0).fZ), up, tri.DiffuseRGBA(0));
        poly[1].Init(hsPoint3(tri.Position(1).fX, tri.Position(1).fY, tri.Position(1).fZ), up, tri.DiffuseRGBA(1));
        poly[2].Init(hsPoint3(tri.Position(2).fX, tri.Position(2).fY, tri.Position(2).fZ), up, tri.DiffuseRGBA(2));

        // If we got a polygon
        if( IPolyClip(poly, vPos) )
        {
            // tessalate the polygon into dst
            IConstruct(dst, poly, baseHasAlpha);
        }
    }
}

// Cutout
void plCutter::Cutout(const plAccessSpan& src, std::vector<plCutoutPoly>& dst) const
{
    if( !src.HasAccessTri() )
        return;

    if( src.HasWaterHeight() )
    {
        ICutoutConstHeight(src, dst);
        return;
    }

    if( !(src.GetLocalToWorld().fFlags & hsMatrix44::kIsIdent) )
    {
        ICutoutTransformed(src, dst);
        return;
    }

    bool baseHasAlpha = 0 != (src.GetMaterial()->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha);

    plAccTriIterator tri(&src.AccessTri());
    // For each tri
    for( tri.Begin(); tri.More(); tri.Advance() )
    {
        // Do a polygon clip of tri to box
        static hsTArray<plCutoutVtx> poly;
        poly.SetCount(3);

        hsPoint3 vPos[3];
        vPos[0] = tri.Position(0);
        vPos[1] = tri.Position(1);
        vPos[2] = tri.Position(2);

        poly[0].Init(vPos[0], tri.Normal(0), tri.DiffuseRGBA(0));
        poly[1].Init(vPos[1], tri.Normal(1), tri.DiffuseRGBA(1));
        poly[2].Init(vPos[2], tri.Normal(2), tri.DiffuseRGBA(2));

        // If we got a polygon
        if( IPolyClip(poly, vPos) )
        {
            // tessalate the polygon into dst
            IConstruct(dst, poly, baseHasAlpha);
        }
    }
}

void plCutter::IConstruct(std::vector<plCutoutPoly>& dst, hsTArray<plCutoutVtx>& poly, bool baseHasAlpha) const
{
    plCutoutPoly& dstPoly = dst.emplace_back();
    dstPoly.fVerts.Swap(poly);
    dstPoly.fBaseHasAlpha = baseHasAlpha;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

bool plCutter::CutoutGrid(int nWid, int nLen, plFlatGridMesh& grid) const
{
    hsVector3 halfU = fDirU * (fLengthU * fLengthU * 0.5f);
    hsVector3 halfV = fDirV * (fLengthV * fLengthV * 0.5f);
    return MakeGrid(nWid, nLen, fWorldBounds.GetCenter(), halfU, halfV, grid);
}

bool plCutter::MakeGrid(int nWid, int nLen, const hsPoint3& center, const hsVector3& halfU, const hsVector3& halfV, plFlatGridMesh& grid)
{
    if( nWid < 3 )
        nWid = 3;
    if( !(nWid & 0x1) )
        nWid++;
    if( nLen < 3 )
        nLen = 3;
    if( !(nLen & 0x1) )
        nLen++;

    grid.fVerts.SetCount(nWid * nLen);

    hsVector3 dux = halfU;
    hsVector3 dvx = halfV;
    dux.fZ = 0;
    dvx.fZ = 0;

    hsPoint3 corner = center;
    corner.fZ = 0;
    corner += -dux;
    corner += -dvx;

    float sWid = 1.f / float(nWid-1);
    float sLen = 1.f / float(nLen-1);

    dux *= 2.f * sWid;
    dvx *= 2.f * sLen;

    float du = sWid;
    float dv = sLen;
    int j;
    for( j = 0; j < nLen; j++ )
    {
        int i;
        for( i = 0; i < nWid; i++ )
        {
            plCutoutMiniVtx& vtx = grid.fVerts[j * nWid + i];

            vtx.fPos = corner;
            vtx.fPos += dux * (float)i;
            vtx.fPos += dvx * (float)j;

            vtx.fUVW.fX = du * i;
            vtx.fUVW.fY = dv * j;
            vtx.fUVW.fZ = 0.5f;
        }
    }

    int idx = 0;
    grid.fIdx.SetCount(2 * (nWid-1) * (nLen-1) * 3);
    for( j = 1; j < nLen; )
    {
        int i;
        for( i = 1; i < nWid; )
        {
            grid.fIdx[idx++] =  j    * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + i;
            grid.fIdx[idx++] = (j-1) * nWid + (i-1);

            grid.fIdx[idx++] = (j-1) * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + i;
            grid.fIdx[idx++] = (j-1) * nWid + i;

            i++;

            grid.fIdx[idx++] = (j-1) * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + (i-1);
            grid.fIdx[idx++] = (j-1) * nWid + i;

            grid.fIdx[idx++] = (j-1) * nWid + i;
            grid.fIdx[idx++] =  j    * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + i;

            i++;
        }

        j++;

        for( i = 1; i < nWid; )
        {
            grid.fIdx[idx++] = (j-1) * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + (i-1);
            grid.fIdx[idx++] = (j-1) * nWid + i;

            grid.fIdx[idx++] = (j-1) * nWid + i;
            grid.fIdx[idx++] =  j    * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + i;

            i++;

            grid.fIdx[idx++] =  j    * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + i;
            grid.fIdx[idx++] = (j-1) * nWid + (i-1);

            grid.fIdx[idx++] = (j-1) * nWid + (i-1);
            grid.fIdx[idx++] =  j    * nWid + i;
            grid.fIdx[idx++] = (j-1) * nWid + i;

            i++;
        }
        
        j++;
    }

    return grid.fIdx.GetCount() > 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Test hackage ensues
/////////////////////////////////////////////////////////////////////////////////////////////////////
void TestCutter(const plKey& key, const hsVector3& size, const hsPoint3& pos)
{
    plCutter cutter;

    cutter.SetLength(size);

    static hsVector3 dir(0.f, 1.f, 0.f);
    static hsVector3 up(0.f, 0.f, 1.f);

    cutter.Set(pos, dir, up);

    plSceneObject* so = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
    if( !so )
        return;
    const plDrawInterface* di = so->GetDrawInterface();
    if( !di )
        return;

    static plDrawableSpans* drawable = nullptr;
    bool newDrawable = !drawable;
    bool haveNormal = true;

    std::vector<uint32_t> retIndex;

    std::vector<plAccessSpan> src;
    plAccessGeometry::Instance()->OpenRO(di, src);
    
    if (src.empty())
        return;

    for (const plAccessSpan& span : src)
    {

        static std::vector<plCutoutPoly> dst;
        dst.clear();
#if 1
        cutter.Cutout(span, dst);
#else
        hsPoint3 corner;
        hsVector3 ax[3];
        cutter.GetWorldBounds().GetCorner(&corner);
        cutter.GetWorldBounds().GetAxes(ax+0, ax+1, ax+2);
        int iAx = 0;
        int jAx = 1;
        dst.resize(6);
        int xx;
        for( xx = 0; xx < 3; xx++ )
        {
            dst[xx].fVerts.SetCount(4);
            
            dst[xx].fVerts[0].fPos = corner;
            dst[xx].fVerts[0].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[0].fUVW.Set(0,0,0);

            dst[xx].fVerts[1].fPos = corner;
            dst[xx].fVerts[1].fPos += ax[iAx];
            dst[xx].fVerts[1].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[1].fUVW.Set(1,0,0);

            dst[xx].fVerts[2].fPos = corner;
            dst[xx].fVerts[2].fPos += ax[iAx];
            dst[xx].fVerts[2].fPos += ax[jAx];
            dst[xx].fVerts[2].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[2].fUVW.Set(1.f,1.f,0);

            dst[xx].fVerts[3].fPos = corner;
            dst[xx].fVerts[3].fPos += ax[jAx];
            dst[xx].fVerts[3].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[3].fUVW.Set(0,1.f,0);

            iAx++;
            jAx = iAx > 1 ? 0 : iAx+1;
        }
        corner += ax[0];
        corner += ax[1];
        corner += ax[2];
        ax[0] = -ax[0];
        ax[1] = -ax[1];
        ax[2] = -ax[2];
        iAx = 0;
        jAx = 1;
        for( xx = 3; xx < 6; xx++ )
        {
            dst[xx].fVerts.SetCount(4);
            
            dst[xx].fVerts[0].fPos = corner;
            dst[xx].fVerts[0].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[0].fUVW.Set(0,0,0);

            dst[xx].fVerts[3].fPos = corner;
            dst[xx].fVerts[3].fPos += ax[iAx];
            dst[xx].fVerts[3].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[3].fUVW.Set(1.f,0,0);

            dst[xx].fVerts[2].fPos = corner;
            dst[xx].fVerts[2].fPos += ax[iAx];
            dst[xx].fVerts[2].fPos += ax[jAx];
            dst[xx].fVerts[2].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[2].fUVW.Set(1.f,1.f,0);

            dst[xx].fVerts[1].fPos = corner;
            dst[xx].fVerts[1].fPos += ax[jAx];
            dst[xx].fVerts[1].fNorm.Set(0,0,1.f);
            dst[xx].fVerts[1].fUVW.Set(0,1.f,0);

            iAx++;
            jAx = iAx > 1 ? 0 : iAx+1;
        }
        haveNormal = false;
#endif

        // What's our total number of verts?
        // Total number of tris?
        int numVerts = 0;
        int numTris = 0;
        for (const plCutoutPoly& poly : dst)
        {
            if (poly.fVerts.GetCount())
            {
                numVerts += poly.fVerts.GetCount();
                numTris += poly.fVerts.GetCount()-2;
            }
        }
        if( !numTris )
            continue;

        std::vector<hsPoint3> pos(numVerts);
        std::vector<hsVector3> norm(numVerts);
        std::vector<hsPoint3> uvw(numVerts);
        std::vector<hsColorRGBA> col(numVerts);

        int iPoly = 0;
        int iVert = 0;
        for (int iv = 0; iv < numVerts; iv++)
        {
            pos[iv] = dst[iPoly].fVerts[iVert].fPos;
            norm[iv] = dst[iPoly].fVerts[iVert].fNorm;
            uvw[iv] = dst[iPoly].fVerts[iVert].fUVW;
            col[iv] = dst[iPoly].fVerts[iVert].fColor;

            float opac = uvw[iv].fZ < 0.25f 
                ? uvw[iv].fZ * 4.f
                : uvw[iv].fZ > 0.75f
                    ? (1.f - uvw[iv].fZ) * 4.f
                    : 1.f;

            opac *= norm[iv].fZ;
            if( opac < 0 )
                opac = 0;

            if( dst[iPoly].fBaseHasAlpha )
                col[iv].a *= opac;
            else
                col[iv].a = opac;

            if( ++iVert >= dst[iPoly].fVerts.GetCount() )
            {
                iVert = 0;
                iPoly++;
            }
        }

        std::vector<uint16_t> idx;

        uint16_t base = 0;
        for (const plCutoutPoly& poly : dst)
        {
            uint16_t next = base+1;
            for (int k = 2; k < poly.fVerts.GetCount(); k++)
            {
                idx.emplace_back(base);
                idx.emplace_back(next++);
                idx.emplace_back(next);
            }
            base = ++next;
        }

        drawable = plDrawableGenerator::GenerateDrawable(numVerts, pos.data(),
                                                        haveNormal ? norm.data() : nullptr,
                                                        uvw.data(), 1,
                                                        col.data(),
                                                        true, 
                                                        nullptr,
                                                        idx.size(), idx.data(),
                                                        span.GetMaterial(),
                                                        hsMatrix44::IdentityMatrix(), 
                                                        true,
                                                        &retIndex, 
                                                        drawable);

    }

    if( drawable && newDrawable )
        drawable->SetSceneNode(so->GetSceneNode());

}

void TestCutter2(const plKey& key, const hsVector3& size, const hsPoint3& pos, bool flip)
{
    plCutter cutter;

    cutter.SetLength(size);

    static hsVector3 dir(0.f, 1.f, 0.f);
    static hsVector3 up(0.f, 0.f, 1.f);

    cutter.Set(pos, dir, up, flip);

    plSceneObject* so = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
    if( !so )
        return;

    plSceneNode* node = plSceneNode::ConvertNoRef(so->GetSceneNode()->ObjectIsLoaded());
    if( !node )
        return;

    static plDrawableSpans* drawable = nullptr;
    bool newDrawable = !drawable;
    bool haveNormal = true;

    std::vector<uint32_t> retIndex;

    std::vector<plDrawVisList> drawVis;
    node->Harvest(&cutter.GetIsect(), drawVis);
    if (drawVis.empty())
        return;

    hsTArray<plAccessSpan> src;

    size_t numSpan = 0;
    for (const plDrawVisList& dvList : drawVis)
        numSpan += dvList.fVisList.size();

    src.SetCount(numSpan);

    size_t iDraw = 0, iSpan = 0;
    for (size_t i = 0; i < numSpan; i++)
    {
        plAccessGeometry::Instance()->OpenRO(drawVis[iDraw].fDrawable, drawVis[iDraw].fVisList[iSpan], src[i]);

        if (++iSpan >= drawVis[iDraw].fVisList.size())
        {
            iDraw++;
            iSpan = 0;
        }
    }

    
    for (int i = 0; i < src.GetCount(); i++)
    {
        static std::vector<plCutoutPoly> dst;
        dst.clear();
        cutter.Cutout(src[i], dst);

        // What's our total number of verts?
        // Total number of tris?
        int numVerts = 0;
        int numTris = 0;
        for (const plCutoutPoly& poly : dst)
        {
            if (poly.fVerts.GetCount())
            {
                numVerts += poly.fVerts.GetCount();
                numTris += poly.fVerts.GetCount()-2;
            }
        }
        if( !numTris )
            continue;

        std::vector<hsPoint3> pos(numVerts);
        std::vector<hsVector3> norm(numVerts);
        std::vector<hsPoint3> uvw(numVerts);
        std::vector<hsColorRGBA> col(numVerts);

        int iPoly = 0;
        int iVert = 0;
        int iv;
        for( iv = 0; iv < numVerts; iv++ )
        {
            pos[iv] = dst[iPoly].fVerts[iVert].fPos;
            norm[iv] = dst[iPoly].fVerts[iVert].fNorm;
            uvw[iv] = dst[iPoly].fVerts[iVert].fUVW;
            col[iv] = dst[iPoly].fVerts[iVert].fColor;

            float opac = uvw[iv].fZ < 0.25f 
                ? uvw[iv].fZ * 4.f
                : uvw[iv].fZ > 0.75f
                    ? (1.f - uvw[iv].fZ) * 4.f
                    : 1.f;

            opac *= norm[iv].fZ;
            if( opac < 0 )
                opac = 0;

            if( dst[iPoly].fBaseHasAlpha )
                col[iv].a *= opac;
            else
                col[iv].a = opac;


            if( ++iVert >= dst[iPoly].fVerts.GetCount() )
            {
                iVert = 0;
                iPoly++;
            }
        }

        std::vector<uint16_t> idx;

        uint16_t base = 0;
        for (const plCutoutPoly& poly : dst)
        {
            uint16_t next = base+1;
            for (int k = 2; k < poly.fVerts.GetCount(); k++)
            {
                idx.emplace_back(base);
                idx.emplace_back(next++);
                idx.emplace_back(next);
            }
            base = ++next;
        }

        drawable = plDrawableGenerator::GenerateDrawable(numVerts, pos.data(),
                                                        haveNormal ? norm.data() : nullptr,
                                                        uvw.data(), 1,
                                                        col.data(),
                                                        false, 
                                                        nullptr,
                                                        idx.size(), idx.data(),
                                                        src[i].GetMaterial(), 
                                                        hsMatrix44::IdentityMatrix(), 
                                                        true,
                                                        &retIndex, 
                                                        drawable);

    }

    for (size_t i = 0; i < numSpan; i++)
    {
        plAccessGeometry::Instance()->Close(src[i]);
    }

    if( drawable && newDrawable )
        drawable->SetSceneNode(so->GetSceneNode());

}
