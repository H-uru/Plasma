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
#ifndef PL_PARTICLEMTL_H
#define PL_PARTICLEMTL_H

#include "Max.h"
//#include "istdplug.h"
#include "iparamb2.h"
//#include "iparamm2.h"
#include "../resource.h"

class Bitmap;

#define PARTICLE_MTL_CLASS_ID Class_ID(0x26df05ff, 0x60660749)

extern TCHAR *GetString(int id);

class plParticleMtl : public Mtl
{
protected:
	IParamBlock2	*fBasicPB;
	Interval		fIValid;

public:
	IMtlParams *fIMtlParams;

	enum
	{
		kRefBasic,
	};
	enum
	{
		kBlkBasic,
	};

	enum // Param block indicies
	{
		kOpacity,
		kColor,
		kWidth,
		kHeight,
		kXTiles,
		kYTiles,
		kNormal,
		kBlend,
		kOrientation,
		kBitmap,
		kTexmap,
		kColorAmb,
		kNoFilter
	};
	enum
	{
		kBlendNone,
		kBlendAlpha,
		kBlendAdd
	};
	enum
	{
		kOrientVelocity,
		kOrientUp,
		kOrientVelStretch,
		kOrientVelFlow
	};

	enum
	{
		kNormalViewFacing,
		kNormalUp,
		kNormalNearestLight,
		kNormalFromCenter,
		kNormalVelUpVel,
		kEmissive,
		kNumNormalOptions
	};

	static const char *NormalStrings[];

	plParticleMtl(BOOL loading);
	void DeleteThis() { delete this; }

	//From Animatable
	Class_ID ClassID() { return PARTICLE_MTL_CLASS_ID; }		
	SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
	void GetClassName(TSTR& s) { s = GetString(IDS_PARTICLE_MTL); }

	ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);
	void Update(TimeValue t, Interval& valid);
	Interval Validity(TimeValue t);
	void Reset();

	void NotifyChanged();

	BOOL SupportsMultiMapsInViewport() { return FALSE; }
	void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb);

	// Shade and displacement calculation
	void Shade(ShadeContext& sc);
	void ShadeWithBackground(ShadeContext &sc, Color background);
	float EvalDisplacement(ShadeContext& sc); 
	Interval DisplacementValidity(TimeValue t); 	

	// SubTexmap access methods
	int NumSubTexmaps();
	Texmap* GetSubTexmap(int i);
	void SetSubTexmap(int i, Texmap *m);
	TSTR GetSubTexmapSlotName(int i);
	TSTR GetSubTexmapTVName(int i);
	
	BOOL SetDlgThing(ParamDlg* dlg);

	// Loading/Saving
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	RefTargetHandle Clone( RemapDir &remap );
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message);

	int NumSubs();
	Animatable* SubAnim(int i); 
	TSTR SubAnimName(int i);

	int NumRefs();
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);

	int	NumParamBlocks();
	IParamBlock2* GetParamBlock(int i);
	IParamBlock2* GetParamBlockByID(BlockID id);


//	void SetParamDlg(ParamDlg *dlg);

//	void SetNumSubTexmaps(int num);

	DllExport Control *GetAmbColorController();
	DllExport Control *GetColorController();
	DllExport Control *GetOpacityController();
	DllExport Control *GetWidthController();
	DllExport Control *GetHeightController();

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

	ULONG	Requirements( int subMtlNum );
};

#endif //PL_PARTICLEMTL_H