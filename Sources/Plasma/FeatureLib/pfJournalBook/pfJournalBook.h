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
#ifndef _pfJournalBook_h
#define _pfJournalBook_h

//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfJournalBook Class 													//
//	A generic, high-level, abstract method of creating various Myst-like	//
//	books within the game with very little effort, while ensuring that they	//
//	all remain consistent in appearance and operability.					//
//																			//
//////////////////////////////////////////////////////////////////////////////
//																			//
//	Journal books are created via a journal book template. The template		//
//	takes the form of an extremely simplified version of HTML. The esHTML	//
//	has the following tags defined:											//
//		<p>		- Start of a new paragraph. </p> isn't used.				//
//		<img>	- Places an image in-line with the text. Options are:		//
//			align=left/right/center											//
//			src=<name of plMipmap>											//
//			link=<eventID> - Defines the image as clickable. When the user	//
//							 clicks the image, it will generate an event	//
//							 with the given event ID and send it to the		//
//							 calling Python handler.						//
//			blend=none/alpha - Controls the blending method of drawing		//
//							   the image									//
//			pos=<x>,<y>		- Positions the image absolutely on the page	//
//							  using the upper-left pixel given. Absolute-	//
//							  placed images have no effect on text flow.	//
//			glow=<seconds[,min,[max]]>										//
//							- Defines a glow special effect, which amounts	//
//							  to the image oscillating in opacity across	//
//							  the time interval specified. Optionally, you	//
//							  can also specify the min and max opacity,		//
//							  from 0 to 1.									//
//			check=<onColor,offColor,default>								//
//							- Makes the image act as a checkbox, flipping	//
//							  between on and off states (as defined by the	//
//							  blending colors) on clicks. Default is either //
//							  1 for	on or 0 for off. Link notifys are sent  //
//							  by checkboxes, with the event type			//
//							  "kNotifyCheckUnchecked" negative if the		//
//							  state is switching to off. Cannot be used		//
//							  with "glow".									//
//			resize=yes/no	- Defines whether the image will be resized		//
//							  with the book or not, defaults to yes			//
//			opacity=0 to 1  - Defines the opacity of the image, 0 is		//
//							  completely transparent, 1 is opaque			//
//		<pb>	- Page break												//
//		<font>  - Set the font for the following text. Options are:			//
//			face=<face name>												//
//			size=<point size>												//
//			style=r/b/i/bi													//
//			color=rrggbb - Hex color										//
//			spacing=<pixels> - line spacing in pixels						//
//		<cover>	 - Optionally sets the cover mipmap for the book.			//
//			src=<mipmap name> - Selects the mipmap to be used, using the	//
//								same search methods as <img>. Unlike <img>	//
//								though, <cover> has no restriction on		//
//								mipmap format.								//
//			tint=rrggbb		  - Hex color for tinting the cover image		//
//			tintfirst=yes/no  - Tint the cover before applying decals,		//
//								defaults to yes. A no option overrides the	//
//								individual tint options on the <decal> tags	//
//		<margin> - Optionally sets the margin size in pixels, default is 16 //
//			top=<size>														//
//			left=<size>														//
//			bottom=<size>													//
//			right=<size>													//
//		<book>	- Optionally sets the size of the book in percent (0-1),	//
//				  can be overridden by the SetBookSize funtion.				//
//			height=<size>													//
//			width=<size>													//
//		<decal>	- Optionally specifies a decal to be drawn on the cover in	//
//				  the specified position. Options are:						//
//			src=<mipmap name> - Selects the mipmap to be used, using the	//
//								same search methods as <imp>. Unlike <img>	//
//								though, <decal> has no restrictions on		//
//								mipmap format.								//
//			pos=<x>,<y>		  - Specifies the position of the decal on the	//
//								cover using the upper-left pixel			//
//			align=left/right/center											//
//							  - Aligns the decal horizontally, overriding	//
//								any x coord in the pos option, if no y has	//
//								been specified, then it places it at the	//
//								top of the cover							//
//			resize=yes/no	  - Defines whether the image will be resized	//
//								with the book or not, defaults to yes		//
//			tint=yes/no		  - Defines whether or not this decal is tinted //
//								with the cover. Overridden by the tintfirst //
//								option on the <cover> tag. Defaults to no	//
//		<movie>	- Places a movie (.bik file) inline with the text. Options:	//
//			src=<movie name>  - Selects the movie to be used. (nead search  //
//								methods here eventually)					//
//			align=left/right/center											//
//							  - Aligns the movie horizontally, overriding	//
//								any x coord in the pos option				//
//			link=<eventID>	  - Defines the movie as clickable. When the	//
//								User clicks the movie, it will generate an	//
//								event with the given event ID and send it	//
//								to the calling python handler				//
//			pos=<x>,<y>		  - Specifies the position of the movie on the	//
//								page using the upper left pixel given, does //
//								not influence text flow						//
//			resize=yes/no	  - Defines whether the movie will be resized	//
//								with the book or not, defaults to yes		//
//			oncover=yes/no	  - Defines whether the movie will be placed on //
//								the cover or not, defaults to no. NOTE:		//
//								setting this to yes causes the link option  //
//								to be ignored since cover movies can't link //
//			loop=yes/no		  - Defines whether the movie will loop or not  //
//								defaults to yes								//
//		<editable> - Marks this book as editable (if the GUI supports it)	//
//																			//
//	The pages don't render until displayed. As a result, jumping to a given	//
//	page requires each page from the current position to the destination	//
//	to be rendered. Normally, this won't be a problem, because by default	//
//	books open to the first page, at which point each page renders one		//
//	at a time as the user flips through.									//
//																			//
//	The system assumes that no more than one book will ever actually be		//
//	shown at any time. As a result, the internal geometry for displaying	//
//	each book can be shared, reducing overhead and potential for errors.	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsTemplates.h"
#include "hsColorRGBA.h"

