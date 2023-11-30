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

#ifndef plMaxLightContext_inc
#define plMaxLightContext_inc

#include "hsGeometry3.h"

class plMaxLightContext : public ShadeContext
{
protected:
    TimeValue           fCurrentTime;

    Box3                fBounds;

    Point3              fPos;
    Point3              fNorm;
    Point3              fPertNorm;

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

    BOOL InMtlEditor() override { return false; }    // is this rendering the mtl editor sample sphere?

    LightDesc* Light(int n) override { return nullptr; } // get the nth light.

    TimeValue CurTime() override { return fCurrentTime; }        // current time value

    int FaceNumber() override { return 0; }
    Point3 Normal() override { return fPertNorm; }       // interpolated surface normal, in cameara coords: affected by SetNormal()
    void SetNormal(Point3 p) override { fPertNorm = p; } // used for perturbing normal
    Point3 OrigNormal() override { return fNorm; } // original surface normal: not affected by SetNormal();
    Point3 GNormal() override { return fNorm; }      // geometric (face) normal

    Point3 V() override { return Point3(0.f,1.f,0.f); }              // Unit view vector: from camera towards P
    void SetView(Point3 p) override { } // Set the view vector

    Point3 ReflectVector() override { return V(); }  // reflection vector
    Point3 RefractVector(float ior) override { return V(); } // refraction vector

    Point3 CamPos() override { return Point3(0,0,0); }           // camera position

    Point3 P() override { return fPos; }             // point to be shaded;
    Point3 DP() override { return Point3(0,0,0); }               // deriv of P, relative to pixel, for AA

    Point3 PObj() override { return P(); }               // point in obj coords
    Point3 DPObj() override { return DP(); }         // deriv of PObj, rel to pixel, for AA
    Box3 ObjectBox() override { return fBounds; }        // Object extents box in obj coords
    Point3 PObjRelBox() override;        // Point rel to obj box [-1 .. +1 ]
    Point3 DPObjRelBox() override { return Point3(0,0,0); }      // deriv of Point rel to obj box [-1 .. +1 ]
    void ScreenUV(Point2& uv, Point2 &duv) override { uv.Set(0,0); duv.Set(0,0); } // screen relative uv (from lower left)
    IPoint2 ScreenCoord() override { return IPoint2(0,0); } // integer screen coordinate (from upper left)

    Point3 UVW(int channel=0) override { return Point3(0,0,0); }             // return UVW coords for point
    Point3 DUVW(int channel=0) override { return Point3(0,0,0); }            // return UVW derivs for point
    void DPdUVW(Point3 dP[3],int channel=0) override { dP[0] = dP[1] = dP[2] = Point3(0,0,0); } // Bump vectors for UVW (camera space)

    void GetBGColor(Color &bgcol, Color& transp, BOOL fogBG=TRUE) override { bgcol.Black(); transp.Black(); }   // returns Background color, bg transparency
    // these became virtual in 2023
    #if MAX_VERSION_MAJOR >= 25
    Matrix3 MatrixTo(RefFrame ito) override { return Matrix3::Identity; }
    Matrix3 MatrixFrom(RefFrame ifrom) override { return Matrix3::Identity; }
#endif
    Point3 PointTo(const Point3& p, RefFrame ito) override { return p; }
    Point3 PointFrom(const Point3& p, RefFrame ifrom) override { return p; }
    Point3 VectorTo(const Point3& p, RefFrame ito) override { return p; }
    Point3 VectorFrom(const Point3& p, RefFrame ifrom) override { return p; }
};

inline Point3 plMaxLightContext::PObjRelBox()
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
    Matrix3 WorldToCam() const { return Matrix3(1); }
    Color   GlobalLightLevel() const override { return Color(1.f, 1.f, 1.f); }
    int Progress(int done, int total) const override {
        return 1;
    }
};


#endif // plMaxLightContext_inc
