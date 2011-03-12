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
//	plProgressMgr Header 													//
//																			//
//// Description /////////////////////////////////////////////////////////////
//																			//
//	The plProgressMgr is a method by which any part of the client can		//
//	display a progress bar indicating a lengthy operation.					//
//	Basically, a function/class/whatnot registers an operation with the		//
//	plProgressMgr. 
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plProgressMgr_h
#define _plProgressMgr_h

#include "hsTypes.h"
#include "hsUtils.h"

class plPipeline;
class plPlate;

//// plOperationProgress Definition //////////////////////////////////////////
//	The object you get back when you register a lengthy operation. Call this
//	to update your progress, destroy it when you're done.

class plProgressMgr;
class plOperationProgress
{
	friend class plProgressMgr;
	friend class plDTProgressMgr;

	protected:

		hsScalar	fValue, fMax;
		char		fStatusText[ 256 ];
		char		fTitle[ 256 ];
		UInt32		fContext;
		double		fStartTime;

		UInt32 fElapsedSecs, fRemainingSecs;
		hsScalar fAmtPerSec;

		enum Flags
		{
			kShouldCancel	= 0x1,
			kInitUpdate		= 0x2,
			kFirstUpdate	= 0x4,
			kLastUpdate		= 0x8,
			kAborting		= 0x10,
			kRetry			= 0x20,
			kOverall		= 0x40,
			kAlwaysDrawText	= 0x80,
		};
		UInt8 fFlags;

		plOperationProgress	*fNext, *fBack;

		void IUpdateStats();

		// For overall progress bars
		void IChildUpdateBegin(plOperationProgress* child);
		void IChildUpdateEnd(plOperationProgress* child);

		plOperationProgress( hsScalar length );

	public:

		~plOperationProgress();

		hsScalar GetMax( void ) const { return fMax; }
		hsScalar GetProgress( void ) const { return fValue; }
		const char * GetTitle( void ) const { return fTitle; }
		const char * GetStatusText( void ) const { return fStatusText; }
		UInt32	GetContext( void ) const { return fContext; }
		UInt32 GetElapsedSecs() { return fElapsedSecs; }
		UInt32 GetRemainingSecs() { return fRemainingSecs; }
		hsScalar GetAmtPerSec() { return fAmtPerSec; }

		// Adds on to current value
		void	Increment( hsScalar byHowMuch );

		// Sets current value
		void	SetHowMuch( hsScalar byHowMuch );

		// Set the length
		void	SetLength( hsScalar length );

		// Sets the display text above the bar (nil for nothing)
		void	SetStatusText( const char *text );

		// Sets the title
		void	SetTitle( const char *title );

		// Application data
		void	SetContext( UInt32 context ) { fContext = context;}

		hsBool	IsDone( void ) { return ( fValue < fMax ) ? false : true; }

		// True if this is the initial update (progress was just created)
		bool IsInitUpdate() { return hsCheckBits(fFlags, kInitUpdate); }
		// True if this is the first time the progress was updated
		bool IsFirstUpdate() { return hsCheckBits(fFlags, kFirstUpdate); }
		// Returns true if this is the last update you'll get from this
		// operation (because it's getting deleted)
		bool IsLastUpdate() { return hsCheckBits(fFlags, kLastUpdate); }

		// This type of progress is just tracking the overall progress of it's children
		bool IsOverallProgress() { return hsCheckBits(fFlags, kOverall); }

		// Set if this progress is aborting before it completes.  This will let any overall
		// progress bars above this one know to adjust their totals to not include any amount
		// that wasn't completed, and will set this progress bar to zero
		void SetAborting();
		// If you're reusing an existing progress bar to retry a failed operation, call this.
		// It will set the retry flag, and reset the progress bar so the next update will
		// count as the first.  If you set retry in RegisterOperation, don't use this too.
		void SetRetry();
		bool IsRetry() { return hsCheckBits(fFlags, kRetry); }

		// The progress manager can decide at any time to cancel your operation on you. Check this
		// value if you want to play nice and behave.
		bool ShouldCancel() const { return hsCheckBits(fFlags, kShouldCancel); }

		bool AlwaysDrawText() const { return hsCheckBits(fFlags, kAlwaysDrawText); }

		// Please ignore this and don't use it unless you're me :P
		plOperationProgress* GetPrev() const { return fBack; }
		plOperationProgress* GetNext() const { return fNext; }

		// Or this
		void	SetCancelFlag( hsBool f ) { hsChangeBits(fFlags, kShouldCancel, f); }
};

// This is a callback proc you set that gets called every time the progressManager
// needs updating (like, say, you need to redraw progress bars). The client generally
// sets this callback and nobody should ever touch it.
typedef	void(*plProgressMgrCallbackProc)( plOperationProgress* );

//// Manager Class Definition ////////////////////////////////////////////////

class plProgressMgr
{
	friend class plOperationProgress;

	public:
		// this must match the order of the fStaticTextIDs array
		// for it to be useful
		enum StaticText
		{
			kNone,
			kUpdateText,
		};

	private:

		static plProgressMgr	*fManager;
		static int				fImageRotation[];
		static int				fStaticTextIDs[];

	protected:

		plProgressMgr();

		plOperationProgress		*fOperations;

		plProgressMgrCallbackProc	fCallbackProc;

		StaticText	fCurrentStaticText;

		void IUpdateCallbackProc(plOperationProgress* progress);
		// For derived classes to use, so they don't have to set a callback proc
		virtual void IDerivedCallbackProc(plOperationProgress* progress) {}

		void IUpdateFlags(plOperationProgress* progress);

		plOperationProgress* IRegisterOperation(hsScalar length, const char *title, StaticText staticTextType, bool isRetry, bool isOverall, bool alwaysDrawText);
		// Called by the operation
		void IUnregisterOperation(plOperationProgress* op);

		virtual void Activate() {}
		virtual void Deactivate() {}

		static plProgressMgr	*IGetManager( void ) { return fManager; }

	public:

		virtual ~plProgressMgr();

		static plProgressMgr* GetInstance() { return fManager; }
		static int GetLoadingFrameID(int index);
		static int GetStaticTextID(StaticText staticTextType);

		virtual void	Draw( plPipeline *p ) { }

		plOperationProgress* RegisterOperation(hsScalar length, const char *title = nil, StaticText staticTextType = kNone, bool isRetry = false, bool alwaysDrawText = false);
		plOperationProgress* RegisterOverallOperation(hsScalar length, const char *title = nil, StaticText staticTextType = kNone, bool alwaysDrawText = false);


		plProgressMgrCallbackProc SetCallbackProc( plProgressMgrCallbackProc proc );

		hsBool		IsActive( void ) const { return ( fOperations != nil ) ? true : false; }

		void	CancelAllOps( void );
};


#endif //_plProgressMgr_h