#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plUoid.h"



class pfEsHTMLChunk;
class pfGUIDialogMod;
class plLocation;
class pfGUICheckBoxCtrl;
class pfGUIButtonMod;
class pfJournalDlgProc;
class plDynamicTextMap;
class pfGUIClickMapCtrl;
class plLayerInterface;
class plMipmap;
class pfGUIProgressCtrl;
class hsGMaterial;
class plLayerBink;
class pfGUIMultiLineEditCtrl;

class pfJournalBook;
class pfBookMultiLineEditProc;

class pfBookData : public hsKeyedObject
{
public:
	enum WhichSide
	{
		kLeftSide = 0x01,
		kRightSide = 0x02,
		kBothSides = 0x03,
		kNoSides = 0
	};

	enum DynDisplayIndex
	{
		kLeftPage = 0,
		kRightPage,
		kTurnFrontPage,
		kTurnBackPage
	};
	
	pfBookData(const char *guiName = nil);
	virtual ~pfBookData();

	void LoadGUI(); // need this seperate because the plKey isn't setup until the constructor is done

	CLASSNAME_REGISTER(pfBookData);
	GETINTERFACE_ANY(pfBookData, hsKeyedObject);

	virtual hsBool MsgReceive(plMessage *pMsg);

	pfGUIDialogMod *Dialog() const {return fDialog;}
	pfGUICheckBoxCtrl *CoverButton() const {return fCoverButton;}
	pfGUICheckBoxCtrl *TurnPageButton() const {return fTurnPageButton;}
	pfGUIClickMapCtrl *LeftPageMap() const {return fLeftPageMap;}
	pfGUIClickMapCtrl *RightPageMap() const {return fRightPageMap;}
	plLayerInterface *CoverLayer() const {return fCoverLayer;}
	hsGMaterial *CoverMaterial() const {return fCoverMaterial;}
	hsGMaterial *PageMaterial(int index) const {if ((index<0)||(index>3)) return nil; else return fPageMaterials[index];}
	pfGUIButtonMod *LeftCorner() const {return fLeftCorner;}
	pfGUIButtonMod *RightCorner() const {return fRightCorner;}
	pfGUIProgressCtrl *WidthCtrl() const {return fWidthCtrl;}
	pfGUIProgressCtrl *HeightCtrl() const {return fHeightCtrl;}
	plMipmap *DefaultCover() const {return fDefaultCover;}
	pfJournalBook *CurBook() const {return fCurrBook;}

	hsBool StartedOpen() {return fStartedOpen;}
	hsBool CurrentlyOpen() {return fCurrentlyOpen;}
	hsBool CurrentlyTurning() {return fCurrentlyTurning;}
	hsBool IsEditable() {return fEditable;}
	WhichSide CurSFXPages() {return fCurrSFXPages;}

