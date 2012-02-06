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

#include "HeadSpin.h"
#include "plAnimCmdMsg.h"
#include "hsStream.h"

plAnimCmdMsg::~plAnimCmdMsg()
{
    ClearCmd();
}


void plAnimCmdMsg::ClearCmd() 
{ 
    plMessageWithCallbacks::Clear();
    fCmd.Clear(); 
}

void plAnimCmdMsg::SetAnimName(const plString &name)
{
    fAnimName = name;
}

plString plAnimCmdMsg::GetAnimName()
{
    return fAnimName;
}

void plAnimCmdMsg::SetLoopName(const plString &name)
{
    fLoopName = name;
}

plString plAnimCmdMsg::GetLoopName()
{
    return fLoopName;
}

hsBool plAnimCmdMsg::CmdChangesAnimTime()
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
    stream->ReadLE(&fBegin);
    stream->ReadLE(&fEnd);
    stream->ReadLE(&fLoopEnd);
    stream->ReadLE(&fLoopBegin);
    stream->ReadLE(&fSpeed);
    stream->ReadLE(&fSpeedChangeRate);
    stream->ReadLE(&fTime);

    fAnimName = stream->ReadSafeString_TEMP();
    fLoopName = stream->ReadSafeString_TEMP();
}

void plAnimCmdMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessageWithCallbacks::Write(stream, mgr);

    fCmd.Write(stream);
    stream->WriteLE(fBegin);
    stream->WriteLE(fEnd);
    stream->WriteLE(fLoopEnd);
    stream->WriteLE(fLoopBegin);
    stream->WriteLE(fSpeed);
    stream->WriteLE(fSpeedChangeRate);
    stream->WriteLE(fTime);

    stream->WriteSafeString(fAnimName);
    stream->WriteSafeString(fLoopName);
}

/////////////////////////////////////////////////////////////////////////////////////////

plAGCmdMsg::~plAGCmdMsg()
{
    ClearCmd();
}

void plAGCmdMsg::SetAnimName(const plString &name)
{
    fAnimName = name;
}

plString plAGCmdMsg::GetAnimName()
{
    return fAnimName;
}

void plAGCmdMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);

    fCmd.Read(stream);
    stream->ReadLE(&fBlend);
    stream->ReadLE(&fBlendRate);
    stream->ReadLE(&fAmp);
    stream->ReadLE(&fAmpRate);

    fAnimName = stream->ReadSafeString_TEMP();
}

void plAGCmdMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);

    fCmd.Write(stream);
    stream->WriteLE(fBlend);
    stream->WriteLE(fBlendRate);
    stream->WriteLE(fAmp);
    stream->WriteLE(fAmpRate);

    stream->WriteSafeString(fAnimName);
}

/////////////////////////////////////////////////////////////////////////////////////

void plAGDetachCallbackMsg::SetAnimName(const plString &name)
{
    fAnimName = name;
}

plString plAGDetachCallbackMsg::GetAnimName()
{
    return fAnimName;
}
