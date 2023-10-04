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
#include "plTweak.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#include "MaxMain/plPlasmaRefMsgs.h"

#include "plPickNode.h"

#include "plWaterComponent.h"
#include "plSoftVolumeComponent.h"

#include "plDrawable/plWaveSetBase.h"
#include "plDrawable/plWaveSet7.h"
#include "plDrawable/plFixedWaterState7.h"

#include "plPipeline/plDynamicEnvMap.h"

#include "MaxMain/plPluginResManager.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnMessage/plObjRefMsg.h"

#include "plScene/plVisRegion.h"

static constexpr float kPercentToFrac(1.e-2f);


// Preliminary setup bookkeeping
void DummyCodeIncludeFuncWater()
{
}

CLASS_DESC(plWaterComponent, gWaterCompDesc, "Large Water",  "Water", COMP_TYPE_WATER, WATER_COMP_CID)

ParamBlockDesc2 gWaterBk
(   
    plComponent::kBlkComp, _T("Water"), 0, &gWaterCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

    plWaterComponent::kNumRollups,
        plWaterComponent::kRef, IDD_COMP_W_REFOBJECT, IDS_COMP_W_REFOBJECT, 0, 0, 0,
//      plWaterComponent::kBasicWater, IDD_COMP_W_BASICWATER, IDS_COMP_W_BASICWATER, 0, 0, 0,
        plWaterComponent::kGeoWater, IDD_COMP_W_GEOWATER, IDS_COMP_W_GEOWATER, 0, 0, 0,
        plWaterComponent::kTexWater, IDD_COMP_W_TEXWATER, IDS_COMP_W_TEXWATER, 0, 0, 0,
        plWaterComponent::kAdvWater, IDD_COMP_W_ADVWATER, IDS_COMP_W_ADVWATER, 0, 0, 0,
        plWaterComponent::kEnvMap, IDD_COMP_W_ENVMAP, IDS_COMP_W_ENVMAP, 0, 0, 0,
        plWaterComponent::kVtxHelp, IDD_COMP_W_VTXHELP, IDS_COMP_W_VTXHELP, 0, 0, 0,
        plWaterComponent::kBasicShore, IDD_COMP_W_BASICSHORE, IDS_COMP_W_BASICSHORE, 0, 0, 0,
        plWaterComponent::kAdvShore, IDD_COMP_W_ADVSHORE, IDS_COMP_W_ADVSHORE, 0, 0, 0,

    plWaterComponent::kRefObject, _T("RefObject"),  TYPE_INODE,     0, 0,
        p_ui,   plWaterComponent::kRef, TYPE_PICKNODEBUTTON, IDC_COMP_W_REFOBJECT,
        p_prompt, IDS_COMP_CHOOSE_OBJECT,
        p_end,

    // WATER BASIC
    plWaterComponent::kWindSpeed, _T("WindSpeed"), TYPE_FLOAT,  0, 0,   
//      p_default, 30.0,
//      p_range, -1.0, 50.0,
//      p_ui,   plWaterComponent::kBasicWater, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
//      IDC_COMP_W_WINDSPEED, IDC_COMP_W_WINDSPEED_SPIN, 1.0,
        p_end,    

    plWaterComponent::kWaterTint,   _T("WaterTint"), TYPE_RGBA, 0, 0,
//      p_default, Color(0.1, 0.2, 0.2),
//      p_ui, plWaterComponent::kBasicWater, TYPE_COLORSWATCH,  IDC_COMP_W_WATERTINT,
        p_end,

    plWaterComponent::kWaterOpac, _T("WaterOpac"), TYPE_FLOAT,  0, 0,   
//      p_default, 100.0,
//      p_range, 0.0, 100.0,
//      p_ui,   plWaterComponent::kBasicWater, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
//      IDC_COMP_W_WATEROPAC, IDC_COMP_W_WATEROPAC_SPIN, 1.0,
        p_end,    

    plWaterComponent::kSpecularTint,    _T("SpecularTint"), TYPE_RGBA,  0, 0,
        p_default, Color(1.0, 1.0, 1.0),
        p_ui, plWaterComponent::kGeoWater, TYPE_COLORSWATCH,  IDC_COMP_W_SPECULARTINT,
        p_end,

    plWaterComponent::kRippleScale, _T("RippleScale"), TYPE_FLOAT,  0, 0,   
        p_default, 25.0,
        p_range, 5.0, 1000.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_RIPPLESCALE, IDC_COMP_W_TEX_RIPPLESCALE_SPIN, 1.0,
        p_end,    

    plWaterComponent::kDispersion, _T("Dispersion"), TYPE_FLOAT,    0, 0,   
//      p_default, 0.0,
//      p_range, 0.0, 100.0,
//      p_ui,   plWaterComponent::kBasicWater, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
//      IDC_COMP_W_DISPERSION, IDC_COMP_W_DISPERSION_SPIN, 1.0,
        p_end,    

    // WATER ADVANCED
    plWaterComponent::kDepthOpac, _T("DepthOpac"), TYPE_FLOAT,  0, 0,   
        p_default, 3.0,
        p_range, 0.5, 20.0,
        p_ui,   plWaterComponent::kAdvWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_DEPTHOPAC, IDC_COMP_W_DEPTHOPAC_SPIN, 1.0,
        p_end,    

    plWaterComponent::kDepthRefl, _T("DepthRefl"), TYPE_FLOAT,  0, 0,   
        p_default, 3.0,
        p_range, 0.5, 20.0,
        p_ui,   plWaterComponent::kAdvWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_DEPTHREFL, IDC_COMP_W_DEPTHREFL_SPIN, 1.0,
        p_end,    

    plWaterComponent::kDepthWave, _T("DepthWave"), TYPE_FLOAT,  0, 0,   
        p_default, 4.0,
        p_range, 0.5, 20.0,
        p_ui,   plWaterComponent::kAdvWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_DEPTHWAVE, IDC_COMP_W_DEPTHWAVE_SPIN, 1.0,
        p_end,    

    plWaterComponent::kZeroOpac, _T("ZeroOpac"), TYPE_FLOAT,    0, 0,   
        p_default, -1.0,
        p_range, -10.0, 10.0,
        p_ui,   plWaterComponent::kAdvWater, TYPE_SPINNER,  EDITTYPE_FLOAT, 
        IDC_COMP_W_ZEROOPAC, IDC_COMP_W_ZEROOPAC_SPIN, 1.0,
        p_end,    

    plWaterComponent::kZeroRefl, _T("ZeroRefl"), TYPE_FLOAT,    0, 0,   
        p_default, 0.0,
        p_range, -10.0, 10.0,
        p_ui,   plWaterComponent::kAdvWater, TYPE_SPINNER,  EDITTYPE_FLOAT, 
        IDC_COMP_W_ZEROREFL, IDC_COMP_W_ZEROREFL_SPIN, 1.0,
        p_end,    

    plWaterComponent::kZeroWave, _T("ZeroWave"), TYPE_FLOAT,    0, 0,   
        p_default, 0.0,
        p_range, -10.0, 10.0,
        p_ui,   plWaterComponent::kAdvWater, TYPE_SPINNER,  EDITTYPE_FLOAT, 
        IDC_COMP_W_ZEROWAVE, IDC_COMP_W_ZEROWAVE_SPIN, 1.0,
        p_end,    

    // SHORE BASIC
    plWaterComponent::kShoreTint,   _T("ShoreTint"), TYPE_RGBA, 0, 0,
        p_default, Color(0.2, 0.4, 0.4),
        p_ui, plWaterComponent::kBasicShore, TYPE_COLORSWATCH,  IDC_COMP_W_SHORETINT,
        p_end,

    plWaterComponent::kShoreOpac, _T("ShoreOpac"), TYPE_FLOAT,  0, 0,   
        p_default, 40.0,
        p_range, 0.0, 100.0,
        p_ui,   plWaterComponent::kBasicShore, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_SHOREOPAC, IDC_COMP_W_SHOREOPAC_SPIN, 1.0,
        p_end,    

    plWaterComponent::kWispiness, _T("Wispiness"), TYPE_FLOAT,  0, 0,   
        p_default, 50.0,
        p_range, 0.0, 200.0,
        p_ui,   plWaterComponent::kBasicShore, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_WISPINESS, IDC_COMP_W_WISPINESS_SPIN, 1.0,
        p_end,    

    // SHORE ADVANCED
    plWaterComponent::kPeriod, _T("Period"), TYPE_FLOAT,    0, 0,   
        p_default, 100.0,
        p_range, 50.0, 200.0,
        p_ui,   plWaterComponent::kAdvShore, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_PERIOD, IDC_COMP_W_PERIOD_SPIN, 1.0,
        p_end,    

    plWaterComponent::kFinger, _T("Finger"), TYPE_FLOAT,    0, 0,   
        p_default, 100.0,
        p_range, 50.0, 300.0,
        p_ui,   plWaterComponent::kAdvShore, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_FINGER, IDC_COMP_W_FINGER_SPIN, 1.0,
        p_end,    

    plWaterComponent::kEnvObject, _T("EnvObject"),  TYPE_INODE,     0, 0,
        p_ui,   plWaterComponent::kEnvMap, TYPE_PICKNODEBUTTON, IDC_COMP_W_ENVOBJECT,
        p_prompt, IDS_COMP_CHOOSE_OBJECT,
        p_end,

    plWaterComponent::kEnvSize, _T("EnvSize"), TYPE_INT, 0, 0,
        p_default, 256,
        p_range, 32, 512,
        p_ui,   plWaterComponent::kEnvMap, TYPE_SPINNER, EDITTYPE_INT, 
        IDC_COMP_W_ENVSIZE, IDC_COMP_W_ENVSIZE_SPIN,    1.f,
        p_end,

    plWaterComponent::kEnvRadius, _T("EnvRadius"), TYPE_FLOAT,  0, 0,   
        p_default, 500.0,
        p_range, 5.0, 10000.0,
        p_ui,   plWaterComponent::kEnvMap, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_ENVRADIUS, IDC_COMP_W_ENVRADIUS_SPIN, 1.0,
        p_end,    

    plWaterComponent::kEdgeOpac, _T("EdgeOpac"), TYPE_FLOAT,    0, 0,   
        p_default, 100.0,
        p_range, 0.0, 100.0,
        p_ui,   plWaterComponent::kAdvShore, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_EDGEOPAC, IDC_COMP_W_EDGEOPAC_SPIN, 1.0,
        p_end,    

    plWaterComponent::kEdgeRadius, _T("EdgeRadius"), TYPE_FLOAT,    0, 0,   
        p_default, 100.0,
        p_range, 50.0, 300.0,
        p_ui,   plWaterComponent::kAdvShore, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_EDGERADIUS, IDC_COMP_W_EDGERADIUS_SPIN, 1.0,
        p_end,    


    plWaterComponent::kEnvRefresh, _T("EnvRefresh"), TYPE_FLOAT,    0, 0,   
        p_default, 0.0,
        p_range, 0.0, 3600.0,
        p_ui,   plWaterComponent::kEnvMap, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_ENVREFRESH, IDC_COMP_W_ENVREFRESH_SPIN, 10.0,
        p_end,    

    plWaterComponent::kGeoMinLen, _T("GeoMinLen"), TYPE_FLOAT,  0, 0,   
        p_default, 4.0,
        p_range, 0.1, 50.0,
        p_ui,   plWaterComponent::kGeoWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_GEO_MINLEN, IDC_COMP_W_GEO_MINLEN_SPIN, 1.0,
        p_end,    

    plWaterComponent::kGeoMaxLen, _T("GeoMaxLen"), TYPE_FLOAT,  0, 0,   
        p_default, 8.0,
        p_range, 0.1, 50.0,
        p_ui,   plWaterComponent::kGeoWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_GEO_MAXLEN, IDC_COMP_W_GEO_MAXLEN_SPIN, 1.0,
        p_end,    

    plWaterComponent::kGeoAmpOverLen, _T("GeoAngleDev"), TYPE_FLOAT,    0, 0,   
        p_default, 10.0,
        p_range, 0.0, 100.0,
        p_ui,   plWaterComponent::kGeoWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_GEO_AMPOVERLEN, IDC_COMP_W_GEO_AMPOVERLEN_SPIN, 1.0,
        p_end,    

    plWaterComponent::kGeoAngleDev, _T("GeoAngleDev"), TYPE_FLOAT,  0, 0,   
        p_default, 20.0,
        p_range, 0.0, 180.0,
        p_ui,   plWaterComponent::kGeoWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_GEO_ANGLEDEV, IDC_COMP_W_GEO_ANGLEDEV_SPIN, 10.0,
        p_end,    

    plWaterComponent::kGeoChop, _T("GeoChop"), TYPE_FLOAT,  0, 0,   
        p_default, 50.0,
        p_range, 0.0, 500.0,
        p_ui,   plWaterComponent::kGeoWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_GEO_CHOP, IDC_COMP_W_GEO_CHOP_SPIN, 1.0,
        p_end,    

    plWaterComponent::kTexMinLen, _T("TexMinLen"), TYPE_FLOAT,  0, 0,   
        p_default, 0.1,
        p_range, 0.01, 4.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_MINLEN, IDC_COMP_W_TEX_MINLEN_SPIN, 1.0,
        p_end,    

    plWaterComponent::kTexMaxLen, _T("TexMaxLen"), TYPE_FLOAT,  0, 0,   
        p_default, 4.0,
        p_range, 0.1, 50.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_MAXLEN, IDC_COMP_W_TEX_MAXLEN_SPIN, 1.0,
        p_end,    

    plWaterComponent::kTexAmpOverLen, _T("TexAngleDev"), TYPE_FLOAT,    0, 0,   
        p_default, 10.0,
        p_range, 0.0, 100.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_AMPOVERLEN, IDC_COMP_W_TEX_AMPOVERLEN_SPIN, 1.0,
        p_end,    

    plWaterComponent::kTexAngleDev, _T("TexAngleDev"), TYPE_FLOAT,  0, 0,   
        p_default, 20.0,
        p_range, 0.0, 180.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_ANGLEDEV, IDC_COMP_W_TEX_ANGLEDEV_SPIN, 1.0,
        p_end,    

    plWaterComponent::kNoise, _T("Noise"), TYPE_FLOAT,  0, 0,   
        p_default, 50.0,
        p_range, 0.0, 300.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_NOISE, IDC_COMP_W_TEX_NOISE_SPIN, 1.0,
        p_end,    

    plWaterComponent::kTexChop, _T("TexChop"), TYPE_FLOAT,  0, 0,   
        p_default, 50.0,
        p_range, 0.0, 500.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_CHOP, IDC_COMP_W_TEX_CHOP_SPIN, 1.0,
        p_end,    

    plWaterComponent::kSpecStart, _T("SpecStart"), TYPE_FLOAT,  0, 0,   
        p_default, 50.0,
        p_range, 0.0, 1000.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_SPECSTART, IDC_COMP_W_TEX_SPECSTART_SPIN, 10.0,
        p_end,    

    plWaterComponent::kSpecEnd, _T("SpecEnd"), TYPE_FLOAT,  0, 0,   
        p_default, 1000.0,
        p_range, 0.0, 10000.0,
        p_ui,   plWaterComponent::kTexWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_TEX_SPECEND, IDC_COMP_W_TEX_SPECEND_SPIN, 10.0,
        p_end,    

    plWaterComponent::kSpecularMute, _T("SpecularMute"), TYPE_FLOAT,    0, 0,   
        p_default, 30.0,
        p_range, 0.0, 100.0,
        p_ui,   plWaterComponent::kGeoWater, TYPE_SPINNER,  EDITTYPE_POS_FLOAT, 
        IDC_COMP_W_GEO_SPECMUTE, IDC_COMP_W_GEO_SPECMUTE_SPIN, 1.0,
        p_end,

    p_end
);


