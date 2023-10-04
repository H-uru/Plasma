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

#ifndef _plRTLightsPB_h
#define _plRTLightsPB_h

///////////////////////////////////////////////////////////////////////////////
//// Spotlights ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2  RTSpotLightBlk
(
    /// Main def
    plRTSpotLight::kBlkSpotLight, _T("RT Spot Light"), 0,   plRTSpotLightDesc::GetDesc(), P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plRTSpotLight::kRefSpotLight, 

    /// Rollout definitions
    3, 
    plRTSpotLight::kLightMap1,      IDD_LIGHT_PARAM,        IDS_LIGHT_GEN_PARAMS,       0,  0,  &gLiteDlgProc,  
    plRTSpotLight::kLightMap2,      IDD_LIGHT_ATTEN,        IDS_LIGHT_ATTEN_PARAMSS,    0,  0,  &gLiteDlgProc,
    plRTSpotLight::kLightMap3,      IDD_FREE_SPOTLIGHT,     IDS_LIGHT_SPOT_PARAMS,      0,  0,  &gLiteDlgProc,
        
        /// Rollout 1 - General Parameters
        plRTSpotLight::kLightOn,        _T("on"),   TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, true,    
            p_ui, plRTSpotLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_LIGHT_ON,
        p_end,
        
        plRTSpotLight::kAffectDiffuse,      _T("AffectDiffuse"),    TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, true,    
            p_ui, plRTSpotLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_LIGHT_DIFFUSE,
        p_end,
        
        plRTSpotLight::kAmbientOnlyStub,        _T("AmbientOnly"),  TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, false,   
            p_ui, plRTSpotLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_AMBIENT_ONLY_STUB,
        p_end,
        
        plRTSpotLight::kCastShadows,        _T("CastShadows"),  TYPE_BOOL,  0, IDS_DS_CASTSHADOWSS,
//          p_default, false,   
//          p_ui, plRTSpotLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_CAST_SHADOWS,
        p_end,
        
        plRTSpotLight::kLightColor, _T("LightColor"),           TYPE_RGBA,  P_ANIMATABLE, IDS_DS_LIGHTCOL,
            p_default, Color(255,255,255),
            p_ui, plRTSpotLight::kLightMap1,            TYPE_COLORSWATCH,       IDC_LIGHT_COLOR,
            p_end,

        plRTSpotLight::kSpec,   _T("AffectSpecular"),       TYPE_BOOL,  P_ANIMATABLE, IDS_DS_SPEC,
            p_default, false,
            //p_enable_ctrls, plRTSpotLight::kSpecularColorSwatch,
            p_ui, plRTSpotLight::kLightMap1,        TYPE_SINGLECHEKBOX, IDC_AFFECT_SPECULAR,
            p_end,

        plRTSpotLight::kSpecularColorSwatch,_T("SpecularColor"),    TYPE_RGBA,  P_ANIMATABLE, IDS_DS_SPECCOL,
            p_default, Color(255,255,255),
            p_ui, plRTSpotLight::kLightMap1,            TYPE_COLORSWATCH,       IDC_LIGHT_COLOR_SPECULAR,
            p_end,

        plRTSpotLight::kIntensity,      _T("IntensityEditSpinner"), TYPE_FLOAT, P_ANIMATABLE, IDS_DB_MULTIPLIER,    
            p_range, -250.0, 250.0,
            p_default, 1.0,
            p_ui, plRTSpotLight::kLightMap1,            TYPE_SPINNER,   EDITTYPE_FLOAT,
            IDC_LMULT,IDC_LMULTSPINNER, .05f,
            p_end,

        /// Rollout 2 - Attenuation Parameters

        plRTSpotLight::kUseAttenuationBool,     _T("AttenOnBool"),      TYPE_BOOL,  0, IDS_DS_USEFARATTEN,
            p_default, TRUE,
            p_ui, plRTSpotLight::kLightMap2,        TYPE_SINGLECHEKBOX, IDC_LIGHT_ATTENBOOL,
            p_enable_ctrls, 2, plRTSpotLight::kAttenMaxFalloffEdit,plRTSpotLight::kAttenTypeRadio,
            p_end,

        plRTSpotLight::kAttenMaxFalloffEdit,        _T("MaxFalloffEdit"),       TYPE_FLOAT, P_ANIMATABLE, IDS_DS_ATTENSTARTNEAR,    
            p_range, 0.0, 999999999.0,
            p_default, 200.0,
            p_ui, plRTSpotLight::kLightMap2,            TYPE_SPINNER,   EDITTYPE_POS_UNIVERSE,
            IDC_LIGHT_ATTEN,    IDC_LIGHT_ATTEN_SPIN, .05f,
            p_end,

        plRTSpotLight::kAttenTypeRadio, _T("LightShapeRadio"),  TYPE_INT, 0,    IDS_DS_LIGHTSHAPE_RATIO,
            p_default, 0,
            //p_vals, 1, 2, 
            p_ui, plRTSpotLight::kLightMap2,        TYPE_RADIO, 3,
            IDC_LIGHT_ATTEN_LINEAR_RADIO,   IDC_LIGHT_ATTEN_QUAD_RADIO, IDC_LIGHT_ATTEN_NONE_RADIO,
            
            p_end,
    
        /// Rollout 3 - Spotlight Parameters

        plRTSpotLight::kShowConeBool,   _T("ShowConeBool"),         TYPE_BOOL,  0, IDS_DS_SHOWCONE,
            p_default, false,
            p_ui, plRTSpotLight::kLightMap3,        TYPE_SINGLECHEKBOX, IDC_SHOW_CONE,
            p_end,


        plRTSpotLight::kHotSpot,    _T("HotSpot"),                  TYPE_FLOAT, P_ANIMATABLE, IDS_DB_HOTSIZE,   
            p_range, 0.0, 179.0f,
            p_default, 43.0,
            p_ui, plRTSpotLight::kLightMap3,        TYPE_SPINNER,   EDITTYPE_POS_FLOAT,
            IDC_LHOTSIZE,       IDC_LHOTSIZESPINNER, .05f,
            p_end,    

        plRTSpotLight::kFallOff,    _T("FallOff"),                  TYPE_FLOAT, P_ANIMATABLE, IDS_DB_FALLSIZE,  
            p_range, 0.0, 179.0f,
            p_default, 45.0,
            p_ui, plRTSpotLight::kLightMap3,        TYPE_SPINNER,   EDITTYPE_POS_FLOAT,
            IDC_LFALLOFF,       IDC_LFALLOFFSPINNER, .05f,
            p_end,

        plRTSpotLight::kUseProjectorBool,   _T("UseProjBool"),      TYPE_BOOL,  0, IDS_DS_PROJ_PARAMS,
            p_default, false,
            p_enable_ctrls, 1, plRTSpotLight::kProjMapTexButton,
            p_ui, plRTSpotLight::kLightMap3,        TYPE_SINGLECHEKBOX, IDC_PROJECTOR,
            p_end,        

        plRTSpotLight::kProjMapTexButton,   _T("ProjMapButton"),    TYPE_BITMAP,    P_SHORT_LABELS, IDS_DS_PROJMAP,
            p_ui, plRTSpotLight::kLightMap3,        TYPE_BITMAPBUTTON,  IDC_PROJ_MAPNAME,
            p_accessor,     plLightTexPBAccessor::Instance(),
            p_end,

        plRTLightBase::kProjTypeRadio, _T("ProjTypeRadio"),     TYPE_INT,       0, 0,
            p_default, plRTLightBase::kIlluminate,
            p_ui,       plRTSpotLight::kLightMap3,      TYPE_RADIO, 4,  IDC_ILLUMINATE, IDC_ADD, IDC_MULT, IDC_MADD,
            p_vals,                     plRTLightBase::kIlluminate, plRTLightBase::kAdd, plRTLightBase::kMult, plRTLightBase::kMADD,
            p_end,

        plRTLightBase::kProjNoCompress, _T("NoCompress"),       TYPE_BOOL,  0, IDS_DS_PROJ_PARAMS,
            p_default, false,
            p_ui, plRTSpotLight::kLightMap3,        TYPE_SINGLECHEKBOX, IDC_PROJ_NOCOMPRESS,
            p_end,        

        plRTLightBase::kProjNoMip,  _T("NoMip"),        TYPE_BOOL,  0, IDS_DS_PROJ_PARAMS,
            p_default, false,
            p_ui, plRTSpotLight::kLightMap3,        TYPE_SINGLECHEKBOX, IDC_PROJ_NOMIP,
            p_end,

    p_end
);


