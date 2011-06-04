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
#include "plCameraMsg.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plKey.h"


//
//
// camera modifier message implementation
//
//

plCameraMsg::plCameraMsg() : 
fNewCam(nil),
fTriggerer(nil),
fTransTime(0),
fSubject(nil),
fPipe(nil),
fConfig(nil),
fActivated(false)
{
}

plCameraMsg::plCameraMsg(const plKey &s, const plKey &r, const double* t)  : 
fNewCam(nil),
fTriggerer(nil),
fTransTime(0),
fSubject(nil),
fPipe(nil),
fConfig(nil),
fActivated(false),
plMessage(s, r, t)
{
}

// IO
void plCameraMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	fCmd.Read(stream);
	fTransTime = stream->ReadSwapDouble();
	fActivated = stream->ReadBool();
	fNewCam = mgr->ReadKey(stream);
	fTriggerer = mgr->ReadKey(stream);
	fConfig.Read(stream);	
}

void plCameraMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	fCmd.Write(stream);
	stream->WriteSwapDouble(fTransTime);
	stream->WriteBool(fActivated);
	mgr->WriteKey(stream, fNewCam);
	mgr->WriteKey(stream, fTriggerer);
	fConfig.Write(stream);
}	

void plCameraConfig::Read(hsStream* stream)
{
	fAccel = stream->ReadSwapFloat();
	fDecel = stream->ReadSwapFloat();
	fVel = stream->ReadSwapFloat();
	fFPAccel = stream->ReadSwapFloat();
	fFPDecel = stream->ReadSwapFloat();
	fFPVel = stream->ReadSwapFloat();
	fFOVw = stream->ReadSwapFloat(); 
	fFOVh = stream->ReadSwapFloat();	
	fOffset.fX = stream->ReadSwapFloat();
	fOffset.fY = stream->ReadSwapFloat();
	fOffset.fZ = stream->ReadSwapFloat();
	fWorldspace = stream->ReadBool();
}

void plCameraConfig::Write(hsStream* stream)
{
	stream->WriteSwapFloat(fAccel);
	stream->WriteSwapFloat(fDecel);
	stream->WriteSwapFloat(fVel);
	stream->WriteSwapFloat(fFPAccel);
	stream->WriteSwapFloat(fFPDecel);
	stream->WriteSwapFloat(fFPVel);
	stream->WriteSwapFloat(fFOVw);
	stream->WriteSwapFloat(fFOVh);
	stream->WriteSwapFloat(fOffset.fX);
	stream->WriteSwapFloat(fOffset.fY);
	stream->WriteSwapFloat(fOffset.fZ);
	stream->WriteBool(fWorldspace);
}







// IO
void plCameraTargetFadeMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	fSubject = mgr->ReadKey(stream);
	fFadeOut = stream->ReadBool();
}

void plCameraTargetFadeMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	mgr->WriteKey(stream, fSubject);
	stream->WriteBool(fFadeOut);
}	

// IO
void plIfaceFadeAvatarMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	fSubject = mgr->ReadKey(stream);
	fFadeOut = stream->ReadBool();
	fEnable = stream->ReadBool();
	fDisable = stream->ReadBool();
}

void plIfaceFadeAvatarMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	mgr->WriteKey(stream, fSubject);
	stream->WriteBool(fFadeOut);
	stream->WriteBool(fEnable);
	stream->WriteBool(fDisable);
}	

