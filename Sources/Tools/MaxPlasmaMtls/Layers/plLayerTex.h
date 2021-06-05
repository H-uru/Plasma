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
#ifndef __PLMAXLAYER__H
#define __PLMAXLAYER__H

#include "plPlasmaMAXLayer.h"

class ClassDesc2;
class IParamBlock2;
class Bitmap;

ClassDesc2* GetLayerTexDesc();

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

class plLayerTex : public plPlasmaMAXLayer
{
protected:
    // Parameter block
    IParamBlock2    *fBitmapPB;
    UVGen           *fUVGen;

    IMtlParams      *fMtlParams;

    TexHandle *fTexHandle;
    TimeValue fTexTime;

    Bitmap *fBM;
    static ParamDlg *fUVGenDlg;
    Interval        fIValid;
   
    friend class BitmapDlgProc;

public:
    // Ref nums
    enum
    {
        kRefUVGen,
        kRefBasic, // DEAD, but left in so we don't die.
        kRefBitmap,
    };

    // Block ID's
    enum
    {
        kBlkBasic,
        kBlkBitmap,
    };

    plLayerTex();
    ~plLayerTex();
    void DeleteThis() override { delete this; }

    //From MtlBase
    ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp) override;
    BOOL SetDlgThing(ParamDlg* dlg) override;
    void Update(TimeValue t, Interval& valid) override;
    void Reset() override;
    Interval Validity(TimeValue t) override;
    ULONG LocalRequirements(int subMtlNum) override;

    //From Texmap
    RGBA EvalColor(ShadeContext& sc) override;
    float EvalMono(ShadeContext& sc) override;
    Point3 EvalNormalPerturb(ShadeContext& sc) override;

    // For displaying textures in the viewport
    BOOL SupportTexDisplay() override { return TRUE; }
    void ActivateTexDisplay(BOOL onoff) override;
    BITMAPINFO *GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, int forceW=0, int forceH=0) override;
    DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override;
protected:
    void IChanged();
    void IDiscardTexHandle();

public:
    void GetUVTransform(Matrix3 &uvtrans) override { fUVGen->GetUVTransform(uvtrans); }
    int GetTextureTiling() override { return  fUVGen->GetTextureTiling(); }
    int GetUVWSource() override { return fUVGen->GetUVWSource(); }
    int GetMapChannel() override { return fUVGen->GetMapChannel(); }    // only relevant if above returns UVWSRC_EXPLICIT
    UVGen *GetTheUVGen() override { return fUVGen; }
    
    //TODO: Return anim index to reference index
    int SubNumToRefNum(int subNum) override { return subNum; }
    
    BOOL    DiscardColor() override;
    BOOL    DiscardAlpha() override;
    
    // Loading/Saving
    IOResult Load(ILoad *iload) override;
    IOResult Save(ISave *isave) override;

    //From Animatable
    Class_ID ClassID() override { return LAYER_TEX_CLASS_ID; }
    SClass_ID SuperClassID() override { return TEXMAP_CLASS_ID; }
    void GetClassName(TSTR& s) override;

    RefTargetHandle Clone(RemapDir &remap) override;
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
        PartID& partID, RefMessage message) override;

    int NumSubs() override;
    Animatable* SubAnim(int i) override;
    MSTR SubAnimName(int i) override;

    // TODO: Maintain the number or references here 
    int NumRefs() override;
    RefTargetHandle GetReference(int i) override;
    void SetReference(int i, RefTargetHandle rtarg) override;
    
    int NumParamBlocks() override;   // return number of ParamBlocks in this instance
    IParamBlock2* GetParamBlock(int i) override; // return i'th ParamBlock
    IParamBlock2* GetParamBlockByID(BlockID id) override; // return id'd ParamBlock
    
    bool HasAlpha(); // Checks if the bitmap for this layer has an alpha channel
    virtual Bitmap* GetBitmap(TimeValue t);
        
    const MCHAR* GetTextureName();
    
    // Accessors needed by the base class for the various bitmap related elements
    Bitmap *GetMaxBitmap(int index = 0) override { return fBM; }
    PBBitmap *GetPBBitmap(int index = 0) override;
    int     GetNumBitmaps() override { return 1; }

    // Virtual function called by plBMSampler to get various things while sampling the layer's image
    bool    GetSamplerInfo(plBMSamplerData *samplerData) override;

    // Backdoor for the texture find and replace util.  Assumes input has the correct aspect ratio and is power of 2.
    void SetExportSize(int x, int y) override;
    
protected:
    void ISetPBBitmap(PBBitmap *pbbm, int index = 0) override;
    void ISetMaxBitmap(Bitmap *bitmap, int index = 0) override { fBM = bitmap; }

};

#endif // __PLMAXLAYER__H
