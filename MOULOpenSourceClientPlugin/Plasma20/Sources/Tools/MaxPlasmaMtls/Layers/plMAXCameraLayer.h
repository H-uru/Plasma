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
#ifndef plMAXCameraLayer_inc
#define plMAXCameraLayer_inc

#include "Max.h"
#include "../resource.h"
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
	IParamBlock2*	fParmsPB;
	Interval		fIValid;

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
	void DeleteThis() { delete this; }

	//From MtlBase
	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
	BOOL SetDlgThing(ParamDlg* dlg);
	void Update(TimeValue t, Interval& valid);
	void Reset();
	Interval Validity(TimeValue t);
	ULONG LocalRequirements(int subMtlNum);

	//From Texmap
	RGBA EvalColor(ShadeContext& sc);
	float EvalMono(ShadeContext& sc);
	Point3 EvalNormalPerturb(ShadeContext& sc);

	// For displaying textures in the viewport
	BOOL SupportTexDisplay() { return FALSE; }
	void ActivateTexDisplay(BOOL onoff);
	BITMAPINFO *GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, int forceW=0, int forceH=0);
	DWORD GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);

protected:
	void	ICacheCosines();
	void	IChanged();
	void	IDiscardTexHandle();

public:

	int SubNumToRefNum(int subNum) { return subNum; }
	virtual BOOL	DiscardColor() { return true; }

	// Loading/Saving
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	//From Animatable
	Class_ID ClassID() { return MAX_CAMERA_LAYER_CLASS_ID; }
	SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
	void GetClassName(TSTR& s) { s = GetString(IDS_MAX_CAMERA_LAYER); }

	RefTargetHandle Clone( RemapDir &remap );
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
		PartID& partID,  RefMessage message);

	int NumSubs();
	Animatable* SubAnim(int i);
	TSTR SubAnimName(int i);

	int NumRefs();
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	int	NumParamBlocks();	// return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i); // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id); // return id'd ParamBlock

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

	static const char *kUVStrings[];
	static const UInt8 kMaxUVSrc;

	// Pure virtual accessors for the various bitmap related elements
	virtual Bitmap *GetMaxBitmap(int index = 0) { hsAssert(false, "Function call not valid on this type of layer."); return nil; }
	virtual PBBitmap *GetPBBitmap(int index = 0) { hsAssert(false, "Function call not valid on this type of layer."); return nil; }
	virtual int		GetNumBitmaps(void) { return 0; }

protected:
	virtual void ISetMaxBitmap(Bitmap *bitmap, int index = 0) { hsAssert(false, "Function call not valid on this type of layer."); }
	virtual void ISetPBBitmap(PBBitmap *pbbm, int index = 0) { hsAssert(false, "Function call not valid on this type of layer."); }
};

#endif // plMAXCameraLayer_inc
