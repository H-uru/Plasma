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
//  pfGUIDialogMod Header                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIDialogMod_h
#define _pfGUIDialogMod_h


#include "pnModifier/plSingleModifier.h"
#include "pfGameGUIMgr.h"
#include "hsMatrix44.h"

class plMessage;
class plPostEffectMod;
class pfGUIControlMod;
class pfGUIDialogProc;
class pfGUIListElement;
class pfGUIColorScheme;

class pfGUIDialogMod : public plSingleModifier
{
    private:
        pfGUIDialogMod  *fNext, **fPrevPtr;

    protected:

        uint32_t      fTagID;     // 0 if none

        uint32_t      fVersion;   // Nice for syncing to C++ code

        plPostEffectMod             *fRenderMod;
        bool                        fEnabled;
        char                        fName[ 128 ];
        hsTArray<pfGUIControlMod *> fControls;
        pfGUIControlMod             *fControlOfInterest;
        pfGUIControlMod             *fFocusCtrl;
        pfGUIControlMod             *fMousedCtrl;   // Which one is the mouse over?
        pfGUIColorScheme            *fColorScheme;

        pfGUIDialogProc             *fHandler;
        plKey                       fProcReceiver;      // Non-nil means we handle everything by creating notify messages and sending them to this key

        hsTArray<pfGUIListElement *>    fDragElements;
        bool                            fDragMode, fDragReceptive;
        pfGUIControlMod                 *fDragTarget;
        pfGUIControlMod                 *fDragSource;

        plKey           fSceneNodeKey;


        virtual bool IEval( double secs, float del, uint32_t dirty ); // called only by owner object's Eval()

        void    IHandleDrag( hsPoint3 &mousePt, pfGameGUIMgr::EventType event, uint8_t modifiers );

    public:

        enum
        {
            kRenderModRef = 0,
            kControlRef,
            kRefDerviedStart
        };

        enum Flags
        {
            kModal,
            kDerivedFlagsStart
        };

        pfGUIDialogMod();
        virtual ~pfGUIDialogMod();

        CLASSNAME_REGISTER( pfGUIDialogMod );
        GETINTERFACE_ANY( pfGUIDialogMod, plSingleModifier );


        virtual bool    MsgReceive( plMessage* pMsg );
        
        virtual void Read( hsStream* s, hsResMgr* mgr );
        virtual void Write( hsStream* s, hsResMgr* mgr );

        void        SetSceneNodeKey( plKey &key ) { fSceneNodeKey = key; }
        plKey       GetSceneNodeKey( void );

        virtual void    SetEnabled( bool e );
        bool            IsEnabled( void ) { return fEnabled; }

        const char  *GetName( void ) { return fName; }

        void        ScreenToWorldPoint( float x, float y, float z, hsPoint3 &outPt );
        hsPoint3    WorldToScreenPoint( const hsPoint3 &inPt );

        virtual bool    HandleMouseEvent( pfGameGUIMgr::EventType event, float mouseX, float mouseY, uint8_t modifiers );
        bool            HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers );
        bool            HandleKeyPress( wchar_t key, uint8_t modifiers );
        void            UpdateInterestingThings( float mouseX, float mouseY, uint8_t modifiers, bool modalPreset );

        void            SetControlOfInterest( pfGUIControlMod *c );
        pfGUIControlMod *GetControlOfInterest( void ) const { return fControlOfInterest; }
        uint32_t          GetDesiredCursor( void ) const;

        void        UpdateAspectRatio( void );
        void        UpdateAllBounds( void );
        void        RefreshAllControls( void );

        void            AddControl( pfGUIControlMod *ctrl );
        uint32_t          GetNumControls( void ) { return fControls.GetCount(); }
        pfGUIControlMod *GetControl( uint32_t idx ) { return fControls[ idx ]; }

        pfGUIColorScheme    *GetColorScheme( void ) const { return fColorScheme; }

        void    LinkToList( pfGUIDialogMod **prevPtr )
        {
            fNext = *prevPtr;
            if( *prevPtr )
                (*prevPtr)->fPrevPtr = &fNext;
            fPrevPtr = prevPtr;
            *prevPtr = this;
        }

        void    Unlink( void )
        {
            if( fNext )
                fNext->fPrevPtr = fPrevPtr;
            *fPrevPtr = fNext;

            fPrevPtr = nil;
            fNext = nil;
        }

        void            SetFocus( pfGUIControlMod *ctrl );
        void            Show( void );
        void            ShowNoReset( void );
        void            Hide( void );
        bool            IsVisible( void ) { return IsEnabled(); }

        pfGUIControlMod *GetFocus( void ) { return fFocusCtrl; }

        pfGUIDialogMod  *GetNext( void ) { return fNext; }
        uint32_t          GetTagID( void ) { return fTagID; }
        pfGUIControlMod *GetControlFromTag( uint32_t tagID );

        void            SetHandler( pfGUIDialogProc *hdlr );
        pfGUIDialogProc *GetHandler( void ) const { return fHandler; }

        plPostEffectMod *GetRenderMod( void ) const { return fRenderMod; }

        // This sets the handler for the dialog and ALL of its controls
        void            SetHandlerForAll( pfGUIDialogProc *hdlr );

        // Just a little macro-type thing here
        void            SetControlHandler( uint32_t tagID, pfGUIDialogProc *hdlr );

        /// Methods for doing drag & drop of listElements

        void    ClearDragList( void );
        void    AddToDragList( pfGUIListElement *e );
        void    EnterDragMode( pfGUIControlMod *source );

        uint32_t  GetVersion( void ) const { return fVersion; }

        // Export only
        void        SetRenderMod( plPostEffectMod *mod ) { fRenderMod = mod; }
        void        SetName( const char *name ) { hsStrncpy( fName, name, sizeof( fName ) - 1 ); }
        void        AddControlOnExport( pfGUIControlMod *ctrl );
        void        SetTagID( uint32_t id ) { fTagID = id; }
        void        SetProcReceiver( plKey key ) { fProcReceiver = key; }
        void        SetVersion( uint32_t version ) { fVersion = version; }
};

#endif // _pfGUIDialogMod_h