class plWaterCompPostLoadCallback : public PostLoadCallback
{
public:
    plWaterComponent*   fWaterComp;

    plWaterCompPostLoadCallback(plWaterComponent* wc) : fWaterComp(wc) {}

    void proc(ILoad *iload) override
    {
        fWaterComp->CheckForObsoleteParams();

        delete this;
    }
};

IOResult plWaterComponent::Load(ILoad* iLoad)
{
    iLoad->RegisterPostLoadCallback(new plWaterCompPostLoadCallback(this));

    return plComponent::Load(iLoad);
}

void plWaterComponent::CheckForObsoleteParams()
{
    if( (fCompPB->GetFloat(kDispersion) >= 0)
        ||(fCompPB->GetFloat(kWindSpeed) >= 0) )
    {
        // Okay, these are old. Need to set some default values based
        // on the old obsolete ones. Basically, we need to go from:
        //
        // kDispersion =>
        //      GeoAngleDev
        //      TexAngleDev
        //
        // kWindSpeed
        //      GeoMinLen
        //      GeoMaxLen
        //      GeoAmpOverLen = 0.1
        //
        //      TexMinLen
        //      TexMaxLen
        //      TexAmpOverLen
        //
        //      Noise
        //
        // kEnvRadius
        //      SpecStart = kEnvRadius / 2.f
        //      SpecEnd = kEnvRadius * 2.f

        // Okay, here we go.
        float dispersion = fCompPB->GetFloat(kDispersion) / 100.f;
        plConst(float) kMinAng(5.f);
        plConst(float) kMaxAng(180.f);
        
        float angleDev = kMinAng + dispersion * (kMaxAng - kMinAng);
        fCompPB->SetValue(kGeoAngleDev, TimeValue(0), angleDev);
        fCompPB->SetValue(kTexAngleDev, TimeValue(0), angleDev);

        float windSpeed = fCompPB->GetFloat(kWindSpeed);
        const float kGravConst(32.f); // ft/s^2
        float waveLen = windSpeed * windSpeed / kGravConst;
        waveLen /= 2.f;
        if( waveLen < 1.f )
            waveLen = 1.f;
        fCompPB->SetValue(kGeoMinLen, TimeValue(0), waveLen/2.f);
        fCompPB->SetValue(kGeoMaxLen, TimeValue(0), waveLen*2.f);
        fCompPB->SetValue(kGeoAmpOverLen, TimeValue(0), 10.f);

        float rippleScale = fCompPB->GetFloat(kRippleScale);
        fCompPB->SetValue(kTexMinLen, TimeValue(0), 4.f / 256.f * rippleScale);
        fCompPB->SetValue(kTexMaxLen, TimeValue(0), 32.f / 256.f * rippleScale);
        float amp = 0.01f;
        float specMute = 0.5f;
        if( windSpeed < 15.f )
        {
            float p = windSpeed / 15.f;
            amp += p * (0.1f - 0.01f);

            specMute += (1-p)* 0.5f;
        }
        fCompPB->SetValue(kTexAmpOverLen, TimeValue(0), amp*100.f);
        fCompPB->SetValue(kSpecularMute, TimeValue(0), specMute*100.f);

        fCompPB->SetValue(kNoise, TimeValue(0), 50.f);

        float envRad = fCompPB->GetFloat(kEnvRadius);
        float specStart = envRad / 2.f;
        float specEnd = envRad * 2.f;
        fCompPB->SetValue(kSpecStart, TimeValue(0), specStart);
        fCompPB->SetValue(kSpecEnd, TimeValue(0), specEnd);
        
        // Set them negative so we don't keep doing this.
        fCompPB->SetValue(kDispersion, TimeValue(0), -1.f);
        fCompPB->SetValue(kWindSpeed, TimeValue(0), -1.f);
    }
}


