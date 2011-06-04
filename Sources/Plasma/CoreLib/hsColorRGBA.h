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

#ifndef hsColorRGBA_inc
#define hsColorRGBA_inc

#include "hsScalar.h"
#include "hsStream.h"

struct hsColorRGBA {
	hsScalar		r,g,b,a;
	
	hsRGBAColor32 ToRGBA32() const;

	hsColorRGBA& Set(hsScalar red, hsScalar grn, hsScalar blu, hsScalar alp) { r = red; g = grn; b = blu; a = alp; return *this; }

	hsBool operator==(const hsColorRGBA&c) const { return (r==c.r)&&(g==c.g)&&(b==c.b)&&(a==c.a); }
	hsBool operator!=(const hsColorRGBA&c) const { return !(c == *this); }

	friend inline hsColorRGBA operator+(const hsColorRGBA& s, const hsColorRGBA& t);
	hsColorRGBA& operator+=(const hsColorRGBA& s);
	
	friend inline hsColorRGBA operator*(const hsColorRGBA& s, const hsColorRGBA& t);
	hsColorRGBA& operator*=(const hsColorRGBA& s);
	
	friend inline hsColorRGBA operator-(const hsColorRGBA& s, const hsColorRGBA& t);
	hsColorRGBA& operator-=(const hsColorRGBA& s);
	
	friend inline hsColorRGBA operator*(const hsColorRGBA& c, const hsScalar s);
	friend inline hsColorRGBA operator*(const hsScalar s, const hsColorRGBA& c);
	hsColorRGBA& operator*=(const hsScalar s);

	hsColorRGBA&	FromARGB32(UInt32 c);
	UInt32			ToARGB32() const;

	void Read(hsStream *stream);
	void Write(hsStream *stream) const;
};

inline void hsColorRGBA::Read(hsStream *s)
{
	r = s->ReadSwapScalar();
	g = s->ReadSwapScalar();
	b = s->ReadSwapScalar();
	a = s->ReadSwapScalar();
}
inline void hsColorRGBA::Write(hsStream *s) const
{
	s->WriteSwapScalar(r);
	s->WriteSwapScalar(g);
	s->WriteSwapScalar(b);
	s->WriteSwapScalar(a);
}

inline hsColorRGBA& hsColorRGBA::FromARGB32(UInt32 c)
{
	const hsScalar oo255 = 1.f / 255.f;
	a = hsScalar((c >> 24) & 0xff) * oo255;
	r = hsScalar((c >> 16) & 0xff) * oo255;
	g = hsScalar((c >> 8) & 0xff) * oo255;
	b = hsScalar((c >> 0) & 0xff) * oo255;
	return *this;
}

inline UInt32 hsColorRGBA::ToARGB32() const
{
	return (UInt32(a * 255.99f) << 24)
		| (UInt32(r * 255.99f) << 16)
		| (UInt32(g * 255.99f) << 8)
		| (UInt32(b * 255.99f) << 0);
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

inline hsColorRGBA operator*(const hsColorRGBA& t, const hsScalar s)
{
	hsColorRGBA res;
	return res.Set(s * t.r, s * t.g, s * t.b, s * t.a);
}
inline hsColorRGBA operator*(const hsScalar s, const hsColorRGBA&t)
{
	return t * s;
}
inline hsColorRGBA& hsColorRGBA::operator*=(const hsScalar s)
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
	hsColorRGBA			fShade;
	hsColorRGBA			fColor;
	hsBool				fFlags;
};


#endif // hsColorRGBA_inc
