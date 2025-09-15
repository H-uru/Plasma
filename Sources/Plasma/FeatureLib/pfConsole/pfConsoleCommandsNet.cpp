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
//  Actual NET Console Commands and Groups                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
#define LIMIT_CONSOLE_COMMANDS 1
#endif

#include <string_theory/format>

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsTimer.h"

#include "pfConsole.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plKeyImp.h"
#include "pnMessage/plAudioSysMsg.h"
#include "pnMessage/plClientMsg.h"
#include "pnMessage/plEnableMsg.h"
#include "pnModifier/plLogicModBase.h"
#include "pnUUID/pnUUID.h"

#include "plAgeDescription/plAgeDescription.h"
#include "plAgeLoader/plAgeLoader.h"
#include "plAgeLoader/plResPatcher.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plConsoleMsg.h"
#include "plMessage/plOneShotMsg.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetClient/plNetLinkingMgr.h"
#include "plNetCommon/plNetObjectDebugger.h"
#include "plNetCommon/plSpawnPointInfo.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plNetMessage/plNetMessage.h"
#include "plResMgr/plKeyFinder.h"
#include "plSDL/plSDL.h"
#include "plStatusLog/plStatusLog.h"
#include "plUnifiedTime/plUnifiedTime.h"
#include "plVault/plVault.h"

#include "pfConsoleCore/pfConsoleCmd.h"
#include "pfPython/plPythonSDLModifier.h"

// FIXME FIXME
#include "../../Apps/plClient/plClient.h"

#define PF_SANITY_CHECK( cond, msg ) { if( !( cond ) ) { PrintString( msg ); return; } }

//// DO NOT REMOVE!!!!
//// This is here so Microsoft VC won't decide to "optimize" this file out
PF_CONSOLE_FILE_DUMMY(Net)
//// DO NOT REMOVE!!!!

//// Defining Console Commands ///////////////////////////////////////////////
//
//  You define console commands by using the PF_CONSOLE_CMD macro. The format
//  of the macro is:
//
//      PF_CONSOLE_CMD( groupName, functionName, "paramList", "Help string" )
//
//  Where:
//      - groupName is a string representing what command group the command
//        is in. Subgroups are specified by an underscore, i.e. to put a command
//        in the Draw subgroup of the Graphics group, you would specify
//        "Graphics_Draw". Specifying "" means to put the command in the base
//        command group--i.e. it has no group.
//      - functionName is required; it specifies the function name (really?!?!).
//        Function names must be globally unique, so you can't have a Draw
//        function in both the Graphics and SceneAPI groups. Sorry. :(
//      - paramList specifies the parameters and types to the function.
//        The smallest list you can have is "", which means "no parameters".
//        If you have parameters, it must be in a comma-delimited string.
//        You can either specify types or labels and types, so you can say
//        "int, float" or "int x, float value". Currently, the labels are only
//        used when printing out usage strings, but they will be used later
//        for auto-labeling GUI elements, so please put them in where viable.
//
//        White space does not matter. Valid types are int, float, char, string
//        bool (auto-conversion of "true"/"false" strings to 1 or 0) and "...".
//        "..." is a special type that means the same as the C equivalent:
//        "there can be zero or more parameters here and I don't care what the
//        type is" is the gist of it.
//      - helpString is a short description of the function, which currently
//        isn't used, but will be used in the future when implementing help
//        (could you have guessed it? :) Please fill it in when the function
//        name isn't obvious (i.e. SetFogColor doesn't really need one)
//  
//  The actual C code prototype looks like:
//      void pfConsoleCmd_groupName_functionName(int32_t numParams, pfConsoleCmdParam *params, void (*PrintString)(const ST::string&));
//
//  numParams is exactly what it sounds like. params is an array of console
//  parameter objects, each of which are rather nifty in that they can be cast
//  immediately to whatever type you asked for in your parameter list
//  ("paramList" above). So if your paramList was "int", then params[ 0 ]
//  can be cast to an int immediately, such as int x = params[ 0 ];
//  If you attempt to cast a parameter to a type other than the one specified
//  in the paramList, you get an hsAssert saying so, so don't do it! Any
//  parameters that fall under "..." are automagically strings, but they can
//  be cast to any valid type without an assert. So basically, if you want
//  to still do your own conversion, just specify "..." as the entire paramList
//  and treat the params array as if it were an array of strings.
//
//  Thus, the net result of the paramList is that it lets the console engine
//  do the parameter parsing for you. If the paramters given to the function
//  do not match the list (this includes too many or too few parameters), a
//  usage string is printed out and the function is not called. Thus, the
//  ONLY parameter error you can possibly have is casting a parameter object
//  to a type other than you asked for. So don't do it!
//
//  (Note: this makes numParams almost obsolete; the only reason it still has
//  a use is for "...", which of course allows variable number of parameters.)
//
//  PrintString is a function that lets you print output to the on-screen
//  console. It is guaranteed to be non-null. Worst case is that it points
//  to a dummy function that does nothing, but it will *always* be valid.
//
//  To define console command groups, you use the macro:
//
//      PF_CONSOLE_GROUP( group )
//
//  where "group" is the name without quotes of the group you want to create.
//  To create a subgroup inside a group, use:
//
//      PF_CONSOLE_SUBGROUP( parent, group )
//
//  where "parent" is the parent group for the subgroup. "parent" can have
//  underscores in it just like the group of a CONSOLE_CMD, so you can say
//
//      PF_CONSOLE_SUBGROUP( Graphics_Render, Drawing )
//
//  to create the Graphics_Render_Drawing subgroup. All groups must be
//  defined before any commands that are in that group. Note that although
//  the 
//
//////////////////////////////////////////////////////////////////////////////

