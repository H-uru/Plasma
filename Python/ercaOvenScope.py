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
Module: ercaOvenScope
Age: Ercana
Date: December, 2003
Author: Chris Doyle, based on xTelescope.py by Mark DeForest, Bill Slease, Doug McBride
This is the handler for Ercana's bakery oven-scopes
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys
import string
import xEnum
from math import *


# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "clk: Oven scope",netForce=1)
Camera = ptAttribSceneobject(2,"Oven scope camera")
Behavior = ptAttribBehavior(3, "Scope behavior (multistage)",netForce=1)
Vignette = ptAttribString(4,"Vignette dialog - by Name")
SDLBakeryPwr = ptAttribString(5,"SDL: bakery power")
SDLScopePwr = ptAttribString(6,"SDL: scope power")
CameraBad = ptAttribSceneobject(7,"Oven scope camera - no power")
VignetteBad = ptAttribString(8,"Vignette dialog - no power")
ScopeNum = ptAttribInt(9,"Oven scope #")
RespTimeSlider = ptAttribResponder(10,"resp: time slider",['off','on'])
RespAmountSlider = ptAttribResponder(11,"resp: amount slider",['off','on'])
RespTempSlider = ptAttribResponder(12,"resp: temp slider",['off','on'])
RespSoundTest = ptAttribResponder(13,"resp: sound test")
RespMayBake = ptAttribResponder(14,"resp: may bake",['no','yes'])
RespSfxTimerWheel = ptAttribResponder(15,"resp: timer wheel sfx",['off','on'])


#-----------------------------------
# konstants for the GUI buttons and time scale

kTimeSlider = 100
kAmountSlider = 200
kTempSlider = 300
kBakeBtn = 400
kTimerWheel = 500
kTempWheel = 600
kTimeScale = 180  #  each slider notch is 3 minutes each   ## THIS IS THE FINAL SETTING
#kTimeScale = 2  #  each slider notch is 2 secs each         ## THIS IS FOR QA TESTING ONLY!

# ---------
# globals

LocalAvatar = None
boolScopeOperator = 0
boolOperated = 0
Telescope = ptInputInterface()
boolBakeryPwr = 0
boolScopePwr = 0
WasPowered = 1
listTimeSDLs = ['ercaTimeSlider1','ercaTimeSlider2','ercaTimeSlider3','ercaTimeSlider4']
listAmountSDLs = ['ercaAmountSlider1','ercaAmountSlider2','ercaAmountSlider3','ercaAmountSlider4']
listTempSDLs = ['ercaTempSlider1','ercaTempSlider2','ercaTempSlider3','ercaTempSlider4']
timeSDL = None
amountSDL = None
tempSDL = None
byteTime = 0
byteAmount = 0
byteTemp = 0
timeSlider = None
amountSlider = None
tempSlider = None
bakeBtn = None
timerWheel = None
tempWheel = None
boolMayBake = 0
IsBaking = 0
exitScope = 0
setTempWheel = 0

