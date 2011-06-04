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
#include "hsInterp.h"
#include "../plTransform/hsAffineParts.h"
#include "hsColorRGBA.h"
#include "hsPoint2.h"

//
///////////////////////////////////////////////////////
// linear interpolation
///////////////////////////////////////////////////////
//
void hsInterp::LinInterp(hsScalar k1, hsScalar k2, hsScalar t, hsScalar* result)
{
	*result = k1 + t * (k2 - k1);
}

void hsInterp::LinInterp(const hsScalarTriple* k1, const hsScalarTriple* k2, hsScalar t, 
					  hsScalarTriple* result)
{
	if (t==0.0)
		*result = *k1;
	else
	if (t==1.0)
		*result = *k2;
	else
	{
		LinInterp(k1->fX, k2->fX, t, &result->fX);
		LinInterp(k1->fY, k2->fY, t, &result->fY);
		LinInterp(k1->fZ, k2->fZ, t, &result->fZ);
	}
}

void hsInterp::LinInterp(const hsColorRGBA* k1, const hsColorRGBA* k2, hsScalar t, 
					  hsColorRGBA* result, UInt32 flags)
{
	if (t==0.0)
	{
		// copy
		result->r = k1->r;
		result->g = k1->g;
		result->b = k1->b;
		if (!(flags & kIgnoreAlpha))
			result->a = k1->a;
		return;
	}

	if (t==1.0)
	{
		result->r = k2->r;
		result->g = k2->g;
		result->b = k2->b;
		if (!(flags & kIgnoreAlpha))
			result->a = k2->a;
		return;
	}
	
	LinInterp(k1->r, k2->r, t, &result->r);
	LinInterp(k1->g, k2->g, t, &result->g);
	LinInterp(k1->b, k2->b, t, &result->b);
	if (!(flags & kIgnoreAlpha))
		LinInterp(k1->a, k2->a, t, &result->a);
}

void hsInterp::LinInterp(const hsMatrix33* k1, const hsMatrix33* k2, hsScalar t, 
					  hsMatrix33* result, UInt32 flags)
{
	if (t==0.0)
	{
		// copy
		result->fMap[0][0] = k1->fMap[0][0];
		result->fMap[0][1] = k1->fMap[0][1];
		result->fMap[0][2] = k1->fMap[0][2];

		result->fMap[1][0] = k1->fMap[1][0];
		result->fMap[1][1] = k1->fMap[1][1];
		result->fMap[1][2] = k1->fMap[1][2];

		if (!(flags & kIgnoreLastMatRow))
		{
			result->fMap[2][0] = k1->fMap[2][0];
			result->fMap[2][1] = k1->fMap[2][1];
			result->fMap[2][2] = k1->fMap[2][2];
		}
		return;
	}
	if (t==1.0)
	{
		// copy
		result->fMap[0][0] = k2->fMap[0][0];
		result->fMap[0][1] = k2->fMap[0][1];
		result->fMap[0][2] = k2->fMap[0][2];

		result->fMap[1][0] = k2->fMap[1][0];
		result->fMap[1][1] = k2->fMap[1][1];
		result->fMap[1][2] = k2->fMap[1][2];

		if (!(flags & kIgnoreLastMatRow))
		{
			result->fMap[2][0] = k2->fMap[2][0];
			result->fMap[2][1] = k2->fMap[2][1];
			result->fMap[2][2] = k2->fMap[2][2];
		}
		return;
	}

	LinInterp(k1->fMap[0][0], k2->fMap[0][0], t, &result->fMap[0][0]);
	LinInterp(k1->fMap[0][1], k2->fMap[0][1], t, &result->fMap[0][1]);
	LinInterp(k1->fMap[0][2], k2->fMap[0][2], t, &result->fMap[0][2]);

	LinInterp(k1->fMap[1][0], k2->fMap[1][0], t, &result->fMap[1][0]);
	LinInterp(k1->fMap[1][1], k2->fMap[1][1], t, &result->fMap[1][1]);
	LinInterp(k1->fMap[1][2], k2->fMap[1][2], t, &result->fMap[1][2]);

	if (!(flags & kIgnoreLastMatRow))
	{
		LinInterp(k1->fMap[2][0], k2->fMap[2][0], t, &result->fMap[2][0]);
		LinInterp(k1->fMap[2][1], k2->fMap[2][1], t, &result->fMap[2][1]);
		LinInterp(k1->fMap[2][2], k2->fMap[2][2], t, &result->fMap[2][2]);
	}
}

