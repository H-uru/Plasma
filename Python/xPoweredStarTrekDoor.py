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
Module: xPoweredStarTrekDoor
Age: global
Date: September 2002
Author: Doug McBride, (modifying Pete's "xStarTrekDoor.py" code)
When powered, auto-open, auto-close door
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
Activate = ptAttribActivator(1, "Region Sensor",netForce=1)
respDoor = ptAttribResponder(2, "Door Responder",['open','close'])
actPower = ptAttribNamedActivator(3, "Actvtr: Power Source")

# globals

doorState = "close"
doorMoving = 0
doorCued = 0
doorHistory = "close"


class xPoweredStarTrekDoor(ptModifier):
    "Standard Star Trek Door"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5216
        
        version = 1
        self.version = version
        print "__init__xPoweredStarTrekDoor v.", version

    def OnServerInitComplete(self):
        if self.SDL == None:
#            print "xPoweredStarTrekDoor.OnFirstUpdate():\tERROR---missing SDL (%s)" % actPower.value
            return
        self.SDL.setDefault("haspower",(0,))

    def OnNotify(self,state,id,events):

        global doorCued
        global doorMoving
        global doorState
        print "DoorMoving = ", doorMoving
        
        if state and id==actPower.id:
            print "message from GearActivated"
            for event in events:
                print event
                if event[0] == 4:
                    if event[3] == 1: # power on
                        self.SDL["haspower"] = (1,)

                elif event[3] == 0: #power off
                    self.SDL["haspower"] = (0,)
                
                else: #unexpected value 
                    print "xPoweredStarTrekDoor.OnNotify:\t'%s' ERROR---got bogus msg - power = %d" % (Activate.value,self.SDL["enabled"][0])
                    return
                    
#                print "xPoweredStarTrekDoor.OnNotify:\t'%s' got msg - power = %d" % (actPower.value,self.SDL["enabled"][0])


#Pete's original code

        
#       if state and id == Activate.id: # and PtWasLocallyNotified(self.key): # region is activated.

        if state and id == Activate.id and self.SDL["haspower"][0]==1: # and PtWasLocallyNotified(self.key): # region is activated.

            
            if doorState=="close": # keep track of what the freaking door should do next.
                doorState="open"
            else:
                doorState="close"
            
            if not doorMoving:
                self.doorAction()
                print "door played"
            else: # got a command, but door is busy so cue it
                doorCued=1
                print "door cued"
        elif state and id == respDoor.id:
            # Callback from door finishing movement
            print "callbackfromdoor"
            doorMoving=0
            if doorCued:
                doorCued=0
                self.doorAction()

    def doorAction(self):
        global doorMoving
        global doorHistory        
        if doorHistory != doorState:
            doorMoving=1
            doorHistory=doorState
            respDoor.run(self.key,state=doorState)
            print "Door Begin %s" % doorState




