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
//  pfGameConsoleCommands                                                   //
//                                                                          //
//  Set of console commands that are actually meant for implementing        //
//  gameplay. Simpler than Python, easier to implement for programmers, and //
//  far lower level, but should be used with extreme caution.               //
//                                                                          //
//  Why? Because unlike normal console commands, which will eventually be   //
//  disabled in the shipping product (thus no danger of any fan cracking    //
//  them), these WILL be in the finished product. Debugging hacks and temp  //
//  workarounds to new features are NOT to be implemented here.             //
//                                                                          //
//  Because of their nature, all Game. console commands should be approved  //
//  by Brice before checkin.                                                //
//                                                                          //
//  Preferred method of adding Game. commands:                              //
//      1. Implement in normal console command groups (Debug. if nothing    //
//         else)                                                            //
//      2. Get the command approved                                         //
//      3. Once command is tested and approved and deemed both worthy and   //
//         bug-free, cut-and-paste the command into the Game. group         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
#define LIMIT_CONSOLE_COMMANDS 1
#endif

#include "plgDispatch.h"
#include "plPipeline.h"

#include "pfConsole.h"

#include "pnKeyedObject/plFixedKey.h"

#include "plAvatar/plAnimStage.h"
#include "plAvatar/plAvBrainGeneric.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plAvatar/plAvatarMgr.h"
#include "plGImage/plMipmap.h"
#include "plMessage/plAvatarMsg.h"
#include "plPipeline/plCaptureRender.h"

#include "pfConsoleCore/pfConsoleCmd.h"
#include "pfMessage/pfGameGUIMsg.h"
#include "pfMessage/pfKIMsg.h"

#define PF_SANITY_CHECK( cond, msg ) { if( !( cond ) ) { PrintString( msg ); return; } }

//// DO NOT REMOVE!!!!
//// This is here so Microsoft VC won't decide to "optimize" this file out
PF_CONSOLE_FILE_DUMMY(Game)
//// DO NOT REMOVE!!!!

//////////////////////////////////////////////////////////////////////////////
//// Game Group Commands /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( Game )

PF_CONSOLE_CMD( Game, TakeScreenshot, "int width, int height", "Captures a screenshot" )
{
    hsAssert(pfConsole::GetPipeline(), "Game.TakeScreenShot needs a plPipeline!");

    int width = params[0];
    int height = params[1];

    // Let's use plCaptureRender so that we have a really nice image.
    // We'll take care of saving in pfConsole::MsgReceive
    plCaptureRender::Capture(pfConsole::GetInstance()->GetKey(), width, height);
}

#ifndef LIMIT_CONSOLE_COMMANDS
PF_CONSOLE_CMD( Game, LoadDialog, "string dlgName", "Loads the given GUI dialog into memory" )
{
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        pfGameGUIMsg    *msg = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kLoadDialog );
        msg->SetString(params[0]);
        plgDispatch::MsgSend( msg );
    }
}

PF_CONSOLE_CMD( Game, LoadLocalDialog, "string ageName, string dlgName", "Loads the given GUI dialog into memory" )
{
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        pfGameGUIMsg    *msg = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kLoadDialog );
        msg->SetString(params[1]);
        msg->SetAge(params[0]);
        plgDispatch::MsgSend( msg );
    }
}

PF_CONSOLE_CMD( Game, ShowDialog, "string dlgName", "Shows the given GUI dialog" )
{
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        pfGameGUIMsg    *msg = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kShowDialog );
        msg->SetString(params[0]);
        plgDispatch::MsgSend( msg );
    }
}

PF_CONSOLE_CMD( Game, HideDialog, "string dlgName", "Hides the given GUI dialog" )
{
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        pfGameGUIMsg    *msg = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kHideDialog );
        msg->SetString(params[0]);
        plgDispatch::MsgSend( msg );
    }
}



PF_CONSOLE_CMD( Game, SwitchDialog, "string olddlgName, string newdlgName", "Hide olddlg and show newdlg" )
{
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        pfGameGUIMsg    *msg = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kHideDialog );
        msg->SetString(params[0]);
        plgDispatch::MsgSend( msg );

        pfGameGUIMsg    *msg2 = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kShowDialog );
        msg2->SetString(params[1]);
        plgDispatch::MsgSend( msg2 );
    }
}

