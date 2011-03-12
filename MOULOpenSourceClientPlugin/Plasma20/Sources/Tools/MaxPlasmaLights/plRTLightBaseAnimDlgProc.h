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
//	Animation Rollout Dialog Proc Class Definition  						 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.6.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#if 0//ndef _plRTLightBaseAnimDlgProc_h
#define _plRTLightBaseAnimDlgProc_h

#include "../MaxComponent/plNotetrackDlg.h"

///////////////////////////////////////////////////////////////////////////////
//// Dialog Proc for Animation Rollout ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTLightBaseAnimDlgProc : public ParamMap2UserDlgProc
{
	protected:
		plNoteTrackDlg fNoteTrackDlg;

		HWND		fhWnd;

		static plRTLightBaseAnimDlgProc		fInstance;
		static const char					*kDecalNameNone;

	public:
		BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
		void DeleteThis() { fNoteTrackDlg.DeleteCache(); }
		void SetThing( ReferenceTarget *m );

		static plRTLightBaseAnimDlgProc	*Instance() { return &fInstance; }

	protected:
		void IInitControls( Animatable *anim, IParamBlock2 *pb );

};


#endif //_plRTLightBaseAnimDlgProc_h

