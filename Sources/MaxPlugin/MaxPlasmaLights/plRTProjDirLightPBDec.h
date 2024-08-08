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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  ParamBlock Type Konstants for plRT*Lights                                //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  8.2.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRTProjDirLightPBDec_h
#define _plRTProjDirLightPBDec_h


///////////////////////////////////////////////////////////////////////////////
//// Projected Directional Lights /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2  RTProjDirLightBlk
(
    /// Main def
    plRTProjDirLight::kBlkMain, _T("RTProjDir"), 0, plRTProjDirLightDesc::GetDesc(), P_AUTO_CONSTRUCT + P_AUTO_UI, 
    plRTProjDirLight::kRefMainRollout, 

    /// Rollout definition
    IDD_LIGHT_PARAM,        IDS_LIGHT_GEN_PARAMS,       0,  0,  &gPPDirLiteDlgProc, 

    plRTLightBase::kLightOn,        _T("on"),   TYPE_BOOL,  0, IDS_RTLIGHT_ON,
        p_default, true,    
        p_ui, TYPE_SINGLECHEKBOX,   IDC_LIGHT_ON,
        p_end,
    
    plRTLightBase::kAffectDiffuse,      _T("AffectDiffuse"),    TYPE_BOOL,  0, IDS_RTLIGHT_ON,
        p_default, true,    
        p_ui, TYPE_SINGLECHEKBOX,   IDC_LIGHT_DIFFUSE,
        p_end,
    
    plRTLightBase::kAmbientOnlyStub,        _T("AmbientOnly"),  TYPE_BOOL,  0, IDS_RTLIGHT_ON,
        p_default, false,   
        p_ui, TYPE_SINGLECHEKBOX,   IDC_AMBIENT_ONLY_STUB,
        p_end,
    
    plRTLightBase::kCastShadows,    _T("CastShadows"),  TYPE_BOOL,  0, IDS_DS_CASTSHADOWSS,
//      p_default, false,   
//      p_ui, TYPE_SINGLECHEKBOX,   IDC_CAST_SHADOWS,
        p_end,
    
    plRTLightBase::kLightColor,     _T("LightColor"),   TYPE_RGBA,  P_ANIMATABLE, IDS_DS_LIGHTCOL,
        p_default, Color(255,255,255),
        p_ui, TYPE_COLORSWATCH,         IDC_LIGHT_COLOR,
        p_end,

    plRTLightBase::kSpec,           _T("AffectSpecular"),   TYPE_BOOL,  P_ANIMATABLE, IDS_DS_SPEC,
        p_default, false,
        p_ui, TYPE_SINGLECHEKBOX,   IDC_AFFECT_SPECULAR,
        p_end,

    plRTLightBase::kSpecularColorSwatch,_T("SpecularColor"),TYPE_RGBA,  P_ANIMATABLE, IDS_DS_SPECCOL,
        p_default, Color(255,255,255),
        p_ui, TYPE_COLORSWATCH,         IDC_LIGHT_COLOR_SPECULAR,
        p_end,

    plRTLightBase::kIntensity,      _T("IntensityEditSpinner"), TYPE_FLOAT, P_ANIMATABLE, IDS_DB_MULTIPLIER,    
        p_range, -250.0, 250.0,
        p_default, 1.0,
        p_ui, TYPE_SPINNER, EDITTYPE_FLOAT,
        IDC_LMULT,IDC_LMULTSPINNER, .05f,
        p_end,

    p_end
);

///////////////////////////////////////////////////////////////////////////////
//// Projection Rollout ParamBlock ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2  plRTProjParamBlock
(
    /// Main def
    plRTProjDirLight::kBlkProj, _T("RTProjDir_Proj"), 0, plRTProjDirLightDesc::GetDesc(), P_AUTO_CONSTRUCT + P_AUTO_UI, 
    plRTProjDirLight::kRefProjRollout, 

    /// Rollout definition
    IDD_PROJ_DIRECTIONAL, IDS_PROJ_PARAMS, 0, 0, &gPPDirLiteDlgProc,    

    plRTProjDirLight::kWidth,       _T("Width"), TYPE_FLOAT,    P_ANIMATABLE, IDS_PROJWIDTH,    
        p_range, 0.0, 999999999.0,
        p_default, 200.0,
        p_ui, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,  IDC_WIDTH, IDC_WIDTHSPINNER, .05f,
        p_end,

    plRTProjDirLight::kHeight,      _T("Height"), TYPE_FLOAT,   P_ANIMATABLE, IDS_PROJHEIGHT,   
        p_range, 0.0, 999999999.0,
        p_default, 200.0,
        p_ui, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
        IDC_HEIGHT, IDC_HEIGHTSPINNER, .05f,
        p_end,

    plRTProjDirLight::kRange,       _T("Range"), TYPE_FLOAT,    P_ANIMATABLE, IDS_PROJDEPTH,    
        p_range, 0.0, 999999999.0,
        p_default, 200.0,
        p_ui, TYPE_SPINNER, EDITTYPE_POS_UNIVERSE,
        IDC_DEPTH,  IDC_DEPTHSPINNER, .05f,
        p_end,

    // Old way--here for backwards compatability
    plRTProjDirLight::kProjMap, _T("ProjMapButton"),    TYPE_BITMAP,    P_SHORT_LABELS, IDS_DS_PROJMAP,
//      p_ui, TYPE_BITMAPBUTTON,    IDC_PROJ_MAPNAME,
//      p_accessor,     plRTProjPBAccessor::Instance(),
        p_end,

    // New way
    plRTProjDirLight::kTexmap,  _T("texmap"),   TYPE_TEXMAP,    0, 0,
        p_end,
        
    plRTLightBase::kProjTypeRadio, _T("ProjTypeRadio"),     TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 4,  IDC_ILLUMINATE, IDC_ADD, IDC_MULT, IDC_MADD,
        p_vals,                     plRTLightBase::kIlluminate, plRTLightBase::kAdd, plRTLightBase::kMult, plRTLightBase::kMADD,
        p_default, plRTLightBase::kIlluminate,
        p_end,

    plRTLightBase::kProjNoCompress, _T("NoCompress"),       TYPE_BOOL,  0, IDS_DS_PROJ_PARAMS,
        p_default, false,
        p_ui, TYPE_SINGLECHEKBOX,   IDC_PROJ_NOCOMPRESS,
        p_end,        

    plRTLightBase::kProjNoMip,  _T("NoMip"),        TYPE_BOOL,  0, IDS_DS_PROJ_PARAMS,
        p_default, false,
        p_ui, TYPE_SINGLECHEKBOX,   IDC_PROJ_NOMIP,
        p_end,        

    p_end
);

#endif //_plRTProjDirLightPBDec_h