	void StartedOpen(hsBool startedOpen) {fStartedOpen = startedOpen;}
	void CurrentlyOpen(hsBool currentlyOpen) {fCurrentlyOpen = currentlyOpen;}
	void CurrentlyTurning(hsBool currentlyTurning) {fCurrentlyTurning = currentlyTurning;}
	void CurBook(pfJournalBook *curBook) {fCurrBook = curBook;}

	// Quick helper
	plDynamicTextMap *GetDTMap(UInt32 which);
	pfGUIMultiLineEditCtrl *GetEditCtrl(UInt32 which);

	// Seeks the width and height animations to set the desired book size. Sizes are in % across the animation
	void SetCurrSize(hsScalar w, hsScalar h);

	// Enables or disables the left and right page corners, to indicate current turnage state
	void UpdatePageCorners(WhichSide which);

	// Plays our book close animation
	void PlayBookCloseAnim(hsBool closeIt = true, hsBool immediate = false);

	// Finishes the start of the triggered page flip (once we're sure the animation is at the new frame)
	void StartTriggeredFlip(hsBool flipBackwards);

	// kill the flipping of a page... we are probably closing the book
	void KillPageFlip();

	// Registers (or unregisters) for time messages so we can process special FX if we need to
	void RegisterForSFX(WhichSide whichSides);

	void HitEndOfControlList(Int32 cursorPos);
	void HitBeginningOfControlList(Int32 cursorPos);

	void EnableEditGUI(hsBool enable=true);
	void DisableEditGUI() {EnableEditGUI(false);}
	
protected:
	friend class pfJournalDlgProc;

	enum Refs
	{
		kRefDialog = 0,
		kRefDefaultCover
	};

	std::string			fGUIName;

	// The pointer to our dialog
	pfGUIDialogMod		*fDialog;

	// And other interesting pointers
	pfGUICheckBoxCtrl	*fCoverButton;
	pfGUICheckBoxCtrl	*fTurnPageButton;
	pfGUIClickMapCtrl	*fLeftPageMap;
	pfGUIClickMapCtrl	*fRightPageMap;
	plLayerInterface	*fCoverLayer;
	hsGMaterial			*fCoverMaterial;
	hsGMaterial			*fPageMaterials[4];
	pfGUIButtonMod		*fLeftCorner;
	pfGUIButtonMod		*fRightCorner;
	pfGUIProgressCtrl	*fWidthCtrl;
	pfGUIProgressCtrl	*fHeightCtrl;
	
	pfGUIMultiLineEditCtrl *fLeftEditCtrl;
	pfGUIMultiLineEditCtrl *fRightEditCtrl;
	pfGUIMultiLineEditCtrl *fTurnFrontEditCtrl;
	pfGUIMultiLineEditCtrl *fTurnBackEditCtrl;

	// Pointer to our default (base) cover mipmap
	plMipmap			*fDefaultCover;

	// The current book using our data
	pfJournalBook		*fCurrBook;

	// Which side(s) we're currently doing SFX for
	WhichSide	fCurrSFXPages;

	// Base time to calc SFX anim positions from
	hsScalar	fBaseSFXTime;
	hsBool		fResetSFXFlag;
	hsBool		fSFXUpdateFlip;		// So we only update alternating pages every frame, to save processor time

	// What it says
	hsBool		fCurrentlyTurning, fCurrentlyOpen, fStartedOpen;

	hsBool		fEditable;
	Int32		fAdjustCursorTo;

	// Inits our dialog template
	void IInitTemplate(pfGUIDialogMod *templateDlg);

	// Process SFX for this frame
	void IHandleSFX(hsScalar currTime, WhichSide whichSide = kNoSides);

	// Yet another step in the page flip, to make SURE we're already showing the turning page before we fill in the page behind it
	void IFillUncoveringPage(hsBool rightSide);

	// Triggers the start of the page-flipping animation, as well as sets up the callback for when it's finished
	void ITriggerPageFlip(hsBool flipBackwards, hsBool immediate);

	// Finishes the triggered page flip, on callback
	void IFinishTriggeredFlip(hsBool wasBackwards);
};

class pfJournalBook : public hsKeyedObject
{
	public:

