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
Module: xTelescope
Age: global
Date: April 9, 2002
Author: Mark DeForest, Bill Slease, Doug McBride
This is tha handler for the standard telescope
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "Activate Telescope",netForce=1)
Camera = ptAttribSceneobject(2,"Telescope camera")
Behavior = ptAttribBehavior(3, "Telescope behavior (multistage)",netForce=1)
Vignette = ptAttribString(4,"Vignette dialog - by Name")

# ---------
# globals


LocalAvatar = None
boolScopeOperator = 0
boolOperated = 0
Telescope = ptInputInterface()

class xTelescope(ptModifier):
    "Standard telescope modifier class"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 201
        
        version = 4
        minorVersion = 2
        self.version = version
        PtDebugPrint("__init__xTelescope v%d.%d" % (version,minorVersion) )

    def OnFirstUpdate(self):
        self.SDL.setDefault("boolOperated",(0,))
        self.SDL.setDefault("OperatorID",(-1,))
        self.SDL.sendToClients("boolOperated")
        self.SDL.sendToClients("OperatorID")


    def Load(self):
        global boolScopeOperated

        solo = true
        if len(PtGetPlayerList()):
            solo = false

        boolOperated = self.SDL["boolOperated"][0]
        if boolOperated:
            if solo:
                PtDebugPrint("xTelescope.Load():\tboolOperated=%d but no one else here...correcting" % boolOperated,level=kDebugDumpLevel)
                boolOperated = 0
                self.SDL["boolOperated"] = (0,)
                self.SDL["OperatorID"] = (-1,)
                Activate.enable()
            else:
                Activate.disable()
                PtDebugPrint("xTelescope.Load():\tboolOperated=%d, disabling telescope clickable" % boolOperated,level=kDebugDumpLevel)

    def AvatarPage(self, avObj, pageIn, lastOut):
        "reset scope accessibility if scope user quits or crashes"
        global boolScopeOperated
        
        if pageIn:
            return
            
        avID = PtGetClientIDFromAvatarKey(avObj.getKey())
        if avID == self.SDL["OperatorID"][0]:
            Activate.enable()
            self.SDL["OperatorID"] = (-1,)
            self.SDL["boolOperated"] = (0,)
            PtDebugPrint("xTelescope.AvatarPage(): telescope operator paged out, reenabled telescope.",level=kDebugDumpLevel)
        else:
            return
            
    def __del__(self):
        "unload the dialog that we loaded"
        PtUnloadDialog(Vignette.value)

    def OnNotify(self,state,id,events):
        "Activated... start telescope"
        global LocalAvatar
        global boolScopeOperator
        PtDebugPrint("xTelescope:OnNotify  state=%f id=%d events=" % (state,id),events,level=kDebugDumpLevel)
        if state and id == Activate.id and PtWasLocallyNotified(self.key):
            LocalAvatar = PtFindAvatar(events)
            self.IStartTelescope()
        # check if its an advance stage notify
        for event in events:
            if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                if boolScopeOperator:
                    self.IEngageTelescope()
                    boolScopeOperator = 0
                break


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
        global boolScopeOperator
        # disable the activator (only one in the telescope at a time)
        PtSendKIMessage(kDisableKIandBB,0)
        Activate.disable()
        boolScopeOperator = 1  # me! I'm the operator
        self.SDL["boolOperated"] = (1,)
        avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
        self.SDL["OperatorID"] = (avID,)
        PtDebugPrint("xTelescope.OnNotify:\twrote SDL - scope operator id = ", avID,level=kDebugDumpLevel)
        # start the behavior
        Behavior.run(LocalAvatar)

    def IEngageTelescope(self):
        global Telescope

        Telescope.pushTelescope()
        "After the behavior gets our eyes in the telescope, engage ourselves with the camera"
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
        global boolScopeOperator
        global Telescope

        Telescope.popTelescope()
        # exit every thing
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtHideDialog(Vignette.value)
        virtCam = ptCamera()
        virtCam.restore(Camera.sceneobject.getKey())
        PtRecenterCamera()
        # exit behavior...which is in the next stage
        # ... but just gotoStage the last stage by number because of a bug in the SDL send state
        Behavior.gotoStage(LocalAvatar,2)
        #Behavior.nextStage(LocalAvatar)
        #disable the Control key events
        PtDisableControlKeyEvents(self.key)
        # re-enable the telescope for someone else to use
        boolScopeOperator = 0
        self.SDL["boolOperated"] = (0,)
        self.SDL["OperatorID"] = (-1,)
        #Re-enable first person camera
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        PtAtTimeCallback(self.key,3,1) # wait for player to finish exit one-shot, then reenable clickable
        PtDebugPrint("xTelescope.IQuitTelescope:\tdelaying clickable reenable",level=kDebugDumpLevel)
        
    def OnTimer(self,id):
        if id==1:
            Activate.enable()
            PtDebugPrint("xTelescope.OnTimer:\tclickable reenabled",level=kDebugDumpLevel)
            PtSendKIMessage(kEnableKIandBB,0)