//
// utility functions
//
//////////////////////////////////////////////////////////////////////////////
plKey FindSceneObjectByName(const ST::string& name, const ST::string& ageName, ST::string& statusStr, bool subString=false);
plKey FindObjectByName(const ST::string& name, int type, const ST::string& ageName, ST::string& statusStr, bool subString=false);
plKey FindObjectByNameAndType(const ST::string& name, const char* typeName, const ST::string& ageName,
                              ST::string statusStr, bool subString=false);

//////////////////////////////////////////////////////////////////////////////
//// Network Group Commands //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PF_CONSOLE_GROUP( Net )     // Defines a main command group


#ifndef LIMIT_CONSOLE_COMMANDS
// Net.Ping
PF_CONSOLE_CMD( Net,
                Ping,
                "bool enable",
                "Enable/disable server ping" )
{
    bool enable = params[0];
    NetClientPingEnable(enable);
}

//
// Temp until we get real text chat
//
PF_CONSOLE_CMD( Net,        // groupName
               Chat,        // fxnName
               "...",       // string params
               "broadcast chat msg" )   // helpString
{
    // send chat text
    ST::string text=plNetClientMgr::GetInstance()->GetPlayerName();
    text += ":";
    int i;
    for(i=0;i<numParams;i++)
    {
        text += static_cast<const ST::string&>(params[i]);
        text += " ";
    }
    plConsoleMsg    *cMsg = new plConsoleMsg( plConsoleMsg::kAddLine, text );
    cMsg->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce);
    cMsg->SetBCastFlag(plMessage::kLocalPropagate, 0);
    plgDispatch::MsgSend( cMsg );
    
    PrintString(text); // show locally too
}


PF_CONSOLE_CMD( Net,        // groupName
               ShowRelevanceRegions,    // fxnName
               "bool on", // paramList
               "Show player relevance regions" )    // helpString
{
    bool on = params[0];
    plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowRelevanceRegions, on);  
    if (on)
    {
        plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowLists, false);  
        plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowRooms, false);  
    }

}

PF_CONSOLE_CMD( Net,        // groupName
               ShowLists,   // fxnName
               "bool on", // paramList
               "Show debug player lists" )  // helpString
{
    bool on = params[0];
    plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowLists, on); 
    if (on)
    {
        plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowRooms, false);  
        plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowRelevanceRegions, false);   
    }
}

PF_CONSOLE_CMD( Net,        // groupName
               ShowRooms,   // fxnName
               "bool on", // paramList
               "Show debug room lists" )    // helpString
{
    bool on = params[0];
    plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowRooms, on); 
    if (on)
    {
        plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowLists, false);  
        plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kShowRelevanceRegions, false);   
    }
}

