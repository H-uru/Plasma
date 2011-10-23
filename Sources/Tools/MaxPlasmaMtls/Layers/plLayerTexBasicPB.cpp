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
#include "hsTypes.h"
#include "plLayerTex.h"
#include "plLayerTexBasicPB.h"
#include "../resource.h"

class BasicDlgProc;
extern BasicDlgProc gBasicDlgProc;

static ParamBlockDesc2 gBasicParamBlk
(
    plLayerTex::kBlkBasic, _T("basicLayer"),  0, GetLayerTexDesc(),//NULL,
    P_AUTO_CONSTRUCT + P_AUTO_UI, plLayerTex::kRefBasic,

    // UI
    IDD_LAYER_BASIC, IDS_LAYER_BASIC, 0, 0, &gBasicDlgProc,

    // Usage
    kBasicUsage,        _T("usage"),        TYPE_INT,       0, 0,
        end,

    end
);
ParamBlockDesc2 *GetBasicBlk() { return &gBasicParamBlk; }

static const char *kUsageTypes[] =
{
    "None",
    "Base Texture",
    "Detail",
    "Grime",
    "Map Blend",
    "Highlight/Specular",
    "Alpha Mask",
    "Shadow/Light Map",
    "Helper Object",
    "Best Guess"
};

class BasicDlgProc : public ParamMap2UserDlgProc
{
public:
    virtual BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        IParamBlock2 *pb = map->GetParamBlock();

        switch (msg)
        {
        case WM_INITDIALOG:
            {
                HWND hUsage = GetDlgItem(hWnd, IDC_USAGE_TYPE);
                for (int i = 0; i < kUsageNumTypes; i++)
                    SendMessage(hUsage, CB_ADDSTRING, 0, (LPARAM)kUsageTypes[i]);
                SendMessage(hUsage, CB_SETCURSEL, pb->GetInt(kBasicUsage), 0);
            }
            break;

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
                switch (LOWORD(wParam))
                {
                case IDC_USAGE_TYPE:
                    {
                        int cur = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                        if (LOWORD(wParam) == IDC_USAGE_TYPE)
                            pb->SetValue(kBasicUsage, t, cur);
                        return true;
                    }
                    break;
                }
                break;
            }
            break;
        }

        return false;
    }
    virtual void DeleteThis() {};
};
static BasicDlgProc gBasicDlgProc;
