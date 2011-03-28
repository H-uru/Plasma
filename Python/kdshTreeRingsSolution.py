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
Module: kdshTreeRingsSolution
Age: Kadish Tolesa
Date: March 2002
Author: Doug McBride
As SDL values are updated in the Kadish.sdl file, this file animates and updates the rings accordingly. 
Also checks for the solution.
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys
import string

# If there were another way to have done this, believe me, I would have. 
# In the mean time, the following 122 items are dedicated to the memory of Pete Gage

#Ring 01
OuterRing01_01 = ptAttribAnimation(1, "anim: OuterRing01_01")
OuterRing01_02 = ptAttribAnimation(2, "anim: OuterRing01_02")
OuterRing01_03 = ptAttribAnimation(3, "anim: OuterRing01_03")
OuterRing01_04 = ptAttribAnimation(4, "anim: OuterRing01_04")
OuterRing01_05 = ptAttribAnimation(5, "anim: OuterRing01_05")
OuterRing01_06 = ptAttribAnimation(6, "anim: OuterRing01_06")
OuterRing01_07 = ptAttribAnimation(7, "anim: OuterRing01_07")
OuterRing01_08 = ptAttribAnimation(8, "anim: OuterRing01_08")

MiddleRing01_01 = ptAttribAnimation(9, "anim: MiddleRing01_01")
MiddleRing01_02 = ptAttribAnimation(10, "anim: MiddleRing01_02")
MiddleRing01_03 = ptAttribAnimation(11, "anim: MiddleRing01_03")
MiddleRing01_04 = ptAttribAnimation(12, "anim: MiddleRing01_04")
MiddleRing01_05 = ptAttribAnimation(13, "anim: MiddleRing01_05")
MiddleRing01_06 = ptAttribAnimation(14, "anim: MiddleRing01_06")
MiddleRing01_07 = ptAttribAnimation(15, "anim: MiddleRing01_07")
MiddleRing01_08 = ptAttribAnimation(16, "anim: MiddleRing01_08")

InnerRing01_01 = ptAttribAnimation(17, "anim: InnerRing01_01")
InnerRing01_02 = ptAttribAnimation(18, "anim: InnerRing01_02")
InnerRing01_03 = ptAttribAnimation(19, "anim: InnerRing01_03")
InnerRing01_04 = ptAttribAnimation(20, "anim: InnerRing01_04")
InnerRing01_05 = ptAttribAnimation(21, "anim: InnerRing01_05")
InnerRing01_06 = ptAttribAnimation(22, "anim: InnerRing01_06")
InnerRing01_07 = ptAttribAnimation(23, "anim: InnerRing01_07")
InnerRing01_08 = ptAttribAnimation(24, "anim: InnerRing01_08")

#Ring 02
OuterRing02_01 = ptAttribAnimation(25, "anim: OuterRing02_01")
OuterRing02_02 = ptAttribAnimation(26, "anim: OuterRing02_02")
OuterRing02_03 = ptAttribAnimation(27, "anim: OuterRing02_03")
OuterRing02_04 = ptAttribAnimation(28, "anim: OuterRing02_04")
OuterRing02_05 = ptAttribAnimation(29,"anim: OuterRing02_05")
OuterRing02_06 = ptAttribAnimation(30, "anim: OuterRing02_06")
OuterRing02_07 = ptAttribAnimation(31, "anim: OuterRing02_07")
OuterRing02_08 = ptAttribAnimation(32, "anim: OuterRing02_08")

MiddleRing02_01 = ptAttribAnimation(33, "anim: MiddleRing02_01")
MiddleRing02_02 = ptAttribAnimation(34, "anim: MiddleRing02_02")
MiddleRing02_03 = ptAttribAnimation(35, "anim: MiddleRing02_03")
MiddleRing02_04 = ptAttribAnimation(36, "anim: MiddleRing02_04")
MiddleRing02_05 = ptAttribAnimation(37, "anim: MiddleRing02_05")
MiddleRing02_06 = ptAttribAnimation(38, "anim: MiddleRing02_06")
MiddleRing02_07 = ptAttribAnimation(39, "anim: MiddleRing02_07")
MiddleRing02_08 = ptAttribAnimation(40, "anim: MiddleRing02_08")

