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
Module: tldnBucketBrain
Age: Teledahn
Author: Bill Slease, Doug McBride
Redesigned by: Tye Hooley
Date: April 2002
Handles all aspects of teledahn's bucket system operations
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys
from xEnum import Enum

#These variables are global but are necessary for the max inputs:
kOff = "off"
kOn  = "on"

# define the attributes that will be entered in max
actGo1  = ptAttribActivator(1,"Actvtr: Send Lever")
actPwr  = ptAttribActivator(3,"Deprecated")    #Not Used

respGo = ptAttribResponder(2,"Rspndr: Make Buckets Go")
respStop = ptAttribResponder(4,"Rspndr: Make Buckets Stop")

objBucket1 = ptAttribSceneobject(5,"Bucket #1")
objBucket2 = ptAttribSceneobject(6,"Bucket #2")
objBucket3 = ptAttribSceneobject(7,"Bucket #3")
objBucket4 = ptAttribSceneobject(8,"Bucket #4")

actBktEnter1  = ptAttribActivator(9,"Act: Enter Bucket CLICK")
respBktEnter = ptAttribResponder(10,"Rspndr: Enter Bucket")

actDumpAvatar = ptAttribActivator(12,"Actvtr: Dump Avatar")
respDumpAvatar = ptAttribResponder(13,"Rspndr: Dump Avatar")

actDoorClose = ptAttribActivator(14,"Actvtr: Close Door")
respDoorClose = ptAttribResponder(15,"Rspndr: Close Door")
actDoorOpen = ptAttribActivator(16,"Actvtr: Open Door")
respDoorOpen = ptAttribResponder(17,"Rspndr: Open Door")

actEntryPoint = ptAttribActivator(18,"Actvtr: At Entry Point")

actWRCCGo  = ptAttribNamedActivator(19,"Actvtr: WRCC Send Lever")
respWRCCGo  = ptAttribNamedResponder(20,"Rspndr: WRCC Send Lever")

actMode  = ptAttribNamedActivator(21,"Actvtr: WRCC Mode Switch")  #Not USED!

respBktOpen01 = ptAttribResponder(23,"Rspndr: bkt bottom #1")
respBktOpen02 = ptAttribResponder(24,"Rspndr: bkt bottom #2")
respBktOpen03 = ptAttribResponder(25,"Rspndr: bkt bottom #3")
respBktOpen04 = ptAttribResponder(26,"Rspndr: bkt bottom #4")

respSpores = ptAttribNamedResponder(27,"Rspndr: workroom fx")

actBktExit  = ptAttribActivator(28,"Actvtr: Exit Bucket")
respBktExit = ptAttribResponder(29,"Rspndr: Exit Bucket",netForce=1)

respDumpCam = ptAttribResponder(30,"Rspndr: Dump Cam")
respWRCCLED = ptAttribNamedResponder(31,"Rspndr: WRCC LED", [kOff, kOn])

respInitSounds = ptAttribResponder(32,"Rspndr: Init Sounds")  #Not USED!

respExtSendLeverPull = ptAttribResponder(33,"Rspndr: Ext Lvr Pull")
respExtSendLeverPush = ptAttribResponder(34,"Rspndr: Ext Lvr Push")
respExtSendLeverTick = ptAttribResponder(35,"Deprecated")   #Not USED!

respTimerPwrOffRun = ptAttribResponder(36,"Rspndr: Timer PwrOff")
actTimerPwrOff  = ptAttribActivator(37,"Actvtr: Timer PwrOff")
respTimerBktDelayRun = ptAttribResponder(38,"Rspndr: Timer BktDelay")
actTimerBktDelay  = ptAttribActivator(39,"Actvtr: Timer BktDelay")

actBktEnter2  = ptAttribActivator(40,"Act: Enter Bucket WALK")

respCountdownStart  = ptAttribResponder(41,"Resp: SFX Countdown Start")
respCountdownStop  = ptAttribResponder(42,"Resp: SFX Countdown Stop")

SensorAtEntry = ptAttribSceneobject(43,"SensorAtEntry")

actEnteredBucketByRail = ptAttribActivator(44,"act: Entered Bucket By Rail")  #Not used!
actExitedBucketByRail = ptAttribActivator(45,"act: Exited Bucket By Rail")    #Not used!
respAnimSensorAtEntry = ptAttribResponder(46,"resp: Animate SensorAtEntry")   #Not used!

#Camera Control Max Attributes
Camera01     = ptAttribSceneobject(47,"Bucket Cam 1")   
Camera02     = ptAttribSceneobject(48,"Bucket Cam 2")
Camera03     = ptAttribSceneobject(49,"Bucket Dump Cam 3")
rgnSensor01  = ptAttribActivator(50,"Bucket Cam 1 Region")
rgnSensor02  = ptAttribActivator(51,"Bucket Cam 2 Region")
rgnSensor03  = ptAttribActivator(52,"Bucket Dump Cam 3 Region")

# global variables 
AgeStartedIn = None         #The age this script started in...  (do we even need this anymore???)
buckets = ()                    #The bucket objects
scriptInitialized = 0   #If the script has finished initializing...
avaIDAtEntry  = 0
avaIDAtDump   = 0

#AgeSDL variable names
kStringAgeSDLLoopMode      = "tldnBucketContinuousLoopMode"
kStringAgeSDLLeverPulled   = "tldnBucketLowerLeverPulled"
kStringAgeSDLBucketState   = "tldnBucketState"
kStringAgeSDLRiders        = "tldnRiders"
kStringAgeSDLBucketAtEntry = "tldnBucketAtEntry"
kStringAgeSDLShrmDoorOpen  = "tldnShrmDoorOpen"
kStringAgeSDLBucketAtDump  = "tldnBucketAtDump"
kStringAgeSDLPowerOn       = "tldnWorkroomPowerOn"

#Bucket States and Inputs
kBucketStates = Enum("Stop, QRun, QBoardQRun, Run, Dump, QStop, DumpQBoard, DumpQStop, DumpQBoardQStop, QBoard, Boarded")
kBucketInputs = Enum("Power, Timer, StopCB, Board, BoardCB, Dump, DumpCB, WCPswitch")

      

