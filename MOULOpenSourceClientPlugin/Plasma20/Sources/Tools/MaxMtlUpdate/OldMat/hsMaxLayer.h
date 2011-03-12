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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef __HSMAXLAYER_H
#define __HSMAXLAYER_H

#include <bmmlib.h>
#include "hsMaxLayerBase.h"
//#include "hsMaxMtlRes.h"

//=========================================================================================
// Flags and constants...
//=========================================================================================

// ParamBlock entries
#define PB_LAYER_CLIPU          0
#define PB_LAYER_CLIPV          1
#define PB_LAYER_CLIPW          2
#define PB_LAYER_CLIPH          3
#define PB_LAYER_JITTER         4
#define PB_LAYER_AMBIENT        5
#define PB_LAYER_COLOR          6
#define PB_LAYER_SHININESS      7
#define PB_LAYER_SHIN_STR       8
#define PB_LAYER_SELFI          9
#define PB_LAYER_OPAC           10
#define PB_LAYER_OPFALL         11
#define PB_LAYER_FILTER         12
#define PB_LAYER_WIRESZ         13
#define PB_LAYER_IOR            14
#define PB_LAYER_BOUNCE         15
#define PB_LAYER_STATFRIC       16
#define PB_LAYER_SLIDFRIC       17
#define PB_LAYER_DIMLEV         18
#define PB_LAYER_DIMMULT        19
#define PB_LAYER_MAPPERCENT     20
#define PB_LAYER_MIPMAPBLUR     21
#define PB_LAYER_LODBIAS        22
#define PB_LAYER_DETAILBIAS     23
#define PB_LAYER_DETAILMAX      24
#define PB_LAYER_ENVIRONMAPSIZE 25
#define LAYER_NPARAMS 26

#define HSMAX_LAYER_LOCK_AD         0x1

class hsMaxLayer;
class hsMaxLayerDlg;

static LRESULT CALLBACK HiliteWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

// which edit control enum
enum EditControl {Hc, Sc, Vc, Rc, Gc, Bc};

void GetBMName(BitmapInfo& bi, TSTR &fname);

//=========================================================================================
// BMSampler
//=========================================================================================

class BMSampler : public MapSampler {
    Bitmap *bm;
    hsMaxLayer *tex;
    int alphaSource;
    float u0,v0,u1,v1,ufac,vfac,ujit,vjit;
    int bmw,bmh,clipx, clipy, cliph;
    float fclipw,fcliph, fbmh, fbmw;
public:
    BMSampler() { bm = NULL; }
    void Init(hsMaxLayer *bmt);
    int PlaceUV(ShadeContext& sc, float &u, float &v, int iu, int iv);
    void PlaceUVFilter(ShadeContext& sc, float &u, float &v, int iu, int iv);
    AColor Sample(ShadeContext& sc, float u,float v);
    AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv);
    //      float SampleMono(ShadeContext& sc, float u,float v);
    //      float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
};

//=========================================================================================
// BM\AlphaSampler
//=========================================================================================
class BMAlphaSampler : public MapSampler {
    Bitmap *bm;
    hsMaxLayer *tex;
    float u0,v0,u1,v1,ufac,vfac,ujit,vjit;
    int bmw,bmh,clipx, clipy, cliph;
    float fclipw,fcliph, fbmh, fbmw;
public:
    BMAlphaSampler() { bm = NULL; }
    void Init(hsMaxLayer *bmt);
    int PlaceUV(ShadeContext &sc, float &u, float &v, int iu, int iv);
    void  PlaceUVFilter(ShadeContext &sc, float &u, float &v, int iu, int iv);
    AColor Sample(ShadeContext& sc, float u,float v) { return AColor(0,0,0,0);}
    AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv) { return AColor(0,0,0,0);}
    float SampleMono(ShadeContext& sc, float u,float v);
    float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv);
};

//=========================================================================================
// hsMaxLayerNotify... this calls hsMaxLayer::NotifyChanged when a bitmap changes
//=========================================================================================

class hsMaxLayerNotify : public BitmapNotify {
public:
    void SetTex(hsMaxLayer *tx) { tex  = tx; }
    int Changed(ULONG flags);
    
private:
    hsMaxLayer *tex;
};

