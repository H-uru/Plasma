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
Module: City.py
Age: City
Date: October 2002
event manager hooks for the City
"""

from Plasma import *
from PlasmaTypes import *

IsPublic = 0
IsKadishGallery = 0

sdlS1FinaleBahro = [    "islmS1FinaleBahro","islmS1FinaleBahroCity1","islmS1FinaleBahroCity2",\
                            "islmS1FinaleBahroCity3","islmS1FinaleBahroCity4","islmS1FinaleBahroCity5",\
                            "islmS1FinaleBahroCity6"]
pagesS1FinaleBahro = [    "bahroFlyers_arch","bahroFlyers_city1","bahroFlyers_city2",\
                            "bahroFlyers_city3","bahroFlyers_city4","bahroFlyers_city5",\
                            "bahroFlyers_city6"]
#S1FinaleBahro = []


class city(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5026
        self.version = 1

        global IsPublic
        global IsKadishGallery

        parentname = None

        try:
            agevault = ptAgeVault()
            ageinfo = agevault.getAgeInfo()
            parent = ageinfo.getParentAgeLink()
            parentinfo = parent.getAgeInfo()
            parentname = parentinfo.getAgeFilename()
        except:
            pass
    
        if parentname == "Neighborhood":
            IsPublic = 1
            print "city.__init__(): city version = public"
        else:
            print "city.__init__(): city version = Yeesha"
        
        if not IsPublic:
            pass
            #return

        try:
            linkmgr = ptNetLinkingMgr()
            link = linkmgr.getCurrAgeLink()
            spawnPoint = link.getSpawnPoint()
            spTitle = spawnPoint.getTitle()
            spName = spawnPoint.getName()
        except:
            spTitle = "title unknown"
            spName = "spawn point unknown"

        print "city.__init__(): spTitle = ",spTitle
        print "city.__init__(): spName = ",spName

        ## NOT USING THIS FOR NOW - MAY NEED TO LOAD IN SPECIFIC PAGE(S) FOR FUTURE CITY AREAS...
#        if spTitle == "KadishGallery":
#            print "city.__init__(): we're linking into KadishGallery, only that page will be loaded"
#            IsKadishGallery = 1
#        else:
#            print "city.__init__(): we're NOT linking into KadishGallery, will load the entire city"
        
        pages = []

        # Add the common pages
        ## actually, we'll just set these to load automatically in the .age file
        #pages += ["KadishGallery""]
        
        # For the non-public age, add all the remaining pages
        if not IsKadishGallery:
            pages += ["canyon","cavetjunction","courtyard","ferry","greatstair","guildhall","harbor","HarborReflect"]
            pages += ["islmGreatZeroState","islmJCNote","islmNegilahnCreatureChartGUI","islmNickNote","islmPodMapGUI"]
            pages += ["islmWatsonLetterGUI","KahloPub","kahlopubtunnel","library","LibraryInterior"]
            pages += ["MuseumInteriorPage","palace","trailerCamPage"]
            pages += ["islmBahroShoutFerry","islmBahroShoutLibrary","islmBahroShoutPalace"]
            #pages += ["islmDRCStageState01","islmDRCStageState02","islmDRCTentTablePic","islmFanSoundRun"]
            #pages += ["islmLibBanners00Vis","islmLibBanners02Vis","islmLibBanners03Vis"]
            #pages += ["LibraryAhnonayVis","LibraryErcanaVis","LibraryGarrisonVis","LibraryKadishVis","LibraryTeledahnVis"]

            PtPageInNode(pages)


    def OnFirstUpdate(self):    
        pass


    def OnServerInitComplete(self):
#        if not self.sceneobject.isLocallyOwned():
#            return
        try:
            ageSDL = PtGetAgeSDL()
            n = 0
            for sdl in sdlS1FinaleBahro:
                ageSDL.setFlags(sdl,1,1)
                ageSDL.sendToClients(sdl)
                ageSDL.setNotify(self.key,sdl,0.0)
                val = ageSDL[sdl][0]
                if val:
                    self.ILoadS1FinaleBahro(n,1)
                n += 1
        except:
            print "ERROR!  Couldn't find all Bahro sdl, leaving default = 0"

#        ageSDL = PtGetAgeSDL()
#        
#        ageSDL.setFlags("islmKadishGalleryDoorVis",1,1)
#        ageSDL.sendToClients("islmKadishGalleryDoorVis")
#        ageSDL.setNotify(self.key,"islmKadishGalleryDoorVis",0.0)
#
#        boolDoorVis = ageSDL["islmKadishGalleryDoorVis"][0]
#        if boolDoorVis and not IsKadishGallery:
#            ageSDL["islmKadishGalleryDoorVis"] = (0,)
#        elif not boolDoorVis and IsKadishGallery:
#            ageSDL["islmKadishGalleryDoorVis"] = (1,)


    def Load(self):
        pass        


    def OnNotify(self,state,id,events):
        pass


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("city.OnSDLNotify():\t VARname: %s, SDLname: %s, tag: %s, value: %d" % (VARname,SDLname,tag,ageSDL[VARname][0]))

        if VARname in sdlS1FinaleBahro:
#            if not self.sceneobject.isLocallyOwned():
#                return
            id = sdlS1FinaleBahro.index(VARname)
            val = ageSDL[sdlS1FinaleBahro[id]][0]
            self.ILoadS1FinaleBahro(id,val)


    def ILoadS1FinaleBahro(self,bahro,state):
        print "city.ILoadS1FinaleBahro(): bahro = %d, load = %d" % (bahro,state)
#        if not self.sceneobject.isLocallyOwned():
#            return
        if state:
            PtPageInNode(pagesS1FinaleBahro[bahro])
        else:
            PtPageOutNode(pagesS1FinaleBahro[bahro])


    def OnBackdoorMsg(self, target, param):
        if target == "kadishgallerydoors":
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("islmKadishGalleryDoorVis",1,1)
            ageSDL.sendToClients("islmKadishGalleryDoorVis")
            ageSDL.setNotify(self.key,"islmKadishGalleryDoorVis",0.0)
            tmpdoors = ageSDL["islmKadishGalleryDoorVis"][0]
            if param == "on" or param == "1":
                if not tmpdoors:
                    ageSDL["islmKadishGalleryDoorVis"] = (1,)
            elif param == "off" or param == "0":
                if tmpdoors:
                    ageSDL["islmKadishGalleryDoorVis"] = (0,)
        elif target == "s1finale":
            if param == "on" or param == "1":
                n = 0
                for p in pagesS1FinaleBahro:
                    self.ILoadS1FinaleBahro(n,1)
                    n += 1
            elif param == "off" or param == "0":
                n = 0
                for p in pagesS1FinaleBahro:
                    self.ILoadS1FinaleBahro(n,0)
                    n += 1


