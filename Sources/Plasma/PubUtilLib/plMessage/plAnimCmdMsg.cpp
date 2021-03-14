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

#include "hsStream.h"

#include "plAnimCmdMsg.h"

plAnimCmdMsg::~plAnimCmdMsg()
{
    ClearCmd();
}


void plAnimCmdMsg::ClearCmd() 
{ 
    plMessageWithCallbacks::Clear();
    fCmd.Clear(); 
}

bool plAnimCmdMsg::CmdChangesAnimTime()
{
    return (Cmd(kContinue) ||
            Cmd(kStop) ||
            Cmd(kGoToTime) ||
            Cmd(kToggleState) ||
            Cmd(kGoToBegin) ||
            Cmd(kGoToEnd) ||
            Cmd(kGoToLoopBegin) ||
            Cmd(kGoToLoopEnd) ||
            Cmd(kIncrementForward) ||
            Cmd(kIncrementBackward) ||
            Cmd(kFastForward) ||
            Cmd(kPlayToTime) ||
            Cmd(kPlayToPercentage));
}
    
void plAnimCmdMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessageWithCallbacks::Read(stream, mgr);

    fCmd.Read(stream);
    stream->ReadLEFloat(&fBegin);
    stream->ReadLEFloat(&fEnd);
    stream->ReadLEFloat(&fLoopEnd);
    stream->ReadLEFloat(&fLoopBegin);
    stream->ReadLEFloat(&fSpeed);
    stream->ReadLEFloat(&fSpeedChangeRate);
    stream->ReadLEFloat(&fTime);

    fAnimName = stream->ReadSafeString();
    fLoopName = stream->ReadSafeString();
}

void plAnimCmdMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessageWithCallbacks::Write(stream, mgr);

    fCmd.Write(stream);
    stream->WriteLEFloat(fBegin);
    stream->WriteLEFloat(fEnd);
    stream->WriteLEFloat(fLoopEnd);
    stream->WriteLEFloat(fLoopBegin);
    stream->WriteLEFloat(fSpeed);
    stream->WriteLEFloat(fSpeedChangeRate);
    stream->WriteLEFloat(fTime);

    stream->WriteSafeString(fAnimName);
    stream->WriteSafeString(fLoopName);
}

/////////////////////////////////////////////////////////////////////////////////////////

plAGCmdMsg::~plAGCmdMsg()
{
    ClearCmd();
}

void plAGCmdMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);

    fCmd.Read(stream);
    stream->ReadLEFloat(&fBlend);
    stream->ReadLEFloat(&fBlendRate);
    stream->ReadLEFloat(&fAmp);
    stream->ReadLEFloat(&fAmpRate);

    fAnimName = stream->ReadSafeString();
}

void plAGCmdMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);

    fCmd.Write(stream);
    stream->WriteLEFloat(fBlend);
    stream->WriteLEFloat(fBlendRate);
    stream->WriteLEFloat(fAmp);
    stream->WriteLEFloat(fAmpRate);

    stream->WriteSafeString(fAnimName);
}

