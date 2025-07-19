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

#include <cmath>

struct hsPolar {
    float       fRadius;
    float       fAngle;
};

struct hsPoint2 {
    float  fX, fY;

    hsPoint2& Set(float x, float y)
    {
        fX = x;
        fY = y;
        return *this;
    }
    hsPoint2& operator+=(const hsPoint2& s)
    {
        this->fX += s.fX;
        this->fY += s.fY;
        return *this;
    }
    hsPoint2& operator-=(const hsPoint2& s)
    {
        this->fX -= s.fX;
        this->fY -= s.fY;
        return *this;
    }

    int operator==(const hsPoint2& ss) const
    {
        return (ss.fX == fX && ss.fY == fY);
    }
    int operator!=(const hsPoint2& ss)
    {
        return !(ss == *this);
    }

    friend hsPoint2 operator+(const hsPoint2& s, const hsPoint2& t)
    {
        hsPoint2  result;
        result.Set(s.fX + t.fX, s.fY + t.fY);
        return result;
    }
    friend hsPoint2 operator-(const hsPoint2& s, const hsPoint2& t)
    {
        hsPoint2  result;
        result.Set(s.fX - t.fX, s.fY - t.fY);
        return result;
    }
    friend hsPoint2 operator-(const hsPoint2& s)
    {
        hsPoint2  result = { -s.fX, -s.fY };
        return result;
    }

    friend hsPoint2 operator*(const hsPoint2& s, float t)
    {
        hsPoint2   result;
        result.Set(s.fX * t, s.fY * t);
        return result;
    }
    friend hsPoint2 operator*(float t, const hsPoint2& s)
    {
        hsPoint2   result;
        result.Set(s.fX * t, s.fY * t);
        return result;
    }

    hsPoint2*  Grid(float period);
    bool       CloseEnough(const hsPoint2* p, float tolerance) const;

    float       MagnitudeSquared() const { return fX * fX + fY * fY; }
    float       Magnitude() const { return Magnitude(fX, fY); }
    hsPolar*    ToPolar(hsPolar* polar) const;

    static float Magnitude(float x, float y) { return sqrt(x * x + y * y); }
    static float Distance(const hsPoint2& p1, const hsPoint2& p2);
    static hsPoint2 Average(const hsPoint2& a, const hsPoint2& b)
    {
        hsPoint2   result;
        result.Set((a.fX + b.fX) * float(0.5), (a.fY + b.fY) * float(0.5));
        return result;
    }
    static float ComputeAngle(const hsPoint2& a, const hsPoint2& b, const hsPoint2& c);
};

#endif  // hsPoint2_Defined

