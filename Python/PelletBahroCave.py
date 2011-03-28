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
Module: PelletBahroCave.py #----->Formerly BahroCave02.py
Age: PelletBahroCave #----->Formerly BahroCave02
Date: March 2004
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *
from xPsnlVaultSDL import *
import string
import time


# ---------
# max wiring
# ---------

symbolStates = ['State 1','State 2','State 3','State 4','State 5','State 6','State 7']

SDLGotPellet   = ptAttribString(1,"SDL: got pellet")
RespDropPellet   = ptAttribResponder(2,"resp: got pellet",['upper','lower'],netForce=1)
RespFadeInPellet   = ptAttribResponder(3,"resp: fade-in pellet",netForce=1)
RespPlayDud   = ptAttribResponder(4,"resp: pellet dud",netForce=1)
RespPlayBubbles   = ptAttribResponder(5,"resp: pellet bubbles",['Hi','Med','Low'],netForce=1)
RespPlaySteam   = ptAttribResponder(6,"resp: pellet steam",['Hi','Med','Low'],netForce=1)
RespPlayOrangeGlow   = ptAttribResponder(7,"resp: pellet orange glow",['Hi','Med','Low'],netForce=1)
RespPlayBoom   = ptAttribResponder(8,"resp: pellet explosion",['Hi','Med','Low'],netForce=1)
RespPlayWhiteGlow   = ptAttribResponder(9,"resp: pellet white glow",netForce=1)
RespSymbolOnN   = ptAttribResponder(10,"resp: N symbol on",statelist=symbolStates,netForce=1)
RespSymbolOffN   = ptAttribResponder(11,"resp: N symbol off",statelist=symbolStates,netForce=1)
RespSymbolOnE   = ptAttribResponder(12,"resp: E symbol on",statelist=symbolStates,netForce=1)
RespSymbolOffE   = ptAttribResponder(13,"resp: E symbol off",statelist=symbolStates,netForce=1)
RespSymbolOnS   = ptAttribResponder(14,"resp: S symbol on",statelist=symbolStates,netForce=1)
RespSymbolOffS   = ptAttribResponder(15,"resp: S symbol off",statelist=symbolStates,netForce=1)
RespSymbolOnW   = ptAttribResponder(16,"resp: W symbol on",statelist=symbolStates,netForce=1)
RespSymbolOffW   = ptAttribResponder(17,"resp: W symbol off",statelist=symbolStates,netForce=1)

respListSymbolsOn = [RespSymbolOnN,RespSymbolOnE,RespSymbolOnS,RespSymbolOnW]
respListSymbolsOff = [RespSymbolOffN,RespSymbolOffE,RespSymbolOffS,RespSymbolOffW]


# globals
gotPellet = 0
lowerCave = 0
sdlSolutions = []
chronSolutions = []
SymbolsOnSecs = 0.0

#contants
sdlSolutionNames = ["plltImagerSolutionN","plltImagerSolutionE","plltImagerSolutionS","plltImagerSolutionW"]


