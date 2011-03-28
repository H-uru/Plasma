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
Module: xPodBattery
Age: global
Date: December 2003
Manages the charging and draining of the Pod Battery.
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
fSunrise                = ptAttribFloat(1, "Percent the sun rises", 0.0, (0.0, 1.0))
fSunset                 = ptAttribFloat(2, "Percent the sun sets", 0.5, (0.0, 1.0))

SDLBatteryCharge        = ptAttribString(3, "SDL: Battery Charge")
SDLBatteryCapacity      = ptAttribString(4, "SDL: Battery Capacity")
SDLBatteryLastUpdated   = ptAttribString(5, "SDL: Battery Last Updated")

SDLPodLights            = ptAttribString(6, "SDL: Pod Lights")
SDLSpeaker01            = ptAttribString(7, "SDL: Speaker01")
SDLSpeaker02            = ptAttribString(8, "SDL: Speaker02")
SDLSpeaker03            = ptAttribString(9, "SDL: Speaker03")
SDLSpeaker04            = ptAttribString(10, "SDL: Speaker04")
SDLSpotlight01          = ptAttribString(11, "SDL: Spotlight01")
SDLSpotlight02          = ptAttribString(12, "SDL: Spotlight02")
SDLSpotlight03          = ptAttribString(13, "SDL: Spotlight03")

actSpeaker01            = ptAttribActivator(14, "Act: Speaker01")
actSpeaker02            = ptAttribActivator(15, "Act: Speaker02")
actSpeaker03            = ptAttribActivator(16, "Act: Speaker03")
actSpeaker04            = ptAttribActivator(17, "Act: Speaker04")
actSpotlight01          = ptAttribActivator(18, "Act: Spotlight01")
actSpotlight02          = ptAttribActivator(19, "Act: Spotlight02")
actSpotlight03          = ptAttribActivator(20, "Act: Spotlight03")
actPodLights            = ptAttribActivator(21, "Act: Pod Lights")

behSpeaker01            = ptAttribResponder(22, "Beh: Speaker01")
behSpeaker02            = ptAttribResponder(23, "Beh: Speaker02")
behSpeaker03            = ptAttribResponder(24, "Beh: Speaker03")
behSpeaker04            = ptAttribResponder(25, "Beh: Speaker04")
behSpotlight01          = ptAttribResponder(26, "Beh: Spotlight01")
behSpotlight02          = ptAttribResponder(27, "Beh: Spotlight02")
behSpotlight03          = ptAttribResponder(28, "Beh: Spotlight03")
behPodLights            = ptAttribResponder(29, "Beh: Pod Lights", ["1", "0"])

respSpeaker01           = ptAttribResponder(30, "Resp: Speaker01", ["1", "0"], netForce=1)
respSpeaker02           = ptAttribResponder(31, "Resp: Speaker02", ["1", "0"], netForce=1)
respSpeaker03           = ptAttribResponder(32, "Resp: Speaker03", ["1", "0"], netForce=1)
respSpeaker04           = ptAttribResponder(33, "Resp: Speaker04", ["1", "0"], netForce=1)
#respSpotlight01         = ptAttribNamedResponder(34, "Resp: Spotlight01", ["1", "0"])
#respSpotlight02         = ptAttribNamedResponder(35, "Resp: Spotlight02", ["1", "0"])
#respSpotlight03         = ptAttribNamedResponder(36, "Resp: Spotlight03", ["1", "0"])
respPodLights           = ptAttribResponder(37, "Resp: Pod Lights", ["1", "0"], netForce=1)

boolEmergencyPower      = ptAttribBoolean(38, "Emergency Power Only")

respPodLightsTripped    = ptAttribResponder(39, "Resp: Power Tripped", netForce=1)

#globals
kTimeIncrement = 10 # seconds between battery updates
CurrentTime = 0
Avvie = None

# Battery unit cost per hour
kPowerOnDrain = 6.6
kMicrophoneOnDrain = 4.0
kSpotlightOnDrain = 8.0