// Component implementation
plWaterComponent::plWaterComponent()
:   fWaveSet()
{
    fClassDesc = &gWaterCompDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plWaterComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
    node->SetRunTimeLight(true);
    node->SetForceMaterialCopy(true);
    node->SetForceShadow(false);
    node->SetNoShadow(true);
    node->SetSmoothAll(true);
    node->SetForceSortable(true);
    node->SetNoSpanSort(true);
    node->SetNoPreShade(true);

    // Turn on calculation of edge lengths XXX
    node->SetCalcEdgeLens(true);

    // Make a note that we're vertex shaded (at least ripplecomponent needs to know).XXX
    node->SetVS(true);

    // Turn off the convexity test. We want to be sorted.XXX
    node->SetConcave(true);

    node->SetReverseSort(true);

    node->SetWaterHeight(IGetWaterHeight());

    return true;
}

bool plWaterComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{ 
    if( !fWaveSet )
        IMakeWaveSet(node, pErrMsg);

    // Do it again in case some idiot is trying to override.XXX

    pErrMsg->Set(!node->HasLoadMask(), node->GetName(), "PS water has no representation component").CheckAndAsk();
    pErrMsg->Set(false);

    return true; 
}

bool plWaterComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    if( !fWaveSet )
        return true;

    plObjRefMsg* refMsg = new plObjRefMsg(node->GetKey(), plRefMsg::kOnRequest, -1, plObjRefMsg::kModifier);
    hsgResMgr::ResMgr()->AddViaNotify(fWaveSet->GetKey(), refMsg, plRefFlags::kActiveRef);

    return true;
}