InnerRing02_01 = ptAttribAnimation(41, "anim: InnerRing02_01")
InnerRing02_02 = ptAttribAnimation(42, "anim: InnerRing02_02")
InnerRing02_03 = ptAttribAnimation(43, "anim: InnerRing02_03")
InnerRing02_04 = ptAttribAnimation(44, "anim: InnerRing02_04")
InnerRing02_05 = ptAttribAnimation(45, "anim: InnerRing02_05")
InnerRing02_06 = ptAttribAnimation(46, "anim: InnerRing02_06")
InnerRing02_07 = ptAttribAnimation(47, "anim: InnerRing02_07")
InnerRing02_08 = ptAttribAnimation(48, "anim: InnerRing02_08")


#Ring 03
OuterRing03_01 = ptAttribAnimation(49, "anim: OuterRing03_01")
OuterRing03_02 = ptAttribAnimation(50, "anim: OuterRing03_02")
OuterRing03_03 = ptAttribAnimation(51, "anim: OuterRing03_03")
OuterRing03_04 = ptAttribAnimation(52, "anim: OuterRing03_04")
OuterRing03_05 = ptAttribAnimation(53, "anim: OuterRing03_05")
OuterRing03_06 = ptAttribAnimation(54, "anim: OuterRing03_06")
OuterRing03_07 = ptAttribAnimation(55, "anim: OuterRing03_07")
OuterRing03_08 = ptAttribAnimation(56, "anim: OuterRing03_08")

MiddleRing03_01 = ptAttribAnimation(57, "anim: MiddleRing03_01")
MiddleRing03_02 = ptAttribAnimation(58, "anim: MiddleRing03_02")
MiddleRing03_03 = ptAttribAnimation(59, "anim: MiddleRing03_03")
MiddleRing03_04 = ptAttribAnimation(60, "anim: MiddleRing03_04")
MiddleRing03_05 = ptAttribAnimation(61, "anim: MiddleRing03_05")
MiddleRing03_06 = ptAttribAnimation(62, "anim: MiddleRing03_06")
MiddleRing03_07 = ptAttribAnimation(63, "anim: MiddleRing03_07")
MiddleRing03_08 = ptAttribAnimation(64, "anim: MiddleRing03_08")

InnerRing03_01 = ptAttribAnimation(65, "anim: InnerRing03_01")
InnerRing03_02 = ptAttribAnimation(66, "anim: InnerRing03_02")
InnerRing03_03 = ptAttribAnimation(67, "anim: InnerRing03_03")
InnerRing03_04 = ptAttribAnimation(68, "anim: InnerRing03_04")
InnerRing03_05 = ptAttribAnimation(69, "anim: InnerRing03_05")
InnerRing03_06 = ptAttribAnimation(70, "anim: InnerRing03_06")
InnerRing03_07 = ptAttribAnimation(71, "anim: InnerRing03_07")
InnerRing03_08 = ptAttribAnimation(72, "anim: InnerRing03_08")

#"Fake" Ring 1 as seen in the GUI when looking through Scope #2
GUIOuter01_01 = ptAttribAnimation(73, "anim: GUIOuter01_01")
GUIOuter01_02 = ptAttribAnimation(74, "anim: GUIOuter01_02")
GUIOuter01_03 = ptAttribAnimation(75, "anim: GUIOuter01_03")
GUIOuter01_04 = ptAttribAnimation(76, "anim: GUIOuter01_04")
GUIOuter01_05 = ptAttribAnimation(77, "anim: GUIOuter01_05")
GUIOuter01_06 = ptAttribAnimation(78, "anim: GUIOuter01_06")
GUIOuter01_07 = ptAttribAnimation(79, "anim: GUIOuter01_07")
GUIOuter01_08 = ptAttribAnimation(80, "anim: GUIOuter01_08")