#endif


#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Net,        // groupName
               LocalTriggers,       // fxnName
               "", // paramList
               "Make triggers localOnly" )  // helpString
{
    plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kLocalTriggers); 
}

PF_CONSOLE_CMD( Net,        // groupName
               SetConsoleOutput,        // fxnName
               "bool onoff", // paramList
               "send log output to the debug window" )  // helpString
{
    plNetClientMgr::GetInstance()->SetConsoleOutput( params[0] );
}



/////////////

// ENABLE/DISABLE LINKING
PF_CONSOLE_CMD( Net,        // groupName
               EnableLinking,       // fxnName
               "bool enable", // paramList
               "Enable/disable linking." )  // helpString
{   
    if ( (int)params[0] )
    {
        plNetLinkingMgr::GetInstance()->SetEnabled( true );
        PrintString("Linking enabled.");
    }
    else
    {
        plNetLinkingMgr::GetInstance()->SetEnabled( false );
        PrintString("Linking disabled.");
    }
}

/**
 * Parse spawn point info from a string entered by the user,
 * returning a useful parse error message if the input is invalid.
 *
 * @param input Spawn point info string to parse
 * @param error Parse error message, or empty string on success
 * @return Parsed spawn point info object
 */
static plSpawnPointInfo TryParseSpawnPointInfo(const ST::string& input, ST::string& error)
{
    // Ignore any trailing semicolon.
    ST::string inputNoSemicolon = input.trim_right(";");
    // Fail on semicolon anywhere else.
    if (inputNoSemicolon.contains(';')) {
        error = ST_LITERAL("Cannot contain a semicolon");
        return {};
    }

    // Check for the expected number of colon-separated parts.
    // Title and spawn point name are required, camera stack is optional.
    auto parts = inputNoSemicolon.split(':');
    if (parts.size() < 2) {
        error = ST_LITERAL("Missing colon, expected e.g. " kDefaultSpawnPtTitle ":" kDefaultSpawnPtName);
        return {};
    } else if (parts.size() > 3) {
        error = ST::format("At most 2 colons allowed, found {}", parts.size() - 1);
        return {};
    }

    plSpawnPointInfo ret;
    ret.SetTitle(parts[0]);
    ret.SetName(parts[1]);
    if (parts.size() >= 3) {
        ret.SetCameraStack(parts[2]);
    }

    error.clear();
    return ret;
}

// GENERIC LINK. PLS WILL LOAD-BALANCE YOU TO A PUBLIC INSTANCE.
PF_CONSOLE_CMD( Net,        // groupName
               LinkToAge,       // fxnName
               "string ageFilename", // paramList
               "Link to an age." )  // helpString
{   
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);
    link.SetLinkingRules( plNetCommon::LinkingRules::kBasicLink );
    plNetLinkingMgr::GetInstance()->LinkToAge( &link );
    PrintString("Linking to age...");
}