bool plWaterComponent::DeInit(plMaxNode* node, plErrorMsg* pErrMsg)
{ 
    if( fWaveSet )
        fWaveSet->GetKey()->UnRefObject();
    fWaveSet = nullptr;

    return true; 
}

bool plWaterComponent::IReadRefObject(plMaxNodeBase* node, plFixedWaterState7& ws)
{
    INode* ref = fCompPB->GetINode(kRefObject);
    if( !ref )
    {
        ref = node;
    }
    if( !ref )
        return false;

    Matrix3 xfm = ref->GetNodeTM(TimeValue(0));
    ws.fWaterHeight = xfm.GetTrans().z;

    Point3 y = xfm.GetRow(1);
    hsVector3 dir(-y.x, -y.y, 0.f);
    dir.Normalize();
    ws.fWindDir = dir;

    return true;
}

bool plWaterComponent::IReadEnvObject(plMaxNode* node, plErrorMsg* pErrMsg, plFixedWaterState7& ws)
{
    INode* ref = fCompPB->GetINode(kEnvObject);
    if( !ref )
    {
        ref = node;
    }
    plDynamicEnvMap* env = plEnvMapComponent::GetEnvMap((plMaxNode*)ref);
    if( !env )
    {
        uint32_t size = fCompPB->GetInt(kEnvSize);
        uint32_t i;
        for( i = 9; i > 5; i-- )
        {
            if( (1UL << i) <= size )
                break;
        }
        size = uint32_t(1 << i);

        env = new plDynamicEnvMap(size, size, 32);
        hsgResMgr::ResMgr()->NewKey(ST::string(ref->GetName()), env, node->GetLocation(), node->GetLoadMask());

        Point3 pos = ref->GetNodeTM(TimeValue(0)).GetTrans();
        env->SetPosition(hsPoint3(pos.x, pos.y, pos.z));
        env->SetYon(10000.f);
        env->SetRefreshRate(fCompPB->GetFloat(kEnvRefresh));
    }
    if( !env )
        return false;

    ws.fEnvCenter = env->GetPosition();
    ws.fEnvRefresh = env->GetRefreshRate();

    fWaveSet->SetEnvSize(env->GetWidth());


    plGenRefMsg* refMsg = new plGenRefMsg(fWaveSet->GetKey(), plRefMsg::kOnCreate, -1, plWaveSet7::kRefEnvMap);
    hsgResMgr::ResMgr()->SendRef(env->GetKey(), refMsg, plRefFlags::kActiveRef);

    ws.fEnvRadius = fCompPB->GetFloat(kEnvRadius);


    return true;
}

bool plWaterComponent::IGetRefObject(plMaxNode* node)
{
    plMaxNode* ref = (plMaxNode*)fCompPB->GetINode(kRefObject);
    if( (ref != node)           // We have an exterior reference node
        && ref->CanConvert()    // it's being exported
        && ref->IsTMAnimated() )
    {
        plSceneObject* refObj = ref->GetSceneObject();

        fWaveSet->SetRefObject(refObj);

        return true;
    }

    return false;
}

