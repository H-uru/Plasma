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
from PlasmaVaultConstants import *

chronName = ptAttribString(1, "Chronicle Name")
respTrue = ptAttribResponder(2, "Run if TRUE")
respFalse = ptAttribResponder(3, "Run if FALSE")
initFastFwd = ptAttribBoolean(4, "F-Forward on Init", default=True)
defaultValue = ptAttribBoolean(5, "Default setting", default=False)

class xPlayerChronicleRespond(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = -1
        self.version = 1

    def OnServerInitComplete(self):
        if chron := ptVault().findChronicleEntry(chronName.value):
            self._Eval(chron, ff=initFastFwd.value)
        else:
            self._Eval(defaultValue.value, ff=initFastFwd.value)

    def OnVaultEvent(self, event, tupData):
        if event == PtVaultCallbackTypes.kVaultNodeSaved:
            if chron := tupData[0].upcastToChronicleNode():
                if chron.getName() == chronName.value:
                    self._Eval(chron)

    def OnBackdoorMsg(self, target, param):
        if target == chronName:
            self._Eval(param)

    def _Eval(self, value, ff=False):
        if isinstance(value, ptVaultChronicleNode):
            value = value.getValue().strip().lower()
        if isinstance(value, str):
            if value in {"1", "true", "on", "show", "yes"}:
                value = True
            elif value in {"0", "false", "off", "hide", "no", ""}:
                value = False
            else:
                PtDebugPrint(f"xPlayerChronicleRespond._Eval():\tUnhandled value string '{value}', doing nothing")
                return
        assert isinstance(value, bool), "value should be a boolean"

        if value:
            PtDebugPrint(f"xPlayerChronicleRespond._Eval():\tRunning TRUE responder on '{self.sceneobject.getName()}' {ff=}", level=kDebugDumpLevel)
            respTrue.run(self.key, fastforward=ff)
        else:
            PtDebugPrint(f"xPlayerChronicleRespond._Eval():\tRunning FALSE responder on '{self.sceneobject.getName()}' {ff=}", level=kDebugDumpLevel)
            respFalse.run(self.key, fastforward=ff)
