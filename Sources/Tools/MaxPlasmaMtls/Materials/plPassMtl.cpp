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
#include "hsBitVector.h"

#include "MaxMain/MaxAPI.h"

#include "../resource.h"

#include "plPassMtl.h"
#include "../Shaders.h"
#include "MaxComponent/plMaxAnimUtils.h"

#include "plPassBaseParamIDs.h"
#include "plPassMtlBasicPB.h"
#include "plPassMtlLayersPB.h"

#include "../Layers/plLayerTex.h"
#include "../Layers/plStaticEnvLayer.h"

extern HINSTANCE hInstance;

class plPassMtlClassDesc : public plClassDesc2
{
public:
    int             IsPublic() override     { return TRUE; }
    void*           Create(BOOL loading) override { return new plPassMtl(loading); }
    const MCHAR*    ClassName() override    { return GetString(IDS_PASS_MTL); }
    SClass_ID       SuperClassID() override { return MATERIAL_CLASS_ID; }
    Class_ID        ClassID() override      { return PASS_MTL_CLASS_ID; }
    const MCHAR*    Category() override     { return nullptr; }
    const MCHAR*    InternalName() override { return _M("PlasmaMaterial"); }
    HINSTANCE       HInstance() override    { return hInstance; }
};
static plPassMtlClassDesc plPassMtlDesc;
ClassDesc2* GetPassMtlDesc() { return &plPassMtlDesc; }

// For initializing paramblock descriptor
ParamBlockDesc2 *GetPassBasicPB();
ParamBlockDesc2 *GetPassAdvPB();
ParamBlockDesc2 *GetPassLayersPB();

#include "plPassMtlAdvPBDec.h"
#include "plPassMtlBasicPBDec.h"
#include "plPassMtlLayersPBDec.h"
#include "plPassMtlAnimPBDec.h"

#include "plAnimStealthNode.h"

plPassMtl::plPassMtl(BOOL loading) : plPassMtlBase( loading )
{
    plPassMtlDesc.MakeAutoParamBlocks( this );
    fLayersPB->SetValue( kPassLayBase, 0, new plLayerTex );
    fLayersPB->SetValue( kPassLayTop, 0, new plLayerTex );

    // If we do this later (like, when the dialog loads) something blows up,
    // somewhere in Max.  It didn't in 4, it does in 7.  This seems to fix it.
    if (!loading)
        IVerifyStealthPresent(ENTIRE_ANIMATION_NAME);
}

plPassMtl::~plPassMtl()
{
}

void plPassMtl::GetClassName(MSTR& s MAX_NAME_LOCALIZED2) MAX24_CONST
{
    s = GetString(IDS_PASS_MTL);
}

ParamDlg* plPassMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) 
{
    fIMtlParams = imp;
    IAutoMParamDlg* masterDlg = plPassMtlDesc.CreateParamDlgs(hwMtlEdit, imp, this);

    return (ParamDlg*)masterDlg;    
}

BOOL plPassMtl::SetDlgThing(ParamDlg* dlg)
{
    return FALSE;
}

Interval plPassMtl::Validity(TimeValue t)
{
#if 0 // mf horse
    Interval valid = FOREVER;       

/*  for (int i = 0; i < fSubTexmap.Count(); i++) 
    {
        if (fSubTexmap[i]) 
            valid &= fSubTexmap[i]->Validity(t);
    }
*/  
//  float u;
//  fPBlock->GetValue(pb_spin,t,u,valid);
    return valid;
#else // mf horse
    auto name = GetName();

    // mf horse - Hacking in something like real validity checking
    // to get material animations working. No warranty, this is just
    // better than nothing.
    Interval v = FOREVER;
    fBasicPB->GetValidity(t, v);
    fAdvPB->GetValidity(t, v);

    if( fLayersPB->GetTexmap(kPassLayBase) )
        v &= fLayersPB->GetTexmap(kPassLayBase)->Validity(t);
    if( fLayersPB->GetTexmap(kPassLayTop) )
        v &= fLayersPB->GetTexmap(kPassLayTop)->Validity(t);
    return v;
#endif // mf horse
}

