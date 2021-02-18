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
#include "plCameraMsg.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "pnKeyedObject/plKey.h"


//
//
// camera modifier message implementation
//
//

plCameraMsg::plCameraMsg() :
fTransTime(),
fSubject(),
fPipe(),
fActivated()
{
}

plCameraMsg::plCameraMsg(const plKey &s, const plKey &r, const double* t)  : 
fTransTime(),
fSubject(),
fPipe(),
fActivated(),
plMessage(s, r, t)
{
}

// IO
void plCameraMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);
    fCmd.Read(stream);
    fTransTime = stream->ReadLEDouble();
    fActivated = stream->ReadBool();
    fNewCam = mgr->ReadKey(stream);
    fTriggerer = mgr->ReadKey(stream);
    fConfig.Read(stream);   
}

void plCameraMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);
    fCmd.Write(stream);
    stream->WriteLEDouble(fTransTime);
    stream->WriteBool(fActivated);
    mgr->WriteKey(stream, fNewCam);
    mgr->WriteKey(stream, fTriggerer);
    fConfig.Write(stream);
}   

void plCameraConfig::Read(hsStream* stream)
{
    fAccel = stream->ReadLEFloat();
    fDecel = stream->ReadLEFloat();
    fVel = stream->ReadLEFloat();
    fFPAccel = stream->ReadLEFloat();
    fFPDecel = stream->ReadLEFloat();
    fFPVel = stream->ReadLEFloat();
    fFOVw = stream->ReadLEFloat(); 
    fFOVh = stream->ReadLEFloat();    
    fOffset.fX = stream->ReadLEFloat();
    fOffset.fY = stream->ReadLEFloat();
    fOffset.fZ = stream->ReadLEFloat();
    fWorldspace = stream->ReadBool();
}

void plCameraConfig::Write(hsStream* stream)
{
    stream->WriteLEFloat(fAccel);
    stream->WriteLEFloat(fDecel);
    stream->WriteLEFloat(fVel);
    stream->WriteLEFloat(fFPAccel);
    stream->WriteLEFloat(fFPDecel);
    stream->WriteLEFloat(fFPVel);
    stream->WriteLEFloat(fFOVw);
    stream->WriteLEFloat(fFOVh);
    stream->WriteLEFloat(fOffset.fX);
    stream->WriteLEFloat(fOffset.fY);
    stream->WriteLEFloat(fOffset.fZ);
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

