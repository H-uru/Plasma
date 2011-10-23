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
#ifndef PL_DECALMTL_H
#define PL_DECALMTL_H

#include "Max.h"
#include "iparamb2.h"

#include "../resource.h"
#include "plPassMtlBase.h"

#define DECAL_MTL_CLASS_ID  Class_ID(0x691d2257, 0x419d629e)

extern TCHAR *GetString(int id);

class plDecalMtl : public plPassMtlBase
{
protected:

    virtual void        ICloneRefs( plPassMtlBase *target, RemapDir &remap );

public:

    enum
    {
        kRefBasic,
        kRefLayers,
        kRefAdv,
        kRefAnim,
    };
    
    enum Blocks
    {
        kBlkBasic,
        kBlkLayers,
        kBlkAdv,
        kBlkAnim,
    };

    plDecalMtl(BOOL loading);
    void DeleteThis() { delete this; }

    //From Animatable
    Class_ID ClassID() { return DECAL_MTL_CLASS_ID; }       
    SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
    void GetClassName(TSTR& s) { s = GetString(IDS_DECAL_MTL); }

    ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
    void Update(TimeValue t, Interval& valid);
    Interval Validity(TimeValue t);

    void NotifyChanged();

    BOOL SupportsMultiMapsInViewport() { return FALSE; }
    void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb);

    // Shade and displacement calculation
    void Shade(ShadeContext& sc);
    void ShadeWithBackground(ShadeContext &sc, Color background);
    float EvalDisplacement(ShadeContext& sc); 
    Interval DisplacementValidity(TimeValue t);     

    virtual RefTargetHandle GetReference( int i );
    virtual void            SetReference( int i, RefTargetHandle rtarg );

    // SubTexmap access methods
    int NumSubTexmaps();
    Texmap* GetSubTexmap(int i);
    void SetSubTexmap(int i, Texmap *m);
    TSTR GetSubTexmapSlotName(int i);
    TSTR GetSubTexmapTVName(int i);
   int SubTexmapOn(int i);
      
    BOOL SetDlgThing(ParamDlg* dlg);

    RefTargetHandle Clone( RemapDir &remap );
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
        PartID& partID,  RefMessage message);

    int NumSubs();
    Animatable* SubAnim(int i); 
    TSTR SubAnimName(int i);

    int NumParamBlocks();
    IParamBlock2* GetParamBlock(int i);
    IParamBlock2* GetParamBlockByID(BlockID id);


//  void SetParamDlg(ParamDlg *dlg);

//  void SetNumSubTexmaps(int num);

    // From MtlBase and Mtl
    void SetAmbient(Color c, TimeValue t);      
    void SetDiffuse(Color c, TimeValue t);      
    void SetSpecular(Color c, TimeValue t);
    void SetShininess(float v, TimeValue t);
    Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE);
    Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE);
    Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE);
    float GetXParency(int mtlNum=0, BOOL backFace=FALSE);
    float GetShininess(int mtlNum=0, BOOL backFace=FALSE);      
    float GetShinStr(int mtlNum=0, BOOL backFace=FALSE);
    float WireSize(int mtlNum=0, BOOL backFace=FALSE);

    ULONG   Requirements( int subMtlNum );

    virtual bool    HasAlpha();
    // Massive list of inherited accessor functions for ParamBlock data

    // Advanced Block
    virtual int     GetBasicWire();
    virtual int     GetMeshOutlines();
    virtual int     GetTwoSided();
    virtual int     GetSoftShadow();
    virtual int     GetNoProj();
    virtual int     GetVertexShade();
    virtual int     GetNoShade();
    virtual int     GetNoFog();
    virtual int     GetWhite();
    virtual int     GetZOnly();
    virtual int     GetZClear();
    virtual int     GetZNoRead();
    virtual int     GetZNoWrite();
    virtual int     GetZInc();
    virtual int     GetAlphaTestHigh();

    // Animation block
    virtual const char*  GetAnimName();
    virtual int     GetAutoStart();
    virtual int     GetLoop();
    virtual const char*  GetAnimLoopName();
    virtual int     GetEaseInType();
    virtual float   GetEaseInMinLength();
    virtual float   GetEaseInMaxLength();
    virtual float   GetEaseInNormLength();
    virtual int     GetEaseOutType();
    virtual float   GetEaseOutMinLength();
    virtual float   GetEaseOutMaxLength();
    virtual float   GetEaseOutNormLength();
    virtual int     GetUseGlobal();
    virtual const char*  GetGlobalVarName();

    // Basic block
    virtual int     GetColorLock();
    virtual Color   GetAmbColor();
    virtual Color   GetColor();
    virtual int     GetOpacity();
    virtual int     GetEmissive();
    virtual int     GetUseSpec();
    virtual int     GetShine();
    virtual Color   GetSpecularColor();
    virtual Control *GetPreshadeColorController();
    virtual Control *GetAmbColorController();
    virtual Control *GetOpacityController();
    virtual Control *GetSpecularColorController();
    virtual int     GetDiffuseColorLock();
    virtual Color   GetRuntimeColor();
    virtual Control *GetRuntimeColorController();
    
    // Layer block
    virtual Texmap *GetBaseLayer();
    virtual int     GetTopLayerOn();
    virtual Texmap *GetTopLayer();
    virtual int     GetLayerBlend();
    virtual int     GetOutputAlpha();
    virtual int     GetOutputBlend();
};

#endif //PL_DECALMTL_H