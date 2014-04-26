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
//  pfGUIControlMod Header                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIControlMod_h
#define _pfGUIControlMod_h


#include "pnModifier/plSingleModifier.h"
#include "hsBounds.h"
#include "plMessage/plInputEventMsg.h"
#include "pfGameGUIMgr.h"
#include "hsColorRGBA.h"
#include "hsRefCnt.h"

class plMessage;
class plPostEffectMod;
class pfGUIDialogMod;
class pfGUICtrlProcObject;
class pfGUIDropTargetProc;
class plDynamicTextMap;
class plLayerInterface;

//// pfGUIColorScheme ////////////////////////////////////////////////////////
//  Tiny helper wrapper for a set of colors used to draw various controls

class pfGUIColorScheme : public hsRefCnt
{
    public:
        hsColorRGBA fForeColor, fBackColor;
        hsColorRGBA fSelForeColor, fSelBackColor;
        bool        fTransparent;

        plString    fFontFace;
        uint8_t     fFontSize;
        uint8_t     fFontFlags;

        enum FontFlags
        {
            kFontBold       = 0x01,
            kFontItalic     = 0x02,
            kFontShadowed   = 0x04
        };

        pfGUIColorScheme();
        pfGUIColorScheme( hsColorRGBA &foreColor, hsColorRGBA &backColor );
        pfGUIColorScheme( const plString &face, uint8_t size, uint8_t fontFlags );

        void    SetFontFace(const plString &face) { fFontFace = face; }

        void    Read( hsStream *s );
        void    Write( hsStream *s );

        bool    IsBold() const { return ( fFontFlags & kFontBold ) ? true : false; }
        bool    IsItalic() const { return ( fFontFlags & kFontItalic ) ? true : false; }
        bool    IsShadowed() const { return ( fFontFlags & kFontShadowed ) ? true : false; }

    protected:

        void    IReset( void );
};

//// Class Def ///////////////////////////////////////////////////////////////

class pfGUISkin;
class pfGUIControlMod : public plSingleModifier
{
    friend class pfGUIDialogMod;

    protected:

        uint32_t              fTagID;
        bool                fEnabled, fFocused, fVisible, fInteresting;
        bool                fNotifyOnInteresting;
        pfGUIDialogMod      *fDialog;

        hsBounds3           fBounds, fInitialBounds;        // Z component is 0-1
        float            fScreenMinZ;    // Closest Z coordinate in screen space
        hsPoint3            fScreenCenter;
        bool                fBoundsValid, fCenterValid;
        hsMatrix44          fXformMatrix;   // Only used for doing drag work, etc.

        pfGUICtrlProcObject *fHandler;
        pfGUIDropTargetProc *fDropTargetHdlr;

        plDynamicTextMap    *fDynTextMap;   // Some controls use this; for others, it'll be nil
        plLayerInterface    *fDynTextLayer; // Juse so we can reset the transform. Sheesh!

        pfGUIColorScheme    *fColorScheme;
        plSceneObject       *fProxy;

        hsTArray<hsPoint3>  fBoundsPoints;      // For more accurate bounds tests

        hsTArray<int>   fSoundIndices;  // Indices of sounds to trigger on the target SO's audible interface

        pfGUISkin       *fSkin;


        bool            ISetUpDynTextMap( plPipeline *pipe );
        virtual void    IPostSetUpDynTextMap( void ) {}
        virtual void    IGrowDTMDimsToDesiredSize( uint16_t &width, uint16_t &height ) { }

        virtual bool    IEval( double secs, float del, uint32_t dirty ); // called only by owner object's Eval()

        void            ISetDialog( pfGUIDialogMod *mod ) { fDialog = mod; }
        void            IScreenToLocalPt( hsPoint3 &pt );

        virtual void    IUpdate( void ) {;}
        void            ISetHandler( pfGUICtrlProcObject *h, bool clearInheritFlag = false );

        void            IPlaySound( uint8_t guiCtrlEvent, bool loop = false );
        void            IStopSound( uint8_t guiCtrlEvent );

        virtual uint32_t      IGetDesiredCursor( void ) const { return 0; }   // As specified in plInputInterface.h

    public:

        pfGUIControlMod();
        virtual ~pfGUIControlMod();

        CLASSNAME_REGISTER( pfGUIControlMod );
        GETINTERFACE_ANY( pfGUIControlMod, plSingleModifier );