GUIMiddle01_01 = ptAttribAnimation(81, "anim: GUIMiddle01_01")
GUIMiddle01_02 = ptAttribAnimation(82, "anim: GUIMiddle01_02")
GUIMiddle01_03 = ptAttribAnimation(83, "anim: GUIMiddle01_03")
GUIMiddle01_04 = ptAttribAnimation(84, "anim: GUIMiddle01_04")
GUIMiddle01_05 = ptAttribAnimation(85, "anim: GUIMiddle01_05")
GUIMiddle01_06 = ptAttribAnimation(86, "anim: GUIMiddle01_06")
GUIMiddle01_07 = ptAttribAnimation(87, "anim: GUIMiddle01_07")
GUIMiddle01_08 = ptAttribAnimation(88, "anim: GUIMiddle01_08")

GUIInner01_01 = ptAttribAnimation(89, "anim: GUIInner01_01")
GUIInner01_02 = ptAttribAnimation(90, "anim: GUIInner01_02")
GUIInner01_03 = ptAttribAnimation(91, "anim: GUIInner01_03")
GUIInner01_04 = ptAttribAnimation(92, "anim: GUIInner01_04")
GUIInner01_05 = ptAttribAnimation(93, "anim: GUIInner01_05")
GUIInner01_06 = ptAttribAnimation(94, "anim: GUIInner01_06")
GUIInner01_07 = ptAttribAnimation(95, "anim: GUIInner01_07")
GUIInner01_08 = ptAttribAnimation(96, "anim: GUIInner01_08")


#"Fake" Ring 2 as seen in the GUI when looking through Scope #3
GUIOuter02_01 = ptAttribAnimation(97, "anim: GUIOuter02_01")
GUIOuter02_02 = ptAttribAnimation(98, "anim: GUIOuter02_02")
GUIOuter02_03 = ptAttribAnimation(99, "anim: GUIOuter02_03")
GUIOuter02_04 = ptAttribAnimation(100, "anim: GUIOuter02_04")
GUIOuter02_05 = ptAttribAnimation(101, "anim: GUIOuter02_05")
GUIOuter02_06 = ptAttribAnimation(102, "anim: GUIOuter02_06")
GUIOuter02_07 = ptAttribAnimation(103, "anim: GUIOuter02_07")
GUIOuter02_08 = ptAttribAnimation(104, "anim: GUIOuter02_08")

GUIMiddle02_01 = ptAttribAnimation(105, "anim: GUIMiddle02_01")
GUIMiddle02_02 = ptAttribAnimation(106, "anim: GUIMiddle02_02")
GUIMiddle02_03 = ptAttribAnimation(107, "anim: GUIMiddle02_03")
GUIMiddle02_04 = ptAttribAnimation(108, "anim: GUIMiddle02_04")
GUIMiddle02_05 = ptAttribAnimation(109, "anim: GUIMiddle02_05")
GUIMiddle02_06 = ptAttribAnimation(110, "anim: GUIMiddle02_06")
GUIMiddle02_07 = ptAttribAnimation(111, "anim: GUIMiddle02_07")
GUIMiddle02_08 = ptAttribAnimation(112, "anim: GUIMiddle02_08")

GUIInner02_01 = ptAttribAnimation(113, "anim: GUIInner02_01")
GUIInner02_02 = ptAttribAnimation(114, "anim: GUIInner02_02")
GUIInner02_03 = ptAttribAnimation(115, "anim: GUIInner02_03")
GUIInner02_04 = ptAttribAnimation(116, "anim: GUIInner02_04")
GUIInner02_05 = ptAttribAnimation(117, "anim: GUIInner02_05")
GUIInner02_06 = ptAttribAnimation(118, "anim: GUIInner02_06")
GUIInner02_07 = ptAttribAnimation(119, "anim: GUIInner02_07")
GUIInner02_08 = ptAttribAnimation(120, "anim: GUIInner02_08")

actScope2 = ptAttribActivator(121, "Act: Scope2") 
actScope3 = ptAttribActivator(122, "Act: Scope3") 

# globals
ScopeNumber  = 2
Outerbearing = 1
Middlebearing = 1
Innerbearing = 1

StillSolved = false
kPatienceDelayToSolve = 2