bool plWaterComponent::IMakeWaveSet(plMaxNode* node, plErrorMsg* pErrMsg)
{
    // Go ahead and create the WaveSet modifier. There will be just
    // one created by this component, everyone has to share.
    fWaveSet = new plWaveSet7;
    hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), fWaveSet, node->GetLocation(), node->GetLoadMask());

    // Set up the parameters
    plFixedWaterState7 ws;

    // Things we get off the reference (plane) object
    // First we look to see if it's an animated runtime object we need to keep track of.
    // Either way, we just grab the static info we need off of it, for init or forever.
    IGetRefObject(node);
    IReadRefObject(node, ws);

    // Things we get from our paramblock
    plFixedWaterState7::WaveState& geoState = ws.fGeoState;
    geoState.fMaxLength = fCompPB->GetFloat(kGeoMaxLen);
    geoState.fMinLength = fCompPB->GetFloat(kGeoMinLen);
    geoState.fAmpOverLen = fCompPB->GetFloat(kGeoAmpOverLen) * kPercentToFrac;
    geoState.fChop = fCompPB->GetFloat(kGeoChop) * kPercentToFrac;
    geoState.fAngleDev = hsDegreesToRadians(fCompPB->GetFloat(kGeoAngleDev));

    plFixedWaterState7::WaveState& texState = ws.fTexState;
    texState.fMaxLength = fCompPB->GetFloat(kTexMaxLen);
    texState.fMinLength = fCompPB->GetFloat(kTexMinLen);
    texState.fAmpOverLen = fCompPB->GetFloat(kTexAmpOverLen) * kPercentToFrac;
    texState.fChop = fCompPB->GetFloat(kTexChop) * kPercentToFrac;
    texState.fAngleDev = hsDegreesToRadians(fCompPB->GetFloat(kTexAngleDev));

    hsVector3 specVec;
    specVec[ws.kNoise] = fCompPB->GetFloat(kNoise) * kPercentToFrac;
    specVec[ws.kSpecStart] = fCompPB->GetFloat(kSpecStart);
    specVec[ws.kSpecEnd] = fCompPB->GetFloat(kSpecEnd);
    ws.fSpecVec = specVec;

    ws.fWispiness = fCompPB->GetFloat(kWispiness) * kPercentToFrac;
    ws.fPeriod = fCompPB->GetFloat(kPeriod) * kPercentToFrac;
    ws.fRippleScale = fCompPB->GetFloat(kRippleScale);
    ws.fFingerLength = fCompPB->GetFloat(kFinger) * kPercentToFrac;

    ws.fDepthFalloff = hsVector3(fCompPB->GetFloat(kDepthOpac), fCompPB->GetFloat(kDepthRefl), fCompPB->GetFloat(kDepthWave));
    ws. fWaterOffset = hsVector3(-fCompPB->GetFloat(kZeroOpac), -fCompPB->GetFloat(kZeroRefl), -fCompPB->GetFloat(kZeroWave));
    ws.fMaxAtten = hsVector3(1.f, 1.f, 1.f);
    ws.fMinAtten = {};

    IReadEnvObject(node, pErrMsg, ws);

    // Some colors
    ws.fWaterTint = hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f);

    Color specTint = fCompPB->GetColor(kSpecularTint);
    float specMute = fCompPB->GetFloat(kSpecularMute) * kPercentToFrac;
    ws.fSpecularTint = hsColorRGBA().Set(specTint.r, specTint.g, specTint.b, specMute);

    ws.fMaxColor = hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f); // Always white/opaque?
    Color shoreTint = fCompPB->GetColor(kShoreTint);
    float shoreOpac = fCompPB->GetFloat(kShoreOpac) * kPercentToFrac;
    ws.fMinColor = hsColorRGBA().Set(shoreTint.r, shoreTint.g, shoreTint.b, shoreOpac);

    // Not really used.
    ws.fShoreTint = hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f);

    ws.fEdgeOpac = fCompPB->GetFloat(kEdgeOpac) * kPercentToFrac;
    ws.fEdgeRadius = fCompPB->GetFloat(kEdgeRadius) * kPercentToFrac;

    fWaveSet->SetState(ws, 0.f);

    // Add a ref to the waveset.
    fWaveSet->GetKey()->RefObject();

    return true;
}

float plWaterComponent::IGetWaterHeight()
{
    plMaxNodeBase* node = nullptr;
    
    int i;
    for( i = 0; i < NumTargets(); i++ )
    {
        node = GetTarget(i);
        if( node )
            break;
    }

    plFixedWaterState7 ws;
    IReadRefObject(node, ws);

    return ws.fWaterHeight;
}

float plWaterComponent::GetWaterHeight(INode* node)
{
    if( !node )
        return 0.f;

    plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
    if( !comp )
        return 0.f;

    if( comp->ClassID() != WATER_COMP_CID )
        return 0.f;

    plWaterComponent* water = (plWaterComponent*)comp;
    return water->IGetWaterHeight();
}


plWaveSetBase* plWaterComponent::GetWaveSet(INode* node)
{
    if( !node )
        return nullptr;

    plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
    if( !comp )
        return nullptr;

    if( comp->ClassID() != WATER_COMP_CID )
        return nullptr;

    plWaterComponent* water = (plWaterComponent*)comp;
    return water->IGetWaveSet();
}

plWaveSetBase* plWaterComponent::GetWaveSetFromNode(plMaxNode* node)
{
    if( !node )
        return nullptr;

    int n = node->NumAttachedComponents();
    int i;
    for( i = 0; i < n; i++ )
    {
        plComponentBase* comp = node->GetAttachedComponent(i);
        if( comp && (comp->ClassID() == WATER_COMP_CID) )
        {
            plWaterComponent* water = (plWaterComponent*)comp;
            return water->IGetWaveSet();
        }
    }
    return nullptr;
}

static void ISetWaterDependencies(plMaxNode* node, INode* waterNode)
{
    if( !waterNode )
        return;

    plComponentBase *comp = ((plMaxNodeBase*)waterNode)->ConvertToComponent();
    if( !comp )
        return;

    if( comp->ClassID() != WATER_COMP_CID )
        return;

    INodeTab nodeList;
    comp->AddTargetsToList(nodeList);
    int i;
    for( i = 0; i < nodeList.Count(); i++ )
    {
        if( nodeList[i] )
            node->AddRenderDependency((plMaxNodeBase*)nodeList[i]);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

class plShoreCompSelProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    void DeleteThis() override { }
};

INT_PTR plShoreCompSelProc::DlgProc(TimeValue t, IParamMap2 *paramMap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            IParamBlock2 *pb = paramMap->GetParamBlock();
            INode* node = pb->GetINode(plShoreComponent::kWaveSet);
            TSTR newName(node ? node->GetName() : _T("Pick"));
            ::SetWindowText(::GetDlgItem(hWnd, IDC_COMP_SHORE_CHOSE), newName);
        }
        return TRUE;

    case WM_COMMAND:
        if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_COMP_SHORE_CHOSE) )
        {
            IParamBlock2 *pb = paramMap->GetParamBlock();
            std::vector<Class_ID> cids;
            cids.push_back(WATER_COMP_CID);
            if( plPick::Node(pb, plShoreComponent::kWaveSet, &cids, true, true) )
            {
                INode* node = pb->GetINode(plShoreComponent::kWaveSet);
                TSTR newName(node ? node->GetName() : _T("Pick"));
                ::SetWindowText(::GetDlgItem(hWnd, IDC_COMP_SHORE_CHOSE), newName);
                paramMap->Invalidate(plShoreComponent::kWaveSet);
                ShowWindow(hWnd, SW_HIDE);
                ShowWindow(hWnd, SW_SHOW);
            }

            return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}

