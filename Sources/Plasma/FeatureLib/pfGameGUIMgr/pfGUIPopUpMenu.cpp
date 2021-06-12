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

#include "pfGameGUIMgr.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "plViewTransform.h"

#include <string_theory/format>

#include "pfGUIButtonMod.h"
#include "pfGUIControlHandlers.h"
#include "pfGUICtrlGenerator.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIDialogNotifyProc.h"
#include "pfGUIMenuItem.h"
#include "pfGUIPopUpMenu.h"

#include "pnMessage/plClientMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plGImage/plDynamicTextMap.h"
#include "plMessage/plLayRefMsg.h"
#include "plPipeline/plDebugText.h"
#include "plScene/plPostEffectMod.h"
#include "plScene/plSceneNode.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"

class pfPopUpKeyGenerator
{
    public:
        ST::string  fPrefix;
        uint32_t    fKeyCount;
        plLocation  fLoc;

        pfPopUpKeyGenerator(ST::string p, const plLocation &loc)
            : fPrefix(std::move(p)), fKeyCount(), fLoc(loc)
        { }

        plKey   CreateKey( hsKeyedObject *ko )
        {
            ST::string name = ST::format("{}-{}", fPrefix, fKeyCount++);

            return hsgResMgr::ResMgr()->NewKey( name, ko, fLoc );
        }
};

//// Router Proc So The Parent Can Handle Click Events ///////////////////////

class pfGUIMenuItemProc : public pfGUICtrlProcObject
{
    protected:

        pfGUIPopUpMenu      *fParent;
        uint32_t              fIndex;

    public:

        pfGUIMenuItemProc( pfGUIPopUpMenu *parent, uint32_t idx )
        {
            fParent = parent;
            fIndex = idx;
        }

        void    DoSomething(pfGUIControlMod *ctrl) override
        {
            fParent->IHandleMenuSomething( fIndex, ctrl );
        }

        void    HandleExtendedEvent(pfGUIControlMod *ctrl, uint32_t event) override
        {
            fParent->IHandleMenuSomething( fIndex, ctrl, (int32_t)event );
        }
};

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIPopUpMenu::pfGUIPopUpMenu()
    : fNeedsRebuilding(), fParent(), fKeyGen(), fSubMenuOpen(-1),
      fMargin(4), fSkin(), fWaitingForSkin(), fParentNode(),
      fOriginX(), fOriginY(), fOriginAnchor(), fOriginContext(),
      fAlignment(kAlignDownRight)
{
    SetFlag( kModalOutsideMenus );
}

pfGUIPopUpMenu::~pfGUIPopUpMenu()
{
    SetSkin(nullptr);

//  if (fParentNode != nullptr)
//      fParentNode->GetKey()->UnRefObject();

    ITearDownMenu();
    ClearItems();

    delete fKeyGen;
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIPopUpMenu::MsgReceive( plMessage *msg )
{
    plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( msg );
    if (ref != nullptr)
    {
        if( ref->fType == kRefSkin )
        {
            if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                fSkin = pfGUISkin::ConvertNoRef( ref->GetRef() );
                fWaitingForSkin = false;
            }
            else
                fSkin = nullptr;

            fNeedsRebuilding = true;
            if( IsVisible() )
            {
                // Rebuild NOW
                ITearDownMenu();
                IBuildMenu();
            }
            return true;
        }
        else if( ref->fType == kRefSubMenu )
        {
            if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fMenuItems[ ref->fWhich ].fSubMenu = pfGUIPopUpMenu::ConvertNoRef( ref->GetRef() );
            else
                fMenuItems[ref->fWhich].fSubMenu = nullptr;
            return true;
        }
        else if( ref->fType == kRefOriginAnchor )
        {
            if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fOriginAnchor = plSceneObject::ConvertNoRef( ref->GetRef() );
            else
                fOriginAnchor = nullptr;
            return true;
        }
        else if( ref->fType == kRefOriginContext )
        {
            if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fOriginContext = pfGUIDialogMod::ConvertNoRef( ref->GetRef() );
            else
                fOriginContext = nullptr;
            return true;
        }
        else if( ref->fType == kRefParentNode )
        {
            if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fParentNode = plSceneNode::ConvertNoRef( ref->GetRef() );
            else
                fParentNode = nullptr;
            return true;
        }
    }

    return pfGUIDialogMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIPopUpMenu::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIDialogMod::Read( s, mgr );

    // In case we need it...
    fKeyGen = new pfPopUpKeyGenerator( GetName(), GetKey()->GetUoid().GetLocation() );

    fOriginX = fOriginY = -1.f;

    fMargin = s->ReadLE16();

    uint32_t count = s->ReadLE32();
    fMenuItems.resize(count);
    for (uint32_t i = 0; i < count; i++)
    {
        char readTemp[ 256 ];
        s->Read( sizeof( readTemp ), readTemp );
        wchar_t *wReadTemp = hsStringToWString( readTemp );
        fMenuItems[ i ].fName = wReadTemp;
        delete [] wReadTemp;
        
        fMenuItems[ i ].fHandler = pfGUICtrlProcWriteableObject::Read( s );

        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefSubMenu ), plRefFlags::kActiveRef );
    }

    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSkin ), plRefFlags::kActiveRef );
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginAnchor ), plRefFlags::kPassiveRef );
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginContext ), plRefFlags::kPassiveRef );

    fAlignment = (Alignment)s->ReadByte();

    fNeedsRebuilding = true;
}

