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
Module: ercaBakePellets.py
Age: Ercana
Date: December 2003
Author: Chris Doyle
toggles an age sdl bool only if another age sdl bool is true
"""

from Plasma import *
from PlasmaTypes import *
import string
import PlasmaControlKeys
import xEnum
from math import *


# ---------
# max wiring
# ---------

SDLFinishTime   = ptAttribString(1,"SDL: baking finish time")
RespOven1   = ptAttribResponder(2, "resp: oven 1 baking",['off','on'])
RespOven2   = ptAttribResponder(3, "resp: oven 2 baking",['off','on'])
RespOven3   = ptAttribResponder(4, "resp: oven 3 baking",['off','on'])
RespOven4   = ptAttribResponder(5, "resp: oven 4 baking",['off','on'])
SDLPellet1   = ptAttribString(6,"SDL: pellet 1")
SDLPellet2   = ptAttribString(7,"SDL: pellet 2")
SDLPellet3   = ptAttribString(8,"SDL: pellet 3")
SDLPellet4   = ptAttribString(9,"SDL: pellet 4")
SDLPellet5   = ptAttribString(10,"SDL: pellet 5")


# ---------
# globals
# ---------

TimeSDLs = ['ercaTimeSlider1','ercaTimeSlider2','ercaTimeSlider3','ercaTimeSlider4']
AmountSDLs = ['ercaAmountSlider1','ercaAmountSlider2','ercaAmountSlider3','ercaAmountSlider4']
TempSDLs = ['ercaTempSlider1','ercaTempSlider2','ercaTempSlider3','ercaTempSlider4']

time1 = 0
time2 = 0
time3 = 0
time4 = 0
amt1 = 0
amt2 = 0
amt3 = 0
amt4 = 0
temp1 = 0
temp2 = 0
temp3 = 0
temp4 = 0

byteFinishTime = 0
Oven1Start = 0
Oven2Start = 0
Oven3Start = 0
Oven4Start = 0
ovenStartList = []
Pellet1 = 0
Pellet2 = 0
Pellet3 = 0
Pellet4 = 0
Pellet5 = 0
Recipe = 0
RecipeSDL = 0
kTimeScale = 180  #  each slider notch is 3 minutes each   ## THIS IS THE FINAL SETTING
#kTimeScale = 2  #  each slider notch is 2 secs each         ## THIS IS FOR QA TESTING ONLY!
kTimeIncrement = 1
Oven1On = 0
Oven2On = 0
Oven3On = 0
Oven4On = 0
boolPelletMachine = 0


class ercaBakePellets(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 7031
        self.version = 6


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global byteFinishTime
        global Pellet1
        global Pellet2
        global Pellet3
        global Pellet4
        global Pellet5
        global boolPelletMachine
        
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(SDLFinishTime.value,1,1)
        ageSDL.sendToClients(SDLFinishTime.value)
        ageSDL.setFlags(SDLPellet1.value,1,1)
        ageSDL.sendToClients(SDLPellet1.value)
        ageSDL.setFlags(SDLPellet2.value,1,1)
        ageSDL.sendToClients(SDLPellet2.value)
        ageSDL.setFlags(SDLPellet3.value,1,1)
        ageSDL.sendToClients(SDLPellet3.value)
        ageSDL.setFlags(SDLPellet4.value,1,1)
        ageSDL.sendToClients(SDLPellet4.value)
        ageSDL.setFlags(SDLPellet5.value,1,1)
        ageSDL.sendToClients(SDLPellet5.value)
        
        try:
            ageSDL = PtGetAgeSDL()
        except:
            print "ercaBakePellet.OnServerInitComplete():\tERROR---Cannot find the Ercana Age SDL"
            if self.sceneobject.isLocallyOwned():
                ageSDL[SDLFinishTime.value] = (0,)
                ageSDL[SDLPellet1.value] = (0,)
                ageSDL[SDLPellet2.value] = (0,)
                ageSDL[SDLPellet3.value] = (0,)
                ageSDL[SDLPellet4.value] = (0,)
                ageSDL[SDLPellet5.value] = (0,)
        
        ageSDL.setNotify(self.key,SDLFinishTime.value,0.0)
        ageSDL.setNotify(self.key,SDLPellet1.value,0.0)
        ageSDL.setNotify(self.key,SDLPellet2.value,0.0)
        ageSDL.setNotify(self.key,SDLPellet3.value,0.0)
        ageSDL.setNotify(self.key,SDLPellet4.value,0.0)
        ageSDL.setNotify(self.key,SDLPellet5.value,0.0)
        
        byteFinishTime = ageSDL[SDLFinishTime.value][0]
        Pellet1 = ageSDL[SDLPellet1.value][0]
        Pellet2 = ageSDL[SDLPellet2.value][0]
        Pellet3 = ageSDL[SDLPellet3.value][0]
        Pellet4 = ageSDL[SDLPellet4.value][0]
        Pellet5 = ageSDL[SDLPellet5.value][0]
        
        ageSDL.setFlags("ercaPelletMachine",1,1)
        ageSDL.sendToClients("ercaPelletMachine")
        ageSDL.setNotify(self.key,"ercaPelletMachine",0.0)
        boolPelletMachine = ageSDL["ercaPelletMachine"][0]

        if byteFinishTime != 0:
            CurTime = PtGetDniTime()
            if CurTime > byteFinishTime:
                #self.IDoFormula()
                fastforward = 1
                self.IGetRecipe(fastforward)
            else:
                fastforward = 1
                self.IGetRecipe(fastforward)
        else:
            RespOven1.run(self.key,state="off",fastforward=1)
            RespOven2.run(self.key,state="off",fastforward=1)
            RespOven3.run(self.key,state="off",fastforward=1)
            RespOven4.run(self.key,state="off",fastforward=1)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global byteFinishTime
        global Pellet1
        global Pellet2
        global Pellet3
        global Pellet4
        global Pellet5
        global Oven1On
        global Oven2On
        global Oven3On
        global Oven4On
        global boolPelletMachine
        ageSDL = PtGetAgeSDL()
        
        if VARname == "ercaPelletMachine":
            boolPelletMachine = ageSDL["ercaPelletMachine"][0]
        if VARname == SDLFinishTime.value:
            byteFinishTime = ageSDL[SDLFinishTime.value][0]
            PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for ercaBakeFinishTime is now %d" % (byteFinishTime))
            if byteFinishTime == 1:
                print "ONLY CALL GetRecipe ONCE"
                fastforward = 0
                self.IGetRecipe(fastforward)
            elif byteFinishTime == 0:
                print "BYTEFINISHTIME set to O"
                if Oven1On:
                    RespOven1.run(self.key,state="off")
                    Oven1On = 0
                if Oven2On:
                    RespOven2.run(self.key,state="off")
                    Oven2On = 0
                if Oven3On:
                    RespOven3.run(self.key,state="off")
                    Oven3On = 0
                if Oven4On:
                    RespOven4.run(self.key,state="off")
                    Oven4On = 0
        if VARname == SDLPellet1.value:
            Pellet1 = ageSDL[SDLPellet1.value][0]
            PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet1 is now %d" % (Pellet1))
        if VARname == SDLPellet2.value:
            Pellet2 = ageSDL[SDLPellet2.value][0]
            PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet2 is now %d" % (Pellet2))
        if VARname == SDLPellet3.value:
            Pellet3 = ageSDL[SDLPellet3.value][0]
            PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet3 is now %d" % (Pellet3))
        if VARname == SDLPellet4.value:
            Pellet4 = ageSDL[SDLPellet4.value][0]
            PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet4 is now %d" % (Pellet4))
        if VARname == SDLPellet5.value:
            Pellet5 = ageSDL[SDLPellet5.value][0]
            PtDebugPrint("ercaBakePellets:OnSDLNotify:  SDL for Pellet5 is now %d" % (Pellet5))


    def OnNotify(self,state,id,events):
        global byteFinishTime
        ageSDL = PtGetAgeSDL()


    def IGetRecipe(self,fastforward):
        global time1
        global time2
        global time3
        global time4
        global amt1
        global amt2
        global amt3
        global amt4
        global temp1
        global temp2
        global temp3
        global temp4
        global byteFinishTime
        ageSDL = PtGetAgeSDL()
        
        time1 = ageSDL[TimeSDLs[0]][0]
        time2 = ageSDL[TimeSDLs[1]][0]
        time3 = ageSDL[TimeSDLs[2]][0]
        time4 = ageSDL[TimeSDLs[3]][0]
        amt1 = ageSDL[AmountSDLs[0]][0]
        amt2 = ageSDL[AmountSDLs[1]][0]
        amt3 = ageSDL[AmountSDLs[2]][0]
        amt4 = ageSDL[AmountSDLs[3]][0]
        temp1 = ageSDL[TempSDLs[0]][0]
        temp2 = ageSDL[TempSDLs[1]][0]
        temp3 = ageSDL[TempSDLs[2]][0]
        temp4 = ageSDL[TempSDLs[3]][0]
        
        PtDebugPrint("ercaBakePellets:IGetRecipe:  time1 = %d, time2 = %d, time3 = %d, time4 = %d" % (time1,time2,time3,time4))
        PtDebugPrint("ercaBakePellets:IGetRecipe:  amt1 = %d, amt2 = %d, amt3 = %d, amt4 = %d" % (amt1,amt2,amt3,amt4))
        PtDebugPrint("ercaBakePellets:IGetRecipe:  temp1 = %d, temp2 = %d, temp3 = %d, temp4 = %d" % (temp1,temp2,temp3,temp4))
        
        timeList = [time1,time2,time3,time4]
        timeList.sort()
        timeList.reverse()
        
        PtDebugPrint("Longest time is %d" % (timeList[0]))
        PtDebugPrint("Shortest time is %d" % (timeList[3]))
        
        if fastforward == 0:
            StartTime = PtGetDniTime()
            byteFinishTime = (StartTime + (timeList[0] * kTimeScale))
            if self.sceneobject.isLocallyOwned():
                ageSDL[SDLFinishTime.value] = (byteFinishTime,)
        
        self.ISetOvens(fastforward)


    def ISetOvens(self,fastforward):
        global Oven1Start
        global Oven2Start
        global Oven3Start
        global Oven4Start
        global ovenStartList
        
        Oven1Start = (byteFinishTime - (time1 * kTimeScale))
        Oven2Start = (byteFinishTime - (time2 * kTimeScale))
        Oven3Start = (byteFinishTime - (time3 * kTimeScale))
        Oven4Start = (byteFinishTime - (time4 * kTimeScale))
        
        ovenStartList = [Oven1Start,Oven2Start,Oven3Start,Oven4Start]
        
        print "IGetRecipe.Oven1Start = ",Oven1Start
        print "IGetRecipe.Oven2Start = ",Oven2Start
        print "IGetRecipe.Oven3Start = ",Oven3Start
        print "IGetRecipe.Oven4Start = ",Oven4Start
        
        self.IUpdateOvens(fastforward)


    def IUpdateOvens(self,fastforward):
        print "ercaBakePellets.IUpdateOvens()"
        global ovenStartList
        global Oven1On
        global Oven2On
        global Oven3On
        global Oven4On
        ageSDL = PtGetAgeSDL()
        
        CurTime = PtGetDniTime()
        if CurTime < byteFinishTime:
            #~ for start in ovenStartList:
                #~ if start == CurTime:
                    #~ PtDebugPrint("%s start baking." % (start))
            if fastforward == 0:
                if boolPelletMachine and ((CurTime+7) >= byteFinishTime) and self.sceneobject.isLocallyOwned():
                    print "7 seconds or less until baking is done, and the machine is open, so CLOSING..."
                    ageSDL[SDLPellet1.value] = (0,)
                    ageSDL[SDLPellet2.value] = (0,)
                    ageSDL[SDLPellet3.value] = (0,)
                    ageSDL[SDLPellet4.value] = (0,)
                    ageSDL[SDLPellet5.value] = (0,)
                    ageSDL["ercaPelletMachine"] = (0,)
                if CurTime == Oven1Start:
                    print "Start Oven1 bake"
                    RespOven1.run(self.key,state="on",fastforward=fastforward)
                    Oven1On = 1
                if CurTime == Oven2Start:
                    print "Start Oven2 bake"
                    RespOven2.run(self.key,state="on",fastforward=fastforward)
                    Oven2On = 1
                if CurTime == Oven3Start:
                    print "Start Oven3 bake"
                    RespOven3.run(self.key,state="on",fastforward=fastforward)
                    Oven3On = 1
                if CurTime == Oven4Start:
                    print "Start Oven4 bake"
                    RespOven4.run(self.key,state="on",fastforward=fastforward)
                    Oven4On = 1
            else:
                if CurTime > Oven1Start:
                    print "Start Oven1 bake"
                    RespOven1.run(self.key,state="on",fastforward=fastforward)
                    Oven1On = 1
                if CurTime > Oven2Start:
                    print "Start Oven2 bake"
                    RespOven2.run(self.key,state="on",fastforward=fastforward)
                    Oven2On = 1
                if CurTime > Oven3Start:
                    print "Start Oven3 bake"
                    RespOven3.run(self.key,state="on",fastforward=fastforward)
                    Oven3On = 1
                if CurTime > Oven4Start:
                    print "Start Oven4 bake"
                    RespOven4.run(self.key,state="on",fastforward=fastforward)
                    Oven4On = 1
                
            PtAtTimeCallback(self.key,kTimeIncrement,1)
        else:
            self.IDoFormula()


    def OnTimer(self,id):
        global Recipe
        global RecipeSDL
        ageSDL = PtGetAgeSDL()
        
        if id == 1:
            if byteFinishTime != 0:
                fastforward = 0
                self.IUpdateOvens(fastforward)
            else:
                print "OnTimer callback, but baking was either finished or cancelled.  Doing nothing."
        elif id == 2:
            if self.sceneobject.isLocallyOwned():
                print "Timer done, Pellets now created with Recipe of: ",Recipe
                ageSDL[SDLPellet1.value] = (RecipeSDL,)
                ageSDL[SDLPellet2.value] = (RecipeSDL,)
                ageSDL[SDLPellet3.value] = (RecipeSDL,)
                ageSDL[SDLPellet4.value] = (RecipeSDL,)
                ageSDL[SDLPellet5.value] = (RecipeSDL,)


    def IDoFormula(self,backdoor=0):
        if not backdoor:
            print "Baking done.  IDoFormula called."
        global time1
        global time2
        global time3
        global time4
        global amt1
        global amt2
        global amt3
        global amt4
        global temp1
        global temp2
        global temp3
        global temp4
        global Pellet1
        global Pellet2
        global Pellet3
        global Pellet4
        global Pellet5
        global Recipe
        global RecipeSDL
        ageSDL = PtGetAgeSDL()
        
        pelletList = [Pellet1,Pellet2,Pellet3,Pellet4,Pellet5]
        testVal = 0
        for pellet in pelletList:
            if pellet > 0:
                testVal = 1
                break
        
        if testVal and self.sceneobject.isLocallyOwned() and not backdoor:
            ageSDL[SDLPellet1.value] = (0,)
            ageSDL[SDLPellet2.value] = (0,)
            ageSDL[SDLPellet3.value] = (0,)
            ageSDL[SDLPellet4.value] = (0,)
            ageSDL[SDLPellet5.value] = (0,)
        
        z = 0
        
        # Oven1 adjustments
        time1_val = (time1 * 0.025)
        time1_opt = 2.1
        amt1_val = (amt1 * 0.1)
        amt1_opt = 5.8
        temp1_val = (temp1 * 0.02)
        temp1_opt = 1.75
        
        if time1_val > time1_opt:
            time1_val = (time1_val - (time1_val - time1_opt))
        if amt1_val > amt1_opt:
            amt1_val = (amt1_val - (amt1_val - amt1_opt))
        if temp1_val > temp1_opt:
            temp1_val = (temp1_val - (temp1_val - temp1_opt))
        
        # Oven2 adjustments
        time2_val = (time2 * 0.028)
        time2_opt = 2.0
        amt2_val = (amt2 * 0.1)
        amt2_opt = 7.4
        temp2_val = (temp2 * 0.017)
        temp2_opt = 1.1
        
        if time2_val > time2_opt:
            time2_val = (time2_val - (time2_val - time2_opt))
        if amt2_val > amt2_opt:
            amt2_val = (amt2_val - (amt2_val - amt2_opt))
        if temp2_val > temp2_opt:
            temp2_val = (temp2_val - (temp2_val - temp2_opt))
        
        # Oven3 adjustments
        time3_val = (time3 * 0.035)
        time3_opt = 3.0
        amt3_val = (amt3 * 0.1)
        amt3_opt = 6.0
        temp3_val = (temp3 * 0.0126)
        temp3_opt = 0.9
        
        if time3_val > time3_opt:
            time3_val = (time3_val - (time3_val - time3_opt))
        if amt3_val > amt3_opt:
            amt3_val = (amt3_val - (amt3_val - amt3_opt))
        if temp3_val > temp3_opt:
            temp3_val = (temp3_val - (temp3_val - temp3_opt))
        
        # Oven4 adjustments
        time4_val = (time4 * 0.028)
        time4_opt = 2.5
        amt4_val = (amt4 * 0.1)
        amt4_opt = 8.2
        temp4_val = (temp4 * 0.04)
        temp4_opt = 3.5
        
        if time4_val > time4_opt:
            time4_val = (time4_val - (time4_val - time4_opt))
        if amt4_val > amt4_opt:
            amt4_val = (amt4_val - (amt4_val - amt4_opt))
        if temp4_val > temp4_opt:
            temp4_val = (temp4_val - (temp4_val - temp4_opt))
        
        # Formulas
        z1 = (0.15 * amt1_val * 0.75 * time1_val * sin(time1_val) * sin(time1_val * (temp1_val)))
        z2 = (0.081 * amt2_val * time2_val * temp2_val * sin(0.7 * time2_val) * sin(1.38 * time2_val * temp2_val))
        z3 = (0.07425 * amt3_val * time3_val * temp3_val * sin(0.6 * time3_val) * sin(1.44 * time3_val * temp3_val))
        if (time4_val * temp4_val) > 3.1:
            z4 = 0
        else:
            z4 = (0.0864 * amt4_val * time4_val * temp4_val * sin(0.7 * time4_val) * sin(1.38 * time4_val * temp4_val))
        
        # Final recipe
        z = z1 + z2 + z3 + z4
        
        if backdoor:
            print "time1_val = ",time1_val
            print "amt1_val = ",amt1_val
            print "temp1_val = ",temp1_val,"\n"
            print "time2_val = ",time2_val
            print "amt2_val = ",amt2_val
            print "temp2_val = ",temp2_val,"\n"
            print "time3_val = ",time3_val
            print "amt3_val = ",amt3_val
            print "temp3_val = ",temp3_val,"\n"
            print "time4_val = ",time4_val
            print "amt4_val = ",amt4_val
            print "temp4_val = ",temp4_val,"\n"
            print "z1 = ",z1
            print "z2 = ",z2
            print "z3 = ",z3
            print "z4 = ",z4
            print "z = ",z,"\n"
            recipetest = int(round(100 * z))
            print "recipe test = ",recipetest
            return

        Recipe = int(round(100 * z))
        
#        if Recipe == 0:
#            Recipe = 1
            
        print "Pellet recipe = ",Recipe
        
        RecipeSDL = Recipe + 300
        
        if RecipeSDL < 1:
            RecipeSDL = 1
        
        print "Recipe SDL converted to ",RecipeSDL
        
        if self.sceneobject.isLocallyOwned():
            ageSDL[SDLFinishTime.value] = (0,)
        
        PtAtTimeCallback(self.key,0.1,2)


    def OnBackdoorMsg(self,target,param):
        global time1
        global time2
        global time3
        global time4
        global amt1
        global amt2
        global amt3
        global amt4
        global temp1
        global temp2
        global temp3
        global temp4
        if target == "recipe":
            ageSDL = PtGetAgeSDL()
            time1 = ageSDL[TimeSDLs[0]][0]
            time2 = ageSDL[TimeSDLs[1]][0]
            time3 = ageSDL[TimeSDLs[2]][0]
            time4 = ageSDL[TimeSDLs[3]][0]
            amt1 = ageSDL[AmountSDLs[0]][0]
            amt2 = ageSDL[AmountSDLs[1]][0]
            amt3 = ageSDL[AmountSDLs[2]][0]
            amt4 = ageSDL[AmountSDLs[3]][0]
            temp1 = ageSDL[TempSDLs[0]][0]
            temp2 = ageSDL[TempSDLs[1]][0]
            temp3 = ageSDL[TempSDLs[2]][0]
            temp4 = ageSDL[TempSDLs[3]][0]
            print "RECIPE TEST:\n"
            PtDebugPrint("oven1 = %d, %d, %d" % (time1,amt1,temp1))
            PtDebugPrint("oven2 = %d, %d, %d" % (time2,amt2,temp2))
            PtDebugPrint("oven3 = %d, %d, %d" % (time3,amt3,temp3))
            PtDebugPrint("oven4 = %d, %d, %d\n" % (time4,amt4,temp4))
            #PtDebugPrint("time1 = %d, time2 = %d, time3 = %d, time4 = %d" % (time1,time2,time3,time4))
            #PtDebugPrint("amt1 = %d, amt2 = %d, amt3 = %d, amt4 = %d" % (amt1,amt2,amt3,amt4))
            #PtDebugPrint("temp1 = %d, temp2 = %d, temp3 = %d, temp4 = %d" % (temp1,temp2,temp3,temp4))
            self.IDoFormula(1)

