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
#include "HeadSpin.h"
#include "hsAffineParts.h"
#include "../plInterp/hsInterp.h"
#include "hsStream.h"

#include "plProfile.h"

#define PL_OPTIMIZE_COMPOSE

inline void QuatTo3Vectors(const hsQuat& q, hsVector3* const v)
{
	v[0][0] = 1.0f - 2.0f*q.fY*q.fY - 2.0f*q.fZ*q.fZ;
	v[0][1] = 2.0f*q.fX*q.fY - 2.0f*q.fW*q.fZ;
	v[0][2] = 2.0f*q.fX*q.fZ + 2.0f*q.fW*q.fY;

	v[1][0] = 2.0f*q.fX*q.fY + 2.0f*q.fW*q.fZ;
	v[1][1] = 1.0f - 2.0f*q.fX*q.fX - 2.0f*q.fZ*q.fZ;
	v[1][2] = 2.0f*q.fY*q.fZ - 2.0f*q.fW*q.fX;

	v[2][0] = 2.0f*q.fX*q.fZ - 2.0f*q.fW*q.fY;
	v[2][1] = 2.0f*q.fY*q.fZ + 2.0f*q.fW*q.fX;
	v[2][2] = 1.0f - 2.0f*q.fX*q.fX - 2.0f*q.fY*q.fY;
}

inline void QuatTo3VectorsTranspose(const hsQuat& q, hsVector3* const v)
{
	v[0][0] = 1.0f - 2.0f*q.fY*q.fY - 2.0f*q.fZ*q.fZ;
	v[1][0] = 2.0f*q.fX*q.fY - 2.0f*q.fW*q.fZ;
	v[2][0] = 2.0f*q.fX*q.fZ + 2.0f*q.fW*q.fY;

	v[0][1] = 2.0f*q.fX*q.fY + 2.0f*q.fW*q.fZ;
	v[1][1] = 1.0f - 2.0f*q.fX*q.fX - 2.0f*q.fZ*q.fZ;
	v[2][1] = 2.0f*q.fY*q.fZ - 2.0f*q.fW*q.fX;

	v[0][2] = 2.0f*q.fX*q.fZ - 2.0f*q.fW*q.fY;
	v[1][2] = 2.0f*q.fY*q.fZ + 2.0f*q.fW*q.fX;
	v[2][2] = 1.0f - 2.0f*q.fX*q.fX - 2.0f*q.fY*q.fY;
}

//
// Constructors
// Convert from Gems struct for now
//
hsAffineParts::hsAffineParts(gemAffineParts *ap)
{
	AP_SET((*this), (*ap));
}

//
//
//
hsAffineParts::hsAffineParts()
{

}

//
//
//
void hsAffineParts::Reset()
{
	fT.Set(0,0,0);
	fQ.Identity();
	fU.Identity();
	fK.Set(1,1,1);
	fF = 1.0;
}

