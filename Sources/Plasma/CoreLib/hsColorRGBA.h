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

#ifndef hsColorRGBA_inc
#define hsColorRGBA_inc

#include "hsStream.h"

struct hsColor32 {

    uint8_t   b, g, r, a;

    inline void SetARGB(uint8_t aa, uint8_t rr, uint8_t gg, uint8_t bb)
    {
        this->a = aa;
        this->r = rr;
        this->g = gg;
        this->b = bb;
    }

    //  Compatibility inlines, should be depricated
    inline void Set(uint8_t rr, uint8_t gg, uint8_t bb)
    {
        this->r = rr;
        this->g = gg;
        this->b = bb;
    }
    inline void Set(uint8_t aa, uint8_t rr, uint8_t gg, uint8_t bb)
    {
        this->SetARGB(aa, rr, gg, bb);
    }

    int operator==(const hsColor32& aa) const
    {
        return *(uint32_t*)&aa == *(uint32_t*)this;
    }
    int operator!=(const hsColor32& aa) { return !(aa == *this); }
};
typedef hsColor32 hsRGBAColor32;

struct hsColorRGBA {
    float        r,g,b,a;
    
    hsRGBAColor32 ToRGBA32() const;

    hsColorRGBA& Set(float red, float grn, float blu, float alp) { r = red; g = grn; b = blu; a = alp; return *this; }

    bool operator==(const hsColorRGBA&c) const { return (r==c.r)&&(g==c.g)&&(b==c.b)&&(a==c.a); }
    bool operator!=(const hsColorRGBA&c) const { return !(c == *this); }

    friend inline hsColorRGBA operator+(const hsColorRGBA& s, const hsColorRGBA& t);
    hsColorRGBA& operator+=(const hsColorRGBA& s);
    
    friend inline hsColorRGBA operator*(const hsColorRGBA& s, const hsColorRGBA& t);
    hsColorRGBA& operator*=(const hsColorRGBA& s);
    
    friend inline hsColorRGBA operator-(const hsColorRGBA& s, const hsColorRGBA& t);
    hsColorRGBA& operator-=(const hsColorRGBA& s);
    
    friend inline hsColorRGBA operator*(const hsColorRGBA& c, const float s);
    friend inline hsColorRGBA operator*(const float s, const hsColorRGBA& c);
    hsColorRGBA& operator*=(const float s);

    hsColorRGBA&    FromARGB32(uint32_t c);
    uint32_t          ToARGB32() const;
    uint32_t          ToARGB32Premultiplied() const;

    void Read(hsStream *stream);
    void Write(hsStream *stream) const;
};

inline void hsColorRGBA::Read(hsStream *s)
{
    r = s->ReadLEFloat();
    g = s->ReadLEFloat();
    b = s->ReadLEFloat();
    a = s->ReadLEFloat();
}
inline void hsColorRGBA::Write(hsStream *s) const
{
    s->WriteLEFloat(r);
    s->WriteLEFloat(g);
    s->WriteLEFloat(b);
    s->WriteLEFloat(a);
}

inline hsColorRGBA& hsColorRGBA::FromARGB32(uint32_t c)
{
    const float oo255 = 1.f / 255.f;
    a = float((c >> 24) & 0xff) * oo255;
    r = float((c >> 16) & 0xff) * oo255;
    g = float((c >> 8) & 0xff) * oo255;
    b = float((c >> 0) & 0xff) * oo255;
    return *this;
}

inline uint32_t hsColorRGBA::ToARGB32() const
{
    return (uint32_t(a * 255.99f) << 24)
        | (uint32_t(r * 255.99f) << 16)
        | (uint32_t(g * 255.99f) << 8)
        | (uint32_t(b * 255.99f) << 0);
}

inline uint32_t hsColorRGBA::ToARGB32Premultiplied() const
{
    return (uint32_t(a * 255.0f + 0.5f) << 24)
        | (uint32_t(a * r * 255.0f + 0.5f) << 16)
        | (uint32_t(a * g * 255.0f + 0.5f) << 8)
        | (uint32_t(a * b * 255.0f + 0.5f) << 0);
}

inline hsColorRGBA operator+(const hsColorRGBA& s, const hsColorRGBA& t)
{
    hsColorRGBA res;
    return res.Set(s.r + t.r, s.g + t.g, s.b + t.b, s.a + t.a);
}
inline hsColorRGBA& hsColorRGBA::operator+=(const hsColorRGBA& s)
{
    r += s.r;
    g += s.g;
    b += s.b;
    a += s.a;
    return *this;
}

inline hsColorRGBA operator*(const hsColorRGBA& s, const hsColorRGBA& t)
{
    hsColorRGBA res;
    return res.Set(s.r * t.r, s.g * t.g, s.b * t.b, s.a * t.a);
}
inline hsColorRGBA& hsColorRGBA::operator*=(const hsColorRGBA& s)
{
    r *= s.r;
    g *= s.g;
    b *= s.b;
    a *= s.a;
    return *this;
}

inline hsColorRGBA operator-(const hsColorRGBA& s, const hsColorRGBA& t)
{
    hsColorRGBA res;
    return res.Set(s.r - t.r, s.g - t.g, s.b - t.b, s.a - t.a);
}
inline hsColorRGBA& hsColorRGBA::operator-=(const hsColorRGBA& s)
{
    r -= s.r;
    g -= s.g;
    b -= s.b;
    a -= s.a;
    return *this;
}

inline hsColorRGBA operator*(const hsColorRGBA& t, const float s)
{
    hsColorRGBA res;
    return res.Set(s * t.r, s * t.g, s * t.b, s * t.a);
}
inline hsColorRGBA operator*(const float s, const hsColorRGBA&t)
{
    return t * s;
}
inline hsColorRGBA& hsColorRGBA::operator*=(const float s)
{
    r *= s;
    g *= s;
    b *= s;
    a *= s;
    return *this;
}

class hsColorOverride
{
public:
    enum {
        kNone,
        kModColor,
        kModAlpha,
        kModShade
    };
    hsColorRGBA         fShade;
    hsColorRGBA         fColor;
    unsigned int        fFlags;
};


#endif // hsColorRGBA_inc
