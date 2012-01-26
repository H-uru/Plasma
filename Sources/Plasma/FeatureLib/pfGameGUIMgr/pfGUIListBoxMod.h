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
//  pfGUIListBoxMod Header                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIListBoxMod_h
#define _pfGUIListBoxMod_h

#include "pfGUIControlMod.h"

class plMessage;
class hsGMaterial;
class plTextGenerator;
class pfGUIListElement;
class pfScrollProc;
class pfGUIValueCtrl;

class pfGUIListBoxMod : public pfGUIControlMod
{
    friend class pfScrollProc;

    protected:

        struct plSmallRect
        {
            int16_t   fLeft, fTop, fRight, fBottom;

            void    Set( int16_t l, int16_t t, int16_t r, int16_t b );
            hsBool  Contains( int16_t x, int16_t y );

            plSmallRect& operator=(const int zero) { fLeft = fTop = fRight = fBottom = 0; return *this; }
        };

        pfGUIValueCtrl  *fScrollControl;

        pfScrollProc    *fScrollProc;

        hsTArray<pfGUIListElement *>    fElements;
        int32_t                           fCurrClick, fScrollPos, fCurrHover;
        uint8_t                           fModsAtDragTime;
        int32_t                           fMinSel, fMaxSel;
        hsBool                          fCheckScroll, fClicking;
        int32_t                           fSingleSelElement;
        hsBool                          fScrollRangeUpdateDeferred;
        hsBool                          fLocked, fReadyToRoll;
        hsTArray<plSmallRect>           fElementBounds;
        hsTArray<int16_t>                 fWrapStartIdxs;


        virtual hsBool IEval( double secs, float del, uint32_t dirty ); // called only by owner object's Eval()

        void    ICalcScrollRange( void );
        void    ICalcWrapStarts( void );

        virtual void    IUpdate( void );
        virtual void    IPostSetUpDynTextMap( void );
        virtual uint32_t  IGetDesiredCursor( void ) const;

        int32_t   IGetItemFromPoint( hsPoint3 &mousePt );
        void    IFindSelectionRange( int32_t *min, int32_t *max );
        void    ISelectRange( int8_t min, int8_t max, hsBool select );

    public:

        pfGUIListBoxMod();
        virtual ~pfGUIListBoxMod();

        CLASSNAME_REGISTER( pfGUIListBoxMod );
        GETINTERFACE_ANY( pfGUIListBoxMod, pfGUIControlMod );

        enum OurFlags
        {
            kSingleSelect = kDerivedFlagsStart,
            kDragAndDropCapable,
            kDisableSelection,
            kDisableKeyActions,
            kAllowMultipleElementsPerRow,
            kScrollLeftToRight,
            kAllowMousePassThrough,
            kGrowLeavesAndProcessOxygen,
            kHandsOffMultiSelect,       // Do multiselect w/o needing ctrl or shift
            kForbidNoSelection
        };

        // Extended event types
        enum ExtendedEvents
        {
            kScrollPosChanged,
            kItemAdded,
            kItemRemoved,
            kListCleared
        };

        enum
        {
            kRefScrollCtrl = kRefDerivedStart
        };

        virtual hsBool  MsgReceive( plMessage* pMsg );
        
        virtual void Read( hsStream* s, hsResMgr* mgr );
        virtual void Write( hsStream* s, hsResMgr* mgr );

        virtual void    HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers );
        virtual void    HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers );
        virtual void    HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers );
        virtual void    HandleMouseHover( hsPoint3 &mousePt, uint8_t modifiers );
        virtual void    HandleMouseDblClick( hsPoint3 &mousePt, uint8_t modifiers );

        virtual hsBool  HandleKeyPress( wchar_t key, uint8_t modifiers );
        virtual hsBool  HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers );

        virtual hsBool  FilterMousePosition( hsPoint3 &mousePt );

        virtual void PurgeDynaTextMapImage();

        // Returns selected element. Only valid for kSingleSelect list boxes
        int32_t   GetSelection( void ) { return fSingleSelElement; }
        void    SetSelection( int32_t item );
        void    RemoveSelection( int32_t item );
        void    AddSelection( int32_t item );
        
        virtual void    ScrollToBegin( void );
        virtual void    ScrollToEnd( void );
        virtual void    SetScrollPos( int32_t pos );
        virtual int32_t   GetScrollPos( void );
        virtual int32_t   GetScrollRange( void );


        void    Refresh( void ) { IUpdate(); }

        virtual void        SetColorScheme( pfGUIColorScheme *newScheme );

        // Element manipulation

        uint16_t  AddElement( pfGUIListElement *el );
        void    RemoveElement( uint16_t index );
        int16_t   FindElement( pfGUIListElement *toCompareTo );
        void    ClearAllElements( void );

        void    LockList( void );
        void    UnlockList( void );

        uint16_t              GetNumElements( void );
        pfGUIListElement    *GetElement( uint16_t idx );

        uint16_t  AddString( const char *string );
        uint16_t  AddString( const wchar_t *string );
        int16_t   FindString( const char *toCompareTo );
        int16_t   FindString( const wchar_t *toCompareTo );

        // Export only
        void    SetScrollCtrl( pfGUIValueCtrl *ctrl ) { fScrollControl = ctrl; }
        void    SetSingleSelect( hsBool yes ) { if( yes ) SetFlag( kSingleSelect ); else ClearFlag( kSingleSelect ); }
};

#endif // _pfGUIListBoxMod_h
