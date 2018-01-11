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

#include "HeadSpin.h"
#include "hsColorRGBA.h"
#include "hsResMgr.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNode.h"
#include "resource.h"

#include "MaxMain/plPlasmaRefMsgs.h"
#include "plDrawable/plGeometrySpan.h"
#include "plLightMapComponent.h"

void DummyCodeIncludeFuncLightMap()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//      LightMap Component
//      
//
//
//


//Max desc stuff necessary below.
CLASS_DESC(plLightMapComponent, gLightMapDesc, "Light Map",  "LightMap", COMP_TYPE_GRAPHICS, LIGHTMAP_COMP_CID)

enum
{
    kMapChannel,                    //Inserted in v1
    kResolutionLevelRadio,          //Inserted in v1, removed in v2
    kResSpinControl,                    //Inserted in v2
    kMapInitColor,
    kCompress,
    kShared
};

ParamBlockDesc2 gLightMapBk
(
    plComponent::kBlkComp, _T("lightMap"), 0, &gLightMapDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_LIGHTMAP, IDS_COMP_LIGHTMAPS, 0, 0, nullptr,

        kMapChannel, _T("UVW Channel Light Map"), TYPE_INT, 0, 0,
        p_ui,   TYPE_SPINNER, EDITTYPE_INT, IDC_COMP_LIGHTMAP_EDIT1, IDC_COMP_LIGHTMAP_SPIN1,   0.4,
        p_default, 1,
        p_range, 1, plGeometrySpan::kMaxNumUVChannels,
        p_end,
//
//      kResolutionLevelRadio,  _T("Resolution Level"),     TYPE_INT,       0, 0,
//      p_ui,       TYPE_RADIO, 5,  IDC_RADIO_LM1,  IDC_RADIO_LM2,  IDC_RADIO_LM3,  IDC_RADIO_LM4, IDC_RADIO_LM5,
//      p_vals,         0, 1,   2,  3,  4,
//      p_default,      2,
//
//
        kResSpinControl, _T("Resolution Spinner"),  TYPE_INT,       0,  0,
        p_ui,   TYPE_SLIDER,    EDITTYPE_INT, IDC_COMP_LM_DUMMY, IDC_COMP_LIGHT_SLIDER, 4,
        p_range, 0, 4,
        p_default, 2,
        p_end,
    
        kMapInitColor, _T("Initial map color"), TYPE_RGBA, 0, 0,
        p_ui, TYPE_COLORSWATCH,         IDC_COMP_LIGHTMAP_COLOR,
        p_default, Color(0,0,0),
        p_end,

        kCompress,  _T("Compress"), TYPE_BOOL,      0, 0,
            p_default,  TRUE,
            p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_LIGHTMAP_COMPRESS,
            p_end,

        kShared,  _T("Shared"), TYPE_BOOL,      0, 0,
            p_default,  FALSE,
            p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_LIGHTMAP_SHARED,
            p_end,

    p_end
);




plLightMapComponent::plLightMapComponent()
{
    fClassDesc = &gLightMapDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}




bool plLightMapComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plLightMapComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fLightMapKey = nullptr;
    return true;
}


bool plLightMapComponent::PreConvert(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    return true;
}


float plLightMapComponent::GetScale() const
{
    int resBut = fCompPB->GetInt(kResSpinControl);
    float res = 1.f;
    switch( resBut )
    {
    case 4:
        res = 9.f;
        break;
    case 3:
        res = 3.f;
        break;
    default:
    case 2:
        res = 1.f;
        break;
    case 1:
        res = 0.5f;
        break;
    case 0:
        res = 0.25f;
        break;
    }

    return res;
}

uint32_t plLightMapComponent::GetUVWSrc() const
{
    return fCompPB->GetInt(kMapChannel)-1;
}

bool plLightMapComponent::GetCompress() const
{
    return fCompPB->GetInt(kCompress);
}

bool plLightMapComponent::GetShared() const
{
    return fCompPB->GetInt(kShared);
}

hsColorRGBA plLightMapComponent::GetInitColor() const
{
    Color color = fCompPB->GetColor(kMapInitColor);
    hsColorRGBA col;
    col.Set(color.r, color.g, color.b, 1.f);
    return col;
}