void    pfGUIPopUpMenu::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIDialogMod::Write( s, mgr );


    s->WriteLE16( fMargin );

    s->WriteLE32((uint32_t)fMenuItems.size());
    for (const pfMenuItem& item : fMenuItems)
    {
        char writeTemp[ 256 ];
        char *sName = hsWStringToString(item.fName.c_str());
        strncpy( writeTemp, sName, sizeof( writeTemp ) );
        delete [] sName;
        s->Write( sizeof( writeTemp ), writeTemp );

        // Write the handler out (if it's not a writeable, damn you)
        pfGUICtrlProcWriteableObject::Write((pfGUICtrlProcWriteableObject *)item.fHandler, s);

        mgr->WriteKey(s, item.fSubMenu);
    }

    // Note: we force parentNode to nil here because we only use it when we dynamically
    // create nodes at runtime and need to unref and destroy them later. Since we're
    // reading from disk, we'll already have a sceneNode somewhere, so we don't need
    // this.
    fParentNode = nullptr;

    mgr->WriteKey( s, fSkin );
    mgr->WriteKey( s, fOriginAnchor );
    mgr->WriteKey( s, fOriginContext );

    s->WriteByte( (uint8_t)fAlignment );
}

void    pfGUIPopUpMenu::SetOriginAnchor( plSceneObject *anchor, pfGUIDialogMod *context )
{
    fOriginAnchor = anchor; 
    fOriginContext = context; 
    hsgResMgr::ResMgr()->AddViaNotify( fOriginAnchor->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginAnchor ), plRefFlags::kPassiveRef );
    hsgResMgr::ResMgr()->AddViaNotify( fOriginContext->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginContext ), plRefFlags::kPassiveRef );
}

//// SetEnabled //////////////////////////////////////////////////////////////

void    pfGUIPopUpMenu::SetEnabled( bool e )
{
    if( e && fNeedsRebuilding )
    {
        // Make sure our menu is rebuilt before enabling
        ITearDownMenu();
        IBuildMenu();
    }
    else if( !e )
    {
        if (fParent != nullptr)
            pfGUIPopUpMenu::ConvertNoRef( fParent )->fSubMenuOpen = -1;

        // Hide our submenus if we have any open
        if( fSubMenuOpen != -1 )
        {
            fMenuItems[ fSubMenuOpen ].fSubMenu->Hide();
            fSubMenuOpen = -1;
        }
    }

    pfGUIDialogMod::SetEnabled( e );
}

void    pfGUIPopUpMenu::Show( float x, float y )
{
    fOriginX = x;
    fOriginY = y;
    pfGUIDialogMod::Show(); // C++ is kinda stupid if it can't find this naturally
    ISeekToOrigin();
}

void    pfGUIPopUpMenu::ISeekToOrigin()
{
#if 0
    uint32_t i;
    float   x = 0.5f/*fOriginX*/, y = fOriginY;

    for (pfGUIControlMod* ctrl : fControls)
    {
        if (ctrl != nullptr)
        {
            ctrl->SetObjectCenter( x, y );

//          const hsBounds3 &bnds = ctrl->GetBounds();
            y += fMenuItems[ i ].fYOffsetToNext;//bnds.GetMaxs().fY - bnds.GetMins().fY;

/*          hsMatrix44 p2l, l2p = GetTarget()->GetLocalToWorld();

            hsPoint3 center, origin;
            ScreenToWorldPoint( fOriginX, fOriginY, 100.f, center );
            ScreenToWorldPoint( 0.f, 0.f, 100.f, origin );

            center = origin - center;

            center.fZ = 0.f;
            l2p.SetTranslate( &center );
            l2p.GetInverse( &p2l );

            GetTarget()->SetTransform( l2p, p2l );
*/      }
    }
#endif
}