///////////////////////////////////////////////////////////////////////////////
//// Omni Lights //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2  RTOmniLightBlk
(
    /// Main def
    plRTOmniLight::kBlkOmniLight, _T("RT Omni Light"), 0,   plRTOmniLightDesc::GetDesc(), P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plRTOmniLight::kRefOmniLight, 

    /// Rollout definitions
    2, 
    plRTOmniLight::kLightMap1,      IDD_LIGHT_PARAM,    IDS_LIGHT_GEN_PARAMS,       0,  0,  &gLiteDlgProc,  
    plRTOmniLight::kLightMap2,      IDD_LIGHT_ATTEN,    IDS_LIGHT_ATTEN_PARAMSS,    0,  0,  &gLiteDlgProc,

        /// Rollout 1 - General Parameters
        plRTOmniLight::kLightOn,        _T("on"),   TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, true,    
            p_ui, plRTOmniLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_LIGHT_ON,
        p_end,
        
        plRTOmniLight::kAffectDiffuse,      _T("AffectDiffuse"),    TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, true,    
            p_ui, plRTOmniLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_LIGHT_DIFFUSE,
        p_end,
        
        plRTOmniLight::kAmbientOnlyStub,        _T("AmbientOnly"),  TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, false,   
            p_ui, plRTOmniLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_AMBIENT_ONLY_STUB,
        p_end,
        
        plRTOmniLight::kCastShadows,        _T("CastShadows"),  TYPE_BOOL,  0, IDS_DS_CASTSHADOWSS,
//          p_default, false,   
//          p_ui, plRTOmniLight::kLightMap1,    TYPE_SINGLECHEKBOX, IDC_CAST_SHADOWS,
            p_end,
        
        plRTOmniLight::kLightColor, _T("LightColor"),           TYPE_RGBA,  P_ANIMATABLE, IDS_DS_LIGHTCOL,
            p_default, Color(255,255,255),
            p_ui, plRTOmniLight::kLightMap1,            TYPE_COLORSWATCH,       IDC_LIGHT_COLOR,
            p_end,

        plRTOmniLight::kSpec,   _T("AffectSpecular"),       TYPE_BOOL,  P_ANIMATABLE, IDS_DS_SPEC,
            p_default, false,
            p_ui, plRTOmniLight::kLightMap1,        TYPE_SINGLECHEKBOX, IDC_AFFECT_SPECULAR,
            p_end,

        plRTOmniLight::kSpecularColorSwatch,_T("SpecularColor"),    TYPE_RGBA,  P_ANIMATABLE, IDS_DS_SPECCOL,
            p_default, Color(255,255,255),
            p_ui, plRTOmniLight::kLightMap1,            TYPE_COLORSWATCH,       IDC_LIGHT_COLOR_SPECULAR,
            p_end,


        plRTOmniLight::kIntensity,      _T("IntensityEditSpinner"), TYPE_FLOAT, P_ANIMATABLE, IDS_DB_MULTIPLIER,    
            p_range, -250.0, 250.0,
            p_default, 1.0,
            p_ui, plRTOmniLight::kLightMap1,            TYPE_SPINNER,   EDITTYPE_POS_FLOAT,
            IDC_LMULT,IDC_LMULTSPINNER, .05f,
            p_end,


        /// Rollout 2 - Light Attenuation Parameters

        plRTOmniLight::kUseAttenuationBool,     _T("AttenOnBool"),      TYPE_BOOL,  0, IDS_DS_USEFARATTEN,
            p_default, TRUE,
            p_ui, plRTOmniLight::kLightMap2,        TYPE_SINGLECHEKBOX, IDC_LIGHT_ATTENBOOL,
            p_enable_ctrls, 2, plRTOmniLight::kAttenMaxFalloffEdit, plRTOmniLight::kAttenTypeRadio,
            p_end,

        plRTOmniLight::kAttenMaxFalloffEdit,        _T("MaxFalloffEdit"),       TYPE_FLOAT, P_ANIMATABLE, IDS_DS_ATTENSTARTNEAR,    
            p_range, 0.0, 999999999.0,
            p_default, 200.0,
            p_ui, plRTOmniLight::kLightMap2,            TYPE_SPINNER,   EDITTYPE_POS_UNIVERSE,
            IDC_LIGHT_ATTEN,    IDC_LIGHT_ATTEN_SPIN, .05f,
            p_end,

        plRTOmniLight::kAttenTypeRadio, _T("LightShapeRadio"),  TYPE_INT, 0,    IDS_DS_LIGHTSHAPE_RATIO,
            p_default, 0,
            //p_vals, 1, 2, 
            p_ui, plRTOmniLight::kLightMap2,        TYPE_RADIO, 3,
            IDC_LIGHT_ATTEN_LINEAR_RADIO,   IDC_LIGHT_ATTEN_QUAD_RADIO, IDC_LIGHT_ATTEN_NONE_RADIO,
            p_end,

    p_end
);


