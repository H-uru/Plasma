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
//	plDynamicTextMap Class Header											 //
//	I derived from plMipmap not because I inherit a lot of the functionality,//
//	but because this acts very very much like a mipmap with one mip level.	 //
//	The only difference is that the actual data gets generated on the fly,	 //
//	instead of converted at export time. However, to the outside world,		 //
//	we're just a plain old mipmap, and I'd like to keep it that way.		 //
//																			 //
//	Note that we are also of a fixed format--ARGB8888. Keeps things nice and //
//	simple that way.														 //
//																			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	1.14.2002 mcn - Created.												 //
//	10.28.2002 mcn - Changing the arrangement a bit. Now we have a single	 //
//					 writable "creation" surface that actually generates	 //
//					 data, which then copies out on a flush to the actual	 //
//					 mipmap data. This is slower because it requires two	 //
//					 copies (second one when we prepare to write to a new	 //
//					 surface, unless kDiscard is specified as a flush option)//
//					 but it lets us allocate only one OS surface, which saves//
//					 us on Win98/ME where we're very limited in terms of	 //
//					 available OS surfaces.									 //
//					 To facilitate this, we create a new abstract class to	 //
//					 encapsulate the actual GDI functionality of Windows.	 //
//					 This way, we have two options when creating DTMaps:	 //
//					 allocate an OS writer per surface, which lets us avoid	 //
//					 the copy problem mentioned above, or allocate one to	 //
//					 share. This will also let us optimize by switching to	 //
//					 non-shared writers once we write a non-OS-reliant		 //
//					 writer/text renderer.								     //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDynamicTextMap_h
#define _plDynamicTextMap_h

#include "plMipmap.h"
#include "hsColorRGBA.h"

struct hsMatrix44;

//// Class Definition /////////////////////////////////////////////////////////

class plFont;
class plDynamicTextMap : public plMipmap
{
	protected:

		UInt16		fVisWidth, fVisHeight;

		virtual UInt32 	Read( hsStream *s );
		virtual UInt32 	Write( hsStream *s );

	public:
		//// Public Flags ////

		enum Justify
		{
			kLeftJustify = 0,
			kCenter,
			kRightJustify
		};

		//// Public Data /////
		
		
		//// Public Members ////


		plDynamicTextMap();
		plDynamicTextMap( UInt32 width, UInt32 height, hsBool hasAlpha = false, UInt32 extraWidth = 0, UInt32 extraHeight = 0 );
		virtual ~plDynamicTextMap();

		CLASSNAME_REGISTER( plDynamicTextMap );
		GETINTERFACE_ANY( plDynamicTextMap, plMipmap );


		void			Create( UInt32 width, UInt32 height, hsBool hasAlpha, UInt32 extraWidth = 0, UInt32 extraHeight = 0 );
		void			SetNoCreate( UInt32 width, UInt32 height, hsBool hasAlpha );

		virtual void	Reset( void );

		virtual void	Read( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Read( s, mgr ); this->Read( s ); }
		virtual void	Write( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Write( s, mgr ); this->Write( s ); }

		virtual UInt8	GetNumLevels( void ) const { return 1; }

		virtual void		Colorize( void ) { ; }
		virtual plMipmap	*Clone( void );
		virtual void		CopyFrom( plMipmap *source );


		/// Operations to perform on the text block

		hsBool	IsValid() { return IIsValid(); }

		// allow the user of the DynaTextMap that they are done with the image... for now
		// ... the fImage will be re-created on the next operation that requires the image
		void	PurgeImage();

		void	ClearToColor( hsColorRGBA &color );

		enum FontFlags
		{
			kFontBold		= 0x01,
			kFontItalic		= 0x02,
			kFontShadowed	= 0x04
		};

		void	SetFont( const char *face, UInt16 size, UInt8 fontFlags = 0, hsBool antiAliasRGB = true );
		void	SetFont( const wchar_t *face, UInt16 size, UInt8 fontFlags = 0, hsBool antiAliasRGB = true );
		void	SetLineSpacing( Int16 spacing );
		void	SetTextColor( hsColorRGBA &color, hsBool blockRGB = false );
		void	SetJustify( Justify j );