//// IHandleMenuSomething ////////////////////////////////////////////////////
//  Handles a normal event from one of the item controls.

void    pfGUIPopUpMenu::IHandleMenuSomething( uint32_t idx, pfGUIControlMod *ctrl, int32_t extended )
{
    if( extended != -1 )
    {
        if( fSubMenuOpen != -1 && fSubMenuOpen != idx )
        {
            // Better close the submenu(s)
            fMenuItems[ fSubMenuOpen ].fSubMenu->Hide();
            fSubMenuOpen = -1;
        }

        if (extended == pfGUIMenuItem::kMouseHover && fMenuItems[idx].fSubMenu != nullptr)
        {
            // Open new submenu
            const hsBounds3 &bnds = ctrl->GetBounds();
            fMenuItems[ idx ].fSubMenu->Show( bnds.GetMaxs().fX, bnds.GetMins().fY );
            fSubMenuOpen = idx;
        }
    }
    else
    {
        if (fMenuItems[idx].fHandler != nullptr)
            fMenuItems[ idx ].fHandler->DoSomething( ctrl );
        
        // If item isn't a sub-menu, close this menu. Else add to list of menus to close
        // once the smallest submenu goes away
        if (fMenuItems[idx].fSubMenu == nullptr)
        {
            // Basically, we want to hide ourselves and as many up in the chain of command as
            // can be hidden
            pfGUIPopUpMenu *menu = this;
            while (menu != nullptr && !menu->HasFlag(kStayOpenAfterClick))
            {
                menu->Hide();
                menu = pfGUIPopUpMenu::ConvertNoRef( menu->fParent );
            }
        }
        else
        {
            // Show relative to the corner of our current item
            const hsBounds3 &bnds = ctrl->GetBounds();
            fMenuItems[ idx ].fSubMenu->Show( bnds.GetMaxs().fX, bnds.GetMins().fY );
            fSubMenuOpen = idx;
        }
    }
}

//// IBuildMenu //////////////////////////////////////////////////////////////
//  Given the list of menu items, builds our set of dynamic buttons

