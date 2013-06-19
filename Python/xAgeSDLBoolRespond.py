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

sdlName = ptAttribString(1, "Age SDL Var Name")
respTrue = ptAttribResponder(2, "Run if bool true:")
respFalse = ptAttribResponder(3, "Run if bool false:")
vmFastFwd = ptAttribBoolean(4, "F-Forward on VM notify", default=True)
initFastFwd = ptAttribBoolean(5, "F-Forward on Init", default=True)
defaultValue = ptAttribBoolean(6, "Default setting", default=False)
evalOnFirstUpdate = ptAttribBoolean(7, "Init SDL On First Update?", default=False)
useVaultSDL = ptAttribBoolean(8, "Use Vault SDL?", default=False)


class xAgeSDLBoolRespond(ptResponder):
    """Runs a given responder based on the value of an SDL boolean"""

    id = 5034
    version = 2

    def OnBackdoorMsg(self, target, param):
        if target == sdlName.value:
            value = param.lower() in {"on", "true", "1"}
            self._Execute(value, False)

    def OnFirstUpdate(self):
        if evalOnFirstUpdate.value:
            self._Setup()

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        if VARname != sdlName.value:
            return

        ageSDL = PtGetAgeSDL()
        value = ageSDL[sdlName.value][0]
        PtDebugPrint("xAgeSDLBoolRespond.OnSDLNotify():\tVARname:%s, SDLname:%s, value:%d, playerID:%d" % (VARname, SDLname, value, playerID), level=kDebugDumpLevel)

        # A playerID of zero indicates a vault mangler change
        if playerID:
            try:
                avatar = PtGetAvatarKeyFromClientID(playerID).getSceneObject()
            except:
                avatar = None
            ff = False
        else:
            avatar = None
            ff = vmFastFwd.value
        self._Execute(value, ff, avatar)

    def OnServerInitComplete(self):
        if not evalOnFirstUpdate.value:
            self._Setup()

    def _Execute(self, value, ff, avatar=None):
        resps = ("FALSE", "TRUE")
        PtDebugPrint("xAgeSDLBoolRespond._Execute():\tRunning %s responder on %s ff=%d" % (resps[int(value)], self.sceneobject.getName(), ff), level=kDebugDumpLevel)
        if value:
            respTrue.run(self.key, avatar=avatar, fastforward=ff)
        else:
            respFalse.run(self.key, avatar=avatar, fastforward=ff)

    def _Setup(self):
        if not sdlName.value:
            raise self._Raise("Missing SDL variable name")

        if useVaultSDL.value:
            vault = ptAgeVault()
            if not vault:
                self._Raise("Age Vault for '%s' is None" % PtGetAgeName())
            ageSDL = vault.getAgeSDL()
            if not ageSDL:
                self._Raise("Vault SDL for %s' is None" % PtGetAgeName())

            var = ageSDL.findVar(sdlName.value)
            if not var:
                self._Raise("Invalid variable '%s' for descriptor '%s'" % (sdlName.value, ageSDL.getName()))
            self._Execute(var.getBool(), initFastFwd.value)
        else:
            ageSDL = PtGetAgeSDL()
            if not ageSDL:
                self._Raise("xAgeSDLBoolRespond._Initialize():\tAgeSDL is None. Initing '%s' to its default" % self.sceneobject.getName())
                return

            # Setup the SDL for notifications
            ageSDL.setFlags(sdlName.value, 1, 1)
            ageSDL.sendToClients(sdlName.value)
            ageSDL.setNotify(self.key, sdlName.value, 0.0)

            # Now do the nasty
            self._Execute(ageSDL[sdlName.value][0], initFastFwd.value)

    def _Raise(self, msg):
        self._Execute(defaultValue.value, initFastFwd.value)
        raise RuntimeEror(msg)