class tldnBucketBrain(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5006
        
        version = 18
        self.version = version
        PtDebugPrint("__init__tldnBucketBrain v.%s" %version)


    def OnFirstUpdate(self):
        global AgeStartedIn

        AgeStartedIn = PtGetAgeName()

        #~ SensorAtEntry.value.physics.suppress(1)
        

    def OnServerInitComplete(self): 
        global scriptInitialized
        global buckets
        global avaIDAtEntry 

        ageSDL = PtGetAgeSDL()

        #Disable the buckets until we are ready to initialize them
        PtDebugPrint("tldnBucketBrain.OnServerInitComplete():\tBucket enter/exit detectors DISABLED",level=kDebugDumpLevel)
        actBktEnter1.disable()
        actBktEnter2.disable()
        actBktExit.disable()
        PtDisableControlKeyEvents(self.key)

        #Init the bucket lever
        leverPulledState = ageSDL[kStringAgeSDLLeverPulled][0]
        if leverPulledState:
            respExtSendLeverPull.run(self.key,fastforward=1)
            PtDebugPrint("tldnBucketBrain.OnServerInitComplete():\tLever starting in the pulled state (on)",level=kDebugDumpLevel)
        else:
            PtDebugPrint("tldnBucketBrain.OnServerInitComplete():\tLever starting in the pushed state (off)",level=kDebugDumpLevel)

        #Setup the buckets
        buckets = (objBucket1.value, objBucket2.value, objBucket3.value, objBucket4.value)

        #In order to initialize everything else, we need to determine the number of players....
        numPlayers = len(PtGetPlayerList())

        if numPlayers == 0:
            # loading cause I just joined age and there's no one else here so...clear out any phantom riders
            PtDebugPrint("tldnBucketBrain.OnServerInitComplete():\tRe-initializing age as I'm the only one here!",level=kDebugDumpLevel)
            ageSDL[kStringAgeSDLRiders] = (-1,-1,-1,-1)
            print "Tye: Resetting avatar at entry!"
            avaIDAtEntry = -1

            #If we're creating a brand new age we need to intialize this variable.  Since it's not done by default,
            #We'll have to do it manually!  In order to find out if it's intialized we need to try and access it.
            #If we get an error, then we need to initialize it; otherwise, we're good to go!
            bucketAtEntry = -1
            try:
                bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
                bucketMode = ageSDL[kStringAgeSDLLoopMode][0]
            except:
                ageSDL[kStringAgeSDLBucketAtEntry] = (0,)
                bucketAtEntry = 0
                bucketMode = 0

            #enable the bucket enter activators
            if bucketAtEntry > -1:
                PtDebugPrint("tldnBucketBrain.OnServerInit():\tEnabling the bucket enter activators!",level=kDebugDumpLevel)
                actBktEnter1.enable()
                actBktEnter2.enable()

            #Since no one was in the age, we'll skip all other states and go to either a running or stopped state
            state = 0
            if ageSDL[kStringAgeSDLPowerOn][0] == 1 and ageSDL[kStringAgeSDLLeverPulled][0] == 1 and bucketMode == 1:
                state = kBucketStates.Run
                ageSDL[kStringAgeSDLLoopMode] = (1,)
                respWRCCLED.run(self.key, kOn, fastforward=1)
            else:
                state = kBucketStates.Stop
                respWRCCLED.run(self.key, kOff, fastforward=1)
            
            #Make sure we update the bucket state!
            ageSDL[kStringAgeSDLBucketState] = (state, 1)  #use FF here because we'll run manually below...
            self.RunBucketState(state, 0)  #Start the buckets if necessary (we do it this just incase there was no update to the SDL)

        else:
            #Tye: Not sure why I said the first line!  PLEASE CHECK!!!
            # loading cause master left and I've been chosen
            # or because I've joined an age with others already playing...
            # assume all bucket riders are valid and let AvatarPage prune em
            
            riders = ageSDL[kStringAgeSDLRiders]
            PtDebugPrint("tldnBucketBrain.OnServerInitComplete()\t Current bucket rider list: ", riders,level=kDebugDumpLevel)
            index = 0
            for rider in riders:
                if rider != -1:
                    avatarKey = PtGetAvatarKeyFromClientID(rider)
                    avatarObj = avatarKey.getSceneObject()
                    PtAttachObject(avatarObj.getKey(), buckets[index].getKey())
                    avatarObj.physics.disable()
                    avatarObj.draw.disable()
                    #avatarObj.warpObj(buckets[index].getKey())
                index = index + 1

            
            #Setup bucket enter activators
            bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]

            #Tye: we can probably simplify the conditional block below (we were verbose to define the structure, and in case we need them later)
            #Now we can setup the activators as necessary....
            if bucketAtEntry  < 0:
                #The bucket is not at the entry point!
                print "Tye: Resetting avatar at entry!"
                avaIDAtEntry = -1

                #Normally we'd disable the enter activators, but for safety's sake, we disabled them earlier
                #There's no need to be redundant....
                pass
            elif riders[bucketAtEntry] < 0:  
                #The bucket is empty and ready to accept a rider.
                avaIDAtEntry = -1
                print "Tye: Resetting avatar at entry!"
                
                #Enable the enter activators
                PtDebugPrint("tldnBucketBrain.OnServerInit():\tEnabling the bucket enter activators!",level=kDebugDumpLevel)
                actBktEnter1.enable()
                actBktEnter2.enable()
                
            else:
                #The bucket contains a rider.
                avaIDAtEntry = riders[bucketAtEntry]

                #Normally we'd disable the enter activators and disable the drawing/physics on the avatar
                #but we did this earlier (activators located in OnFirstUpdate(), avatar stuff located directly above this conditional logic)
                #There's no need to be redundant....
                pass

        
            #Init the bucket state
            state = ageSDL[kStringAgeSDLBucketState][0]
            self.RunBucketState(state, 1)  #initialize to the current state

        #Init the shroom door
        if ageSDL[kStringAgeSDLShrmDoorOpen][0] == 1:
            respDoorOpen.run(self.key,fastforward=1)
        else:
            pass

       

        #Init the SDL callbacks
        if AgeStartedIn == PtGetAgeName():
            #Register SDL variables to send notifies
            ageSDL.setFlags(kStringAgeSDLBucketState,1,1)
            ageSDL.sendToClients(kStringAgeSDLBucketState)
            ageSDL.setNotify(self.key,kStringAgeSDLBucketState,0.0)

            ageSDL.setFlags(kStringAgeSDLLeverPulled,1,1)
            ageSDL.sendToClients(kStringAgeSDLLeverPulled) 
            #ageSDL.setNotify(self.key,kStringAgeSDLLeverPulled,0.0)   #don't need to send out notify as they'll get it as they need it.
            
            ageSDL.setFlags(kStringAgeSDLPowerOn,1,1)
            ageSDL.sendToClients(kStringAgeSDLPowerOn)
            ageSDL.setNotify(self.key,kStringAgeSDLPowerOn,0.0)

            ageSDL.setFlags(kStringAgeSDLRiders,1,1)
            ageSDL.sendToClients(kStringAgeSDLRiders)
            ageSDL.setNotify(self.key,kStringAgeSDLRiders,0.0)
            #Notify Not necessary for kStringAgeSDLRiders

            ageSDL.setFlags(kStringAgeSDLBucketAtEntry, 1,1)
            ageSDL.sendToClients(kStringAgeSDLBucketAtEntry)
            ageSDL.setNotify(self.key,kStringAgeSDLBucketAtEntry,0.0)            
            #Notify Not necessary for kStringAgeSDLBucketAtEntry

            ageSDL.setFlags(kStringAgeSDLBucketAtDump,1,1)
            ageSDL.sendToClients(kStringAgeSDLBucketAtDump)
            ageSDL.setNotify(self.key,kStringAgeSDLBucketAtDump,0.0)

            ageSDL.setFlags(kStringAgeSDLShrmDoorOpen,1,1)
            ageSDL.sendToClients(kStringAgeSDLShrmDoorOpen)
            ageSDL.setNotify(self.key,kStringAgeSDLShrmDoorOpen,0.0)

            ageSDL.setFlags(kStringAgeSDLLoopMode,1,1)
            ageSDL.sendToClients(kStringAgeSDLLoopMode) 
            ageSDL.setNotify(self.key,kStringAgeSDLLoopMode,0.0)
            #Notify not necessary for kStringAgeSDLLoopMode 



        #Let everyone know we're finished intializing
        scriptInitialized = 1
                   
    def AvatarPage(self, avaObj, pageIn, lastOut):
        "prune bucket rider list if a bucket rider quits or crashes"
        global buckets
        global avaIDAtEntry

        if pageIn:
            # I'm not sure why this was added... maybe Doug being over caution
            # commenting out the avatar draw and physics on page in
            #  --- Mark DeForest
            #avObj.draw.enable() #re-enable draw on this avatar, just in case he was linked out while in the bucket and other clients still in Teledahn still think he's invisible
            #avObj.physics.enable()
            return

        #Okay, there's an issue with the logic used here...
        #--->Say we get a page out with many people in the age...
        #--->The ava who links out... no big deal
        #--->The second client processes this function... 
        #--->Okay seems reasonable, but in the process destroys the rider list
        #--->Any clients who get the list late may not prune enable the bucket enter activators properly! 
        #--->This = OUCH!
        #--->
        #--->One solution would have the avatar linking out strip him/herself from the rider list
        #--->Using OnSDLNotify() the rest of the clients can make the necessary changes...
        #--->This seems much more reasonable...  but due to time constraints will need to be done at a later date...
        #  --Tye

        ageSDL = PtGetAgeSDL()

        avaID = PtGetClientIDFromAvatarKey(avaObj.getKey())
        PtDebugPrint("tldnBucketBrain.AvatarPage():\tAvatar %s is trying to page out" % avaID,level=kDebugDumpLevel)


        riders = ageSDL[kStringAgeSDLRiders]
        index = 0
        for rider in riders:
            if rider == avaID:
                avaIDAtEntry = -1
                print "Tye: Resetting avatar at entry!"
                PtDebugPrint("tldnBucketBrain.AvatarPage():\tbucket rider left age, pruning bucket rider list",level=kDebugDumpLevel)
                rider = -1

                if PtGetClientIDFromAvatarKey(PtGetLocalAvatar().getKey()) == avaID:
                    #We're the local avatar, we need to make sure that we enable physics and drawables if we're in a bucket!
                    #Tye: I don't think this is ever called!  We might need to do this elsewhere...
                    #Also the first person cam is not resolved either!
                    avaObj.physics.enable()
                    avaObj.draw.enable()
                
                #Everyone else needs clickables enabled if the bucket is at the entry
                #Make sure to change the bucket state if the rider was in the entry bucket
                bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
                if bucketAtEntry != index:
                    return

                #Now jump to the correct state!
                bucketState = ageSDL[kStringAgeSDLBucketState][0]

                if bucketState in (kBucketStates.Boarded, kBucketStates.QBoard):
                    ageSDL[kStringAgeSDLBucketState] = (kBucketStates.Stop, 1)
                    actBktEnter1.enable()
                    actBktEnter2.enable()
                elif bucketState == kBucketStates.QBoardQRun:
                    ageSDL[kStringAgeSDLBucketState] = (kBucketStates.Run, 0)
                    actBktEnter1.disable()
                    actBktEnter2.disable()
                elif bucketState == kBucketStates.DumpQBoard:
                    ageSDL[kStringAgeSDLBucketState] = (kBucketStates.Dump, 0)
                    actBktEnter1.enable()
                    actBktEnter2.enable()
                elif bucketState == kBucketStates.DumpQBoardQStop:
                    ageSDL[kStringAgeSDLBucketState] = (kBucketStates.DumpQStop, 0)
                    actBktEnter1.enable()
                    actBktEnter2.enable()
                else:  #just disable for all other states for safety's sake...
                    actBktEnter1.disable()
                    actBktEnter2.disable()
                    
                break
            index = index + 1

        #I wish I could get only one person to update this, but unfortunately it's not possible here as 
        #it's possible the person linking out is the owner of the objects in the age (i.e. sceneobject ownership testing)
        riders = None   #Need to release variable otherwise the SDL refuses to update (thus why I've moved it ouside of the 'for' loop)
        ageSDL.setIndex(kStringAgeSDLRiders,index,-1)

        PtDebugPrint("tldnBucketBrain.AvatarPage():\tPruned rider: %s from bucket: %s" %(avaID, index),level=kDebugDumpLevel)
        return       


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global scriptInitialized
        ageSDL = PtGetAgeSDL()

        PtDebugPrint("****> RECEIVED: SDL UPDATE: var= %s  <****" % VARname,level=kDebugDumpLevel)

        #------------------------------------------------
        #  Power On/Off Notification
        #------------------------------------------------
        if VARname == kStringAgeSDLPowerOn:
            powerOn = ageSDL[kStringAgeSDLPowerOn][0]

            if powerOn and ageSDL[kStringAgeSDLLeverPulled][0]:
                PtDebugPrint("tldnBucketBrain.OnSDLNotify():\t The power is turned on and will start the buckets",level=kDebugDumpLevel)
                self.UpdateBucketState(kBucketInputs.Power, ageSDL[kStringAgeSDLPowerOn][0])
            elif not powerOn:
                PtDebugPrint("tldnBucketBrain.OnSDLNotify()\t The power has been turned off",level=kDebugDumpLevel)
                self.UpdateBucketState(kBucketInputs.Power, ageSDL[kStringAgeSDLPowerOn][0])
            else:
                PtDebugPrint("tldnBucketBrain.OnSDLNotify()\t The power has been turned on, but awaiting the lever pull to start buckets",level=kDebugDumpLevel)
            return

        #------------------------------------------------
        # Bucket State Notification
        #------------------------------------------------
        if VARname == kStringAgeSDLBucketState:
            curBucketState = ageSDL[kStringAgeSDLBucketState][0]
            fastforward = ageSDL[kStringAgeSDLBucketState][1]
            PtDebugPrint ("Debug: tldnBucketBrain.OnSDLNotify(): *****>Changed Bucket State to: %s, with FF = %s"% (kBucketStates.ToString(curBucketState),fastforward),level=kDebugDumpLevel)
            self.RunBucketState(curBucketState, fastforward=fastforward)

            #Set WCP bucket LED
            offStates = (kBucketStates.QBoard, kBucketStates.Stop, kBucketStates.QRun, kBucketStates.QBoardQRun, kBucketStates.Boarded)
            if curBucketState in offStates:
                state = kOff
            else:
                state = kOn
            if scriptInitialized:
                respWRCCLED.run(self.key, state)
            else:
                respWRCCLED.run(self.key, state, fastforward=1)
            return

               
                        
    def OnNotify(self,state,id,events):
        global buckets
        global avaIDAtEntry
        global avaIDAtDump
        global scriptInitialzied

        ageSDL = PtGetAgeSDL()

        if not state:
            return

        avatar = PtFindAvatar(events)


        #-----------------------------------#
        #         Camera Handling           #
        #-----------------------------------#
        if id==rgnSensor01.id or id==rgnSensor02.id or id == rgnSensor03.id:
            for event in events:
                if event[0] == 1:
                    objTrigger = event[2]
                                  
                if event[1] == 1: # a bucket entered rngBucketSnsEntryPoint
                    riders = ageSDL[kStringAgeSDLRiders]
                    avaID = PtGetClientIDFromAvatarKey(PtGetLocalAvatar().getKey())
                    index = 0
                    for bucket in buckets:
                        if bucket == objTrigger and riders[index] == avaID:  #if local avatar is in the bucket
                            if id == rgnSensor01.id:
                                cam = ptCamera()
                                cam.save(Camera01.sceneobject.getKey())
                                return
                            elif id == rgnSensor02.id:
                                cam = ptCamera()
                                cam.save(Camera02.sceneobject.getKey())
                                return
                            elif id == rgnSensor03.id:
                                cam = ptCamera()
                                cam.save(Camera03.sceneobject.getKey())
                                return
                        index = index + 1
                else:
                    PtDebugPrint("ERROR: tldnBucketBrain.OnNotify():\t Encountered unknown camera event.",level=kErrorLevel)
                    return
            PtDebugPrint("DEBUG: tldnBucketBrain.OnNotify():\t No local avatars in the bucket that triggered the camera change.",level=kDebugDumpLevel)
            return


        #-----------------------------------#
        #       Buckets Start/Stop          #
        #-----------------------------------#
        #This is the work room control panel (or center) bucket mode switch, we need to run the animation...
        if id == actWRCCGo.id:
            respWRCCGo.run(self.key, events=events)
            return



        #This is the callback from the workroom control panel bucket mode switch...
        #Start the buckets NOW!
        if id == respWRCCGo.id:
            PtDebugPrint("tldnBucketBrain.OnNotify():\tGot callback from workroom control panel bucket mode switch!  Trying to start buckets!",level=kDebugDumpLevel)
            #Don't process unless we are the age owner!
            if not SensorAtEntry.sceneobject.isLocallyOwned():
                return
            
            if ageSDL[kStringAgeSDLPowerOn][0] == 0:
                PtDebugPrint("tldnBucketBrain.OnNotify():\tCannot start the buckets from the workroom when the power is off!",level=kDebugDumpLevel)
                return

            #This switch doesn't actually fit into this state machine model very well
            #We'll provide our own state switching below...
            self.UpdateBucketState(kBucketInputs.WCPswitch)

            return


        #lever pull/push activator
        if id == actGo1.id:
            PtDebugPrint("tldnBucketBrain.OnNotify():\tEncountered the lever pull/push",level=kDebugDumpLevel)
            
            leverPulled = ageSDL[kStringAgeSDLLeverPulled][0]
            if leverPulled:
                respExtSendLeverPush.run(self.key,events=events, avatar=avatar)
            else:
                respExtSendLeverPull.run(self.key,events=events, avatar=avatar)
            return


        #This is a callback from the bucket lever responder
        if id == respExtSendLeverPull.id or id == respExtSendLeverPush.id:
            #Set the new lever value (the avatar who owns the age)
            if not SensorAtEntry.sceneobject.isLocallyOwned():
                return
            
            #if SensorAtEntry.sceneobject.isLocallyOwned():
            leverPulled = ageSDL[kStringAgeSDLLeverPulled][0]
            ageSDL[kStringAgeSDLLeverPulled] = (abs(leverPulled - 1),)
            
            #Stop/Run the buckets
            if id == respExtSendLeverPull.id: #Start buckets if power is on
                if ageSDL[kStringAgeSDLPowerOn][0]:
                    #Set the Workroom control panel to continuous mode!
                    if ageSDL[kStringAgeSDLLoopMode][0] == 0:
                        ageSDL[kStringAgeSDLLoopMode] = (1,)
                    
                    self.UpdateBucketState(kBucketInputs.Power, 1)


            elif id == respExtSendLeverPush.id: #Stop buckets
                #Preferably this would be in RunBucketState(), 
                #but this must happen even if we get a Fastforward
                #So it's here out of convenience
                respCountdownStop.run(self.key)  #Stop the countdown timer (if started)!
                self.UpdateBucketState(kBucketInputs.Power, 0)                                 
            return

        #This is the callback from the bucket Run Timer
        if id == actTimerBktDelay.id:
            #Only process if we own the age
            if not SensorAtEntry.sceneobject.isLocallyOwned():
                return
            
            #This callback comes in even if we stop the timer so we need to ensure that
            #we're actually wanting to start the buckets.
            if not ageSDL[kStringAgeSDLLeverPulled][0]:
                PtDebugPrint("tldnBucketBrain.OnNotify():\tSomeone pushed the lever back, ignoring Run Timer CB.",level=kDebugDumpLevel)
                return

            self.UpdateBucketState(kBucketInputs.Timer)
            return

        #This is the callback from the bucket Stop Timer
        if id == actTimerPwrOff.id:
            #Make sure that if someone pulled the lever again, then ignore the CallBack
            if not SensorAtEntry.sceneobject.isLocallyOwned():
                return

            #This callback comes in even if we stop the timer so we need to ensure that
            #we're actually wanting to stop the buckets.
            if ageSDL[kStringAgeSDLLeverPulled][0] and ageSDL[kStringAgeSDLPowerOn][0]:
                PtDebugPrint("tldnBucketBrain.OnNotify():\tPower is off or level is pulled, ignoring Stop Timer CB.",level=kDebugDumpLevel)
                return

            self.UpdateBucketState(kBucketInputs.StopCB)
            return


        #-----------------------------------#
        #      Ava enter/exit buckets       #
        #-----------------------------------#
        #Bucket reached entry point turn on/off enter/exit functionality
        if id==actEntryPoint.id:
            #We can get notifies before the script has finished initializing.
            #Thus if we haven't finished initializing, then disregard any notifications
            #Hopefully we don't throw away any useful data!!!
            if not scriptInitialized:
                return

            for event in events:
                #We only want to process collision events!
                if event[0] != PtEventType.kCollision:
                    continue

                #Get the bucketAtEntry
                bucketAtEntry = self.FindBucketIndex(event[2])

                #Tye: I think it might be appropriate to not allow changes to the activators if the buckets are running!
                #Although I haven't had time to explore the effects of this reasoning......
                #For now I've commented out the next few lines!

                #Disable the entrances if the buckets exited the sensor
                #state = ageSDL[kStringAgeSDLBucketState][0]
                #running = (state == kBucketStates.Run) or (state == kBucketStates.QStop)
                

                if event[1] == 0: # a bucket exited rngBucketSnsEntryPoint
                    bucketAtEntry = -1
                    actBktEnter1.disable()
                    actBktEnter2.disable()
                    if avaIDAtEntry > 0:
                        avaIDAtEntry = -1
                elif event[1] == 1: # a bucket entered rngBucketSnsEntryPoint
                    if bucketAtEntry < 0:
                        PtDebugPrint("ERROR: tldnBucketBrain.OnNotify():\tCannot enable a bucket that is not at the entry point!",level=kErrorLevel)
                    elif ageSDL[kStringAgeSDLRiders][bucketAtEntry] < 1:  
                        PtDebugPrint("tldnBucketBrain.OnNotify():\tEncountered a bucket entering the loading point, the index is: ", bucketAtEntry,level=kDebugDumpLevel)
                        #bucket@entry is vacant...
                        #We need to delay the activators until the bucket is at a complete stop
                        #So handle this in the timer.
                        PtAtTimeCallback(self.key,3,1) 
                
                #Save the bucketAtEntry
                #we could save it earlier, but it may be updated if the bucket leaves the collision sensor                
                #Only one person can update the SDL
                if not SensorAtEntry.sceneobject.isLocallyOwned():
                    return
                ageSDL[kStringAgeSDLBucketAtEntry] = (bucketAtEntry,)                    
            return

        #An Ava entered a bucket
        if id==actBktEnter1.id or id==actBktEnter2.id:
            #To ensure that a timing issue doesn't occur, make sure to check the bucket State!
            bucketState = ageSDL[kStringAgeSDLBucketState][0]
            boardStates = [kBucketStates.Stop, kBucketStates.QRun, kBucketStates.Dump, kBucketStates.DumpQStop]
            if not(bucketState in boardStates):
                PtDebugPrint("DEBUG: tldnBucketBrain.OnNotify():\tCannot enter bucket during an improper state: %s" %kBucketStates.ToString(bucketState))
                actBktEnter1.disable()
                actBktEnter2.disable()
                return

            #Can't enter an occupied bucket!
            if self.RiderInDockedBucket():
                PtDebugPrint("DEBUG: tldnBucketBrain.OnNotify()\tCannot enter bucket while it is occupied",level=kDebugDumpLevel)
                return

            #Setup avatar for callbacks
            avaIDAtEntry = PtGetClientIDFromAvatarKey(avatar.getKey())

            if ageSDL[kStringAgeSDLBucketState][0] == kBucketStates.Dump and avaIDAtDump == avaIDAtEntry:
                PtDebugPrint("ERROR: tldnBucketBrain.OnNotify():\tPHYSICS ERROR--->the avatar being dumped cannot enter a bucket!!!",level=kErrorLevel)
                return


            #Disable all enter/exit activators until the avatar finishes the behavior
            actBktEnter1.disable()
            actBktEnter2.disable()
            actBktExit.disable()
            
            #We only want one client updating the state...
            if avatar.getKey() == PtGetLocalAvatar().getKey():
                #Put the avatar in the bucket Structure!
                bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
                ageSDL.setIndexNow(kStringAgeSDLRiders, bucketAtEntry, avaIDAtEntry)

                #Update the bucket State...
                self.UpdateBucketState(kBucketInputs.Board, val=1)
                                      
            return
    
        #Received callback from bucket enter responder
        if id==respBktEnter.id:
            #Make sure to disable physics and drawing for the avatar...
            bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]

            if bucketAtEntry < 0:
                return

            avaID = ageSDL[kStringAgeSDLRiders][bucketAtEntry]
            avaIDAtEntry = avaID
            avaKey = PtGetAvatarKeyFromClientID(avaID)
            avaObj = avaKey.getSceneObject()
            avaObj.physics.disable()
            avaObj.draw.disable()

            #From here on out we want only one person updating and we'll make it the person who got in the bucket!
            if avaKey != PtGetLocalAvatar().getKey():
                return

            actBktExit.enable()
            self.UpdateBucketState(kBucketInputs.BoardCB, val=1)
            return

        #Received bucket exit request
        if id==actBktExit.id:
            #To ensure that a timing issue doesn't occur, make sure to check the bucket State!
            bucketState = ageSDL[kStringAgeSDLBucketState][0]
            boardedStates = [kBucketStates.Boarded, kBucketStates.Dump, kBucketStates.QRun]  #Note: QRun requires a person to be in the bucket!
            if not(bucketState in boardedStates):
                avaObj = self.getAvatarFromBucketAtEntry()
                #This check is to ensure that QRun may be a vaild state if a rider is in the bucket
                #(used or logic due to simplification)
                if avaObj == None or bucketState != kBucketStates.QRun:
                    PtDebugPrint("DEBUG: tldnBucketBrain.OnNotify():\tCannot exit a bucket during an improper state: %s" %kBucketStates.ToString(bucketState))
                    actBktExit.disable()
                    return

            #We Only want the avatar who is exiting to update the bucket state
            if avatar.getKey() == PtGetLocalAvatar().getKey():
                bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
                ageSDL.setIndex(kStringAgeSDLRiders, bucketAtEntry, -1)
                self.UpdateBucketState(kBucketInputs.Board, val=0)
            return

        #Received callback from the bucket exit responder
        if id==respBktExit.id:
            #This error checking may not be necessary-->Why would a responder callback if a bucket was never at the entry point???
            bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
            if bucketAtEntry < 0:
                PtDebugPrint("ERROR: tldnBucketBrain.OnNotify():\tAn avatar is trying to exit bucket #%s that is not at the entry point: " %bucketAtEntry,level=kErrorLevel)
                return
            
            avaID = avaIDAtEntry
            avaKey = PtGetAvatarKeyFromClientID(avaID)
            avaObj = avaKey.getSceneObject()

            #Enable physics on the avatar...
            avaObj.physics.enable(1)
                        
            #Tye: this is where we need to delete the rider from the cache...
            avaIDAtEntry  = -1

            actBktEnter1.enable()
            actBktEnter2.enable()
            actBktExit.disable()

            #only the action performing avatar can complete this!
            if avaKey == PtGetLocalAvatar().getKey():
                #enable first person cam if necessary
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                PtEnableMouseMovement()
                PtDisableControlKeyEvents(self.key)
                #update the bucket State
                self.UpdateBucketState(kBucketInputs.BoardCB, val=0)
            return



        #-----------------------------------#
        #           Bucket Dump             #
        #-----------------------------------#
        #We could actually perform all actions for the bucket dumping here in the OnNotify() functions as each is locally notified
        #But to stay consistent, I'm following the state machine model used above in an active way (i.e. non-passive).

        #Start bucket dump animation
        if id==actDumpAvatar.id:
            #Get the Bucket that reached the dump point
            bucketAtDump = None

            for event in events:
                if event[0] == 1:
                    #Get the bucketAtDump
                    bucketAtDump = self.FindBucketIndex(event[2])
                    break

            if bucketAtDump == None or bucketAtDump < 0:
                #Tye: Print some error message...
                return
            
            ava = ageSDL[kStringAgeSDLRiders][bucketAtDump]
            if ava < 0:
                #Tye: Print some log message...
                return
            
            avaIDAtDump = ava

            if not SensorAtEntry.sceneobject.isLocallyOwned():
                return
            
            PtDebugPrint("tldnBucketBrain.OnNotify():\tBucket #%s reached the bucket dump dump point" % bucketAtDump,level=kDebugDumpLevel)
            ageSDL[kStringAgeSDLBucketAtDump] = (bucketAtDump,)
            self.UpdateBucketState(kBucketInputs.Dump)


        #Start the avatar and spores falling animations
        if id==respBktOpen01.id or id==respBktOpen02.id or id==respBktOpen03.id or id==respBktOpen04.id:
            bucketAtDump = ageSDL[kStringAgeSDLBucketAtDump][0]

            #Get Avatar!
            try:
                avaKey = PtGetAvatarKeyFromClientID(avaIDAtDump)
                avaObj = avaKey.getSceneObject()
            except:
                PtDebugPrint("ERROR: tldnBucketBrain.OnNotify():\trespBktOpen-->There is no avatar in the bucket to dump!!",level=kErrorLevel)

                #We only want the age owner to remove the non-existant rider from the bucket!!!
                if SensorAtEntry.sceneobject.isLocallyOwned():
                    ageSDL.setIndex(kStringAgeSDLRiders, bucketAtDump, -1)
                return

            #Draw the avatar and detach the avatar from the bucket
            avaObj.draw.enable()
            bucket = buckets[bucketAtDump]
            PtDetachObject(avaObj.getKey(), bucket.getKey())

            #Run Responders
            myEvents=[[1,1,avaObj, bucket]]
            respDumpAvatar.run(self.key,events=myEvents)                                             
            respSpores.run(self.key)
            return                  


        #Finally, this is the DumpCB...
        if id==respDumpAvatar.id:
            if avaIDAtDump < 0:
                PtDebugPrint("ERROR: tldnBucketBrain.OnNotify():\trespDumpAvatar-->Avatar at dump point is less than zero!",level=kErrorLevel)
                return

            #Tye: note this is something that can be refactored....
            avaKey = PtGetAvatarKeyFromClientID(avaIDAtDump)
            avaObj = avaKey.getSceneObject()
            if type(avaObj) == type(None):
                return

            avaObj.physics.enable(1)

            #If we're the local avatar, make sure to setup the camera and allow mouse movements
            if avaKey == PtGetLocalAvatar().getKey():
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                PtEnableMouseMovement()
                PtDisableControlKeyEvents(self.key)

            #We're done with the avaIDAtDump so reset it for safety's sake...
            avaIDAtDump = -1


            #We only want one person propigating the callback and other necessary SDLs
            if not SensorAtEntry.sceneobject.isLocallyOwned():
                return
            
            bucketAtDump = ageSDL[kStringAgeSDLBucketAtDump][0]
            ageSDL.setIndexNow(kStringAgeSDLRiders, bucketAtDump, -1)
            ageSDL[kStringAgeSDLBucketAtDump] = (-1,)
            self.UpdateBucketState(kBucketInputs.DumpCB)
            PtDebugPrint("tldnBucketBrain.OnNotify():\tRemoved rider from bucket #%d" % bucketAtDump,level=kDebugDumpLevel)



        #-----------------------------------#
        #          Workroom Door            #
        #-----------------------------------#
        shrmDoorOpen = ageSDL[kStringAgeSDLShrmDoorOpen][0]
        
        if id==actDoorOpen.id and shrmDoorOpen == 0: 
            respDoorOpen.run(self.key)
            ageSDL[kStringAgeSDLShrmDoorOpen] = (1,)
            return


        if id==actDoorClose.id and shrmDoorOpen == 1:
            for event in events:
                if event[0] == 1:
                    objBucketAtDoor = event[2]

            bucketAtDump = self.FindBucketIndex(objBucketAtDoor)

            if bucketAtDump < 0:
                #Tye: we need some error output here!
                return

            if bucketAtDump != ageSDL[kStringAgeSDLBucketAtDump] and SensorAtEntry.sceneobject.isLocallyOwned():
                #This should never happen, but just in case, we'll add it in....
                ageSDL[kStringAgeSDLBucketAtDump] = (bucketAtDump,)

            
            rider = ageSDL[kStringAgeSDLRiders][bucketAtDump]
            if rider > 0:
                #Run bucket Dump responder....
                respDumpCam.run(self.key)
            else:
                if ageSDL[kStringAgeSDLLoopMode][0] == 1:
                    return
                else:
                    #This is a hack to make sure that the buckets will start again if stopped with the looping mode switch in the workroom
                    if SensorAtEntry.sceneobject.isLocallyOwned(): 
                        ageSDL[kStringAgeSDLBucketState] = (kBucketStates.Stop, 0)

            respDoorClose.run(self.key)
            respStop.run(self.key)

            #Only one person can update the SDL
            if not SensorAtEntry.sceneobject.isLocallyOwned():
                return
            
            ageSDL[kStringAgeSDLShrmDoorOpen] = (0,)
            return


 
    def OnTimer(self,timer):
        if timer == 1:
            #We're trying to enable the bucket get in activators, but since we awaited the stop, something could have changed
            #Make sure the buckets are really stopping.
            ageSDL = PtGetAgeSDL()
            state = ageSDL[kStringAgeSDLBucketState][0] 
            stopped = state != kBucketStates.Run and state != kBucketStates.QStop
            if stopped == 1:  #The buckets stopped
                actBktEnter1.enable()
                actBktEnter2.enable()

            else: #re-starting the buckets
                actBktEnter1.disable()
                actBktEnter2.disable()

    def BeginAgeUnLoad(self,foo):
        PtDisableControlKeyEvents(self.key)       

    def OnControlKeyEvent(self,controlKey,activeFlag):
        ageSDL = PtGetAgeSDL()
        
        bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
        ava = PtGetLocalAvatar()
        avaID = PtGetClientIDFromAvatarKey(ava.getKey())

        if bucketAtEntry < 0 or avaID != ageSDL[kStringAgeSDLRiders][bucketAtEntry]:
            PtDebugPrint("tldnBucketBrain.OnControlKeyEvent():\tIgnoring exit request as the bucket is not at the entry point",level=kDebugDumpLevel)
            return

        curBucketState = ageSDL[kStringAgeSDLBucketState][0]

        if curBucketState != kBucketStates.Boarded:
            PtDebugPrint("tldnBucketBrain.OnControlKeyEvent()\tCannot exit buckets while they are not in the Boarded State",level=kDebugDumpLevel)
            return

        if controlKey == PlasmaControlKeys.kKeyExitMode or controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight or controlKey == PlasmaControlKeys.kKeyMoveForward:
            PtDisableControlKeyEvents(self.key)                    
            
            #Since OnControlKeyEvents won't be received by the clients who didn't push ESC, send a notify so all client immediately draw.enable the exiting avatar

            #Tye: May want to do some error checking here???
            ageSDL.setIndex(kStringAgeSDLRiders, bucketAtEntry, -1)                
            self.UpdateBucketState(kBucketInputs.Board, val=0)
            
            #Tye: LOOK AT THIS, SHOULD it be here?!?"
            note = ptNotify(self.key)
            note.clearReceivers()
            note.addReceiver(self.key)
            note.setActivate(1)
            note.addVarNumber("AvatarExitingBucket",avaID)
            note.send()
            

    def UpdateBucketState(self, param, val=0):
        "This is the TRUE bucket brain (i.e. state machine)"
        global buckets

        #Get the current bucke state...
        ageSDL = PtGetAgeSDL()
        curBucketState = ageSDL[kStringAgeSDLBucketState][0]

        fastforward = 0  #fastforward to state       
        error = 0  #error reporting

        PtDebugPrint("tldnBucketBrain.UpdateBucketState():-->Incomming state change request: state (%s), parameter (%s), value (%s)" % (kBucketStates.ToString(curBucketState), kBucketInputs.ToString(param),val),level=kDebugDumpLevel)


        ############[ Power ]#############
        if param == kBucketInputs.Power:
            #--------------[ Power-->QStop ]---------------
            if curBucketState == kBucketStates.QStop and val == 1:
                curBucketState = kBucketStates.Run
                fastforward = 1
            #--------------[ Power-->Stop ]---------------
            elif curBucketState ==  kBucketStates.Stop and val == 1:
                curBucketState = kBucketStates.QRun
            #--------------[ Power-->QRun ]---------------
            elif curBucketState == kBucketStates.QRun and val == 0:
                if self.RiderInDockedBucket() == 1:
                    curBucketState = kBucketStates.Boarded
                else:
                    curBucketState = kBucketStates.Stop
                fastforward = 1
            #--------------[ Power-->Run ]---------------
            elif curBucketState == kBucketStates.Run and val == 0:
                curBucketState = kBucketStates.QStop
            #--------------[ Power-->QBoard ]---------------
            elif curBucketState == kBucketStates.QBoard and val == 1:
                curBucketState = kBucketStates.QBoardQRun
                fastforward = 1
            #--------------[ Power-->QBoardQRun ]---------------
            elif curBucketState == kBucketStates.QBoardQRun and val == 0:
                curBucketState = kBucketStates.QBoard
                fastforward = 1
            #--------------[ Power-->Boarded ]---------------
            elif curBucketState == kBucketStates.Boarded and val == 1:
                curBucketState = kBucketStates.QRun
            #--------------[ Power-->Dump ]---------------
            elif curBucketState == kBucketStates.Dump and val == 0:
                curBucketState = kBucketStates.DumpQStop
            #--------------[ Power-->DumpQStop ]---------------
            elif curBucketState == kBucketStates.DumpQStop and val == 1:
                curBucketState = kBucketStates.Dump
                fastforward = 1
            #--------------[ Power-->DumpQBoard ]---------------
            elif curBucketState == kBucketStates.DumpQBoard and val == 0:
                curBucketState = kBucketStates.DumpQBoardQStop
                fastforward = 1
            #--------------[ Power-->DumpQBoardQStop ]---------------
            elif curBucketState == kBucketStates.DumpQBoardQStop and val == 1:
                curBucketState = kBucketStates.DumpQBoard
                fastforward = 1
            else:
                error = 1

        ############[ Timer ]#############                
        elif param == kBucketInputs.Timer:
            if curBucketState == kBucketStates.QRun:
                curBucketState = kBucketStates.Run
            else:
                error = 1

        ############[ StopCB ]#############
        elif param == kBucketInputs.StopCB:
            if curBucketState == kBucketStates.QStop:
                curBucketState = kBucketStates.Stop
            else:
                error = 1

        ############[ Board ]#############
        elif param == kBucketInputs.Board:
            #--------------[ Board-->QStop ]---------------
            if curBucketState == kBucketStates.Stop:
                curBucketState = kBucketStates.QBoard
            #--------------[ Board-->Boarded ]---------------
            elif curBucketState == kBucketStates.Boarded:
                curBucketState = kBucketStates.QBoard
            #--------------[ Board-->QRun ]---------------                
            elif curBucketState == kBucketStates.QRun:
                curBucketState = kBucketStates.QBoardQRun
            #--------------[ Board-->Dump ]---------------                
            elif curBucketState == kBucketStates.Dump:
                curBucketState = kBucketStates.DumpQBoard
            #--------------[ Board-->DumpQStop ]---------------
            elif curBucketState == kBucketStates.DumpQStop:
                curBucketState = kBucketStates.DumpQBoardQStop
            else:
                error = 1


        
        ############[ BoardCB ]#############
        elif param == kBucketInputs.BoardCB:
            riderInBucket = self.RiderInDockedBucket()  #Tye might not be necessary to put this here...

            #--------------[ BoardCB-->QBoard ]---------------
            if curBucketState == kBucketStates.QBoard:
                if riderInBucket == 1:
                    curBucketState = kBucketStates.Boarded
                else:
                    curBucketState = kBucketStates.Stop
            #--------------[ BoardCB-->QBoardQRun ]---------------
            elif curBucketState == kBucketStates.QBoardQRun:
                curBucketState = kBucketStates.Run
            #--------------[ BoardCB-->DumpQBoard ]---------------
            elif curBucketState == kBucketStates.DumpQBoard:
                curBucketState = kBucketStates.Dump
                fastforward = 1
            #--------------[ BoardCB-->DumpQBoardQStop ]---------------
            elif curBucketState == kBucketStates.DumpQBoardQStop:
                curBucketState = kBucketStates.DumpQStop
                fastforward = 1
            else:
                error = 1
                


        ############[ Dump ]#############
        elif param == kBucketInputs.Dump:
            #--------------[ Dump-->Run ]---------------
            if curBucketState == kBucketStates.Run:
                curBucketState = kBucketStates.Dump
            elif curBucketState == kBucketStates.Stop:
                curBucketState = kBucketStates.DumpQStop
            else:
                error = 1


        ############[ DumpCB ]#############
        elif param == kBucketInputs.DumpCB:
            #Get Loop mode....
            loopMode = ageSDL[kStringAgeSDLLoopMode][0]
            #--------------[ DumpCB-->Dump ]---------------
            if curBucketState == kBucketStates.Dump:
                if loopMode == 1:
                    curBucketState = kBucketStates.Run
                else:
                    fastforward = 1
                    if self.RiderInDockedBucket():
                        curBucketState = kBucketStates.Boarded
                    else:
                        curBucketState = kBucketStates.Stop
            #--------------[ DumpCB-->DumpQStop ]---------------
            elif curBucketState == kBucketStates.DumpQStop:
                fastforward = 1
                if self.RiderInDockedBucket():
                    curBucketState = kBucketStates.Boarded
                else:
                    curBucketState = kBucketStates.Stop
            #--------------[ DumpCB-->DumpQBoard ]---------------
            elif curBucketState == kBucketStates.DumpQBoard:
                fastforward = 1
                if loopMode == 1:
                    curBucketState = kBucketStates.QBoardQRun
                else:
                    curBucketState = kBucketStates.QBoard
            #--------------[ DumpCB-->DumpQBoardQStop ]---------------
            elif curBucketState == kBucketStates.DumpQBoardQStop:
                curBucketState = kBucketStates.QBoard
            else:
                error = 1

        ############[ WCPswitch ]#############
        elif param == kBucketInputs.WCPswitch:
            instantStartStates = (kBucketStates.Stop, kBucketStates.Boarded, kBucketStates.QRun, kBucketStates.QStop)
            #--------------[ WCPswitch-->instantStartStates ]---------------
            if curBucketState in instantStartStates:
                curBucketState = kBucketStates.Run
            #--------------[ WCPswitch-->QBoard ]---------------
            elif curBucketState == kBucketStates.QBoard:
                curBucketState = kBucketStates.QBoardQRun
                fastforward = 1
            #--------------[ WCPswitch-->DumpQBoardQStop ]---------------
            elif curBucketState == kBucketStates.DumpQBoardQStop:
                curBucketState = kBucektStates.DumpQBoard
                fastforward = 1
            #--------------[ WCPswitch-->DumpQStop ]---------------    
            elif curBucketState == kBucketStates.DumpQStop:
                curBucketState = kBucketStates.Dump
                fastforward = 1
            else:
                error = 1



        ############[ UNRECOGNIZED INPUT ]#############    
        else:
            #Need to cut out here; if we try to enter error reporting below, then we'll crash the script!
            PtDebugPrint("ERROR: tldnBucketBrain.UpdateBucketState():-->Unknown input: %s" % param,level=kErrorLevel)
            return

        #Propigate to all clients if an error did not occur
        #Tye: we could just prop to all clients irregardless of an error?!???
        if error:
            PtDebugPrint("ERROR: tldnBucketBrain.UpdateBucketState():-->Unknown state (%s), parameter (%s), and value (%s) combo" % (kBucketStates.ToString(curBucketState), kBucketInputs.ToString(param),val),level=kErrorLevel)
        else:
            #Push state to other clients..
            PtDebugPrint("tldnBucketBrain.UpdateBucketState(): ----[ Updating EVERYONE\'s State to: %s ]----" % (kBucketStates.ToString(curBucketState)),level=kDebugDumpLevel)
            ageSDL[kStringAgeSDLBucketState] = (curBucketState, fastforward)




    def RunBucketState(self, curBucketState, fastforward = 0, param = 0):
        "Actually initiates the actions associated with a particular state"
        global buckets
        global avaIDAtEntry 

        if fastforward:
            return
        
        ageSDL = PtGetAgeSDL()
        #************************************
        #************[ QStop ]***************
        #************************************
        if curBucketState == kBucketStates.QStop:
            #Stop buckets
            respCountdownStop.run(self.key)
            respTimerPwrOffRun.run(self.key)
        #************************************
        #************[ QRun ]****************
        #************************************
        elif curBucketState == kBucketStates.QRun:
            #Start buckets
            respTimerBktDelayRun.run(self.key)
            respCountdownStart.run(self.key)
        #************************************
        #************[ Run ]*****************
        #************************************
        elif curBucketState == kBucketStates.Run:
            respGo.run(self.key)
        #************************************
        #************[ Stop ]****************
        #************************************
        elif curBucketState == kBucketStates.Stop:
            respStop.run(self.key)
        #************************************            
        #************[ QBoard ]**************
        #************************************
        elif curBucketState == kBucketStates.QBoard:
            self.EnterExitBucket()
        #***************************************
        #************[ QBoardQRun ]*************
        #***************************************
        elif curBucketState == kBucketStates.QBoardQRun:
            self.EnterExitBucket()

        #************************************
        #************[ Boarded ]*************
        #************************************        
        elif curBucketState == kBucketStates.Boarded:
            #Get the avatar that triggered the Boarded state
            bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
            if bucketAtEntry < 0:
                #Tye: Error message would be appropriate here!
                return

            avaID = ageSDL[kStringAgeSDLRiders][bucketAtEntry]
            if avaID < 1:
                #Tye: Error message would be appropriate here!
                return

            avaIDAtEntry = avaID   #This is a cheap hack to setup the avatar for exiting the bucket

            avaKey = PtGetAvatarKeyFromClientID(avaID)
            avaObj = avaKey.getSceneObject()

            #Setup bucket exit for the local avatar
            if PtGetLocalAvatar().getKey() == avaKey:
                PtEnableControlKeyEvents(self.key)
                actBktExit.enable()

            #Tye: these are redundant (others located in OnNotify() from the get in activators)!
            #Not sure why we need them...
            PtAttachObject(avaKey, buckets[bucketAtEntry].getKey())
            avaObj.physics.disable()
            avaObj.draw.disable()

        #************************************
        #************[ Dump ]****************
        #************************************
        elif curBucketState == kBucketStates.Dump:
            bucketAtDump = ageSDL[kStringAgeSDLBucketAtDump][0]
            avaID = ageSDL[kStringAgeSDLRiders][bucketAtDump]

            #Run the responder....
            respList = (respBktOpen01, respBktOpen02, respBktOpen03, respBktOpen04)
            respList[bucketAtDump].run(self.key)

        #**************************************
        #************[ DumpQStop ]*************
        #**************************************
        elif curBucketState == kBucketStates.DumpQStop:
            #Dump the bucket!  
            #the only time we should enter here is from a stop state (i.e. the buckets are stopping with a rider at the top)
            bucketAtDump = ageSDL[kStringAgeSDLBucketAtDump][0]
            avaID = ageSDL[kStringAgeSDLRiders][bucketAtDump]

            #Run the responder....
            respList = (respBktOpen01, respBktOpen02, respBktOpen03, respBktOpen04)
            respList[bucketAtDump].run(self.key)
        #**************************************
        #************[ DumpQBoard ]************
        #**************************************
        elif curBucketState == kBucketStates.DumpQBoard:
            #Need to board the bucket
            self.EnterExitBucket()
            pass
        #**************************************
        #************[ DumpQBoardQStop ]*******
        #**************************************
        elif curBucketState == kBucketStates.DumpQBoardQStop:
            self.EnterExitBucket()
            pass
        else:
            PtDebugPrint("ERROR: tldnBucketBrain.RealizeBucketState():-->Received Unknown State: %s" % (curBucketState),level=kErrorLevel)

    def EnterExitBucket(self):
        "The action of an avatar entering/exiting a docked bucket"
        global buckets
        global avaIDAtEntry 

        ageSDL = PtGetAgeSDL()
        bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
        if bucketAtEntry < 0:
            #Tye Write debug statement here!
            return

        avaID = ageSDL[kStringAgeSDLRiders][bucketAtEntry]
        
        if avaID < 0:
            #Somone is trying to exit the bucket...
            #This is counter-intuitive as we set the data structure before performing the action
            #Thus if a avatar is not in the data structure, we need to remove him/her
            PtDebugPrint("tldnBucketBrain.EnterExitBucket():\tAvatar Exiting bucket---Running AvatarExitBucket()!",level=kDebugDumpLevel)
    
            #Make sure that we're not running while trying to exit the buckets...
            state = ageSDL[kStringAgeSDLBucketState][0] 
            stopped = state != kBucketStates.Run and state != kBucketStates.QStop
            if not stopped:
                return

            #Get avatar object, remember the avatar exiting is stored in the cache variable called avaIDAtEntry
            avaID = avaIDAtEntry
            try:
                avaKey = PtGetAvatarKeyFromClientID(avaID)
                avaObj = avaKey.getSceneObject()
            except:
                PtDebugPrint("ERROR: tldnBucketBrain.EnterExitBucket():\tCouldn't find the avatar exiting the bucket!!!",level=kErrorLevel)
                return
                  
            avaObj.draw.enable()
            PtDetachObject(avaKey, buckets[bucketAtEntry].getKey())
            respBktExit.run(self.key,avatar=avaObj)
            #avaObj.physics.enable(1)
            return

        #Get the avatar
        try:
            avaKey = PtGetAvatarKeyFromClientID(avaID)
            avaObj = avaKey.getSceneObject()
        except:
            PtDebugPrint("ERROR: tldnBucketBrain.EnterExitBucket():\tCouldn't find an avatar to add to the bucket!!!",level=kErrorLevel)
            return

        #Storing the avatar in the cache for later use!
        avaIDAtEntry = avaID

        #attach avatar to bucket and run avatar enter bucket responder
        PtAttachObject(avaKey, buckets[bucketAtEntry].getKey())
        avaObj.physics.disable()
        respBktEnter.run(self.key, avatar=avaObj)

        #make sure to set up the camera and mouse movements while climbing in
        if avaKey == PtGetLocalAvatar().getKey():
            cam = ptCamera()
            cam.undoFirstPerson()
            cam.disableFirstPersonOverride()
            PtDisableMouseMovement()



    def RiderInDockedBucket(self):
        "Find out if a rider is in the docked bucket"
        global buckets
                
        ageSDL = PtGetAgeSDL()

        riders = ageSDL[kStringAgeSDLRiders]
        bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
        if bucketAtEntry < 0:
            return 0
        if riders[bucketAtEntry] > 0:
            return 1
        else:
            return 0

        
    def FindBucketIndex(self, objBucket):
        "Finds the index for a given bucket object"
        global buckets

        index = 0
        for bucket in buckets:
            if bucket == objBucket:
                return index
            index = index + 1

        #Didn't find the corresponding bucket
        return -1
        

    def getAvatarFromBucketAtEntry(self):
        "Returns the avatar object of a rider inside the bucket at the entry!"
        ageSDL = PtGetAgeSDL()
        bucketAtEntry = ageSDL[kStringAgeSDLBucketAtEntry][0]
        if bucketAtEntry < 0:
            PtDebugPrint("ERROR: tldnBucketBrain.getAvatarFromBucketAtEntry():\tCannot retrieve the avatar from a bucket that's not at the entry point",level=kErrorLevel)
            return None
        avaID = ageSDL[kStringAgeSDLRiders][bucketAtEntry]
        if avaID < 1:
            PtDebugPrint("ERROR: tldnBucketBrain.getAvatarFromBucketAtEntry():\t Cannot retrieve the avatar from an empty bucket!",level=kErrorLevel)
            return None
        avaKey = PtGetAvatarKeyFromClientID(avaID)
        return avaKey.getSceneObject()



