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
//	plCAnimParamBlock.cpp - Common animation paramblock and associated code	//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	10.3.02 mcn	- Created from the gory destruction of the separated anim	//
//				  component and material animations.						//
//																			//
//// Notes ///////////////////////////////////////////////////////////////////
//																			//
//	This file is shared currently between MaxComponent and MaxPlasmaMtl.	//
//	Since they are in separate plugins currently, you MUST include this		//
//	.cpp file in both projects so that they can both compile in the code.	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "plCAnimParamBlock.h"

#include "../plInterp/plAnimEaseTypes.h"
#include "../MaxPlasmaMtl/plPassMtlBase.h"


//// Static ParamBlock Template //////////////////////////////////////////////
//	This is the base paramBlock for all animation paramBlocks. The macro that
//	creates the latter just includes this one (defined here to keep the .h
//	file simple, among other reasons)

// Since we can't reference plComponent directly, this #define is just so we
// remember what the hell it is. If this changes in plComponentBase.h, change
// it here too.
int plCAnimPB::plPBBaseDec::kPlComponentBlkID = 0;
int plCAnimPB::plPBBaseDec::kPlComponentRefID = 0;


// Maybe plEaseAccessor should be moved here?
static plEaseAccessor sAnimCompEaseAccessor( plCAnimPB::plPBBaseDec::kPlComponentBlkID, 
											plCAnimPB::kAnimEaseInMin, plCAnimPB::kAnimEaseInMax, plCAnimPB::kAnimEaseInLength,
											plCAnimPB::kAnimEaseOutMin, plCAnimPB::kAnimEaseOutMax, plCAnimPB::kAnimEaseOutLength);

ParamBlockDesc2 plCAnimPB::plPBBaseDec::fAnimBlock
(
	kPlComponentBlkID, _T( "animation" ), 0, NULL, 0/*P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP*/, kPlComponentRefID,

	// map rollups (don't define Procs, they'll just be overridden in the macro defs anyway)
/*	2,
	plCAnimPB::kRollMain, IDD_COMP_ANIM, IDS_COMP_ANIM, 0, 0, nil,
	plCAnimPB::kRollEase, IDD_COMP_ANIM_EASE, IDS_COMP_ANIM_EASE, 0, APPENDROLL_CLOSED, nil,
*/

	// Anim Main rollout
	plCAnimPB::kAnimAutoStart, _T("autoStart"),	TYPE_BOOL,		0, 0,
		p_ui,		plCAnimPB::kRollMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_AUTOSTART_CKBX,
		p_default,	FALSE,
		end,
	plCAnimPB::kAnimLoop,		_T("loop"),			TYPE_BOOL,		0, 0,
		p_ui,		plCAnimPB::kRollMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_LOOP_CKBX,
		p_default,	FALSE,
		end,
	plCAnimPB::kAnimName,		_T("animName"),		TYPE_STRING,	0, 0,
		end,
	plCAnimPB::kAnimUseGlobal,		_T("UseGlobal"),	TYPE_BOOL,	0, 0,
		p_default,	FALSE,
		p_ui,	plCAnimPB::kRollMain, TYPE_SINGLECHEKBOX,	IDC_COMP_ANIM_USE_GLOBAL,
		end,
	plCAnimPB::kAnimGlobalName,	_T("GlobalName"),	TYPE_STRING,	0,	0,
		p_default, _T(""),
		end,
	plCAnimPB::kAnimLoopName,	_T("loopName"),		TYPE_STRING,	0, 0,
		end,
	plCAnimPB::kAnimPhysAnim,	_T("PhysAnim"),		TYPE_BOOL,	0, 0,
		p_default, TRUE,
		p_ui,	plCAnimPB::kRollMain, TYPE_SINGLECHEKBOX, IDC_COMP_ANIM_PHYSANIM,
		end,

	// Anim Ease rollout
	plCAnimPB::kAnimEaseInType,	_T("easeInType"),	TYPE_INT,		0, 0,
		p_ui,		plCAnimPB::kRollEase, TYPE_RADIO, 3, IDC_COMP_ANIM_EASE_IN_NONE, IDC_COMP_ANIM_EASE_IN_CONST_ACCEL, IDC_COMP_ANIM_EASE_IN_SPLINE,
		p_vals,		plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
		p_default,	plAnimEaseTypes::kNoEase,
		end,
	plCAnimPB::kAnimEaseInLength,	_T("easeInLength"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	plCAnimPB::kRollEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_IN_TIME, IDC_COMP_ANIM_EASE_IN_TIME_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	plCAnimPB::kAnimEaseInMin,		_T("easeInMin"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	plCAnimPB::kRollEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_IN_MIN, IDC_COMP_ANIM_EASE_IN_MIN_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	plCAnimPB::kAnimEaseInMax,	_T("easeInMax"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	plCAnimPB::kRollEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_IN_MAX, IDC_COMP_ANIM_EASE_IN_MAX_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,

	plCAnimPB::kAnimEaseOutType,	_T("easeOutType"),	TYPE_INT,		0, 0,
		p_ui,		plCAnimPB::kRollEase, TYPE_RADIO, 3, IDC_COMP_ANIM_EASE_OUT_NONE, IDC_COMP_ANIM_EASE_OUT_CONST_ACCEL, IDC_COMP_ANIM_EASE_OUT_SPLINE,
		p_vals,		plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
		p_default,	plAnimEaseTypes::kNoEase,
		end,
	plCAnimPB::kAnimEaseOutLength,	_T("easeOutLength"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	kRollEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_OUT_TIME, IDC_COMP_ANIM_EASE_OUT_TIME_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	plCAnimPB::kAnimEaseOutMin,		_T("easeOutMin"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	plCAnimPB::kRollEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_OUT_MIN, IDC_COMP_ANIM_EASE_OUT_MIN_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,
	plCAnimPB::kAnimEaseOutMax,	_T("easeOutMax"),	TYPE_FLOAT,		0, 0,	
		p_default, 1.0,
		p_range, 0.1, 99.0,
		p_ui,	plCAnimPB::kRollEase, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_ANIM_EASE_OUT_MAX, IDC_COMP_ANIM_EASE_OUT_MAX_SPIN, 1.0,
		p_accessor, &gAnimCompEaseAccessor,
		end,

	end
);

