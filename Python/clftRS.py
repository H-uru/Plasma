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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
Module: Riven scope
Age: Cleft
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "Activate Telescope",netForce=1)
Camera = ptAttribSceneobject(2,"Telescope camera")
Vignette = ptAttribString(4,"Vignette dialog - by Name")

# ---------
# globals


LocalAvatar = None
Telescope = ptInputInterface()

class clftRS(ptModifier):
    "Standard telescope modifier class"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 150
        version = 4
        minorVersion = 1
        self.version = version
        PtDebugPrint("__init__clftRS v%d.%d" % (version,minorVersion) )

    def OnFirstUpdate(self):
        pass

    def __del__(self):
        "unload the dialog that we loaded"
        PtUnloadDialog(Vignette.value)

    def OnNotify(self,state,id,events):
        "Activated... start telescope"
        global LocalAvatar
        PtDebugPrint("xTelescope:OnNotify  state=%f id=%d events=" % (state,id),events,level=kDebugDumpLevel)
        if state and id == Activate.id and PtWasLocallyNotified(self.key):
            LocalAvatar = PtFindAvatar(events)
            self.IStartTelescope()


    def OnGUINotify(self,id,control,event):
        "Notifications from the vignette"
        PtDebugPrint("GUI Notify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel)
        if event == kDialogLoaded:
            # if the dialog was just loaded then show it
            control.show()

    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IQuitTelescope()
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IQuitTelescope()
 
    def IStartTelescope(self):
        "Start the action of looking at the telescope"
        global LocalAvatar
        # disable the activator (only one in the telescope at a time)
        PtSendKIMessage(kDisableKIandBB,0)
        self.IEngageTelescope()

    def IEngageTelescope(self):
        global Telescope

        Telescope.pushTelescope()
        # Disable First Person Camera
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        # set camera to telescope
        virtCam = ptCamera()
        virtCam.save(Camera.sceneobject.getKey())
        # show the cockpit
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtLoadDialog(Vignette.value,self.key)
            if ( PtIsDialogLoaded(Vignette.value) ):
                PtShowDialog(Vignette.value)
        # get control key events
        PtEnableControlKeyEvents(self.key)

    def IQuitTelescope(self):
        "Disengage and exit the telescope mode"
        global LocalAvatar
        global Telescope

        Telescope.popTelescope()
        # exit every thing
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtHideDialog(Vignette.value)
        virtCam = ptCamera()
        virtCam.restore(Camera.sceneobject.getKey())
        PtRecenterCamera()
        #disable the Control key events
        PtDisableControlKeyEvents(self.key)
        #Re-enable first person camera
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        PtAtTimeCallback(self.key,3,1) # wait for player to finish exit one-shot, then reenable clickable
        PtDebugPrint("xTelescope.IQuitTelescope:\tdelaying clickable reenable",level=kDebugDumpLevel)
        
    def OnTimer(self,id):
        if id==1:
            PtSendKIMessage(kEnableKIandBB,0)
