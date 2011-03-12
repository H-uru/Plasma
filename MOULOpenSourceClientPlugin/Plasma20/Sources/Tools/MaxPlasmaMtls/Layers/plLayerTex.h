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
#ifndef __PLMAXLAYER__H
#define __PLMAXLAYER__H

#include "Max.h"
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
	IParamBlock2	*fBitmapPB;
	IParamBlock2	*fBasicPB;
	UVGen			*fUVGen;

	IMtlParams		*fMtlParams;

	TexHandle *fTexHandle;
	TimeValue fTexTime;

	Bitmap *fBM;
	static ParamDlg *fUVGenDlg;
	Interval		fIValid;
   
	friend class BitmapDlgProc;

public:
	// Ref nums
	enum
	{
		kRefUVGen,
		kRefBasic, // DEAD, but left in for backwards compatability
		kRefBitmap,
	};

	// Block ID's
	enum
	{
		kBlkBasic, // DEAD
		kBlkBitmap,
	};

	plLayerTex();
	~plLayerTex();
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
	BOOL SupportTexDisplay() { return TRUE; }
	void ActivateTexDisplay(BOOL onoff);
	BITMAPINFO *GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, int forceW=0, int forceH=0);
	DWORD GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);
protected:
	void IChanged();
	void IDiscardTexHandle();

public:
	void GetUVTransform(Matrix3 &uvtrans) { fUVGen->GetUVTransform(uvtrans); }
	int GetTextureTiling() { return  fUVGen->GetTextureTiling(); }
	int GetUVWSource() { return fUVGen->GetUVWSource(); }
	virtual int GetMapChannel () { return fUVGen->GetMapChannel(); }	// only relevant if above returns UVWSRC_EXPLICIT
	UVGen *GetTheUVGen() { return fUVGen; }
	
	//TODO: Return anim index to reference index
	int SubNumToRefNum(int subNum) { return subNum; }
	
	virtual BOOL	DiscardColor();
	virtual BOOL	DiscardAlpha();
	
	// Loading/Saving
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	//From Animatable
	Class_ID ClassID() { return LAYER_TEX_CLASS_ID; }		
	SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
	void GetClassName(TSTR& s);

	RefTargetHandle Clone( RemapDir &remap );
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message);

	int NumSubs();
	Animatable* SubAnim(int i); 
	TSTR SubAnimName(int i);

	// TODO: Maintain the number or references here 
	int NumRefs();
	RefTargetHandle GetReference(int i);
	void SetReference(int i, RefTargetHandle rtarg);
	
	int	NumParamBlocks();	// return number of ParamBlocks in this instance
	IParamBlock2* GetParamBlock(int i); // return i'th ParamBlock
	IParamBlock2* GetParamBlockByID(BlockID id); // return id'd ParamBlock
	
	bool HasAlpha(); // Checks if the bitmap for this layer has an alpha channel
	virtual Bitmap* GetBitmap(TimeValue t);
		
	const char *GetTextureName();
	
	// Accessors needed by the base class for the various bitmap related elements
	virtual Bitmap *GetMaxBitmap(int index = 0) { return fBM; }
	virtual PBBitmap *GetPBBitmap( int index = 0 ); 
	virtual int		GetNumBitmaps( void ) { return 1; }

	// Virtual function called by plBMSampler to get various things while sampling the layer's image
	virtual bool	GetSamplerInfo( plBMSamplerData *samplerData );

	// Backdoor for the texture find and replace util.  Assumes input has the correct aspect ratio and is power of 2.
	virtual void SetExportSize(int x, int y);
	
protected:
	virtual void ISetPBBitmap( PBBitmap *pbbm, int index = 0 ); 
	virtual void ISetMaxBitmap(Bitmap *bitmap, int index = 0) { fBM = bitmap; }

};

#endif // __PLMAXLAYER__H
