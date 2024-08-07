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
#include "plBumpMtl.h"
#include "plBumpMtlBasicPB.h"

#include "MaxMain/MaxCompat.h"


class BumpBasicDlgProc : public ParamMap2UserDlgProc
{
#if 1
protected:

public:
    BumpBasicDlgProc() {}
    ~BumpBasicDlgProc() { }
#endif

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        IParamBlock2 *pb = map->GetParamBlock();

        switch (msg)
        {
        case WM_INITDIALOG:
            return TRUE;
        }
        return FALSE;
    }
    void DeleteThis() override { }
};
static BumpBasicDlgProc gBumpBasicDlgProc;

static ParamBlockDesc2 gBumpBasicPB
(
    plBumpMtl::kBlkBasic, _T("basic"), IDS_PASS_BASIC, GetBumpMtlDesc(),//nullptr,
    P_AUTO_CONSTRUCT + P_AUTO_UI, plBumpMtl::kRefBasic,

    // UI
    IDD_BUMP_BASIC, IDS_PASS_BASIC, 0, 0, &gBumpBasicDlgProc,

    kBumpBasLayer,          _T("bumpLayer"),    TYPE_TEXMAP,        0, IDS_BASIC_AMB,
        p_ui,               TYPE_TEXMAPBUTTON, IDC_LAYER1,
        p_subtexno, 0,
        p_end,

    kBumpBasRunColor,       _T("runtimeColor"),     TYPE_RGBA,          P_ANIMATABLE, IDS_BASIC_RUNCOLOR,
//      p_ui,           TYPE_COLORSWATCH, IDC_LAYER_RUNCOLOR,
        p_ui,           TYPE_SPINNER, EDITTYPE_INT, IDC_INTEN_EDIT, IDC_INTEN_SPIN, 
                        IDC_DUMMY_EDIT1, IDC_DUMMY_SPIN1, IDC_DUMMY_EDIT2, IDC_DUMMY_SPIN2,
                        SPIN_AUTOSCALE,
        p_default,      Color(1,0,0),
        p_end,

    // Specularity
    kBumpBasSpecular,   _T("useSpec"),      TYPE_BOOL,      0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_SHADE_SPECULAR,
        p_end,

    p_end
);
ParamBlockDesc2 *GetBumpBasicPB() { return &gBumpBasicPB; }
