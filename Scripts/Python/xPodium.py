# -*- coding: utf-8 -*-
""" *==LICENSE==*

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

 *==LICENSE==* """

from Plasma import *
from PlasmaTypes import *

actSwitch01 = ptAttribActivator(1, "act: Podium Button")
respButtonOneshot = ptAttribResponder(2, "resp: Push Podium oneshot")
respList = ptAttribResponderList(3, "ResponderList", byObject=1)
stringFormat = ptAttribString(4, "Responder name format string")
stringVarName = ptAttribString(5, "Age SDL variable name")

class xPodium(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5245
        version = 3
        self.version = version
        PtDebugPrint("__init__xPodium v.", version, ".0")

    def OnFirstUpdate(self):
        if not stringVarName.value:
            PtDebugPrint("ERROR: xPodium.OnFirstUpdate():\tERROR: missing SDL var name")

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        if not PtGetPlayerList():
            ageSDL[stringVarName.value] = (0,)
        ageSDL.setNotify(self.key, stringVarName.value, 0.0)
        ageSDL.sendToClients(stringVarName.value)
        ageSDL.setFlags(stringVarName.value, 1, 1)

    def OnNotify(self, state, id, events):
        ageSDL = PtGetAgeSDL()
        xPodiumSpeech = ageSDL[stringVarName.value][0]

        if not state:
            return

        if id == actSwitch01.id:
            respButtonOneshot.run(self.key, events=events)
            return

        elif id == respButtonOneshot.id and self.sceneobject.isLocallyOwned():
            xPodiumSpeech -= 1
            if xPodiumSpeech < 0:
                xPodiumSpeech = len(respList.byObject) - 1
            respName = (stringFormat.value % xPodiumSpeech)
            if respName in respList.byObject:
                ageSDL[stringVarName.value] = (xPodiumSpeech,)
            else:
                PtDebugPrint("ERROR: xPodium Invalid speech %d selected!" % xPodiumSpeech)
                ageSDL[stringVarName.value] = (0,)

    def OnSDLNotify(self, VARname, SDLname, PlayerID, tag):
        if VARname != stringVarName.value:
            return

        ageSDL = PtGetAgeSDL()
        xPodiumSpeech = ageSDL[stringVarName.value][0]
        respName = (stringFormat.value % xPodiumSpeech)

        if xPodiumSpeech >= 0 and respName in respList.byObject:
            PtDebugPrint("xPodium: Playing speech", respName)
            respList.run(self.key, objectName=respName)
