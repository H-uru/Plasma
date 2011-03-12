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
//	plInputInterfaceMgr.h - The manager of all input interface layers		//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	2.20.02 mcn	- Created.													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plInputInterfaceMgr_h
#define _plInputInterfaceMgr_h

#include "../pnModifier/plSingleModifier.h"
#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "../pnInputCore/plKeyMap.h"

//// Class Definition ////////////////////////////////////////////////////////

class hsStream;
class hsResMgr;
class plInputInterface;
//class plKeyMap;
enum plKeyDef;
enum ControlEventCode;
class plKey;
class plCtrlCmd;
class plKeyCombo;
class plDefaultKeyCatcher;
class plKeyBinding;

class plInputInterfaceMgr : public plSingleModifier
{
	protected:

		static plInputInterfaceMgr	*fInstance;

		hsTArray<plInputInterface *>	fInterfaces;
		hsTArray<plCtrlCmd *>			fMessageQueue;
		hsTArray<plKey>					fReceivers;

#ifdef MCN_DISABLE_OLD_WITH_NEW_HACK
		hsTArray<ControlEventCode>		fDisabledCodes;
		hsTArray<UInt32>				fDisabledKeys;
#endif

		hsBool		fClickEnabled;
		Int32		fCurrentCursor;
		hsScalar	fCursorOpacity;
		hsBool		fForceCursorHidden;
		Int32		fForceCursorHiddenCount;
		plInputInterface		*fCurrentFocus;
		plDefaultKeyCatcher		*fDefaultCatcher;

		
		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty );

		void	IAddInterface( plInputInterface *iface );
		void	IRemoveInterface( plInputInterface *iface );

		void	IUpdateCursor( Int32 newCursor );
		hsBool	ICheckCursor(plInputInterface *iFace); // returns true if the iface changed cursor settings
			
		void	IWriteConsoleCmdKeys( plKeyMap *keyMap, FILE *keyFile );
		void	IWriteNonConsoleCmdKeys( plKeyMap *keyMap, FILE *keyFile );

		plKeyMap	*IGetRoutedKeyMap( ControlEventCode code );	// Null for console commands
		void		IUnbind( const plKeyCombo &key );

		const char	*IKeyComboToString( const plKeyCombo &combo );
		
	public:

		plInputInterfaceMgr();
		virtual ~plInputInterfaceMgr();

		CLASSNAME_REGISTER( plInputInterfaceMgr );
		GETINTERFACE_ANY( plInputInterfaceMgr, plSingleModifier );

		virtual hsBool	MsgReceive( plMessage *msg );
		virtual void	Read( hsStream* s, hsResMgr* mgr );
		virtual void	Write( hsStream* s, hsResMgr* mgr );

		void	Init( void );
		void	Shutdown( void );

		void		InitDefaultKeyMap( void );
		void		WriteKeyMap( void );
		void		RefreshInterfaceKeyMaps( void );

		void	SetCurrentFocus(plInputInterface *focus);
		void	ReleaseCurrentFocus(plInputInterface *focus);
		void	SetDefaultKeyCatcher( plDefaultKeyCatcher *c ) { fDefaultCatcher = c; }

		hsBool	IsClickEnabled() { return fClickEnabled; }

		void	ForceCursorHidden( hsBool requestedState );

		// Binding routers
		void	BindAction( const plKeyCombo &key, ControlEventCode code );
		void	BindAction( const plKeyCombo &key1, const plKeyCombo &key2, ControlEventCode code );
		void	BindConsoleCmd( const plKeyCombo &key, const char *cmd, plKeyMap::BindPref pref = plKeyMap::kNoPreference );

		const plKeyBinding* FindBinding( ControlEventCode code );
		const plKeyBinding* FindBindingByConsoleCmd( const char *cmd );

		void	ClearAllKeyMaps();
		void	ResetClickableState();
		static plInputInterfaceMgr	*GetInstance( void ) { return fInstance; }
};

//// plCtrlCmd ///////////////////////////////////////////////////////////////
//	Networkable helper class that represents a single control statement

class plCtrlCmd
{
	private:
		char*				fCmd;
		plInputInterface	*fSource;

	public:
		plCtrlCmd( plInputInterface *source ) : fCmd(nil),fPct(1.0f), fSource(source) {;}
		~plCtrlCmd() { delete [] fCmd; }

		const char* GetCmdString()			{ return fCmd; }
		void SetCmdString(const char* cs)	{ delete [] fCmd; fCmd=hsStrcpy(cs); }

		ControlEventCode	fControlCode;
		hsBool				fControlActivated;
		hsPoint3			fPt;
		hsScalar			fPct;

		hsBool				fNetPropagateToPlayers;

		void Read( hsStream* s, hsResMgr* mgr );
		void Write( hsStream* s, hsResMgr* mgr );

		plInputInterface	*GetSource( void ) const { return fSource; }
};

//// Tiny Virtual Class For The Default Key Processor ////////////////////////
//
//	Basically, if you want to be the one to catch the leftover key events,
//	derive from this class and pass yourself to inputIFaceMgr.
//	(it'll auto-tell inputIFaceMgr when it goes away)
//
//	Note: if you want to do more than just get the darned key event (like
//	mouse events or key bindings or change the cursor or the like), don't do
//	this; create your own plInputInterface instead.

class plKeyEventMsg;
class plDefaultKeyCatcher
{
	public:
		virtual ~plDefaultKeyCatcher();
		virtual void	HandleKeyEvent( plKeyEventMsg *eventMsg ) = 0;
};


#endif // _plInputInterfaceMgr_h
