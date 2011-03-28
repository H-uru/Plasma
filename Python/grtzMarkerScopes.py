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
Module: grtzMarkerScope.py
Age: Great Zero
Date: Nov. 12, 2003
"""
MaxVersionNumber = 1
MinorVersionNumber = 2

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys


aTrigger  = ptAttribActivator(1,"Activate the scope", netForce=1)
aBehavior = ptAttribBehavior(2, "Telescope behavior (multistage)",netForce=1)

# ---------
# globals


LocalAvatar = None
boolScopeOperator = 0
boolOperated = 0
Telescope = ptInputInterface()


class grtzMarkerScopes(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 212
        self.version = MaxVersionNumber
        PtDebugPrint("grtzMarkerScope: Max version %d - minor version %d" % (MaxVersionNumber,MinorVersionNumber),level=kDebugDumpLevel)
        self.pendingClose = 0

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
                PtDebugPrint("grtzMarkerScope.Load():\tboolOperated=%d but no one else here...correcting" % boolOperated,level=kDebugDumpLevel)
                boolOperated = 0
                self.SDL["boolOperated"] = (0,)
                self.SDL["OperatorID"] = (-1,)
                aTrigger.enable()
            else:
                aTrigger.disable()
                PtDebugPrint("grtzMarkerScope.Load():\tboolOperated=%d, disabling telescope clickable" % boolOperated,level=kDebugDumpLevel)

    def AvatarPage(self, avObj, pageIn, lastOut):
        "reset scope accessibility if scope user quits or crashes"
        global boolScopeOperated
        
        if pageIn:
            return
            
        avID = PtGetClientIDFromAvatarKey(avObj.getKey())
        if avID == self.SDL["OperatorID"][0]:
            aTrigger.enable()
            self.SDL["OperatorID"] = (-1,)
            self.SDL["boolOperated"] = (0,)
            PtDebugPrint("grtzMarkerScope.AvatarPage(): telescope operator paged out, reenabled telescope.",level=kDebugDumpLevel)
        else:
            return
            
    def __del__(self):
        "unload the dialog that we loaded"
        PtUnloadDialog("MarkerGameGUI")

    def OnNotify(self,state,id,events):
        "Activated... start telescope"
        global LocalAvatar
        global boolScopeOperator
        PtDebugPrint("grtzMarkerScope:OnNotify  state=%f id=%d events=" % (state,id),events,level=kDebugDumpLevel)

        if id == (-1):
            if self.pendingClose == 1:
                self.pendingClose = 0
                self.IQuitTelescope()


        if state and id == aTrigger.id and PtWasLocallyNotified(self.key):
            LocalAvatar = PtFindAvatar(events)
            self.IStartTelescope()
        # check if its an advance stage notify
        for event in events:
            if event[0] == kMultiStageEvent and (event[1] == 0 or event[1] == 1) and event[2] == kAdvanceNextStage:
                if boolScopeOperator:
                    self.IEngageTelescope()
                    boolScopeOperator = 0
                break


    def OnGUINotify(self,id,control,event):
        "Notifications from the vignette"
        PtDebugPrint("grtzMarkerScope.GUI Notify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel)
        if event == kDialogLoaded:
            # if the dialog was just loaded then show it
            control.show()


    def OnControlKeyEvent(self,controlKey,activeFlag):
        self.pendingClose = 1

    def IStartTelescope(self):
        "Start the action of looking at the telescope"
        global LocalAvatar
        global boolScopeOperator
        # disable the activator (only one in the telescope at a time)
        PtSendKIMessage(kDisableKIandBB,0)
        aTrigger.disable()
        boolScopeOperator = 1  # me! I'm the operator
        self.SDL["boolOperated"] = (1,)
        avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
        self.SDL["OperatorID"] = (avID,)
        PtDebugPrint("grtzMarkerScope.OnNotify:\twrote SDL - scope operator id = ", avID,level=kDebugDumpLevel)
        # start the behavior
        aBehavior.run(LocalAvatar)

    def IEngageTelescope(self):
        global Telescope

        Telescope.pushTelescope()
        "After the behavior gets our eyes in the telescope, engage ourselves with the camera"
        # show the cockpit
        PtLoadDialog("MarkerGameGUI")
        if ( PtIsDialogLoaded("MarkerGameGUI") ):
            PtShowDialog("MarkerGameGUI")
        # get control key events
        PtEnableControlKeyEvents(self.key)

    def IQuitTelescope(self):
        "Disengage and exit the telescope mode"
        global LocalAvatar
        global boolScopeOperator
        global Telescope

        Telescope.popTelescope()
        # exit every thing
        note = ptNotify(self.key)
        note.addVarNumber("Quit",1)
        note.netPropagate(0)
        note.netForce(0)
        note.send()
        # exit behavior...which is in the next stage
        PtAtTimeCallback(self.key,0.5,1) # wait for player to finish exit one-shot, then reenable clickable
        #disable the Control key events
        PtDisableControlKeyEvents(self.key)
        # re-enable the telescope for someone else to use
        boolScopeOperator = 0
        self.SDL["boolOperated"] = (0,)
        self.SDL["OperatorID"] = (-1,)
        PtDebugPrint("grtzMarkerScope.IQuitMarkerScope:\tdelaying clickable reenable",level=kDebugDumpLevel)
        
    def OnTimer(self,id):
        global LocalAvatar
        if id == 1:
            # just use gotoStage the last stage by number because of a bug in the SDL send state
            #aBehavior.nextStage(LocalAvatar)
            aBehavior.gotoStage(LocalAvatar,3)
            PtAtTimeCallback(self.key,3,2) # wait for player to finish exit one-shot, then reenable clickable
        if id == 2:
            aTrigger.enable()
            PtDebugPrint("grtzMarkerScope.OnTimer:\tclickable reenabled",level=kDebugDumpLevel)
            PtSendKIMessage(kEnableKIandBB,0)
