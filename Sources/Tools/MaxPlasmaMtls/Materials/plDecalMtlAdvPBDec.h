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
#include "plDecalMtl.h"
#include "plPassBaseParamIDs.h"

#include "MaxMain/MaxCompat.h"


using namespace plPassBaseParamIDs;

static ParamBlockDesc2 gDecalAdvPB
(
    plDecalMtl::kBlkAdv, _T("advanced"), IDS_PASS_ADV, GetDecalMtlDesc(),
    P_AUTO_CONSTRUCT + P_AUTO_UI, plDecalMtl::kRefAdv,

    // UI
    IDD_PASS_ADV, IDS_PASS_ADV, 0, APPENDROLL_CLOSED, nullptr,

    // Misc Properties
    kPBAdvWire,         _T("basicWire"),    TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_MISC_WIRE,
        p_end,
    kPBAdvMeshOutlines, _T("meshOutlines"), TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_MISC_MESHOUTLINES,
        p_end,
    kPBAdvTwoSided,     _T("twoSided"),     TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_MISC_TWOSIDED,
        p_end,

    // Shade properties
    kPBAdvSoftShadow,       _T("softShadow"),   TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_SHADE_SOFTSHADOW,
        p_end,
    kPBAdvNoProj,           _T("noProj"),       TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_SHADE_NO_PROJ,
        p_end,
    kPBAdvVertexShade,  _T("vertexShade"),  TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_SHADE_VERTEXSHADE,
        p_end,
    kPBAdvNoShade,      _T("noShade"),      TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_SHADE_NOSHADE,
        p_end,
    kPBAdvNoFog,            _T("noFog"),        TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_SHADE_NO_FOG,
        p_end,
    kPBAdvWhite,            _T("white"),        TYPE_BOOL,      0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_SHADE_WHITE,
        p_end,

    // Z Properties
    kPBAdvZOnly,        _T("zOnly"),        TYPE_BOOL,      0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_Z_ZONLY,
        p_end,
    kPBAdvZClear,       _T("zClear"),       TYPE_BOOL,      0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_Z_ZCLEAR,
        p_end,
    kPBAdvZNoRead,      _T("zNoRead"),      TYPE_BOOL,      0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_Z_ZNOREAD,
        p_end,
    kPBAdvZNoWrite,     _T("zNoWrite"),     TYPE_BOOL,      0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_Z_ZNOWRITE,
        p_default,      TRUE,
        p_end,
    kPBAdvZInc,         _T("zInc"),         TYPE_BOOL,      0, 0,
        p_ui,           TYPE_SINGLECHEKBOX, IDC_Z_INC,
        p_default,      TRUE,
        p_end,

    kPBAdvAlphaTestHigh,            _T("aTestHigh"),            TYPE_BOOL,      0, 0,
        p_default,      FALSE,
        p_end,

    p_end
);
ParamBlockDesc2 *GetDecalAdvPB() { return &gDecalAdvPB; }