//// GetReference ////////////////////////////////////////////////////////////
//  Note: need to overload because MAX for some reason writes out the
//  references by their INDEX. ARRRRGH!

RefTargetHandle plPassMtl::GetReference( int i )
{
    switch( i )
    {
        case kRefBasic:  return fBasicPB;
        case kRefAdv:    return fAdvPB;
        case kRefLayers: return fLayersPB;
        case kRefAnim:   return fAnimPB;
    }

    return plPassMtlBase::GetReference( i );
}

//// SetReference ////////////////////////////////////////////////////////////
//  Note: need to overload because MAX for some reason writes out the
//  references by their INDEX. ARRRRGH!

void plPassMtl::SetReference(int i, RefTargetHandle rtarg)
{
    if (i == kRefBasic)
        fBasicPB = (IParamBlock2 *)rtarg;
    else if (i == kRefAdv)
        fAdvPB = (IParamBlock2 *)rtarg;
    else if (i == kRefLayers)
        fLayersPB = (IParamBlock2 *)rtarg;
    else if (i == kRefAnim)
        fAnimPB = (IParamBlock2 *)rtarg;
    else
        plPassMtlBase::SetReference( i, rtarg );
}

/*===========================================================================*\
 |  Subanim & References support
\*===========================================================================*/

int plPassMtl::NumSubs()
{
    return 6;
}

MSTR plPassMtl::SubAnimName(int i MAX_NAME_LOCALIZED2)
{
    switch (i)
    {
    case 0: return fBasicPB->GetLocalName();
    case 1: return fAdvPB->GetLocalName();
    case 2: return fLayersPB->GetLocalName();
    case 3: return fAnimPB->GetLocalName();
    case 4: return _M("Base Layer");
    case 5: return _M("Top Layer");
    }

    return _M("");
}

Animatable* plPassMtl::SubAnim(int i)
{
    switch (i)
    {
    case 0: return fBasicPB;
    case 1: return fAdvPB;
    case 2: return fLayersPB;
    case 3: return fAnimPB;
    case 4: return fLayersPB->GetTexmap(kPassLayBase);
    case 5:
        if (fLayersPB->GetInt(kPassLayTopOn))
            return fLayersPB->GetTexmap(kPassLayTop);
        break;
    }

    return nullptr;
}

int plPassMtl::NumParamBlocks()
{
    return 4;
}

IParamBlock2* plPassMtl::GetParamBlock(int i)
{
    return (IParamBlock2*)GetReference(i);
}

