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
Module: grsnPrisonRandomItems  
Age: Garrison Prison
Date: February 2002
Randomizes contents of the Garrison Prison Cell
"""

from Plasma import *
from PlasmaTypes import *
import whrandom

#globals
kMinNumItems = 1
kMaxNumItems = 10
kChanceOfYP = 0.5 # 0.0 = no chance. 1.0 = 100% chance

AllItems = [
    "grsnPrisonBones01vis", 
    "grsnPrisonBones02vis",
    "grsnPrisonBowls01vis",
    "grsnPrisonBowls02vis",
    "grsnPrisonBowls03vis",
    "grsnPrisonBowls04vis",
    "grsnPrisonBowls05vis",
    "grsnPrisonChains01vis", 
    "grsnPrisonChains02vis",
    "grsnPrisonDirt01vis",
    "grsnPrisonDirt02vis",
    "grsnPrisonMattress01vis",
    "grsnPrisonMattress02vis",
    "grsnPrisonWindow01vis",
    "grsnPrisonWindow02vis"
    ]
    

class grsnPrisonRandomSDLItems(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5205

        version = 2
        self.version = version
        print "__init__grsnPrisonRandomItems v.", version

    def OnServerInitComplete(self):
        global AllItems
        
        return
        
        ageSDL = PtGetAgeSDL()
        
        # pick items to display
        whrandom.seed()

        ItemsToPick = whrandom.randint(kMinNumItems, kMaxNumItems)
        AlreadyPicked = [ ]

        while len(AlreadyPicked) < ItemsToPick:
            randitem = whrandom.choice(AllItems)
            if randitem not in AlreadyPicked:
                AlreadyPicked.append (randitem)
    
        for item in AlreadyPicked:
            if (item == "grsnPrisonBones01vis"):
                ageSDL["grsnPrisonBones01vis"]=(1,) 
                
            if (item == "grsnPrisonBones02vis"):
                ageSDL["grsnPrisonBones02vis"]=(1,)
                
            if (item == "grsnPrisonBowls01vis"):
                ageSDL["grsnPrisonBowls01vis"]=(1,)
                
            if (item == "grsnPrisonBowls02vis"):
                ageSDL["grsnPrisonBowls02vis"]=(1,)
                
            if (item == "grsnPrisonBowls03vis"):
                ageSDL["grsnPrisonBowls03vis"]=(1,)
                
            if (item == "grsnPrisonBowls04vis"):
                ageSDL["grsnPrisonBowls04vis"]=(1,)
                
            if (item == "grsnPrisonBowls05vis"):
                ageSDL["grsnPrisonBowls05vis"]=(1,)
                
            if (item == "grsnPrisonChains01vis"):
                ageSDL["grsnPrisonChains01vis"]=(1,)
                
            if (item == "grsnPrisonChains02vis"):
                ageSDL["grsnPrisonChains02vis"]=(1,)
                
            if (item == "grsnPrisonDirt01vis"):
                ageSDL["grsnPrisonDirt01vis"]=(1,)
                
            if (item == "grsnPrisonDirt02vis"):
                ageSDL["grsnPrisonDirt02vis"]=(1,)
                
            if (item == "grsnPrisonMattress01vis"):
                ageSDL["grsnPrisonMattress01vis"]=(1,)
                
            if (item == "grsnPrisonMattress02vis"):
                ageSDL["grsnPrisonMattress02vis"]=(1,)
                
            if (item == "grsnPrisonWindow01vis"):
                ageSDL["grsnPrisonWindow01vis"]=(1,)
                
            if (item == "grsnPrisonWindow02vis"):
                ageSDL["grsnPrisonWindow02vis"]=(1,)
            
        self.IManageYeeshaPage()

    def IManageYeeshaPage(self):
        vault = ptVault()
                

        if type(vault) != type(None): #is the Vault online?
            entry = vault.findChronicleEntry("VisitedGrsnPrison")
            
            if type(entry) == type(None): 
                vault.addChronicleEntry("VisitedGrsnPrison",1,"yes")
                PtDebugPrint ("grsnPrisonRandomItems: This is your first visit to the Prison. Updated Chronicle.")
                
            else:
                PtDebugPrint ("grsnPrisonRandomItems: You've been to the Prison before.")
                
                chance = whrandom.random()
                ageSDL = PtGetAgeSDL()
                if chance > (1-kChanceOfYP):
                    ageSDL["grsnYeeshaPage02Vis"] = (1,)
                    PtDebugPrint ("grsnPrisonRandomItems: A YP is here.")
                else:
                    ageSDL["grsnYeeshaPage02Vis"] = (0,)
                    PtDebugPrint ("grsnPrisonRandomItems: A YP is NOT here.")
                
                ageSDL.sendToClients("grsnYeeshaPage02Vis")
                        
        else:
            PtDebugPrint("grsnPrisonRandomItems: Error trying to access the Chronicle." )


