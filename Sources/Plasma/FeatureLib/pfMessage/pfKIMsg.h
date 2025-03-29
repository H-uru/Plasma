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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfKIMsg Header                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfKIMsg_h
#define _pfKIMsg_h

#include "HeadSpin.h"
#include "hsStream.h"
#include "pnMessage/plMessage.h"




class pfKIMsg : public plMessage
{
#ifndef KI_CONSTANTS_ONLY
    protected:

        uint8_t   fCommand;
        uint32_t  fFlags;

        // for the hack chat message thingy
        ST::string  fUser;
        uint32_t    fPlayerID;
        ST::string  fString;

        // for the SetChatFadeDelay
        float fDelay;

        // other values
        int32_t   fValue;

        void IInit()
        {
            fCommand = kNoCommand;
            fString = ST::string();
            fUser = ST::string();
            fPlayerID = 0;
            fFlags = 0;
            fDelay = 0.0;
            fValue = 0;
        }

#endif // def KI_CONSTANTS_ONLY

    public:
        enum 
        {
            kHACKChatMsg,               // send chat message via pfKIMsg
            kEnterChatMode,             // toggle chat mode
            kSetChatFadeDelay,          // set the chat delay
            kSetTextChatAdminMode,      // set the chat admin mode... not used (see CCR)
            kDisableKIandBB,            // disable KI and blackbar (for things like AvaCusta)
            kEnableKIandBB,             // re-enable the KI and blackbar
            kYesNoDialog,               // display a Yes/No dialog
            kAddPlayerDevice,           // add a device player list, such as imager
            kRemovePlayerDevice,        // remove a device from player list
            kUpgradeKILevel,            // upgrade the KI to higher level
            kDowngradeKILevel,          // downgrade KI to next lower level
            kRateIt,                    // display the "RateIt"(tm) dialog
            kSetPrivateChatChannel,     // set the private chat channel (for private rooms)
            kUnsetPrivateChatChannel,   // unset private chat channel
            kStartBookAlert,            // blink the book image on the blackbar
            kMiniBigKIToggle,           // shortcut to toggling the miniKI/bigKI
            kKIPutAway,                 // shortcut to hiding all of the KI
            kChatAreaPageUp,            // shortcut to paging up the chat area
            kChatAreaPageDown,          // shortcut to paging down the chat area
            kChatAreaGoToBegin,         // shortcut to going to the beginning of the chat area
            kChatAreaGoToEnd,           // shortcut to going to the end of the chat area
            kKITakePicture,             // shortcut to taking a picture in the KI
            kKICreateJournalNote,       // shortcut to creating a journal note in the KI
            kKIToggleFade,              // shortcut to toggle fade mode
            kKIToggleFadeEnable,        // shortcut to toggling fade enabled
            kKIChatStatusMsg,           // display status message in chat window
            kKILocalChatStatusMsg,      // display status message in chat window
            kKIUpSizeFont,              // bump up the size of the chat area font
            kKIDownSizeFont,            // down size the font of the chat area
            kKIOpenYeehsaBook,          // open the playerbook if not already open
            kKIOpenKI,                  // open up in degrees the KI
            kKIShowCCRHelp,             // show the CCR help dialog
            kKICreateMarker,            // create a marker
            kKICreateMarkerFolder,      // create a marker folder in the current Age's journal folder
            kKILocalChatErrorMsg,       // display error message in chat window
            kKIPhasedAllOn,             // turn on all the phased KI functionality
            kKIPhasedAllOff,            // turn off all the phased KI functionality
            kKIOKDialog,                // display an OK dialog box (localized)
            kDisableYeeshaBook,         // don't allow linking with the Yeesha book (gameplay)
            kEnableYeeshaBook,          // re-allow linking with the Yeesha book
            kQuitDialog,                // put up quit dialog
            kTempDisableKIandBB,        // temp disable KI and blackbar (done by av system)
            kTempEnableKIandBB,         // temp re-enable the KI and blackbar (done by av system)
            kDisableEntireYeeshaBook,   // disable the entire Yeeshabook, not for gameplay, but prevent linking
            kEnableEntireYeeshaBook,
            kKIOKDialogNoQuit,          // display OK dialog in the KI without quiting afterwards
            kGZUpdated,                 // the GZ game was updated
            kGZInRange,                 // a GZ marker is in range
            kGZOutRange,                // GZ markers are out of range
            kUpgradeKIMarkerLevel,      // upgrade the KI Marker level (current 0 and 1)
            kKIShowMiniKI,              // force the miniKI up
            kGZFlashUpdate,             // flash an update without saving (for animation of GZFill in)
            kStartJournalAlert,         // blink the journal image on the blackbar
            kAddJournalBook,            // add the journal to the blackbar
            kRemoveJournalBook,         // remove the journal from the blackbar
            kKIOpenJournalBook,         // open the journal book
            kMGStartCGZGame,            // Start CGZ Marker Game
            kMGStopCGZGame,             // Stop CGZ Marker Game
            kKICreateMarkerNode,        // Creates the marker game vault Node
            kStartKIAlert,              // start the KI alert
            kUpdatePelletScore,         // Updates the pellet score
            kFriendInviteSent,          // Friend invite was attempted and result received
            kRegisterImager,            // Register imager with the KI
            kNoCommand
        };

