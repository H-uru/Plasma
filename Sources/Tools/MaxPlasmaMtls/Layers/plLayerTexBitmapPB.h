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
#ifndef PL_LAYERTEXBITMAPPB_H
#define PL_LAYERTEXBITMAPPB_H

// Param ID's
enum
{
	kBmpUseBitmap,
	kBmpBitmap,

	// Cropping/Placement
	kBmpApply,
	kBmpCropPlace,
	kBmpClipU,
	kBmpClipV,
	kBmpClipW,
	kBmpClipH,

	// Misc
	kBmpDiscardColor,
	kBmpInvertColor,
	kBmpDiscardAlpha,
	kBmpInvertAlpha,

	// Texture quality
	kBmpNonCompressed,
	kBmpScaling,

	// Mipmap
	kBmpNoFilter,
	kBmpMipBlur,
	kBmpMipBias,
	kBmpMipBiasAmt,

	// Max only
	kBmpMonoOutput,
	kBmpRGBOutput,

	// Detail
	kBmpUseDetail,
	kBmpDetailStartSize,
	kBmpDetailStopSize,
	kBmpDetailStartOpac,
	kBmpDetailStopOpac,

	// New export size controls
	kBmpExportWidth,
	kBmpExportHeight,
	kBmpExportLastWidth,		// Annoying fields, these two, but they're necessary
	kBmpExportLastHeight,		// for clamping the spinners to powers of 2

	// Keep a sysmem copy of the texture
	kBmpNoDiscard
};

enum
{
	kScalingAny,
	kScalingHalf,
	kScalingNone
};

#endif //PL_LAYERTEXBITMAPPB_H