bool    pfGUIPopUpMenu::IBuildMenu()
{
    if (fWaitingForSkin && fSkin == nullptr)
        return false;       // Still waiting to get our skin before building

    pfGUIColorScheme *scheme = new pfGUIColorScheme();
    scheme->fForeColor.Set( 0, 0, 0, 1 );
    scheme->fBackColor.Set( 1, 1, 1, 1 );

    // If we don't have origin points, get them from translating our anchor
    if (fOriginX == -1 || fOriginY == -1 && fOriginAnchor != nullptr)
    {
        hsPoint3 scrnPt;
        const plDrawInterface *di = fOriginAnchor->GetDrawInterface();
        if (di != nullptr)
        {
            scrnPt = di->GetLocalBounds().GetCenter();
            scrnPt = fOriginAnchor->GetLocalToWorld() * scrnPt;
        }
        else
            scrnPt = fOriginAnchor->GetLocalToWorld().GetTranslate();
        if (fOriginContext != nullptr)
            scrnPt = fOriginContext->WorldToScreenPoint( scrnPt );
        else
            scrnPt = WorldToScreenPoint( scrnPt );

        if( fOriginX == -1 )
            fOriginX = scrnPt.fX;
        if( fOriginY == -1 )
            fOriginY = scrnPt.fY;
    }

    float x = fOriginX, y = fOriginY;
    float width = 0.f, height = 0.f;
    float topMargin = (fSkin != nullptr) ? fSkin->GetBorderMargin() : 0.f;
    
    // First step: loop through and calculate the size of our menu
    // The PROBLEM is that we can't do that unless we have a friggin surface on
    // which to calculate the text extents! So sadly, we're going to have to create
    // a whole new DTMap and use it to calculate some stuff
    plDynamicTextMap *scratch = new plDynamicTextMap( 8, 8, false );
    scratch->SetFont( scheme->fFontFace, scheme->fFontSize, scheme->fFontFlags, true );
    for (const pfMenuItem& item : fMenuItems)
    {
        uint16_t  thisW, thisH;
        thisW = scratch->CalcStringWidth(item.fName.c_str(), &thisH);
        if (item.fSubMenu != nullptr)
        {
            if (fSkin != nullptr)
                thisW += 4 + ( fSkin->GetElement( pfGUISkin::kSubMenuArrow ).fWidth << 1 );
            else
                thisW += scratch->CalcStringWidth(" >>", nullptr);
        }
        thisH += 2; // Give us at least one pixel on each side

        int margin = fMargin;
        if (fSkin != nullptr)
            margin = fSkin->GetBorderMargin() << 1;

        if( width < thisW + margin )
            width = (float)(thisW + margin);

        if (fSkin != nullptr)
            margin = fSkin->GetItemMargin() << 1;

        if( height < thisH + margin )
            height = (float)(thisH + margin);
    }
    delete scratch;

    width += 4; // give us a little space, just in case

    uint32_t scrnWidth, scrnHeight;
    // A cheat here, I know, but I'm lazy
    plDebugText::Instance().GetScreenSize( &scrnWidth, &scrnHeight );

    // Use the same base res calc that dtMaps use
    if( !HasFlag( kScaleWithResolution ) )
    {
        // Just use what we were passed in
    }
    else
    {
        // Scale with the resolution so that we take up the same % of screen space no matter what resolution
        // Assume a base "resolution" of 1024xX, where X is such that the ratio "1024/X = scrnWidth/scrnHt" holds
        const int kBaseScaleRes = 1024;
        scrnHeight = ( scrnHeight * kBaseScaleRes ) / scrnWidth;
        scrnWidth = kBaseScaleRes;
    }

    width /= (float)scrnWidth;
    height /= (float)scrnHeight;
    topMargin /= (float)scrnHeight;

    switch( fAlignment )
    {
        case kAlignUpLeft:
            x -= width;
            y -= height * fMenuItems.size();
            break;
        case kAlignUpRight:
            y -= height * fMenuItems.size();
            break;
        case kAlignDownLeft:
            x -= width;
            break;
        case kAlignDownRight:
            break;
    }

    if (y + height * fMenuItems.size() > 1.f)
    {
        // Make sure we don't go off the bottom
        y = 1.f - height * fMenuItems.size();
    }
    // And the top (takes precedence)
    if( y < 0.f )
        y = 0.f;

    // Control positions are in the lower left corner, so increment Y by 1 control height first
    y += height;// + topMargin;

    std::vector<pfGUIPopUpMenu *> buildList;

    for (size_t i = 0; i < fMenuItems.size(); i++)
    {
        hsGMaterial *mat = ICreateDynMaterial();

        float thisMargin = (i == 0 || i == fMenuItems.size() - 1) ? topMargin : 0.f;
        float thisOffset = (i == fMenuItems.size() - 1) ? topMargin : 0.f;

        pfGUIMenuItem *button = pfGUIMenuItem::ConvertNoRef( pfGUICtrlGenerator::Instance().CreateRectButton( this, fMenuItems[ i ].fName.c_str(), x, y + thisOffset, width, height + thisMargin, mat, true ) );
        if (button != nullptr)
        {
            button->SetColorScheme( scheme );
            button->SetName( fMenuItems[ i ].fName.c_str() );
            button->SetHandler( new pfGUIMenuItemProc( this, i ) );
            // make the tag ID the position in the menu list
            button->SetTagID(i);
            button->SetDynTextMap( mat->GetLayer( 0 ), plDynamicTextMap::ConvertNoRef( mat->GetLayer( 0 )->GetTexture() ) );
            button->SetFlag( pfGUIMenuItem::kReportHovers );
            button->SetSkin(fSkin, (i == 0) ? pfGUIMenuItem::kTop
                                 : (i == fMenuItems.size() - 1) ? pfGUIMenuItem::kBottom
                                 : pfGUIMenuItem::kMiddle);
            if (fMenuItems[i].fSubMenu != nullptr)
            {
                button->SetFlag( pfGUIMenuItem::kDrawSubMenuArrow );
                buildList.emplace_back(pfGUIPopUpMenu::ConvertNoRef(fMenuItems[i].fSubMenu));
            }
        }

        // Tiny bit of overlap to prevent gaps
        fMenuItems[ i ].fYOffsetToNext = height + thisOffset;
        y += height + thisOffset;// - ( 1.f / kBaseScaleResY );
    }

    fNeedsRebuilding = false;

#if 0
    // Finally, go down our list of submenus and rebuild them, since they'll need to be rebuilt soon anyway,
    // and at least this way it's all in one pass

    // Also, we need to bump the tag ID used, such as adding parent menuItem TagID * 100.. or something

    // Disabled because right now we can't move menus, which is required for this to work
    for (pfGUIPopUpMenu* buildMenu : buildList)
        buildMenu->IBuildMenu();
#endif

    return true;
}

