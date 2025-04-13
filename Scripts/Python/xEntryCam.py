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

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *

actRegionSensor = ptAttribActivator(1, "act: Camera Region Sensor")
camera = ptAttribSceneobject(2, "Entry Camera")
undoFirstPerson = ptAttribBoolean(3, "Override the player's first person camera setting?", default=True)

class xEntryCam(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 900378302
        self.version = 1

        self._entryCamActive = False
        self._linkedIn = False

        PtDebugPrint(f"xEntryCam.__init__() {self.version=}", level=kWarningLevel)

    def _DisableEnterRgn(self, reason="unknown"):
        PtDebugPrint(f"xEntryCam._DisableEnterRgn(): [{self.sceneobject.getName()}] Disabling entry region... [Reason: {reason}]", level=kWarningLevel)

        # We're tracking the enable/disable state of the entry camera region in
        # a variable because disabling the activator seems to propagate out to
        # the host scene object, disabling both regions. We need the exit region
        # to be active, though, to ensure the entry camera is popped off on exit,
        # not when we link-in... that defeats the purpose!
        self._linkedIn = True
        PtGetLocalAvatar().avatar.unRegisterForBehaviorNotify(self.key)

    def OnServerInitComplete(self):
        # When we finish linking in, all entry regions will be disabled.
        PtGetLocalAvatar().avatar.registerForBehaviorNotify(self.key)

        # No need for server arbitration on these because it's all local.
        actRegionSensor.volumeSensorNoArbitration()

    def OnBehaviorNotify(self, type, id, state):
        PtDebugPrint(f"xEntryCam.OnBehaviorNotify(): {type=} {id=} {state=}", level=kDebugDumpLevel)
        if type == PtBehaviorTypes.kBehaviorTypeLinkIn:
            self._DisableEnterRgn("done linking in")

    def OnNotify(self, state, id, events):
        if PtFindAvatar(events) != PtGetLocalAvatar():
            return
        if id != actRegionSensor.id:
            return

        # Search for a collision event in the notification's event records. The second item in
        # the record is the entered state. Note that the engine is currently passing integers
        # instead of proper booleans, and a value of `None` indicates that no collision
        # event was found at all. Be careful!
        rgnEntered = next((event[1] for event in events if event[0] == kCollisionEvent), None)

        if rgnEntered is None:
            PtDebugPrint(f"xEntryCam.OnNotify(): [{self.sceneobject.getName()}] Ignoring spurious notification", level=kDebugDumpLevel)
        elif rgnEntered:
            if self._linkedIn:
                PtDebugPrint(f"xEntryCam.OnNotify(): [{self.sceneobject.getName()}] Ignoring entry cam enter trigger.", level=kDebugDumpLevel)
                return

            self._DisableEnterRgn("entry cam triggered")
            PtDebugPrint(f"xEntryCam.OnNotify(): [{self.sceneobject.getName()}] Pushing entry camera {camera.value.getName()}", level=kWarningLevel)
            if not self._entryCamActive:
                virtCam = ptCamera()
                virtCam.save(camera.value.getKey())
                # This will only force the player out of first person if the age creator says that we should do so *and* the player
                # has *not* enabled the "stay in first person" flag. If the player has "stay in first person" enabled, then they
                # will not be forced into third person for this entry camera.
                if undoFirstPerson.value:
                    virtCam.undoFirstPerson()
            self._entryCamActive = True
        else:
            if not self._entryCamActive:
                PtDebugPrint(f"xEntryCam.OnNotify(): [{self.sceneobject.getName()}] Ignoring entry cam region exit trigger.", level=kDebugDumpLevel)
                return

            PtDebugPrint(f"xEntryCam.OnNotify(): [{self.sceneobject.getName()}] Popping entry camera {camera.value.getName()}", level=kWarningLevel)
            virtCam = ptCamera()
            virtCam.restore(camera.value.getKey())
            self._entryCamActive = False
