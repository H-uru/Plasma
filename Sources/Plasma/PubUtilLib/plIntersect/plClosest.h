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

#ifndef plIsect_inc
#define plIsect_inc

struct hsPoint3;
struct hsVector3;


class plClosest
{
public:
enum plClosestClampFlags
{
    kClampLower0    = 0x1,
    kClampUpper0    = 0x2,
    kClamp0         = kClampLower0 | kClampUpper0,
    kClampLower1    = 0x4,
    kClampLower     = kClampLower0 | kClampLower1,
    kClampUpper1    = 0x8,
    kClampUpper     = kClampUpper0 | kClampUpper1,
    kClamp1         = kClampLower1 | kClampUpper1,
    kClamp          = kClamp0 | kClamp1
};


    // Return clamp flags for where clamped
    static uint32_t PointOnLine(const hsPoint3& p0,                       // Point
                            const hsPoint3& p1, const hsVector3& v1,    // Line
                            hsPoint3& cp,                               // Output closest point on line to p0
                            uint32_t clamp);                              // Clamp on ends of segment (use Lower1/Upper1)

    // Return clamp flags for where clamped
    static uint32_t PointsOnLines(const hsPoint3& p0, const hsVector3& v0,// First line
                              const hsPoint3& p1, const hsVector3& v1,  // Second line
                              hsPoint3& cp0,                            // Output closest on line0 to line1
                              hsPoint3& cp1,                            // Output closest on line1 to line0
                              uint32_t clamp);                            // Clamp on ends

    //  Return true if p0 is inside or on sphere.
    static hsBool PointOnSphere(const hsPoint3& p0,                     // Point
                            const hsPoint3& center, float rad,       // Sphere
                            hsPoint3& cp);                              // Output closest on sphere to p0

    // Return true if p0 is inside box.
    static hsBool PointOnBox(const hsPoint3& p0,                        // Point
                            const hsPoint3& corner,                     // Box defined by corner point and 3 (presumably but 
                            const hsVector3& axis0,                     // not required) ortho axes.
                            const hsVector3& axis1,
                            const hsVector3& axis2,
                            hsPoint3& cp);

    // Return true if line intersects or is inside sphere.
    static hsBool PointOnSphere(const hsPoint3& p0, const hsVector3& v0,    // Line
                            const hsPoint3& center, float rad,           // Sphere
                            hsPoint3& cp,                                   // Output closest on sphere to p0, or entry point if line hits sphere
                            uint32_t clamp);

    // Return true if line intersects or is inside box.
    static hsBool PointOnBox(const hsPoint3& p0, const hsVector3& v0,   // Line
                            const hsPoint3& corner,                     // Box defined by corner point and 3 (presumably but 
                            const hsVector3& axis0,                     // not required) ortho axes.
                            const hsVector3& axis1,
                            const hsVector3& axis2,
                            hsPoint3& cp,
                            uint32_t clamp);

    // Return true if point inside (negative side) of plane
    static hsBool PointOnPlane(const hsPoint3& p0,
                            const hsPoint3& pPln, const hsVector3& n,
                            hsPoint3& cp);

    // Return true if line passes through plane.
    static hsBool PointOnPlane(const hsPoint3& p0, const hsVector3& v0,
                            const hsPoint3& pPln, const hsVector3& n,
                            hsPoint3& cp,
                            uint32_t clamp);

    // Two identical functions, just different wrapping. First version repacks
    // parameters and calls second.
    static hsBool PointBetweenBoxes(const hsPoint3& aCorner,
                                    const hsVector3& aAxis0,
                                    const hsVector3& aAxis1,
                                    const hsVector3& aAxis2,
                                    const hsPoint3& bCorner,
                                    const hsVector3& bAxis0,
                                    const hsVector3& bAxis1,
                                    const hsVector3& bAxis2,
                                    hsPoint3& cp0, hsPoint3& cp1);

    static hsBool PointBetweenBoxes(const hsPoint3& aCorner,
                                    const hsVector3* aAxes[3],
                                    const hsPoint3& bCorner,
                                    const hsVector3* bAxes[3],
                                    hsPoint3& cp0, hsPoint3& cp1);
};

#endif // plVolume_inc
