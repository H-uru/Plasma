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
//	plStencil.h - Header for various stencil settings and enums              //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	5.17.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plStencil_h
#define _plStencil_h

#include "hsTypes.h"


//// Stencil Caps /////////////////////////////////////////////////////////////

class plStencilCaps
{
	public:

		enum Depths
		{
			kDepth1Bit	= 0x01,
			kDepth4Bits	= 0x02,
			kDepth8Bits	= 0x04
		};

		enum CompareFuncs
		{
			kCmpNever = 0,
			kCmpLessThan,			
			kCmpEqual,
			kCmpLessThanOrEqual,
			kCmpGreaterThan,
			kCmpNotEqual,
			kCmpGreaterThanOrEqual,
			kCmpAlways

		};

		enum Ops
		{
			kOpKeep			= 0x01,
			kOpSetToZero	= 0x02,
			kOpReplace		= 0x04,
			kOpIncClamp		= 0x08,
			kOpDecClamp		= 0x10,
			kOpInvert		= 0x20,
			kOpIncWrap		= 0x40,
			kOpDecWrap		= 0x80

		};

		hsBool		fIsSupported;
		UInt8		fSupportedDepths;
		UInt8		fSupportedOps;
};

#endif // _plStencil_h
