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
//
//////////////////////////////////////////////////////////////////////////
// EULER STUFF
// See Gems IV, Ken Shoemake
//////////////////////////////////////////////////////////////////////////
//
#include <float.h>	// for FLT_EPSILON
#include "hsEuler.h"
#include "hsQuat.h"
#include "hsMatrix44.h"

enum QuatPart 
{
	X, Y, Z, W
};

//
// Construct quaternion from Euler angles (in radians).
//
void hsEuler::GetQuat(hsQuat* qu)
{ 
    double a[3], ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;

	hsEuler ea=*this;	// copy
    EulGetOrd(ea.fOrder,i,j,k,h,n,s,f);
    if (f==EulFrmR) 
	{
		hsScalar t = ea.fX; ea.fX = ea.fZ; ea.fZ = t;
	}
    if (n==EulParOdd) 
		ea.fY = -ea.fY;
    ti = ea.fX*0.5; tj = ea.fY*0.5; th = ea.fZ*0.5;
    ci = cos(ti);  cj = cos(tj);  ch = cos(th);
    si = sin(ti);  sj = sin(tj);  sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) 
	{
		a[i] = cj*(cs + sc);	/* Could speed up with */
		a[j] = sj*(cc + ss);	/* trig identities. */
		a[k] = sj*(cs - sc);
		qu->fW = static_cast<float>(cj*(cc - ss));
    } 
	else 
	{
		a[i] = cj*sc - sj*cs;
		a[j] = cj*ss + sj*cc;
		a[k] = cj*cs - sj*sc;
		qu->fW = static_cast<float>(cj*cc + sj*ss);
    }
    if (n==EulParOdd) 
		a[j] = -a[j];
    qu->fX = static_cast<float>(a[X]); 
	qu->fY = static_cast<float>(a[Y]);
	qu->fZ = static_cast<float>(a[Z]);
}

//
// Construct matrix from Euler angles (in radians). 
//
void hsEuler::GetMatrix44(hsMatrix44* mat)
{
    double ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;

	hsEuler ea=*this;	// copy
    EulGetOrd(ea.fOrder,i,j,k,h,n,s,f);
    if (f==EulFrmR) 
	{
		hsScalar t = ea.fX; ea.fX = ea.fZ; ea.fZ = t;
	}
    if (n==EulParOdd) 
	{
		ea.fX = -ea.fX; ea.fY = -ea.fY; ea.fZ = -ea.fZ;
	}
    ti = ea.fX;	  tj = ea.fY;	th = ea.fZ;
    ci = cos(ti); cj = cos(tj); ch = cos(th);
    si = sin(ti); sj = sin(tj); sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) 
	{
		mat->fMap[i][i] = static_cast<float>(cj);
		mat->fMap[i][j] = static_cast<float>(sj*si);
		mat->fMap[i][k] = static_cast<float>(sj*ci);

		mat->fMap[j][i] = static_cast<float>(sj*sh);
		mat->fMap[j][j] = static_cast<float>(-cj*ss+cc);
		mat->fMap[j][k] = static_cast<float>(-cj*cs-sc);

		mat->fMap[k][i] = static_cast<float>(-sj*ch);
		mat->fMap[k][j] = static_cast<float>(cj*sc+cs);
		mat->fMap[k][k] = static_cast<float>(cj*cc-ss);
    } 
	else 
	{
		mat->fMap[i][i] = static_cast<float>(cj*ch);
		mat->fMap[i][j] = static_cast<float>(sj*sc-cs);
		mat->fMap[i][k] = static_cast<float>(sj*cc+ss);

		mat->fMap[j][i] = static_cast<float>(cj*sh);
		mat->fMap[j][j] = static_cast<float>(sj*ss+cc);
		mat->fMap[j][k] = static_cast<float>(sj*cs-sc);

		mat->fMap[k][i] = static_cast<float>(-sj);
		mat->fMap[k][j] = static_cast<float>(cj*si);
		mat->fMap[k][k] = static_cast<float>(cj*ci);
    }
    mat->fMap[W][X]=mat->fMap[W][Y]=mat->fMap[W][Z]=mat->fMap[X][W]=mat->fMap[Y][W]=mat->fMap[Z][W]=0.0; 
	mat->fMap[W][W]=1.0;
}