//// ITearDownMenu ///////////////////////////////////////////////////////////
//  Destroys all of our dynamic controls representing the menu

void    pfGUIPopUpMenu::ITearDownMenu()
{
    for (pfGUIControlMod* ctrl : fControls)
    {
        if (ctrl != nullptr)
        {
            // It's not enough to release the key, we have to have the sceneNode release the key, too.
            // Easy enough to do by just setting it's sn to nil
            if (ctrl->GetTarget() != nullptr)
                ctrl->GetTarget()->SetSceneNode(nullptr);

            // Now release it from us
            GetKey()->Release(ctrl->GetKey());
        }
    }

    fNeedsRebuilding = true;
}

//// HandleMouseEvent ////////////////////////////////////////////////////////

bool        pfGUIPopUpMenu::HandleMouseEvent( pfGameGUIMgr::EventType event, float mouseX, float mouseY,
                                                uint8_t modifiers )
{
    bool r = pfGUIDialogMod::HandleMouseEvent( event, mouseX, mouseY, modifiers );
    if( r == false && event == pfGameGUIMgr::kMouseUp )
    {
        // We don't want to be active anymore!
        if( !HasFlag( kStayOpenAfterClick ) )
        {
            Hide();

            // Now we pass the click to our parent. Why? Because it's possible that someone above us
            // will either a) also want to hide (cancel the entire menu selection) or b) select
            // another option
            if (fParent != nullptr)
                return fParent->HandleMouseEvent( event, mouseX, mouseY, modifiers );
        }
    }

    return (fParent != nullptr) ? r : (HasFlag(kModalOutsideMenus) || (fSubMenuOpen != -1));
}

//// ClearItems //////////////////////////////////////////////////////////////
//  Clears the list of template items

void    pfGUIPopUpMenu::ClearItems()
{
    for (const pfMenuItem& item : fMenuItems)
    {
        if (item.fHandler != nullptr)
        {
            if (item.fHandler->DecRef())
                delete item.fHandler;
        }
    }

    fMenuItems.clear();

    fNeedsRebuilding = true;
}

//// AddItem /////////////////////////////////////////////////////////////////
//  Append a new item to the list of things to build the menu from 

void    pfGUIPopUpMenu::AddItem( const char *name, pfGUICtrlProcObject *handler, pfGUIPopUpMenu *subMenu )
{
    wchar_t *wName = hsStringToWString(name);
    AddItem(wName,handler,subMenu);
    delete [] wName;
}

void    pfGUIPopUpMenu::AddItem( const wchar_t *name, pfGUICtrlProcObject *handler, pfGUIPopUpMenu *subMenu )
{
    pfMenuItem  &newItem = fMenuItems.emplace_back();

    newItem.fName = name;
    newItem.fHandler = handler;
    if (newItem.fHandler != nullptr)
        newItem.fHandler->IncRef();
    newItem.fSubMenu = subMenu;

    if (subMenu != nullptr)
        subMenu->fParent = this;

    fNeedsRebuilding = true;
}

//// ICreateDynMaterial //////////////////////////////////////////////////////
//  Creates the hsGMaterial tree for a single layer with a plDynamicTextMap.