// LINK TO A SPECIFIC AGE INSTANCE BY GUID.
PF_CONSOLE_CMD( Net,        // groupName
               LinkToAgeInstance,       // fxnName
               "string ageFilename, string ageGuid", // paramList
               "Link to a specific age by guid." )  // helpString
{   
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);
    //link.GetAgeInfo()->SetAgeInstanceName( params[0] );
    //link.GetAgeInfo()->SetAgeUserDefinedName( params[0] );
    plUUID guid(params[1]);
    link.GetAgeInfo()->SetAgeInstanceGuid( &guid );
    link.SetLinkingRules( plNetCommon::LinkingRules::kBasicLink );
    plNetLinkingMgr::GetInstance()->LinkToAge( &link );
    PrintString("Linking to age...");
}
// LINK TO MY PREVIOUS AGE
PF_CONSOLE_CMD( Net,        // groupName
               LinkToPrevAge,       // fxnName
               "", // paramList
               "Link to my last age." ) // helpString
{   
    plNetLinkingMgr::GetInstance()->LinkToPrevAge( );
    PrintString("Linking to previous age...");
}
// LINK WITH ORIGINAL LINKING BOOK
PF_CONSOLE_CMD( Net,
               LinkWithOriginalBook,
               "string ageFilename, ...",
               "Link to specified age using Original Age Linking Book rules. Optional second argument is a spawn point in the format Title:SpawnPointName (defaults to " kDefaultSpawnPtTitle ":" kDefaultSpawnPtName ")." )
{
    if (numParams > 2) {
        PrintString(ST::format("Expected 1 or 2 arguments, not {}", numParams));
        return;
    }

    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);

    if (numParams >= 2) {
        ST::string error;
        link.SetSpawnPoint(TryParseSpawnPointInfo(params[1], error));
        if (!error.empty()) {
            PrintString(ST::format("Invalid spawn point: {}", error));
            return;
        }
    }

    link.SetLinkingRules( plNetCommon::LinkingRules::kOriginalBook );
    plNetLinkingMgr::GetInstance()->LinkToAge( &link );
    PrintString("Linking to age with original book...");
}
// LINK TO AN AGE I OWN
PF_CONSOLE_CMD( Net,
               LinkWithOwnedBook,
               "string ageFilename",
               "Link to specified age using Personal Age Linking Book rules" )
{
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);
    link.SetLinkingRules( plNetCommon::LinkingRules::kOwnedBook );
    plNetLinkingMgr::GetInstance()->LinkToAge( &link );
    PrintString("Linking to age I own...");
}
// LINK TO AN AGE I CAN VISIT
PF_CONSOLE_CMD( Net,
               LinkWithVisitBook,
               "string ageFilename",
               "Link to specified age using Personal Age Linking Book rules" )
{
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);
    link.SetLinkingRules( plNetCommon::LinkingRules::kVisitBook );
    plNetLinkingMgr::GetInstance()->LinkToAge( &link );
    PrintString("Linking to age I can visit...");
}
// LINK TO A SUB AGE
PF_CONSOLE_CMD( Net,
               LinkWithSubAgeBook,
               "string ageFilename",
               "Link to a sub-age of the current age" )
{
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);
    link.SetLinkingRules( plNetCommon::LinkingRules::kSubAgeBook );
    plNetLinkingMgr::GetInstance()->LinkToAge( &link );
    PrintString("Linking to a sub-age...");
}

// LINK TO A PLAYER'S CURRENT AGE
PF_CONSOLE_CMD( Net,
               LinkToPlayersAge,
               "int playerID",
               "Link to specified player's current age" )
{
    plNetLinkingMgr::GetInstance()->LinkToPlayersAge( (int)params[0] );
    PrintString("Linking to a player's current age...");
}
// LINK A PLAYER HERE
PF_CONSOLE_CMD( Net,
               LinkPlayerHere,
               "int playerID",
               "Link specified player to our current age." )
{
    plNetLinkingMgr::GetInstance()->LinkPlayerHere( (int)params[0] );
    PrintString("Linking player to our current age...");
}
// LINK A PLAYER BACK TO HIS LAST AGE
PF_CONSOLE_CMD( Net,
               LinkPlayerToPrevAge,
               "int playerID",
               "Link specified player back to his last age." )
{
    plNetLinkingMgr::GetInstance()->LinkPlayerToPrevAge( (int)params[0] );
    PrintString("Linking player back to his last age...");
}

// LINK TO MY NEIGHBORHOOD AGE
PF_CONSOLE_CMD( Net,
               LinkToMyNeighborhoodAge,
               "",
               "Link to my neighborhood" )
{
    plNetLinkingMgr::GetInstance()->LinkToMyNeighborhoodAge();
    PrintString("Linking to my neighborhood...");
}

// LINK TO MY PERSONAL AGE
PF_CONSOLE_CMD( Net,
               LinkToMyPersonalAge,
               "",
               "Link to my personal age" )
{
    plNetLinkingMgr::GetInstance()->LinkToMyPersonalAge();
    PrintString("Linking to my personal age...");
}

#endif


#ifndef LIMIT_CONSOLE_COMMANDS