plShoreCompSelProc gShoreCompSelProc;

CLASS_DESC(plShoreComponent, gShoreCompDesc, "Shore Line",  "Shore", COMP_TYPE_WATER, SHORE_COMP_CID)



ParamBlockDesc2 gShoreCompBk
(
    plComponent::kBlkComp, _T("Shore"), 0, &gShoreCompDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_SHORE, IDS_COMP_SHORE,  0, 0, &gShoreCompSelProc,

    plShoreComponent::kWaveSet, _T("WaveSet"),  TYPE_INODE,     0, 0,
        p_end,

    p_end
);

plShoreComponent::plShoreComponent()
{
    fClassDesc = &gShoreCompDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plShoreComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
    node->SetRunTimeLight(true);
    node->SetForceMaterialCopy(true);
    node->SetForceShadow(false);
    node->SetNoShadow(true);
    node->SetSmoothAll(true);
    node->SetForceSortable(true);
    node->SetNoSpanSort(true);
    node->SetNoPreShade(true);

    ISetWaterDependencies(node, fCompPB->GetINode(kWaveSet, 0, 0));

    // Turn on calculation of edge lengths XXX
    node->SetCalcEdgeLens(true);

    // Make a note that we're vertex shaded (at least ripplecomponent needs to know).XXX
    node->SetVS(true);

    // Turn off the convexity test. We want to be sorted.XXX
    node->SetConcave(true);

    node->SetReverseSort(true);

    node->SetWaterHeight(plWaterComponent::GetWaterHeight(fCompPB->GetINode(kWaveSet, 0, 0)));

    return true;
}

bool plShoreComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    pErrMsg->Set(!node->HasLoadMask(), node->GetName(), "PS shore has no representation component").CheckAndAsk();
    pErrMsg->Set(false);


    return true;
}

bool plShoreComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    plWaveSetBase* waveSet = plWaterComponent::GetWaveSet(fCompPB->GetINode(kWaveSet, 0, 0));
    if( waveSet )
    {
        plSceneObject* obj = node->GetSceneObject();
        if( obj )
        {
            waveSet->AddShore(obj->GetKey());
        }
        else
        {
            pErrMsg->Set(true, node->GetName(), "Invalid object selected for shore. Ignoring").CheckAndAsk();
            pErrMsg->Set(false);
        }
    }
    else
    {
        pErrMsg->Set(true, node->GetName(), "No Water Component selected for shore. Ignoring").CheckAndAsk();
        pErrMsg->Set(false);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

class plWDecalCompSelProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    void DeleteThis() override { }
};

INT_PTR plWDecalCompSelProc::DlgProc(TimeValue t, IParamMap2 *paramMap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            IParamBlock2 *pb = paramMap->GetParamBlock();
            INode* node = pb->GetINode(plWDecalComponent::kWaveSet);
            TSTR newName(node ? node->GetName() : _T("Pick"));
            ::SetWindowText(::GetDlgItem(hWnd, IDC_COMP_WDECAL_CHOSE), newName);
        }
        return TRUE;

    case WM_COMMAND:
        if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_COMP_WDECAL_CHOSE) )
        {
            IParamBlock2 *pb = paramMap->GetParamBlock();
            std::vector<Class_ID> cids;
            cids.push_back(WATER_COMP_CID);
            if( plPick::Node(pb, plWDecalComponent::kWaveSet, &cids, true, true) )
            {
                INode* node = pb->GetINode(plWDecalComponent::kWaveSet);
                TSTR newName(node ? node->GetName() : _T("Pick"));
                ::SetWindowText(::GetDlgItem(hWnd, IDC_COMP_WDECAL_CHOSE), newName);
                paramMap->Invalidate(plWDecalComponent::kWaveSet);
                ShowWindow(hWnd, SW_HIDE);
                ShowWindow(hWnd, SW_SHOW);
            }

            return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}

plWDecalCompSelProc gWDecalCompSelProc;

CLASS_DESC(plWDecalComponent, gWDecalCompDesc, "Water Decal",  "WDecal", COMP_TYPE_WATER, WDECAL_COMP_CID)



ParamBlockDesc2 gWDecalCompBk
(
    plComponent::kBlkComp, _T("WDecal"), 0, &gWDecalCompDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_WDECAL, IDS_COMP_WDECAL,  0, 0, &gWDecalCompSelProc,

    plWDecalComponent::kWaveSet, _T("WaveSet"), TYPE_INODE,     0, 0,
        p_end,

    plWDecalComponent::kEnv,  _T("Env"), TYPE_BOOL,         0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_WDECAL_ENV,
        p_end,

    p_end
);

plWDecalComponent::plWDecalComponent()
{
    fClassDesc = &gWDecalCompDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plWDecalComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
    node->SetRunTimeLight(true);
    node->SetForceMaterialCopy(true);
    node->SetForceShadow(false);
    node->SetNoShadow(true);
    node->SetSmoothAll(true);
    node->SetForceSortable(true);
    node->SetNoSpanSort(true);
    node->SetNoPreShade(true);

    // This should be optional.
    if( fCompPB->GetInt(kEnv) )
        node->SetWaterDecEnv(true);

    ISetWaterDependencies(node, fCompPB->GetINode(kWaveSet, 0, 0));

    // Turn on calculation of edge lengths XXX
    node->SetCalcEdgeLens(true);

    // Make a note that we're vertex shaded (at least ripplecomponent needs to know).XXX
    node->SetVS(true);

    // Turn off the convexity test. We want to be sorted.XXX
    node->SetConcave(true);

    node->SetReverseSort(true);

    node->SetWaterHeight(plWaterComponent::GetWaterHeight(fCompPB->GetINode(kWaveSet, 0, 0)));

    return true;
}

bool plWDecalComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    pErrMsg->Set(!node->HasLoadMask(), node->GetName(), "PS water decal has no representation component").CheckAndAsk();
    pErrMsg->Set(false);


    return true;
}

