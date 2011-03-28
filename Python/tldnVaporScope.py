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
Module: tldnVaporScope
Age: global
Date: June 9, 2003
Author: Mark DeForest, Bill Slease, Doug McBride
This is the handler for the Vapor miner scope thingy that shoots wharks and rocks
- Needed to solve the "get up in the second floor of the warshroom"
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys
import string

# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "Activator for VaporScope",netForce=1)
Camera = ptAttribSceneobject(2,"VaporScope camera")
Behavior = ptAttribBehavior(3, "VaporScope behavior (multistage)",netForce=1)
Vignette = ptAttribString(4,"Vignette dialog - by Name")

animVaporTurret = ptAttribAnimation(10, "Anim: VaporTurret",netForce=1)
animVaporPitch = ptAttribAnimation(11, "Anim: VaporPitch",netForce=1)

actStopAtTop = ptAttribActivator(12, "Actvtr: Stop Scope @ Top")
actStopAtBottom = ptAttribActivator(13, "Actvtr: Stop Scope @ Btm")


respShootGun  = ptAttribResponder(20,"Rspndr: ShootVapor")


#-----------------------------------
# konstants for the GUI buttons

kLeftScopeBtn = 36
kRightScopeBtn = 37
kUpScopeBtn = 38
kDownScopeBtn = 39
kFireScopeBtn = 40
kZoomSlider = 100

kExitButton = 199

kMinFOV = 70.0
kMaxFOV = 25.0
kDegreesPerSlider = float(kMinFOV-kMaxFOV)/10.0

kTimerDisengage = 1
kTimerDisengageTime = 3

kTimerThrottleFiring = 2
kTimerThrottleTime = 0.25

# ---------
# globals

boolScopeAtTop = 0
boolScopeAtBottom = 1

gInitialFOV = 0.0

gThrottleShooting = 0

LocalAvatar = None
boolScopeOperator = 0
boolOperated = 0

