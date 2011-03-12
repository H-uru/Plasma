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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plCAnimParamBlock.h - Common animation paramblock and associated code	//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	10.3.02 mcn	- Created from the gory destruction of the separated anim	//
//				  component and material animations.						//
//																			//
//// Notes ///////////////////////////////////////////////////////////////////
//																			//
//	This file is shared currently between MaxComponent and MaxPlasmaMtl.	//
//	Since the latter links to virtually nothing, this header MUST remain	//
//	MAX-pure; no reference to components or any other Plasma-specific stuff	//
//	above basically nucleusLib. Once we merge materials back into the main	//
//	plugins, this restriction can go away.									//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plCAnimParamBlock_h
#define _plCAnimParamBlock_h

#include "Max.h"
#include "iparamb2.h"
#include "iparamm2.h"


/// Note: the namespace is so our enums and such don't clash with anything
/// else
namespace plCAnimPB
{

//// ParamBlock IDs //////////////////////////////////////////////////////////
//	IDs for the common paramblock def below

enum Rollouts
{
	kRollMain,
	kRollEase
};

enum 
{
	kAnimRadio_DEAD,
	kAnimAutoStart,			// Start the Animation on load  (V2)
	kAnimLoop,				// Start Looping at Begin Location
	kAnimBegin_DEAD,
	kAnimEnd_DEAD,
	kAnimLoopSegCkBx_DEAD,
	kAnimLoopSegBeg_DEAD,
	kAnimLoopSegEnd_DEAD,
	kAnimName,				// Name of the notetrack animation to play
	kAnimLoopSegBegBox_DEAD,
	kAnimLoopSegEndBox_DEAD,
	kAnimUseGlobal,
	kAnimGlobalName,
	kAnimLoopName,			// Name of the notetrack specified loop
	kAnimEaseInType,
	kAnimEaseOutType,
	kAnimEaseInLength,
	kAnimEaseOutLength,
	kAnimEaseInMin,
	kAnimEaseInMax,
	kAnimEaseOutMin,
	kAnimEaseOutMax,
	kAnimPhysAnim,
};

//// Static ParamBlock Accessor //////////////////////////////////////////////
//	So we can ensure we only have one definition of our base paramBlock, but
//	can access it multiple times without having to worry about which is the
//	first. Defined here so the macro can get to it; don't reference it
//	directly!

class plPBBaseDec
{
	public:
		static int	kPlComponentBlkID;
		static int	kPlComponentRefID;

		static ParamBlockDesc2		fAnimBlock;
};

//// ParamBlock Macro ////////////////////////////////////////////////////////
//	Use this macro to create your paramblock for your animation object. This
//	way, all the paramblocks for animation info will be identical!
//
//	Ex. Usage: kDefineAnimPB( gAnimBlock, &gAnimDesc, PASS_ANIM, PASS_ANIM_EASE, gAnimMainProc, gAnimEaseProc );

#define kDefineAnimPB( blockName, descPtr, mainResIDName, easeResIDName, mainProc, easeProc ) \
	static ParamBlockDesc2 blockName##( \
										\
		plPBBaseDec::kPlComponentBlkID, _T("animation"), 0, descPtr, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plPBBaseDec::kPlComponentRefID, \
		2,	/* # rollouts */			\
		plCAnimPB::kRollMain, IDD_##mainResIDName, IDS_##mainResIDName, 0, 0, mainProc,					\
		plCAnimPB::kRollEase, IDD_##easeResIDName, IDS_##easeResIDName, 0, 0, easeProc,					\
		&plPBBaseDec::fAnimBlock,		/* ParamBlock to include */									\
		end,																						\
	);
		


};	// namespace

#endif //_plCAnimParamBlock_h
