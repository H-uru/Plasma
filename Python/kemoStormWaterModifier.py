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
Module: kemoStormWaterModifier.py
Age: Garden
Date: January 2004
"""

from Plasma import *
from PlasmaTypes import *

theWater = ptAttribWaveSet(1, "Wave set")
startRain = ptAttribActivator(2, "Start rain drops")
stopRain = ptAttribActivator(3, "Stop rain drops")

NoiseStartValue = 0
TexAmpStartValue = 0

class kemoStormWaterModifier(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5330
        self.version = 1

    def OnFirstUpdate(self):
        global NoiseStartValue
        global TexAmpStartValue
        
        NoiseStartValue = theWater.waveset.getSpecularNoise()
        TexAmpStartValue = theWater.waveset.getTexAmpOverLen()
        
    def OnNotify(self,state,id,events):

        if not state:
            return
        
        if id == startRain.id:
            print "starting rain drops on water"
            rainlevel = 3.0
            texamp  = 0.2
            #print "changing specular noise from %f to %f" % (theWater.waveset.getSpecularNoise(), rainlevel)
            #print "changing tex amplitude  from %f to %f" % (theWater.waveset.getTexAmpOverLen(), texamp)
            theWater.waveset.setSpecularNoise(rainlevel, 5)
            theWater.waveset.setTexAmpOverLen(texamp, 5)
        elif id == stopRain.id:
            print "stoping raindrops on water"
            rainlevel = NoiseStartValue
            texamp = TexAmpStartValue
            #print "changing specular noise from %f to %f" % (theWater.waveset.getSpecularNoise(), rainlevel)
            #print "changing tex amplitude  from %f to %f" % (theWater.waveset.getTexAmpOverLen(), texamp)
            theWater.waveset.setSpecularNoise(rainlevel, 5)
            theWater.waveset.setTexAmpOverLen(texamp, 5)

    def OnServerInitComplete(self):
        pass

    def OnTimer(self, id):
        pass
            
    def OnBackdoorMsg(self, target, param):
        if target == "watertest":
            wdir = dir(theWater.waveset)

            val = 1
            for x in wdir:
                if x[:3] == "set":
                    if x == "setWindDir" or x == "setWaterOffset" or x == "setDepthFalloff" or x == "setMaxAtten" or x == "setMinAtten":
                        p = ptVector3(0,val,0)
                    elif x == "setWaterTint":
                        p = ptColor().red()
                    elif x == "setSpecularTint":
                        p = ptColor().green()
                    elif x == "setEnvCenter":
                        p = ptPoint3(0,val,0)
                    else:
                        if x[3:] == "SpecularMute":
                            p = 0.5
                        else:
                            p = val

                    print "Using: get/set" + x[3:]

                    startval = getattr(theWater.waveset, "get" + x[3:])()

                    if type(p) == type(ptColor().red()):
                        print "\tstartval = " + str( (startval.getRed(), startval.getGreen(), startval.getBlue()) )
                        print "\tsetting to " + str( (p.getRed(), p.getGreen(), p.getBlue()) )
                    elif type(p) == type(ptPoint3()) or type(p) == type(ptVector3()):
                        print "\tstartval = " + str( (startval.getX(), startval.getY(), startval.getZ()) )
                        print "\tsetting to " + str( (p.getX(), p.getY(), p.getZ()) )
                    else:
                        print "\tstartval = " + str(startval)
                        print "\tsetting to " + str(p)
                    
                    getattr(theWater.waveset, x)(p)

                    endval = getattr(theWater.waveset, "get" + x[3:])()

                    if type(p) == type(ptColor().red()):
                        print "\tendval = " + str( (endval.getRed(), endval.getGreen(), endval.getBlue()) )
                    elif type(p) == type(ptPoint3()) or type(p) == type(ptVector3()):
                        print "\tendval = " + str( (endval.getX(), endval.getY(), endval.getZ()) )
                    else:
                        print "\tendval = " + str(endval)
                    
                    val += 1
