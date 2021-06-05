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
#ifndef plMAXCameraLayer_inc
#define plMAXCameraLayer_inc

#include "plPlasmaMAXLayer.h"

class ClassDesc2;
class IParamBlock2;

ClassDesc2* GetMAXCameraLayerDesc();

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

class plMAXCameraLayer : public plPlasmaMAXLayer
{
protected:
    // Parameter block
    IParamBlock2*   fParmsPB;
    Interval        fIValid;

public:
    // Ref nums
    enum
    {
        kRefMain
    };

    // Block ID's
    enum
    {
        kBlkMain
    };

    plMAXCameraLayer();
    ~plMAXCameraLayer();
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
    BOOL SupportTexDisplay() override { return FALSE; }
    void ActivateTexDisplay(BOOL onoff) override;
    BITMAPINFO *GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, int forceW=0, int forceH=0) override;
    DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) override;

protected:
    void    ICacheCosines();
    void    IChanged();
    void    IDiscardTexHandle();

public:

    int SubNumToRefNum(int subNum) override { return subNum; }
    BOOL    DiscardColor() override { return true; }

    // Loading/Saving
    IOResult Load(ILoad *iload) override;
    IOResult Save(ISave *isave) override;

    //From Animatable
    Class_ID ClassID() override { return MAX_CAMERA_LAYER_CLASS_ID; }
    SClass_ID SuperClassID() override { return TEXMAP_CLASS_ID; }
    void GetClassName(TSTR& s) override;

    RefTargetHandle Clone(RemapDir &remap) override;
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
        PartID& partID, RefMessage message) override;

    int NumSubs() override;
    Animatable* SubAnim(int i) override;
    MSTR SubAnimName(int i) override;

    int NumRefs() override;
    RefTargetHandle GetReference(int i) override;
    void SetReference(int i, RefTargetHandle rtarg) override;

    int NumParamBlocks() override;   // return number of ParamBlocks in this instance
    IParamBlock2* GetParamBlock(int i) override; // return i'th ParamBlock
    IParamBlock2* GetParamBlockByID(BlockID id) override; // return id'd ParamBlock

    const char *GetTextureName( int which );

    // ParamBlock accessors
    enum
    {
        kCamera,
        kUVSource,
        kExplicitCam,
        kRootNode,
        kDisableColor,
        kForce,
    };

    // Pure virtual accessors for the various bitmap related elements
    Bitmap *GetMaxBitmap(int index = 0) override { hsAssert(false, "Function call not valid on this type of layer."); return nullptr; }
    PBBitmap *GetPBBitmap(int index = 0) override { hsAssert(false, "Function call not valid on this type of layer."); return nullptr; }
    int     GetNumBitmaps() override { return 0; }

protected:
    void ISetMaxBitmap(Bitmap *bitmap, int index = 0) override { hsAssert(false, "Function call not valid on this type of layer."); }
    void ISetPBBitmap(PBBitmap *pbbm, int index = 0) override { hsAssert(false, "Function call not valid on this type of layer."); }
};

#endif // plMAXCameraLayer_inc
