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
showOnTrue = ptAttribBoolean(2, "Show on true", default=True)
defaultValue = ptAttribBoolean(3, "Default setting", default=False)
evalOnFirstUpdate = ptAttribBoolean(4, "Eval On First Update?", default=False)

class xAgeSDLBoolShowHide(ptMultiModifier, object):
    """Shows or hides attached SceneObjects based on the value of an SDL boolean variable"""

    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5037
        self.version = 2

    def OnBackdoorMsg(self, target, param):
        if sdlName.value:
            if target == sdlName.value:
                if param.lower() in {"on", "1", "true"}:
                    self._EnableObject()
                elif param.lower() in {"off", "0", "false"}:
                    self._DisableObject()
                else:
                    PtDebugPrint("xAgeSDLBoolShowHide.OnBackDoorMsg:  Received unexpected parameter on %s" % self.sceneobject.getName())

    def OnFirstUpdate(self):
        if evalOnFirstUpdate.value:
            self._Setup()

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        if VARname == sdlName.value:
            ageSDL = PtGetAgeSDL()
            self.sdl_value = ageSDL[sdlName.value][0]

    def OnServerInitComplete(self):
        if not evalOnFirstUpdate.value:
            self._Setup()

    def _DisableObject(self):
        PtDebugPrint("xAgeSDLBoolShowHide.DisableObject:  Attempting to disable drawing and collision on %s..." % self.sceneobject.getName(), level=kDebugDumpLevel)
        self.sceneobject.draw.disable()
        self.sceneobject.physics.suppress(True)

    def _EnableObject(self):
        PtDebugPrint("xAgeSDLBoolShowHide.EnableObject:  Attempting to enable drawing and collision on %s..." % self.sceneobject.getName(), level=kDebugDumpLevel)
        self.sceneobject.draw.enable()
        self.sceneobject.physics.suppress(False)

    def _Setup(self):
        ageSDL = PtGetAgeSDL()
        if not ageSDL:
            PtDebugPrint("xAgeSDLBoolShowHide._Setup():\tAgeSDLHook is null... You've got problems, friend.")
            self.sdl_value = defaultValue.value # start at default
            return None

        if sdlName.value:
            # So, apparently, Cyan's artists like trailing whitespace...
            if sdlName.value.find(" ") != -1:
                PtDebugPrint("xAgeSDLBoolShowHide._Setup():\tWARNING: %s's SDL variable '%s' has whitespace. Removing!" % (self.sceneobject.getName(), sdlName.value))
                sdlName.value = sdlName.value.replace(" ", "")

            ageSDL.setFlags(sdlName.value, 1, 1)
            ageSDL.sendToClients(sdlName.value)
            ageSDL.setNotify(self.key, sdlName.value, 0.0)

            # Sometimes, Cyan's artists just fail.
            try:
                self.sdl_value = ageSDL[sdlName.value][0]
            except LookupError:
                PtDebugPrint("xAgeSDLBoolShowHide._Setup():\tVariable '%s' is invalid on object '%s'" % (sdlName.value, self.sceneobject.getName()))
                self.sdl_value = defaultValue.value
        else:
            self.sdl_value = defaultValue.value # start at default
            raise RuntimeError("You forgot to set the SDL Variable Name!")

    def _set_sdl_value(self, value):
        if value ^ showOnTrue.value:
            self._DisableObject()
        else:
            self._EnableObject()
    sdl_value = property(fset=_set_sdl_value)
