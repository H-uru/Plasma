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
//  pfGUIPopUpMenu Header                                                   //
//                                                                          //
//  Pop-up menus are really just dialogs that know how to create themselves //
//  and create buttons on themselves to simulate a menu (after all, that's  //
//  all a menu really is anyway).                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIPopUpMenu_h
#define _pfGUIPopUpMenu_h

#include "hsBounds.h"

#include <string>

#include "pfGUIDialogMod.h"

class hsGMaterial;
class pfGUIButtonMod;
class pfGUICtrlProcObject;
class pfGUIMenuItemProc;
class pfGUISkin;
class plMessage;
class plMipmap;
class pfPopUpKeyGenerator;
class plSceneNode;

class pfGUIPopUpMenu : public pfGUIDialogMod
{
    public:

        enum Alignment
        {
            kAlignUpLeft,
            kAlignUpRight,
            kAlignDownLeft,
            kAlignDownRight     // Default
        };

    protected:

        friend class pfGUIMenuItemProc;

        pfGUIDialogMod      *fParent;   // Pop-up menus also have a sense of who owns them
        plSceneNode         *fParentNode;

        pfPopUpKeyGenerator *fKeyGen;   // Generates keys for our dynamic objects

        class pfMenuItem
        {
            // Simple wrapper class that tells us how to build our menu
            public:
                std::wstring        fName;
                pfGUICtrlProcObject *fHandler;
                pfGUIPopUpMenu      *fSubMenu;
                float               fYOffsetToNext;     // Filled in by IBuildMenu()

                pfMenuItem() : fHandler(), fSubMenu(), fYOffsetToNext() { }
        };

        // Array of info to rebuild our menu from. Note that this is ONLY used when rebuilding
        bool                    fNeedsRebuilding, fWaitingForSkin;
        float                   fOriginX, fOriginY;
        uint16_t                fMargin;
        std::vector<pfMenuItem> fMenuItems;
        int32_t                 fSubMenuOpen;

        pfGUISkin               *fSkin;

        plSceneObject           *fOriginAnchor;
        pfGUIDialogMod          *fOriginContext;

        Alignment               fAlignment;


        bool        IBuildMenu();
        void        ITearDownMenu();

        hsGMaterial *ICreateDynMaterial();

        void        IHandleMenuSomething( uint32_t idx, pfGUIControlMod *ctrl, int32_t extended = -1 );

        void        ISeekToOrigin();

    public:

        pfGUIPopUpMenu();
        virtual ~pfGUIPopUpMenu();

        CLASSNAME_REGISTER( pfGUIPopUpMenu );
        GETINTERFACE_ANY( pfGUIPopUpMenu, pfGUIDialogMod );

        enum MenuFlags
        {
            kStayOpenAfterClick = kDerivedFlagsStart,
            kModalOutsideMenus,
            kOpenSubMenusOnHover,
            kScaleWithResolution
        };

        enum Refs
        {
            kRefSkin = kRefDerviedStart,
            kRefSubMenu,
            kRefOriginAnchor,
            kRefOriginContext,
            kRefParentNode
        };

        bool    MsgReceive(plMessage* pMsg) override;
        
        void Read(hsStream* s, hsResMgr* mgr) override;
        void Write(hsStream* s, hsResMgr* mgr) override;

        void    SetEnabled(bool e) override;
        bool    HandleMouseEvent(pfGameGUIMgr::EventType event, float mouseX, float mouseY, uint8_t modifiers) override;

        void            Show( float x, float y );

        void    SetOriginAnchor( plSceneObject *anchor, pfGUIDialogMod *context );
        void    SetAlignment( Alignment a ) { fAlignment = a; }
        void    ClearItems();
        void    AddItem(const wchar_t *name, pfGUICtrlProcObject *handler, pfGUIPopUpMenu *subMenu = nullptr);
        void    SetSkin( pfGUISkin *skin );

        static pfGUIPopUpMenu   *Build( const char *name, pfGUIDialogMod *parent, float x, float y, const plLocation &destLoc = plLocation::kGlobalFixedLoc );

};

// Skin definition. Here for now 'cause only the menus use it, but might move it later
class pfGUISkin : public hsKeyedObject
{
    public:
        enum Elements
        {
            kUpLeftCorner = 0,
            kTopSpan,
            kUpRightCorner,
            kRightSpan,
            kLowerRightCorner,
            kBottomSpan,
            kLowerLeftCorner,
            kLeftSpan,
            kMiddleFill,
            kSelectedFill,
            kSubMenuArrow,
            kSelectedSubMenuArrow,
            kTreeButtonClosed,
            kTreeButtonOpen,
            kNumElements
        };

        class pfSRect
        {
            public:
                uint16_t  fX, fY, fWidth, fHeight;

                void    Empty() { fX = fY = fWidth = fHeight = 0; }
                void    Read( hsStream *s );
                void    Write( hsStream *s );
        };

    protected:

        plMipmap    *fTexture;
        pfSRect     fElements[ kNumElements ];
        uint16_t      fItemMargin, fBorderMargin;

    public:

        pfGUISkin();
        pfGUISkin( plMipmap *texture );
        virtual ~pfGUISkin();

        CLASSNAME_REGISTER( pfGUISkin );
        GETINTERFACE_ANY( pfGUISkin, hsKeyedObject );

        enum Refs
        {
            kRefMipmap
        };

        void    Read(hsStream *s, hsResMgr *mgr) override;
        void    Write(hsStream *s, hsResMgr *mgr) override;
        bool    MsgReceive(plMessage *msg) override;

        plMipmap        *GetTexture() const { return fTexture; }
        void            SetTexture( plMipmap *tex );

        const pfSRect   &GetElement( uint32_t idx ) const { return fElements[ idx ]; }
        bool            IsElementSet( uint32_t idx ) const { return ( fElements[ idx ].fWidth > 0 && fElements[ idx ].fHeight > 0 ); }
        void            SetElement( uint32_t idx, uint16_t x, uint16_t y, uint16_t w, uint16_t h );

        void            SetMargins( uint16_t item, uint16_t border ) { fItemMargin = item; fBorderMargin = border; }
        uint16_t          GetItemMargin() const { return fItemMargin; }
        uint16_t          GetBorderMargin() const { return fBorderMargin; }
};

#endif // _pfGUIPopUpMenu_h