class kdshTreeRingsSolution(ptModifier):
    "Standard telescope modifier class"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5233
        
        version = 6
        self.version = version
        print "__init__kdshTreeRingsSolution v.", version,".2"

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()        
        if ageSDL == None:
            print "kdshTreeRingsResp.OnFirstUpdate():\tERROR---missing age SDL (%s)" % varstring.value

        ageSDL.setNotify(self.key,"OuterRing01",0.0)
        ageSDL.setNotify(self.key,"MiddleRing01",0.0)
        ageSDL.setNotify(self.key,"InnerRing01",0.0)

        ageSDL.setNotify(self.key,"OuterRing02",0.0)
        ageSDL.setNotify(self.key,"MiddleRing02",0.0)
        ageSDL.setNotify(self.key,"InnerRing02",0.0)
  
        ageSDL.setNotify(self.key,"OuterRing03",0.0)
        ageSDL.setNotify(self.key,"MiddleRing03",0.0)
        ageSDL.setNotify(self.key,"InnerRing03",0.0)
    
        self.InitRings()

    def InitRings(self):
        ageSDL = PtGetAgeSDL()        
        
        OuterRing01 = ageSDL["OuterRing01"][0]
        MiddleRing01 = ageSDL["MiddleRing01"][0]
        InnerRing01 = ageSDL["InnerRing01"][0]
        OuterRing02 = ageSDL["OuterRing02"][0]
        MiddleRing02 = ageSDL["MiddleRing02"][0]
        InnerRing02 = ageSDL["InnerRing02"][0]
        OuterRing03 = ageSDL["OuterRing03"][0]
        MiddleRing03 = ageSDL["MiddleRing03"][0]
        InnerRing03 = ageSDL["InnerRing03"][0]

        print "kdshTreeRingSolution: When I got here:"
        #~ print "\tOuterRing01=",OuterRing01
        #~ print "\tMiddleRing01=",MiddleRing01
        #~ print "\tInnerRing01=",InnerRing01
        #~ print "\tOuterRing02=",OuterRing02
        #~ print "\tMiddleRing02=",MiddleRing02
        #~ print "\tInnerRing02=",InnerRing02
        #~ print "\tOuterRing03=",OuterRing03
        #~ print "\tMiddleRing03=",MiddleRing03
        #~ print "\tInnerRing03=",InnerRing03

        for i in ["Outer", "Middle", "Inner"]:
            for j in ["1","2","3"]:
                
                ffcode1 = "InitState = "+i + "Ring0" +j
                #~ print "ffcode1 = ", ffcode1
                
                exec ffcode1
                #~ print "InitState = ", InitState
                
                ffcode2 = i + "Ring0" + j + "_0" + str(InitState) + ".animation.skipToEnd()"
                #~ print "ffcode2 = ", ffcode2
                
                exec ffcode2
            
                #~ print "Fastforwarding: Set = ",j," Ring = ",i," Position = ",InitState
                print "\t",i,"Ring0",j," = ", InitState

        
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global StillSolved
        ageSDL = PtGetAgeSDL()             
        #~ PtDebugPrint("kdshTreeRingsSolution.OnSDLNotify():\t VARname:%s, playerID:%s, tag:%s" % (VARname,playerID,tag))

        StillSolved = false
        newbearing = ageSDL[VARname][0]
        #~ print "VARname = ", VARname, "newbear = ", newbearing
        
        code = VARname + "_0" + str(newbearing) + ".animation.play()"
        #~ print "code = ", code
        exec code # this runs the animation on the actual ring in the garden
        
        if "3" in VARname: 
            #~ print "TRS: Nothing to ff. VARname = ", VARname
            pass
        else:
            GUIcode = "GUI" + string.join(string.split(VARname, "Ring"), "") + "_0" + str(newbearing) + ".animation.play()"
            #~ print "GUIcode = ", GUIcode
            exec GUIcode # this runs the animation on the "fake" ring in front of the GUI

