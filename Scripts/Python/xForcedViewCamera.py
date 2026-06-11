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
"""
Module: xForcedViewCamera
Age: global
Date: November 23, 2025
Author: Darryl Pogue
Script to force a camera viewpoint from a clickable, with an optional vignette.
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
Activator = ptAttribActivator(1, "Activation clickable")
Camera = ptAttribSceneobject(2, "Viewpoint camera")
Vignette = ptAttribString(3, "Optional vignette dialog - by Name", default=None)

class xForcedViewCamera(ptModifier):
    """
    Modifier class to force a specific viewpoint camera upon clicking an
    activator, with an optional GUI vignette dialog.

    This is similar to how telescopes work, but without the multi-stage avatar
    behaviour and without a player taking ownership of the clickable (i.e.,
    multiple players can activate the viewpoint camera concurrently).
    """

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 231
        version = 1
        minorVersion = 0
        self.version = version
        PtDebugPrint(f"__init__xForcedViewCamera v{version}.{minorVersion}")

    def __del__(self):
        """
        Unload the vignette dialog (if one was specified)
        """
        if Vignette.value:
            PtUnloadDialog(Vignette.value)

    def OnInit(self):
        self.inputInterface = ptInputInterface()

    def OnNotify(self, state, id, events):
        """
        Notifications from the clickable activator
        """
        PtDebugPrint(f"xForcedViewCamera.OnNotify  state={state} id={id} events=", events, level=kDebugDumpLevel)
        if state and id == Activator.id and PtWasLocallyNotified(self.key):
            self.IEngageCamera()


    def OnGUINotify(self, id, control, event):
        """
        Notifications from the vignette
        """
        PtDebugPrint(f"xForcedViewCamera.OnGUINotify id={id}, event={event} control=", control, level=kDebugDumpLevel)
        if event == kDialogLoaded:
            # if the dialog was just loaded then show it
            control.show()

    def OnControlKeyEvent(self, controlKey, activeFlag):
        """
        Notifications from key presses
        """
        exitKeys = [
            PlasmaControlKeys.kKeyExitMode,
            PlasmaControlKeys.kKeyMoveBackward,
            PlasmaControlKeys.kKeyRotateLeft,
            PlasmaControlKeys.kKeyRotateRight
        ]

        if controlKey in exitKeys:
            self.IExitCamera()

    def IEngageCamera(self):
        """
        Enter the forced camera view
        """
        PtDebugPrint("xForcedViewCamera.IEngageCamera", level=kDebugDumpLevel)

        # Disable the blackbar, the clickable (locally), and go into telescope mode
        PtSendKIMessage(kDisableKIandBB, 0)
        Activator.disable()
        self.inputInterface.pushTelescope()

        # Disable First Person Camera
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()

        # Set camera to the viewpoint camera
        virtCam = ptCamera()
        virtCam.save(Camera.sceneobject.getKey())

        # Show the vignette (if specified)
        if Vignette.value:
            PtLoadDialog(Vignette.value, self.key)
            if PtIsDialogLoaded(Vignette.value):
                PtShowDialog(Vignette.value)

        # Get control key events
        PtEnableControlKeyEvents(self.key)

    def IExitCamera(self):
        """
        Disengage and exit the forced camera view
        """
        PtDebugPrint("xForcedViewCamera.IExitCamera", level=kDebugDumpLevel)

        # Exit every thing
        self.inputInterface.popTelescope()
        if Vignette.value:
            PtHideDialog(Vignette.value)

        # Restore the previous camera
        virtCam = ptCamera()
        virtCam.restore(Camera.sceneobject.getKey())
        PtRecenterCamera()

        # Disable the Control key events
        PtDisableControlKeyEvents(self.key)

        # Re-enable first person camera
        cam = ptCamera()
        cam.enableFirstPersonOverride()

        # Re-enable the blackbar and clickable
        Activator.enable()
        PtSendKIMessage(kEnableKIandBB, 0)