        enum Flags
        {
            kPrivateMsg     = 0x00000001,
            kAdminMsg       = 0x00000002,
            kGlobalMsg      = 0x00000004,
            kInterAgeMsg    = 0x00000008,
            kStatusMsg      = 0x00000010,
            kNeighborMsg    = 0x00000020,   // sending to all the neighbors
            kSubtitleMsg    = 0x00000040,
            kLocKeyMsg      = 0x00000080,
            kChannelMask    = 0x0000ff00
        };

        static const char* kChronicleKILevel;
        enum KILevels
        {
            kNanoKI = 0,
            kMicroKI = 1,
            kNormalKI = 2
        };

#ifndef KI_CONSTANTS_ONLY

        pfKIMsg() : plMessage(nullptr, nullptr, nullptr) { SetBCastFlag(kBCastByExactType); IInit(); }
        pfKIMsg(uint8_t command) : plMessage(nullptr, nullptr, nullptr) { SetBCastFlag(kBCastByExactType); IInit(); fCommand = command; }
        pfKIMsg(const plKey& receiver, uint8_t command) : plMessage(nullptr, nullptr, nullptr) { AddReceiver(receiver); IInit(); fCommand = command; }

        CLASSNAME_REGISTER( pfKIMsg );
        GETINTERFACE_ANY( pfKIMsg, plMessage );

        void Read(hsStream* s, hsResMgr* mgr) override
        { 
            plMessage::IMsgRead( s, mgr ); 
            s->ReadByte(&fCommand);
            fUser = s->ReadSafeString();
            fPlayerID = s->ReadLE32();
            fString = s->ReadSafeWString();
            fFlags = s->ReadLE32();
            fDelay = s->ReadLEFloat();
            fValue = s->ReadLE32();
        }
        
        void Write(hsStream* s, hsResMgr* mgr) override
        { 
            plMessage::IMsgWrite( s, mgr ); 
            s->WriteByte(fCommand);
            s->WriteSafeString( fUser );
            s->WriteLE32( fPlayerID );
            s->WriteSafeWString( fString );
            s->WriteLE32( fFlags );
            s->WriteLEFloat(fDelay);
            s->WriteLE32( fValue );
        }

        uint8_t     GetCommand() const { return fCommand; }

        void        SetString( const ST::string &str ) { fString = str; }
        ST::string  GetString() { return fString; }

        void        SetUser(const ST::string &str, uint32_t pid=0) { fUser = str; fPlayerID = pid; }
        ST::string  GetUser() const { return fUser; }
        uint32_t    GetPlayerID() const { return fPlayerID; }

        void        SetFlags( uint32_t flags ) { fFlags = flags; }
        uint32_t    GetFlags() const { return fFlags; }

        void        SetDelay( float delay ) { fDelay = delay; }
        float       GetDelay() { return fDelay; }

        void        SetIntValue( int32_t value ) { fValue = value; }
        int32_t     GetIntValue() { return fValue; }

#endif // def KI_CONSTANTS_ONLY
};

#endif // _pfKIMsg_h
