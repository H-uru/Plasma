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
//	pfConsole Header														//
//																			//
//	9.21.2001 mcn - Added pfLogDisplays. These are classes that represent 	//
//					a scrolling buffer, much like the console, for on-		//
//					screen display of error logs and the such. Currently	//
//					managed by the console, but hopefully once we have		//
//					a general manager for dialogs/menus (debug or			//
//					otherwise), that manager will take the displays over.	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfConsole_h
#define _pfConsole_h

#include "hsTypes.h"
#include "../pnKeyedObject/hsKeyedObject.h"


class plPipeline;

//// Class Definition ////////////////////////////////////////////////////////

class pfConsoleEngine;
class plKeyEventMsg;
class pfConsoleInputInterface;

class pfConsole : public hsKeyedObject 
{
	friend class pfConsoleInputInterface;

	protected:

		enum Konstants 
		{
			kNumHistoryItems = 16,
			kModeHidden = 0,
			kModeSingleLine = 1,
			kModeFull = 2,
			kEffectDivisions = 1000,
			kMaxCharsWide = 80,
			kHelpDelay = 32,
			kCursorBlinkRate = 16,
			kMsgHintTimeout = 64,
			kWorkingLineSize = 256
		};


		UInt32	fNumDisplayLines;

		Int32	fEffectCounter;
		float	fLastTime;
		UInt32	fHelpTimer;
		char	fLastHelpMsg[ kWorkingLineSize ];
		UInt8	fMode;		// 0 - invisible, 1 - single line, 2 - full
		hsBool	fInited, fHelpMode, fPythonMode, fPythonFirstTime, fFXEnabled;
		UInt32	fPythonMultiLines;
		short	fCursorTicks;
		UInt32	fMsgTimeoutTimer;

		char	fHistory[ kNumHistoryItems ][ kMaxCharsWide ];
		UInt32	fHistoryCursor, fHistoryRecallCursor;
		char	*fDisplayBuffer;
		char	fWorkingLine[ kWorkingLineSize ];
		UInt32	fWorkingCursor;

		pfConsoleInputInterface	*fInputInterface;

		pfConsoleEngine		*fEngine;

		void	IHandleKey( plKeyEventMsg *msg );
		char	IKeyEventToChar( plKeyEventMsg *msg );

		static UInt32		fConsoleTextColor;
		static pfConsole	*fTheConsole;
		static void	_cdecl IAddLineCallback( const char *string );

		static plPipeline	*fPipeline;

		void	IAddLine( const char *string, short leftMargin = 0 );
		void	IAddParagraph( const char *string, short margin = 0 );
		void	IClear( void );

		void	ISetMode( UInt8 mode );
		void	IEnableFX( hsBool e ) { fFXEnabled = e; }
		hsBool	IFXEnabled( void ) { return fFXEnabled; }

		void	IPrintSomeHelp( void );
		void	IUpdateTooltip( void );

	public:

		pfConsole();
		~pfConsole();

		CLASSNAME_REGISTER( pfConsole );
		GETINTERFACE_ANY( pfConsole, plReceiver );
		
		static pfConsole * GetInstance ();

		virtual hsBool	MsgReceive( plMessage *msg );
	
		void	Init( pfConsoleEngine *engine );
		void	Draw( plPipeline *p );

		static void	AddLine( const char *string ) { fTheConsole->IAddParagraph( string ); }
		static void	AddLineF(const char * fmt, ...);
		static void	Clear( void ) { fTheConsole->IClear(); }
		static void Hide( void ) { fTheConsole->ISetMode(kModeHidden); }

		static void EnableEffects( hsBool enable ) { fTheConsole->IEnableFX( enable ); }
		static hsBool AreEffectsEnabled( void ) { return fTheConsole->IFXEnabled(); }
		static void	SetTextColor( UInt32 color ) { fConsoleTextColor = color; }
		static UInt32 GetTextColor() { return fConsoleTextColor; }

		static void			SetPipeline( plPipeline *pipe ) { fPipeline = pipe; }
		static plPipeline	*GetPipeline( void ) { return fPipeline; }
		
		static void RunCommandAsync (const char cmd[]);
};

#endif //_pfConsole_h

