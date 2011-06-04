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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plPlates - Header file for the plates and plPlateManager				//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plPlates_h
#define _plPlates_h

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsColorRGBA.h"
#include "hsTemplates.h"
#include "hsUtils.h"
#include "hsMatrix44.h"



//// plPlate Class Definition ////////////////////////////////////////////////
//	plPlate is the actual plate object that represents one plate on the 
//	screen. It has a transform matrix (which includes position, scale and
//	rotation), a material, a depth value and a color that is applied to all
//	four corners. All plates are parallelograms. 

class plPlateManager;
class hsGMaterial;
class plMipmap;
class plBitmap;

class plPlate
{
	friend class plPlateManager;

	protected:
		
		hsMatrix44		fXformMatrix;
		hsGMaterial		*fMaterial;
		plMipmap		*fMipmap;
		hsScalar		fDepth, fOpacity;
		UInt32			fFlags;
		char			fTitle[ 64 ];

		plPlate			*fNext;
		plPlate			**fPrevPtr;

		plPlate			**fOwningHandle;

		static UInt32	fMagicUniqueKeyInt;

		plPlate( plPlate** owningHandle );
		virtual ~plPlate();

		void	ILink( plPlate **back );

		void	IUnlink( void )
		{
			hsAssert( fPrevPtr, "Plate not in list" );
			if( fNext )
				fNext->fPrevPtr = fPrevPtr;
			*fPrevPtr = fNext;

			fNext = nil;
			fPrevPtr = nil;
		}

		void ISetResourceAlphas(UInt32 colorKey);

	public:

		enum 
		{
			kFlagVisible		= 0x00000001,
			kFlagLocalMaterial	= 0x00000002,
			kFlagIsAGraph		= 0x00000004
		};

		/// Basic properties

		void	SetTransform( hsMatrix44 &matrix, hsBool reSort = true );
		void	SetMaterial( hsGMaterial *material );
		void	SetTexture(plBitmap *texture); // Creates a new single layer material to use the texture.
		void	SetTitle( const char *title ) { if( title != nil ) strncpy( fTitle, title, sizeof( fTitle ) ); else fTitle[ 0 ] = 0; }

		hsGMaterial		*GetMaterial( void ) { return fMaterial; }
		hsMatrix44		&GetTransform( void ) { return fXformMatrix; }
		const char		*GetTitle( void ) { return fTitle; }
		UInt32			GetFlags( void ) { return fFlags; }

		void	SetVisible( hsBool vis ) { if( vis ) fFlags |= kFlagVisible; else fFlags &= ~kFlagVisible; }
		hsBool	IsVisible( void );

		void	SetOpacity( hsScalar opacity = 1.f );

		plPlate	*GetNext( void ) { return fNext; }


		/// Helper functions
		
		void	SetDepth( hsScalar depth) { fDepth = depth; }
		void	SetPosition( hsScalar x, hsScalar y, hsScalar z = -1.0f );
		void	SetSize( hsScalar width, hsScalar height, bool adjustByAspectRatio = false );

		plMipmap		*CreateMaterial( UInt32 width, UInt32 height, hsBool withAlpha, plMipmap* texture = NULL );
		void			CreateFromResource( const char *resName, UInt32 colorKey = 0x00ff00ff );
		void			ReloadFromResource( const char *resName, UInt32 colorKey = 0x00ff00ff );
		void			CreateFromJPEGResource( const char *resName, UInt32 colorKey = 0x00ff00ff );
		void			ReloadFromJPEGResource( const char *resName, UInt32 colorKey = 0x00ff00ff );
};

//// plGraphPlate Class Definition ///////////////////////////////////////////
//	A derivation of plPlate that maintains a procedural texture which displays
//	a scrolling graph of data.

class plGraphPlate : public plPlate
{
	protected:
		UInt32			fBGHexColor, fAxesHexColor, fGraphHexColor;
		std::vector<UInt32>	fDataHexColors;
		UInt32			fMin, fMax, fLabelMin, fLabelMax;
		std::vector<Int32>	fLastValues;
		std::vector<std::string>	fLabelText;

		UInt32		IMakePow2( UInt32 value );
		void		IDrawNumber( UInt32 number, UInt32 *dataPtr, UInt32 stride, UInt32 color );
		void		IDrawDigit( char digit, UInt32 *dataPtr, UInt32 stride, UInt32 color );

	public:
		plGraphPlate( plPlate **owningHandle );
		virtual ~plGraphPlate();

		void	SetDataRange( UInt32 min, UInt32 max, UInt32 width );
		void	SetDataLabels( UInt32 min, UInt32 max );
		void	SetLabelText( char *text1, char *text2 = nil, char *text3 = nil, char *text4 = nil );
		void	SetLabelText( const std::vector<std::string> & text );
		void	ClearData( void );

		void	AddData( Int32 value, Int32 value2 = -1, Int32 value3 = -1, Int32 value4 = -1 );
		void	AddData( std::vector<Int32> values );

		void	SetColors( UInt32 bgHexColor = 0x80000000, UInt32 axesHexColor = 0xffffffff, UInt32 dataHexColor = 0xff00ff00, UInt32 graphHexColor = 0x80ff0000 );
		void	SetDataColors( UInt32 hexColor1 = 0xff00ff00, UInt32 hexColor2 = 0xff0000ff, UInt32 hexColor3 = 0xffffff00, UInt32 hexColor4 = 0xffff00ff );
		void	SetDataColors( const std::vector<UInt32> & hexColors );

		const char		*GetLabelText( int i ) { return fLabelText[ i ].c_str(); }
		const UInt32	GetDataColor( int i ) { return fDataHexColors[ i ]; }
		const UInt32	GetNumLabels() { return fLabelText.size(); }
		const UInt32	GetNumColors() { return fDataHexColors.size(); }
};

//// plPlateManager Class Definition /////////////////////////////////////////
//	This class handles all the plates--it keeps track of all the plates, 
//	creates and destroys them, and draws them when the pipeline tells it to.

class plPipeline;

class plPlateManager
{
	friend class plPlate;

	private:

		static plPlateManager	*fInstance;

	protected:

		plPlate		*fPlates;
		plPipeline	*fOwner;
		hsBool		fCreatedSucessfully;

		plPlateManager( plPipeline *pipe ) 
		{
			fInstance = this;
			fPlates = nil;
			fOwner = pipe;
			fCreatedSucessfully = true;
		}

		virtual void	IDrawToDevice( plPipeline *pipe ) = 0;

		void			IResortPlate( plPlate *plate, bool fromCurrent );

	public:

		virtual ~plPlateManager();
		
		static plPlateManager	&Instance( void ) { return *fInstance; }
		static bool InstanceValid( void ) { return fInstance != nil; }

		void		CreatePlate( plPlate **handle );
		void		CreatePlate( plPlate **handle, hsScalar width, hsScalar height );
		void		CreatePlate( plPlate **handle, hsScalar x, hsScalar y, hsScalar width, hsScalar height );

		void		CreateGraphPlate( plGraphPlate **handle );

		void		DestroyPlate( plPlate *plate );

		void		SetPlateScreenPos( plPlate *plate, UInt32 x, UInt32 y );
		void		SetPlatePixelSize( plPlate *plate, UInt32 pWidth, UInt32 pHeight );

		UInt32		GetPipeWidth( void );
		UInt32		GetPipeHeight( void );
		void		DrawToDevice( plPipeline *pipe );

		hsBool		IsValid( void ) { return fCreatedSucessfully; }
};

// Sets the hInstance that we load our resources from.  A SceneViewer hack.
void SetHInstance(void *instance);

#endif //_plPlates_h

