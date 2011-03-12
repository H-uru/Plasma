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

#ifndef plBumpMapGen_inc
#define plBumpMapGen_inc

class plMipmap;

class plBumpMapGen
{
public:
	enum {
		// output signed values with 0=>-1 and 255=>1, 127=>0, default is 2's complement, 0=>0, 255=>-1, 127=>1
		kBias				= 0x1,
		kMaximize			= 0x2,
		kNormalize			= 0x4,
		kScaleHgtByAlpha	= 0x8,
		kBubbleTest			= 0x10
	};
	static		plMipmap* QikBumpMap(plMipmap* dst, const plMipmap* src, UInt32 mask, UInt32 flags);

	static		plMipmap* QikNormalMap(plMipmap* dst, const plMipmap* src, UInt32 mask, UInt32 flags, hsScalar smooth=1.f); // higher smooth means less bumpy, valid range [0..inf].

	static		plMipmap* TwosCompToBias(plMipmap* dst);
	static		plMipmap* MakeCompatibleBlank(const plMipmap* src);
};

#endif // plBumpMapGen_inc
