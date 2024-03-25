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
# Include Plasma code
from Plasma import *
from PlasmaTypes import *

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################

climbTrigger = ptAttribActivator(1,"the trigger to climb")
climbBehavior = ptAttribBehavior(2,"climb behavior")

descendTrigger = ptAttribActivator(3,"the trigger to descend")
descendBehavior = ptAttribBehavior(4,"descend behavior")

climbTopReset = ptAttribActivator(5,"clear region at top")
climbBottomReset = ptAttribActivator(6,"clear region at bottom")

climbSoundAnim = ptAttribAnimation(7, "climb up sound anim")
descendSoundAnim = ptAttribAnimation(8,"climb down sound anim")

##############################################################
# grsn1stFloorClimb    
##############################################################
class grsn1stFloorClimb(ptResponder):
    "subworld transition test"
   
    # constants
    
    def __init__(self):
        "construction"
        PtDebugPrint("grsn1stFloorClimb::init begin")
        ptResponder.__init__(self)
        self.id = 50112
        self.version = 4
        PtDebugPrint("grsn1stFloorClimb::init end")        

    def OnFirstUpdate(self):
        self.SDL.setDefault("intSDLClimber",(-1,))
        self.SDL.setDefault("intSDLDescender",(-1,))
        climbTopReset.enable()
        climbBottomReset.enable()
        climbTrigger.enable()
        descendTrigger.enable()
        
    def OnNotify(self,state,id,events):
         PtDebugPrint("********************************")
         for event in events:

             PtDebugPrint("event[0] " + repr(event[0]))
             PtDebugPrint("event[1] " + repr(event[1]))
             PtDebugPrint("event[2] " + repr(event[2]))
             
             climber = PtFindAvatar(events)
             climberID = PtGetClientIDFromAvatarKey(climber.getKey())
             PtDebugPrint("climber ID = " + repr(climberID))
             currentClimber = self.SDL["intSDLClimber"][0]
             currentDescender = self.SDL["intSDLDescender"][0]
             PtDebugPrint("current climber = " + repr(currentClimber))
             PtDebugPrint("current descender = " + repr(currentClimber))
             
             # execute climb behaviors
                                                    
             if id == climbBehavior.id:
                 if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage:
                     climbSoundAnim.animation.play()
                     PtDebugPrint("starting to climb")   
                     return
                 if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                     PtDebugPrint("finshed climb")
                     return

             if id == descendBehavior.id:
                 if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage:
                     descendSoundAnim.animation.play()
                     PtDebugPrint("starting to descend")   
                     return
                 if event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage:
                     # temporary hack here
                     climber.physics.warp(ptPoint3(116.787,-478.983,-263.936))
                     # end temporary hack
                     PtDebugPrint("finshed descent")
                     return
                 
             # initiate climbing or descending
             
             if id == climbTrigger.id and event[0] == 1 and event[1] == 1: #someone is entering
                 if self.SDL["intSDLClimber"][0] == -1 and self.SDL["intSDLDescender"][0] == -1: #nobody currently climbing up/down
                     PtDebugPrint("initiated climbing avatar # "+ repr(climberID))
                     self.SDL["intSDLClimber"]=(climberID,)
                     climbBehavior.run(climber)
                 return

             if id == descendTrigger.id and event[0] == 1 and event[1] == 1: #someone is entering
                 if self.SDL["intSDLClimber"][0] == -1 and self.SDL["intSDLDescender"][0] == -1: #no one currently climbing / descending    
                     PtDebugPrint("initiated descent")
                     self.SDL["intSDLDescender"]=(climberID,)

                     test = self.SDL["intSDLDescender"][0]
                     PtDebugPrint("just set descender to " + repr(test))

                     descendBehavior.run(climber)
                 return

             # reset the climb regions
             
             if id == climbTopReset.id and event[0] == 1 and event[1] == 0: #someone is exiting
                 if self.SDL["intSDLClimber"][0] == climberID and self.SDL["intSDLDescender"][0] == -1: #it's our recent climber    
                     PtDebugPrint("reset climb / descend regions")
                     self.SDL["intSDLClimber"]=(-1,)
                 return

             if id == climbBottomReset.id and event[0] == 1 and event[1] == 0: #someone is exiting
                 if self.SDL["intSDLClimber"][0] == -1 and self.SDL["intSDLDescender"][0] == climberID: #it's our recent descender    
                     PtDebugPrint("reset climb / descend regions")
                     self.SDL["intSDLDescender"]=(-1,)
                 return

