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
Module: tldnWRCCBrain
Age: Teledahn
Date: April 2002
Author: Bill Slease
-- Brain for the Workroom Control Center
-- hatch one-shot action here for multiplayer timing reasons
-- bucket controls NOT here for multiplayer timing reasons: see tldnBucketBrain
"""

from Plasma import *
from PlasmaTypes import *

#Globals for Responder States
kOn  = "on"
kOff = "off"


# define the attributes that will be entered in max

actPwr  = ptAttribNamedActivator(1,"Actvtr: Power")

actDrainLvr = ptAttribActivator(2, "Actvtr: Drain Lever")
respDrainLvr = ptAttribResponder(3, "Rspndr: Drain Lever",['drain','pull','off','reset'])
respCabinDrain = ptAttribNamedResponder(4, "Rspndr: Cabin Drain",['drain','reset'])

actLockSwitch = ptAttribActivator(5, "Actvtr: Hatch Lock Switch")
respLockSwitch = ptAttribResponder(6, "Rspndr: Hatch Lock Switch",['unlock','lock','lockfail','unlockfail'])
respHatchLock = ptAttribNamedResponder(7, "Rspndr: Hatch Lock",['on','off','flash','stopflash'])

actOpenHatchAbv = ptAttribNamedActivator(8, "Actvtr: Open Hatch from Abv")
actCloseHatch =  ptAttribNamedActivator(10, "Actvtr: Close Hatch")
respHatchOps = ptAttribNamedResponder(11, "Rspndr: Hatch Ops",['lockedabove','openabove','lockedbelow','openbelow','close'])
respHatchLadder = ptAttribNamedResponder(12, "Rspndr: Hatch Ladder",['enable','disable'])

respHatchGlare=ptAttribResponder(13, "HatchPanelToggle",['on','off'])

actHatchOpenedA  = ptAttribNamedActivator(14,"Actvtr: Ldr Up Open Hatch")
actHatchOpenedB  = ptAttribNamedActivator(15,"Actvtr: Ldr Dn Open Hatch")

respBlocker = ptAttribNamedResponder(16, "Rspndr: Hole Blocker",['on','off'])

actSitClickable = ptAttribActivator(17,"Actvtr: Sit Clickable")

actSitComponent = ptAttribActivator(18,"The Sit down component")

respWRCCMaster = ptAttribResponder(19, "resp: WRCC master dummy",['on','off'])

actBucketLoopMode = ptAttribActivator(20, "actvtr: Bucket Loop Mode")
respBucketLoopMode = ptAttribResponder(21, "resp: Bucket Loop Mode", [kOn, kOff])


# global variables

pwrOn = 0 
hatchLocked = 1
hatchOpen = 0
cabinDrained = 0
cabinDraining = 0
kStringAgeSDLPowerOn = "tldnWorkroomPowerOn"
kStringAgeSDLCabinDrained = "tldnCabinDrained"
kStringAgeSDLHatchOpen = "tldnHatchOpen"
kStringAgeSDLHatchLocked = "tldnHatchLocked"
kStringAgeSDLBucketLoopMode = "tldnBucketContinuousLoopMode"

AgeStartedIn = None

class tldnWRCCBrain(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5010
        version = 9
        self.version = version

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
        # non-age sdl vars
        self.SDL.setDefault("boolOperated",(0,))
        self.SDL.setDefault("OperatorID",(-1,))
        self.SDL.sendToClients("boolOperated")
        self.SDL.sendToClients("OperatorID")

    def AvatarPage(self, avObj, pageIn, lastOut):
        "reset wrcc accessibility if wrcc sitter quits or crashes"
        global boolScopeOperated
        
        if pageIn:
            return
            
        avID = PtGetClientIDFromAvatarKey(avObj.getKey())
        if avID == self.SDL["OperatorID"][0]:
            actSitClickable.enable()
            self.SDL["OperatorID"] = (-1,)
            self.SDL["boolOperated"] = (0,)
            #respWRCCMaster.run(self.key,state='off')
            print "tldnWRCCBrain.AvatarPage(): WRCC operator paged out, reenabled WRCC."
        else:
            return


    def Load(self):
        global boolWRCCOperated
        
        solo = true
        if len(PtGetPlayerList()):
            solo = false

        boolOperated = self.SDL["boolOperated"][0]
        if boolOperated:
            if solo:
                print "tldnWRCCBrain.Load():\tboolOperated=%d but no one else here...correcting" % boolOperated
                boolOperated = 0
                self.SDL["boolOperated"] = (0,)
                self.SDL["OperatorID"] = (-1,)
                actSitClickable.enable()
                #respWRCCMaster.run(self.key,state='off')
            else:
                actSitClickable.disable()
                print "tldnWRCCBrain.Load():\tboolOperated=%d, disabling WRCC clickable" % boolOperated


    def OnServerInitComplete(self):
        global pwrOn
        global hatchLocked
        global hatchOpen
        global cabinDrained
        
        # register for notification of age SDL var changes
        if AgeStartedIn == PtGetAgeName():
            # age sdl vars
            ageSDL = PtGetAgeSDL()
            # set flags on age SDL vars we'll be changing
            ageSDL.setFlags(kStringAgeSDLCabinDrained,1,1)
            ageSDL.sendToClients(kStringAgeSDLCabinDrained)
            ageSDL.setFlags(kStringAgeSDLHatchOpen,1,1)
            ageSDL.sendToClients(kStringAgeSDLHatchOpen)
            ageSDL.setFlags(kStringAgeSDLHatchLocked,1,1)
            ageSDL.sendToClients(kStringAgeSDLHatchLocked)
    
            ageSDL.setNotify(self.key,kStringAgeSDLPowerOn,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLCabinDrained,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLHatchOpen,0.0)
            ageSDL.setNotify(self.key,kStringAgeSDLHatchLocked,0.0)

            ageSDL.setFlags(kStringAgeSDLBucketLoopMode,1,1)
            ageSDL.sendToClients(kStringAgeSDLBucketLoopMode) 
            ageSDL.setNotify(self.key,kStringAgeSDLBucketLoopMode,0.0)

            # get initial SDL state
            try:
                pwrOn = ageSDL[kStringAgeSDLPowerOn][0]
                cabinDrained = ageSDL[kStringAgeSDLCabinDrained][0]
                hatchOpen = ageSDL[kStringAgeSDLHatchOpen][0]
                hatchLocked = ageSDL[kStringAgeSDLHatchLocked][0]
                if hatchLocked and hatchOpen:
                    # invalid state - correct it
                    hatchLocked = 0
                    ageSDL[kStringAgeSDLHatchLocked] = (0,)                
            except:
                pwrOn = false
                cabinDrained = false
                hatchOpen = false
                hatchLocked = true
                PtDebugPrint("tldnWRCCBrain.OnServerInitComplete():\tERROR: age sdl read failed, defaulting:")
            PtDebugPrint("tldnWRCCBrain.OnServerInitComplete():\t%s = %d, %s=%d" % (kStringAgeSDLPowerOn,pwrOn,kStringAgeSDLCabinDrained,cabinDrained) )
            PtDebugPrint("tldnWRCCBrain.OnServerInitComplete():\t%s = %d, %s=%d" % (kStringAgeSDLHatchOpen,hatchOpen,kStringAgeSDLHatchLocked,hatchLocked) )
            
            # init cabin drained
            if cabinDrained:
                respDrainLvr.run(self.key,state='drain',fastforward=true) # move the needle to drained position
                respCabinDrain.run(self.key,state='drain',fastforward=true)
            # init wrcc lock switch and hatch state LED
            if hatchLocked:
                respLockSwitch.run(self.key,state='lock',fastforward=true)
            else:
                respLockSwitch.run(self.key,state='unlock',fastforward=true)
            if hatchOpen and pwrOn:
                respHatchGlare.run(self.key,state='on',fastforward=true)
            else:
                respHatchGlare.run(self.key,state='off',fastforward=true)
            # init hatchLocked LED, open/closed, detectors, exclude regions
            if not hatchLocked and pwrOn:
                respHatchLock.run(self.key,state='on',fastforward=true)
            else:
                respHatchLock.run(self.key,state='off',fastforward=true)
            if hatchOpen:
                respHatchOps.run(self.key,state='openabove',fastforward=true)
                actCloseHatch.enable()
                actOpenHatchAbv.disable()
                respBlocker.run(self.key,state='on')
            else:
                respHatchOps.run(self.key,state='close',fastforward=true)
                actCloseHatch.disable()
                actOpenHatchAbv.enable()
                respBlocker.run(self.key,state='off')

            #Setup Bucket Loop Mode
            try:
                bucketMode = ageSDL[kStringAgeSDLBucketLoopMode][0]
            except:
                PtDebugPrint("tldnWRCCBrain.OnServerInitComplete():\tCould not read Bucket Loop Mode SDL, defaulting to: ON")
                bucketMode = true
                ageSDL[kStringAgeSDLBucketLoopMode] = (bucketMode,)

            if bucketMode == true:
                respBucketLoopMode.run(self.key, state=kOn, fastforward=true)
            else:
                respBucketLoopMode.run(self.key, state=kOff, fastforward=true)

            return


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global pwrOn
        global hatchLocked
        global hatchOpen
        global cabinDrained
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("tldnWRCCBrain.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
            
            if tag == "ignore":
                return
    
            ############
            # Power On/Off   #
            ############
            if VARname == kStringAgeSDLPowerOn:
                pwrOn = ageSDL[kStringAgeSDLPowerOn][0]
                if pwrOn:
                    # power came on...if LEDs should be on, turn em on now
                    if not hatchLocked:
                        respHatchLock.run(self.key,state='on')
                    if hatchOpen:
                        respHatchGlare.run(self.key,state='on')                    
                else:
                    # power went out...if LEDs were on, turn em off
                    if not hatchLocked:
                        respHatchLock.run(self.key,state='off')
                    if hatchOpen:
                        respHatchGlare.run(self.key,state='off')                                        
                return
    
            ############
            # Cabin Drained   #
            ############
            if VARname == kStringAgeSDLCabinDrained:
                cabinDrained = ageSDL[kStringAgeSDLCabinDrained][0]
                if playerID:
                    return
                # so assume change is from vault manager - do some scene management
                if cabinDrained:
                    respDrainLvr.run(self.key,state='drain',fastforward=true)
                    respCabinDrain.run(self.key,state='drain',fastforward=true)
                else:
                    respDrainLvr.run(self.key,state='reset',fastforward=true)                
                    respCabinDrain.run(self.key,state='reset',fastforward=true)
                return
    
            ##############
            # Hatch Open/Close    #
            ##############
            if VARname == kStringAgeSDLHatchOpen:
                hatchOpen = ageSDL[kStringAgeSDLHatchOpen][0]
                if playerID:
                    return
                # so assume change is from vault manager - do some scene management
                if hatchOpen:
                    respHatchOps.run(self.key,state='openabove',fastforward=true)
                    actCloseHatch.enable()
                    actOpenHatchAbv.disable()
                    respBlocker.run(self.key,state='on')
                    if pwrOn:
                        respHatchGlare.run(self.key,state='on',fastforward=true)
                else:
                    respHatchOps.run(self.key,state='close',fastforward=true)
                    actCloseHatch.disable()
                    actOpenHatchAbv.enable()
                    respBlocker.run(self.key,state='off')
                    if pwrOn: # light wasn't on otherwise
                        respHatchGlare.run(self.key,state='off',fastforward=true)
                return
    
            ###########
            # Hatch Lock    #
            ###########
            if VARname == kStringAgeSDLHatchLocked:
                hatchLocked = ageSDL[kStringAgeSDLHatchLocked][0]
                if playerID:
                    return
                # so assume change is from vault manager - do some scene management
                if hatchLocked:
                    respLockSwitch.run(self.key,state='lock',fastforward=true)
                    if pwrOn:  # light wasn't on otherwise
                        respHatchLock.run(self.key,state='off',fastforward=true)
                else:
                    respLockSwitch.run(self.key,state='unlock',fastforward=true)
                    if pwrOn:
                        respHatchLock.run(self.key,state='on',fastforward=true)
                return


            ###########
            # Bucket Loop Mode    #
            ###########
            if VARname == kStringAgeSDLBucketLoopMode:
                bucketMode = ageSDL[kStringAgeSDLBucketLoopMode][0]
                if bucketMode == true:
                    respBucketLoopMode.run(self.key, state=kOn)
                else:
                    respBucketLoopMode.run(self.key, state=kOff)
                return
            


    def OnNotify(self,state,id,events):
        global cabinDraining
        global hatchLocked
        global hatchOpen
        global cabinDrained
        global LocalAvatar
        
        for event in events: #this detects the untrigger of the sitting component
            if event[0]==6 and event[1]==1 and id== actSitComponent.id:
                if state==0: #true as player stands up
                    print "tldnWRCCBrain.OnNotify():\tWRCC unoccupied, re-enabling WRCC sit clickable."
                    actSitClickable.enable()
                    self.SDL["boolOperated"] = (0,)
                    self.SDL["OperatorID"] = (-1,)
                    
                    if PtWasLocallyNotified(self.key):
                        PtEnableMouseMovement()
                        print "tldnWRCCBrain.OnNotify():\tI'm the one who stood up, will hide panel's clickables"
                        respWRCCMaster.run(self.key,state='on')

        if not state: # notification is from some kind of untrigger
            return

        if id==actSitClickable.id:
            print "tldnWRCCBrain.OnNotify():\tWRCC occupied, disabling WRCC sit clickable."
            actSitClickable.disable()
            self.SDL["boolOperated"] = (1,)
                    
            LocalAvatar = PtFindAvatar(events)
            avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
            self.SDL["OperatorID"] = (avID,)
            print "tldnWRCCBrain.OnNotify:\twrote SDL - scope operator id = ", avID 
            
            if PtWasLocallyNotified(self.key):
                PtDisableMouseMovement()
                print "tldnWRCCBrain.OnNotify():\tI'm the one who sat down, will show panel's clickables"
                respWRCCMaster.run(self.key,state='off')
        
        if id==actDrainLvr.id:
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                if pwrOn and hatchLocked and not cabinDrained:
                    respDrainLvr.run(self.key,state='drain')
                    print "tldnWRCCBrain.OnNotify:\tDraining under-cabin."
                    ageSDL.setTagString(kStringAgeSDLCabinDrained,"ignore") # ignore so we don't set global cabinDrained yet...see respCabinDrain callback below
                    ageSDL[kStringAgeSDLCabinDrained] = (1,) # set this now so folks coming in will just see it drained
                elif pwrOn and hatchLocked and cabinDrained:
                    respDrainLvr.run(self.key,state='pull')
                    print "tldnWRCCBrain.OnNotify:\tUnder-cabin already empty."
                else:
                    respDrainLvr.run(self.key,state='off')
                    print "tldnWRCCBrain.OnNotify:\tCan't operate drain -- pwrOn=%d hatchLocked=%d." % (pwrOn,hatchLocked)
            return

        if id==respDrainLvr.id:
                respCabinDrain.run(self.key,state='drain')
                respHatchLock.run(self.key,state='flash')                               
                cabinDraining = 1
                return

        if id==respCabinDrain.id:
                respHatchLock.run(self.key,state='stopflash')                           
                cabinDraining = 0
                cabinDrained = 1 # set sdl earlier - this is just for people who were here while all this was going on
                return          

        if id==actLockSwitch.id:
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                if pwrOn and not hatchOpen and not cabinDraining:
                    if hatchLocked:
                        respLockSwitch.run(self.key,state='unlock')
                        respHatchLock.run(self.key,state='on')
                        hatchLocked = 0
                        ageSDL[kStringAgeSDLHatchLocked] = (0,)
                    else:
                        respLockSwitch.run(self.key,state='lock')
                        respHatchLock.run(self.key,state='off')
                        hatchLocked = 1
                        ageSDL[kStringAgeSDLHatchLocked] = (1,)
                elif hatchLocked:
                    respLockSwitch.run(self.key,state='unlockfail')
                    print "tldnWRCCBrain.OnNotify:\tCan't unlock hatch -- pwrOn=%d hatchOpen=%d cabinDraining=%d." % (pwrOn,hatchOpen,cabinDraining)
                else:
                    respLockSwitch.run(self.key,state='lockfail')
                    print "tldnWRCCBrain.OnNotify:\tCan't lock hatch -- pwrOn=%d hatchOpen=%d cabinDraining=%d." % (pwrOn,hatchOpen,cabinDraining)
            return

        if id==actOpenHatchAbv.id:
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                if hatchLocked:
                    respHatchOps.run(self.key,events=events,state='lockedabove')
                elif not hatchLocked and not hatchOpen:
                    respHatchOps.run(self.key,events=events,state='openabove')
                    hatchOpen = 1
                    ageSDL[kStringAgeSDLHatchOpen] = (1,)
                    respHatchGlare.run(self.key,events=events,state='on') #panel glare on - hatch is open
            return

        if id==actCloseHatch.id:
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                respHatchOps.run(self.key,events=events,state='close')
                #~ respHatchLadder.run(self.key,state='disable')
                hatchOpen = 0
                ageSDL[kStringAgeSDLHatchOpen] = (0,)
                respHatchGlare.run(self.key,events=events,state='off') #panel glare off - hatch is closed
            return

        
        if id == actBucketLoopMode.id:
            ageSDL = PtGetAgeSDL()
            bucketMode = ageSDL[kStringAgeSDLBucketLoopMode][0]
            if bucketMode == true:
                ageSDL[kStringAgeSDLBucketLoopMode] = (false,)
            else:
                ageSDL[kStringAgeSDLBucketLoopMode] = (true,)