//
//
void hsInterp::LinInterp(const hsMatrix44* mat1, const hsMatrix44* mat2, hsScalar t, 
					  hsMatrix44* out, UInt32 flags)
{
	if (flags == 0)
	{
		if( 0 == t )
		{
			*out = *mat1;
			return;
		}

		if( hsScalar1 == t )
		{
			*out = *mat2;
			return;
		}
	}

	if( flags & kIgnorePartsScale )
	{
		if (!(flags & kIgnorePartsRot))
		{
			// interp rotation with quats
			hsQuat q1, q2, qOut;
			q1.SetFromMatrix(mat1);
			q2.SetFromMatrix(mat2);
			LinInterp(&q1, &q2, t, &qOut);
			
			qOut.Normalize();
			qOut.MakeMatrix(out);
		}
		else
			out->Reset();
#if 1
		hsAssert(mat2->fMap[3][0]==0 && mat2->fMap[3][1]==0 && mat2->fMap[3][2]==0 && mat2->fMap[3][3]==1,
			"matrix prob?");
#else
		// copy 
		for(int i=0; i<3; i++)
			out->fMap[3][i] = mat2->fMap[3][i];
#endif
		
		if (!(flags & kIgnorePartsPos))
		{
			// interp translation
			hsPoint3 p1,p2,pOut;
			mat1->GetTranslate(&p1);
			mat2->GetTranslate(&p2);
			LinInterp(&p1, &p2, t, &pOut);
			out->SetTranslate(&pOut);
			out->NotIdentity();		// in case no rot
		}
	}
	else
	{
		// Complete decomp and parts interp

		gemAffineParts gemParts1, gemParts2;
		hsAffineParts parts1, parts2, partsOut;
	
		decomp_affine(mat1->fMap, &gemParts1); 
		AP_SET(parts1, gemParts1);

		decomp_affine(mat2->fMap, &gemParts2); 
		AP_SET(parts2, gemParts2);

		LinInterp(&parts1, &parts2, t, &partsOut, flags);	// flags will be parsed here

		partsOut.ComposeMatrix(out);
	}
}

void hsInterp::LinInterp(const hsQuat* k1, const hsQuat* k2, hsScalar t, hsQuat* result)
{
	if (t==0.0)
		*result = *k1;
	else
	if (t==1.0)
		*result = *k2;
	else
	{
		result->SetFromSlerp(*k1, *k2, t);
	}
}

void hsInterp::LinInterp(const hsScaleValue* k1, const hsScaleValue* k2, hsScalar t, 
					  hsScaleValue* result)
{
	LinInterp(&k1->fS, &k2->fS, t, &result->fS);	// Stretch rotation	
	LinInterp(&k1->fQ, &k2->fQ, t, &result->fQ);	// Stretch factor	
}

void hsInterp::LinInterp(const hsAffineParts* k1, const hsAffineParts* k2, hsScalar t, 
					  hsAffineParts* result, UInt32 flags)
{		
	if (t==0.0)
	{
		// copy
		if (!(flags & kIgnorePartsPos))
			result->fT = k1->fT;
		if (!(flags & kIgnorePartsRot))
			result->fQ = k1->fQ;
		if (!(flags & kIgnorePartsScale))
		{
			// same as preserveScale
			result->fU = k1->fU;
			result->fK = k1->fK;
		}
		result->fF = k1->fF;

		return;
	}
	
	if (flags & kPreservePartsScale)
	{
		result->fU = k1->fU;	// just copy scale from 1st key
		result->fK = k1->fK;
	}

	if (t==1.0)
	{
		// copy
		if (!(flags & kIgnorePartsPos))
			result->fT = k2->fT;
		if (!(flags & kIgnorePartsRot))
			result->fQ = k2->fQ;
		if (!(flags & (kIgnorePartsScale | kPreservePartsScale)))
		{
			result->fU = k2->fU;
			result->fK = k2->fK;
		}
		result->fF = k2->fF;

		return;
	}
	
	if(k1->fF!=k2->fF)
		hsStatusMessageF("WARNING: Inequality in affine parts flip value.");

//	hsAssert(k1->fF==k2->fF, "inequality in affine parts flip value");
	if (!(flags & kIgnorePartsPos))
		LinInterp(&k1->fT, &k2->fT, t, &result->fT);	// Translation
	if (!(flags & kIgnorePartsRot))
	{
		LinInterp(&k1->fQ, &k2->fQ, t, &result->fQ);	// Essential rotation	
	}
	
	if (!(flags & (kIgnorePartsScale | kPreservePartsScale)))
	{
		LinInterp(&k1->fU, &k2->fU, t, &result->fU);	// Stretch rotation	
		LinInterp(&k1->fK, &k2->fK, t, &result->fK);	// Stretch factor	
	}

#if 0
	if (!(flags & kIgnorePartsDet))
		LinInterp(k1->fF, k2->fF, t, &result->fF);	// Flip rot var
#else
	result->fF = k1->fF;
#endif
}

