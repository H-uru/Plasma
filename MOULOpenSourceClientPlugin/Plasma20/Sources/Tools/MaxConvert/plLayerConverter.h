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
//	plLayerConverter - Utility class that converts plPlasmaMAXLayers into	 //
//					   other stuff.											 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	1.13.2002 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plLayerConverter_h
#define _plLayerConverter_h

#include "hsTypes.h"
#include "hsTemplates.h"
#include "Max.h"


//// Class Definition /////////////////////////////////////////////////////////

class plErrorMsg;
class plLayerInterface;
class plMaxNode;
class plPlasmaMAXLayer;
class plLayer;
class plLocation;
class plBitmapData;
class plDynamicTextMap;
class plBitmap;
class plCubicRenderTarget;
class hsConverterUtils;
class plDynamicTextLayer;

class plLayerConverter
{
	private:

		plLayerConverter();

	protected:

		static const Class_ID	fDerivedTypes[];

	public:

		~plLayerConverter();
		static plLayerConverter &Instance( void );

		void	Init( hsBool save, plErrorMsg *msg );
		void	DeInit( void );

		plLayerInterface	*ConvertTexmap( Texmap *texmap,	plMaxNode *maxNode,
											UInt32 blendFlags, hsBool preserveUVOffset, hsBool upperLayer );
		plBitmap *CreateSimpleTexture(const char *fileName, const plLocation &loc, UInt32 clipID = 0, UInt32 texFlags = 0, bool useJPEG = false);
		
		void	MuteWarnings( void );
		void	UnmuteWarnings( void );

	protected:

		plErrorMsg	*fErrorMsg;
		UInt32		fWarned, fSavedWarned;
		hsBool		fSaving;

		Interface           *fInterface;
		hsConverterUtils	&fConverterUtils;

		const char	*fDbgNodeName;

		hsTArray<plPlasmaMAXLayer *>	fConvertedLayers;


		plLayer				*ICreateLayer( const char *name, hsBool upperLayer, plLocation &loc );
		void				IProcessUVGen( plPlasmaMAXLayer *srcLayer, plLayer *destLayer, plBitmapData *bitmapData, hsBool preserveUVOffset );
		plDynamicTextMap	*ICreateDynTextMap( const char *layerName, UInt32 width, UInt32 height, hsBool includeAlpha, plMaxNode *node );

		plLayer				*IAssignTexture( plBitmapData *bd, plMaxNode *maxNode, plLayer *destLayer, hsBool upperLayer, int clipID = -1 );
		plCubicRenderTarget	*IMakeCubicRenderTarget( const char *name, plMaxNode *maxNode, plMaxNode *anchor );

		// Add your function to process your layer type here
		plLayerInterface	*IConvertLayerTex( plPlasmaMAXLayer *layer, plMaxNode *maxNode, UInt32 blendFlags, hsBool preserveUVOffset, hsBool upperLayer );
		plLayerInterface	*IConvertStaticEnvLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, UInt32 blendFlags, hsBool preserveUVOffset, hsBool upperLayer );
		plLayerInterface	*IConvertDynamicEnvLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, UInt32 blendFlags, hsBool preserveUVOffset, hsBool upperLayer );
		plLayerInterface	*IConvertCameraLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, UInt32 blendFlags, hsBool preserveUVOffset, hsBool upperLayer );
		plLayerInterface	*IConvertDynamicTextLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, UInt32 blendFlags, hsBool preserveUVOffset, hsBool upperLayer );

		plBitmap*			IGetAttenRamp( plMaxNode *maxNode, BOOL isAdd, int loClamp, int hiClamp);
		plLayer*			ICreateAttenuationLayer(const char* name, plMaxNode *maxNode, int uvwSrc, float tr0, float op0, float tr1, float op1, int loClamp, int hiClamp);
		plLayerInterface*	IConvertAngleAttenLayer(plPlasmaMAXLayer *layer, plMaxNode *maxNode, UInt32 blendFlags, hsBool preserveUVOffset, hsBool upperLayer);

		void				IRegisterConversion( plPlasmaMAXLayer *origLayer, plLayerInterface *convertedLayer );

		UInt32				*IGetInitBitmapBuffer( plDynamicTextLayer *layer ) const;

};

#endif // _plLayerConverter_h