class tldnVaporScope(ptModifier):
    "Standard telescope modifier class"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5190
        maxVersion = 5
        minorVersion = 6
        self.version = maxVersion
        PtDebugPrint("tldnVaporScope::init v%d.%d" % (maxVersion,minorVersion))

    def OnFirstUpdate(self):
        self.SDL.setDefault("boolOperated",(0,))
        self.SDL.setDefault("OperatorID",(-1,))
        self.SDL.setDefault("scopeAtTop",(0,))
        self.SDL.setDefault("scopeAtBottom",(1,))
        self.SDL.sendToClients("boolOperated")
        self.SDL.sendToClients("OperatorID")
        self.SDL.sendToClients("scopeAtTop")
        self.SDL.sendToClients("scopeAtBottom")

    def Load(self):
        global boolScopeOperated
        solo = true
        if len(PtGetPlayerList()):
            solo = false
        boolOperated = self.SDL["boolOperated"][0]
        if boolOperated:
            if solo:
                PtDebugPrint("tldnVaporScope.Load():\tboolOperated=%d but no one else here...correcting" % boolOperated,level=kDebugDumpLevel)
                boolOperated = 0
                self.SDL["boolOperated"] = (0,)
                self.SDL["OperatorID"] = (-1,)
                Activate.enable()
            else:
                Activate.disable()
                PtDebugPrint("tldnVaporScope.Load():\tboolOperated=%d, disabling telescope clickable" % boolOperated,level=kDebugDumpLevel)

    def OnServerInitComplete(self):
        global boolScopeAtTop
        global boolScopeAtBottom
        boolScopeAtTop = self.SDL["scopeAtTop"][0]
        boolScopeAtBottom = self.SDL["scopeAtBottom"][0]

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
            PtDebugPrint("tldnVaporScope.AvatarPage(): telescope operator paged out, reenabled telescope.",level=kDebugDumpLevel)
        else:
            return
            
    def __del__(self):
        "unload the dialog that we loaded"
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtUnloadDialog(Vignette.value)

    def OnNotify(self,state,id,events):
        "Activated... start telescope"
        global LocalAvatar
        global boolScopeOperator
        global boolScopeAtTop
        global boolScopeAtBottom
        PtDebugPrint("tldnVaporScope:OnNotify  state=%f id=%d events=" % (state,id),events,level=kDebugDumpLevel)

        #####################################
        # Stop Scope at vertical boundaries #
        #####################################
        if state and id==actStopAtTop.id:
            PtDebugPrint("tldnVaporScope: got event - at top",level=kDebugDumpLevel)
            boolScopeAtTop = 1
            self.SDL["scopeAtTop"] = (1,)
            return     
        if state and id==actStopAtBottom.id:
            PtDebugPrint("tldnVaporScope: got event - near bottom",level=kDebugDumpLevel)
            boolScopeAtBottom = 1
            self.SDL["scopeAtBottom"] = (1,)
            return            

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
        global boolScopeAtTop
        global boolScopeAtBottom
        global gInitialFOV
        global gThrottleShooting
        #PtDebugPrint("tldnVaporScope:GUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel)
        if event == kDialogLoaded:
            # if the dialog was just loaded then show it
            control.show()
        elif event == kShowHide:
            if control.isEnabled():
                # get the FOV and set the Zoom slider
                curCam = ptCamera()
                gInitialFOV = curCam.getFOV()
                zoomSlider = ptGUIControlKnob(control.getControlFromTag(kZoomSlider))
                if gInitialFOV > kMinFOV:
                    gInitialFOV = kMinFOV
                elif gInitialFOV < kMaxFOV:
                    gInitialFOV = kMaxFOV
                zsliderValue = (gInitialFOV-kMaxFOV)/kDegreesPerSlider
                zoomSlider.setValue(zsliderValue)
                PtDebugPrint("tldnVaporScope:ShowDialog:  current FOVh at %f - setting slider to %f" % (gInitialFOV,zsliderValue))
        elif event == kValueChanged:
            if type(control) != type(None):
                knobID = control.getTagID()
                if knobID == kZoomSlider:
                    newFOV = (control.getValue() * kDegreesPerSlider) + kMaxFOV
                    curCam = ptCamera()
                    curCam.setFOV(newFOV,0.5)
                    PtDebugPrint("tldnVaporScope:ValueChange:  slider=%f and setting FOV to %f" % (control.getValue(),newFOV))
        elif event == kAction:
            if type(control) != type(None):
                btnID = control.getTagID()
                if btnID == kLeftScopeBtn:
                    if isinstance(control,ptGUIControlButton) and control.isButtonDown():
                        PtDebugPrint("tldnVaporScope:GUINotify Left button down",level=kDebugDumpLevel)
                        animVaporTurret.value.backwards(0)
                        animVaporTurret.value.resume()
                    else:
                        PtDebugPrint("tldnVaporScope:GUINotify Left button up",level=kDebugDumpLevel)
                        animVaporTurret.value.stop()
                elif btnID == kRightScopeBtn:
                    if isinstance(control,ptGUIControlButton) and control.isButtonDown():
                        PtDebugPrint("tldnVaporScope:GUINotify Right button down",level=kDebugDumpLevel)
                        animVaporTurret.value.backwards(1)
                        animVaporTurret.value.resume()
                    else:
                        PtDebugPrint("tldnVaporScope:GUINotify Right button up",level=kDebugDumpLevel)
                        animVaporTurret.value.stop()
                elif btnID == kUpScopeBtn:
                    if isinstance(control,ptGUIControlButton) and control.isButtonDown():
                        if not boolScopeAtTop:
                            PtDebugPrint("tldnVaporScope:GUINotify Up button down",level=kDebugDumpLevel)
                            animVaporPitch.value.backwards(0)
                            animVaporPitch.value.resume()
                    else:
                        PtDebugPrint("tldnVaporScope:GUINotify Up button up",level=kDebugDumpLevel)
                        animVaporPitch.value.stop()
                    if boolScopeAtBottom:
                        boolScopeAtBottom = 0
                        self.SDL["scopeAtBottom"] = (0,)
                elif btnID == kDownScopeBtn:
                    if isinstance(control,ptGUIControlButton) and control.isButtonDown():
                        if not boolScopeAtBottom:
                            PtDebugPrint("tldnVaporScope:GUINotify Down button down",level=kDebugDumpLevel)
                            animVaporPitch.value.backwards(1)
                            animVaporPitch.value.resume()
                    else:
                        PtDebugPrint("tldnVaporScope:GUINotify Down button up",level=kDebugDumpLevel)
                        animVaporPitch.value.stop()
                    if boolScopeAtTop:
                        boolScopeAtTop = 0
                        self.SDL["scopeAtTop"] = (0,)
                elif btnID == kFireScopeBtn:
                    if not gThrottleShooting:
                        PtDebugPrint("tldnVaporScope:GUINotify Shoot vapor",level=kDebugDumpLevel)
                        respShootGun.run(self.key)
                        ####-# reset the slider back to initialFOV, cause the animation of the scope will set it back
                        ####-zoomSlider = ptGUIControlKnob(control.getOwnerDialog().getControlFromTag(kZoomSlider))
                        ####-zsliderValue = (gInitialFOV-kMaxFOV)/kDegreesPerSlider
                        ####-zoomSlider.setValue(zsliderValue)
                        # need to see if we hit anything and run their responder
                        PtRequestLOSScreen(self.key,42,0.5,0.5,10000,PtLOSObjectType.kShootable,PtLOSReportType.kReportHitOrMiss)
                        gThrottleShooting = 1
                        try:
                            if type(Vignette.value) != type(None) and Vignette.value != "":
                                scopeDlg = PtGetDialogFromString(Vignette.value)
                                if scopeDlg:
                                    try:
                                        fireBtn = ptGUIControlButton(scopeDlg.getControlFromTag(kFireScopeBtn))
                                        fireBtn.disable()
                                    except KeyError:
                                        PtDebugPrint("tldnVaporScope:GUINotify can't find the fire button",level=kDebugDumpLevel)
                        except KeyError:
                            PtDebugPrint("tldnVaporScope:GUINotify can't find VaporScope dialog",level=kDebugDumpLevel)
                        PtAtTimeCallback(self.key,kTimerThrottleTime,kTimerThrottleFiring) # wait for player to finish exit one-shot, then reenable clickable
                    else:
                        PtDebugPrint("tldnVaporScope:GUINotify Throttling",level=kDebugDumpLevel)
                elif btnID == kExitButton:
                    self.IQuitTelescope()


    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IQuitTelescope()
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IQuitTelescope()

    def OnLOSNotify(self,ID,noHitFlag,sceneobject,hitPoint,distance):
        PtDebugPrint("tldnVaporScope:LOSNotify:  ID=%d  noHitFlag=%d at a distance of %g" % (ID,noHitFlag,distance),level=kDebugDumpLevel)
        PtShootBulletFromScreen(self.key,0.5,0.5,1.0,10000)
        if sceneobject:
            PtDebugPrint("tldnVaporScope:LOSNotify: ===>hit object %s at point(%g,%g,%g)" % (sceneobject.getName(),hitPoint.getX(),hitPoint.getY(),hitPoint.getZ()),level=kDebugDumpLevel)
            # first look for a python file guy (before responders)
            pmlist = sceneobject.getPythonMods()
            if len(pmlist) > 0:
                PtDebugPrint("tldnVaporScope:LOSNotify:  ...python mod list:",level=kDebugDumpLevel)
                for pm in pmlist:
                    PtDebugPrint("       %s" % (pm.getName()),level=kDebugDumpLevel)
                    if string.lower(pm.getName()).startswith("vaporminerhitme"):
                        PtDebugPrint("tldnVaporScope:LOS: VaporMiner HIT!",level=kDebugDumpLevel)
                        notify = ptNotify(self.key)
                        notify.clearReceivers()
                        notify.addReceiver(pm)
                        notify.setActivate(1.0)
                        notify.send()
            else:
                PtDebugPrint("tldnVaporScope:LOSNotify: ...no python mods found",level=kDebugDumpLevel)
            # next look for responders attached
            resplist = sceneobject.getResponders()
            if len(resplist) > 0:
                PtDebugPrint("tldnVaporScope:LOSNotify:  ...responder list:",level=kDebugDumpLevel)
                for resp in resplist:
                    PtDebugPrint("       %s" % (resp.getName()),level=kDebugDumpLevel)
                    if string.lower(resp.getName()).startswith("vaporminerhitme"):
                        PtDebugPrint("tldnVaporScope:LOS: VaporMiner HIT!",level=kDebugDumpLevel)
                        atResp = ptAttribResponder(42)
                        atResp.__setvalue__(resp)
                        atResp.run(self.key)
            else:
                PtDebugPrint("tldnVaporScope:LOSNotify: ...no responders found",level=kDebugDumpLevel)
        else:
            PtDebugPrint("tldnVaporScope:LOSNotify: No sceneobject found...",level=kDebugDumpLevel)

    def IStartTelescope(self):
        "Start the action of looking at the telescope"
        global LocalAvatar
        global boolScopeOperator
        # disable the activator (only one in the telescope at a time)
        Activate.disable()
        PtSendKIMessage(kDisableKIandBB,0)
        boolScopeOperator = 1  # me! I'm the operator
        self.SDL["boolOperated"] = (1,)
        avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
        self.SDL["OperatorID"] = (avID,)
        PtDebugPrint("tldnVaporScope.OnNotify:\twrote SDL - scope operator id = ", avID,level=kDebugDumpLevel)
        # start the behavior
        Behavior.run(LocalAvatar)
        

    def IEngageTelescope(self):
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
            PtLoadDialog(Vignette.value,self.key, "Teledahn")
            if ( PtIsDialogLoaded(Vignette.value) ):
                PtShowDialog(Vignette.value)
        # get control key events
        PtEnableControlKeyEvents(self.key)

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
        # re-enable the telescope for someone else to use
        boolScopeOperator = 0
        self.SDL["boolOperated"] = (0,)
        self.SDL["OperatorID"] = (-1,)
        #Re-enable first person camera
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        PtAtTimeCallback(self.key,kTimerDisengageTime,kTimerDisengage) # wait for player to finish exit one-shot, then reenable clickable
        PtDebugPrint("tldnVaporScope.IQuitTelescope:\tdelaying clickable reenable")
        
    def OnTimer(self,id):
        global gThrottleShooting
        if id == kTimerDisengage:
            Activate.enable()
            PtSendKIMessage(kEnableKIandBB,0)
            PtDebugPrint("tldnVaporScope.OnTimer:\tclickable reenabled",level=kDebugDumpLevel)
        elif id == kTimerThrottleFiring:
            # we can not let firing happen... again
            gThrottleShooting = 0
            try:
                if type(Vignette.value) != type(None) and Vignette.value != "":
                    scopeDlg = PtGetDialogFromString(Vignette.value)
                    if scopeDlg:
                        try:
                            fireBtn = ptGUIControlButton(scopeDlg.getControlFromTag(kFireScopeBtn))
                            fireBtn.enable()
                        except KeyError:
                            PtDebugPrint("tldnVaporScope:Timer can't find the fire button",level=kDebugDumpLevel)
            except KeyError:
                PtDebugPrint("tldnVaporScope:Timer can't find VaporScope dialog",level=kDebugDumpLevel)