//
///////////////////////////////////////////////////////
// Key interpolation
///////////////////////////////////////////////////////
//

void hsInterp::BezScalarEval(const hsScalar value1, const hsScalar outTan,
							 const hsScalar value2, const hsScalar inTan,
							 const hsScalar t, const hsScalar tanScale, hsScalar *result)
{
#if 0
	// If the tangents were what you'd expect them to be... Hermite splines, than this code
	// would make sense. But no, Max likes to store them in a scaled form based on the
	// time of each frame. If we ever optimize this further, we could do the scaling on export,
	// but I need this to work right now before all the artists hate me too much.
	const hsScalar t2 = t * t;
	const hsScalar t3 = t2 * t;

	const hsScalar term1 = 2 * t3 - 3 * t2;

	*result = ((term1 + 1) * value1) +
		(-term1 * value2) +
		((t3 - 2 * t2 + 1) * outTan) +
		((t3 - t2) * inTan);
#else
	const hsScalar oneMinusT = (1.0f - t);
	const hsScalar tSq = t * t;
	const hsScalar oneMinusTSq = oneMinusT * oneMinusT;

	*result = (oneMinusT * oneMinusTSq * value1) +
		(3.f * t * oneMinusTSq * (value1 + outTan * tanScale)) +
		(3.f * tSq * oneMinusT * (value2 + inTan * tanScale)) +
		(tSq * t * value2);
#endif
}

void hsInterp::BezInterp(const hsBezPoint3Key* k1, const hsBezPoint3Key* k2, const hsScalar t, hsScalarTriple* result)
{
	hsScalar scale = (k2->fFrame - k1->fFrame) * MAX_TICKS_PER_FRAME / 3.f;
	BezScalarEval(k1->fValue.fX, k1->fOutTan.fX, k2->fValue.fX, k2->fInTan.fX, t, scale, &result->fX);
	BezScalarEval(k1->fValue.fY, k1->fOutTan.fY, k2->fValue.fY, k2->fInTan.fY, t, scale, &result->fY);
	BezScalarEval(k1->fValue.fZ, k1->fOutTan.fZ, k2->fValue.fZ, k2->fInTan.fZ, t, scale, &result->fZ);
}

void hsInterp::BezInterp(const hsBezScalarKey* k1, const hsBezScalarKey* k2, const hsScalar t, hsScalar* result)
{
	hsScalar scale = (k2->fFrame - k1->fFrame) * MAX_TICKS_PER_FRAME / 3.f;
	BezScalarEval(k1->fValue, k1->fOutTan, k2->fValue, k2->fInTan, t, scale, result);
}

