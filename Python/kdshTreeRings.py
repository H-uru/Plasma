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
Module: kdshTreeRings
Age: Kadish Tolesa
Date: March 2002
Author: Doug McBride
As players engage and use the GUI, this sets the proper SDL values in the Kadish.sdl file
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "Activate Telescope", netForce=1)
Camera = ptAttribSceneobject(2,"Telescope camera")
Behavior = ptAttribBehavior(3, "Telescope behavior (multistage)",netForce=1)

ScopeNumber = ptAttribInt(4, "Scope Number (1-3)")

actResetBtn = ptAttribActivator(5, "act:Reset(only on scope3")
respResetBtn = ptAttribResponder(6, "resp:Reset Button",['Reset','OnInit'])

respSfxRings = ptAttribResponder(7, "resp: Sfx Rings")
OnlyOneOwner = ptAttribSceneobject(8,"OnlyOneOwner") #ensures that after a oneshots, only one client toggles the SDL values

# globals
LocalAvatar = None
boolScopeOperator = 0
boolOperated = 0


Telescope = ptInputInterface()

kGUIRingTurnLeft = 110
kGUIRingTurnCenter = 111
kGUIRingTurnRight = 112

oldbearing = 0
OuterRing = 0
MiddleRing = 0
InnerRing = 0




