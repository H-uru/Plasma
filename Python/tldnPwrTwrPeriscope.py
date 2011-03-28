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
Module: tldnPwrTwrPeriscope
Age: Teledahn
This is the handler for the power tower periscope
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
Activate = ptAttribActivator(11, "Activate Telescope",netForce=1)
Camera = ptAttribSceneobject(12,"Telescope camera")
Behavior = ptAttribBehavior(13, "Telescope behavior (multistage)",netForce=1)
Vignette = ptAttribString(14,"Vignette dialog - by Name")

#actPump  = ptAttribActivator(15,"Actvtr: Pump Script")
respPowerOn  = ptAttribResponder(16,"Rspndr: MainPwrOn")
respPowerOff = ptAttribResponder(17,"Rspndr: MainPwrOff")

animScopeLeft = ptAttribAnimation(18, "Anim: ScopeLeft",netForce=1)
animScopeUp = ptAttribAnimation(19, "Anim: ScopeUp",netForce=1)

actSun = ptAttribActivator(20, "Actvtr: Sun")

actStopAtTop = ptAttribActivator(21, "Actvtr: Stop Scope @ Top")
actStopAtBtm = ptAttribActivator(22, "Actvtr: Stop Scope @ Btm")

# ---------
# globals

boolPwrMain = 0
boolTwrRaised = 0

kGUIBlocker = 35
kGUILeftBtn = 36
kGUIRightBtn = 37
kGUIUpBtn = 38
kGUIDownBtn = 39
kScopeClear = 1
kScopeBlocked = 0

kTimerIDDelayPowerOff = 10
kTimerIDClearOperator = 11
kTimerIDTowerInMotion = 12

scopeSpdLeft = 0
scopeSpdLeftInc = 0.5
scopeSpdLeftMax = 2
scopeSpdUp = 0
scopeSpdUpInc = 0.5
scopeSpdUpMax = 1
boolScopeAtTop = 0
boolScopeAtBtm = 1

boolScopeOperator = 0
LocalAvatar = None

# Teledahn.sdl vars
kStringAgeSDLPumpCount = "tldnPwrTwrPumpCount"  # will be used to determine if tower is up or down (3=up, all else down)
kStringAgeSDLMainPowerOn = "tldnMainPowerOn"

AgeStartedIn = None