//=========================================================================================
// hsMaxLayer: a material layer with (possibly) texture info, blending info and shading info
//=========================================================================================

class hsMaxLayer : public hsMaxLayerBase {
    friend class hsMaxLayerPostLoad;
    friend class hsMaxLayerDlg;
    friend class BMSampler;
    friend class BMAlphaSampler;
    friend class BMCropper;
public:
    hsMaxLayer();
    ~hsMaxLayer();
    
    ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
    void Update(TimeValue t, Interval& valid);
    void Reset();
    Interval Validity(TimeValue t)              { Interval v; Update(t,v); return ivalid; }
    TSTR GetFullName();
    
    void SetOutputLevel(TimeValue t, float v)   { }
    void SetFilterType(int ft);
    void SetAlphaSource(int as);
    void SetEndCondition(int endcond)           { endCond = endcond; }
    void SetAlphaAsMono(BOOL onoff)             { alphaAsMono = onoff; }
    void SetAlphaAsRGB(BOOL onoff)              { alphaAsRGB = onoff; }
    void SetPremultAlpha(BOOL onoff)            { premultAlpha = onoff; }
    void SetMapName(TCHAR *name) { 
        bi.SetName(name); 
        FreeBitmap();
        if (paramDlg)
            ReloadBitmap();
    }
    void SetStartTime(TimeValue t) { startTime = t; }
    void SetPlaybackRate(float r) { pbRate = r; }
    
    void SetClipU(TimeValue t, float f) { clipu  = f; pblock->SetValue( PB_LAYER_CLIPU, t, f);      }
    void SetClipV(TimeValue t, float f) { clipv  = f; pblock->SetValue( PB_LAYER_CLIPV, t, f);      }
    void SetClipW(TimeValue t, float f) { clipw  = f; pblock->SetValue( PB_LAYER_CLIPW, t, f);      }
    void SetClipH(TimeValue t, float f) { cliph  = f; pblock->SetValue( PB_LAYER_CLIPH, t, f);      }
    void SetJitter(TimeValue t, float f) { cliph  = f; pblock->SetValue( PB_LAYER_JITTER, t, f);        }
    
    int GetFilterType() { return filterType; }
    int GetAlphaSource() { return alphaSource; }
    int GetEndCondition() { return endCond; }
    BOOL GetAlphaAsMono(BOOL onoff) { return alphaAsMono; }
    BOOL GetAlphaAsRGB(BOOL onoff) { return alphaAsRGB; }
    BOOL GetPremultAlpha(BOOL onoff) { return premultAlpha; }
    TCHAR *GetMapName() { return (TCHAR *)bi.Name(); }
    TimeValue GetStartTime() { return startTime; }
    float GetPlaybackRate() { return pbRate; }
    StdUVGen* GetUVGen() { return (StdUVGen*)uvGen; }
    TextureOutput* GetTexout() { return 0; }
    Bitmap *GetBitmap(TimeValue t) { LoadBitmap(t);     return thebm; }
    float GetClipU(TimeValue t) { return pblock->GetFloat( PB_LAYER_CLIPU, t); }
    float GetClipV(TimeValue t) { return pblock->GetFloat( PB_LAYER_CLIPV, t); }
    float GetClipW(TimeValue t) { return pblock->GetFloat( PB_LAYER_CLIPW, t); }
    float GetClipH(TimeValue t) { return pblock->GetFloat( PB_LAYER_CLIPH, t); }
    float GetJitter(TimeValue t) { return pblock->GetFloat( PB_LAYER_JITTER, t); }
    void StuffCropValues(); // stuff new values into the cropping VFB
    
    void UpdtSampler() {
        mysamp.Init(this);
        alphasamp.Init(this);
    }
    
    void NotifyChanged();
    Bitmap* BuildBitmap(int size);
    void FreeBitmap();
    BMMRES LoadBitmap(TimeValue t);
    int CalcFrame(TimeValue t);
    void ScaleBitmapBumpAmt(float f);
    void ReloadBitmap();
    
    // Evaluate the color of map for the context.
    RGBA EvalColor(ShadeContext& sc);
    float EvalMono(ShadeContext& sc);
    Point3 EvalNormalPerturb(ShadeContext& sc);
    