###
#Check to see if the Puzzle has been solved
###

        OuterRing01 = ageSDL["OuterRing01"][0]
        MiddleRing01 = ageSDL["MiddleRing01"][0]
        InnerRing01 = ageSDL["InnerRing01"][0]
        OuterRing02 = ageSDL["OuterRing02"][0]
        MiddleRing02 = ageSDL["MiddleRing02"][0]
        InnerRing02 = ageSDL["InnerRing02"][0]
        OuterRing03 = ageSDL["OuterRing03"][0]
        MiddleRing03 = ageSDL["MiddleRing03"][0]
        InnerRing03 = ageSDL["InnerRing03"][0]
            
        print "Current Scope Positions [Outer, Middle, Inner]:"
        print "\tRing #1: [", OuterRing01 ,", ",MiddleRing01,", ",InnerRing01," ]"
        print "\tRing #2: [", OuterRing02 ,", ",MiddleRing02,", ",InnerRing02," ]"
        print "\tRing #3: [", OuterRing03 ,", ",MiddleRing03,", ",InnerRing03," ]"
        
        if OuterRing01 ==   5 and\
            MiddleRing01 == 6 and\
            InnerRing01 ==  3 and\
            OuterRing02 ==  1 and\
            MiddleRing02 == 1 and\
            InnerRing02 ==  4 and\
            OuterRing03 == 4 and\
            MiddleRing03 == 3 and\
            InnerRing03 == 6:
                print "Tree Ring Puzzle solved. Opening Door."
                StillSolved = true
                PtAtTimeCallback(self.key,kPatienceDelayToSolve,1) # Put in this delay to avoid people "speed clicking" past solution and opening door. 

    def OnNotify(self,state,id,events):
        global ScopeNumber
        global Outerbearing
        global Middlebearing
        global Innerbearing
        
        
        if id == actScope2.id:
            #~ print "kdshTreeRingsSolution: Scope #2 occupied. Fast forwarding Scope 1 Rings in GUI"
            ScopeNumber = 2
        elif id == actScope3.id:
            #~ print "kdshTreeRingsSolution: Scope #3 occupied. Fast forwarding Scope 2 Rings in GUI"
            ScopeNumber = 3
        else:
            print "ERROR: Not sure who the notify came from."
            print "id = ", id
            return
            
        ageSDL = PtGetAgeSDL()        
        if ageSDL == None:
            print "kdshTreeRings.OnFirstUpdate():\tERROR---missing age SDL (%s)" % varstring.value

        Outerbearing = ageSDL["OuterRing0" + str(ScopeNumber-1)][0]
        Middlebearing = ageSDL["MiddleRing0" + str(ScopeNumber-1)][0]
        Innerbearing = ageSDL["InnerRing0" + str(ScopeNumber-1)][0]

        #~ print "Outerbearing = ", Outerbearing
        #~ print "Middlebearing = ", Middlebearing
        #~ print "Innerbearing = ", Innerbearing
        

        GUIcode = "GUIOuter0" + str(ScopeNumber-1) + "_0" + str(Outerbearing) + ".animation.skipToEnd()"
        #~ print "FF Outer code = ", GUIcode
        exec GUIcode 

        GUIcode = "GUIMiddle0" + str(ScopeNumber-1) + "_0" + str(Middlebearing) + ".animation.skipToEnd()"
        #~ print "FF Middle code = ", GUIcode
        exec GUIcode 

        GUIcode = "GUIInner0" + str(ScopeNumber-1) + "_0" + str(Innerbearing) + ".animation.skipToEnd()"
        #~ print "FF Inner code = ", GUIcode
        exec GUIcode 
        
        #~ GUIcode = "GUI" + string.join(string.split(VARname, "Ring"), "") + "_0" + str(newbearing) + ".animation.play()"
        #~ print "GUIcode = ", GUIcode
        #~ exec GUIcode # this runs the animation on the "fake" ring in front of the GUI        
        
        

    def OnTimer(self,timer):
        global StillSolved
        ageSDL = PtGetAgeSDL()        
        if timer == 1:
            if StillSolved:
                ageSDL.setTagString("TreeRingDoorClosed","fromOutside")                
                ageSDL["TreeRingDoorClosed"] = (0,)
            else:
                print "Patience, grasshopper."
            