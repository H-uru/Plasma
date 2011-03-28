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
Module: tldnSlavePrisonDoors
Age: teledahn
Date: June 24, 2003
Author: Mark DeForest
This is the brain for the prison/pressure plate puzzle
"""

from Plasma import *
from PlasmaTypes import *
import string
import xSndLogTracks

# ---------
# max wiring
# ---------

respInnerDoor1 = ptAttribResponder(1,"Prison Inner door 1",statelist=["up","down"])
respInnerDoor2 = ptAttribResponder(2,"Prison Inner door 2",statelist=["up","down"])
respOuterDoor = ptAttribResponder(3,"Prison Outer door",statelist=["up","down"])

actPlate1 = ptAttribActivator(6,"Activator Plate01")
respPlate1 = ptAttribResponder(7,"Responder Plate01",statelist=["State 1","State 2"])
actPlate2 = ptAttribActivator(8,"Activator Plate02")
respPlate2 = ptAttribResponder(9,"Responder Plate02",statelist=["State 1","State 2"])
actPlate3 = ptAttribActivator(10,"Activator Plate03")
respPlate3 = ptAttribResponder(11,"Responder Plate03",statelist=["State 1","State 2"])
actPlate4 = ptAttribActivator(12,"Activator Plate04")
respPlate4 = ptAttribResponder(13,"Responder Plate04",statelist=["State 1","State 2"])
actPlate5 = ptAttribActivator(14,"Activator Plate05")
respPlate5 = ptAttribResponder(15,"Responder Plate05",statelist=["State 1","State 2"])
actPlate6 = ptAttribActivator(16,"Activator Plate06")
respPlate6 = ptAttribResponder(17,"Responder Plate06",statelist=["State 1","State 2"])
actPlate7 = ptAttribActivator(18,"Activator Plate07")
respPlate7 = ptAttribResponder(19,"Responder Plate07",statelist=["State 1","State 2"])

respSecretDoor = ptAttribResponder(20,"resp: secret door",['open','close'])


# -----------
# globals
# -----------

respToPadSDL = {    # Plate responder, panel SDL var
                    'tldnSlaveActivePanel01' : respPlate1,\
                    'tldnSlaveActivePanel02' : respPlate2,\
                    'tldnSlaveActivePanel03' : respPlate3,\
                    'tldnSlaveActivePanel04' : respPlate4,\
                    'tldnSlaveActivePanel05' : respPlate5,\
                    'tldnSlaveActivePanel06' : respPlate6,\
                    'tldnSlaveActivePanel07' : respPlate7,\
                }

activatorToResp = { #     id     : responder name
                    actPlate1.id : respPlate1,\
                    actPlate2.id : respPlate2,\
                    actPlate3.id : respPlate3,\
                    actPlate4.id : respPlate4,\
                    actPlate5.id : respPlate5,\
                    actPlate6.id : respPlate6,\
                    actPlate7.id : respPlate7,\
                }

PlateSDLs = [ 
                ('tldnSlaveActivePlate01',actPlate1.id),\
                ('tldnSlaveActivePlate02',actPlate2.id),\
                ('tldnSlaveActivePlate03',actPlate3.id),\
                ('tldnSlaveActivePlate04',actPlate4.id),\
                ('tldnSlaveActivePlate05',actPlate5.id),\
                ('tldnSlaveActivePlate06',actPlate6.id),\
                ('tldnSlaveActivePlate07',actPlate7.id),\
            ]

activatorToSDL = { #     id      : SDL plate name
                    actPlate1.id : 'tldnSlaveActivePlate01',\
                    actPlate2.id : 'tldnSlaveActivePlate02',\
                    actPlate3.id : 'tldnSlaveActivePlate03',\
                    actPlate4.id : 'tldnSlaveActivePlate04',\
                    actPlate5.id : 'tldnSlaveActivePlate05',\
                    actPlate6.id : 'tldnSlaveActivePlate06',\
                    actPlate7.id : 'tldnSlaveActivePlate07',\
                }

padSDLToplateSDL = { #    paddle SDL name    : plate SDL name
                    'tldnSlaveActivePanel01' : 'tldnSlaveActivePlate01',\
                    'tldnSlaveActivePanel02' : 'tldnSlaveActivePlate02',\
                    'tldnSlaveActivePanel03' : 'tldnSlaveActivePlate03',\
                    'tldnSlaveActivePanel04' : 'tldnSlaveActivePlate04',\
                    'tldnSlaveActivePanel05' : 'tldnSlaveActivePlate05',\
                    'tldnSlaveActivePanel06' : 'tldnSlaveActivePlate06',\
                    'tldnSlaveActivePanel07' : 'tldnSlaveActivePlate07',\
                }

AgeStartedIn = None

class tldnSlavePrisonDoors(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 4000
        self.version = 2
        PtDebugPrint("tldSlavePrisonDoors.__init__: version %d,%d" % (self.version,2),level=kWarningLevel)

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnServerInitComplete(self):
        PtDebugPrint("tldSlavePrisonDoors.OnServerInitComplete:",level=kDebugDumpLevel)
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            # look at the SDL for the plates if we are NOT the first person in the Age
            players = PtGetPlayerList()
            if len(players) > 0:
                PtDebugPrint("   - and the number of players is %d, so get plate states from SDL"%(len(players)),level=kDebugDumpLevel)
                for platesdl,actid in PlateSDLs:
                    boolCurrentValue = ageSDL[platesdl][0]
                    if boolCurrentValue:
                        activatorToResp[actid].run(self.key, state="State 1",fastforward=1)
                    # if its false then there is no need to animate the plate up (should already be in the that state)
            for PadSDL in respToPadSDL.keys():
                ageSDL.setNotify(self.key,PadSDL,0.0)
            ageSDL.setNotify(self.key,"tldnSlaveCaveSecretDoorOpen",0.0)
            # evaluate plates to paddles and set doors accordingly
            self.IEvalPlateAndPaddles(fastforward=1)

    # in case someone other than me changes my var(s)
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        PtDebugPrint("tldSlavePrisonDoors.OnSDLNotify:",level=kDebugDumpLevel)
        # is it a var we care about?
        foundSDL = 0
        for PadSDL in respToPadSDL.keys():
            if VARname == PadSDL:
                foundSDL = 1
                break
        if not foundSDL:
            return
        # evaluate plates to paddles and set doors accordingly
        self.IEvalPlateAndPaddles()

    def OnNotify(self,state,id,events):
        PtDebugPrint("tldSlavePrisonDoors.OnNotify:",level=kDebugDumpLevel)
        # make sure its one of the activators we know about
        if id in activatorToResp:
            for event in events:
                print "event = ", event
            # evaluate plates to paddles and set doors accordingly
            self.IEvalPlateAndPaddles(activator=id)
            ageSDL = PtGetAgeSDL()
            # NOTE:
            # setting the SDL for the plates is ONLY for evaluation at ServerInit and for people who come in second
            #
            for event in events:
                if event[0] == 1 and event[1] == 1:
                    activatorToResp[id].run(self.key, state="State 1")
                    ageSDL[activatorToSDL[id]] = (1,)
                elif event[0] == 1 and event[1] == 0:
                    activatorToResp[id].run(self.key, state="State 2")
                    ageSDL[activatorToSDL[id]] = (0,)
            
        else:
            self.IEvalPlateAndPaddles()
        
        # RAWA's Huevo
        if id == actPlate5.id:
            for event in events:
                if event[0] == 1 and event[1] == 1:
                    avatar = PtFindAvatar(events)
                    if avatar == PtGetLocalAvatar():
                        xSndLogTracks.LogTrack("27","94")
                    

    def IEvalPlateAndPaddles(self,activator=None,fastforward=0):
        if AgeStartedIn != PtGetAgeName():
            return
        ageSDL = PtGetAgeSDL()
        # assume plates and paddles match
        theyMatch = 1
        for PadSDL,resp in respToPadSDL.items():
            try:
                paddle = ageSDL[PadSDL][0]
            except LookupError:
                theyMatch = 0
                PtDebugPrint("tldSlavePrisonDoors.IEvalPlateAndPaddles: error on SDL %s" % (PadSDL),level=kErrorLevel)
                break
            # paddle == 0 is down, paddle == 1 is up
            
            # determine the pressure plate status
            plate = -1      # first assume
            if fastforward:
                # if we are doing this a serverinit time, then look at the SDLs for answer to plate
                try:
                    plate = ageSDL[padSDLToplateSDL[PadSDL]][0]
                    # reverse to match SDL of paddle(and make sure its either 0 or 1
                    if plate == 0:
                        plate = 1
                    else:
                        plate = 0
                except LookupError:
                    theyMatch = 0
                    PtDebugPrint("tldSlavePrisonDoors.IEvalPlateAndPaddles: error on SDL %s" % (padSDLToplateSDL[PadSDL]),level=kErrorLevel)
                    break
            else:
                # otherwise look at the responder states
                if type(resp.value) == type([]) and len(resp.value) > 0:
                    plate = resp.value[0].getSceneObject().getResponderState()
                    # plate == 0 is up (no weight) and plate == 1 is pressed down (has weight)
    
                    # check to see if this is the plate that is activating
                    if activator and activatorToResp[activator] == resp:
                        # this is the plate that is activating, so plates will be next state.. already like SDL
                        # make sure its a 0 or a 1
                        if plate == 0:
                            plate = 0
                        else:
                            plate = 1
                    else:
                        # reverse to match SDL of paddle(and make sure its either 0 or 1
                        if plate == 0:
                            plate = 1
                        else:
                            plate = 0
            if plate == -1:
                PtDebugPrint("tldSlavePrisonDoors.IEvalPlateAndPaddles: can't find responder for SDLvar %s" % (PadSDL),level=kErrorLevel)
                theyMatch = 0
                break
            # are they a mismatch?
            if paddle != plate:
                theyMatch = 0
                break
        if theyMatch:
            # raise the inner doors and shut the outer door
            PtDebugPrint("tldSlavePrisonDoors.IEvalPlateAndPaddles: inner doors up, outer down",level=kDebugDumpLevel)
            if fastforward or respInnerDoor1.getState() != "up":
                respInnerDoor1.run(self.key,state="up",fastforward=fastforward)
            if fastforward or respInnerDoor2.getState() != "up":
                respInnerDoor2.run(self.key,state="up",fastforward=fastforward)
            if fastforward or respOuterDoor.getState() != "down":
                respOuterDoor.run(self.key,state="down",fastforward=fastforward)
        
            ageSDL["tldnSlaveCaveSecretDoorOpen"] = (1,)
            print "tldnSlavePrisonDoors: The exp1 secret panel should now be open."
        
        else:
            # raise the outer doors and shut the inner doors
            PtDebugPrint("tldSlavePrisonDoors.IEvalPlateAndPaddles: inner doors down, outer up",level=kDebugDumpLevel)
            if fastforward or respInnerDoor1.getState() != "down":
                respInnerDoor1.run(self.key,state="down",fastforward=fastforward)
            if fastforward or respInnerDoor2.getState() != "down":
                respInnerDoor2.run(self.key,state="down",fastforward=fastforward)
            if fastforward or respOuterDoor.getState() != "up":
                respOuterDoor.run(self.key,state="up",fastforward=fastforward)

            ageSDL["tldnSlaveCaveSecretDoorOpen"] = (0,)
            print "tldnSlavePrisonDoors: The exp1 secret panel should now be closed."


    def OnBackdoorMsg(self, target, param):
        if target == "spyroombook":
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("tldnSlaveCaveSecretDoorVis",1,1)
            ageSDL.sendToClients("tldnSlaveCaveSecretDoorVis")
            ageSDL.setNotify(self.key,"tldnSlaveCaveSecretDoorVis",0.0)
            ageSDL.setFlags("tldnSlaveCaveSecretDoorOpen",1,1)
            ageSDL.sendToClients("tldnSlaveCaveSecretDoorOpen")
            ageSDL.setNotify(self.key,"tldnSlaveCaveSecretDoorOpen",0.0)
            tmpbookvis = ageSDL["tldnSlaveCaveSecretDoorVis"][0]
            tmpbookopen = ageSDL["tldnSlaveCaveSecretDoorOpen"][0]
            if param == "on" or param == "1":
                if not tmpbookvis:
                    ageSDL["tldnSlaveCaveSecretDoorVis"] = (1,)
                if not tmpbookopen:
                    ageSDL["tldnSlaveCaveSecretDoorOpen"] = (1,)
            elif param == "off" or param == "0":
                if tmpbookvis:
                    ageSDL["tldnSlaveCaveSecretDoorVis"] = (0,)
                if tmpbookopen:
                    ageSDL["tldnSlaveCaveSecretDoorOpen"] = (0,)

