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
#ifndef hsPoint2_Defined
#define hsPoint2_Defined

#include "hsScalar.h"

#if __MWERKS__
    //  This guy disables MetroWerks' desire to only include a file once, which obviously gets
    //  in the way of our little HS_POINT2.inc trick
    #pragma once off
#endif

#define HS_POINT2_NAME  hsIntPoint2
#define HS_POINT2_TYPE      Int32
#include "HS_POINT2.inc"
};

#define HS_POINT2_NAME  hsFixedPoint2
#define HS_POINT2_TYPE      hsFixed
#include "HS_POINT2.inc"

    hsFixedPoint2&  operator=(const hsIntPoint2& src)
                {
                    this->fX    = hsIntToFixed(src.fX);
                    this->fY    = hsIntToFixed(src.fY);
                    return *this;
                }

    hsFixed Magnitude() const { return hsMagnitude32(fX, fY); }

    static hsFixed  Magnitude(hsFixed x, hsFixed y)
                {
                    return hsMagnitude32(x, y);
                }
    static hsFixed  Distance(const hsFixedPoint2& p1, const hsFixedPoint2& p2)
                {
                    return hsMagnitude32(p2.fX - p1.fX, p2.fY - p1.fY);
                }
};

#if HS_CAN_USE_FLOAT
    struct hsPolar {
        float       fRadius;
        float       fAngle;
    };

    #define HS_POINT2_NAME  hsFloatPoint2
    #define HS_POINT2_TYPE      float
    #include "HS_POINT2.inc"

        hsFloatPoint2&  operator=(const hsIntPoint2& src)
            {
                this->fX    = float(src.fX);
                this->fY    = float(src.fY);
                return *this;
            }

        friend hsFloatPoint2 operator*(const hsFloatPoint2& s, float t)
            {
                hsFloatPoint2   result;
                result.Set(s.fX * t, s.fY * t);
                return result;
            }
        friend hsFloatPoint2 operator*(float t, const hsFloatPoint2& s)
            {
                hsFloatPoint2   result;
                result.Set(s.fX * t, s.fY * t);
                return result;
            }

        hsFloatPoint2*  Grid(float period);
        hsBool  CloseEnough(const hsFloatPoint2* p, float tolerance) const;

        float       Magnitude() const { return hsFloatPoint2::Magnitude(fX, fY); }
        float       MagnitudeSquared() const { return fX * fX + fY * fY; }
        hsPolar*    ToPolar(hsPolar* polar) const;

        static float    Magnitude(float x, float y) { return hsSquareRoot(x * x + y * y); }
        static hsScalar Distance(const hsFloatPoint2& p1, const hsFloatPoint2& p2);
        static hsFloatPoint2    Average(const hsFloatPoint2& a, const hsFloatPoint2& b)
                        {
                            hsFloatPoint2   result;
                            result.Set((a.fX + b.fX) * float(0.5), (a.fY + b.fY) * float(0.5));
                            return result;
                        }
        static hsScalar ComputeAngle(const hsFloatPoint2& a, const hsFloatPoint2& b, const hsFloatPoint2& c);
    };
#endif

#if HS_SCALAR_IS_FIXED
    typedef hsFixedPoint2   hsPoint2;
#else
    typedef hsFloatPoint2   hsPoint2;
#endif

#endif  // hsPoint2_Defined

