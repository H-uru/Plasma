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
//  pfGUICheckBoxCtrl Definition                                            //
//                                                                          //
//  Almost like buttons, only they keep their stated (pressed/unpressed)    //
//  when you click them, instead of reverting on mouse up.                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUICheckBoxCtrl.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plInputCore/plInputInterface.h"
#include "plMessage/plAnimCmdMsg.h"

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUICheckBoxCtrl::pfGUICheckBoxCtrl()
{
    SetFlag( kWantsInterest );
    fChecked = false;
    fClicking = false;
    fPlaySound = true;
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUICheckBoxCtrl::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUICheckBoxCtrl::MsgReceive( plMessage *msg )
{
    return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUICheckBoxCtrl::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Read(s, mgr);

    fAnimationKeys.clear();
    uint32_t count = s->ReadLE32();
    fAnimationKeys.reserve(count);
    for (uint32_t i = 0; i < count; i++)
        fAnimationKeys.emplace_back(mgr->ReadKey(s));

    fAnimName = s->ReadSafeString();
    fChecked = s->ReadBool();
}

void    pfGUICheckBoxCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Write( s, mgr );

    s->WriteLE32((uint32_t)fAnimationKeys.size());
    for (const plKey &key : fAnimationKeys)
        mgr->WriteKey(s, key);

    s->WriteSafeString( fAnimName );
    s->WriteBool( fChecked );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void    pfGUICheckBoxCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, bool force )
{
    pfGUIControlMod::UpdateBounds( invXformMatrix, force );
    if (!fAnimationKeys.empty())
        fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void    pfGUICheckBoxCtrl::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    fClicking = true;
    if(fPlaySound)
        IPlaySound( kMouseDown );
}

void    pfGUICheckBoxCtrl::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{
    if( fClicking )
    {
        fClicking = false;

        if(fPlaySound)
            IPlaySound( kMouseUp );

        // Don't run the command if the mouse is outside our bounds
        if( fBounds.IsInside( &mousePt ) )
        {
            SetChecked( !fChecked );
            DoSomething();
        }
    }
}

//// SetChecked //////////////////////////////////////////////////////////////

void    pfGUICheckBoxCtrl::SetChecked( bool checked, bool immediate /*= false*/ )
{
    fChecked = checked;
    if (!fAnimationKeys.empty())
    {
        plAnimCmdMsg *msg = new plAnimCmdMsg();
        if( fChecked )
        {
            // Moving to true
            if( immediate )
            {
                msg->SetCmd( plAnimCmdMsg::kGoToEnd );
            }
            else
            {
                msg->SetCmd( plAnimCmdMsg::kContinue );
                msg->SetCmd( plAnimCmdMsg::kSetForewards );
                msg->SetCmd( plAnimCmdMsg::kGoToBegin );
            }
        }
        else
        {
            // Moving to false
            if( immediate )
            {
                msg->SetCmd( plAnimCmdMsg::kGoToBegin );
            }
            else
            {
                msg->SetCmd( plAnimCmdMsg::kContinue );
                msg->SetCmd( plAnimCmdMsg::kSetBackwards );
                msg->SetCmd( plAnimCmdMsg::kGoToEnd );
            }
        }
        msg->SetAnimName( fAnimName );
        msg->AddReceivers( fAnimationKeys );
        msg->Send();
    }
}

void    pfGUICheckBoxCtrl::SetAnimationKeys(const std::vector<plKey> &keys, const ST::string &name)
{
    fAnimationKeys = keys;
    fAnimName = name;
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

uint32_t      pfGUICheckBoxCtrl::IGetDesiredCursor() const
{
    if( fClicking )
        return plInputInterface::kCursorClicked;

    return plInputInterface::kCursorPoised;
}

