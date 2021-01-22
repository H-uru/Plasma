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

#ifndef plTriUtils_inc
#define plTriUtils_inc

struct hsPoint3;
struct hsVector3;

class plTriUtils
{
public:
    enum Bary {
        kInsideTri      = 0x0,
        kOnVertex0      = 0x1,
        kOnVertex1      = 0x2,
        kOnVertex2      = 0x4,
        kOnVertex       = kOnVertex0 | kOnVertex1 | kOnVertex2,
        kOnEdge01       = 0x8,
        kOnEdge12       = 0x10,
        kOnEdge02       = 0x20,
        kOnEdge         = kOnEdge01 | kOnEdge12 | kOnEdge02,

        kOutsideTri     = 0x100,

        kDegenerateTri  = 0x200,
    };


protected:
    Bary IComputeBarycentric(const hsVector3& v12, float invLenSq12, const hsVector3& v0, const hsVector3& v1, hsPoint3& out);

    int ISelectAxis(const hsVector3& norm);
    bool IFastBarycentric(int iAx, const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsPoint3&p, hsPoint3& out);
public:

    Bary ComputeBarycentricProjection(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, hsPoint3&p, hsPoint3& out);
    Bary ComputeBarycentric(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsPoint3&p, hsPoint3& out);


    bool FastBarycentricProjection(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, hsPoint3&p, hsPoint3& out);
    bool FastBarycentric(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsPoint3&p, hsPoint3& out);

    bool ProjectOntoPlane(const hsVector3& norm, float dist, hsPoint3& p);
    bool ProjectOntoPlane(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, hsPoint3& p);
    bool ProjectOntoPlaneAlongVector(const hsVector3& norm, float dist, const hsVector3& vec, hsPoint3& p);
    bool ProjectOntoPlaneAlongVector(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsVector3& vec, hsPoint3& p);

};

#endif // plTriUtils_inc