		// Enums of event types for the Book plNotifyMsg type
		enum NotifyTypes
		{
			kNotifyImageLink = 0,
			kNotifyShow,
			kNotifyHide,
			kNotifyNextPage,
			kNotifyPreviousPage,
			kNotifyCheckUnchecked,
			kNotifyClose,
		};

		// The constructor takes in the esHTML source for the journal, along with
		// the name of the mipmap to use as the cover of the book. The callback
		// key is the keyed object to send event messages to (see <img> tag).
		pfJournalBook( const char *esHTMLSource, plKey coverImageKey = nil, plKey callbackKey = nil, const plLocation &hintLoc = plLocation::kGlobalFixedLoc, const char *guiName = nil );
		pfJournalBook( const wchar_t *esHTMLSource, plKey coverImageKey = nil, plKey callbackKey = nil, const plLocation &hintLoc = plLocation::kGlobalFixedLoc, const char *guiName = nil );

		virtual ~pfJournalBook();

		CLASSNAME_REGISTER( pfJournalBook );
		GETINTERFACE_ANY( pfJournalBook, hsKeyedObject );

		// Our required virtual
		virtual hsBool	MsgReceive( plMessage *pMsg );

		// Init the singleton, for client startup
		static void	SingletonInit( void );
		
		// Shutdown the singleton
		static void	SingletonShutdown( void );
		
		// loads a gui
		static void LoadGUI( const char *guiName );

		// unloads a gui if we don't need it any more and want to free up memory
		static void UnloadGUI( const char *guiName );

		// unloads all GUIs except for the default
		static void UnloadAllGUIs();

		void	SetGUI( const char *guiName );

		// Shows the book, optionally starting open or closed
		void	Show( hsBool startOpened = false );

	
		/// NOTE: The following functions expose functionality that is normally
		/// handled by the book logic itself. So you should only need to use these
		/// in unusual circumstances.


		// Book handles hiding itself once someone clicks away.	
		void	Hide( void );

		// Opens the book, optionally to the given page
		void	Open( UInt32 startingPage = 0 );

		// Closes the book.
		void	Close( void );

		// Advances forward one page
		void	NextPage( void );

		// Same, only back
		void	PreviousPage( void );

		// For completeness...
		void	GoToPage( UInt32 pageNumber );

		// See below. Just forces a full calc of the cached info
		void	ForceCacheCalculations( void );

		// Closes the book, then calls Hide() once it's done closing
		void	CloseAndHide( void );

		// Sets the book size scaling. 1,1 would be full size, 0,0 is the smallest size possible
		void	SetBookSize( hsScalar width, hsScalar height );

		// What page are we on?
		UInt32	GetCurrentPage( void ) const { return fCurrentPage; }

		// Set the margin (defaults to 16 pixels)
		void	SetPageMargin( UInt32 margin ) { fPageTMargin = fPageLMargin = fPageBMargin = fPageRMargin = margin; }

		// Turns on or off page turning
		void	AllowPageTurning( hsBool allow ) { fAllowTurning = allow; }

		// grabs a certain movie based on it's index in the source file
		plKey	GetMovie( UInt8 index );

		// turns on and off editing of the book
		void	SetEditable( hsBool editable=true );

		// returns the text contained by the edit controls
		std::string GetEditableText();

		void	SetEditableText(std::string text);
		
	protected:

		struct loadedMovie
		{
			pfEsHTMLChunk *movieChunk;
			plLayerBink *movieLayer;
		};

		friend class pfJournalDlgProc;
		friend class pfBookData;

		// Our compiled esHTML source
		std::wstring				fUncompiledSource;
		plLocation					fDefLoc;
		hsTArray<pfEsHTMLChunk *>	fHTMLSource;
		hsTArray<pfEsHTMLChunk *>	fCoverDecals; // stored in a separate location so we can draw them all immediately

		hsTArray<loadedMovie *> fLoadedMovies;

		// The key of the mipmap to use as the cover image
		plKey	fCoverMipKey;
		bool	fTintCover;
		hsColorRGBA fCoverTint;
		bool	fTintFirst; // tint before applying decals?

		// Receiver key to send notifys to, if any
		plKey	fCallbackKey;
		bool	fCoverFromHTML;
		// Cached array of page starts in the esHTML source. Generated as we flip through
		// the book, so that going backwards can be done efficiently.
		hsTArray<UInt32>	fPageStarts;

		// is the book done showing and ready for more page calculations
		hsBool	fAreWeShowing;