class kdshTreeRings(ptModifier):
    "Standard telescope modifier class"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5228
        
        version = 13
        self.version = version
        print "__init__kdshTreeRings v.", version,".1"

    def OnFirstUpdate(self):
        PtLoadDialog("kdshScope0" + str(ScopeNumber.value), self.key, "Kadish")       
        #~ print "kdshTreeRings: Loading dialog ", ("kdshScope0" + str(ScopeNumber.value))              
    """  ###Commented this out because it was never available anyway (note the 2nd defn)!!!
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        if ageSDL == None:
            print "kdshTreeRings.OnFirstUpdate():\tERROR---missing age SDL (%s)" % varstring.value
      
        if ScopeNumber.value == 1:
            ageSDL.sendToClients("boolOperatedScope01")
            ageSDL.sendToClients("OperatorIDScope01")
            ageSDL.setFlags("boolOperatedScope01",1,1)  
            ageSDL.setFlags("OperatorIDScope01",1,1)  


            ageSDL.sendToClients("OuterRing01")
            ageSDL.sendToClients("MiddleRing01")
            ageSDL.sendToClients("InnerRing01")
            ageSDL.setFlags("OuterRing01",1,1)
            ageSDL.setFlags("MiddleRing01",1,1)
            ageSDL.setFlags("InnerRing01",1,1)

        elif ScopeNumber.value == 2:
            ageSDL.sendToClients("boolOperatedScope02")
            ageSDL.sendToClients("OperatorIDScope02")
            ageSDL.setFlags("boolOperatedScope02",1,1)  
            ageSDL.setFlags("OperatorIDScope02",1,1)  
            
            ageSDL.sendToClients("OuterRing02")
            ageSDL.sendToClients("MiddleRing02")
            ageSDL.sendToClients("InnerRing02")
            ageSDL.setFlags("OuterRing02",1,1)
            ageSDL.setFlags("MiddleRing02",1,1)
            ageSDL.setFlags("InnerRing02",1,1)

            
        elif ScopeNumber.value == 3:
            ageSDL.sendToClients("boolOperatedScope03")
            ageSDL.sendToClients("OperatorIDScope03")
            ageSDL.setFlags("boolOperatedScope03",1,1)  
            ageSDL.setFlags("OperatorIDScope03",1,1)  

            ageSDL.sendToClients("OuterRing03")
            ageSDL.sendToClients("MiddleRing03")
            ageSDL.sendToClients("InnerRing03")            
            ageSDL.setFlags("OuterRing03",1,1)
            ageSDL.setFlags("MiddleRing03",1,1)
            ageSDL.setFlags("InnerRing03",1,1)
"""

    def OnServerInitComplete(self):
        global OuterRing
        global MiddleRing
        global InnerRing        
        global boolScopeOperated
        ageSDL = PtGetAgeSDL()             

        OuterRing = ageSDL["OuterRing0" + str(ScopeNumber.value)][0]
        MiddleRing = ageSDL["MiddleRing0" + str(ScopeNumber.value)][0]
        InnerRing = ageSDL["InnerRing0" + str(ScopeNumber.value)][0]
        
        print "Current %s Ring settings:" % (ScopeNumber.value)
        print "/tOuterRing: ", OuterRing
        print "/tMiddleRing: ", MiddleRing
        print "/tInnerRing: ", InnerRing

        solo = true
        if len(PtGetPlayerList()):
            solo = false

        boolOperated = ageSDL["boolOperatedScope0" + str(ScopeNumber.value)][0]
        if boolOperated:
            if solo:
                print "kdshTreeRings.Load():\tboolOperated=%d but no one else here...correcting" % boolOperated
                boolOperated = 0
                ageSDL["boolOperatedScope0" + str(ScopeNumber.value)] = (0,)
                ageSDL["OperatorIDScope0" + str(ScopeNumber.value)] = (-1,)
                Activate.enable()
            else:
                Activate.disable()
                print "kdshTreeRings.Load():\tboolOperated=%d, disabling telescope clickable" % boolOperated
        #START-->multiplayer fix
        ageSDL.sendToClients(('boolOperatedScope0' + str(ScopeNumber.value)))
        ageSDL.setFlags(('boolOperatedScope0' + str(ScopeNumber.value)), 1, 1)
        ageSDL.sendToClients(('OperatorIDScope0' + str(ScopeNumber.value)))
        ageSDL.setFlags(('OperatorIDScope0' + str(ScopeNumber.value)), 1, 1)
        ageSDL.sendToClients(('OuterRing0' + str(ScopeNumber.value)))
        ageSDL.setFlags(('OuterRing0' + str(ScopeNumber.value)), 1, 1)
        ageSDL.sendToClients(('MiddleRing0' + str(ScopeNumber.value)))
        ageSDL.setFlags(('MiddleRing0' + str(ScopeNumber.value)), 1, 1)
        ageSDL.sendToClients(('InnerRing0' + str(ScopeNumber.value)))
        ageSDL.setFlags(('InnerRing0' + str(ScopeNumber.value)), 1, 1)
        #END-->multiplayer fix

        if ScopeNumber.value == 3:
            boolDoorClosed = ageSDL["TreeRingDoorClosed"][0]
            if boolDoorClosed:
                respResetBtn.run(self.key,state='OnInit',fastforward=1)


    def AvatarPage(self, avObj, pageIn, lastOut):
        "reset scope accessibility if scope user quits or crashes"
        global boolScopeOperated
        ageSDL = PtGetAgeSDL()             
        
        if pageIn:
            return
            
        avID = PtGetClientIDFromAvatarKey(avObj.getKey())
        if avID == ageSDL["OperatorIDScope0" + str(ScopeNumber.value)][0]:
            Activate.enable()
            ageSDL["OperatorIDScope0" + str(ScopeNumber.value)] = (-1,)
            ageSDL["boolOperatedScope0" + str(ScopeNumber.value)] = (0,)
            print "kdshTreeRings.AvatarPage(): telescope operator paged out, reenabled telescope."
        else:
            return
            
    def __del__(self):
        "unload the dialog that we loaded"
        #~ PtUnloadDialog(DialogName)
        

    def OnNotify(self,state,id,events):
        global LocalAvatar
        global boolScopeOperator
        ageSDL = PtGetAgeSDL()                   
        #~ print "kdshTreeRings:OnNotify  state=%f id=%d events=" % (state,id),events
        
        if state and id == Activate.id and PtWasLocallyNotified(self.key):
            LocalAvatar = PtFindAvatar(events)
            self.IStartTelescope()
            
        elif id == actResetBtn.id:
            respResetBtn.run(self.key,state='Reset',events=events)
            
        elif id == respResetBtn.id and OnlyOneOwner.sceneobject.isLocallyOwned():
            print "kdshTreeRing Reset Button Pushed. Puzzle resetting."
            
            #close the door
            ageSDL.setTagString("TreeRingDoorClosed","fromInside")
            ageSDL["TreeRingDoorClosed"] = (1,)
            
            #reset the positions of the rings
            for scope in [1,2,3]:
                ageSDL["OuterRing0" + str(scope)] = (1,)
                ageSDL["MiddleRing0" + str(scope)] = (1,)
                ageSDL["InnerRing0" + str(scope)] = (1,)
            
            
        # check if its an advance stage notify
        for event in events:
            if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                if boolScopeOperator:
                    self.IEngageTelescope()
                    boolScopeOperator = 0
                break


    def OnGUINotify(self,id,control,event):
        global oldbearing
        global OuterRing01
        ageSDL = PtGetAgeSDL()   
        
        #~ print "kdshTreeRings: GUI Notify id=%d, event=%d control=" % (id,event),control
        
        #~ if event == kExitMode:
            #~ self.IQuitTelescope()            
        
        if event == kDialogLoaded:
            return
            print "GUI Notify id=%d, event=%d control=" % (id,event),control
            # if the dialog was just loaded then show it
            #~ control.show()
            PtShowDialog("kdshScope0" + str(ScopeNumber.value))
            print "kdshTreeRings: Showing scope dialog ", ("kdshScope0" + str(ScopeNumber.value))
            
        btnID = 0

        if isinstance(control,ptGUIControlButton):
            btnID = control.getTagID()
        
        if event == 5: # duplicate send on the first click of each button. Ignore it.            
            return

        if btnID == kGUIRingTurnLeft:
            newbearing = ageSDL["OuterRing0" + str(ScopeNumber.value)][0] + 1
            if newbearing == 9:
                newbearing = 1
            ageSDL["OuterRing0" + str(ScopeNumber.value)] = (newbearing,)
            #~ PtDebugPrint ("kdshTreeRings: updated SDL %s value to %s" % (("OuterRing0" + str (ScopeNumber.value)), newbearing))
        
        if btnID == kGUIRingTurnCenter or btnID == kGUIRingTurnLeft:
            newbearing = ageSDL["MiddleRing0" + str(ScopeNumber.value)][0] + 1
            if newbearing == 9:
                newbearing = 1
            ageSDL["MiddleRing0" + str(ScopeNumber.value)] = (newbearing,)
            #~ PtDebugPrint ("kdshTreeRings: updated SDL %s value to %s" % (("MiddleRing0" + str (ScopeNumber.value)), newbearing))

            
        if btnID == kGUIRingTurnRight or btnID == kGUIRingTurnCenter or btnID == kGUIRingTurnLeft:
            
            #No matter which of the three buttons is pushed, play the sound
            respSfxRings.run(self.key)

            newbearing = ageSDL["InnerRing0" + str(ScopeNumber.value)][0] + 1
            if newbearing == 9:
                newbearing = 1
            ageSDL["InnerRing0" + str(ScopeNumber.value)] = (newbearing,)
            #~ PtDebugPrint ("kdshTreeRings: updated SDL %s value to %s" % (("InnerRing0" + str (ScopeNumber.value)), newbearing))
            
    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IQuitTelescope()
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IQuitTelescope()


 
    def IStartTelescope(self):
        "Start the action of looking at the telescope"
        ageSDL = PtGetAgeSDL()   
        global LocalAvatar
        global boolScopeOperator
        
        #disable KI and linking book
        PtSendKIMessage(kDisableKIandBB,0)
        
        # disable the activator (only one in the telescope at a time)
        Activate.disable()
        boolScopeOperator = 1  # me! I'm the operator
        ageSDL["boolOperatedScope0" + str(ScopeNumber.value)] = (1,)
        avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
        ageSDL["OperatorIDScope0" + str(ScopeNumber.value)] = (avID,)
        print "kdshTreeRings.OnNotify:\twrote SDL - scope operator id = ", avID
       # start the behavior
        Behavior.run(LocalAvatar)
        

    def IEngageTelescope(self):
        global Telescope
        ageSDL = PtGetAgeSDL()   
        
        Telescope.pushTelescope()
        "After the behavior gets our eyes in the telescope, engage ourselves with the camera"
        
        #Send note to kdshTreeRingsSolution to fast forward the fake rings
        note = ptNotify(self.key)
        note.setActivate(1.0)
        note.addVarNumber("FastForward",ScopeNumber.value)
        note.send()
        # get control key events
        PtEnableControlKeyEvents(self.key)

        
        
        
        # Disable First Person Camera
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        # set camera to telescope
        virtCam = ptCamera()
        virtCam.save(Camera.sceneobject.getKey())
        
        # show the cockpit
        PtShowDialog("kdshScope0" + str(ScopeNumber.value))
        #~ print "kdshTreeRings: Showing scope dialog ", ("kdshScope0" + str(ScopeNumber.value))


    def IQuitTelescope(self):
        "Disengage and exit the telescope mode"
        global LocalAvatar
        global boolScopeOperator
        global Telescope
        ageSDL = PtGetAgeSDL()      
        
        Telescope.popTelescope()
        # exit every thing
        PtHideDialog("kdshScope0" + str(ScopeNumber.value))
        
        PtHideDialog("kdshScope0" + str(ScopeNumber.value))
        
        virtCam = ptCamera()
        virtCam.restore(Camera.sceneobject.getKey())
        # exit behavior...which is in the next stage
        #Behavior.gotoStage(LocalAvatar,2)
        Behavior.nextStage(LocalAvatar)
        #disable the Control key events
        PtDisableControlKeyEvents(self.key)
        PtSendKIMessage(kEnableKIandBB,0)
        # re-enable the telescope for someone else to use
        boolScopeOperator = 0
        ageSDL["boolOperatedScope0" + str(ScopeNumber.value)] = (0,)
        ageSDL["OperatorIDScope0" + str(ScopeNumber.value)] = (-1,)
        #Re-enable first person camera
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        PtAtTimeCallback(self.key,3,1) # wait for player to finish exit one-shot, then reenable clickable
        #~ PtDebugPrint("kdshTreeRings.IQuitTelescope:\tdelaying clickable reenable")
        
    def OnTimer(self,id):
        if id==1:
            Activate.enable()
            #~ PtDebugPrint("kdshTreeRings.OnTimer:\tScope #%s clickable reenabled" % (ScopeNumber.value))