		void	DrawString( UInt16 x, UInt16 y, const char *text );
		void	DrawString( UInt16 x, UInt16 y, const wchar_t *text );
		void	DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 width, UInt16 height );
		void	DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 width, UInt16 height );
		void	DrawClippedString( Int16 x, Int16 y, const char *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height );
		void	DrawClippedString( Int16 x, Int16 y, const wchar_t *text, UInt16 clipX, UInt16 clipY, UInt16 width, UInt16 height );
		void	DrawWrappedString( UInt16 x, UInt16 y, const char *text, UInt16 width, UInt16 height, UInt16 *lastX = nil, UInt16 *lastY = nil );
		void	DrawWrappedString( UInt16 x, UInt16 y, const wchar_t *text, UInt16 width, UInt16 height, UInt16 *lastX = nil, UInt16 *lastY = nil );
		UInt16	CalcStringWidth( const char *text, UInt16 *height = nil );
		UInt16	CalcStringWidth( const wchar_t *text, UInt16 *height = nil );
		void	CalcWrappedStringSize( const char *text, UInt16 *width, UInt16 *height, UInt32 *firstClippedChar = nil, UInt16 *maxAscent = nil, UInt16 *lastX = nil, UInt16 *lastY = nil );
		void	CalcWrappedStringSize( const wchar_t *text, UInt16 *width, UInt16 *height, UInt32 *firstClippedChar = nil, UInt16 *maxAscent = nil, UInt16 *lastX = nil, UInt16 *lastY = nil );
		void	FillRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color );
		void	FrameRect( UInt16 x, UInt16 y, UInt16 width, UInt16 height, hsColorRGBA &color );
		void	SetFirstLineIndent( Int16 indent );

		enum DrawMethods
		{
			kImgNoAlpha,		// Just copy color data, force alpha to full if present
			kImgBlend,			// Blend color onto dest using src alpha, keep dest alpha
			kImgSprite			// Copy color data and alphas
		};
		void	DrawImage( UInt16 x, UInt16 y, plMipmap *image, DrawMethods method = kImgNoAlpha );
		void	DrawClippedImage( UInt16 x, UInt16 y, plMipmap *image, UInt16 srcClipX, UInt16 srcClipY, 
								UInt16 srcClipWidth, UInt16 srcClipHeight, DrawMethods method = kImgNoAlpha );

		void	FlushToHost( void );

		hsBool	MsgReceive( plMessage *msg );

		UInt16	GetVisibleWidth( void ) { return fVisWidth; }
		UInt16	GetVisibleHeight( void ) { return fVisHeight; }

		// Since the dynamic text can actually create a texture bigger than you were expecting,
		// you want to be able to apply a layer texture transform that will compensate. This
		// function will give you that transform. Just feed it into plLayer->SetTransform().

		hsMatrix44	GetLayerTransform( void );

		void	SetInitBuffer( UInt32 *buffer );

		// Gets for font values
		Justify		GetFontJustify( void ) const { return fJustify; }
		const char	*GetFontFace( void ) const { return fFontFace; }
		UInt16		GetFontSize( void ) const { return fFontSize; }
		hsBool		GetFontAARGB( void ) const { return fFontAntiAliasRGB; }
		hsColorRGBA	GetFontColor( void ) const { return fFontColor; }
		hsBool		GetFontBlockRGB( void ) const { return fFontBlockRGB; }
		Int16		GetLineSpacing( void ) const { return fLineSpacing; }

		plFont		*GetCurrFont( void ) const { return fCurrFont; }

		virtual void	Swap( plDynamicTextMap *other );

	protected:

		//// Protected Members ////

		hsBool		IIsValid( void );
		void		IClearFromBuffer( UInt32 *clearBuffer );

		UInt32		*IAllocateOSSurface( UInt16 width, UInt16 height );
		void		IDestroyOSSurface( void );

		hsBool		fHasAlpha, fShadowed;

		Justify		fJustify;
		char		*fFontFace;
		UInt16		fFontSize;
		UInt8		fFontFlags;
		hsBool		fFontAntiAliasRGB;
		hsColorRGBA	fFontColor;
		hsBool		fFontBlockRGB;
		Int16		fLineSpacing;

		plFont		*fCurrFont;

		UInt32		*fInitBuffer;
		
		hsBool		fHasCreateBeenCalled;
};


#endif // _plDynamicTextMap_h
