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
#ifndef plAvDefs_h
#define plAvDefs_h

/** This is where we keep definitions that are used by different avatar classes.
	Typically, these don't change very much, so there's a compile-time savings
	if we don't keep them with class headers, which tend to be more volatile.
*/

#include "../CoreLib/hsGeometry3.h"			// for the hsVector3, below


// animation alignment types (used by "calcanimalignment")
// when starting oneshots, we need to first get your handle into a position to play the oneshot properly
// there are different approaches to calculating this aligment
/** \enum Alignment
	There are several different ways to get into position to play a detailed
	interaction animation. Interactions always have an associated "seek point"
	indicating an alignment point, but there are several different types of
	alignment. */
enum plAvAlignment {
	kAlignHandle,			/// align our handle with the seek point
	kAlignHandleAnimEnd,	/** align our handle at the seek point "minus" the animation
								i.e. after seeking and then playing the animation, our
								handle should wind up aligned with the seek point. */
	kAlignWorld,			/** align our handle with the world origin; the animation
								is defined in global space */
	kAlignBone,				/// align a specific bone with the seek point
	kAlignBoneAnimEnd,		/** align our handle so that the bone is at the seek point
								after the animation has finished playing. */
	kAlignEnsure16 = 0xffff
};

const hsVector3 kAvatarUp(0,0,1);
const hsVector3 kAvatarForward(0,-1,0);
const hsVector3 kAvatarRight(-1,0,0);

/** \enum PinType
	Tells us what category of animation this channel affects.
	Primarly used to determine if two channels are competing,
	which you can't tell strictly from the type of data the
	channel handles. */
enum plAGPinType
{
	kAGPinUnknown,		// this applicator hasn't decided its pin type
	kAGPinTransform,		// this applicator munches the entire transform
	kNumPinTypes
};

#define kAGMaxBlendPriority 0x0fffffff
#define kAGMinBlendPriority 0x00000000
#define kAGMedBlendPriority 0x0000ffff


#endif