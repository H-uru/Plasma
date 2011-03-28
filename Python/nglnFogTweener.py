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
Module: nglnFogTweener
Age: Negilahn
Date: December 2003
Author: Doug McBride
Animates the Fog RGB and Density troughout the Negilahn Day cycle
"""

from Plasma import *
from PlasmaTypes import *
import string

stringVarName = ptAttribString(1,"Battery Updated SDL")

SunriseRGB = ptAttribString(2,"Sunrise: (Red,Green,Blue)")
SunriseDensity = ptAttribString(3,"Sunrise: (Start,End,Density)")

NoonRGB = ptAttribString(4,"Noon: (Red,Green,Blue)")
NoonDensity = ptAttribString(5,"Noon: (Start,End,Density)")

SunsetRGB = ptAttribString(6,"Sunset: (Red,Green,Blue)")
SunsetDensity = ptAttribString(7,"Sunset: (Start,End,Density)")

MidnightRGB = ptAttribString(8,"Midnight: (Red,Green,Blue)")
MidnightDensity = ptAttribString(9,"Midnight: (Start,End,Density)")

#globals
kSunrisePct = 0
kNoonPct = .25
kSunsetPct = .50
kMidnightPct = .75

SunriseR = 0
SunriseG = 0
SunriseB = 0
SunriseS = 0
SunriseE = 0
SunriseD = 0

NoonR = 0
NoonG = 0
NoonB = 0
NoonS = 0
NoonE = 0
NoonD = 0

SunsetR = 0
SunsetG = 0
SunsetB = 0
SunsetS = 0
SunsetE = 0
SunsetD = 0

MidnightR = 0
MidnightG = 0
MidnightB = 0
MidnightS = 0
MidnightE = 0
MidnightD = 0

# Reset start and end values used for iinterpolating
StartR = 0
StartG = 0
StartB = 0
EndR = 0
EndG = 0
EndB = 0
StartS = 0
EndS = 0
StartE = 0
EndE = 0
StartD = 0
EndD = 0


class nglnFogTweener(ptMultiModifier):

    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 5243
        version = 3
        self.version = version
        print "__init__nglnFogTweener v.", version        
        
        self.SunriseRGBList = []
        self.SunriseDensityList = []
        self.NoonRGBList = []
        self.NoonDensityList = []
        self.SunsetRGBList = []
        self.SunsetDensityList = []
        self.MidnightRGBList = []
        self.MidnightDensityList = []

    def OnFirstUpdate(self):
        global SunriseR
        global SunriseG
        global SunriseB
        global SunriseS
        global SunriseE
        global SunriseD
        global NoonR
        global NoonG
        global NoonB
        global NoonS
        global NoonE
        global NoonD
        global SunsetR
        global SunsetG
        global SunsetB
        global SunsetS
        global SunsetE
        global SunsetD
        global MidnightR
        global MidnightG
        global MidnightB
        global MidnightS
        global MidnightE
        global MidnightD

        
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(stringVarName.value,1,1)
        ageSDL.sendToClients(stringVarName.value)
        
        self.SunriseRGBList = SunriseRGB.value.split(",")
        SunriseR = string.atof(self.SunriseRGBList[0])
        SunriseG = string.atof(self.SunriseRGBList[1])
        SunriseB = string.atof(self.SunriseRGBList[2])
        
        self.SunriseDensityList = SunriseDensity.value.split(",")
        SunriseS = string.atof(self.SunriseDensityList[0])
        SunriseE = string.atof(self.SunriseDensityList[1])
        SunriseD = string.atof(self.SunriseDensityList[2])

        self.NoonRGBList = NoonRGB.value.split(",")
        NoonR = string.atof(self.NoonRGBList[0])
        NoonG = string.atof(self.NoonRGBList[1])
        NoonB = string.atof(self.NoonRGBList[2])

        self.NoonDensityList = NoonDensity.value.split(",")
        NoonS = string.atof(self.NoonDensityList[0])
        NoonE = string.atof(self.NoonDensityList[1])
        NoonD = string.atof(self.NoonDensityList[2])

        self.SunsetRGBList = SunsetRGB.value.split(",")
        SunsetR = string.atof(self.SunsetRGBList[0])
        SunsetG = string.atof(self.SunsetRGBList[1])
        SunsetB = string.atof(self.SunsetRGBList[2])

        self.SunsetDensityList = SunsetDensity.value.split(",")
        SunsetS = string.atof(self.SunsetDensityList[0])
        SunsetE = string.atof(self.SunsetDensityList[1])
        SunsetD = string.atof(self.SunsetDensityList[2])


        self.MidnightRGBList = MidnightRGB.value.split(",")
        MidnightR = string.atof(self.MidnightRGBList[0])
        MidnightG = string.atof(self.MidnightRGBList[1])
        MidnightB = string.atof(self.MidnightRGBList[2])
        
        self.MidnightDensityList = MidnightDensity.value.split(",")
        MidnightS = string.atof(self.MidnightDensityList[0])
        MidnightE = string.atof(self.MidnightDensityList[1])
        MidnightD = string.atof(self.MidnightDensityList[2])        
        
        print "nglnFogTweener.OnFirstUpdate: SunriseRGB=(%s,%s,%s), NoonRGB=(%s,%s,%s), SunsetRGB=(%s,%s,%s), MidnightRGB=(%s,%s,%s) " % (SunriseR, SunriseG, SunriseB, NoonR, NoonG, NoonB, SunsetR, SunsetG, SunsetB, MidnightR, MidnightG, MidnightB)
        print "nglnFogTweener.OnFirstUpdate: SunriseDensity=(%s,%s,%s), NoonDensity=(%s,%s,%s), SunsetDensity=(%s,%s,%s), MidnightDensity=(%s,%s,%s) " % (SunriseS, SunriseE, SunriseD, NoonS, NoonE, NoonD, SunsetS, SunsetE, SunsetD, MidnightS, MidnightE, MidnightD)
        
            
    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setNotify(self.key,stringVarName.value,0.0)
        
        self.CalculateNewFogValues()
        
        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname != stringVarName.value:
            return
        else:
            self.CalculateNewFogValues()


    def CalculateNewFogValues(self):
        global StartR
        global EndR
        global StartG
        global EndG
        global StartB
        global EndB
        
        global StartS
        global EndS
        global StartE
        global EndE
        global StartD
        global EndD
        
            
        #~ print "nglnFogTweener.CalculateNewFogValues: The battery was updated. Time to update the Fog."
        
        AgeTimeOfDayPercent = PtGetAgeTimeOfDayPercent()      
        
        print "nglnFogTweener: The day is %.2f%% through its complete cycle."  % (AgeTimeOfDayPercent*100) 
        if AgeTimeOfDayPercent > kSunrisePct and AgeTimeOfDayPercent < kNoonPct:
            StartR = SunriseR
            EndR = NoonR
            StartG = SunriseG
            EndG = NoonG
            StartB = SunriseB
            EndB = NoonB
            StartS = SunriseS
            EndS = NoonS
            StartE = SunriseE
            EndE = NoonE
            StartD = SunriseD
            EndD = NoonD
            
            TweenPct = (AgeTimeOfDayPercent - kSunrisePct) / (kNoonPct - kSunrisePct)
            
            print "\tIt's after Sunrise and before Noon. (%.2f%% through this quadrant)" % (TweenPct*100)

            
        elif AgeTimeOfDayPercent > kNoonPct and AgeTimeOfDayPercent < kSunsetPct:
            StartR = NoonR
            EndR = SunsetR
            StartG = NoonG
            EndG = SunsetG
            StartB = NoonB
            EndB = SunsetB
            
            StartS = NoonS
            EndS = SunsetS
            StartE = NoonE
            EndE = SunsetE
            StartD = NoonD
            EndD = SunsetD
            
            TweenPct = (AgeTimeOfDayPercent - kNoonPct) / (kSunsetPct - kNoonPct)
            print "\tIt's after Noon and before Sunset. (%.2f%% this quadrant)" % (TweenPct*100)
            
        elif AgeTimeOfDayPercent > kSunsetPct and AgeTimeOfDayPercent < kMidnightPct:
            StartR = SunsetR
            EndR = MidnightR
            StartG = SunsetG
            EndG = MidnightG
            StartB = SunsetB
            EndB = MidnightB
            
            StartS = SunsetS
            EndS = MidnightS
            StartE = SunsetE
            EndE = MidnightE
            StartD = SunsetD
            EndD = MidnightD
            
            TweenPct = (AgeTimeOfDayPercent - kSunsetPct) / (kMidnightPct - kSunsetPct)
            print "\tIt's after Sunset and before Midnight. (%.2f%% this quadrant)" % (TweenPct*100)
            
        elif AgeTimeOfDayPercent > kMidnightPct and AgeTimeOfDayPercent < 1:
            StartR = MidnightR
            EndR = SunriseR
            StartG = MidnightG
            EndG = SunriseG
            StartB = MidnightB
            EndB = SunriseB
            
            StartS = MidnightS
            EndS = SunriseS
            StartE = MidnightE
            EndE = SunriseE
            StartD = MidnightD
            EndD = SunriseD
            
            TweenPct = (AgeTimeOfDayPercent - kMidnightPct) / (1 - kMidnightPct)
            print "\tIt's after Midnight and before Sunrise. (%.2f%% this quadrant)" % (TweenPct*100)
            
        else: 
            print "ERROR: I can't tell what time it is."
        
        #~ print "OnSDLNotify: StartR=%s, EndR=%s, StartG=%s, EndG==%s, StartB=%s, EndB=%s" % (StartR, EndR, StartG, EndG, StartB, EndB)
        
        self.UpdateFog(StartR, EndR, StartG, EndG, StartB, EndB, StartS, EndS, StartE, EndE, StartD, EndD, TweenPct)
    
    def UpdateFog(self, StartR, EndR, StartG, EndG, StartB, EndB, StartS, EndS, StartE, EndE, StartD, EndD, TweenPct):
        #~ print "TweenPct = ", TweenPct
        print "UpdateFog: StartR=%s, EndR=%s, StartG=%s, EndG=%s, StartB=%s, EndB=%s" % (StartR, EndR, StartG, EndG, StartB, EndB)
        print "UpdateFog: StartS=%s, EndS=%s, StartE=%s, EndE=%s, StartD=%s, EndD=%s" % (StartS, EndS, StartE, EndE, StartD, EndD)


        NewR = StartR + ((EndR - StartR) * TweenPct)
        NewG = StartG + ((EndG - StartG) * TweenPct)
        NewB = StartB + ((EndB - StartB) * TweenPct)
        
        NewS = StartS + ((EndS - StartS) * TweenPct)
        NewE = StartE + ((EndE - StartE) * TweenPct)
        NewD = StartD + ((EndD - StartD) * TweenPct)

        print "UpdateFog: The new fog RGB is (%.3f, %.3f, %.3f)" % (NewR, NewG, NewB)
        print "UpdateFog: The new fog Density is (%.3f, %.3f, %.3f)" % (NewS, NewE, NewD)
        
        newfogcolor = ptColor(red=NewR, green=NewG, blue=NewB)
        
        PtFogSetDefColor(newfogcolor)
        PtFogSetDefLinear(NewS, NewE, NewD)
