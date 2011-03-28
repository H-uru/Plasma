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
"""Module: kdshShadowPath.py
Age: Kadish Tolesa
Author: Doug McBride
Date: July 2002
Operates the Shadow Path puzzle.
"""

from Plasma import *
from PlasmaTypes import *
import string

# define the attributes that will be entered in 3dsMAX
actSwitch01 = ptAttribActivator(1, "Actvr: Switch 01") # Switches are numbered left to right in a counterclockwise fashion
actSwitch02 = ptAttribActivator(2, "Actvr: Switch 02")
actSwitch03 = ptAttribActivator(3, "Actvr: Switch 03")
actSwitch04 = ptAttribActivator(4, "Actvr: Switch 04")
actSwitch05 = ptAttribActivator(5, "Actvr: Switch 05")


respSwitch01 = ptAttribResponder(6,"Rspndr: Switch 01",['on', 'off']) # responders which animate the motion of the switches themselves
respSwitch02 = ptAttribResponder(7,"Rspndr: Switch 02",['on', 'off'])
respSwitch03 = ptAttribResponder(8,"Rspndr: Switch 03",['on', 'off'])
respSwitch04 = ptAttribResponder(9,"Rspndr: Switch 04",['on', 'off'])
respSwitch05 = ptAttribResponder(10,"Rspndr: Switch 05",['on', 'off'])

regZone01 = ptAttribActivator(11, "Stair Zone 01")
regZone02 = ptAttribActivator(12, "Stair Zone 02")
regZone03 = ptAttribActivator(13, "Stair Zone 03")
regZone04 = ptAttribActivator(14, "Stair Zone 04")
regZone05 = ptAttribActivator(15, "Stair Zone 05")
regZone06 = ptAttribActivator(16, "Stair Zone 06")
regZone07 = ptAttribActivator(17, "Stair Zone 07")
regZone08 = ptAttribActivator(18, "Stair Zone 08")
regZone09 = ptAttribActivator(19, "Stair Zone 09")
#regZone10 = ptAttribActivator(20, "Stair Zone 10")

RevealStairs = ptAttribResponder(21,"resp:Open Floor")

FloorZone = ptAttribActivator(22,"Floor Zone")

actResetBtn = ptAttribActivator(23, "act:Reset Button")
respResetBtn = ptAttribResponder(24, "resp:Reset Button")

ConcealStairs = ptAttribResponder(25,"resp:Close Floor")

respBtnPush01 = ptAttribResponder(26,"resp:Btn Push 01",netForce=1)
respBtnPush02 = ptAttribResponder(27,"resp:Btn Push 02",netForce=1)
respBtnPush03 = ptAttribResponder(28,"resp:Btn Push 03",netForce=1)
respBtnPush04 = ptAttribResponder(29,"resp:Btn Push 04",netForce=1)
respBtnPush05 = ptAttribResponder(30,"resp:Btn Push 05",netForce=1)

OnlyOneOwner = ptAttribSceneobject(31,"OnlyOneOwner") #ensures that after a oneshots, only one client toggles the SDL values

regZoneStart = ptAttribActivator(32, "Stair Zone Start")
regZoneFinish = ptAttribActivator(33, "Stair Zone Finish")
regZoneReset = ptAttribActivator(34, "Stair Zone Reset")


# define globals
baton = 0
TwoOnFloor = false
lightClickedByAvatar = None  #the avatar who clicked on a light
resetBtnByAvatar = None  #the avatar who clicked on the reset button
localAvatar = None      #The Local Avatar
gameStarted = 0

