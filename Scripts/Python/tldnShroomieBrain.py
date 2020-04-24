# -*- coding: utf-8 -*-
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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
Module: tldnShroomieBrain
Age: Teledahn
Date: August 2003
Author: Doug McBride
Controls the appearance and behavior of Shroomie
"""

from Plasma import *
from PlasmaTypes import *
import random

# define the attributes that will be entered in max
actLever = ptAttribActivator(1, "act: Feeder Lever")
rgnShore01 = ptAttribActivator(2, "act: Along Shore01")
rgnShore02 = ptAttribActivator(3, "act: Along Shore02")
rgnShore03 = ptAttribActivator(4, "act: Along Shore03")

SpawnFar01 = ptAttribSceneobject(5, "spawnpt: Far 01")
SpawnFar02 = ptAttribSceneobject(6, "spawnpt: Far 02")
SpawnFar03 = ptAttribSceneobject(7, "spawnpt: Far 03")
SpawnFar04 = ptAttribSceneobject(8, "spawnpt: Far 04")
SpawnFar05 = ptAttribSceneobject(9, "spawnpt: Far 05")

SpawnMid01 = ptAttribSceneobject(10, "spawnpt: Med 01")
SpawnMid02 = ptAttribSceneobject(11, "spawnpt: Med 02")
SpawnMid03 = ptAttribSceneobject(12, "spawnpt: Med 03")
SpawnMid04 = ptAttribSceneobject(13, "spawnpt: Med 04")
SpawnMid05 = ptAttribSceneobject(14, "spawnpt: Med 05")

SpawnNear01 = ptAttribSceneobject(15, "spawnpt: Near 01")
SpawnNear02 = ptAttribSceneobject(16, "spawnpt: Near 02")
SpawnNear03 = ptAttribSceneobject(17, "spawnpt: Near 03")
SpawnNear04 = ptAttribSceneobject(18, "spawnpt: Near 04")
SpawnNear05 = ptAttribSceneobject(19, "spawnpt: Near 05")

respTrick01 = ptAttribResponder(20, "Shroomie Trick #1")
respTrick02 = ptAttribResponder(21, "Shroomie Trick #2")
respTrick03 = ptAttribResponder(22, "Shroomie Trick #3")
respTrick04 = ptAttribResponder(23, "Shroomie Trick #4")
ShroomieMaster = ptAttribSceneobject(24, "Shroomie Dummmy")

respVisible = ptAttribResponder(25, "resp: Shroomie Visible")
respInvisible = ptAttribResponder(26, "resp: Shroomie Invisible")

# define globals

#probability that shroomie will appear
LeverProb = .25
Shore01Prob = .10
Shore02Prob = .10
Shore03Prob = .10

Delay1Min = 180
Dealy1Max = 600
Delay2Min = 180
Delay2Max = 600
Delay3Min = 300
Delay3Max = 600
Delay4Min = 300
Delay4Max = 600

ShroomieTotalTimesSeen = 0
ShroomieTimeLastSeen = 0

class tldnShroomieBrain(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5237
        version = 3
        self.version = version
        PtDebugPrint("__init__tldnShroomieBrain v.", version,".2")
        random.seed()

    def OnServerInitComplete(self):
        try:
            ageSDL = PtGetAgeSDL()
        except:
            PtDebugPrint("tldnShroomieBrain:\tERROR---Cannot find the Teledahn Age SDL")
            ageSDL["ShroomieTotalTimesSeen"] = (0, ) 
            ageSDL["ShroomieTimeLastSeen"] = (0, ) 

        ageSDL.sendToClients("ShroomieTotalTimesSeen")
        ageSDL.sendToClients("ShroomieTimeLastSeen")
        
        ageSDL.setFlags("ShroomieTotalTimesSeen",1,1)  
        ageSDL.setFlags("ShroomieTimeLastSeen",1,1)  

        ShroomieTotalTimesSeen = ageSDL["ShroomieTotalTimesSeen"][0]
        ShroomieTimeLastSeen = ageSDL["ShroomieTimeLastSeen"][0]
        
        PtDebugPrint("tldnShroomieBrain: When I got here:")
        PtDebugPrint("\tShroomie has been seen", ShroomieTotalTimesSeen," times.")
        
        if ShroomieTotalTimesSeen:            
            CurrentTime = PtGetDniTime()            
            PtDebugPrint("\tShroomie was last seen ", (CurrentTime - ShroomieTimeLastSeen)," seconds ago.")
            
    def OnNotify(self,state,id,events):
        ageSDL = PtGetAgeSDL()
        
        #~ PtDebugPrint("tldnShroomieBrain.OnNotify:  state=%f id=%d events=" % (state,id),events)

        if not state:
            return
            
        if id in [1,2,3,4]:
            CurrentTime = PtGetDniTime()
            
            if self.CanShroomieBeSeen(CurrentTime):
            
                if id == actLever.id:
                    if self.WillShroomieBeSeen(LeverProb):
                        self.ShroomieSurfaces(1)
                    
                elif id == rgnShore01.id:
                    if self.WillShroomieBeSeen(Shore01Prob):
                        self.ShroomieSurfaces(2)
                    
                elif id == rgnShore02.id:
                    if self.WillShroomieBeSeen(Shore02Prob):
                        self.ShroomieSurfaces(3)
        
                elif id == rgnShore03.id:
                    if self.WillShroomieBeSeen(Shore03Prob):
                        self.ShroomieSurfaces(4)
                        
        elif id in [20,21,22,23]: #Shroomie has just finished his trick. Make him invisible again.
            respInvisible.run(self.key)            

    def CanShroomieBeSeen(self,CurrentTime):
        ageSDL = PtGetAgeSDL()
        CurrentTime = PtGetDniTime()
        
        ShroomieTimeLastSeen = ageSDL["ShroomieTimeLastSeen"][0]

        PtDebugPrint("tldnShroomieBrain: Shroomie was last seen", CurrentTime - ShroomieTimeLastSeen," seconds ago.")

        if (CurrentTime - ShroomieTimeLastSeen) > 240:
            PtDebugPrint("\tShroomie CAN be seen.")
            return True
            
        else:
            PtDebugPrint("\tShroomie CAN'T be seen.")
            return False
            

    def WillShroomieBeSeen(self,probability):
        randnum = random.randint(0,100)
        
        #~ PtDebugPrint("randnum = ",randnum,"probability = ", probability*100)
        
        if randnum < (probability*100):
            PtDebugPrint("\t Shroomie WILL be seen.")
            return True
        else:
            PtDebugPrint("\tShroomie WON'T be seen.")
            
    def ShroomieSurfaces(self,spawn):
        ageSDL = PtGetAgeSDL()

        respVisible.run(self.key)


        if spawn == 1: # it was the lever pull which attracted Shroomie
            tldnMainPowerOn = ageSDL["tldnMainPowerOn"][0]
            whichbehavior = random.randint(1,4)
            
            if tldnMainPowerOn:
                PtDebugPrint("tldnShroomieBrain: The Power Tower noise has scared Shroomie. He'll come, but not very close.")
                NearOrFar = "Far"
                
            else:                 #Determine how far out Shroomie will be seen. Added 12/12/2004
                PtDebugPrint("tldnShroomieBrain: The Power Tower is down, so Shroomie isn't scared by the noise.")

                howclose = random.randint(1,100)
                if howclose == 1:
                    NearOrFar = "Near"
                elif howclose > 1 and howclose < 50:
                    NearOrFar = "Mid"
                elif howclose >= 50:
                    NearOrFar = "Far"
            
            
        else: # it was entering a shoreside zone that attracted Shroomie
            whichbehavior = random.randint(2,4)
            NearOrFar = "Far"
            
        PtDebugPrint("tldnShroomieBrain: whichbehavior = ",whichbehavior," NearOrFar = ",NearOrFar)


        whichspawnpoint = random.randint(1,5)


        if NearOrFar == "Near":
            code = "target = SpawnNear0" + str(whichspawnpoint) + ".sceneobject.getKey()"        
        elif NearOrFar == "Mid":
            code = "target = SpawnMid0" + str(whichspawnpoint) + ".sceneobject.getKey()"            
        elif NearOrFar == "Far":
            code = "target = SpawnFar0" + str(whichspawnpoint) + ".sceneobject.getKey()"
        PtDebugPrint("target code:", code)
        exec(code)
        ShroomieMaster.sceneobject.physics.warpObj(target)
        
        code = "respTrick0" + str(whichbehavior) + ".run(self.key)"
        #~ PtDebugPrint("code = ", code)
        exec(code)


        CurrentTime = PtGetDniTime()
        ageSDL["ShroomieTimeLastSeen"] = (CurrentTime,)

        ShroomieTotalTimesSeen = ageSDL["ShroomieTotalTimesSeen"][0]
        ShroomieTotalTimesSeen = ShroomieTotalTimesSeen + 1
        ageSDL["ShroomieTotalTimesSeen"] = (ShroomieTotalTimesSeen,)
        PtDebugPrint("tldnShroomieBrain: Shroomie has been seen", ShroomieTotalTimesSeen,"times.")