plProfile_CreateTimer("Compose", "Affine", Compose);
plProfile_CreateTimer("ComposeInv", "Affine", ComposeInv);
//
// Create an affine matrix from the various parts
//
// AffineParts:
//    Vector t;	/* Translation components */
//    Quat   q;	/* Essential rotation	  */
//    Quat   u;	/* Stretch rotation	  */
//    Vector k;	/* Stretch factors	  */
//    float  f;	/* Sign of determinant	  */
//
// A matrix M is decomposed by : M = T F R U K Utranspose.
//		T is the translate mat.
//		F is +-Identity (to flip the rotation or not).
//		R is the rot matrix.
//		U is the stretch matrix.
//		K is the scale factor matrix.
//
void hsAffineParts::ComposeMatrix(hsMatrix44 *out) const
{
	plProfile_BeginTiming(Compose);
#ifndef PL_OPTIMIZE_COMPOSE
	// Built U matrix
	hsMatrix44 U;
	fU.MakeMatrix(&U);

	// Build scale factor matrix
	hsMatrix44 K;
	K.MakeScaleMat(&fK);

	// Build Utranspose matrix
	hsMatrix44 Utp;
	U.GetTranspose(&Utp);

	// Build R matrix
	hsMatrix44 R;
	fQ.MakeMatrix(&R);

	// Build flip matrix
//	hsAssert(fF == 1.0 || fF == -1.0, "Invalid flip portion of affine parts");
	hsMatrix44 F;
	if (fF==-1.0)
	{
		hsVector3 s;
		s.Set(-1,-1,-1);
		F.MakeScaleMat(&s);
	}
	else
		F.Reset();

	// Build translate matrix
	hsMatrix44 T;
	T.MakeTranslateMat(&fT);

	//
	// Concat mats
	//
	*out =  K * Utp;
	*out =	U * (*out);
	*out =	R * (*out);		// Q
	*out =	F * (*out);
	*out =	T * (*out);		// Translate happens last
#else // PL_OPTIMIZE_COMPOSE
	// M = T F R U K Ut,
	// but these are mostly very sparse matrices. So rather
	// than construct the full 6 matrices and concatenate them,
	// we'll work out by hand what the non-zero results will be.
	// T =	|1	0	0	Tx|
	//		|0	1	0	Ty|
	//		|0	0	1	Tz|
	// F =	|f	0	0	0|
	//		|0	f	0	0|
	//		|0	0	f	0|, where f is either 1 or -1
	// R =	|R00	R01	R02	0|
	//		|R10	R11	R12	0|
	//		|R20	R21	R22	0|
	// U =	|U00	U01	U02	0|
	//		|U10	U11	U12	0|
	//		|U20	U21	U22	0|
	// K =	|Sx		0	0	0|
	//		|0		Sy	0	0|
	//		|0		0	Sz	0|
	// Ut =	|U00	U10	U20	0|
	//		|U01	U11	U21	0|
	//		|U02	U12	U22	0|, where Uij is from matrix U
	//
	// So, K * Ut = 
	//		|Sx*U00	Sx*U10	Sx*U20	0|
	//		|Sy*U01	Sy*U11	Sy*U21	0|
	//		|Sz*U02	Sz*U12	Sz*U22	0|
	//
	// U * (K * Ut) =
	//		| U0 dot S*U0	U0 dot S*U1	U0 dot S*U2	0|
	//		| U1 dot S*U0	U1 dot S*U1	U1 dot S*U2	0|
	//		| U2 dot S*U0	U2 dot S*U1	U2 dot S*U2	0|
	//
	// Let's call that matrix UK
	//
	// Now R * U * K * Ut = R * UK =
	//		| R0 dot UKc0	R0 dot UKc1	R0 dot UKc2		0|
	//		| R1 dot UKc0	R1 dot UKc1	R1 dot UKc2		0|
	//		| R2 dot UKc0	R2 dot UKc1	R2 dot UKc2		0|, where UKci is column i from UK
	//
	// if f is -1, we negate the matrix we have so far, else we don't. We can 
	// accomplish this cleanly by just negating the scale vector S if f == -1.
	//
	// Since the translate is last, we can just stuff it into the 4th column.
	//
	// Since we only ever use UK as column vectors, we'll just construct it
	// into 3 vectors representing the columns.
	//
	// The quat MakeMatrix function is pretty efficient, but it does a little more work
	// than it has to filling out the whole matrix when we only need the 3x3 rotation,
	// and we'd rather have it in the form of vectors anyway, so we'll use our own
	// quat to 3 vectors function here.

	hsVector3 U[3];
	QuatTo3Vectors(fU, U);

	int i, j;

	hsVector3 UKt[3];
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			// SU[j] = (fK.fX * U[j].fX, fK.fY * U[j].fY, fK.fZ * U[j].fZ)
			UKt[j][i] = U[i].fX * fK.fX * U[j].fX 
				+ U[i].fY * fK.fY * U[j].fY 
				+ U[i].fZ * fK.fZ * U[j].fZ;
		}
	}

	hsVector3 R[3];
	QuatTo3Vectors(fQ, R);

	hsScalar f = fF < 0 ? -1.f : 1.f;
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			out->fMap[i][j] = R[i].InnerProduct(UKt[j]) * f;
		}

		out->fMap[i][3] = fT[i];
	}

	out->fMap[3][0] = out->fMap[3][1] = out->fMap[3][2] = 0.f;
	out->fMap[3][3] = 1.f;
	out->NotIdentity();

#endif // PL_OPTIMIZE_COMPOSE
	plProfile_EndTiming(Compose);
}