hsGMaterial *pfGUIPopUpMenu::ICreateDynMaterial()
{
    hsColorRGBA     black, white;

    
    // Create the new dynTextMap
    plDynamicTextMap    *textMap = new plDynamicTextMap();
    fKeyGen->CreateKey( textMap );

    // Create the material
    hsGMaterial *material = new hsGMaterial;
    fKeyGen->CreateKey( material );

    // Create the layer and attach
    plLayer *lay = material->MakeBaseLayer();
    white.Set( 1.f,1.f,1.f,1.f );
    black.Set( 0.f,0.f,0.f,1.f );

    lay->SetRuntimeColor( black );
    lay->SetPreshadeColor( black );
    lay->SetAmbientColor( white );
    lay->SetClampFlags( hsGMatState::kClampTexture );

    // Do sendRef here, since we're going to need it set pretty darned quick
    hsgResMgr::ResMgr()->SendRef( textMap->GetKey(), new plLayRefMsg( lay->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );

    return material;

}

//// Build ///////////////////////////////////////////////////////////////////
//  Constructs a shiny new pop-up menu at runtime, complete with trimmings

#include "plGImage/plJPEG.h"

pfGUIPopUpMenu  *pfGUIPopUpMenu::Build( const char *name, pfGUIDialogMod *parent, float x, float y, const plLocation &destLoc )
{
    float           fovX, fovY;
    

    // Create the menu and give it a key gen
    pfGUIPopUpMenu  *menu = new pfGUIPopUpMenu();
    menu->fKeyGen = new pfPopUpKeyGenerator( name, destLoc );
    menu->fKeyGen->CreateKey( menu );

    menu->fOriginX = x;
    menu->fOriginY = y;

    // By default, share the same skin as the parent
    if (parent != nullptr && ((pfGUIPopUpMenu *)parent)->fSkin != nullptr)
    {
        menu->fWaitingForSkin = true;
        hsgResMgr::ResMgr()->SendRef( ( (pfGUIPopUpMenu *)parent )->fSkin->GetKey(), new plGenRefMsg( menu->GetKey(), plRefMsg::kOnCreate, -1, pfGUIPopUpMenu::kRefSkin ), plRefFlags::kActiveRef );
    }

    // HACK for now: create us a temp skin to use
/*  static pfGUISkin *skin = nullptr;
    if (skin == nullptr)
    {
        plLocation loc;
        loc.Set( 0x1425 );
        plKey skinKey = hsgResMgr::ResMgr()->FindKey( plUoid( loc, pfGUISkin::Index(), "GUISkin01_GUISkin" ) );
        menu->fWaitingForSkin = true;
        hsgResMgr::ResMgr()->AddViaNotify( skinKey, new plGenRefMsg( menu->GetKey(), plRefMsg::kOnCreate, -1, pfGUIPopUpMenu::kRefSkin ), plRefFlags::kActiveRef );
    }
*/

    // Create the rendermod
    plPostEffectMod *renderMod = new plPostEffectMod;
    menu->fKeyGen->CreateKey( renderMod );

    renderMod->SetHither( 0.5f );
    renderMod->SetYon( 200.f );

    float scrnWidth = 20.f;

    // fovX should be such that scrnWidth is the projected width at z=100
    fovX = atan( scrnWidth / ( 2.f * 100.f ) ) * 2.f;
    fovY = fovX;// * 3.f / 4.f;

    renderMod->SetFovX(hsRadiansToDegrees(fovX));
    renderMod->SetFovY(hsRadiansToDegrees(fovY));

    // Create the sceneNode to go with it
    menu->fParentNode= new plSceneNode;
    menu->fKeyGen->CreateKey( menu->fParentNode );
//  menu->fParentNode->GetKey()->RefObject();
    hsgResMgr::ResMgr()->SendRef( menu->fParentNode->GetKey(), new plGenRefMsg( menu->GetKey(), plRefMsg::kOnCreate, 0, kRefParentNode ), plRefFlags::kActiveRef );     

    hsgResMgr::ResMgr()->AddViaNotify( menu->fParentNode->GetKey(), new plGenRefMsg( renderMod->GetKey(), plRefMsg::kOnCreate, 0, plPostEffectMod::kNodeRef ), plRefFlags::kPassiveRef );       

    menu->SetRenderMod( renderMod );
    menu->SetName( name );

    // Create the dummy scene object to hold the menu
    plSceneObject   *newObj = new plSceneObject;
    menu->fKeyGen->CreateKey( newObj );

    // *#&$(*@&#$ need a coordIface...
    plCoordinateInterface *newCI = new plCoordinateInterface;
    menu->fKeyGen->CreateKey( newCI );

    hsMatrix44 l2w, w2l;
    l2w.Reset();
    l2w.GetInverse( &w2l );

    // Using SendRef here because AddViaNotify will queue the messages up, which doesn't do us any good
    // if we need these refs right away
    hsgResMgr::ResMgr()->SendRef( newCI->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );      
    hsgResMgr::ResMgr()->SendRef( renderMod->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );       
    newObj->SetSceneNode( menu->fParentNode->GetKey() );
    newObj->SetTransform( l2w, w2l );

    hsgResMgr::ResMgr()->SendRef( menu->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );        

    // Add the menu to the GUI mgr
    plGenRefMsg *refMsg = new plGenRefMsg( pfGameGUIMgr::GetInstance()->GetKey(), 
                                            plRefMsg::kOnCreate, 0, pfGameGUIMgr::kDlgModRef );
    hsgResMgr::ResMgr()->AddViaNotify( menu->GetKey(), refMsg, plRefFlags::kActiveRef );        

    menu->ISeekToOrigin();

    return menu;
}

//// SetSkin /////////////////////////////////////////////////////////////////

void    pfGUIPopUpMenu::SetSkin( pfGUISkin *skin )
{
    // Just a function wrapper for SendRef
    if (fSkin != nullptr)
        GetKey()->Release( fSkin->GetKey() );

    if (skin != nullptr)
    {
        hsgResMgr::ResMgr()->SendRef( skin->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSkin ), plRefFlags::kActiveRef );
        fWaitingForSkin = true;
    }
    else
        fWaitingForSkin = false;
}


