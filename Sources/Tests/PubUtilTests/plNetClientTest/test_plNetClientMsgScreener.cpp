/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#include <gtest/gtest.h>
#include <string_theory/string>

#include "plNetClient/plNetClientMsgScreener.h"

// Assorted creatables needed to make it link...
#include "pnAllCreatables.h"
#include "plAllCreatables.h"
#include "pfAnimation/pfAnimationCreatable.h"
#include "pfAudio/pfAudioCreatable.h"
#include "pfCamera/pfCameraCreatable.h"
#include "pfConditional/plConditionalObjectCreatable.h"
#include "pfMessage/pfMessageCreatable.h"

using namespace ST::literals;

static hsRef<plControlEventMsg> IMakeControlEventMsgWithCommand(ST::string command)
{
    hsRef msg(new plControlEventMsg(), hsStealRef);
    msg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
    msg->SetControlActivated(true);
    msg->SetCmdString(std::move(command));
    return msg;
}

static bool IScreenIncomingCommand(ST::string command)
{
    return plNetClientMsgScreener::IScreenIncoming(IMakeControlEventMsgWithCommand(std::move(command)).Get());
}

TEST(plNetClientMsgScreener, IScreenIncomingControlEventMsg)
{
    EXPECT_TRUE(IScreenIncomingCommand("Avatar.Spawn.DontPanic"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.Fog.SetDefColor 0.39999999960000004 0.26666666666399996 0.09999999990000001"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.Fog.SetDefExp 100000 20"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.Fog.SetDefExp2 100000 20"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.Fog.SetDefLinear 1000 10000 1.0"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.Fog.SetDefLinear -300 3000 1"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.SetClearColor 0.6 0.6 0.6"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.SetYon 100000"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.Renderer.Setyon 100000"_st)); // non-standard capitalization
    EXPECT_TRUE(IScreenIncomingCommand("Graphics.SetDebugFlag noFog"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Nav.PageInNode grsnExterior"_st));
    EXPECT_TRUE(IScreenIncomingCommand("Nav.PageOutNode grsnExterior"_st));

    EXPECT_FALSE(IScreenIncomingCommand("Python.Cheat 123"_st)); // unsafe command name
    EXPECT_FALSE(IScreenIncomingCommand("Graphics.Renderer.Fog 789"_st)); // incomplete command name
    EXPECT_FALSE(IScreenIncomingCommand("Graphics.Renderer.GrabCubeMap 456"_st)); // unsafe command name inside mostly safe group
    EXPECT_FALSE(IScreenIncomingCommand("Graphics.Renderer.SetClearColor $var"_st)); // safe command name with console variable in argument
    EXPECT_FALSE(IScreenIncomingCommand("Graphics.Renderer.SetClearColor func()"_st)); // safe command name with other suspicious characters in argument
    EXPECT_FALSE(IScreenIncomingCommand("Graphics.Renderer.SetYon 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"_st)); // safe command name and argument, but longer than kMaxSafeCommandLength
    EXPECT_FALSE(IScreenIncomingCommand("I realized the moment I fell into the fissure that the book would not be destroyed as I had planned."_st)); // not a command

    // Safe command, but in a plControlEventMsg that's not a console command message:

    hsRef nonActivatedMsg = IMakeControlEventMsgWithCommand("Graphics.Renderer.SetYon 100000"_st);
    nonActivatedMsg->SetControlActivated(false);
    EXPECT_FALSE(plNetClientMsgScreener::IScreenIncoming(nonActivatedMsg.Get()));

    hsRef nonCommandMsg = IMakeControlEventMsgWithCommand("Graphics.Renderer.SetYon 100000"_st);
    nonCommandMsg->SetControlCode(B_CONTROL_JUMP);
    EXPECT_FALSE(plNetClientMsgScreener::IScreenIncoming(nonActivatedMsg.Get()));
}
