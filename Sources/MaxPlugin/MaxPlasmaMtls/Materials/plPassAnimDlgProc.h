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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plPassAnimDlgProc - Base Animation Dlg Proc for plPassMtlBase           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plPassAnimDlgProc_h
#define _plPassAnimDlgProc_h

#include "plPassMtlBase.h"

class plAnimStealthNode;
class Interval;
class IParamMap2;
class ReferenceTarget;

class plPassAnimDlgProc : public ParamMap2UserDlgProc, public plMtlChangeCallback
{
    protected:
        // Combo itemdata values
        enum
        {
            kName,      // Name of an animation/loop
            kDefault,   // Default combo value
            kInvalid,   // Invalid entry (couldn't find)
        };

        plAnimStealthNode   *fCurrStealth;
        IParamMap2          *fCurrParamMap;

        bool    fInitingNames;
        HWND fhWnd;

    public:
        plPassAnimDlgProc();
        virtual ~plPassAnimDlgProc();

        INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
        void DeleteThis() override { }
        void SetThing(ReferenceTarget *m) override;
        void Update(TimeValue t, Interval& valid, IParamMap2* pmap) override;

        void    SegmentListChanged() override;

        static plPassAnimDlgProc    &Get();

    protected:
        // Set all the controls to their stored value
        void IInitControls(Animatable *anim, IParamBlock2 *pb);
        void IEnableGlobal(HWND hWnd, bool enable);

        void ILoadNames( IParamBlock2 *pb );

        void    IExposeStealthNode( HelperObject *stealth, IParamMap2 *thisMap );
        void    IUpdateSegmentSel( IParamMap2 *thisMap, bool clear = false );

        void    IEnableEaseStopPoints( IParamMap2 *pm, bool enable );
};

#endif //_plPassAnimDlgProc_h 