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


#include "pfConsoleCore/pfConsoleCmd.h"
#include "pfConsole.h"

#include "plPipeline.h"
#include "plgDispatch.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plTGAWriter.h"
#include "pfMessage/pfGameGUIMsg.h"
#include "hsResMgr.h"
#include "pfGameGUIMgr/pfGUICtrlGenerator.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plAnimStage.h"
#include "plAvatar/plAvBrainGeneric.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plMessage/plAvatarMsg.h"
#include "pnKeyedObject/plFixedKey.h"


#define PF_SANITY_CHECK( cond, msg ) { if( !( cond ) ) { PrintString( msg ); return; } }

//// DO NOT REMOVE!!!!
//// This is here so Microsoft VC won't decide to "optimize" this file out
PF_CONSOLE_FILE_DUMMY(Game)
//// DO NOT REMOVE!!!!

//// plDoesFileExist //////////////////////////////////////////////////////////
//  Utility function to determine whether the given file exists

static hsBool   plDoesFileExist( const char *path )
{
    hsUNIXStream    stream;


    if( !stream.Open( path, "rb" ) )
        return false;

    stream.Close();
    return true;
}

//////////////////////////////////////////////////////////////////////////////
//// Game Group Commands /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( Game )

#ifndef LIMIT_CONSOLE_COMMANDS
PF_CONSOLE_CMD( Game, TakeScreenshot, "...", "Takes a shot of the current frame and saves it to the given file" )
{
    hsAssert( pfConsole::GetPipeline() != nil, "Cannot use this command before pipeline initialization" );

    plMipmap        myMipmap;
    char            fileName[ 512 ];
    uint32_t          uniqueNumber;   


    if( numParams > 1 )
    {
        PrintString( "Too many parameters to TakeScreenshot" );
        return;
    }
    else if( numParams == 1 )
        strcpy( fileName, (char *)params[ 0 ] );
    else
    {
        // Think up a filename
        for( uniqueNumber = 1; uniqueNumber < 1000; uniqueNumber++ )
        {
            sprintf( fileName, "screen%03d.tga", uniqueNumber );
            if( !plDoesFileExist( fileName ) )
                break;
        }
        if( uniqueNumber == 1000 )
        {
            PrintString( "Out of filenames for TakeScreenshot" );
            return;
        }
    }

    if( !pfConsole::GetPipeline()->CaptureScreen( &myMipmap ) )
        PrintString( "Error capturing screenshot" );
    else
    {
        char    str[ 512 ];


        plTGAWriter::Instance().WriteMipmap( fileName, &myMipmap );
        sprintf( str, "Screenshot written to '%s'.", fileName );
        PrintString( str );
    }
}

PF_CONSOLE_CMD( Game, LoadDialog, "string dlgName", "Loads the given GUI dialog into memory" )
{
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        pfGameGUIMsg    *msg = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kLoadDialog );
        msg->SetString( params[ 0 ] );
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
        msg->SetString( params[ 1 ] );
        msg->SetAge( params[ 0 ] );
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
        msg->SetString( params[ 0 ] );
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
        msg->SetString( params[ 0 ] );
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
        msg->SetString( params[ 0 ] );
        plgDispatch::MsgSend( msg );

        pfGameGUIMsg    *msg2 = new pfGameGUIMsg( mgrKey, pfGameGUIMsg::kShowDialog );
        msg2->SetString( params[ 1 ] );
        plgDispatch::MsgSend( msg2 );
    }
}

PF_CONSOLE_SUBGROUP( Game, GUI )

#include "pfGameGUIMgr/pfGUICtrlGenerator.h"

static hsColorRGBA  sDynCtrlColor = hsColorRGBA().Set( 1, 1, 1, 1 ), sDynCtrlTextColor = hsColorRGBA().Set( 0, 0, 0, 1 );