#endif

PF_CONSOLE_CMD( Game, EnterChatMode, "", "Enters in-game chat mode" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kEnterChatMode);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIToggleMini, "", "Toggle between mini and big KI" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kMiniBigKIToggle);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIPutAway, "", "Put KI completely away" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIPutAway);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIChatPageUp, "", "Scroll chat display one page up" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kChatAreaPageUp);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIChatPageDown, "", "Scroll chat display one page down" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kChatAreaPageDown);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIChatToStart, "", "Scroll chat display to top of buffer" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kChatAreaGoToBegin);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIChatToEnd, "", "Scroll chat display to bottom of buffer" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kChatAreaGoToEnd);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KITakePicture, "", "Take picture with KI" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKITakePicture);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KICreateJournal, "", "Create journal note entry" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKICreateJournalNote);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIChatToggleFaded, "", "Toggle fade of chat display" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIToggleFade);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIChatToggleFadeEnable, "", "Toggle enable of chat fade" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIToggleFadeEnable);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIUpSizeFont, "", "Up size the KI font (chatarea)" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIUpSizeFont);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIDownSizeFont, "", "Down size the KI font (chatarea)" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIDownSizeFont);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIOpenYeeshaBook, "", "Open the player's Yeesha book" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIOpenYeehsaBook);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIOpenKI, "", "Open the KI a little at a time" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIOpenKI);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KIHelp, "", "Open the CCR help dialog" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKIShowCCRHelp);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KICreateMarker, "", "Create marker in the working marker folder" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKICreateMarker);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, KICreateMarkerFolder, "", "Create marker folder in current Age's journal folder" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kKICreateMarkerFolder);
    plgDispatch::MsgSend( msg );
}

PF_CONSOLE_CMD( Game, SetChatFadeDelay, "float delayInSecs", "Sets the time in seconds before the chat text disappears" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kSetChatFadeDelay);
    msg->SetDelay( params[0] );
    plgDispatch::MsgSend( msg );
}

#include "plAvatar/plArmatureMod.h"

PF_CONSOLE_CMD( Game, LimitAvatarLOD, "int newLOD", "Zero is (always) highest detail; 2 is (currently) lowest." )
{
    int newLOD = params[0];

    if(newLOD >= 0 && newLOD <= 2)
        plArmatureLODMod::fMinLOD = newLOD;
}


PF_CONSOLE_SUBGROUP( Game, Emote)       // Game.Emote.Shakefist

void Emote(const ST::string& emotion, float fadeIn = 2.0, float fadeOut = 2.0)
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    AvatarEmote(avatar, emotion);
}

PF_CONSOLE_CMD( Game_Emote, Wave, "", "")
{
    Emote(ST_LITERAL("Wave"), 4.0, 1.0);
}

PF_CONSOLE_CMD( Game_Emote, Sneeze, "", "")
{
    Emote(ST_LITERAL("Sneeze"));
}

PF_CONSOLE_CMD( Game_Emote, Dance, "", "")
{
    Emote(ST_LITERAL("Dance"));
}

PF_CONSOLE_CMD( Game_Emote, Laugh, "", "")
{
    Emote(ST_LITERAL("Laugh"));
}

PF_CONSOLE_CMD( Game_Emote, Clap, "", "")
{
    Emote(ST_LITERAL("Clap"), 4.0, 3.0);
}

PF_CONSOLE_CMD( Game_Emote, Talk, "", "")
{
    Emote(ST_LITERAL("Talk"));
}

PF_CONSOLE_CMD( Game_Emote, Sit, "", "")
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    PushSimpleMultiStage(avatar, ST_LITERAL("SitDownGround"), ST_LITERAL("SitIdleGround"), ST_LITERAL("SitStandGround"), true, true, plAGAnim::kBodyLower, plAvBrainGeneric::kSitOnGround);
}

#ifndef PLASMA_EXTERNAL_RELEASE
PF_CONSOLE_CMD( Game, SetLocalClientAsAdmin, "bool enable", "Makes chat messages from this client appear as admin messages" )
{
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kSetTextChatAdminMode);
    msg->SetFlags( params[0] ? pfKIMsg::kAdminMsg : 0 );
    plgDispatch::MsgSend( msg );
}
#endif
