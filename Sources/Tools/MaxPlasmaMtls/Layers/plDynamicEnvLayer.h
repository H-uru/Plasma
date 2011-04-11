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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plDynamicEnvLayer - Dynamic EnvironmentMap MAX Layer					 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.22.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDynamicEnvLayer_h
#define _plDynamicEnvLayer_h

#include "Max.h"
#include "plPlasmaMAXLayer.h"

class ClassDesc2;
class IParamBlock2;

//// Class Definition /////////////////////////////////////////////////////////

class plDynamicEnvLayer : public plPlasmaMAXLayer
{
protected:
	// Parameter block
	IParamBlock2	*fBitmapPB;
	UVGen			*fUVGen;

	IMtlParams		*fIMtlParams;

	TexHandle *fTexHandle;
	TimeValue fTexTime;

	Interval		fIValid;
   
	friend class DELBitmapDlgProc;

public:
	// Ref nums
	enum
	{
		kRefUVGen,
		kRefBitmap,
	};

	// Block ID's
	enum
	{
		kBlkBitmap,
	};

	plDynamicEnvLayer();
	~plDynamicEnvLayer();
	void DeleteThis() { delete this; }		

	//From MtlBase
	ParamDlg	*CreateParamDlg( HWND hwMtlEdit, IMtlParams *imp );
	BOOL		SetDlgThing( ParamDlg* dlg );
	void		Update( TimeValue t, Interval& valid );
	void		Reset();
	Interval	Validity( TimeValue t );
	ULONG		LocalRequirements( int subMtlNum );

	//From Texmap 
	RGBA		EvalColor( ShadeContext& sc );
	float		EvalMono( ShadeContext& sc );
	Point3		EvalNormalPerturb( ShadeContext& sc );

	// For displaying textures in the viewport
	BOOL		SupportTexDisplay() { return TRUE; }
	void		ActivateTexDisplay(BOOL onoff);
	BITMAPINFO	*GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, int forceW=0, int forceH=0);
	DWORD		GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker);

	virtual bool	MustBeUnique( void );

protected:
	void IDiscardTexHandle();

public:
	void		GetUVTransform(Matrix3 &uvtrans) { fUVGen->GetUVTransform(uvtrans); }
	int			GetTextureTiling() { return  fUVGen->GetTextureTiling(); }
	int			GetUVWSource() { return fUVGen->GetUVWSource(); }
	virtual int GetMapChannel() { return fUVGen->GetMapChannel(); }	// only relevant if above returns UVWSRC_EXPLICIT
	UVGen		*GetTheUVGen() { return fUVGen; }
	
	//TODO: Return anim index to reference index
	int SubNumToRefNum( int subNum ) { return kRefBitmap;	/* Only one sub*/  }
	
	// Loading/Saving
	IOResult Load(ILoad *iload);
	IOResult Save(ISave *isave);

	//From Animatable
	Class_ID	ClassID() { return DYNAMIC_ENV_LAYER_CLASS_ID; }		
	SClass_ID	SuperClassID() { return TEXMAP_CLASS_ID; }
	void		GetClassName( TSTR& s );

	RefTargetHandle	Clone( RemapDir &remap );
	RefResult		NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message );

	int				NumSubs();
	Animatable		*SubAnim(int i); 
	TSTR			SubAnimName(int i);

	int				NumRefs();
	RefTargetHandle	GetReference( int i );
	void			SetReference( int i, RefTargetHandle rtarg );

	int				NumParamBlocks();	// return number of ParamBlocks in this instance
	IParamBlock2	*GetParamBlock(int i); // return i'th ParamBlock
	IParamBlock2	*GetParamBlockByID(BlockID id); // return id'd ParamBlock

	/// ParamBlock accessors
	enum
	{
		kScalingAny,
		kScalingHalf,
		kScalingNone
	};

	// Param ID's
	enum
	{
		// General params
		kBmpTextureSize,
		kBmpAnchorNode,
		kBmpLastTextureSize,		// Annoying, but necessary to clamp texture sizes to powers of 2
		kBmpRefract
	};

		// Pure virtual accessors for the various bitmap related elements
		virtual Bitmap *GetMaxBitmap(int index = 0) { hsAssert( false, "Function call not valid on this type of layer." ); return nil; }
		virtual PBBitmap *GetPBBitmap( int index = 0 ) { hsAssert( false, "Function call not valid on this type of layer." ); return nil; }
		virtual int		GetNumBitmaps( void ) { return 0; }

	protected:
		virtual void ISetMaxBitmap(Bitmap *bitmap, int index = 0) { hsAssert( false, "Function call not valid on this type of layer." ); }
		virtual void ISetPBBitmap( PBBitmap *pbbm, int index = 0 ){ hsAssert( false, "Function call not valid on this type of layer." ); }
};

#endif // _plDynamicEnvLayer_h
