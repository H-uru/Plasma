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

#include "hsTypes.h"

#include "plDynaDecal.h"
#include "plAuxSpan.h"

hsBool plDynaSplot::Age(double t, hsScalar ramp, hsScalar decay, hsScalar life)
{
	hsScalar age = hsScalar(t - fBirth);
	if( age >= life )
		return true;

	int n = fNumVerts;
	if( !n )
		return true;

	hsScalar atten = fInitAtten;
	if( age < ramp )
	{
		atten *= age / ramp;
		fFlags |= kFresh;
	}
	else if( age > decay )
	{
		atten *= (life - age) / (life - decay);
		fFlags |= kFresh;
	}
	else if( fFlags & kFresh )
	{
		fFlags &= ~kFresh;
	}
	else 
	{
		return false;
	}

	hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

	if( fFlags & kAttenColor )
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			UInt32 diff = UInt32(origUVW->fZ * atten * 255.99f);
			vtx->fDiffuse = 0xff000000 
				| (diff << 16)
				| (diff << 8)
				| diff;

			vtx++;
			origUVW++;
		}
	}
	else
	if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
	{
		const int stride = sizeof(plDecalVtxFormat);

		hsScalar* sPtr = &origUVW->fZ;

		unsigned char* alpha = (unsigned char*)&fVtxBase->fDiffuse;
		alpha += 3;
		while( n-- )
		{
			hsScalar initOpac = *sPtr;
			*alpha = unsigned char(initOpac * atten * 255.99f);

			alpha += stride;
			sPtr += 3;
		}
	}
	else
	{
		hsScalar* sPtr = &origUVW->fZ;

		char* oPtr = (char*)&fVtxBase->fUVW[1].fX;

		const int stride = sizeof(plDecalVtxFormat);

		while( n-- )
		{
			(*(hsScalar*)oPtr) = *sPtr * atten;

			oPtr += stride;
			sPtr += 3;
		}
	}
	return false;
}

hsBool plDynaRipple::Age(double t, hsScalar ramp, hsScalar decay, hsScalar life)
{
	hsScalar age = hsScalar(t - fBirth);
	if( age >= life )
		return true;

	int n = fNumVerts;
	if( !n )
		return true;

	hsScalar atten = fInitAtten;
	if( age < ramp )
	{
		atten *= age / ramp;
	}
	else if( age > decay )
	{
		atten *= (life - age) / (life - decay);
	}

	hsScalar scaleU = fC1U / (age*fC2U + 1.f);
	hsScalar scaleV = fC1V / (age*fC2V + 1.f);

	hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

	if( fFlags & kAttenColor )
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			UInt32 diff = UInt32(origUVW->fZ * atten * 255.99f);
			vtx->fDiffuse = 0xff000000 
				| (diff << 16)
				| (diff << 8)
				| diff;

			vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
			vtx->fUVW[0].fY = (origUVW->fY - 0.5f) * scaleV + 0.5f;

			vtx++;
			origUVW++;
		}
	}
	else
	if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			unsigned char* alpha = ((unsigned char*)&vtx->fDiffuse) + 3;
			*alpha = unsigned char(origUVW->fZ * atten * 255.99f);

			vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
			vtx->fUVW[0].fY = (origUVW->fY - 0.5f) * scaleV + 0.5f;

			vtx++;
			origUVW++;
		}
	}
	else
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
			vtx->fUVW[0].fY = (origUVW->fY - 0.5f) * scaleV + 0.5f;

			vtx->fUVW[1].fX = origUVW->fZ * atten;

			vtx++;
			origUVW++;
		}
	}
	fFlags &= ~kFresh;
	return false;
}

hsBool plDynaWake::Age(double t, hsScalar ramp, hsScalar decay, hsScalar life)
{
	hsScalar age = hsScalar(t - fBirth);
	if( age >= life )
		return true;

	int n = fNumVerts;
	if( !n )
		return true;

	hsScalar atten = fInitAtten;
	if( age < ramp )
	{
		atten *= age / ramp;
	}
	else if( age > decay )
	{
		atten *= (life - age) / (life - decay);
	}

	hsScalar scaleU = fC1U / (age*fC2U + 1.f);
	hsScalar scaleV = fC1V / (age*fC2V + 1.f);

	hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

	if( fFlags & kAttenColor )
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			UInt32 diff = UInt32(origUVW->fZ * atten * 255.99f);
			vtx->fDiffuse = 0xff000000 
				| (diff << 16)
				| (diff << 8)
				| diff;

			vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
			vtx->fUVW[0].fY = origUVW->fY * scaleV;

			vtx++;
			origUVW++;
		}
	}
	else
	if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			unsigned char* alpha = ((unsigned char*)&vtx->fDiffuse) + 3;
			*alpha = unsigned char(origUVW->fZ * atten * 255.99f);

			vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
			vtx->fUVW[0].fY = origUVW->fY * scaleV;

			vtx++;
			origUVW++;
		}
	}
	else
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
			vtx->fUVW[0].fY = origUVW->fY * scaleV;

			vtx->fUVW[1].fX = origUVW->fZ * atten;

			vtx++;
			origUVW++;
		}
	}
	fFlags &= ~kFresh;
	return false;
}

hsBool plDynaWave::Age(double t, hsScalar ramp, hsScalar decay, hsScalar life)
{
	hsScalar age = hsScalar(t - fBirth);
	if( age >= life )
		return true;

	int n = fNumVerts;
	if( !n )
		return true;

	hsScalar atten = fInitAtten;
	if( age < ramp )
	{
		atten *= age / ramp;
	}
	else if( age > decay )
	{
		atten *= (life - age) / (life - decay);
	}

	hsScalar scale = 1.f + life * fScrollRate;
	hsScalar scroll = -fScrollRate * age;

	hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

	if( fFlags & kAttenColor )
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			UInt32 diff = UInt32(origUVW->fZ * atten * 255.99f);
			vtx->fDiffuse = 0xff000000 
				| (diff << 16)
				| (diff << 8)
				| diff;

			vtx->fUVW[0].fX = origUVW->fX;
			vtx->fUVW[0].fY = origUVW->fY * scale + scroll;

			vtx++;
			origUVW++;
		}
	}
	else
	if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			unsigned char* alpha = ((unsigned char*)&vtx->fDiffuse) + 3;
			*alpha = unsigned char(origUVW->fZ * atten * 255.99f);

			vtx->fUVW[0].fX = origUVW->fX;
			vtx->fUVW[0].fY = origUVW->fY * scale + scroll;

			vtx++;
			origUVW++;
		}
	}
	else
	{
		plDecalVtxFormat* vtx = fVtxBase;

		while( n-- )
		{
			vtx->fUVW[0].fX = origUVW->fX;
			vtx->fUVW[0].fY = origUVW->fY * scale + scroll;

			vtx->fUVW[1].fX = origUVW->fZ * atten;

			vtx++;
			origUVW++;
		}
	}
	fFlags &= ~kFresh;
	return false;
}

hsBool plDynaRippleVS::Age(double t, hsScalar ramp, hsScalar decay, hsScalar life)
{
	hsScalar age = hsScalar(t - fBirth);
	if( age >= life )
		return true;

	int n = fNumVerts;
	if( !n )
		return true;

	fFlags &= ~kFresh;
	return false;
}

