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
//	plPassAnimDlgProc - Base Animation Dlg Proc for plPassMtlBase			//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plPassAnimDlgProc_h
#define _plPassAnimDlgProc_h

#include "plPassMtlBase.h"
#include "iparamm2.h"

class plAnimStealthNode;
class IParamMap2;

class plPassAnimDlgProc : public ParamMap2UserDlgProc, public plMtlChangeCallback
{
	protected:
		// Combo itemdata values
		enum
		{
			kName,		// Name of an animation/loop
			kDefault,	// Default combo value
			kInvalid,	// Invalid entry (couldn't find)
		};

		plAnimStealthNode	*fCurrStealth;
		IParamMap2			*fCurrParamMap;

		bool	fInitingNames;
		HWND fhWnd;

	public:
		plPassAnimDlgProc();
		virtual ~plPassAnimDlgProc();

		BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void DeleteThis() {}
		void SetThing(ReferenceTarget *m);
		virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap);

		void	SegmentListChanged( void );

		static plPassAnimDlgProc	&Get( void );

	protected:
		// Set all the controls to their stored value
		void IInitControls(Animatable *anim, IParamBlock2 *pb);
		void IEnableGlobal(HWND hWnd, hsBool enable);

		void ILoadNames( IParamBlock2 *pb );

		void	IExposeStealthNode( HelperObject *stealth, IParamMap2 *thisMap );
		void	IUpdateSegmentSel( IParamMap2 *thisMap, hsBool clear = false );

		void	IEnableEaseStopPoints( IParamMap2 *pm, bool enable );
};

#endif //_plPassAnimDlgProc_h 