// OFFER LINK TO PLAYER
PF_CONSOLE_CMD( Net,
               OfferLinkToPlayer,
               "string ageFilename, int playerID",
               "Offer a link to a player" )
{
    plNetClientMgr * nc = plNetClientMgr::GetInstance();
    plNetLinkingMgr * lm = plNetLinkingMgr::GetInstance();

    plAgeInfoStruct info;
    info.SetAgeFilename(params[0]);
    
    plAgeLinkStruct link;
    if (!VaultGetOwnedAgeLink(&info, &link)) {
        PrintString( "You don't own an age by that name. Not offering link to player." );
        return;
    }

    
    link.SetLinkingRules(plNetCommon::LinkingRules::kVisitBook);

    plNetLinkingMgr::GetInstance()->OfferLinkToPlayer(&link, (int)params[1] );
    PrintString("Offered age link to player.");
}



/////////////


PF_CONSOLE_CMD( Net,            // groupName
               SetObjUpdateFreq,        // fxnName
               "string objName, float freqInSecs", // paramList
               "Instructs the server to only send me updates about this object periodically" )  // helpString
{
    ST::string status;
    plKey key = FindSceneObjectByName(params[0], {}, status);
    PrintString(status);
    if (!key)
        return;

    float freq = params[1];

    // send net msg
    plNetMsgObjectUpdateFilter netMsg;
    netMsg.SetMaxUpdateFreq(freq);
    netMsg.ObjectListInfo()->AddObject(key);
    plNetClientMgr::GetInstance()->SendMsg(&netMsg);
}

PF_CONSOLE_CMD( Net,        // groupName
               EnableClientDelay,       // fxnName
               "", // paramList
               "Switches on delay in client update loop" )  // helpString
{
    plClient::EnableClientDelay();
}

PF_CONSOLE_CMD( Net,        // groupName
               GetServerTime,       // fxnName
               "", // paramList
               "returns the current server clock" ) // helpString
{
    PrintString(ST::format("Current server time = {}", plNetClientMgr::GetInstance()->GetServerTime().Print()));
}

PF_CONSOLE_CMD( Net,        // groupName
               GetAgeElapsedTime,       // fxnName
               "", // paramList
               "returns the age of the age" )   // helpString
{
    PrintString(ST::format("Current age is {}, elapsed time since birth = {f} secs",
        NetCommGetAge()->ageDatasetName, plNetClientMgr::GetInstance()->GetCurrentAgeElapsedSeconds()));
}

PF_CONSOLE_CMD( Net, DownloadViaManifest,
               "string remoteManifestFileName", // paramList
               "Downloads the given file from the dataserver, then updates all the ages listed in the file" )   // helpString
{
//  if (hsgResMgr::ResMgr() != nullptr)
//  {
//      plResPatcher::PatchFromManifest( params[ 0 ] );
//  }
//  else
        PrintString( "DownloadViaManifest failed: resManager not initialized. This command must be used in an .fni file or later." );
}

PF_CONSOLE_CMD( Net, GetCCRAwayStatus,
               "", // paramList
               "Find out if CCR's are offline" )    // helpString
{
    PrintString(ST::format("The CCR dept is {}", VaultGetCCRStatus() ? "online" : "away"));
}

PF_CONSOLE_CMD( Net,            // groupName
               ForceSetAgeTimeOfDay,        // fxnName
               "float timePercent", // paramList
               "Force the age time of day to a specific value from 0.0 to 1.0.  Set to -1 to unset." )  // helpString
{   
    plNetClientMgr::GetInstance()->SetOverrideAgeTimeOfDayPercent(params[0]);
}

PF_CONSOLE_CMD( Net,            // groupName
               ScreenMessages,      // fxnName
               "bool on", // paramList
               "Screen out illegal net msgs." ) // helpString
{   
    bool on = params[0];
    plNetApp::GetInstance()->SetFlagsBit(plNetApp::kScreenMessages, on);
}

#endif

///////////////////////////////////////
#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_SUBGROUP( Net, DebugObject )     // Creates a DebugFocus sub-group under the net group

PF_CONSOLE_CMD( Net_DebugObject,        // groupName
               AddObject,       // fxnName
               "string objName, ...", // paramList
               "Create a debug log about the specified object. AddObject objName [pageName], wildcards allowed" )   // helpString
{
    const ST::string& objName = params[0];
    const ST::string& pageName = numParams > 1 ? params[1] : ST::string();
    if (plNetObjectDebugger::GetInstance())
        plNetObjectDebugger::GetInstance()->AddDebugObject(objName, pageName);
}