        virtual bool    MsgReceive( plMessage* pMsg );
        
        virtual void Read( hsStream* s, hsResMgr* mgr );
        virtual void Write( hsStream* s, hsResMgr* mgr );

        uint32_t      GetTagID( void ) { return fTagID; }

        virtual void    SetEnabled( bool e );
        virtual bool    IsEnabled( void ) { return fEnabled; }
        virtual void    SetFocused( bool e );
        virtual bool    IsFocused( void ) { return fFocused; }
        virtual void    SetVisible( bool vis );
        virtual bool    IsVisible( void ) { return fVisible; }

        virtual void    SetInteresting( bool i );
        bool            IsInteresting( void ) { return fInteresting; }

        virtual void    SetNotifyOnInteresting( bool state ) { fNotifyOnInteresting = state; }

        pfGUIDialogMod  *GetOwnerDlg( void ) { return fDialog; }
        
        virtual void    Refresh( void );

        virtual void    UpdateBounds( hsMatrix44 *invXformMatrix = nil, bool force = false );
        void            SetObjectCenter( float x, float y );
        virtual hsPoint3 GetObjectCenter() { return fScreenCenter; }
        float        GetScreenMinZ( void ) { return fScreenMinZ; }
        void            CalcInitialBounds( void );

        const hsBounds3 &GetBounds( void );
        bool            PointInBounds( const hsPoint3 &point );

        virtual void    SetTarget( plSceneObject *object );

        // Return false if you actually DON'T want the mouse clicked at this point (should only be used for non-rectangular region rejection)
        virtual bool    FilterMousePosition( hsPoint3 &mousePt ) { return true; }

        virtual void    HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers ) {;}
        virtual void    HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers ) {;}
        virtual void    HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers ) {;}
        virtual void    HandleMouseHover( hsPoint3 &mousePt, uint8_t modifiers ) {;}
        virtual void    HandleMouseDblClick( hsPoint3 &mousePt, uint8_t modifiers ) {;}

        virtual bool    HandleKeyPress( wchar_t key, uint8_t modifiers );
        virtual bool    HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers );

        void            SetHandler( pfGUICtrlProcObject *h ) { ISetHandler( h, true ); }
        void            DoSomething( void );                        // Will call the handler
        void            HandleExtendedEvent( uint32_t event );        // Will call the handler

        pfGUICtrlProcObject *GetHandler( void ) const { return fHandler; }

        void                    SetDropTargetHdlr( pfGUIDropTargetProc *drop );
        pfGUIDropTargetProc     *GetDropTargetHdlr( void ) { return fDropTargetHdlr; }

        enum
        {
            kRefDynTextMap,
            kRefDynTextLayer,
            kRefProxy,
            kRefSkin,
            kRefDerivedStart    = 32
        };

        enum Flags  // plSingleModifier already has SetFlag()/HasFlag()
        {
            kWantsInterest,
            kInheritProcFromDlg,
            kIntangible,        // I.E. it doesn't exists on the screen/can't be clicked on.
                                // Used for group objects like the up/down pair
            kXparentBgnd,
            kScaleTextWithResolution,       // I.E. take up the same space on screen no matter the resolution
            kTakesSpecialKeys,              // I.E. disable bindings for keys like backspace because we want them
            kHasProxy,
            kBetterHitTesting,
            kDerivedFlagsStart = 32
        };

        virtual void        SetColorScheme( pfGUIColorScheme *newScheme );
        pfGUIColorScheme    *GetColorScheme( void ) const;

        virtual void    UpdateColorScheme() { IPostSetUpDynTextMap(); IUpdate(); }

        // should be override by specific GUIcontrol
        virtual void        PurgeDynaTextMapImage() {;}

        // Override from plModifier so we can update our bounds
        virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

        // Forces an immediate play of the given GUI control event sound
        void    PlaySound( uint8_t guiCtrlEvent, bool loop = false ) { IPlaySound( guiCtrlEvent, loop ); }
        void    StopSound( uint8_t guiCtrlEvent ) { IStopSound( guiCtrlEvent ); }

        // Export only
        void        SetTagID( uint32_t id ) { fTagID = id; }
        void        SetDynTextMap( plLayerInterface *layer, plDynamicTextMap *dynText );
        void        SetSoundIndex( uint8_t guiCtrlEvent, int soundIndex );
};

#endif // _pfGUIControlMod_h
