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

#include "plCullPoly.h"

#include "HeadSpin.h"
#include "hsFastMath.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsStream.h"

plCullPoly& plCullPoly::InitFromVerts(uint32_t f)
{
    fFlags = f;

    hsAssert(fVerts.size() > 2, "Initializing from degenerate poly");

    hsVector3 a(&fVerts[1], &fVerts[0]);
    hsVector3 b(&fVerts[2], &fVerts[0]);
    fNorm = a % b;
    hsFastMath::Normalize(fNorm);
    fDist = -(fNorm.InnerProduct(fVerts[0]));

    fCenter.Set(0,0,0);
    for (const hsPoint3& vert : fVerts)
    {
        fCenter += vert;
    }
    fCenter *= 1.f / float(fVerts.size());

    fRadius = ICalcRadius();

    return *this;
}

float plCullPoly::ICalcRadius() const
{
    float radSq = 0;
    for (const hsPoint3& vert : fVerts)
    {
        float tmpSq = hsVector3(&vert, &fCenter).MagnitudeSquared();
        if( tmpSq > radSq )
            radSq = tmpSq;
    }
    return radSq * hsFastMath::InvSqrt(radSq);
}

plCullPoly& plCullPoly::Flip(const plCullPoly& p)
{
    fFlags = p.fFlags;

    fNorm = -p.fNorm;
    fDist = -p.fDist;
    fCenter = p.fCenter;
    fRadius = p.fRadius;

    fVerts = p.fVerts;
    std::reverse(fVerts.begin(), fVerts.end());

    return *this;
}

plCullPoly& plCullPoly::Transform(const hsMatrix44& l2w, const hsMatrix44& w2l, plCullPoly& dst) const
{
    hsMatrix44 tpose;
    w2l.GetTranspose(&tpose);

    dst.fFlags = fFlags;

    dst.fVerts.resize(fVerts.size());

    for (size_t i = 0; i < fVerts.size(); i++)
    {
        dst.fVerts[i] = l2w * fVerts[i];
    }
    dst.fCenter = l2w * fCenter;

    dst.fNorm = tpose * fNorm;
    
    dst.fDist = -(dst.fNorm .InnerProduct(dst.fVerts[0]));

    ICalcRadius();

    return dst;
}

void plCullPoly::Read(hsStream* s, hsResMgr* mgr)
{
    fFlags = s->ReadLE32();

    fNorm.Read(s);
    fDist = s->ReadLEFloat();
    fCenter.Read(s);

    fRadius = s->ReadLEFloat();

    uint32_t n = s->ReadLE32();
    fVerts.resize(n);
    for (uint32_t i = 0; i < n; i++)
        fVerts[i].Read(s);
}

void plCullPoly::Write(hsStream* s, hsResMgr* mgr) const
{
    s->WriteLE32(fFlags);

    fNorm.Write(s);
    s->WriteLEFloat(fDist);
    fCenter.Write(s);

    s->WriteLEFloat(fRadius);

    s->WriteLE32((uint32_t)fVerts.size());
    for (const hsPoint3& vert : fVerts)
        vert.Write(s);
}

#ifdef HS_DEBUGGING
#define MF_VALIDATE_POLYS
#endif // HS_DEBUGGING

#ifdef MF_VALIDATE_POLYS
bool plCullPoly::Validate() const
{
    const float kMinMag = 1.e-8f;
    float magSq = fNorm.MagnitudeSquared();
    if( magSq < kMinMag )
        return false;
    if (fVerts.size() < 3)
        return false;
    hsVector3 norm = hsVector3(&fVerts[1], &fVerts[0]) % hsVector3(&fVerts[2], &fVerts[0]);
    magSq = norm.MagnitudeSquared();
    if( magSq < kMinMag )
        return false;
    norm *= hsFastMath::InvSqrtAppr(magSq);
    for (size_t i = 3; i < fVerts.size(); i++)
    {
        hsVector3 nextNorm = hsVector3(&fVerts[i-1], &fVerts[0]) % hsVector3(&fVerts[i], &fVerts[0]);
        magSq = nextNorm.MagnitudeSquared();
        if( magSq < kMinMag )
            return false;
        nextNorm *= hsFastMath::InvSqrtAppr(magSq);
        if( nextNorm.InnerProduct(norm) < kMinMag )
            return false;
    }
    return true;
}
#else // MF_VALIDATE_POLYS
bool plCullPoly::Validate() const
{
    return true;
}
#endif // MF_VALIDATE_POLYS

