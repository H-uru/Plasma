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
//	plPlasmaMAXLayer - MAX Layer type that is the basis for all Plasma layer //
//					   types												 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	1.13.2002 mcn - Created.												 //
//																			 //
//// Notes from the Author ////////////////////////////////////////////////////
//																			 //
//	This base class is actually a quite-recent addition. As a result, most	 //
//	of the old, non-base-class-structured code is still lying around. This   //
//	code will be slowly converted over as time goes on; the theory was that  //
//	this conversion would be far more likely to occur if the base class		 //
//	actually already existed.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plPlasmaMAXLayer_h
#define _plPlasmaMAXLayer_h

#include "Max.h"
#include "hsTypes.h"

//// Derived Type Class IDs ///////////////////////////////////////////////////
//	If you create a new Plasma layer type, add a define for the class ID here 
//	and also add the ID to the list in plPlasmaMAXLayer.cpp.

const Class_ID LAYER_TEX_CLASS_ID( 0x4223c620, 0x183c4868 );
const Class_ID STATIC_ENV_LAYER_CLASS_ID( 0x379a0a20, 0x3d0b1244 );
const Class_ID DYNAMIC_ENV_LAYER_CLASS_ID( 0x18205c0f, 0x57ea0e10 );
const Class_ID DYN_TEXT_LAYER_CLASS_ID( 0x36e3480f, 0x120120bd );
const Class_ID ANGLE_ATTEN_LAYER_CLASS_ID( 0x6d90918, 0x6160114 );
const Class_ID MAX_CAMERA_LAYER_CLASS_ID( 0xfaf5ec7, 0x13d90d3f );

//// Class Definition /////////////////////////////////////////////////////////

class plLayerInterface;
class plMaxNode;
class plErrorMsg;
class plLayer;
class plDynamicTextMap;
class plBitmapData;
class plLocation;
class plLayerConverter;
class plLayerInterface;
class plBMSamplerData;
class jvUniqueId;
class plLayerTargetContainer;
class plPlasmaMAXLayer : public Texmap
{
	friend class plLayerConverter;

	protected:

		static const Class_ID	fDerivedTypes[];

		plLayerTargetContainer	*fConversionTargets;


		void	IAddConversionTarget( plLayerInterface *target );
		void	IClearConversionTargets( void );

	public:

		plPlasmaMAXLayer();
		virtual ~plPlasmaMAXLayer();

		void	DeleteThis() { delete this; }		


		// Static that checks the classID of the given texMap and, if it's a valid Plasma MAX Layer, returns a pointer to such
		static plPlasmaMAXLayer		*GetPlasmaMAXLayer( Texmap *map );

		// Some layers must be unique for each node they're applied to (i.e. can't be shared among nodes).
		// This returns true if the layer must be unique.
		virtual bool MustBeUnique( void ) { return false; }

		// These let the material make an informed decision on what to do with
		// the color and alpha values coming out of an EvalColor call. Something
		// like an InvertColor can be handled within EvalColor, but there needs
		// to be a way to tell the caller that the color returned should be completely
		// ignored.
		virtual BOOL	DiscardColor() { return false; }
		virtual BOOL	DiscardAlpha() { return false; }


		// Return the number of conversion targets (only valid after the MakeMesh pass)
		int		GetNumConversionTargets( void );

		// Get an indexed conversion target
		plLayerInterface	*GetConversionTarget( int index );
		
		virtual BOOL HandleBitmapSelection(int index = 0);
		virtual void SetBitmap(BitmapInfo *bi, int index = 0);
		virtual void SetBitmapAssetId(jvUniqueId& assetId, int index = 0);
		virtual void GetBitmapAssetId(jvUniqueId& assetId, int index = 0);

		// Pure virtual accessors for the various bitmap related elements
		virtual Bitmap *GetMaxBitmap(int index = 0) = 0;
		virtual PBBitmap *GetPBBitmap( int index = 0 ) = 0;
		virtual int		GetNumBitmaps( void ) = 0;

		// Makes sure the textures are the latest versions (including getting
		// the latest version from AssetMan)
		void RefreshBitmaps();

		hsBool	GetBitmapFileName( char *destFilename, int maxLength, int index = 0 );

		// Virtual function called by plBMSampler to get various things while sampling the layer's image
		virtual bool	GetSamplerInfo( plBMSamplerData *samplerData ) { return false; }

		// Backdoor for the texture find and replace util.  Assumes input has the correct aspect ratio and is power of 2.
		virtual void SetExportSize(int x, int y) {}
		
	protected:
		virtual void ISetMaxBitmap(Bitmap *bitmap, int index = 0) = 0;
		virtual void ISetPBBitmap( PBBitmap *pbbm, int index = 0 ) = 0;

};

#endif // _plPlasmaMAXLayer_h