IParamBlock2* plPassMtl::GetParamBlockByID(BlockID id)
{
    if (fBasicPB->ID() == id)
        return fBasicPB;
    else if (fAdvPB->ID() == id)
        return fAdvPB;
    else if (fLayersPB->ID() == id)
        return fLayersPB;
    else if (fAnimPB->ID() == id)
        return fAnimPB;

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Subtexmap access

int plPassMtl::NumSubTexmaps()
{
    return 2;
}

Texmap* plPassMtl::GetSubTexmap(int i)
{
    if (i == 0)
        return fLayersPB->GetTexmap(kPassLayBase);
    else if (i == 1)
        return fLayersPB->GetTexmap(kPassLayTop);

    return nullptr;
}

void plPassMtl::SetSubTexmap(int i, Texmap *m)
{
    if (i == 0)
        fLayersPB->SetValue(kPassLayBase, 0, m);
    else if (i == 1)
        fLayersPB->SetValue(kPassLayTop, 0, m);
}

MSTR plPassMtl::GetSubTexmapSlotName(int i MAX_NAME_LOCALIZED2)
{
    if (i == 0)
        return _M("Base");
    else if (i == 1)
        return _M("Top");

    return _M("");
}

MSTR plPassMtl::GetSubTexmapTVName(int i)
{
    return GetSubTexmapSlotName(i);
}

int plPassMtl::SubTexmapOn(int i)
{
   if (i == 0)
      return 1;
   else if (i == 1 && fLayersPB->GetInt(kPassLayTopOn))
      return 1;

   return 0;
}


/*===========================================================================*\
 |  Updating and cloning
\*===========================================================================*/

RefTargetHandle plPassMtl::Clone(RemapDir &remap)
{
    plPassMtl *mnew = new plPassMtl(FALSE);
    plPassMtlBase::ICloneBase( mnew, remap );
    return (RefTargetHandle)mnew;
}

void    plPassMtl::ICloneRefs( plPassMtlBase *target, RemapDir &remap )
{
    target->ReplaceReference(kRefBasic, remap.CloneRef(fBasicPB));
    target->ReplaceReference(kRefAdv, remap.CloneRef(fAdvPB));
    target->ReplaceReference(kRefLayers, remap.CloneRef(fLayersPB));
    target->ReplaceReference(kRefAnim, remap.CloneRef(fAnimPB));
}

void plPassMtl::NotifyChanged() 
{
    NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

void plPassMtl::Update(TimeValue t, Interval& valid) 
{   
    // mf horse - Hacking in something like real validity checking
    // to get material animations working. No warranty, this is just
    // better than nothing.
    if (!fIValid.InInterval(t))
    {
        fIValid.SetInfinite();
        if( fLayersPB->GetTexmap(kPassLayBase) )
            fLayersPB->GetTexmap(kPassLayBase)->Update(t, fIValid);
        if( fLayersPB->GetTexmap(kPassLayTop) )
            fLayersPB->GetTexmap(kPassLayTop)->Update(t, fIValid);

//      fLayersPB->GetValue(kMtlLayLayer1On, t, fMapOn[0], fIValid);
/*
        for (int i = 0; i < fSubTexmap.Count(); i++)
        {
            if (fSubTexmap[i]) 
                fSubTexmap[i]->Update(t,fIValid);
        }
*/
    }

    // Our wonderful way of version handling--if the runtimeColor is (-1,-1,-1), we know it's
    // just been initialized, so set it to the static color (this lets us do the right thing for
    // loading old paramBlocks)
    if( fBasicPB )
    {
        Color   run = fBasicPB->GetColor( kPassBasRunColor, 0 );
        if( run == Color(-1,-1,-1) )
        {
            fBasicPB->SetValue( kPassBasRunColor, 0, fBasicPB->GetColor( kPassBasColor, 0 ) );
        }

        // Also, if shineStr is anything other than -1, then it must be an old paramblock and we need
        // to convert to our new specColor (we know this because the original valid range was 0-100)
        int shine = fBasicPB->GetInt( kPassBasShineStr, 0 );
        if( shine != -1 )
        {
            fBasicPB->SetValue( kPassBasSpecColor, 0, Color( (float)shine / 100.f, (float)shine / 100.f, (float)shine / 100.f ) );
            fBasicPB->SetValue( kPassBasShineStr, 0, (int)-1 );
        }       
    }

    valid &= fIValid;
}

/*===========================================================================*\
 |  Determine the characteristics of the material
\*===========================================================================*/

void plPassMtl::SetAmbient(Color c, TimeValue t) {}     
void plPassMtl::SetDiffuse(Color c, TimeValue t) {}     
void plPassMtl::SetSpecular(Color c, TimeValue t) {}
void plPassMtl::SetShininess(float v, TimeValue t) {}
                
Color plPassMtl::GetAmbient(int mtlNum, BOOL backFace)  { return Color(0,0,0); }
Color plPassMtl::GetDiffuse(int mtlNum, BOOL backFace)  { return Color(0,0,0); }
Color plPassMtl::GetSpecular(int mtlNum, BOOL backFace) { return Color(0,0,0); }

float plPassMtl::GetXParency(int mtlNum, BOOL backFace)
{
    int         opacity = fBasicPB->GetInt( kPassBasOpacity, 0 );
    float       alpha = 1.0f - ( (float)opacity / 100.0f );

    return alpha;
}

float plPassMtl::GetShininess(int mtlNum, BOOL backFace)    { return 0.0f; }
float plPassMtl::GetShinStr(int mtlNum, BOOL backFace)  { return 0.0f; }
float plPassMtl::WireSize(int mtlNum, BOOL backFace)        { return 0.0f; }

/////////////////////////////////////////////////////////////////

void plPassMtl::SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb)
{
#if 0
    if (texHandleValid.InInterval(t)) {
        mtl->texture.SetCount(numTexHandlesUsed);
        for (int i=0; i<numTexHandlesUsed; i++) {
            if (texHandle[i]) {
                mtl->texture[i].textHandle = texHandle[i]->GetHandle();
                Texmap *tx = (*maps)[useSubForTex[i]].map;
                cb.GetGfxTexInfoFromTexmap(t, mtl->texture[i], tx );        
                SetTexOps(mtl,i,texOpsType[i]);
                }
            }
        return;
        }
#endif

#if 0   // WTF?!?!?!?
    Texmap *tx[2];
    int diffChan = stdIDToChannel[ ID_DI ];
    int opacChan = stdIDToChannel[ ID_OP ];
    tx[0] = (*maps)[diffChan].IsActive() ? (*maps)[diffChan].map : nullptr;
    tx[1] = (*maps)[opacChan].IsActive() ? (*maps)[opacChan].map : nullptr;
#endif

    int nsupport = cb.NumberTexturesSupported();
#if 0
    BITMAPINFO *bmi[NTEXHANDLES];

    int nmaps=0;
    for (int i=0; i<NTEXHANDLES; i++) {
        if (tx[i]) nmaps ++;
        bmi[i] = nullptr;
        }
    mtl->texture.SetCount(nmaps);
    if (nmaps==0) 
        return;
    for (i=0; i<nmaps; i++)
        mtl->texture[i].textHandle = nullptr;
    texHandleValid.SetInfinite();
    Interval  valid;
    BOOL needDecal = FALSE;
    int ntx = 0;
    int op; 

    int forceW = 0;
    int forceH = 0;
    if (tx[0]) {
        cb.GetGfxTexInfoFromTexmap(t, mtl->texture[0], tx[0]);      
        TextureInfo &ti = mtl->texture[0];
        if (ti.tiling[0]==GW_TEX_NO_TILING||ti.tiling[1]==GW_TEX_NO_TILING)
            needDecal = TRUE;
        op = needDecal?TXOP_ALPHABLEND:TXOP_MODULATE;
        bmi[0] = tx[0]->GetVPDisplayDIB(t,cb,valid,FALSE); 
        if (bmi[0]) {
            texHandleValid &= valid;
            useSubForTex[0] = diffChan;
            ntx = 1;
            forceW = bmi[0]->bmiHeader.biWidth;
            forceH = bmi[0]->bmiHeader.biHeight;
            }
        }
    if (tx[1]) {
        cb.GetGfxTexInfoFromTexmap(t, mtl->texture[ntx], tx[1]);        
        if (nsupport>ntx) {
            bmi[1] = tx[1]->GetVPDisplayDIB(t,cb,valid,TRUE); 
            if (bmi[1]) {
                texHandleValid &= valid;
                StuffAlpha(bmi[1], (*maps)[opacChan].amount, GetOpacity(t),ntx?whiteCol:pShader->GetDiffuseClr(t));
                texHandle[ntx] = cb.MakeHandle(bmi[1]); 
                bmi[1] = nullptr;
                mtl->texture[ntx].textHandle = texHandle[ntx]->GetHandle();
                SetTexOps(mtl,ntx,TXOP_OPACITY);
                useSubForTex[ntx] = opacChan;
                ntx++;
                }
            }
        else {
            if (!needDecal) {
                TextureInfo ti;
//              if (SameUV(mtl->texture[0],mtl->texture[1])) {
                    // Not really correct to combine channels for different UV's but what the heck.
                    bmi[1] = tx[1]->GetVPDisplayDIB(t,cb,valid,TRUE, forceW, forceH); 
                    if (bmi[1]) {
                        texHandleValid &= valid;
                        StuffAlphaInto(bmi[1], bmi[0], (*maps)[opacChan].amount, GetOpacity(t));
                        op = TXOP_OPACITY;
                        free(bmi[1]);
                        bmi[1] = nullptr;
                        }
//                  }
                }
            }
        }
    if (bmi[0]) {
        texHandle[0] = cb.MakeHandle(bmi[0]); 
        bmi[0] = nullptr;
        mtl->texture[0].textHandle = texHandle[0]->GetHandle();
        SetTexOps(mtl,0,op);
        }
    mtl->texture.SetCount(ntx);
    numTexHandlesUsed = ntx;
#endif
}

/*===========================================================================*\
 |  Actual shading takes place
\*===========================================================================*/

void plPassMtl::GetInterpVtxValue(int channel, ShadeContext &sc, Point3 &val)
{
    Mesh *mesh = sc.globContext->GetRenderInstance(sc.NodeID())->mesh;
    if (mesh != nullptr)
    {
        Face *maxFace = &mesh->faces[ sc.FaceNumber() ];
        UVVert *map = mesh->mapVerts(channel);
        if (map != nullptr)
        {
            Point3 p0 = map[maxFace->getVert( 0 )];
            Point3 p1 = map[maxFace->getVert( 1 )];
            Point3 p2 = map[maxFace->getVert( 2 )];
            Point3 interp = sc.BarycentricCoords();
            val.x = interp.x * p0.x + interp.y * p1.x + interp.z * p2.x;
            val.y = interp.x * p0.y + interp.y * p1.y + interp.z * p2.y;
            val.z = interp.x * p0.z + interp.y * p1.z + interp.z * p2.z;
            return;
        }
    }

    // No value defined... set default.
    if (channel == MAP_SHADING)
        val.x = val.y = val.z = 0.0f;
    else
        val.x = val.y = val.z = 1.0f;
}

void plPassMtl::Shade(ShadeContext& sc) 
{
    // Get the background color
    Color backColor, backTrans;
    sc.GetBGColor(backColor, backTrans);

    ShadeWithBackground(sc, backColor);
}

//// Requirements ////////////////////////////////////////////////////////////
//  Tells MAX what we need to render ourselves properly, such as translucency,
//  two-sidedness, etc. Flags are in imtl.h in the MAX SDK.

ULONG   plPassMtl::Requirements( int subMtlNum ) 
{
    ULONG       req = 0;


    req = Mtl::Requirements( subMtlNum );

    // Uncomment this to get the background color fed to our ShadeWithBackground()
    // (slower processing tho)
    //req |= MTLREQ_BGCOL;
    req |= MTLREQ_UV;

    int blendType = fLayersPB->GetInt( kPassLayOutputBlend );
    if( blendType == kBlendAdd )
        req |= MTLREQ_ADDITIVE_TRANSP | MTLREQ_TRANSP;
    else if( blendType == kBlendAlpha )
        req |= MTLREQ_TRANSP;
    else if( fBasicPB->GetInt( kPassBasOpacity, 0 ) != 100 )
        req |= MTLREQ_TRANSP;

    if( fAdvPB->GetInt( kPBAdvTwoSided ) )
        req |= MTLREQ_2SIDE;

    if (req & MTLREQ_FACEMAP)
    {
        int i = 0;
    }
    return req;
}

void plPassMtl::ShadeWithBackground(ShadeContext &sc, Color background, bool useVtxAlpha /* = true */)
{
#if 1

    // old
#if 0
    Color lightCol,rescol, diffIllum0;
    RGBA mval;
    Point3 N0,P;
    BOOL bumped = FALSE;
    int i;

    if (gbufID) 
        sc.SetGBufferID(gbufID);
    
    if (sc.mode == SCMODE_SHADOW) {
        float opac = 0.0;
        for (i=0; i < NumSubTexmaps(); i++)     {
            if (SubTexmapOn(i)) {
                hsMaxLayerBase *hsmLay = (hsMaxLayerBase *)GetSubTexmap(i);
                opac += hsmLay->GetOpacity(t);
            }
        }
        
        float f = 1.0f - opac;
        sc.out.t = Color(f,f,f);
        return;
    }
    
    N0 = sc.Normal();
    P = sc.P();
#endif

    TimeValue t = sc.CurTime();
    Color color(0, 0, 0);
    float alpha = 0.0;

    // Evaluate Base layer
    Texmap *map = fLayersPB->GetTexmap(kPassLayBase);
    if (map && ( map->ClassID() == LAYER_TEX_CLASS_ID 
                || map->ClassID() == STATIC_ENV_LAYER_CLASS_ID ) )
    {
        plLayerTex *layer = (plLayerTex*)map;
        AColor evalColor = layer->EvalColor(sc);

        color = evalColor;
        alpha = evalColor.a;
    }

    // Evaluate Top layer, if it's on
    if (fLayersPB->GetInt(kPassLayTopOn))
    {
        Texmap *map = fLayersPB->GetTexmap(kPassLayTop);
        if (map && ( map->ClassID() == LAYER_TEX_CLASS_ID 
                    || map->ClassID() == STATIC_ENV_LAYER_CLASS_ID 
                    || map->ClassID() == ANGLE_ATTEN_LAYER_CLASS_ID) )
        {
            plPlasmaMAXLayer *layer = (plPlasmaMAXLayer*)map;
            AColor evalColor = layer->EvalColor(sc);

            // Blend layers
            if( !layer->DiscardColor() )
            {
                int blendType = fLayersPB->GetInt(kPassLayBlend);
                switch (blendType)
                {
                case kBlendAdd:
                    color += evalColor * evalColor.a;
                    break;
                case kBlendAlpha:
                    color = (1.0f - evalColor.a) * color + evalColor.a * evalColor;
                    break;
                case kBlendMult:
                    color *= evalColor;
                    break;
                default:    // No blend...
                    color = evalColor;
                    break;
                }
            }
            if( !layer->DiscardAlpha() )
            {
                int alphaType = fLayersPB->GetInt(kPassLayOutputBlend);
                switch( alphaType )
                {
                case kAlphaMultiply:
                    alpha *= evalColor.a;
                    break;
                case kAlphaAdd:
                    alpha += evalColor.a;
                    break;
                case kAlphaDiscard:
                default:
                    break;
                }
            }
        }
    }

#if 1
    AColor black;
    black.Black();
    AColor white;
    white.White();


    SIllumParams ip;
    if (fBasicPB->GetInt(kPassBasEmissive))
    {
        // Emissive objects don't get shaded
        ip.diffIllum = fBasicPB->GetColor(kPassBasColorAmb, t) * color;
        ip.diffIllum.ClampMinMax();
        ip.specIllum = black;
    }
    else
    {
        //
        // Shading setup
        //

        // Setup the parameters for the shader
        ip.amb = fBasicPB->GetColor(kPassBasColorAmb, t);
        ip.diff = fBasicPB->GetColor(kPassBasColor, t) * color;
        ip.diffIllum = black;
        ip.specIllum = black;
        ip.N = sc.Normal();
        ip.V = sc.V();


        //
        // Specularity
        //
        if (fBasicPB->GetInt(kPassBasUseSpec, t))
        {
            ip.sh_str = 1.f;
            ip.spec = fBasicPB->GetColor( kPassBasSpecColor, t );
            ip.ph_exp = (float)pow(2.0f,float(fBasicPB->GetInt(kPassBasShine, t)) / 10.0f);
            ip.shine = float(fBasicPB->GetInt(kPassBasShine, t)) / 100.0f;
        }
        else
        {
            ip.spec = black;
            ip.sh_str = 0;
            ip.ph_exp = 0;
            ip.shine = 0;
        }
        ip.softThresh = 0;

        //

        // Do the shading
        Shader *myShader = GetShader(SHADER_BLINN);
        myShader->Illum(sc, ip);

        // Override shader parameters
        if (fAdvPB->GetInt(kPBAdvNoShade))
        {
            ip.diffIllum = black;
            ip.specIllum = black;
        }
        if (fAdvPB->GetInt(kPBAdvWhite))
        {
            ip.diffIllum = white;
            ip.specIllum = black;
        }

        ip.specIllum.ClampMinMax();
        ip.diffIllum = ip.amb * sc.ambientLight + ip.diff * ip.diffIllum;
        ip.diffIllum.ClampMinMax();
    }

//  AColor returnColor = AColor(opac * ip.diffIllum + ip.specIllum, opac)
#endif

    // Get opacity and combine with alpha
    float opac = float(fBasicPB->GetInt(kPassBasOpacity, t)) / 100.0f;
    alpha *= opac;

    float vtxAlpha = 1.0f;
    if (useVtxAlpha && GetOutputBlend() == plPassMtlBase::kBlendAlpha)
    {
        Point3 p;
        GetInterpVtxValue(MAP_ALPHA, sc, p);
        vtxAlpha = p.x;
    }
    alpha *= vtxAlpha;

    // MAX will do the additive/alpha/no blending for us based on what Requirements()
    // we tell it. However, since MAX's formula is bgnd*sc.out.t + sc.out.c,
    // we have to multiply our output color by the alpha.
    // If we ever need a more complicated blending function, you can request the
    // background color via Requirements() (otherwise it's just black) and then do
    // the blending yourself; however, if the transparency isn't set, the shadows
    // will be opaque, so be careful.
    Color outC = ip.diffIllum + ip.specIllum;

    sc.out.c = ( outC * alpha );
    sc.out.t = Color( 1.f - alpha, 1.f - alpha, 1.f - alpha );

#endif
}

float plPassMtl::EvalDisplacement(ShadeContext& sc)
{
    return 0.0f;
}

Interval plPassMtl::DisplacementValidity(TimeValue t)
{
    Interval iv;
    iv.SetInfinite();

    return iv;  
}

bool plPassMtl::HasAlpha()
{
    return ((plLayerTex *)fLayersPB->GetTexmap(kPassLayBase))->HasAlpha();
}
// Massive list of inherited accessor functions for ParamBlock data

// Advanced Block
int     plPassMtl::GetBasicWire() { return fAdvPB->GetInt(kPBAdvWire); }
int     plPassMtl::GetMeshOutlines() { return fAdvPB->GetInt(kPBAdvMeshOutlines); }
int     plPassMtl::GetTwoSided() { return fAdvPB->GetInt(kPBAdvTwoSided); }
int     plPassMtl::GetSoftShadow() { return fAdvPB->GetInt(kPBAdvSoftShadow); }
int     plPassMtl::GetNoProj() { return fAdvPB->GetInt(kPBAdvNoProj); }
int     plPassMtl::GetVertexShade() { return fAdvPB->GetInt(kPBAdvVertexShade); }
int     plPassMtl::GetNoShade() { return fAdvPB->GetInt(kPBAdvNoShade); }
int     plPassMtl::GetNoFog() { return fAdvPB->GetInt(kPBAdvNoFog); }
int     plPassMtl::GetWhite() { return fAdvPB->GetInt(kPBAdvWhite); }
int     plPassMtl::GetZOnly() { return fAdvPB->GetInt(kPBAdvZOnly); }
int     plPassMtl::GetZClear() { return fAdvPB->GetInt(kPBAdvZClear); }
int     plPassMtl::GetZNoRead() { return fAdvPB->GetInt(kPBAdvZNoRead); }
int     plPassMtl::GetZNoWrite() { return fAdvPB->GetInt(kPBAdvZNoWrite); }
int     plPassMtl::GetZInc() { return fAdvPB->GetInt(kPBAdvZInc); }
int     plPassMtl::GetAlphaTestHigh() { return fAdvPB->GetInt(kPBAdvAlphaTestHigh); }

// Animation block
const MCHAR*  plPassMtl::GetAnimName() { return fAnimPB->GetStr(kPBAnimName); }
int     plPassMtl::GetAutoStart() { return fAnimPB->GetInt(kPBAnimAutoStart); }
int     plPassMtl::GetLoop() { return fAnimPB->GetInt(kPBAnimLoop); }
const MCHAR*  plPassMtl::GetAnimLoopName() { return fAnimPB->GetStr(kPBAnimLoopName); }
int     plPassMtl::GetEaseInType() { return fAnimPB->GetInt(kPBAnimEaseInType); }
float   plPassMtl::GetEaseInNormLength() { return fAnimPB->GetFloat(kPBAnimEaseInLength); }
float   plPassMtl::GetEaseInMinLength() { return fAnimPB->GetFloat(kPBAnimEaseInMin); }
float   plPassMtl::GetEaseInMaxLength() { return fAnimPB->GetFloat(kPBAnimEaseInMax); }
int     plPassMtl::GetEaseOutType() { return fAnimPB->GetInt(kPBAnimEaseOutType); }
float   plPassMtl::GetEaseOutNormLength() { return fAnimPB->GetFloat(kPBAnimEaseOutLength); }
float   plPassMtl::GetEaseOutMinLength() { return fAnimPB->GetFloat(kPBAnimEaseOutMin); }
float   plPassMtl::GetEaseOutMaxLength() { return fAnimPB->GetFloat(kPBAnimEaseOutMax); }
int     plPassMtl::GetUseGlobal() { return fAnimPB->GetInt(ParamID(kPBAnimUseGlobal)); }
const MCHAR*  plPassMtl::GetGlobalVarName() { return fAnimPB->GetStr(ParamID(kPBAnimGlobalName)); }   

// Basic block
int     plPassMtl::GetColorLock() { return fBasicPB->GetInt(kPassBasColorLock); }
Color   plPassMtl::GetAmbColor() { return fBasicPB->GetColor(kPassBasColorAmb); }
Color   plPassMtl::GetColor() { return fBasicPB->GetColor(kPassBasColor); }
int     plPassMtl::GetOpacity() { return fBasicPB->GetInt(kPassBasOpacity); }
int     plPassMtl::GetEmissive() { return fBasicPB->GetInt(kPassBasEmissive); }
int     plPassMtl::GetUseSpec() { return fBasicPB->GetInt(kPassBasUseSpec); }
int     plPassMtl::GetShine() { return fBasicPB->GetInt(kPassBasShine); }
Color   plPassMtl::GetSpecularColor() { return fBasicPB->GetColor(kPassBasSpecColor); }
int     plPassMtl::GetDiffuseColorLock() { return fBasicPB->GetInt(kPassBasDiffuseLock); }
Color   plPassMtl::GetRuntimeColor() { return fBasicPB->GetColor(kPassBasRunColor); }
Control *plPassMtl::GetPreshadeColorController() { return GetParamBlock2Controller(fBasicPB, ParamID(kPassBasColor)); }
Control *plPassMtl::GetAmbColorController() { return GetParamBlock2Controller(fBasicPB, ParamID(kPassBasColorAmb)); }
Control *plPassMtl::GetOpacityController() { return GetParamBlock2Controller(fBasicPB, ParamID(kPassBasOpacity)); }
Control *plPassMtl::GetSpecularColorController() { return GetParamBlock2Controller(fBasicPB, ParamID(kPassBasSpecColor)); }
Control *plPassMtl::GetRuntimeColorController() { return GetParamBlock2Controller(fBasicPB, ParamID(kPassBasRunColor)); }

// Layer block
Texmap *plPassMtl::GetBaseLayer() { return fLayersPB->GetTexmap(kPassLayBase); }
int     plPassMtl::GetTopLayerOn() { return fLayersPB->GetInt(kPassLayTopOn); }
Texmap *plPassMtl::GetTopLayer() { return fLayersPB->GetTexmap(kPassLayTop); }
int     plPassMtl::GetLayerBlend() { return fLayersPB->GetInt(kPassLayBlend); }
int     plPassMtl::GetOutputAlpha() { return fLayersPB->GetInt(kPassLayOutputAlpha); }
int     plPassMtl::GetOutputBlend() { return fLayersPB->GetInt(kPassLayOutputBlend); }
