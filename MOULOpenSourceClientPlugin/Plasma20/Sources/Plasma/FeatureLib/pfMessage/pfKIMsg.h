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
//	pfKIMsg Header 															//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfKIMsg_h
#define _pfKIMsg_h

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "hsStream.h"
#include "../pnMessage/plMessage.h"




class pfKIMsg : public plMessage
{
#ifndef KI_CONSTANTS_ONLY
	protected:

		UInt8	fCommand;
		UInt32	fFlags;

		// for the hack chat message thingy
		char	*fUser;
		UInt32	fPlayerID;
		std::wstring	fString;

		// for the SetChatFadeDelay
		hsScalar fDelay;

		// other values
		Int32	fValue;

		void IInit()
		{
			fCommand = kNoCommand;
			fString = L"";
			fUser = nil;
			fPlayerID = 0;
			fFlags = 0;
			fDelay = 0.0;
			fValue = 0;
		}

#endif // def KI_CONSTANTS_ONLY

	public:
		enum 
		{
			kHACKChatMsg,				// send chat message via pfKIMsg
			kEnterChatMode,				// toggle chat mode
			kSetChatFadeDelay,			// set the chat delay
			kSetTextChatAdminMode,		// set the chat admin mode... not used (see CCR)
			kDisableKIandBB,			// disable KI and blackbar (for things like AvaCusta)
			kEnableKIandBB,				// re-enable the KI and blackbar
			kYesNoDialog,				// display a Yes/No dialog
			kAddPlayerDevice,			// add a device player list, such as imager
			kRemovePlayerDevice,		// remove a device from player list
			kUpgradeKILevel,			// upgrade the KI to higher level
			kDowngradeKILevel,			// downgrade KI to next lower level
			kRateIt,					// display the "RateIt"(tm) dialog
			kSetPrivateChatChannel,		// set the private chat channel (for private rooms)
			kUnsetPrivateChatChannel,	// unset private chat channel
			kStartBookAlert,			// blink the book image on the blackbar
			kMiniBigKIToggle,			// shortcut to toggling the miniKI/bigKI
			kKIPutAway,					// shortcut to hiding all of the KI
			kChatAreaPageUp,			// shortcut to paging up the chat area
			kChatAreaPageDown,			// shortcut to paging down the chat area
			kChatAreaGoToBegin,			// shortcut to going to the beginning of the chat area
			kChatAreaGoToEnd,			// shortcut to going to the end of the chat area
			kKITakePicture,				// shortcut to taking a picture in the KI
			kKICreateJournalNote,		// shortcut to creating a journal note in the KI
			kKIToggleFade,				// shortcut to toggle fade mode
			kKIToggleFadeEnable,		// shortcut to toggling fade enabled
			kKIChatStatusMsg,			// display status message in chat window
			kKILocalChatStatusMsg,		// display status message in chat window
			kKIUpSizeFont,				// bump up the size of the chat area font
			kKIDownSizeFont,			// down size the font of the chat area
			kKIOpenYeehsaBook,			// open the playerbook if not already open
			kKIOpenKI,					// open up in degrees the KI
			kKIShowCCRHelp,				// show the CCR help dialog
			kKICreateMarker,			// create a marker
			kKICreateMarkerFolder,		// create a marker folder in the current Age's journal folder
			kKILocalChatErrorMsg,		// display error message in chat window
			kKIPhasedAllOn,				// turn on all the phased KI functionality
			kKIPhasedAllOff,			// turn off all the phased KI functionality
			kKIOKDialog,				// display an OK dialog box (localized)
			kDisableYeeshaBook,			// don't allow linking with the Yeesha book (gameplay)
			kEnableYeeshaBook,			// re-allow linking with the Yeesha book
			kQuitDialog,				// put up quit dialog
			kTempDisableKIandBB,		// temp disable KI and blackbar (done by av system)
			kTempEnableKIandBB,			// temp re-enable the KI and blackbar (done by av system)
			kDisableEntireYeeshaBook,	// disable the entire Yeeshabook, not for gameplay, but prevent linking
			kEnableEntireYeeshaBook,
			kKIOKDialogNoQuit,			// display OK dialog in the KI without quiting afterwards
			kGZUpdated,					// the GZ game was updated
			kGZInRange,					// a GZ marker is in range
			kGZOutRange,				// GZ markers are out of range
			kUpgradeKIMarkerLevel,		// upgrade the KI Marker level (current 0 and 1)
			kKIShowMiniKI,				// force the miniKI up
			kGZFlashUpdate,				// flash an update without saving (for animation of GZFill in)
			kStartJournalAlert,			// blink the journal image on the blackbar
			kAddJournalBook,			// add the journal to the blackbar
			kRemoveJournalBook,			// remove the journal from the blackbar
			kKIOpenJournalBook,			// open the journal book
			kMGStartCGZGame,			// Start CGZ Marker Game
			kMGStopCGZGame,				// Stop CGZ Marker Game
			kKICreateMarkerNode,		// Creates the marker game vault Node
			kStartKIAlert,				// start the KI alert
			kUpdatePelletScore,			// Updates the pellet score
			kFriendInviteSent,			// Friend invite was attempted and result received
			kRegisterImager,			// Register imager with the KI
			kNoCommand
		};