    void DiscardTexHandle();
    
    BOOL SupportTexDisplay() { return TRUE; }
    void ActivateTexDisplay(BOOL onoff);
    DWORD GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
    void GetUVTransform(Matrix3 &uvtrans) { uvGen->GetUVTransform(uvtrans); }
    int GetTextureTiling() { return  uvGen->GetTextureTiling(); }
    int GetUVWSource() { return uvGen->GetUVWSource(); }
    UVGen *GetTheUVGen() { return uvGen; }
#ifdef MAXR3
	int GetMapChannel () { return uvGen->GetMapChannel(); }
#endif // MAXR3
    
    int RenderBegin(TimeValue t, ULONG flags) { 
        inRender = TRUE;
        return 1;   
    }
    int RenderEnd(TimeValue t) {    
        inRender = FALSE;   
        return 1;   
    }
    int LoadMapFiles(TimeValue t) { LoadBitmap(t);  return 1;   }
    void RenderBitmap(TimeValue t, Bitmap *bm, float scale3D, BOOL filter);
    
    Class_ID ClassID() {    return hsMaxLayerClassID; }
    SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
#ifdef HS_DEBUGGING
    void GetClassName(TSTR& s) { s= GetString(IDS_DS_LAYER_DEBUG); }  
#else
    void GetClassName(TSTR& s) { s= "";}//GetString(IDS_DS_LAYER); }  
#endif
    void DeleteThis() { delete this; }  
    
    // Requirements
    ULONG LocalRequirements(int subMtlNum);
    
    int NumSubs() { return 2; }  
    Animatable* SubAnim(int i);
    TSTR SubAnimName(int i);
    int SubNumToRefNum(int subNum) { return subNum; }
    void InitSlotType(int sType) { if (uvGen) uvGen->InitSlotType(sType); }
    