class kdshShadowPath(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5211

        version = 10
        self.version = version
        print "__init__kdshShadowPath v.", version,".2"
        

    def OnServerInitComplete(self):
        global localAvatar
        localAvatar = PtGetLocalAvatar()

        ageSDL = PtGetAgeSDL()
        
        # register for notification of age SDL var changes        
        ageSDL.setNotify(self.key,"ShadowPathLight01",0.0)
        ageSDL.setNotify(self.key,"ShadowPathLight02",0.0)
        ageSDL.setNotify(self.key,"ShadowPathLight03",0.0)
        ageSDL.setNotify(self.key,"ShadowPathLight04",0.0)
        ageSDL.setNotify(self.key,"ShadowPathLight05",0.0)
        ageSDL.setNotify(self.key,"ShadowPathSolved",0.0)


        ageSDL.sendToClients("ShadowPathLight01")
        ageSDL.sendToClients("ShadowPathLight02")
        ageSDL.sendToClients("ShadowPathLight03")
        ageSDL.sendToClients("ShadowPathLight04")
        ageSDL.sendToClients("ShadowPathLight05")
        ageSDL.sendToClients("ShadowPathSolved")
        
        ageSDL.setFlags("ShadowPathLight01",1,1)  
        ageSDL.setFlags("ShadowPathLight02",1,1)  
        ageSDL.setFlags("ShadowPathLight03",1,1)  
        ageSDL.setFlags("ShadowPathLight04",1,1)  
        ageSDL.setFlags("ShadowPathLight05",1,1)  
        ageSDL.setFlags("ShadowPathSolved",1,1)  



        print "kdshShadowPath: When I got here:"
        # initialize pillar whatnots based on SDL state
        
        for light in [1,2,3,4,5]:
            
            lightstate = ageSDL["ShadowPathLight0" + str(light)][0]  
            print "\t ShadowPathLight0%s = %s " % (light, lightstate)
            
            if lightstate == 1:
                
                code = "respSwitch0" + str(light) + ".run(self.key,fastforward=1)"
                exec code
               
                print "\t\tTurning on light #", light
        
        solved = ageSDL["ShadowPathSolved"][0]
        if solved:
            print "\tThe Shadow Path was already solved. Revealing stairs."
            RevealStairs.run(self.key,fastforward=1)
                

    def Load(self):
        count = 1
        while count < 5:
            print "kdshShadowPath.Load(): ageSDL[ShadowPathLight0",count,"]=%d" % (ageSDL["ShadowPathLight0" + str(count)][0])
            count = count + 1

    def OnNotify(self,state,id,events):
        global TwoOnFloor
        global baton
        global lightClickedByAvatar
        global resetBtnByAvatar
        global LocalAvatar
        global gameStarted
        ageSDL = PtGetAgeSDL()

        print "kdshShadowPath:OnNotify  state=%f id=%d events=" % (state,id),events


        if id == FloorZone.id:
            if events[0][1] == 1:
                print "kdshShadowPath.OnNotify: More than one person on the floor!"
                regZoneStart.disable()
                gameStarted = 0
            elif events[0][1] == 0:
                print "kdshShadowPath.OnNotify: Only one person on the floor!"
                regZoneStart.enable()

        elif id == regZoneReset.id:
            if events[0][1] == 1:
                print "kdshShadowPath.OnNotify: Someone in the stairwell, reset disabled."
                actResetBtn.disable()
            elif events[0][1] == 0:
                print "kdshShadowPath.OnNotify: No one's in the stairwell, reset enabled."
                actResetBtn.enable()

        elif id in [1,2,3,4,5]: #true if one of the five switches clicked
            if not state:
                return
            if PtFindAvatar(events) == localAvatar:
                lightClickedByAvatar = localAvatar  #this should only happen from the local triggerer
            else:
                lightClickedByAvatar = None
                return

            print "Light ", id, " clicked."
            code = 'respBtnPush0' + str(id) + '.run(self.key,events=events)'
            exec code

        elif id in [26,27,28,29,30]: 
            if lightClickedByAvatar != localAvatar:  #Make sure we don't have any rogue avatars reporting...
                lightClickedByAvatar = None
                return

            lightClickedByAvatar = None  #Reset avatar reporting

            print "Light ", id-25, " actually touched by avatar."
            oldstate = ageSDL["ShadowPathLight0" + str(id-25)][0] 
            newstate = abs(oldstate-1) # toggle value of switch state
            ageSDL["ShadowPathLight0" + str(id-25)] = (newstate, ) # write new state value to SDL            
            return

        elif id in [regZone01.id,regZone02.id,regZone03.id,regZone04.id,regZone05.id,regZone06.id,regZone07.id,regZone08.id,regZone09.id]:
            if gameStarted:
                print "Triggered Bad Region!"
                gameStarted = 0

        elif id == regZoneStart.id:
            print "Triggered Start Region!"
            gameStarted = 1

        elif id == regZoneFinish.id:
            if gameStarted:
                gameStarted = 0
                print "kdshShadowPath: Puzzle solved."
                ageSDL["ShadowPathSolved"] = (1, ) # write new state value to SDL     

        elif state and id == actResetBtn.id:
            print "kdshShadowPath Reset Button clicked."
            resetBtnByAvatar = PtFindAvatar(events)
            respResetBtn.run(self.key,events=events)
            
        elif id == respResetBtn.id:
            if resetBtnByAvatar != localAvatar:
                resetBtnByAvatar = None
                return

            print "kdshShadowPath Reset Button Pushed. Puzzle resetting."
            resetBtnByAvatar = None
            
            #turn off the lights
            for light in [1,2,3,4,5]:
                lightstate = ageSDL["ShadowPathLight0" + str(light)][0]  
                if lightstate == 1:
                    code = 'ageSDL["ShadowPathLight0' + str(light) + '"] = (0,)'
                    print "resetcode = ", code
                    exec code
                        
                    print "\tTurning off light #", light
                    
            #...and close the floor
            ageSDL["ShadowPathSolved"] = (0,)
            
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()        
        #~ print "OnSDLNotified."
        
        if VARname == "ShadowPathSolved":
            if ageSDL["ShadowPathSolved"][0] == 1:
                print "kdshShadowPath: Opening floor"
                RevealStairs.run(self.key)
            else:
                print "kdshShadowPath: Closing floor"
                ConcealStairs.run(self.key)
                
        elif VARname[:15] == "ShadowPathLight":
            light = string.atoi(VARname[-2:]) #get the last two digits, which is the light number
                
            #~ print "OnSDLNotify: Light ", light," SDL updated."
                
            newstate = ageSDL["ShadowPathLight0" + str(light)][0] 

            if newstate == 0: # true if that switch is now off
                print "kdshShadowPath.OnSDLNotify: Light", light," was on. Turning it off."
                code = "respSwitch0" + str(light) + ".run(self.key, state='off')"
                #~ print "off code = ", code
                exec code
                
            elif newstate == 1: # true if that switch is now on
                print "kdshShadowPath.OnSDLNotify: Light", light," was off. Turning it on."
                code = "respSwitch0" + str(light) + ".run(self.key, state='on')"
                #~ print "On code = ", code
                exec code

            else: 
                print "Error. Not sure what the light thought it was."

'''
    def BatonPassCheck(self,id,events,ageSDL):
        global baton
        print "##"

        for event in events:
            if event[0] == 7: # pruning a redundant event, so we only process enters and exits
                break
                

            if event[1] == 1 :  #Player enters a zone    
                print "kdshShadowPath: Entered Zone:", id-10
                if id == 11: # true as player enters start of Shadow Path, effectively resetting baton setting
                    baton = 1
                elif id == baton+11: # true if player is entering a new zone, and the "baton" is in one less zone. Progress is being made here in solving the puzzle.
                    baton = baton + 1
                    if baton == 10:
                        print "kdshShadowPath: Puzzle solved."
                        ageSDL["ShadowPathSolved"] = (1, ) # write new state value to SDL     
                
                elif baton != 0: # true if player stepped off the path. Must start over to solve puzzle.
                    baton = 0
                    print "Baton dropped. \n"                    

                    
            elif event[1] == 0:  # Player exits a zone
                print "kdshShadowPath: Exited Zone:", id-10
                if baton != 0 and baton != (id-9): # Correctly advancing players always enter a new zone before exiting the current one. If this progress hasn't already been made, drop the baton.
                    print "kdshShadowPath: Dropped the baton."
                    baton = 0

 
        
        if baton > 0:
            print "Baton value is now:", baton
'''
