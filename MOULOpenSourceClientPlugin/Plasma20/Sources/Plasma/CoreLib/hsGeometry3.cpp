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
#include "hsGeometry3.h"
#include "hsStream.h"

hsVector3 operator%(const hsVector3& t, const hsVector3& s)
{
	hsVector3		result;

	return *result.Set(	hsScalarMul(t.fY, s.fZ) - hsScalarMul(s.fY, t.fZ),
					-hsScalarMul(t.fX, s.fZ) + hsScalarMul(s.fX, t.fZ),
					hsScalarMul(t.fX, s.fY) - hsScalarMul(s.fX, t.fY));
}




//////////////////////////////////
/////////////////////////////////
#if HS_SCALAR_IS_FIXED
hsScalar hsScalarTriple::Magnitude() const
{
	hsWide	result, temp;

	result.Mul(fCoord[0], fCoord[0]);
	temp.Mul(fCoord[1], fCoord[1]);
	result.Add(&temp);
	temp.Mul(fCoord[2], fCoord[2]);
	result.Add(&temp);
	
	return result.Sqrt();
}

hsScalar hsScalarTriple::MagnitudeSquared() const
{
	hsWide	result, temp;

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
#if HS_BUILD_FOR_MAC
	fX = hsSwapEndianFloat(fX);
	fY = hsSwapEndianFloat(fY);
	fZ = hsSwapEndianFloat(fZ);
#endif

}

void hsScalarTriple::Write(hsStream *stream) const
{
	stream->WriteSwapScalar(fX);
	stream->WriteSwapScalar(fY);
	stream->WriteSwapScalar(fZ);
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
	fD=stream->ReadSwapScalar(); 
}

void hsPlane3::Write(hsStream *stream) const 
{ 
	fN.Write(stream); 
	stream->WriteSwapScalar(fD); 
}