		// Our current page
		UInt32	fCurrentPage;

		// are we editing this book? (adjusts how we draw and flip pages)
		hsBool	fAreEditing;
		hsBool	fWantEditing; // the code specifies that we want to edit, but the gui doesn't support it, we will check again if the gui changes

		hsBool	fAllowTurning; // do we allow the user to turn pages?

		// The ending page. -1 until calculated by flipping to it
		UInt32	fLastPage;

		// Per book size
		hsScalar	fWidthScale, fHeightScale;

		// Per book margin around the edge (defaults to 16 pixels)
		UInt32		fPageTMargin, fPageLMargin, fPageBMargin, fPageRMargin;

		// Some animation keys we use
		plKey	fPageTurnAnimKey;

		// Current list of linkable image chunks we have visible on the screen, for quick hit testing
		hsTArray<pfEsHTMLChunk *>	fVisibleLinks;

		static std::map<std::string,pfBookData*> fBookGUIs;
		std::string fCurBookGUI;

		enum Refs
		{
			kRefImage = 0
		};

		// Compiles the given string of esHTML source into our compiled chunk list
		hsBool	ICompileSource( const wchar_t *source, const plLocation &hintLoc );

		// Frees our source array
		void	IFreeSource( void );

		// Compile helpers
		UInt8	IGetTagType( const wchar_t *string );
		hsBool	IGetNextOption( const wchar_t *&string, wchar_t *name, wchar_t *option );

		plKey	IGetMipmapKey( const wchar_t *name, const plLocation &loc );

		// Renders one (1) page into the given DTMap
		void	IRenderPage( UInt32 page, UInt32 whichDTMap, hsBool suppressRendering = false );

		// moves the movie layers from one material onto another
		void	IMoveMovies( hsGMaterial *source, hsGMaterial *dest);

		// Starting at the given chunk, works backwards to determine the full set of current
		// font properties at that point, or assigns defaults if none were specified
		void	IFindFontProps( UInt32 chunkIdx, const wchar_t *&face, UInt8 &size, UInt8 &flags, hsColorRGBA &color, Int16 &spacing );

		// Find the last paragraph chunk and thus the last par alignment settings
		UInt8	IFindLastAlignment( void ) const;

		// Handle clicks on either side of the book
		void	IHandleLeftSideClick( void );
		void	IHandleRightSideClick( void );

		// Just sends out a notify to our currently set receiver key
		void	ISendNotify( UInt32 type, UInt32 linkID = 0 );

		// Close with a notify
		void	ITriggerCloseWithNotify( hsBool closeNotOpen, hsBool immediate );

		// Finish showing the book, due to the animation being done seeking
		void	IFinishShow( hsBool startOpened );

		// Find the current moused link, if any
		Int32	IFindCurrVisibleLink( hsBool rightNotLeft, hsBool hoverNotUp );

		// Ensures that all the page starts are calced up to the given page (but not including it)
		void	IRecalcPageStarts( UInt32 upToPage );

		// Load (or unload) all the images for the book
		void	ILoadAllImages( hsBool unload );
		
		// Purge the DynaTextMaps
		void	IPurgeDynaTextMaps( );

		// Process a click on the given "check box" image
		void	IHandleCheckClick( UInt32 idx, pfBookData::WhichSide which );

		// Draw me an image!
		void	IDrawMipmap( pfEsHTMLChunk *chunk, UInt16 x, UInt16 y, plMipmap *mip, plDynamicTextMap *dtMap, UInt32 whichDTMap, hsBool dontRender );
		
		// Movie functions
		loadedMovie			*IMovieAlreadyLoaded(pfEsHTMLChunk *chunk);
		loadedMovie			*IGetMovieByIndex(UInt8 index);
		plLayerBink			*IMakeMovieLayer(pfEsHTMLChunk *chunk, UInt16 x, UInt16 y, plMipmap *baseMipmap, UInt32 whichDTMap, hsBool dontRender);

		// Cover functions
		plLayerInterface	*IMakeBaseLayer(plMipmap *image);
		plLayerInterface	*IMakeDecalLayer(pfEsHTMLChunk *decalChunk, plMipmap *decal, plMipmap *baseMipmap);
		void	ISetDecalLayers(hsGMaterial *material,hsTArray<plLayerInterface*> layers);
};


#endif //_pfJournalBook_h
