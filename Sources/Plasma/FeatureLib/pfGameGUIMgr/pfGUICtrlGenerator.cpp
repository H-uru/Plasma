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
//  pfGUICtrlGenerator Definitions                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUICtrlGenerator.h"

#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsResMgr.h"

#include <string_theory/format>

#include "pfGUIButtonMod.h"
#include "pfGUIDialogMod.h"
#include "pfGUIMenuItem.h"

#include "pnMessage/plAttachMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plDrawableGenerator.h"
#include "plSurface/hsGMaterial.h"

//// Constructor/Destructor //////////////////////////////////////////////////

//// Instance ////////////////////////////////////////////////////////////////

pfGUICtrlGenerator  &pfGUICtrlGenerator::Instance()
{
    static pfGUICtrlGenerator       myInstance;

    return myInstance;
}

//// IGetNextKeyName /////////////////////////////////////////////////////////

ST::string pfGUICtrlGenerator::IGetNextKeyName(const ST::string& prefix)
{
    static uint32_t keyCount = 0;

    return ST::format("{}{}", prefix, keyCount++);
}

//// IAddKey /////////////////////////////////////////////////////////////////

plKey pfGUICtrlGenerator::IAddKey(hsKeyedObject *ko, const ST::string& prefix)
{
    ST::string keyName;

    keyName = IGetNextKeyName( prefix );
    return hsgResMgr::ResMgr()->NewKey( keyName, ko, plLocation::kGlobalFixedLoc );
}

//// IGenSceneObject /////////////////////////////////////////////////////////

plSceneObject   *pfGUICtrlGenerator::IGenSceneObject( pfGUIDialogMod *dlg, plDrawable *myDraw, plSceneObject *parent,
                                                        hsMatrix44 *l2w, hsMatrix44 *w2l )
{
    plKey snKey = (dlg != nullptr) ? (dlg->GetTarget() != nullptr ? dlg->GetTarget()->GetSceneNode() : nullptr) : nullptr;

    hsgResMgr::ResMgr()->SendRef( myDraw->GetKey(), new plNodeRefMsg( snKey, plRefMsg::kOnCreate, 0, plNodeRefMsg::kDrawable ), plRefFlags::kActiveRef );       

    plDrawInterface *newDI = new plDrawInterface;
    IAddKey(newDI, ST_LITERAL("GUIDrawIFace"));

    plSceneObject   *newObj = new plSceneObject;
    IAddKey(newObj, ST_LITERAL("GUISceneObject"));

    plCoordinateInterface *newCI = new plCoordinateInterface;
    IAddKey(newCI, ST_LITERAL("GUICoordIFace"));

    hsgResMgr::ResMgr()->SendRef( newCI->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );
    hsgResMgr::ResMgr()->SendRef( newDI->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );
    hsgResMgr::ResMgr()->SendRef( myDraw->GetKey(), new plIntRefMsg( newDI->GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kDrawable ), plRefFlags::kActiveRef );

    if (parent == nullptr)
        parent = dlg->GetTarget();

    if (parent != nullptr)
//      hsgResMgr::ResMgr()->SendRef( newCI->GetKey(), new plIntRefMsg( parent->GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kChild ), plRefFlags::kActiveRef );
        hsgResMgr::ResMgr()->SendRef(newCI->GetKey(), new plAttachMsg(parent->GetKey(), nullptr, plRefMsg::kOnRequest), plRefFlags::kActiveRef);
    
    newObj->SetSceneNode( snKey );

    if (l2w != nullptr)
    {
        newObj->SetTransform( *l2w, *w2l );
//      newCI->SetLocalToParent( *l2w, *w2l );
//      myDraw->SetTransform( -1, *l2w, *w2l );
    }

    return newObj;
}

//// CreateRectButton ////////////////////////////////////////////////////////

pfGUIButtonMod  *pfGUICtrlGenerator::CreateRectButton( pfGUIDialogMod *parent, float x, float y, float width, float height,
                                    hsGMaterial *material, bool asMenuItem )
{
    plDrawableSpans *myDraw;
    hsMatrix44      l2w, w2l;


    // Translate x and y from (0:1) to (-10:10)
    x = ( x - 0.5f ) * 20.f;
    y = ( y - 0.5f ) * 20.f;
    // Translate width and height from (0:1) to (-10:10)
    width *= 20.f;
    height *= 20.f;

    // Create drawable that is rectangular
    l2w.Reset();
    hsPoint3 corner(x, -y, -100.f);
    hsVector3 xVec(width, 0.f, 0.f);
    hsVector3 yVec(0.f, height, 0.f);
    hsVector3 zVec(0.f, 0.f, 0.1f);

    myDraw = plDrawableGenerator::GeneratePlanarDrawable( corner, xVec, yVec, material, l2w );

    plSceneObject *newObj = IGenSceneObject( parent, myDraw );

    pfGUIButtonMod *newBtn = asMenuItem ? new pfGUIMenuItem : new pfGUIButtonMod;
    IAddKey(newBtn, ST_LITERAL("GUIButton"));
    hsgResMgr::ResMgr()->SendRef( newBtn->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );
    parent->AddControl( newBtn );
    hsgResMgr::ResMgr()->AddViaNotify( newBtn->GetKey(), new plGenRefMsg( parent->GetKey(), plRefMsg::kOnCreate, parent->GetNumControls() - 1, pfGUIDialogMod::kControlRef ), plRefFlags::kActiveRef );

    return newBtn;
}