		enum Flags
		{
			kPrivateMsg		= 0x00000001,
			kAdminMsg		= 0x00000002,
			kDead			= 0x00000004,
			kUNUSED1		= 0x00000008,
			kStatusMsg		= 0x00000010,
			kNeighborMsg	= 0x00000020,	// sending to all the neighbors
			kChannelMask	= 0x0000ff00
		};

		static const char* kChronicleKILevel;
		enum KILevels
		{
			kNanoKI = 0,
			kMicroKI = 1,
			kNormalKI = 2
		};

#ifndef KI_CONSTANTS_ONLY

		pfKIMsg() : plMessage( nil, nil, nil ) { SetBCastFlag( kBCastByExactType ); IInit(); }
		pfKIMsg( UInt8 command ) : plMessage( nil, nil, nil ) { SetBCastFlag( kBCastByExactType ); IInit(); fCommand = command; }
		pfKIMsg( plKey &receiver, UInt8 command ) : plMessage( nil, nil, nil ) { AddReceiver( receiver ); IInit(); fCommand = command; }
		~pfKIMsg() { delete [] fUser; }

		CLASSNAME_REGISTER( pfKIMsg );
		GETINTERFACE_ANY( pfKIMsg, plMessage );

		virtual void Read(hsStream* s, hsResMgr* mgr) 
		{ 
			plMessage::IMsgRead( s, mgr ); 
			s->ReadSwap( &fCommand );
			fUser = s->ReadSafeString();
			fPlayerID = s->ReadSwap32();

			wchar_t *temp = s->ReadSafeWString();
			if (temp) // apparently ReadSafeWString can return null, which std::wstring doesn't like being assigned
				fString = temp;
			else
				fString = L"";
			delete [] temp;

			fFlags = s->ReadSwap32();
			fDelay = s->ReadSwapScalar();
			fValue = s->ReadSwap32();
		}
		
		virtual void Write(hsStream* s, hsResMgr* mgr) 
		{ 
			plMessage::IMsgWrite( s, mgr ); 
			s->WriteSwap( fCommand );
			s->WriteSafeString( fUser );
			s->WriteSwap32( fPlayerID );
			s->WriteSafeWString( fString.c_str() );
			s->WriteSwap32( fFlags );
			s->WriteSwapScalar(fDelay);
			s->WriteSwap32( fValue );
		}

		UInt8		GetCommand( void ) const { return fCommand; }

		void		SetString( const char *str );
		void		SetString( const wchar_t *str ) { fString = str; }
		std::string GetString( void );
		std::wstring GetStringU( void ) { return fString; }

		void		SetUser( const char *str, UInt32 pid=0 ) { fUser = hsStrcpy( str ); fPlayerID = pid; }
		const char	*GetUser( void ) { return fUser; }
		UInt32		GetPlayerID( void ) { return fPlayerID; }

		void		SetFlags( UInt32 flags ) { fFlags = flags; }
		UInt32		GetFlags( void ) const { return fFlags; }

		void		SetDelay( hsScalar delay ) { fDelay = delay; }
		hsScalar	GetDelay( void ) { return fDelay; }

		void		SetIntValue( Int32 value ) { fValue = value; }
		Int32		GetIntValue( void ) { return fValue; }

#endif // def KI_CONSTANTS_ONLY
};

#endif // _pfKIMsg_h
