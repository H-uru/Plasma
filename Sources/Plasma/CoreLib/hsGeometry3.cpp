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
#include "hsGeometry3.h"
#include "hsStream.h"

hsVector3 operator%(const hsVector3& t, const hsVector3& s)
{
    hsVector3       result;

    return *result.Set( hsScalarMul(t.fY, s.fZ) - hsScalarMul(s.fY, t.fZ),
                    -hsScalarMul(t.fX, s.fZ) + hsScalarMul(s.fX, t.fZ),
                    hsScalarMul(t.fX, s.fY) - hsScalarMul(s.fX, t.fY));
}




//////////////////////////////////
/////////////////////////////////
#if HS_SCALAR_IS_FIXED
hsScalar hsScalarTriple::Magnitude() const
{
    hsWide  result, temp;

    result.Mul(fCoord[0], fCoord[0]);
    temp.Mul(fCoord[1], fCoord[1]);
    result.Add(&temp);
    temp.Mul(fCoord[2], fCoord[2]);
    result.Add(&temp);
    
    return result.Sqrt();
}

hsScalar hsScalarTriple::MagnitudeSquared() const
{
    hsWide  result, temp;

    result.Mul(fCoord[0], fCoord[0]);
    temp.Mul(fCoord[1], fCoord[1]);
    result.Add(&temp);
    temp.Mul(fCoord[2], fCoord[2]);
    result.Add(&temp);
    
    return result.AsFixed();
}
#endif

void hsScalarTriple::Read(hsStream *stream)
{
    
    // DANGER for speed read directly into these variables...ASSUMES fX,fY, and fZ are in contiguous order (PBG)
    stream->Read12Bytes(&fX);
}

void hsScalarTriple::Write(hsStream *stream) const
{
    stream->WriteLEScalar(fX);
    stream->WriteLEScalar(fY);
    stream->WriteLEScalar(fZ);
}

hsPlane3::hsPlane3(const hsPoint3* pt1, const hsPoint3* pt2, const hsPoint3* pt3)
{
    // convert into a point with two vectors
    hsVector3 v1(pt2, pt1);
    hsVector3 v2(pt3, pt1);

    // calculate the normal (wedge)
    fN.fX = (v1.fY * v2.fZ) - (v2.fY * v1.fZ);
    fN.fY = (v1.fZ * v2.fX) - (v2.fZ * v1.fX);
    fN.fZ = (v1.fX * v2.fY) - (v2.fX * v1.fY);
    fN.Normalize();

    fD = -pt1->InnerProduct(&fN);
}


void hsPlane3::Read(hsStream *stream) 
{ 
    fN.Read(stream); 
    fD=stream->ReadLEScalar(); 
}

void hsPlane3::Write(hsStream *stream) const 
{ 
    fN.Write(stream); 
    stream->WriteLEScalar(fD); 
}