PF_CONSOLE_CMD( Net_DebugObject,        // groupName
               RemoveObject,        // fxnName
               "string objName, ...", // paramList
               "Stop focused debugging about the specified object. RemoveObject objName [pageName], wildcards allowed" )    // helpString
{
    const ST::string& objName = params[0];
    const ST::string& pageName = numParams > 1 ? params[1] : ST::string();
    if (plNetObjectDebugger::GetInstance())
        plNetObjectDebugger::GetInstance()->RemoveDebugObject(objName, pageName);
}

PF_CONSOLE_CMD( Net_DebugObject,        // groupName
               ClearAllObjects,     // fxnName
               "", // paramList
               "Stop focused debugging about all objects" ) // helpString
{
    if (plNetObjectDebugger::GetInstance())
        plNetObjectDebugger::GetInstance()->ClearAllDebugObjects();
}

PF_CONSOLE_CMD( Net_DebugObject,        // groupName
               DumpAgeSDLHook,      // fxnName
               "bool dirtyOnly", // paramList
               "Dump the age SDL hook to the object debugger" ) // helpString
{
    const plPythonSDLModifier * mod = plPythonSDLModifier::FindAgeSDL();
    mod->GetStateCache()->DumpToObjectDebugger("AgeSDLHook", params[0] );
}

#endif

///////////////////////////////////////
PF_CONSOLE_SUBGROUP( Net, Voice )       // Creates a VOICE sub-group under a given group

PF_CONSOLE_CMD( Net_Voice,      // groupName
               Echo,        // fxnName
               "bool on", // paramList
               "Turn on/off echoing of voice packets" ) // helpString
{
    bool on = params[0];
    plNetClientMgr::GetInstance()->SetFlagsBit(plNetClientMgr::kEchoVoice, on);
}

PF_CONSOLE_CMD( Net_Voice,                  // groupName
               SetMaxListeningRadius,       // fxnName
               "float dist", // paramList
               "Set the max distance that I hear another player's voice" )  // helpString
{
    float dist = params[0];
    plNetClientMgr::GetInstance()->GetListenList()->kMaxListenDistSq = dist*dist;
}

PF_CONSOLE_CMD( Net_Voice,                  // groupName
               SetMaxNumListeningTo,    // fxnName
               "int num", // paramList
               "Set the max number of other players that I can listen to at once" ) // helpString
{
    int max = params[0];
    plNetClientMgr::GetInstance()->GetListenList()->kMaxListenListSize=max;
}

///////////////////////////////////////
// Account Authentication
PF_CONSOLE_SUBGROUP( Net, Auth )        // Creates an AUTH sub-group under a given group

PF_CONSOLE_CMD( Net_Auth,       // groupName
               SetAccountName,      // fxnName
               "string name", // paramList
               "Sets account name." )   // helpString
{   
    plNetClientMgr * nc = plNetClientMgr::GetInstance();
    nc->SetIniAccountName(static_cast<const ST::string&>(params[0]).c_str());
}

PF_CONSOLE_CMD( Net_Auth,       // groupName
               SetAccountPassword,      // fxnName
               "string password", // paramList
               "Sets account password." )   // helpString
{   
    plNetClientMgr * nc = plNetClientMgr::GetInstance();
    nc->SetIniAccountPass(static_cast<const ST::string&>(params[0]).c_str());
}

#ifndef LIMIT_CONSOLE_COMMANDS

PF_CONSOLE_CMD( Net_Auth,       // groupName
               Disconnect,      // fxnName
               "", // paramList
               "Cause an auth server disconnect" )  // helpString
{
    NetCliAuthUnexpectedDisconnect();
}

#endif


///////////////////////////////////////

#ifndef LIMIT_CONSOLE_COMMANDS
PF_CONSOLE_SUBGROUP( Net, Vault )           // Creates sub-group Net.Vault

PF_CONSOLE_CMD( Net_Vault,      // groupName
               Dump,        // fxnName
               "", // paramList
               "Dump vault to log" )    // helpString
{
    hsAssert(false, "eric, implement me");
//  VaultDump();
}

///////////////////////////////////////