///////////////////////////////////////////////////////////////////////////////
//// Directional Lights ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ParamBlockDesc2  RTTDirLightBlk
(
    /// Main def
    plRTDirLight::kBlkTSpotLight, _T("RT Spot Light"), 0,   plRTDirLightDesc::GetDesc(), P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plRTDirLight::kRefTSpotLight, 

    /// Rollout definitions
    1, 
    plRTDirLight::kLightMap1,       IDD_LIGHT_PARAM,        IDS_LIGHT_GEN_PARAMS,       0,  0,  &gLiteDlgProc,  

        /// Rollout 1 - General Parameters
        plRTDirLight::kLightOn,     _T("on"),   TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, true,    
            p_ui, plRTDirLight::kLightMap1, TYPE_SINGLECHEKBOX, IDC_LIGHT_ON,
        p_end,
        
        plRTDirLight::kAffectDiffuse,       _T("AffectDiffuse"),    TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, true,    
            p_ui, plRTDirLight::kLightMap1, TYPE_SINGLECHEKBOX, IDC_LIGHT_DIFFUSE,
        p_end,
        
        plRTDirLight::kAmbientOnlyStub,     _T("AmbientOnly"),  TYPE_BOOL,  0, IDS_RTLIGHT_ON,
            p_default, false,   
            p_ui, plRTDirLight::kLightMap1, TYPE_SINGLECHEKBOX, IDC_AMBIENT_ONLY_STUB,
        p_end,
        
        plRTDirLight::kCastShadows,     _T("CastShadows"),  TYPE_BOOL,  0, IDS_DS_CASTSHADOWSS,
//          p_default, false,   
//          p_ui, plRTDirLight::kLightMap1, TYPE_SINGLECHEKBOX, IDC_CAST_SHADOWS,
        p_end,
        
        plRTDirLight::kLightColor,  _T("LightColor"),           TYPE_RGBA,  P_ANIMATABLE, IDS_DS_LIGHTCOL,
            p_default, Color(255,255,255),
            p_ui, plRTDirLight::kLightMap1,         TYPE_COLORSWATCH,       IDC_LIGHT_COLOR,
            p_end,

        plRTDirLight::kSpec,    _T("AffectSpecular"),       TYPE_BOOL,  P_ANIMATABLE, IDS_DS_SPEC,
            p_default, false,
            p_ui, plRTDirLight::kLightMap1,     TYPE_SINGLECHEKBOX, IDC_AFFECT_SPECULAR,
            p_end,

        plRTDirLight::kSpecularColorSwatch,_T("SpecularColor"), TYPE_RGBA,  P_ANIMATABLE, IDS_DS_SPECCOL,
            p_default, Color(255,255,255),
            p_ui, plRTDirLight::kLightMap1,         TYPE_COLORSWATCH,       IDC_LIGHT_COLOR_SPECULAR,
            p_end,

        plRTDirLight::kIntensity,       _T("IntensityEditSpinner"), TYPE_FLOAT, P_ANIMATABLE, IDS_DB_MULTIPLIER,    
            p_range, -250.0, 250.0,
            p_default, 1.0,
            p_ui, plRTDirLight::kLightMap1,         TYPE_SPINNER,   EDITTYPE_FLOAT,
            IDC_LMULT,IDC_LMULTSPINNER, .05f,
            p_end,

    p_end
);


#endif //_plRTLightsPB_h