class ercaOvenScope(ptModifier):
    
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 7030
        self.version = 9


    def OnFirstUpdate(self):
        self.SDL.setDefault("boolOperated",(0,))
        self.SDL.setDefault("OperatorID",(-1,))
        self.SDL.sendToClients("boolOperated")
        self.SDL.sendToClients("OperatorID")


    def OnServerInitComplete(self):
        global boolBakeryPwr
        global boolScopePwr
        global timeSDL
        global amountSDL
        global tempSDL
        global byteTime
        global byteAmount
        global byteTemp
        global boolMayBake
        global IsBaking
        
        timeSDL = listTimeSDLs[ScopeNum.value - 1]
        amountSDL = listAmountSDLs[ScopeNum.value - 1]
        tempSDL = listTempSDLs[ScopeNum.value - 1]
        
        ageSDL = PtGetAgeSDL()
        
        ageSDL.setFlags(SDLBakeryPwr.value,1,1)
        ageSDL.sendToClients(SDLBakeryPwr.value)
        ageSDL.setFlags(SDLScopePwr.value,1,1)
        ageSDL.sendToClients(SDLScopePwr.value)
        ageSDL.setFlags(timeSDL,1,1)
        ageSDL.sendToClients(timeSDL)
        ageSDL.setFlags(amountSDL,1,1)
        ageSDL.sendToClients(amountSDL)
        ageSDL.setFlags(tempSDL,1,1)
        ageSDL.sendToClients(tempSDL)
        ageSDL.setFlags("ercaMayBake",1,1)
        ageSDL.sendToClients("ercaMayBake")
        ageSDL.setFlags("ercaBakeFinishTime",1,1)
        ageSDL.sendToClients("ercaBakeFinishTime")
        
        ageSDL.setNotify(self.key,SDLBakeryPwr.value,0.0)
        ageSDL.setNotify(self.key,SDLScopePwr.value,0.0)
        ageSDL.setNotify(self.key,timeSDL,0.0)
        ageSDL.setNotify(self.key,amountSDL,0.0)
        ageSDL.setNotify(self.key,tempSDL,0.0)
        ageSDL.setNotify(self.key,"ercaMayBake",0.0)
        ageSDL.setNotify(self.key,"ercaBakeFinishTime",0.0)
        
        try:
            boolBakeryPwr = ageSDL[SDLBakeryPwr.value][0]
        except:
            PtDebugPrint("ERROR: ercaOvenScope.OnServerInitComplete():\tERROR reading SDL name for bakery power")
            boolBakeryPwr = 0
        PtDebugPrint("DEBUG: ercaOvenScope.OnServerInitComplete():\t%s = %d" % (SDLBakeryPwr.value,ageSDL[SDLBakeryPwr.value][0]) )
        try:
            boolScopePwr = ageSDL[SDLScopePwr.value][0]
        except:
            PtDebugPrint("ERROR: ercaOvenScope.OnServerInitComplete():\tERROR reading SDL name for scope power")
            boolScopePwr = 0
        PtDebugPrint("DEBUG: ercaOvenScope.OnServerInitComplete():\t%s = %d" % (SDLScopePwr.value,ageSDL[SDLScopePwr.value][0]) )
        try:
            boolMayBake = ageSDL["ercaMayBake"][0]
        except:
            PtDebugPrint("ERROR: ercaOvenScope.OnServerInitComplete():\tERROR reading SDL name for ercaMayBake")
            boolMayBake = 0
        PtDebugPrint("DEBUG: ercaOvenScope.OnServerInitComplete():\tercaMayBake = %d" % (ageSDL["ercaMayBake"][0]) )
        try:
            IsBaking = ageSDL["ercaBakeFinishTime"][0]
        except:
            PtDebugPrint("ERROR: ercaOvenScope.OnServerInitComplete():\tERROR reading SDL name for ercaBakeFinishTime")
            IsBaking = 0
        PtDebugPrint("DEBUG: ercaOvenScope.OnServerInitComplete():\tercaBakeFinishTime = %d" % (ageSDL["ercaBakeFinishTime"][0]) )

        byteTime = ageSDL[timeSDL][0]
        byteAmount = ageSDL[amountSDL][0]
        byteTemp = ageSDL[tempSDL][0]

        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtLoadDialog(Vignette.value,self.key)


    def Load(self):
        global boolScopeOperated

        solo = true
        if len(PtGetPlayerList()):
            solo = false

        boolOperated = self.SDL["boolOperated"][0]
        if boolOperated:
            if solo:
                PtDebugPrint("ercaOvenScope.Load():\tboolOperated=%d but no one else here...correcting" % boolOperated,level=kDebugDumpLevel)
                boolOperated = 0
                self.SDL["boolOperated"] = (0,)
                self.SDL["OperatorID"] = (-1,)
                Activate.enable()
            else:
                Activate.disable()
                PtDebugPrint("ercaOvenScope.Load():\tboolOperated=%d, disabling telescope clickable" % boolOperated,level=kDebugDumpLevel)


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
            PtDebugPrint("ercaOvenScope.AvatarPage(): telescope operator paged out, reenabled telescope.",level=kDebugDumpLevel)
        else:
            return


    def __del__(self):
        "unload the dialog that we loaded"
        PtUnloadDialog(Vignette.value)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolBakeryPwr
        global boolScopePwr
        global timeSDL
        global amountSDL
        global tempSDL
        global byteTime
        global byteAmount
        global byteTemp
        global boolMayBake
        global IsBaking
        global timeSlider
        global amountSlider
        global tempSlider
        global bakeBtn
        global timerWheel
        global tempWheel
        ageSDL = PtGetAgeSDL()
        
        if VARname == SDLBakeryPwr.value:
            boolBakeryPwr = ageSDL[SDLBakeryPwr.value][0]
            if boolBakeryPwr == 0:
                if IsBaking != 0 and self.sceneobject.isLocallyOwned():
                    ageSDL["ercaBakeFinishTime"] = (0,)
                if boolMayBake and self.sceneobject.isLocallyOwned():
                    ageSDL["ercaMayBake"] = (0,)
            else:
                self.ICheckScopes()
        
        if VARname == SDLScopePwr.value:
            boolScopePwr = ageSDL[SDLScopePwr.value][0]
        
        if VARname == "ercaMayBake":
            boolMayBake = ageSDL["ercaMayBake"][0]
            PtDebugPrint("ercaOvenScope:OnSDLNotify:  SDL for ercaMayBake now set to %d" % (boolMayBake))
            if boolMayBake:
                RespMayBake.run(self.key,state="yes")
            else:
                RespMayBake.run(self.key,state="no")
        
        if VARname == "ercaBakeFinishTime":
            IsBaking = ageSDL["ercaBakeFinishTime"][0]
            #PtDebugPrint("ercaOvenScope:OnSDLNotify:  SDL for ercaBakeFinishTime now set to %d" % (IsBaking))
            if IsBaking != 0:
                timeSlider.disable()
                amountSlider.disable()
                tempSlider.disable()
                bakeBtn.disable()
            else:
                timerWheel.setValue(0)
                tempWheel.setValue(0)
                timeSlider.enable()
                amountSlider.enable()
                tempSlider.enable()
                bakeBtn.enable()
        
        if VARname == timeSDL:
            byteTimeOld = byteTime
            byteTime = ageSDL[timeSDL][0]
            PtDebugPrint("ercaOvenScope:OnSDLNotify:  SDL for %s now set to %d" % (timeSDL,byteTime))
            RespSoundTest.run(self.key)
            if byteTime == 0 and byteTimeOld > 0:
                RespTimeSlider.run(self.key,state="off")
                if boolMayBake != 0 and self.sceneobject.isLocallyOwned():
                    ageSDL["ercaMayBake"] = (0,)
            elif byteTime > 0 and byteTimeOld == 0:
                RespTimeSlider.run(self.key,state="on")
                if boolMayBake == 0:
                    self.ICheckScopes()

        if VARname == amountSDL:
            byteAmountOld = byteAmount
            byteAmount = ageSDL[amountSDL][0]
            PtDebugPrint("ercaOvenScope:OnSDLNotify:  SDL for %s now set to %d" % (amountSDL,byteAmount))
            RespSoundTest.run(self.key)
            if byteAmount == 0 and byteAmountOld > 0:
                RespAmountSlider.run(self.key,state="off")
                if boolMayBake != 0 and self.sceneobject.isLocallyOwned():
                    ageSDL["ercaMayBake"] = (0,)
            elif byteAmount > 0 and byteAmountOld == 0:
                RespAmountSlider.run(self.key,state="on")
                if boolMayBake == 0:
                    self.ICheckScopes()

        if VARname == tempSDL:
            byteTempOld = byteTemp
            byteTemp = ageSDL[tempSDL][0]
            PtDebugPrint("ercaOvenScope:OnSDLNotify:  SDL for %s now set to %d" % (tempSDL,byteTemp))
            RespSoundTest.run(self.key)
            if byteTemp == 0 and byteTempOld > 0:
                RespTempSlider.run(self.key,state="off")
                if boolMayBake != 0 and self.sceneobject.isLocallyOwned():
                    ageSDL["ercaMayBake"] = (0,)
            elif byteTemp > 0 and byteTempOld == 0:
                RespTempSlider.run(self.key,state="on")
                if boolMayBake == 0:
                    self.ICheckScopes()


    def ICheckScopes(self):
        global boolMayBake
        global boolBakeryPwr
        ageSDL = PtGetAgeSDL()
        
        for xSDL in listTimeSDLs:
            xVal = ageSDL[xSDL][0]
            if xVal == 0:
                break
        for ySDL in listAmountSDLs:
            yVal = ageSDL[ySDL][0]
            if yVal == 0:
                break
        for zSDL in listTempSDLs:
            zVal = ageSDL[zSDL][0]
            if zVal == 0:
                break
        
        if xVal != 0 and yVal != 0 and zVal != 0:
            if boolBakeryPwr:
                if self.sceneobject.isLocallyOwned():
                    ageSDL["ercaMayBake"] = (1,)
            else:
                if self.sceneobject.isLocallyOwned():
                    ageSDL["ercaMayBake"] = (0,)


    def OnNotify(self,state,id,events):
        "Activated... start telescope"
        global LocalAvatar
        global boolScopeOperator
        PtDebugPrint("ercaOvenScope:OnNotify  state=%d id=%d events=" % (state,id),events,level=kDebugDumpLevel)
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
        global timeSDL
        global amountSDL
        global tempSDL
        global byteTime
        global byteAmount
        global byteTemp
        global timeSlider
        global amountSlider
        global tempSlider
        global bakeBtn
        global timerWheel
        global tempWheel
        global byteTime
        global byteAmount
        global byteTemp
        global boolMayBake
        global IsBaking
        global WasPowered
        global setTempWheel
        "Notifications from the vignette"
        PtDebugPrint("GUI Notify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel)
        ageSDL = PtGetAgeSDL()
        
        if event == kDialogLoaded:
            if WasPowered == 0:
                return
            timeSlider = ptGUIControlKnob(control.getControlFromTag(kTimeSlider))
            amountSlider = ptGUIControlKnob(control.getControlFromTag(kAmountSlider))            
            tempSlider = ptGUIControlKnob(control.getControlFromTag(kTempSlider))
            bakeBtn = ptGUIControlButton(control.getControlFromTag(kBakeBtn))
            timerWheel = ptGUIControlProgress(control.getControlFromTag(kTimerWheel))
            tempWheel = ptGUIControlProgress(control.getControlFromTag(kTempWheel))
        
        elif event == kShowHide:
            if WasPowered == 0:
                return
            if control.isEnabled():
                control.show()
                PtDebugPrint("ercaOvenScope:OnGUINotify:  SDL %s is %d" % (timeSDL,byteTime))
                PtDebugPrint("ercaOvenScope:OnGUINotify:  SDL %s is %d" % (amountSDL,byteAmount))
                PtDebugPrint("ercaOvenScope:OnGUINotify:  SDL %s is %d" % (tempSDL,byteTemp))
                if IsBaking != 0:
                    print "OnGUINotfiy.ShowHide: will now set timerWheel to: ",byteTime
                    print "OInGUINotfiy.ShowHide: will now set tempWheel to: ",byteTemp
                    if setTempWheel:
                        tempWheel.setValue(byteTemp)
                    self.IDoTimerWheel()
                    timeSlider.disable()
                    amountSlider.disable()
                    tempSlider.disable()
                    bakeBtn.disable()
                else:
                    timerWheel.setValue(0)
                    tempWheel.setValue(0)
                    timeSlider.enable()
                    amountSlider.enable()
                    tempSlider.enable()
                    bakeBtn.enable()

        elif event == kValueChanged:
            if type(control) != type(None):
                knobID = control.getTagID()
                if knobID == kTimeSlider:
                    newVal = int(round(timeSlider.getValue()))
                    if byteTime != newVal:
                        ageSDL[timeSDL] = (newVal,)
                elif knobID == kAmountSlider:
                    newVal = int(round(amountSlider.getValue()))
                    if byteAmount != newVal:
                        ageSDL[amountSDL] = (newVal,)
                elif knobID == kTempSlider:
                    newVal = int(round(tempSlider.getValue()))
                    if byteTemp != newVal:
                        ageSDL[tempSDL] = (newVal,)
        
        elif event == kAction:
            if type(control) != type(None):
                btnID = control.getTagID()
                if btnID == kBakeBtn:
                    if isinstance(control,ptGUIControlButton) and control.isButtonDown():
                        PtDebugPrint("ercaOvenScope:GUINotify Bake button down",level=kDebugDumpLevel)
                    else:
                        PtDebugPrint("ercaOvenScope:GUINotify Bake button up",level=kDebugDumpLevel)
                        if boolMayBake == 1 and IsBaking == 0:
                            timerPercent = byteTime * .01
                            timerWheel.animateToPercent(timerPercent)
#                            if self.sceneobject.isLocallyOwned():
                            ageSDL["ercaBakeFinishTime"] = (1,)
                            PtAtTimeCallback(self.key,1,2)


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
        PtDebugPrint("ercaOvenScope.OnNotify:\twrote SDL - scope operator id = ", avID,level=kDebugDumpLevel)
        # start the behavior
        Behavior.run(LocalAvatar)


    def IEngageTelescope(self):
        global Telescope
        global WasPowered
        global byteTime
        global byteAmount
        global byteTemp
        global timeSlider
        global amountSlider
        global tempSlider
        global bakeBtn
        global exitScope
        
        exitScope = 0
        Telescope.pushTelescope()
        "After the behavior gets our eyes in the telescope, engage ourselves with the camera"
        # Disable First Person Camera
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        # set camera to telescope
        virtCam = ptCamera()
        
        if boolBakeryPwr and boolScopePwr:
            WasPowered = 1
            virtCam.save(Camera.sceneobject.getKey())
            # show the cockpit
            if ( PtIsDialogLoaded(Vignette.value) ):
                timeSlider.setValue(byteTime)
                if byteTime != 0:
                    RespTimeSlider.run(self.key,state="on",fastforward=1)
                else:
                    RespTimeSlider.run(self.key,state="off",fastforward=1)
                amountSlider.setValue(byteAmount)
                if byteAmount != 0:
                    RespAmountSlider.run(self.key,state="on",fastforward=1)
                else:
                    RespAmountSlider.run(self.key,state="off",fastforward=1)
                tempSlider.setValue(byteTemp)
                if byteTemp != 0:
                    RespTempSlider.run(self.key,state="on",fastforward=1)
                else:
                    RespTempSlider.run(self.key,state="off",fastforward=1)
                if boolMayBake:
                    RespMayBake.run(self.key,state="yes",fastforward=1)
                else:
                    RespMayBake.run(self.key,state="no",fastforward=1)

                PtLoadDialog(Vignette.value,self.key)
                if ( PtIsDialogLoaded(Vignette.value) ):
                    PtShowDialog(Vignette.value)

        else:
            WasPowered = 0
            virtCam.save(CameraBad.sceneobject.getKey())
            # show the cockpit
            if type(VignetteBad.value) != type(None) and VignetteBad.value != "":
                PtLoadDialog(VignetteBad.value,self.key)
                if ( PtIsDialogLoaded(VignetteBad.value) ):
                    PtShowDialog(VignetteBad.value)
        
        # get control key events
        PtEnableControlKeyEvents(self.key)


    def IQuitTelescope(self):
        "Disengage and exit the telescope mode"
        global LocalAvatar
        global boolScopeOperator
        global Telescope
        global WasPowered
        global exitScope
        global timerWheel
        global tempWheel

        exitScope = 1
        Telescope.popTelescope()
        # exit every thing
        
        if WasPowered:
            if type(Vignette.value) != type(None) and Vignette.value != "":
                PtHideDialog(Vignette.value)
            virtCam = ptCamera()
            virtCam.restore(Camera.sceneobject.getKey())
            timerWheel.setValue(0)
            RespSfxTimerWheel.run(self.key,state="off")
            tempWheel.setValue(0)
        else:
            if type(VignetteBad.value) != type(None) and VignetteBad.value != "":
                PtHideDialog(VignetteBad.value)
            virtCam = ptCamera()
            virtCam.restore(CameraBad.sceneobject.getKey())
            WasPowered = 1
        
        PtRecenterCamera()
        # exit behavior...which is in the next stage
        #Behavior.gotoStage(LocalAvatar,2)
        Behavior.nextStage(LocalAvatar)
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
        PtDebugPrint("ercaOvenScope.IQuitTelescope:\tdelaying clickable reenable",level=kDebugDumpLevel)


    def OnTimer(self,id):
        global exitScope
        if id == 1:
            Activate.enable()
            PtDebugPrint("ercaOvenScope.OnTimer:\tclickable reenabled",level=kDebugDumpLevel)
            PtSendKIMessage(kEnableKIandBB,0)
        if id == 2:
            if not exitScope:
                self.IDoTimerWheel()


    def IDoTimerWheel(self):
        print "in IDoTimerWheel."
        global byteTime
        global IsBaking
        global timerWheel
        global byteTemp
        global tempWheel
        global setTempWheel
        ageSDL = PtGetAgeSDL()
        
        if not IsBaking:
            return
        print "ercaOvenScope:IDoTimerWheel: IsBaking is true"
        StartTime = (IsBaking - (byteTime * kTimeScale))
        FinishTime = IsBaking
        CurTime = PtGetDniTime()
        TimeRemaining = (FinishTime - CurTime)
        BakeDuration = (FinishTime - StartTime)
        PreHeat = (StartTime - 3)
        PostHeat = (StartTime + 2)
        
        if CurTime < FinishTime:
            #print "PreHeat = ",PreHeat
            #print "CurTime = ",CurTime
            if PreHeat < CurTime:
                #print "PostHeat = ",PostHeat
                if PostHeat < CurTime:
                    print "setTempWheel =",setTempWheel
                    if not setTempWheel:
                        tempWheel.setValue(byteTemp)
                        setTempWheel = 1
                else:
                    if not setTempWheel:
                        tempPercent = byteTemp * .01
                        tempWheel.animateToPercent(tempPercent)
                        print "animating Temp Wheel, byteTemp = ",byteTemp
                        print "animating Temp Wheel, tempPercent = ",tempPercent
                        setTempWheel = 1
            #print "StartTime = ",StartTime
            if StartTime < CurTime:
                PtDebugPrint("ercaOvenScope:IDoTimerWheel:  Now updating timer wheel of scope# %d to %d seconds remaining" % (ScopeNum.value,TimeRemaining))
                TimeRemaining = (TimeRemaining * 1.0)
                BakeDuration = (BakeDuration * 1.0)
                newTimerVal = ((TimeRemaining / BakeDuration) * (byteTime * 10))
                timerWheel.setValue(newTimerVal)
                RespSfxTimerWheel.run(self.key,state="on")
            else:
                timerWheel.setValue(byteTime*10)
            PtAtTimeCallback(self.key,1,2)