PF_CONSOLE_CMD( Net_Vault,      // groupName
               SetSeen,     // fxnName
               "int nodeId, int seen", // paramList
               "Set or clear node seen flag" )  // helpString
{
    VaultSetNodeSeen((int)params[0], (bool)params[1]);
}

///////////////////////////////////////

PF_CONSOLE_CMD( Net_Vault,      // groupName
               InMyPersonalAge,     // fxnName
               "", // paramList
               "" ) // helpString
{
    bool in = VaultAmInMyPersonalAge();
    PrintString(ST::format("You are {}in your personal age", in ? "" : "not "));
}
PF_CONSOLE_CMD( Net_Vault,      // groupName
               InMyNeighborhoodAge,     // fxnName
               "", // paramList
               "" ) // helpString
{
    bool in = VaultAmInMyNeighborhoodAge();
    PrintString(ST::format("You are {}in your neighborhood age", in ? "" : "not "));
}
PF_CONSOLE_CMD( Net_Vault,      // groupName
               AmOwnerOfCurrentAge,     // fxnName
               "", // paramList
               "" ) // helpString
{
    bool in = VaultAmOwnerOfCurrentAge();
    PrintString(ST::format("You are {}an owner of the current age", in ? "" : "not "));
}
PF_CONSOLE_CMD( Net_Vault,      // groupName
               AmCzarOfCurrentAge,      // fxnName
               "", // paramList
               "" ) // helpString
{
    bool in = VaultAmCzarOfCurrentAge();
    PrintString(ST::format("You are {}czar of the current age", in ? "" : "not "));
}
// REGISTER MT STATION
PF_CONSOLE_CMD( Net_Vault,
               RegisterMTStation,
               "string stationName, string mtSpawnPt",
               "Register an MT Station with your Nexus" )
{
    VaultRegisterMTStation(params[0], params[1]);
    PrintString("Registered MT Station.");
}


// REGISTER OWNED AGE
PF_CONSOLE_CMD( Net_Vault,
               RegisterOwnedAge,
               "string ageName",
               "Add an instance of the specified age to your bookshelf" )
{
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);
    link.GetAgeInfo()->SetAgeInstanceName(params[0]);
    plUUID guid = plUUID::Generate();
    link.GetAgeInfo()->SetAgeInstanceGuid( &guid);
    link.SetSpawnPoint( kDefaultSpawnPoint );
    bool success = VaultRegisterOwnedAgeAndWait(&link);
    PrintString(ST::format("Operation {}.", success ? "Successful" : "Failed"));
}

// UNREGISTER OWNED AGE
PF_CONSOLE_CMD( Net_Vault,
               UnregisterOwnedAge,
               "string ageName",
               "Remove the specified age from your bookshelf" )
{
    plAgeInfoStruct info;
    info.SetAgeFilename(params[0]);
    bool success = VaultUnregisterOwnedAge(&info);
    PrintString(ST::format("Operation {}.", success ? "Successful" : "Failed"));
}

// REGISTER VISIT AGE
PF_CONSOLE_CMD( Net_Vault,
               RegisterVisitAge,
               "string ageName",
               "Add an instance of the specified age to your private links" )
{
    plAgeLinkStruct link;
    link.GetAgeInfo()->SetAgeFilename(params[0]);
    link.GetAgeInfo()->SetAgeInstanceName(params[0]);
    plUUID guid = plUUID::Generate();
    link.GetAgeInfo()->SetAgeInstanceGuid( &guid);
    link.SetSpawnPoint( kDefaultSpawnPoint );
    bool success = VaultRegisterOwnedAgeAndWait(&link);
    PrintString(ST::format("Operation {}.", success ? "Successful" : "Failed"));
}

// UNREGISTER VISIT AGE
PF_CONSOLE_CMD( Net_Vault,
               UnregisterVisitAge,
               "string ageName",
               "Remove all instances of the specified age from your private links" )
{
    plAgeInfoStruct info;
    info.SetAgeFilename(params[0]);

    unsigned count = 0;
    while (VaultUnregisterVisitAge(&info))
        ++count;
        
    PrintString(ST::format("Operation {}.", count > 0 ? "Successful" : "Failed"));
}

#endif