//////////////////////////////////////////////////////////////////////////////
//// pfGUISkin Implementation ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

pfGUISkin::pfGUISkin()
    : fTexture(), fItemMargin(), fBorderMargin()
{
    memset( fElements, 0, sizeof( pfSRect ) * kNumElements );
}

pfGUISkin::pfGUISkin(plMipmap* texture)
    : fTexture(texture), fItemMargin(), fBorderMargin()
{
    if (fTexture != nullptr)
    {
        hsAssert(fTexture->GetKey() != nullptr, "Creating a GUI skin via a mipmap with no key!");
        fTexture->GetKey()->RefObject();
    }
    memset( fElements, 0, sizeof( pfSRect ) * kNumElements );
}

pfGUISkin::~pfGUISkin()
{
    SetTexture(nullptr);
}

void    pfGUISkin::SetTexture( plMipmap *tex )
{
    if (fTexture != nullptr && fTexture->GetKey() != nullptr)
        fTexture->GetKey()->UnRefObject();

    fTexture = tex;
    if (fTexture != nullptr)
    {
        hsAssert(fTexture->GetKey() != nullptr, "Creating a GUI skin via a mipmap with no key!");
        fTexture->GetKey()->RefObject();
    }
}

void    pfGUISkin::SetElement( uint32_t idx, uint16_t x, uint16_t y, uint16_t w, uint16_t h )
{
    fElements[ idx ].fX = x;
    fElements[ idx ].fY = y;
    fElements[ idx ].fWidth = w;
    fElements[ idx ].fHeight = h;
}

void    pfGUISkin::Read( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Read( s, mgr );

    s->ReadLE16(&fItemMargin);
    s->ReadLE16(&fBorderMargin);

    uint32_t count = s->ReadLE32();

    for (uint32_t i = 0; i < count; i++)
        fElements[ i ].Read( s );

    for (uint32_t i = count; i < kNumElements; i++)
        fElements[ i ].Empty();

    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefMipmap ), plRefFlags::kActiveRef );
}

void    pfGUISkin::Write( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Write( s, mgr );

    s->WriteLE16(fItemMargin);
    s->WriteLE16(fBorderMargin);

    s->WriteLE32((uint32_t)kNumElements);

    for (uint32_t i = 0; i < kNumElements; i++)
        fElements[ i ].Write( s );

    mgr->WriteKey( s, fTexture );
}

bool    pfGUISkin::MsgReceive( plMessage *msg )
{
    plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( msg );
    if (ref != nullptr)
    {
        if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            fTexture = plMipmap::ConvertNoRef( ref->GetRef() );
        else
            fTexture = nullptr;

        return true;
    }

    return hsKeyedObject::MsgReceive( msg );
}

void    pfGUISkin::pfSRect::Read( hsStream *s )
{
    s->ReadLE16(&fX);
    s->ReadLE16(&fY);
    s->ReadLE16(&fWidth);
    s->ReadLE16(&fHeight);
}

void    pfGUISkin::pfSRect::Write( hsStream *s )
{
    s->WriteLE16(fX);
    s->WriteLE16(fY);
    s->WriteLE16(fWidth);
    s->WriteLE16(fHeight);
}