class PelletBahroCave(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8000
        self.version = 7


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global gotPellet
        global lowerCave
        global sdlSolutions
        global chronSolutions
        
        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(SDLGotPellet.value,1,1)
        ageSDL.sendToClients(SDLGotPellet.value)

        for sdl in sdlSolutionNames:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(sdl,1,1)
            ageSDL.sendToClients(sdl)
            ageSDL.setNotify(self.key,sdl,0.0)
            val = ageSDL[sdl][0]
            sdlSolutions.append(val)
        chronString = self.GetPelletCaveSolution()
        #print "found pellet cave solution: ",chronString
        try:
            chronString = chronString.split(",")
            for sol in chronString:
                chronSolutions.append(string.atoi(sol))
            print "found pellet cave solution: ",chronSolutions
            print "current sdl values for solution = ",sdlSolutions
            if self.sceneobject.isLocallyOwned():
                self.ShowSymbols()
        except:
            print "ERROR!  Couldn't get the solution information, symbols won't appear"

        linkmgr = ptNetLinkingMgr()
        link = linkmgr.getCurrAgeLink()
        spawnPoint = link.getSpawnPoint()

        spTitle = spawnPoint.getTitle()
        spName = spawnPoint.getName()
        
        if spName == "LinkInPointLower":
            lowerCave = 1
            avatar = 0
            try:
                avatar = PtGetLocalAvatar()
            except:
                print"failed to get local avatar"
                return
            avatar.avatar.registerForBehaviorNotify(self.key)
        else:
            lowerCave = 0
            vault = ptVault()
            entry = vault.findChronicleEntry("GotPellet")
            if type(entry) != type(None):
                entryValue = entry.chronicleGetValue()
                gotPellet = string.atoi(entryValue)
                if gotPellet != 0:
                    entry.chronicleSetValue("%d" % (0))
                    entry.save()
                    avatar = PtGetLocalAvatar()
                    avatar.avatar.registerForBehaviorNotify(self.key)
                else:
                    return
            else:
                return
            
            try:
                ageSDL = PtGetAgeSDL()
            except:
                print "PelletBahroCave.OnServerInitComplete():\tERROR---Cannot find the PelletBahroCave Age SDL"
                ageSDL[SDLGotPellet.value] = (0,)
        
            ageSDL.setNotify(self.key,SDLGotPellet.value,0.0)
        
            pelletSDL = ageSDL[SDLGotPellet.value][0]
            if pelletSDL != gotPellet:
                ageSDL[SDLGotPellet.value] = (gotPellet,)
        
            PtDebugPrint("PelletBahroCave:OnServerInitComplete:  SDL for pellet is now %d" % (gotPellet))


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global sdlSolutions

        if VARname in sdlSolutionNames:
            id = sdlSolutionNames.index(VARname)
            ageSDL = PtGetAgeSDL()
            sdlSolutions[id] = ageSDL[sdlSolutionNames[id]][0]
            print "PelletBahroCave.OnSDLNotify(): ",sdlSolutionNames[id]," now = ",sdlSolutions[id]


    def OnBehaviorNotify(self,type,id,state):
        global gotPellet
        global lowerCave
        
        PtDebugPrint("PelletBahroCave.OnBehaviorNotify(): %d" % (type))
        if type == PtBehaviorTypes.kBehaviorTypeLinkIn and state:
            if not lowerCave:
                if gotPellet != 0:
                    RespFadeInPellet.run(self.key)
                    cam = ptCamera()
                    cam.disableFirstPersonOverride()
                    cam.undoFirstPerson()
        if type == PtBehaviorTypes.kBehaviorTypeLinkIn and not state:
            if lowerCave:
                pass
                #self.IPruneDrops()
            avatar = PtGetLocalAvatar()
            avatar.avatar.unRegisterForBehaviorNotify(self.key)


    def OnNotify(self,state,id,events):
        global gotPellet
        global SymbolsOnSecs
        
        if (id == RespFadeInPellet.id and gotPellet):
            self.IDropUpper(gotPellet)
        
        if (id == RespDropPellet.id and gotPellet):
            print "PelletBahroCave.OnNotify: RespDropPellet callback, will now play FX"
            pellet = (gotPellet - 300)
            if pellet <= 0:
                if pellet <= -250:
                    RespPlaySteam.run(self.key,state="Hi")
                    RespPlayBubbles.run(self.key,state="Hi")
                elif pellet > -250 and pellet <= -150:
                    RespPlaySteam.run(self.key,state="Med")
                    RespPlayBubbles.run(self.key,state="Med")
                elif pellet > -150 and pellet <= -50:
                    RespPlaySteam.run(self.key,state="Low")
                    RespPlayBubbles.run(self.key,state="Low")
                elif pellet > -50 and pellet <= 0:
                    RespPlayDud.run(self.key)
            elif pellet > 0 and pellet <= 270:
                if pellet > 0 and pellet <= 74:
                    RespPlayOrangeGlow.run(self.key,state="Low")
                    SymbolsOnSecs = 3.0
                    PtAtTimeCallback(self.key,1.5,2)
                elif pellet > 74 and pellet <= 149:
                    RespPlayOrangeGlow.run(self.key,state="Med")
                    SymbolsOnSecs = 6.0
                    PtAtTimeCallback(self.key,1.2,2)
                elif pellet > 149 and pellet <= 209:
                    RespPlayOrangeGlow.run(self.key,state="Hi")
                    SymbolsOnSecs = 9.0
                    PtAtTimeCallback(self.key,0.9,2)
                elif pellet > 209 and pellet <= 270:
                    RespPlayWhiteGlow.run(self.key)
                    SymbolsOnSecs = 18.0
                    PtAtTimeCallback(self.key,0.5,2)
            elif pellet > 270:
                if pellet > 270 and pellet <= 295:
                    RespPlayBoom.run(self.key,state="Low")
                elif pellet > 295 and pellet <= 320:
                    RespPlayBoom.run(self.key,state="Med")
                elif pellet > 320:
                    RespPlayBoom.run(self.key,state="Hi")
            PtAtTimeCallback(self.key,0.5,1)


    def OnTimer(self,id):
        global SymbolsOnSecs
        if id == 1:
            cam = ptCamera()
            cam.enableFirstPersonOverride()
        elif id == 2:
            self.PlaySymbols(1)
            PtAtTimeCallback(self.key,SymbolsOnSecs,3)
            SymbolsOnSecs = 0
        elif id == 3:
            self.PlaySymbols(0)


    def IDropUpper(self,recipe):       
        print "in IDropUpper."
        RespDropPellet.run(self.key,state="upper")


    def GetPelletCaveSolution(self):
        ageVault = ptAgeVault()
        ageInfoNode = ageVault.getAgeInfo()
        ageInfoChildren = ageInfoNode.getChildNodeRefList()
        for ageInfoChildRef in ageInfoChildren:
            ageInfoChild = ageInfoChildRef.getChild()
            folder = ageInfoChild.upcastToFolderNode()
            if folder and folder.folderGetName() == "AgeData":
                print "Found age data folder"
                ageDataChildren = folder.getChildNodeRefList()
                for ageDataChildRef in ageDataChildren:
                    ageDataChild = ageDataChildRef.getChild()
                    chron = ageDataChild.upcastToChronicleNode()
                    if chron and chron.getName() == "PelletCaveSolution":
                        solution = chron.getValue()
                        return solution
                    else:
                        return 0


    def ShowSymbols(self):
        global sdlSolutions
        ageSDL = PtGetAgeSDL()
        n = 0
        for sdl in sdlSolutions:
            if sdlSolutions[n] != chronSolutions[n]:
                newVal = chronSolutions[n]
                ageSDL[sdlSolutionNames[n]] = (newVal,)
                sdlSolutions[n] = newVal
            n += 1
        print "SDL solutions list now = ",sdlSolutions


    def PlaySymbols(self,state):
        n = 0
        while n < len(sdlSolutions):
            symstate = symbolStates[sdlSolutions[n]]
            if state:
                respListSymbolsOn[n].run(self.key,state=symstate)
            else:
                respListSymbolsOff[n].run(self.key,state=symstate)
            n += 1

