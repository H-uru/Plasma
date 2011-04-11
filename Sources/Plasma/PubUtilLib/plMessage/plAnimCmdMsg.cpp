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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsTypes.h"
#include "plAnimCmdMsg.h"
#include "hsStream.h"

plAnimCmdMsg::~plAnimCmdMsg()
{
	ClearCmd();
	delete [] fAnimName;
	delete [] fLoopName;
}


void plAnimCmdMsg::ClearCmd() 
{ 
	plMessageWithCallbacks::Clear();
	fCmd.Clear(); 
}

void plAnimCmdMsg::SetAnimName(const char *name)
{
	delete [] fAnimName;
	fAnimName = hsStrcpy(name);
}

const char *plAnimCmdMsg::GetAnimName()
{
	return fAnimName;
}

void plAnimCmdMsg::SetLoopName(const char *name)
{
	delete [] fLoopName;
	fLoopName = hsStrcpy(name);
}

const char *plAnimCmdMsg::GetLoopName()
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
	stream->ReadSwap(&fBegin);
	stream->ReadSwap(&fEnd);
	stream->ReadSwap(&fLoopEnd);
	stream->ReadSwap(&fLoopBegin);
	stream->ReadSwap(&fSpeed);
	stream->ReadSwap(&fSpeedChangeRate);
	stream->ReadSwap(&fTime);

	fAnimName = stream->ReadSafeString();
	fLoopName = stream->ReadSafeString();
}

void plAnimCmdMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessageWithCallbacks::Write(stream, mgr);

	fCmd.Write(stream);
	stream->WriteSwap(fBegin);
	stream->WriteSwap(fEnd);
	stream->WriteSwap(fLoopEnd);
	stream->WriteSwap(fLoopBegin);
	stream->WriteSwap(fSpeed);
	stream->WriteSwap(fSpeedChangeRate);
	stream->WriteSwap(fTime);

	stream->WriteSafeString(fAnimName);
	stream->WriteSafeString(fLoopName);
}

/////////////////////////////////////////////////////////////////////////////////////////

plAGCmdMsg::~plAGCmdMsg()
{
	ClearCmd();
	delete [] fAnimName;
}

void plAGCmdMsg::SetAnimName(const char *name)
{
	delete [] fAnimName;
	fAnimName = hsStrcpy(name);
}

const char *plAGCmdMsg::GetAnimName()
{
	return fAnimName;
}

void plAGCmdMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);

	fCmd.Read(stream);
	stream->ReadSwap(&fBlend);
	stream->ReadSwap(&fBlendRate);
	stream->ReadSwap(&fAmp);
	stream->ReadSwap(&fAmpRate);

	fAnimName = stream->ReadSafeString();
}

void plAGCmdMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	fCmd.Write(stream);
	stream->WriteSwap(fBlend);
	stream->WriteSwap(fBlendRate);
	stream->WriteSwap(fAmp);
	stream->WriteSwap(fAmpRate);

	stream->WriteSafeString(fAnimName);
}

/////////////////////////////////////////////////////////////////////////////////////

plAGDetachCallbackMsg::~plAGDetachCallbackMsg()
{
	delete [] fAnimName;
}

void plAGDetachCallbackMsg::SetAnimName(const char *name)
{
	fAnimName = hsStrcpy(name);
}

char *plAGDetachCallbackMsg::GetAnimName()
{
	return fAnimName;
}