kSunRechargeRate = 6.6
BatteryCharge = 100
BatteryCapacity = 100

kDayLengthInSeconds = 56585 # Length of a Negilahn day in seconds Must match value in Negilahn.age file

#====================================

class xPodBattery(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5242
        version = 5
        self.version = version
        print "__init__xPodBattery v.", version

    ###########################
    def OnServerInitComplete(self):
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "xPodBattery:\tERROR---Cannot find the Negilahn Age SDL"
            ageSDL[SDLBatteryCharge.value] = (100, ) 
            ageSDL[SDLBatteryCapacity.value] = (100, ) 
            ageSDL[SDLBatteryLastUpdated.value] = (0,)
            ageSDL[SDLPodLights.value] = (0,)
            ageSDL[SDLSpeaker01.value] = (0,)
            ageSDL[SDLSpeaker02.value] = (0,)
            ageSDL[SDLSpeaker03.value] = (0,)
            ageSDL[SDLSpeaker04.value] = (0,)
            ageSDL[SDLSpotlight01.value] = (0,)
            ageSDL[SDLSpotlight02.value] = (0,)
            ageSDL[SDLSpotlight03.value] = (0,)

        ageSDL.sendToClients(SDLBatteryCharge.value)
        ageSDL.sendToClients(SDLBatteryCapacity.value)
        ageSDL.sendToClients(SDLPodLights.value)
        ageSDL.sendToClients(SDLSpeaker01.value)
        ageSDL.sendToClients(SDLSpeaker02.value)
        ageSDL.sendToClients(SDLSpeaker03.value)
        ageSDL.sendToClients(SDLSpeaker04.value)
        ageSDL.sendToClients(SDLSpotlight01.value)
        ageSDL.sendToClients(SDLSpotlight02.value)
        ageSDL.sendToClients(SDLSpotlight03.value)

        ageSDL.setFlags(SDLBatteryCharge.value,1,1)
        ageSDL.setFlags(SDLBatteryCapacity.value,1,1)
        ageSDL.setFlags(SDLPodLights.value,1,1)
        ageSDL.setFlags(SDLSpeaker01.value,1,1)
        ageSDL.setFlags(SDLSpeaker02.value,1,1)
        ageSDL.setFlags(SDLSpeaker03.value,1,1)
        ageSDL.setFlags(SDLSpeaker04.value,1,1)
        ageSDL.setFlags(SDLSpotlight01.value,1,1)
        ageSDL.setFlags(SDLSpotlight02.value,1,1)
        ageSDL.setFlags(SDLSpotlight03.value,1,1)

        ageSDL.setNotify(self.key, SDLBatteryCharge.value, 0.0)
        ageSDL.setNotify(self.key, SDLBatteryCapacity.value, 0.0)
        ageSDL.setNotify(self.key, SDLPodLights.value, 0.0)
        ageSDL.setNotify(self.key, SDLSpeaker01.value, 0.0)
        ageSDL.setNotify(self.key, SDLSpeaker02.value, 0.0)
        ageSDL.setNotify(self.key, SDLSpeaker03.value, 0.0)
        ageSDL.setNotify(self.key, SDLSpeaker04.value, 0.0)
        ageSDL.setNotify(self.key, SDLSpotlight01.value, 0.0)
        ageSDL.setNotify(self.key, SDLSpotlight02.value, 0.0)
        ageSDL.setNotify(self.key, SDLSpotlight03.value, 0.0)
        
        AgeTimeOfDayPercent = PtGetAgeTimeOfDayPercent()
        BatteryCharge = ageSDL[SDLBatteryCharge.value][0]
        BatteryCapacity = ageSDL[SDLBatteryCapacity.value][0]
        BatteryLastUpdated = ageSDL[SDLBatteryLastUpdated.value][0]

        if ageSDL[SDLPodLights.value][0]:
            respPodLights.run(self.key, state="1", fastforward=1)
            behPodLights.run(self.key, state="1", fastforward=1)
        if ageSDL[SDLSpeaker01.value][0]:
            respSpeaker01.run(self.key, state="1", fastforward=1)
        if ageSDL[SDLSpeaker02.value][0]:
            respSpeaker02.run(self.key, state="1", fastforward=1)
        if ageSDL[SDLSpeaker03.value][0]:
            respSpeaker03.run(self.key, state="1", fastforward=1)
        if ageSDL[SDLSpeaker04.value][0]:
            respSpeaker04.run(self.key, state="1", fastforward=1)
        #if ageSDL[SDLSpotlight01.value][0]:
        #    respSpotlight01.run(self.key, state="1", fastforward=1)
        #if ageSDL[SDLSpotlight02.value][0]:
        #    respSpotlight02.run(self.key, state="1", fastforward=1)
        #if ageSDL[SDLSpotlight03.value][0]:
        #    respSpotlight03.run(self.key, state="1", fastforward=1)

        print "xPodBattery: The Pod Battery has %s of a possible %s units." % (BatteryCharge, BatteryCapacity)
        CurrentTime = PtGetDniTime()
        
        if len(PtGetPlayerList()) == 0:
            if BatteryLastUpdated == 0:
                ageSDL[SDLBatteryLastUpdated.value] = (CurrentTime,)
                print "xPodBattery: This is your first time here. The Battery has never been updated."
            else:
                self.SimulateDrainDuringVacancy(CurrentTime, BatteryLastUpdated, BatteryCharge)
        
        PtAtTimeCallback(self.key, kTimeIncrement, 1) 

    ###########################
    def SimulateDrainDuringVacancy(self, CurrentTime, BatteryLastUpdated, BatteryCharge):
        TimeSinceUpdate = CurrentTime - BatteryLastUpdated
        WholeDaysVacated = int(TimeSinceUpdate / kDayLengthInSeconds)
        FractionalDaysVacated = TimeSinceUpdate % kDayLengthInSeconds
        AgeTimeOfDayPercent = PtGetAgeTimeOfDayPercent()
        CurrentAgeTimeOfDay = AgeTimeOfDayPercent * kDayLengthInSeconds
        UnoccupiedDaylightSeconds = 0

        print "xPodBattery.SimulateDrainDuringVacancy:"
        print "\tCurrentTime: %d" % (CurrentTime) # current time in seconds since epoch Mountain time
        print "\tBatteryLastUpdated: %d" % (BatteryLastUpdated) # last update in seconds since the epoch Mountain time
        print "\tTimeSinceUpdate: %d" % (TimeSinceUpdate) # total seconds the pod was vacated
        print "\tWholeDaysVacated: %d" % (WholeDaysVacated) # days the pod was vacated
        print "\tFractionalDaysVacated: %d" % (FractionalDaysVacated) # seconds the pod was vacated minus whole days
        print "\tAgeTimeOfDayPercent: %.2f%%" % (AgeTimeOfDayPercent*100) # percentage through the Negilahn day
        print "\tCurrentAgeTimeOfDay: %.2f of %d" % (CurrentAgeTimeOfDay, kDayLengthInSeconds) # seconds into the Negilahn day

        if FractionalDaysVacated > CurrentAgeTimeOfDay:
            TimeOfDayVacated = kDayLengthInSeconds - (FractionalDaysVacated - CurrentAgeTimeOfDay)
            print "\tThe pod was vacated at a time of day later than it is now."

            if TimeOfDayVacated < kDayLengthInSeconds * fSunset.value:
                UnoccupiedDaylightSeconds = (kDayLengthInSeconds * fSunset.value) - (TimeOfDayVacated - CurrentAgeTimeOfDay)
            else:
                if CurrentAgeTimeOfDay < kDayLengthInSeconds * fSunset.value:
                    UnoccupiedDaylightSeconds = CurrentAgeTimeOfDay
                else:
                    UnoccupiedDaylightSeconds = kDayLengthInSeconds * fSunset.value

        elif FractionalDaysVacated < CurrentAgeTimeOfDay:
            TimeOfDayVacated = CurrentAgeTimeOfDay - FractionalDaysVacated
            print "\tThe Pod was vacated at a time of day earlier than it is now."

            if TimeOfDayVacated < kDayLengthInSeconds * fSunset.value:
                if CurrentAgeTimeOfDay <= kDayLengthInSeconds * fSunset.value:
                    UnoccupiedDaylightSeconds = FractionalDaysVacated
                else:
                    UnoccupiedDaylightSeconds = (kDayLengthInSeconds * fSunset.value) - TimeOfDayVacated

        else:
            TimeOfDayVacated = CurrentAgeTimeOfDay
            print "\tThe Pod was vacated at this exact time."

        UnoccupiedDaylightSeconds += (WholeDaysVacated * kDayLengthInSeconds)
        print "\tTimeOfDayVacated: %.2f of %d" % (TimeOfDayVacated, kDayLengthInSeconds) # seconds into the Negilahn day when the pod was vacated
        print "\tUnoccupiedDaylightSeconds: %.2f" % (UnoccupiedDaylightSeconds) # seconds of daylight the pod was vacated

        print "xPodBattery.SimulateDrainDuringVacancy: The following gadgets were left on while the Pod was vacant:"
        self.CalculateCostPerCycle()
        print "xPodBattery.SimulateDrainDuringVacancy: SimulateDrainDuringVacancy: CostThisCycle: %.4f" % (CostThisCycle)

        CumulativeRecharge = (UnoccupiedDaylightSeconds / 3600.0) * kSunRechargeRate
        CumulativeDrain = (TimeSinceUpdate / 10.0) * CostThisCycle

        print "\tCumulativeRecharge: %.2f" % (CumulativeRecharge) # units of rechange
        print "\tCumulativeDrain: %.2f" % (CumulativeDrain) # units of discharge

        BatteryCharge += (CumulativeRecharge - CumulativeDrain)
        self.UpdateBatteryChargeSDL(BatteryCharge)

    ###########################
    def CalculateCostPerCycle(self):
        global CostThisCycle

        ageSDL = PtGetAgeSDL()        
        CostThisCycle = 0
        
        boolPodLights = ageSDL[SDLPodLights.value][0]

        boolSpeaker01 = ageSDL[SDLSpeaker01.value][0]
        boolSpeaker02 = ageSDL[SDLSpeaker02.value][0]
        boolSpeaker03 = ageSDL[SDLSpeaker03.value][0]
        boolSpeaker04 = ageSDL[SDLSpeaker04.value][0]

        boolSpotlight01 = ageSDL[SDLSpotlight01.value][0]
        boolSpotlight02 = ageSDL[SDLSpotlight02.value][0]
        boolSpotlight03 = ageSDL[SDLSpotlight03.value][0]

        if boolPodLights and not boolEmergencyPower.value:
            print "\tPodLights"
            CostThisCycle += (kPowerOnDrain * (kTimeIncrement / 3600.0))

        if boolSpeaker01 and not boolEmergencyPower.value:
            print "\tSpeaker01"
            CostThisCycle += (kMicrophoneOnDrain * (kTimeIncrement / 3600.0))
        if boolSpeaker02 and not boolEmergencyPower.value:
            print "\tSpeaker02"
            CostThisCycle += (kMicrophoneOnDrain * (kTimeIncrement / 3600.0))
        if boolSpeaker03 and not boolEmergencyPower.value:
            print "\tSpeaker03"
            CostThisCycle += (kMicrophoneOnDrain * (kTimeIncrement / 3600.0))
        if boolSpeaker04 and not boolEmergencyPower.value:
            print "\tSpeaker04"
            CostThisCycle += (kMicrophoneOnDrain * (kTimeIncrement / 3600.0))

        if boolSpotlight01 and not boolEmergencyPower.value:
            print "\tSpotlight01"
            CostThisCycle += (kSpotlightOnDrain * (kTimeIncrement / 3600.0))
        if boolSpotlight02 and not boolEmergencyPower.value:
            print "\tSpotlight02"
            CostThisCycle += (kSpotlightOnDrain * (kTimeIncrement / 3600.0))
        if boolSpotlight03 and not boolEmergencyPower.value:
            print "\tSpotlight03"
            CostThisCycle += (kSpotlightOnDrain * (kTimeIncrement / 3600.0))

    ###########################
    def CalculatePowerDrain(self):
        global CostThisCycle

        print "xPodBattery.CalculatePowerDrain: The following gadgets are currently turned on:"
        self.CalculateCostPerCycle()
        print "xPodBattery.CalculatePowerDrain: Over the last %s seconds, you drained the battery %.4f units." %(kTimeIncrement, CostThisCycle)

        RechargeFromSun = 0
        ageSDL = PtGetAgeSDL()
        BatteryCharge = ageSDL[SDLBatteryCharge.value][0]
        BatteryCharge -= CostThisCycle
        AgeTimeOfDayPercent = PtGetAgeTimeOfDayPercent()      

        # If it's currently day, include recharge effect of the Sun
        if AgeTimeOfDayPercent >= fSunrise.value and AgeTimeOfDayPercent <= fSunset.value:
            RechargeFromSun = kSunRechargeRate * (kTimeIncrement / 3600.0)
            print "xPodBattery.CalculatePowerDrain: The time is %.1f%% through the daytime, adding %.4f units to the battery." % (((AgeTimeOfDayPercent * 2) * 100), RechargeFromSun)
            BatteryCharge += RechargeFromSun

        if (CostThisCycle - RechargeFromSun) > 0:
            EstimatedTimeLeft = BatteryCharge / (CostThisCycle * kTimeIncrement)
            print "xPodBattery.CalculatePowerDrain: At this rate, you'll run out of power in %.2f minutes." % (EstimatedTimeLeft)

        self.UpdateBatteryChargeSDL(BatteryCharge)

    ###########################
    def UpdateBatteryChargeSDL(self,BatteryCharge):
        ageSDL = PtGetAgeSDL()

        if BatteryCharge <= 0:
            BatteryCharge = 0
            ageSDL[SDLPodLights.value] = (0,)
        else:
            if BatteryCharge > BatteryCapacity:
                BatteryCharge = BatteryCapacity

        ageSDL[SDLBatteryCharge.value] = (BatteryCharge,)
        print "xPodBattery.UpdateBatteryChargeSDL: The Pod Battery now has %.4f units." % (BatteryCharge)

        CurrentTime = PtGetDniTime()
        ageSDL[SDLBatteryLastUpdated.value] = (CurrentTime,)

    ###########################
    def OnTimer(self,timer):
        if timer == 1:
            if self.sceneobject.isLocallyOwned():
                self.CalculatePowerDrain()
            PtAtTimeCallback(self.key,kTimeIncrement,1) 

    ###########################
    def OnNotify(self,state,id,events):
        global Avvie
        ageSDL = PtGetAgeSDL()

        print "xPodBattery.OnNotify: state=%s id=%d events=" % (state, id), events
        
        if id == actPodLights.id and state:
            Avvie = PtFindAvatar(events)
            newVal = int(not ageSDL[SDLPodLights.value][0])
            behPodLights.run(self.key, state=str(newVal), avatar=Avvie)
        
        elif id == actSpeaker01.id and state:
            Avvie = PtFindAvatar(events)
            behSpeaker01.run(self.key, avatar=Avvie)
        
        elif id == actSpeaker02.id and state:
            Avvie = PtFindAvatar(events)
            behSpeaker02.run(self.key, avatar=Avvie)
        
        elif id == actSpeaker03.id and state:
            Avvie = PtFindAvatar(events)
            behSpeaker03.run(self.key, avatar=Avvie)
        
        elif id == actSpeaker04.id and state:
            Avvie = PtFindAvatar(events)
            behSpeaker04.run(self.key, avatar=Avvie)
        
        elif id == actSpotlight01.id and state:
            Avvie = PtFindAvatar(events)
            behSpotlight01.run(self.key, avatar=Avvie)
        
        elif id == actSpotlight02.id and state:
            Avvie = PtFindAvatar(events)
            behSpotlight02.run(self.key, avatar=Avvie)

        elif id == actSpotlight03.id and state:
            Avvie = PtFindAvatar(events)
            behSpotlight03.run(self.key, avatar=Avvie)

        elif self.sceneobject.isLocallyOwned():
            if id == behPodLights.id:
                newVal = int(not ageSDL[SDLPodLights.value][0])
                ageSDL[SDLPodLights.value] = (newVal,)
            elif id == behSpeaker01.id:
                if ageSDL[SDLPodLights.value][0]:
                    newVal = int(not ageSDL[SDLSpeaker01.value][0])
                    ageSDL[SDLSpeaker01.value] = (newVal,)
            elif id == behSpeaker02.id:
                if ageSDL[SDLPodLights.value][0]:
                    newVal = int(not ageSDL[SDLSpeaker02.value][0])
                    ageSDL[SDLSpeaker02.value] = (newVal,)
            elif id == behSpeaker03.id:
                if ageSDL[SDLPodLights.value][0]:
                    newVal = int(not ageSDL[SDLSpeaker03.value][0])
                    ageSDL[SDLSpeaker03.value] = (newVal,)
            elif id == behSpeaker04.id:
                if ageSDL[SDLPodLights.value][0]:
                    newVal = int(not ageSDL[SDLSpeaker04.value][0])
                    ageSDL[SDLSpeaker04.value] = (newVal,)
            elif id == behSpotlight01.id:
                if ageSDL[SDLPodLights.value][0]:
                    newVal = int(not ageSDL[SDLSpotlight01.value][0])
                    ageSDL[SDLSpotlight01.value] = (newVal,)
            elif id == behSpotlight02.id:
                if ageSDL[SDLPodLights.value][0]:
                    newVal = int(not ageSDL[SDLSpotlight02.value][0])
                    ageSDL[SDLSpotlight02.value] = (newVal,)
            elif id == behSpotlight03.id:
                if ageSDL[SDLPodLights.value][0]:
                    newVal = int(not ageSDL[SDLSpotlight03.value][0])
                    ageSDL[SDLSpotlight03.value] = (newVal,)
        
    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        print "xPodBattery.OnSDLNotify(): VARname:%s, SDLname:%s, tag:%s, value:%s, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID)

        if self.sceneobject.isLocallyOwned():
            if VARname == SDLPodLights.value:
                respPodLights.run(self.key, state=str(ageSDL[SDLPodLights.value][0]))
                if not ageSDL[SDLPodLights.value][0]:
                    print "xPodBattery.OnSDLNotify(): Tripping all SDLs to negative"
                    respPodLightsTripped.run(self.key)
                    ageSDL[SDLSpeaker01.value] = (0,)
                    ageSDL[SDLSpeaker02.value] = (0,)
                    ageSDL[SDLSpeaker03.value] = (0,)
                    ageSDL[SDLSpeaker04.value] = (0,)
                    ageSDL[SDLSpotlight01.value] = (0,)
                    ageSDL[SDLSpotlight02.value] = (0,)
                    ageSDL[SDLSpotlight03.value] = (0,)

            elif VARname == SDLSpeaker01.value:
                respSpeaker01.run(self.key, state=str(ageSDL[SDLSpeaker01.value][0]))

            elif VARname == SDLSpeaker02.value:
                respSpeaker02.run(self.key, state=str(ageSDL[SDLSpeaker02.value][0]))

            elif VARname == SDLSpeaker03.value:
                respSpeaker03.run(self.key, state=str(ageSDL[SDLSpeaker03.value][0]))

            elif VARname == SDLSpeaker04.value:
                respSpeaker04.run(self.key, state=str(ageSDL[SDLSpeaker04.value][0]))