void hsInterp::BezInterp(const hsBezScaleKey* k1, const hsBezScaleKey* k2, const hsScalar t, hsScaleValue* result)
{
	hsScalar scale = (k2->fFrame - k1->fFrame) * MAX_TICKS_PER_FRAME / 3.f;
	BezScalarEval(k1->fValue.fS.fX, k1->fOutTan.fX, k2->fValue.fS.fX, k2->fInTan.fX, t, scale, &result->fS.fX);
	BezScalarEval(k1->fValue.fS.fY, k1->fOutTan.fY, k2->fValue.fS.fY, k2->fInTan.fY, t, scale, &result->fS.fY);
	BezScalarEval(k1->fValue.fS.fZ, k1->fOutTan.fZ, k2->fValue.fS.fZ, k2->fInTan.fZ, t, scale, &result->fS.fZ);	

	// Slerp scale axis
	LinInterp(&k1->fValue.fQ, &k2->fValue.fQ, t, &result->fQ);
}

//
// Get an element from an array of unknown type
//
static inline hsKeyFrame* GetKey(Int32 i, void *keys, Int32 size)
{
	return (hsKeyFrame*) ((char*)keys + size * i);
}

//
// STATIC
// Given a list of keys, and a time, fills in the 2 boundary keys and 
// a fraction (p=0-1) indicating where the time falls between them.
// Returns the index of the first key which can be passed in as a hint (lastKeyIdx)
// for the next search.
//
void hsInterp::GetBoundaryKeyFrames(hsScalar time, UInt32 numKeys, void *keys, UInt32 size,
									hsKeyFrame **kF1, hsKeyFrame **kF2, UInt32 *lastKeyIdx, hsScalar *p, hsBool forwards)
{
	hsAssert(numKeys>1, "Must have more than 1 keyframe");
	int k1, k2;
	UInt16 frame = (UInt16)(time * MAX_FRAMES_PER_SEC);

	// boundary case, past end
	if (frame > GetKey(numKeys-1, keys, size)->fFrame)
	{
		k1=k2=numKeys-1;
		(*kF2) = GetKey(k1, keys, size);
		(*kF1) = (*kF2);
		*p = 0.0;
		goto ret;
	}

	hsKeyFrame *key1, *key2;
	// boundary case, before start
	if (frame < (key1=GetKey(0, keys, size))->fFrame)
	{
		k1=k2=0;
		(*kF1) = GetKey(k1, keys, size);
		(*kF2) = (*kF1);
		*p = 0.0;
		goto ret;
	}

	// prime loop
	int i;
	i = 1;
	if (*lastKeyIdx > 0 && *lastKeyIdx < numKeys - 1)
	{
		// new starting point for search
		if (forwards)
			key1 = GetKey(*lastKeyIdx, keys, size);
		else
			key2 = GetKey(*lastKeyIdx + 1, keys, size);

		i = *lastKeyIdx + 1;
	}
	else if (!forwards)
	{
		key2 = GetKey(1, keys, size);
	}

	// search pairs of keys
	int count;
	if (forwards)
	{
		for (count = 1; count <= numKeys; count++, i++)
		{
			if (i >= numKeys)
			{
				key1 = GetKey(0, keys, size);
				i = 1;
				count++;
			}
				
			key2 = GetKey(i, keys, size);
			if (frame <= key2->fFrame && frame >= key1->fFrame)
			{
				k2=i;
				k1=i-1;
				(*kF2) = key2;
				(*kF1) = key1;
				*p = (time - (*kF1)->fFrame / MAX_FRAMES_PER_SEC) / (((*kF2)->fFrame - (*kF1)->fFrame) / MAX_FRAMES_PER_SEC);
				goto ret;
			}
			key1=key2;
		}
	}
	else
	{
		for (count = 1; count <= numKeys; count++, i--)
		{
			if (i < 1)
			{
				i = numKeys - 1;
				key2 = GetKey(i, keys, size);
				count++;
			}
			
			key1 = GetKey(i - 1, keys, size);
			if (frame <= key2->fFrame && frame >= key1->fFrame)
			{
				k2 = i;
				k1 = i - 1;
				(*kF2) = key2;
				(*kF1) = key1;
				*p = (time - (*kF1)->fFrame / MAX_FRAMES_PER_SEC) / (((*kF2)->fFrame - (*kF1)->fFrame) / MAX_FRAMES_PER_SEC);
				goto ret;
			}
			key2=key1;
		}
	}		

ret:
;

#if 0
	char str[128];
	sprintf(str, "k1=%d, k2=%d, p=%f\n", k1, k2, *p);
	OutputDebugString(str);
#endif
	*lastKeyIdx = k1;
}


