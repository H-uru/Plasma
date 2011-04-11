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

#ifndef plMaxLightContext_inc
#define plMaxLightContext_inc

#include "hsGeometry3.h"

class plMaxLightContext : public ShadeContext
{
protected:
	TimeValue			fCurrentTime;

	Box3				fBounds;

	Point3				fPos;
	Point3				fNorm;
	Point3				fPertNorm;

public:
	plMaxLightContext(const Box3& b, TimeValue t=0) 
	{ 
		fBounds = b; 
		fCurrentTime = t; 

		doMaps = true;
		filterMaps = true;
		backFace = false;
		xshadeID = 0;
		mtlNum = 0;
	}

	void SetPoint(const Point3& p, const Point3& n) { fPos = p; fNorm = fPertNorm = n; } // Must be world space coming in.
	void SetPoint(const hsPoint3& p, const hsVector3& n) { fPos = Point3(p.fX,p.fY,p.fZ); fNorm = fPertNorm = Point3(n.fX,n.fY,n.fZ); } // Must be world space coming in.

	virtual BOOL InMtlEditor() { return false; }	// is this rendering the mtl editor sample sphere?

	virtual LightDesc* Light(int n) { return nil; }	// get the nth light. 

	virtual TimeValue CurTime() { return fCurrentTime; }     	// current time value

	virtual int FaceNumber() { return 0; }
	virtual Point3 Normal() { return fPertNorm; }  		// interpolated surface normal, in cameara coords: affected by SetNormal()
	virtual void SetNormal(Point3 p) { fPertNorm = p; }	// used for perturbing normal
	virtual Point3 OrigNormal() { return fNorm; } // original surface normal: not affected by SetNormal();
	virtual Point3 GNormal() { return fNorm; }		// geometric (face) normal

	virtual Point3 V() { return Point3(0.f,1.f,0.f); }       		// Unit view vector: from camera towards P 
	virtual void SetView(Point3 p) {  }	// Set the view vector

	virtual	Point3 ReflectVector() { return V(); }	// reflection vector
	virtual	Point3 RefractVector(float ior) { return V(); }	// refraction vector

	virtual Point3 CamPos() { return Point3(0,0,0); }			// camera position

	virtual Point3 P() { return fPos; }				// point to be shaded;
	virtual Point3 DP() { return Point3(0,0,0); }    		  	// deriv of P, relative to pixel, for AA

	virtual Point3 PObj() { return P(); }   		  	// point in obj coords
	virtual Point3 DPObj() { return DP(); }   	   	// deriv of PObj, rel to pixel, for AA
	virtual Box3 ObjectBox() { return fBounds; } 	  	// Object extents box in obj coords
	virtual Point3 PObjRelBox();	  	// Point rel to obj box [-1 .. +1 ] 
	virtual Point3 DPObjRelBox() { return Point3(0,0,0); }	  	// deriv of Point rel to obj box [-1 .. +1 ] 
	virtual void ScreenUV(Point2& uv, Point2 &duv) {uv.Set(0,0); duv.Set(0,0); } // screen relative uv (from lower left)
	virtual IPoint2 ScreenCoord() { return IPoint2(0,0); } // integer screen coordinate (from upper left)

	virtual Point3 UVW(int channel=0) { return Point3(0,0,0); }  			// return UVW coords for point
	virtual Point3 DUVW(int channel=0) { return Point3(0,0,0); } 			// return UVW derivs for point
	virtual void DPdUVW(Point3 dP[3],int channel=0) { dP[0] = dP[1] = dP[2] = Point3(0,0,0); } // Bump vectors for UVW (camera space)

	virtual void GetBGColor(Color &bgcol, Color& transp, BOOL fogBG=TRUE) { bgcol.Black(); transp.Black(); }   // returns Background color, bg transparency

	virtual Point3 PointTo(const Point3& p, RefFrame ito) { return p; } 
	virtual Point3 PointFrom(const Point3& p, RefFrame ifrom) { return p; }
	virtual Point3 VectorTo(const Point3& p, RefFrame ito) { return p; }
	virtual Point3 VectorFrom(const Point3& p, RefFrame ifrom) { return p; }
};

inline Point3 plMaxLightContext::PObjRelBox(void)
{
	Point3 q;
	Point3 p = PObj();
	Box3 b = ObjectBox(); 
	q.x = 2.0f*(p.x-b.pmin.x)/(b.pmax.x-b.pmin.x) - 1.0f;
	q.y = 2.0f*(p.y-b.pmin.y)/(b.pmax.y-b.pmin.y) - 1.0f;
	q.z = 2.0f*(p.z-b.pmin.z)/(b.pmax.z-b.pmin.z) - 1.0f;
	return q;
}

class plMaxRendContext : public RendContext 
{
public:
	Matrix3	WorldToCam() const { return Matrix3(1); }
	Color	GlobalLightLevel() const { return Color(1.f, 1.f, 1.f); }
	int	Progress(int done, int total) {
		return 1;
	}
};


#endif // plMaxLightContext_inc