//
// Convert matrix to Euler angles (in radians)
//
void hsEuler::SetFromMatrix44(const hsMatrix44* mat, UInt32 order)
{
    int i,j,k,h,n,s,f;

    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==EulRepYes) 
	{
		double sy = sqrt(mat->fMap[i][j]*mat->fMap[i][j] + mat->fMap[i][k]*mat->fMap[i][k]);
		if (sy > 16*FLT_EPSILON) 
		{
			fX = static_cast<float>(atan2(mat->fMap[i][j], mat->fMap[i][k]));
			fY = static_cast<float>(atan2(sy, (double)mat->fMap[i][i]));
			fZ = static_cast<float>(atan2(mat->fMap[j][i], -mat->fMap[k][i]));
		} else 
		{
			fX = static_cast<float>(atan2(-mat->fMap[j][k], mat->fMap[j][j]));
			fY = static_cast<float>(atan2(sy, (double)mat->fMap[i][i]));
			fZ = 0;
		}
    } 
	else 
	{
		double cy = sqrt(mat->fMap[i][i]*mat->fMap[i][i] + mat->fMap[j][i]*mat->fMap[j][i]);
		if (cy > 16*FLT_EPSILON) 
		{
			fX = static_cast<float>(atan2(mat->fMap[k][j], mat->fMap[k][k]));
			fY = static_cast<float>(atan2((double)(-mat->fMap[k][i]), cy));
			fZ = static_cast<float>(atan2(mat->fMap[j][i], mat->fMap[i][i]));
		} 
		else 
		{
			fX = static_cast<float>(atan2(-mat->fMap[j][k], mat->fMap[j][j]));
			fY = static_cast<float>(atan2((double)(-mat->fMap[k][i]), cy));
			fZ = 0;
		}
    }
    if (n==EulParOdd) 
	{
		fX = -fX; fY = - fY; fZ = -fZ;
	}
    if (f==EulFrmR) 
	{
		hsScalar t = fX; fX = fZ; fZ = t;
	}
    fOrder = order;
}

//
// Convert quaternion to Euler angles (in radians)
//
void hsEuler::SetFromQuat(const hsQuat* q, UInt32 order)
{
    hsMatrix44 mat;
    double Nq = q->fX*q->fX+q->fY*q->fY+q->fZ*q->fZ+q->fW*q->fW;
    double s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
    double xs = q->fX*s,		ys = q->fY*s,	 zs = q->fZ*s;
    double wx = q->fW*xs,	wy = q->fW*ys,	 wz = q->fW*zs;
    double xx = q->fX*xs,	xy = q->fX*ys,	 xz = q->fX*zs;
    double yy = q->fY*ys,	yz = q->fY*zs,	 zz = q->fZ*zs;
    mat.fMap[X][X] = static_cast<float>(1.0 - (yy + zz));
	mat.fMap[X][Y] = static_cast<float>(xy - wz);
	mat.fMap[X][Z] = static_cast<float>(xz + wy);
    mat.fMap[Y][X] = static_cast<float>(xy + wz); 
	mat.fMap[Y][Y] = static_cast<float>(1.0 - (xx + zz)); 
	mat.fMap[Y][Z] = static_cast<float>(yz - wx);
    mat.fMap[Z][X] = static_cast<float>(xz - wy);
	mat.fMap[Z][Y] = static_cast<float>(yz + wx); 
	mat.fMap[Z][Z] = static_cast<float>(1.0 - (xx + yy));
    mat.fMap[W][X] = mat.fMap[W][Y] = mat.fMap[W][Z] = 
		mat.fMap[X][W] = mat.fMap[Y][W] = mat.fMap[Z][W] = 0.0; 
	mat.fMap[W][W] = 1.0;
    SetFromMatrix44(&mat, order);
}
