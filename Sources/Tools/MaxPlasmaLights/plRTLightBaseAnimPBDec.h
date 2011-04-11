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
//	Animation Rollout ParamBlock for Runtime Lights							 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.3.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#if 0//ndef _plRTLightBaseAnimPB_h
#define _plRTLightBaseAnimPB_h

///////////////////////////////////////////////////////////////////////////////
//// Animation Rollout ParamBlock /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ParamBlockDesc2 plRTLightBaseAnimPB
(
	plRTLightBase::kBlkAnim, _T("anim"), IDS_LIGHT_ANIM, nil,	// To be added manually
	P_AUTO_CONSTRUCT + P_AUTO_UI, plRTLightBase::kRefAnimParams,

	// UI
	IDD_LIGHT_ANIM, IDS_LIGHT_ANIM, 0, APPENDROLL_CLOSED, plRTLightBaseAnimDlgProc::Instance(),

	plRTLightBase::kAnimName,		_T("animName"),		TYPE_STRING,		0, 0,
		end,

	plRTLightBase::kAnimAutoStart,	_T("autoStart"),	TYPE_BOOL,			0, 0,
		end,

	plRTLightBase::kAnimLoop,		_T("loop"),			TYPE_BOOL,			0, 0,
		end,
	plRTLightBase::kAnimLoopName,	_T("loopName"),		TYPE_STRING,		0, 0,
		end,

	end
);


#endif //_plRTLightBaseAnimPB_h

