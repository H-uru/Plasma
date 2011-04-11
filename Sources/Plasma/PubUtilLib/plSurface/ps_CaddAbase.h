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


static const UInt32 ps_CaddAbaseByteLen = 68;

static const UInt8 ps_CaddAbaseCodes[] = {
	0x1,	0x1,	0xff,	0xff,
	0x42,	0x0,	0x0,	0x0,
	0x0,	0x0,	0xf,	0xb0,
	0x42,	0x0,	0x0,	0x0,
	0x1,	0x0,	0xf,	0xb0,
	0x2,	0x0,	0x0,	0x0,
	0x0,	0x0,	0x7,	0x80,
	0x0,	0x0,	0xe4,	0xb0,
	0x1,	0x0,	0xe4,	0xb0,
	0x1,	0x0,	0x0,	0x40,
	0x0,	0x0,	0x8,	0x80,
	0x0,	0x0,	0xe4,	0xb0,
	0x5,	0x0,	0x0,	0x0,
	0x0,	0x0,	0xf,	0x80,
	0x0,	0x0,	0xe4,	0x80,
	0x0,	0x0,	0xe4,	0x90,
	0xff,	0xff,	0x0,	0x0
	};

static const plShaderDecl ps_CaddAbaseDecl("sha/ps_CaddAbase.inl", ps_CaddAbase, ps_CaddAbaseByteLen, ps_CaddAbaseCodes);

static const plShaderRegister ps_CaddAbaseRegister(&ps_CaddAbaseDecl);