    // From ref
    int NumRefs() { return 2; }
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);
    
    RefTargetHandle Clone(RemapDir &remap = NoRemap());
    RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
        PartID& partID, RefMessage message );
    
    // From Animatable
    void EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) {
        bi.EnumAuxFiles(nameEnum,flags);
    }
    int SetProperty(ULONG id, void *data);
    void FreeAllBitmaps() { 
        FreeBitmap(); 
    }
    
    int GetFlag(ULONG f) { return (flags&f)?1:0; }
    void SetFlag(ULONG f, ULONG val);

    // from hsMaxLayerBase        
    BOOL KeyAtTime(int id,TimeValue t) { return pblock->KeyFrameAtTime(id,t); }
    Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
    Color GetColor(int mtlNum=0, BOOL backFace=FALSE);

    float GetShininess(int mtlNum=0, BOOL backFace=FALSE) { return 0.f; }
    float GetShinStr(int mtlNum=0, BOOL backFace=FALSE) { return 0.f; }
    float GetMapPercent(int mtlNum=0, BOOL backFace=FALSE) { return 0.f; }
    float GetOpacity(int mtlNum=0, BOOL backFace=FALSE) { return 0.f; }
    float GetMipMapBlur(int mtlNum=0, BOOL backFace=FALSE) { return pblock->GetFloat(PB_LAYER_MIPMAPBLUR, 0); }
    float GetLODBias(int mtlNum=0, BOOL backFace=FALSE) { return pblock->GetFloat(PB_LAYER_LODBIAS, 0); }
    int GetEnvironMapSize(int mtlNum=0, BOOL backFace=FALSE) { return 0; }

    Color GetAmbient(TimeValue t) const;
    Color GetColor(TimeValue t) const;
    
    float GetShininess(TimeValue t) const;
    float GetShinStr(TimeValue t) const;
    float GetMapPercent(TimeValue t) const;
    float GetOpacity(TimeValue t) const;
    float GetMipMapBlur(TimeValue t) const;
    float GetLODBias(TimeValue t) const;
    float GetDetailDropoffStart(TimeValue t) const;
    float GetDetailDropoffStop(TimeValue t) const;
    float GetDetailMax(TimeValue t) const;
    float GetDetailMin(TimeValue t) const;
    int GetEnvironMapSize(TimeValue t) const;

    int GetNumExplicitMipmaps() const           { return mipmapInfo.Count(); }
    TCHAR *GetExplicitMipmapName(int i) const   { return mipmapOn[i] ? (TCHAR *)mipmapInfo[i].Name() : 0; }
    BOOL ExplicitMipmapEnabled(int i) const     { return mipmapOn[i]; }
    int GetExplicitMipmapLevel(int i) const     { return mipmapLevel[i]; }

    BOOL GetDirty() const       { return dirty; }
    ULONG GetBlendFlags() const { return blendFlags; }
    ULONG GetZFlags() const     { return zFlags; }
    ULONG GetShadeFlags() const { return shadeFlags; }
    ULONG GetMiscFlags() const  { return miscFlags; }
	ProcType GetProcType() const	{ return procType; }
	hsMatUsage GetUsage() const { return usageType; }

    // Setting the things in hsMaxLayerBase
    void SetShininess(float v, TimeValue t);        
    void SetShinStr(float v, TimeValue t);      
    void SetMapPercent(float v, TimeValue t);    
    void SetOpacity(float v, TimeValue t);  
    void SetAmbient(Color c, TimeValue t);      
    void SetColor(Color c, TimeValue t);      
    void SetMipMapBlur(float f, TimeValue t);
    void SetLODBias(float f, TimeValue t);
    void SetDetailDropoffStart(float f, TimeValue t);
    void SetDetailDropoffStop(float f, TimeValue t);
    void SetDetailMax(float f, TimeValue t);
    void SetDetailMin(float f, TimeValue t);
    void SetEnvironMapSize(int i, TimeValue t);

    void SetNumExplicitMipmaps(int n);
    void SetExplicitMipmapName(int i, const char *n)    { mipmapInfo[i].SetName(n); }
    void EnableExplicitMipmap(int i, BOOL state)        { mipmapOn[i] = state; }
    void SetExplicitMipmapLevel(int i, int l)           { mipmapLevel[i] = l; }

    void SetDirty(BOOL state)                           { dirty = state; }
    void SetBlendFlag(int i, BOOL state);
    void SetZFlag(int flag, BOOL state);
    void SetShadeFlag(int flag, BOOL state);
    void SetMiscFlag(int flag, BOOL state);
	void SetProcType(ProcType type);
	void SetUsage(hsMatUsage use);
	void GuessUsage();

    // IO
    IOResult Save(ISave *isave);
    IOResult Load(ILoad *iload);
    
	// Colin Hack
	BOOL GetApplyCrop() { return applyCrop; }
	BOOL GetPlaceImage() { return placeImage; }

private:
    UVGen *uvGen;          // ref #0
    IParamBlock *pblock;   // ref #1
    BitmapInfo bi;
    TSTR bmName; // for loading old files only
    Bitmap *thebm;
    hsMaxLayerNotify bmNotify;
    TexHandle *texHandle;
    float pbRate;
    TimeValue startTime;
    Interval ivalid;

    // Samplers
    BMSampler mysamp;
    BMAlphaSampler alphasamp;

    BOOL applyCrop;
    BOOL loadingOld;
    BOOL placeImage;
    BOOL randPlace;
    int filterType;
    int alphaSource;
    int endCond;
    int alphaAsMono;
    int alphaAsRGB;
    float clipu, clipv, clipw, cliph, jitter;
    BOOL premultAlpha;
    BOOL isNew;
    BOOL loadFailed; 
    BOOL inRender;
    hsMaxLayerDlg *paramDlg;
    int texTime;
    Interval texValid;
    Interval clipValid;
    float rumax,rumin,rvmax,rvmin;
    
    // ADDED
    ULONG flags;
    
    Color ambient;
    Color color;

    float opacity;     
    float shine_str; 
    float shininess;   
    float mapPercent;   
    float mipMapBlur;
    float lodBias;
    float detailDropoffStart;
    float detailDropoffStop;
    float detailMax;
    float detailMin;
    int environMapSize;

    BOOL dirty;
    
    ULONG blendFlags;
    ULONG zFlags;
    ULONG shadeFlags;
    ULONG miscFlags;
	ProcType procType;
	hsMatUsage usageType;

    Tab<BitmapInfo> mipmapInfo;        // references
    Tab<BOOL> mipmapOn;
    Tab<int> mipmapLevel;
};

#endif