PF_CONSOLE_CMD( Game_GUI, SetDynamicCtrlColor, "float bgRed, float bgGreen, float bgBlue, float textRed, float textGreen, float textBlue", "" )
{
    sDynCtrlColor.Set( params[ 0 ], params[ 1 ], params[ 2 ], 1.f );
    sDynCtrlTextColor.Set( params[ 3 ], params[ 4 ], params[ 5 ], 1.f );
}


PF_CONSOLE_CMD( Game_GUI, CreateRectButton, "string title, float x, float y, float width, float height, string command", "" )
{
    pfGUICtrlGenerator::Instance().GenerateRectButton( params[ 0 ], params[ 1 ], params[ 2 ],
                                            params[ 3 ], params[ 4 ], 
                                            params[ 5 ], 
                                            sDynCtrlColor, sDynCtrlTextColor );
}

PF_CONSOLE_CMD( Game_GUI, CreateRoundButton, "float x, float y, float radius, string command", "" )
{
    pfGUICtrlGenerator::Instance().GenerateSphereButton( params[ 0 ], params[ 1 ], params[ 2 ], 
                                            params[ 3 ], 
                                            sDynCtrlColor );
}

PF_CONSOLE_CMD( Game_GUI, CreateDragBar, "float x, float y, float width, float height", "")
{
    pfGUICtrlGenerator::Instance().GenerateDragBar( params[ 0 ], params[ 1 ], params[ 2 ], params[ 3 ], sDynCtrlColor );
}

PF_CONSOLE_CMD( Game_GUI, CreateDialog, "string name", "" )
{
    pfGUICtrlGenerator::Instance().GenerateDialog( params[ 0 ] );
}


#endif


//#include "../pfKI/pfKI.h"
#include "pfMessage/pfKIMsg.h"

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
//  pfKI::GetInstance()->SetChatFadeDelay( params[ 0 ] );
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

void Emote(const char *emotion, float fadeIn = 2.0, float fadeOut = 2.0)
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    AvatarEmote(avatar, emotion);
}

PF_CONSOLE_CMD( Game_Emote, Wave, "", "")
{
    Emote("Wave", 4.0, 1.0);
}

PF_CONSOLE_CMD( Game_Emote, Sneeze, "", "")
{
    Emote("Sneeze");
}

PF_CONSOLE_CMD( Game_Emote, Dance, "", "")
{
    Emote("Dance");
}

PF_CONSOLE_CMD( Game_Emote, Laugh, "", "")
{
    Emote("Laugh");
}

PF_CONSOLE_CMD( Game_Emote, Clap, "", "")
{
    Emote("Clap", 4.0, 3.0);
}

PF_CONSOLE_CMD( Game_Emote, Talk, "", "")
{
    Emote("Talk");
}

PF_CONSOLE_CMD( Game_Emote, Sit, "", "")
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    PushSimpleMultiStage(avatar, "SitDownGround", "SitIdleGround", "SitStandGround", true, true, plAGAnim::kBodyLower, plAvBrainGeneric::kSitOnGround);
}

#ifndef PLASMA_EXTERNAL_RELEASE
PF_CONSOLE_CMD( Game, SetLocalClientAsAdmin, "bool enable", "Makes chat messages from this client appear as admin messages" )
{
//  pfKI::GetInstance()->SetTextChatAdminMode( (bool)params[ 0 ] );
    pfKIMsg* msg = new pfKIMsg(pfKIMsg::kSetTextChatAdminMode);
    msg->SetFlags( params[0] ? pfKIMsg::kAdminMsg : 0 );
    plgDispatch::MsgSend( msg );
}
#endif

#include "pfConditional/plObjectInBoxConditionalObject.h"

PF_CONSOLE_CMD( Game, BreakVolumeSensors, "bool break", "reverts to old broken volume sensor logic" )
{
    bool b =  params[ 0 ];
    plVolumeSensorConditionalObject::makeBriceHappyVar = !b;
}