class tldnPwrTwrPeriscope(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5003
        version = 4
        self.version = version
        print "tldnPwrTwrPerscope v.",version,".3"

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtLoadDialog(Vignette.value,self.key, "Teledahn")
        # non-age SDL vars
        self.SDL.setDefault("scopeSpdLeft",(0,))
        self.SDL.setDefault("scopeSpdUp",(0,))
        self.SDL.setDefault("scopeAtTop",(0,))
        self.SDL.setDefault("scopeAtBtm",(0,))
        self.SDL.setDefault("boolOperated",(0,))
        self.SDL.setDefault("OperatorID",(-1,))
        self.SDL.sendToClients("scopeSpdLeft")
        self.SDL.sendToClients("scopeSpdUp")
        self.SDL.sendToClients("scopeAtTop")
        self.SDL.sendToClients("scopeAtBottom")
        self.SDL.sendToClients("OperatorID")
        self.SDL.sendToClients("boolOperated")


    def Load(self):
        solo = true
        if len(PtGetPlayerList()):
            solo = false
        boolOperated = self.SDL["boolOperated"][0]
        if boolOperated:
            if solo:
                PtDebugPrint("tldnPwrTwrPeriscope.Load():\tboolOperated=%d but no one else here...correcting" % (boolOperated))
                boolOperated = 0
                self.SDL["boolOperated"] = (0,)
                self.SDL["OperatorID"] = (-1,)
                Activate.enable()
            else:
                Activate.disable()
                PtDebugPrint("tldnPwrTwrPeriscope.Load():\tboolOperated=%d, disabling periscope clickable" % (boolOperated))

    def OnServerInitComplete(self):
        global boolTwrRaised
        global boolPwrMain
        global scopeSpdLeft
        global scopeSpdUp
        global boolScopeAtTop
        global boolScopeAtBtm
        
        if AgeStartedIn == PtGetAgeName():
            # age sdl vars
            ageSDL = PtGetAgeSDL()
            # set flags on age SDL vars we'll be changing
            ageSDL.setFlags(kStringAgeSDLMainPowerOn,1,1)
            ageSDL.sendToClients(kStringAgeSDLMainPowerOn)
    
            # register for notification of age SDL var changes
            ageSDL.setNotify(self.key,kStringAgeSDLPumpCount,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLMainPowerOn,0.0)
            
            # get initial SDL state
            
            # tower raised?
            boolTwrRaised = false
            try:
                pumpCount = ageSDL[kStringAgeSDLPumpCount][0]
                if pumpCount == 3:
                    boolTwrRaised = true
                PtDebugPrint("tldnPwrTwrPeriscope.OnServerInitComplete():\tageSDL[%s] = %d, TwrRaised = %d" % (kStringAgeSDLPumpCount,pumpCount,boolTwrRaised) )
            except:
                PtDebugPrint("tldnPwrTwrPeriscope.OnServerInitComplete():\tERROR: age sdl read failed, defaulting tower lowered")
            if boolTwrRaised:
                actSun.enable()
            else:
                actSun.disable()
               
            # tower aligned?
            boolPwrMain = false
            try:
                boolPwrMain = ageSDL[kStringAgeSDLMainPowerOn][0]
                PtDebugPrint("tldnPwrTwrPeriscope.OnServerInitComplete():\tPwrMain=%d" % (boolPwrMain))
            except:
                PtDebugPrint("tldnPwrTwrPeriscope.OnServerInitComplete():\tERROR: age sdl read failed, defaulting main power off")
            
            # initialize tower whatnots based on SDL state
            if not boolPwrMain:
                respPowerOff.run(self.key,fastforward=true)            
            # if PwrOn but tower not raised, correct
            if boolPwrMain and not boolTwrRaised:
                PtDebugPrint("tldnPwrTwrPeriscope.OnServerInitComplete():\tturning off main power because tower not raised")
                ageSDL.setTagString(kStringAgeSDLMainPowerOn,"ignore")
                ageSDL[kStringAgeSDLMainPowerOn] = (0,)
                boolPwrMain = 0
                respPowerOff.run(self.key,fastforward=true)
                animScopeLeft.value.stop()
                animScopeUp.value.stop()
                scopeSpdUp = 0
                self.SDL["scopeSpdUp"] = (0,)
                scopeSpdLeft = 0
                self.SDL["scopeSpdLeft"] = (0,)
            if boolPwrMain:
                respPowerOn.run(self.key)          
            
        # non-age SDL vars
        scopeSpdLeft = self.SDL["scopeSpdLeft"][0]
        scopeSpdUp = self.SDL["scopeSpdUp"][0]
        boolScopeAtTop = self.SDL["scopeAtTop"][0]
        boolScopeAtBtm = self.SDL["scopeAtBtm"][0]
        PtDebugPrint("tldnPwrTwrPeriscope.Load():\tscope values: left=%f,up=%f,top=%d,btm=%d" % (scopeSpdLeft,scopeSpdUp,boolScopeAtTop,boolScopeAtBtm))
        
    def AvatarPage(self, avObj, pageIn, lastOut):
        "reset scope accessibility if scope user quits or crashes"
        if pageIn:
            return

        avID = PtGetClientIDFromAvatarKey(avObj.getKey())
        PtDebugPrint("tldnPwrTwrPeriscope.AvatarPage(): Client ID %d paging out" % (avID))
        PtDebugPrint("tldnPwrTwrPeriscope.AvatarPage(): Periscope operator is client id %d" % (self.SDL["OperatorID"][0]))

        if avID == self.SDL["OperatorID"][0]:
            Activate.enable()
            self.SDL["OperatorID"] = (-1,)
            self.SDL["boolOperated"] = (0,)
            PtDebugPrint("tldnPwrTwrPeriscope.AvatarPage(): periscope operator paged out, reenabled periscope.")
            PtDebugPrint("tldnPwrTwrPeriscope.AvatarPage(): OperatorID=%d, boolOperated=%d" % (self.SDL["OperatorID"][0],self.SDL["boolOperated"][0]))
        else:
            return
            
    def __del__(self):
        "make sure the dialog is unloaded"
        PtUnloadDialog(Vignette.value)
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolTwrRaised
        global boolPwrMain
        global scopeSpdUp
        global scopeSpdLeft
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnPwrTwrPeriscope.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
            if tag == "ignore":
                return
            
            ########################
            # Tower Raised or Lowered   #
            ########################
            if VARname == kStringAgeSDLPumpCount:
                pumpCount = ageSDL[kStringAgeSDLPumpCount][0]  # tower goes up on 3
                if pumpCount == 3 and not boolTwrRaised:
                    # tower state changed to raised
                    boolTwrRaised = true
                elif pumpCount != 3 and boolTwrRaised:
                    # tower state changed to lowered
                    boolTwrRaised = false
                else:
                    # tower raised/lowered state didn't change
                    return
                PtDebugPrint("tldnPwrTwrPeriscope.OnSDLNotify:\ttower state changed: boolTwrRaised=%d" % (boolTwrRaised))
                
                #Disable clickable to enter the scope while the tower is in motion
                Activate.disable()
                PtAtTimeCallback(self.key,12,kTimerIDTowerInMotion)
                
                if boolTwrRaised:
                    actSun.enable()
                    
                    if boolScopeOperator:
                        GUIDialog = PtGetDialogFromString(Vignette.value)
                        GUICbxBlocker = ptGUIControlCheckBox(GUIDialog.getControlFromTag(kGUIBlocker))
                        GUICbxBlocker.setChecked(kScopeClear)
                        GUIDialog.refreshAllControls()
                else:
                    #stop the rotating whether the power was on or not
                    animScopeLeft.value.stop()
                    animScopeUp.value.stop()
                    scopeSpdUp = 0
                    self.SDL["scopeSpdUp"] = (0,)
                    scopeSpdLeft = 0
                    self.SDL["scopeSpdLeft"] = (0,)
                    
                    if boolScopeOperator:
                        GUIDialog = PtGetDialogFromString(Vignette.value)
                        GUICbxBlocker = ptGUIControlCheckBox(GUIDialog.getControlFromTag(kGUIBlocker))                    
                        GUICbxBlocker.setChecked(kScopeBlocked)
                        GUIDialog.refreshAllControls()
                    if boolPwrMain and not boolTwrRaised: # power on, tower being lowered
                        PtDebugPrint("tldnPwrTwrPeriscope.OnSDLNotify():\tturning off main power because being lowered")
                        ageSDL.setTagString(kStringAgeSDLMainPowerOn,"towerLowered")
                        ageSDL[kStringAgeSDLMainPowerOn] = (0,)
                return
                
            ########################
            # Main Power Change    #
            ########################
            if VARname == kStringAgeSDLMainPowerOn:
                boolPwrMain = ageSDL[kStringAgeSDLMainPowerOn][0] 
                if not boolPwrMain:
                    if tag == "towerLowered":
                        PtAtTimeCallback(self.key,5,kTimerIDDelayPowerOff) # delay power off till tower actually coming down
                        actSun.disable()
                        #~ animScopeLeft.value.stop()
                        #~ animScopeUp.value.stop()
                        #~ scopeSpdUp = 0
                        #~ self.SDL["scopeSpdUp"] = (0,)
                        #~ scopeSpdLeft = 0
                        #~ self.SDL["scopeSpdLeft"] = (0,)
                    else:
                        respPowerOff.run(self.key) # run immediately
                if boolPwrMain:
                    ageSDL.setTagString(kStringAgeSDLPumpCount,"fastforward")
                    ageSDL[kStringAgeSDLPumpCount] = (3,) # raise the tower...if it's already raised this val will already be 3 and nothing will come back to us, otherwise force the tower to raise
                    respPowerOn.run(self.key)
                return
                
    def OnTimer(self,timer):
        global boolScopeOperator
        
        if timer ==  kTimerIDDelayPowerOff:
            respPowerOff.run(self.key) # run immediately
            return
        
        if timer == kTimerIDClearOperator:
            Activate.enable()
            boolScopeOperator = 0
            self.SDL["boolOperated"] = (0,)
            self.SDL["OperatorID"] = (-1,)
            return
            
        if timer == kTimerIDTowerInMotion:
            Activate.enable()
            return


    def OnNotify(self,state,id,events):
        "Activated... start telescope"
        global LocalAvatar
        global boolPwrMain
        global boolTwrRaised
        global boolScopeAtTop
        global boolScopeAtBtm
        global scopeSpdUp
        global scopeSpdLeft
        global boolScopeOperator

        #####################################
        # Stop Scope at vertical boundaries #
        #####################################
        if state and id==actStopAtTop.id and scopeSpdUp>0:
            PtDebugPrint("PERISCOPE: got event - near top")
            animScopeUp.value.stop()
            boolScopeAtTop = 1
            self.SDL["scopeAtTop"] = (1,)
            scopeSpdUp = 0
            self.SDL["scopeSpdUp"] = (0,)
            return     
        if state and id==actStopAtBtm.id and scopeSpdUp<0:
            PtDebugPrint("PERISCOPE: got event - near bottom")
            animScopeUp.value.stop()
            boolScopeAtBtm = 1
            self.SDL["scopeAtBtm"] = (1,)
            scopeSpdUp = 0
            self.SDL["scopeSpdUp"] = (0,)
            return            

    
        #############################
        # Toggle Main Power via Sun #
        #############################
        if id==actSun.id and boolTwrRaised:
            #print "PERISCOPE: detected sun. state=%d, events to follow" % state
            #print events
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                for event in events:
                    if event[0] == kCollisionEvent: 
                        linedUp = event[1] # enter = 1, exit = 0
                        if linedUp: # sun now lined up with scope
                            PtDebugPrint("tldnPwrTwrPeriscope.OnNotify():\tturning on main power because dish is lined up")
                            ageSDL.setTagString(kStringAgeSDLMainPowerOn,"towerAligned")
                            ageSDL[kStringAgeSDLMainPowerOn] = (1,)
                        else:
                            PtDebugPrint("tldnPwrTwrPeriscope.OnNotify():\tturning off main power because dish is not lined up")
                            ageSDL.setTagString(kStringAgeSDLMainPowerOn,"towerMisaligned")
                            ageSDL[kStringAgeSDLMainPowerOn] = (0,)
                        break
            return

        ######################
        # Enter the scope    #
        ######################
        if state and id == Activate.id:
            LocalAvatar = PtFindAvatar(events)
            if PtWasLocallyNotified(self.key):
                Behavior.run(LocalAvatar)           
                boolScopeOperator = 1 # will use to enable GUI only for me
                self.SDL["boolOperated"] = (1,)
                Activate.disable()
                PtSendKIMessage(kDisableKIandBB,0)
                avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
                self.SDL["OperatorID"] = (avID,)
                PtDebugPrint("tldnPwrTwrPeriscope.OnNotify:\twrote SDL - scope operator id = %d" % (avID))
                PtDebugPrint("tldnPwrTwrPeriscope.OnNotify:\tSDL OperatorID = %d" % (self.SDL["OperatorID"]))
                GUIDialog = PtGetDialogFromString(Vignette.value)
                GUICbxBlocker = ptGUIControlCheckBox(GUIDialog.getControlFromTag(kGUIBlocker))
                if boolTwrRaised:
                    GUICbxBlocker.setChecked(kScopeClear)
                else:
                    GUICbxBlocker.setChecked(kScopeBlocked)
                GUIDialog.refreshAllControls()
                #self.IStartTelescope()

        # if it's an advance stage notify pop up GUI if I'm the guy at the scope
        if PtWasLocallyNotified(self.key):        
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                    if boolScopeOperator:
                        self.IEngageTelescope()
                    break
    
        
    def OnGUINotify(self,id,control,event):
        "Notifications from the vignette"
        global scopeSpdUp
        global scopeSpdLeft
        global boolScopeAtTop
        global boolScopeAtBtm

        if not boolTwrRaised:
            return

        if event != kAction:
            return
        
        btnID = 0
        if isinstance(control,ptGUIControlButton):
            btnID = control.getTagID()
        else:
            #print "SCOPE GUI: non-button notification"
            return

        #print "tldnPwrTwrPeriscope.OnGUINotify():\tbefore click...left=%f,up=%f,top=%d,btm=%d" % (scopeSpdLeft,scopeSpdUp,boolScopeAtTop,boolScopeAtBtm)

        #print "SCOPE GUI: got a button id = %d" % btnID
        if btnID == kGUILeftBtn:
            if scopeSpdLeft == scopeSpdLeftMax:
                return
            scopeSpdLeft = scopeSpdLeft + scopeSpdLeftInc
            if scopeSpdLeft == 0:
                animScopeLeft.value.stop()
            else:
                animScopeLeft.value.speed(scopeSpdLeft)
                animScopeLeft.value.resume()
            self.SDL["scopeSpdLeft"] = (scopeSpdLeft,)
            return

        if btnID == kGUIRightBtn:
            if scopeSpdLeft == -scopeSpdLeftMax:
                return
            scopeSpdLeft = scopeSpdLeft - scopeSpdLeftInc
            if scopeSpdLeft == 0:
                animScopeLeft.value.stop()
            else:               
                animScopeLeft.value.speed(scopeSpdLeft)
                animScopeLeft.value.resume()
            self.SDL["scopeSpdLeft"] = (scopeSpdLeft,)
            return

        if btnID == kGUIUpBtn:
            if scopeSpdUp == scopeSpdUpMax or boolScopeAtTop:
                PtDebugPrint("PERISCOPE: at top or max speed already")
                return
            scopeSpdUp = scopeSpdUp + scopeSpdUpInc
            if scopeSpdUp == 0:
                animScopeUp.value.stop()
            else:
                if scopeSpdUp == scopeSpdUpInc:
                    animScopeUp.value.backwards(0)
                if scopeSpdUp < 0:
                    animScopeUp.value.speed(-scopeSpdUp)
                else:
                    animScopeUp.value.speed(scopeSpdUp)
                animScopeUp.value.resume()
                if boolScopeAtBtm:
                    boolScopeAtBtm = 0
                    self.SDL["scopeAtBtm"] = (0,)
            self.SDL["scopeSpdUp"] = (scopeSpdUp,)
            return
                    
        if btnID == kGUIDownBtn:
            if scopeSpdUp == -scopeSpdUpMax or boolScopeAtBtm:
                PtDebugPrint("PERISCOPE: at bottom or max speed already")
                return
            scopeSpdUp = scopeSpdUp - scopeSpdUpInc
            if scopeSpdUp == 0:
                animScopeUp.value.stop()
            else:
                if scopeSpdUp == -scopeSpdUpInc:
                    animScopeUp.value.backwards(1)
                if scopeSpdUp < 0:
                    animScopeUp.value.speed(-scopeSpdUp)
                else:
                    animScopeUp.value.speed(scopeSpdUp)
                animScopeUp.value.resume()
                if boolScopeAtTop:
                    boolScopeAtTop = 0
                    self.SDL["scopeAtTop"] = (0,)
            self.SDL["scopeSpdUp"] = (scopeSpdUp,)
            return


    def OnControlKeyEvent(self,controlKey,activeFlag):
        "Control key events... anything we're interested in?"
        #print "Got controlKey event %d and its activeFlage is %d" % (controlKey,activeFlag)
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IQuitTelescope()
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IQuitTelescope()

#    def IStartTelescope(self):
#        "Start the action of looking at the telescope"
#        global LocalAvatar
#        # disable the activator (only one in the telescope at a time)
#        Activate.disable()
#        PtSendKIMessage(kDisableKIandBB,0)
#        # start the behavior
#        Behavior.run(LocalAvatar)

    def IEngageTelescope(self):
        "After the behavior gets our eyes in the telescope, engage ourselves with the camera"
        # Disable First Person Camera
        PtDebugPrint("tldnPwrTwrPeriscope:\tFirst Person Camera disabled.")
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        # set camera to telescope
        virtCam = ptCamera()
        virtCam.save(Camera.sceneobject.getKey())
        # show the cockpit
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtLoadDialog(Vignette.value,self.key, "Teledahn")
            if ( PtIsDialogLoaded(Vignette.value) ):
                PtShowDialog(Vignette.value)
        # get control key events
        PtEnableControlKeyEvents(self.key)
        PtDebugPrint("tldnPwrTwrPeriscope.IEngageTelescope():\tentering telescope...left=%f,up=%f,top=%d,btm=%d" % (scopeSpdLeft,scopeSpdUp,boolScopeAtTop,boolScopeAtBtm))

    def IQuitTelescope(self):
        "Disengage and exit the telescope mode"
        global LocalAvatar
        global boolScopeOperator
        # exit every thing
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtHideDialog(Vignette.value)
        virtCam = ptCamera()
        virtCam.restore(Camera.sceneobject.getKey())
        # exit behavior...which is in the next stage
        # ... but just gotoStage the last stage by number because of a bug in the SDL send state
        Behavior.gotoStage(LocalAvatar,2)
        #Behavior.nextStage(LocalAvatar)
        #disable the Control key events
        PtDisableControlKeyEvents(self.key)
        PtSendKIMessage(kEnableKIandBB,0)
        
        # re-enable the telescope for someone else to use in 3 seconds
        PtAtTimeCallback(self.key,3,kTimerIDClearOperator) # Put in this delay to avoid people entering the scope before they've fully exited it


        #Re-enable first person camera
        PtDebugPrint( "tldnPwrTwrPeriscope:\tFirst Person Camera re-enabled.")
        cam = ptCamera()
        cam.enableFirstPersonOverride()