bool plWDecalComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    plWaveSetBase* waveSet = plWaterComponent::GetWaveSet(fCompPB->GetINode(kWaveSet, 0, 0));
    if( waveSet )
    {
        plSceneObject* obj = node->GetSceneObject();
        if( obj )
        {
            waveSet->AddDecal(obj->GetKey());
        }
        else
        {
            pErrMsg->Set(true, node->GetName(), "Invalid object selected for decal. Ignoring").CheckAndAsk();
            pErrMsg->Set(false);
        }
    }
    else
    {
        pErrMsg->Set(true, node->GetName(), "No Water Component selected for decal. Ignoring").CheckAndAsk();
        pErrMsg->Set(false);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

class plEnvMapCompSelProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                for(int i = 0; i < map->GetParamBlock()->Count(plEnvMapComponent::kVisSetNames); i++ )
                {
                    HWND hList = GetDlgItem(hWnd, IDC_COMP_ENVMAP_NAMES_LISTBOX);
                    ListBox_AddString(hList, map->GetParamBlock()->GetStr(plEnvMapComponent::kVisSetNames, 0, i));
                }
            }
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ADD_TARGS)
            {
                std::vector<Class_ID> cids;
                cids.push_back(EFFVISSET_CID);
                IParamBlock2 *pb = map->GetParamBlock();
                plPick::Node(pb, plEnvMapComponent::kVisSets, &cids, false, false);

                map->Invalidate(plEnvMapComponent::kVisSets);
                return TRUE;
            }
            else if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_COMP_ENVMAP_ADD_STRING)
            {
                MCHAR str[256];
                MCHAR *pStr = str;
                ICustEdit *custEdit = GetICustEdit(GetDlgItem(hWnd, IDC_COMP_ENVMAP_ADD_STRING_BOX));
                custEdit->GetText(str, std::size(str));
                custEdit->SetText(_M(""));  // clear text box

                if (str[0] == _M('\0'))    // don't allow empty strings
                    return TRUE;

                HWND hList = GetDlgItem(hWnd, IDC_COMP_ENVMAP_NAMES_LISTBOX);
                ListBox_AddString(hList, pStr);
                map->GetParamBlock()->Append(plEnvMapComponent::kVisSetNames, 1, &pStr);
                return TRUE;
            }
            else if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_COMP_ENVMAP_REMOVE_STRING)
            {
                HWND hList = GetDlgItem(hWnd, IDC_COMP_ENVMAP_NAMES_LISTBOX);
                int curSel = ((int)(DWORD)SNDMSG((hList), LB_GETCURSEL, 0L, 0L));
                ListBox_DeleteString(hList, curSel);
                if (curSel >= 0)
                    map->GetParamBlock()->Delete(ParamID(plEnvMapComponent::kVisSetNames), curSel, 1);

                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plEnvMapCompSelProc gEnvMapCompSelProc;

CLASS_DESC(plEnvMapComponent, gEnvMapCompDesc, "Environment Map",  "EnvMap", COMP_TYPE_WATER, ENVMAP_COMP_CID)



ParamBlockDesc2 gEnvMapCompBk
(
    plComponent::kBlkComp, _T("EnvMap"), 0, &gEnvMapCompDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_ENVMAP, IDS_COMP_ENVMAP,  0, 0, &gEnvMapCompSelProc,

    plEnvMapComponent::kVisSets,    _T("VisSets"),  TYPE_INODE_TAB, 0,      0, 0,
        p_ui,           TYPE_NODELISTBOX, IDC_LIST_TARGS, 0, 0, IDC_DEL_TARGS,
        p_end,

    plEnvMapComponent::kHither, _T("Hither"), TYPE_FLOAT,   0, 0,   
        p_default, 1.0,
        p_range, 0.0, 500.0,
//      p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
//      IDC_COMP_ENVMAP_HITHER, IDC_COMP_ENVMAP_HITHER_SPIN, 1.0,
        p_end,    

    plEnvMapComponent::kYon, _T("Yon"), TYPE_FLOAT,     0, 0,   
        p_default, 1000.0,
        p_range, 10.0, 50000.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_ENVMAP_YON, IDC_COMP_ENVMAP_YON_SPIN, 1.0,
        p_end,    

    plEnvMapComponent::kFogEnable,  _T("FogEnable"), TYPE_BOOL,         0, 0,
        p_default,  FALSE,
//      p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_ENVMAP_FOGENABLE,
//      p_enable_ctrls,     2, plEnvMapComponent::kFogStart, plEnvMapComponent::kFogColor,
        p_end,

    plEnvMapComponent::kFogStart, _T("FogStart"), TYPE_FLOAT,   0, 0,   
        p_default, 0.0,
        p_range, 0.0, 200.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_ENVMAP_FOGSTART, IDC_COMP_ENVMAP_FOGSTART_SPIN, 1.0,
        p_end,    


    plEnvMapComponent::kFogColor, _T("FogColor"), TYPE_RGBA, 0, 0,
        p_ui, TYPE_COLORSWATCH,         IDC_COMP_ENVMAP_FOGCOLOR,
        p_default, Color(0,0,0),
        p_end,

    plEnvMapComponent::kRefreshRate, _T("RefreshRate"), TYPE_FLOAT,     0, 0,   
        p_default, 0.0,
        p_range, 0.0, 3600.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_ENVMAP_REFRESHRATE, IDC_COMP_ENVMAP_REFRESHRATE_SPIN, 1.0,
        p_end,    

    plEnvMapComponent::kEnvSize, _T("EnvSize"), TYPE_INT, 0, 0,
        p_default, 256,
        p_range, 32, 1024,
        p_ui,   TYPE_SPINNER, EDITTYPE_INT, 
        IDC_COMP_ENVMAP_ENVSIZE, IDC_COMP_ENVMAP_ENVSIZE_SPIN,  1.f,
        p_end,

    plEnvMapComponent::kIncChars,  _T("IncChars"), TYPE_BOOL,       0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_ENVMAP_INCCHARS,
        p_end,

    plEnvMapComponent::kMapType, _T("mapType"), TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 2, IDC_COMP_ENVMAP_CUBIC, IDC_COMP_ENVMAP_SINGLE_CAM,
        p_vals,     plEnvMapComponent::kMapCubic, plEnvMapComponent::kMapSingle,
        p_default,  plEnvMapComponent::kMapCubic,
        p_end,

    plEnvMapComponent::kVisSetNames, _T("VisSetNames"), TYPE_STRING_TAB, 0, 0, 0,
        p_end,

    p_end
);

plEnvMapComponent::plEnvMapComponent()
{
    fClassDesc = &gEnvMapCompDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plEnvMapComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
    fMap = nullptr;

    node->SetForceLocal(true);

    return true;
}

bool plEnvMapComponent::PreConvert(plMaxNode* node, plErrorMsg* pErrMsg)
{

    return true;
}

bool plEnvMapComponent::Convert(plMaxNode* node, plErrorMsg* pErrMsg)
{
    // If we make a handler that will update the EnvMap's position when this object
    // moves, we can put it's creation here. Otherwise, there's nought to do, since
    // we generate the envmap on demand.
    return true;
}

plDynamicEnvMap* plEnvMapComponent::GetEnvMap()
{
    return plDynamicEnvMap::ConvertNoRef(IGetMap());
}

plDynamicCamMap* plEnvMapComponent::GetCamMap()
{
    return plDynamicCamMap::ConvertNoRef(IGetMap());
}

plRenderTarget* plEnvMapComponent::IGetMap()
{
    plMaxNode* firstTarg = nullptr;
    int numTarg = NumTargets();
    int i;
    for( i = 0; i < numTarg; i++ )
    {
        if( GetTarget(i) )
        {
            firstTarg = (plMaxNode*)GetTarget(i);
            break;
        }
    }
    if( !firstTarg )
        return nullptr;

    if( !fMap )
    {
        uint32_t size = fCompPB->GetInt(kEnvSize);
        for( i = 9; i > 5; i-- )
        {
            if( (1UL << uint32_t(i)) <= size )
                break;
        }
        size = 1 << uint32_t(i);

        plDynamicEnvMap* env = nullptr;
        plDynamicCamMap* cam = nullptr;
        fMap = nullptr;
        if (fCompPB->GetInt((ParamID(kMapType))) == kMapCubic)
            fMap = env = new plDynamicEnvMap(size, size, 32);
        else if (fCompPB->GetInt((ParamID(kMapType))) == kMapSingle)
            fMap = cam = new plDynamicCamMap(size, size, 32);

        // Need to assign the key before we call all the setup functions.
        hsgResMgr::ResMgr()->NewKey(ST::string(GetINode()->GetName()), fMap, firstTarg->GetLocation(), firstTarg->GetLoadMask());


        if (fCompPB->GetInt((ParamID(kMapType))) == kMapCubic)
        {
            Point3 pos = firstTarg->GetNodeTM(TimeValue(0)).GetTrans();
            env->SetPosition(hsPoint3(pos.x, pos.y, pos.z));
            env->SetRefreshRate(fCompPB->GetFloat(kRefreshRate));
            env->SetHither(fCompPB->GetFloat(kHither));
            env->SetYon(fCompPB->GetFloat(kYon));
            env->SetFogStart(fCompPB->GetFloat(kFogStart) * kPercentToFrac);
            Color fogColor = fCompPB->GetColor(kFogColor);
            env->SetColor(hsColorRGBA().Set(fogColor.r, fogColor.g, fogColor.b, 1.f));
        }
        else if (fCompPB->GetInt((ParamID(kMapType))) == kMapSingle)
        {
            cam->SetRefreshRate(fCompPB->GetFloat(ParamID(kRefreshRate)));
            cam->fHither = fCompPB->GetFloat(ParamID(kHither));
            cam->fYon = fCompPB->GetFloat(ParamID(kYon));
            cam->fFogStart = fCompPB->GetFloat(ParamID(kFogStart)) * kPercentToFrac;
            Color fogColor = fCompPB->GetColor(kFogColor);
            cam->fColor.Set(fogColor.r, fogColor.g, fogColor.b, 1.f);
        }
        if (!fMap)
            return nullptr;

        int visGot = 0;
        int numVis = fCompPB->Count(kVisSets);
        for( i = 0; i < numVis; i++ )
        {
            plEffVisSetComponent* effComp = plEffVisSetComponent::ConvertToEffVisSetComponent((plMaxNode*)fCompPB->GetINode(kVisSets, 0, i));
            if( effComp )
            {
                plVisRegion* effReg = effComp->GetVisRegion(firstTarg);
                if( effReg )
                {
                    plGenRefMsg* refMsg = new plGenRefMsg(fMap->GetKey(), plRefMsg::kOnCreate, -1, plDynamicEnvMap::kRefVisSet);
                    hsgResMgr::ResMgr()->SendRef(effReg->GetKey(), refMsg, plRefFlags::kPassiveRef);

                    visGot++;
                }
            }
        }

        // This allows you to enter the name of an effect vis set(key name), from another max file and use it 
        // as if it we're in the same max file.
        int numVisNames = fCompPB->Count(kVisSetNames);
        for( i = 0; i < numVisNames; i++)
        {
            fMap->SetVisRegionName(fCompPB->GetStr(kVisSetNames, 0, i));
        }

        if (visGot)
        {
            if (env)
                env->SetIncludeCharacters(fCompPB->GetInt(ParamID(kIncChars)) != 0);
            if (cam)
                cam->SetIncludeCharacters(fCompPB->GetInt(ParamID(kIncChars)) != 0);
        }

        // Right now, the envMap doesn't use this, but I plan to make it do so, so I'm
        // going ahead and adding the ref regardless of which type of map we made.
        uint8_t refType = cam ? plDynamicCamMap::kRefRootNode : plDynamicEnvMap::kRefRootNode;
        hsgResMgr::ResMgr()->AddViaNotify(firstTarg->GetSceneObject()->GetKey(), new plGenRefMsg(fMap->GetKey(), plRefMsg::kOnCreate, -1, refType), plRefFlags::kPassiveRef);
    }
    return fMap;
}

plDynamicEnvMap* plEnvMapComponent::GetEnvMap(plMaxNode* node)
{
    plEnvMapComponent* envComp = GetEnvMapComponent(node);
    if (envComp)
        return envComp->GetEnvMap();

    return nullptr;
}

plDynamicCamMap* plEnvMapComponent::GetCamMap(plMaxNode *node)
{
    plEnvMapComponent *envComp = GetEnvMapComponent(node);
    if (envComp)
        return envComp->GetCamMap();

    return nullptr;
}

plEnvMapComponent *plEnvMapComponent::GetEnvMapComponent(plMaxNode *node)
{
    if (!node)
        return nullptr;

    int n = node->NumAttachedComponents();
    int i;
    for (i = 0; i < n; i++)
    {
        plComponentBase *comp = node->GetAttachedComponent(i);
        if (comp && (comp->ClassID() == ENVMAP_COMP_CID))
        {
            return (plEnvMapComponent*)comp;
        }
    }
    return nullptr;
}
