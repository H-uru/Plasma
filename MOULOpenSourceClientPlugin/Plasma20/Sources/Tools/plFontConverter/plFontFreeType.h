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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plFontFreeType Functions												 //
//	Basically our cheat to allow importing fonts via FreeType2 into our		 //
//	plFont system without having to make plFont.cpp reliant on FreeType2.	 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "plFont.h"

class plFontFreeType : public plFont
{
	public:

		struct Options
		{
			UInt8	fSize;
			hsBool	fUseKerning;
			UInt8	fBitDepth;
			UInt32	fScreenRes;
			UInt32	fMaxCharLimit;

			Options() { fSize = 12; fUseKerning = false; fBitDepth = 1; fScreenRes = 96; fMaxCharLimit = 255; }
		};

		hsBool	ImportFreeType( const char *fontPath, Options *options, plBDFConvertCallback *callback );
};