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
#include "plDecalMtlLayersPB.h"

#include "../Layers/plLayerTex.h"

class DecalMtlLayersAccessor;
extern DecalMtlLayersAccessor gLayersAccessor;

class LayersDlgProc;
extern LayersDlgProc gLayersDlgProc;

static ParamBlockDesc2 gDecalMtlLayersPB
(
    plDecalMtl::kBlkLayers, _T("layers"), IDS_PASS_LAYERS, GetDecalMtlDesc(),
    P_AUTO_CONSTRUCT + P_AUTO_UI, plDecalMtl::kRefLayers,

    // UI
    IDD_PASS_LAYERS, IDS_PASS_LAYERS, 0, 0, nullptr,

    kDecalLayBase,          _T("baseLayer"),    TYPE_TEXMAP,        0, IDS_BASIC_AMB,
        p_ui,               TYPE_TEXMAPBUTTON, IDC_LAYER1,
        p_subtexno, 0,
        p_end,

    kDecalLayOutputBlend,   _T("outputBlend"),  TYPE_INT,           0, 0,
        p_ui,           TYPE_RADIO, 3, IDC_OUTPUTB_NONE, IDC_OUTPUTB_ALPHA, IDC_OUTPUTB_ADD,
        p_vals,         plPassMtlBase::kBlendNone, plPassMtlBase::kBlendAlpha, plPassMtlBase::kBlendAdd,
        p_default,      plPassMtlBase::kBlendNone,
        p_end,

    kDecalLayTopOn,         _T("topLayerOn"),   TYPE_BOOL,          0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_TOP_ON,
        p_default,          FALSE,
        p_enable_ctrls,     3, kDecalLayTop, kDecalLayBlend, kDecalLayOutputAlpha,
        p_end,
    kDecalLayTop,           _T("topLayer"),     TYPE_TEXMAP,        0, 0,
        p_ui,               TYPE_TEXMAPBUTTON, IDC_LAYER2,
        p_subtexno, 1,
        p_end,

    kDecalLayBlend,         _T("layerBlend"),   TYPE_INT,           0, 0,
        p_ui,               TYPE_RADIO, 3, IDC_LAYER_ALPHA, IDC_LAYER_ADD, IDC_LAYER_MULTIPLY,
        p_vals,             plPassMtlBase::kBlendAlpha, plPassMtlBase::kBlendAdd, plPassMtlBase::kBlendMult,
        p_default,          plPassMtlBase::kBlendAdd,
        p_end,

    kDecalLayOutputAlpha,   _T("ouputAlpha"),   TYPE_INT,           0, 0,
        p_ui,               TYPE_RADIO, 3, IDC_OUTPUTA_DISCARD, IDC_OUTPUTA_ADD, IDC_OUTPUTA_MULT,
        p_vals,             plPassMtlBase::kAlphaDiscard, plPassMtlBase::kAlphaAdd, plPassMtlBase::kAlphaMultiply,
        p_default,          plPassMtlBase::kAlphaDiscard,
        p_end,

    p_end
);
ParamBlockDesc2 *GetDecalLayersPB() { return &gDecalMtlLayersPB; }
