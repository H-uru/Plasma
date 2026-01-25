# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/
# 
# A fairly simple script to handle looking at a console-like object.
# 
# Detailed setup instructions:
# 1. activator
#     The clickable which triggers looking at the console.
# 
# 2. camera
#     The camera we switch to when "activator" is clicked.
# 
# 3. respUsingConsole (optional)
#     A two-state responder triggered when starting or stopping to look at the console.
#     The first state is called when the avatar stops looking at the console.
#     The second state is called when the avatar starts looking at the console.
# 
# 4. sdlQuitConsole (optional)
#     The name of an SDL variable which, when changed, forces any player to stop
#     interacting with the console.

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

activator = ptAttribActivator(1, "Activate console")
camera = ptAttribSceneobject(2,"Console camera")
respUsingConsole = ptAttribResponder(3, "Responder: Looking at console (stop looking, look at) (optional)", statelist=["stoplook", "lookat"])
sdlQuitConsole = ptAttribString(4, "SDL: exit console on change")

Telescope = ptInputInterface()
lookingAtConsole = False

class xConsoleCamera(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
    
    def OnServerInitComplete(self):
        if sdlQuitConsole.value:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(sdlQuitConsole.value, 1, 1)
            ageSDL.sendToClients(sdlQuitConsole.value)
            ageSDL.setNotify(self.key, sdlQuitConsole.value, 0.0)

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        global lookingAtConsole
        if VARname == sdlQuitConsole.value and lookingAtConsole:
            self.IQuitConsole()

    def OnNotify(self,state,id,events):
        print("xConsoleCamera.OnNotify: state=%f id=%d events=%s" % (state, id, events))
        if state and id == activator.id and PtWasLocallyNotified(self.key) and PtFindAvatar(events) == PtGetLocalAvatar():
            self.ILookAtConsole()

    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IQuitConsole()
        elif controlKey == PlasmaControlKeys.kKeyMoveForward \
                or controlKey == PlasmaControlKeys.kKeyMoveBackward \
                or controlKey == PlasmaControlKeys.kKeyStrafeLeft \
                or controlKey == PlasmaControlKeys.kKeyStrafeRight \
                or controlKey == PlasmaControlKeys.kKeyRotateLeft \
                or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IQuitConsole()

    def ILookAtConsole(self):
        global Telescope
        global lookingAtConsole
        print("xConsoleCamera.ILookAtConsole")
        if lookingAtConsole:
            return
        activator.disable()
        lookingAtConsole = True
        for key in activator.value:
            key.getSceneObject().physics.suppress(True)
        Telescope.pushTelescope()
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        virtCam = ptCamera()
        virtCam.save(camera.sceneobject.getKey())
        #PtDisableMovementKeys()
        PtEnableControlKeyEvents(self.key)
        if respUsingConsole.value:
            respUsingConsole.run(self.key, state="lookat")

    def IQuitConsole(self):
        global Telescope
        global lookingAtConsole
        print("xConsoleCamera.IQuitConsole")
        if not lookingAtConsole:
            return
        lookingAtConsole = False
        Telescope.popTelescope()
        virtCam = ptCamera()
        virtCam.restore(camera.sceneobject.getKey())
        PtRecenterCamera()
        PtDisableControlKeyEvents(self.key)
        #PtEnableMovementKeys()
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        for key in activator.value:
            key.getSceneObject().physics.suppress(False)
        activator.enable()
        if respUsingConsole.value:
            respUsingConsole.run(self.key, state="stoplook")