void hsAffineParts::ComposeInverseMatrix(hsMatrix44 *out) const
{
	plProfile_BeginTiming(Compose);
#ifndef PL_OPTIMIZE_COMPOSE
	// Built U matrix
	hsMatrix44 U;
	fU.Conjugate().MakeMatrix(&U);

	// Build scale factor matrix
	hsMatrix44 K;
	hsVector3 invK;
	invK.Set(hsScalarInvert(fK.fX),hsScalarInvert(fK.fY),hsScalarInvert(fK.fZ));
	K.MakeScaleMat(&invK);

	// Build Utranspose matrix
	hsMatrix44 Utp;
	U.GetTranspose(&Utp);

	// Build R matrix
	hsMatrix44 R;
	fQ.Conjugate().MakeMatrix(&R);

	// Build flip matrix
//	hsAssert(fF == 1.0 || fF == -1.0, "Invalid flip portion of affine parts");
	hsMatrix44 F;
	if (fF==-1.0)
	{
		hsVector3 s;
		s.Set(-1,-1,-1);
		F.MakeScaleMat(&s);
	}
	else
		F.Reset();

	// Build translate matrix
	hsMatrix44 T;
	T.MakeTranslateMat(&-fT);

	//
	// Concat mats
	//
	*out = Utp * K;
	*out = (*out) * U;
	*out = (*out) * R;
	*out = (*out) * F;
	*out = (*out) * T;
#else // PL_OPTIMIZE_COMPOSE
	// Same kind of thing here, except now M = Ut K U R F T
	// and again
	// T =	|1	0	0	Tx|
	//		|0	1	0	Ty|
	//		|0	0	1	Tz|
	// F =	|f	0	0	0|
	//		|0	f	0	0|
	//		|0	0	f	0|, where f is either 1 or -1
	// R =	|R00	R01	R02	0|
	//		|R10	R11	R12	0|
	//		|R20	R21	R22	0|
	// U =	|U00	U01	U02	0|
	//		|U10	U11	U12	0|
	//		|U20	U21	U22	0|
	// K =	|Sx		0	0	0|
	//		|0		Sy	0	0|
	//		|0		0	Sz	0|
	// Ut =	|U00	U10	U20	0|
	//		|U01	U11	U21	0|
	//		|U02	U12	U22	0|, where Uij is from matrix U
	//
	// So, Ut * K = 
	//		|U00*Sx		U10*Sy	U20*Sz	0|
	//		|U01*Sx		U11*Sy	U21*Sz	0|
	//		|U02*Sx		U12*Sy	U22*Sz	0|
	//
	// (Ut * K) * U = UK =
	//		|Ut0*S dot Ut0	Ut0*S dot Ut1	Ut0*S dot Ut2	0|
	//		|Ut1*S dot Ut0	Ut1*S dot Ut1	Ut1*S dot Ut2	0|
	//		|Ut2*S dot Ut0	Ut2*S dot Ut1	Ut2*S dot Ut2	0|
	//
	// (((Ut * K) * U) * R)[i][j] = UK[i] dot Rc[j]
	//
	// Again we'll stuff the flip into the scale.
	//
	// Now, because the T is on the other end of the concat (closest
	// to the vertex), we can't just stuff it in. If Mr is the 
	// rotation part of the final matrix (Ut * K * U * R * F), then
	// the translation components M[i][3] = Mr[i] dot T.
	//		
	//
	hsVector3 Ut[3];
	QuatTo3VectorsTranspose(fU.Conjugate(), Ut);

	int i, j;

	hsVector3 invK;
	invK.Set(hsScalarInvert(fK.fX),hsScalarInvert(fK.fY),hsScalarInvert(fK.fZ));
	hsVector3 UK[3];
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			// SUt[i] = (Ut[i].fX * invK.fX, Ut[i].fY * invK.fY, Ut[i].fZ * invK.fZ)
			// So SUt[i].InnerProduct(Ut[j]) ==
			//		Ut[i].fX * invK.fX * Ut[j].fX 
			//			+ Ut[i].fY * invK.fY * Ut[j].fY 
			//			+ Ut[i].fZ * invK.fZ * Ut[j].fZ 

			UK[i][j] = Ut[i].fX * invK.fX * Ut[j].fX
				+ Ut[i].fY * invK.fY * Ut[j].fY
				+ Ut[i].fZ * invK.fZ * Ut[j].fZ;
		}
	}

	hsVector3 Rt[3];
	QuatTo3VectorsTranspose(fQ.Conjugate(), Rt);

	hsScalar f = fF < 0 ? -1.f : 1.f;
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 3; j++ )
		{
			out->fMap[i][j] = UK[i].InnerProduct(Rt[j]) * f;
		}

		out->fMap[i][3] = -(fT.InnerProduct((hsPoint3*)(&out->fMap[i])));
	}

	out->fMap[3][0] = out->fMap[3][1] = out->fMap[3][2] = 0.f;
	out->fMap[3][3] = 1.f;
	out->NotIdentity();

#endif // PL_OPTIMIZE_COMPOSE
	plProfile_EndTiming(Compose);
}

//
// Given 2 affineparts structs and a p value (between 0-1),
// compute a new affine parts.
//
void hsAffineParts::SetFromInterp(const hsAffineParts &ap1, const hsAffineParts &ap2, float p)
{
	hsAssert(p>=0.0 && p<=1.0, "Interpolate param must be 0-1");

#if 0
	// Debug
	float rad1,rad2, rad3;
	hsVector3 axis1, axis2, axis3;
	k1->fQ.GetAngleAxis(&rad1, &axis1);
	k2->fQ.GetAngleAxis(&rad2, &axis2);
	fQ.GetAngleAxis(&rad3, &axis3);
#endif

	hsInterp::LinInterp(&ap1, &ap2, p, this);
}

//
// Read
//
void hsAffineParts::Read(hsStream *stream)
{
	fT.Read(stream);
	fQ.Read(stream);
	fU.Read(stream);
	fK.Read(stream);
	fF = stream->ReadSwapFloat();
}

//
// Write
//
void hsAffineParts::Write(hsStream *stream)
{
	fT.Write(stream);
	fQ.Write(stream);
	fU.Write(stream);
	fK.Write(stream);
	stream->WriteSwapFloat(fF);
}
