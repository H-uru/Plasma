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
Module: Cleft.py
Age: Cleft
Date: February 2003
Event Manager hooks for Cleft
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *


#rgnSnsrFissureDrop = ptAttribActivator(1, "rgn snsr: fissure drop spawn")
respFissureDropStart = ptAttribResponder(1,"resp: fissure drop start")
respFissureDropMain = ptAttribResponder(2,"resp: fissure drop main")

loadTomahna = 0
loadZandi = 0
loadBook = 0
fissureDrop = 0

#kIntroPlayedChronicle = "IntroPlayed"


class Cleft(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5209
        self.version = 22

        #var used to load in Cleft/Tomahna specific stuff based on chronicle vals
        global loadTomahna
        global loadZandi
        global loadBook
        
        loadTomahna = 0
        loadZandi = 0
        loadBook = 0

        #checks chronicle entries, if don't exist or is set to no,
        #then decides if Tomahna or Zandi should be paged in

        vault = ptVault()
        entryCleft = vault.findChronicleEntry("CleftSolved")
        if type(entryCleft) != type(None):
            entryCleftValue = entryCleft.chronicleGetValue()
            if entryCleftValue != "yes":
                loadZandi = 1
                loadBook = 1
        elif type(entryCleft) == type(None):
            loadZandi = 1
            loadBook = 1

        vault = ptVault()
        entryTomahna = vault.findChronicleEntry("TomahnaLoad")
        if type(entryTomahna) != type(None):
            entryTomahnaValue = entryTomahna.chronicleGetValue()
            if entryTomahnaValue == "yes":
                loadTomahna = 1
                if loadZandi:
                    loadZandi = 0
                if loadBook:
                    loadBook = 0

        pages = []

        # Add the age specific pages
        if loadTomahna:
            pages += ["Cleft","tmnaDesert","MaleShortIdle","FemaleShortIdle","YeeshaFinalEncounter","FemaleTurnRight180","MaleTurnRight180","clftSndLogTracks","clftAtrusGoggles"]
        else:
            pages += ["Desert","Cleft","FemaleCleftDropIn","MaleCleftDropIn","clftJCsDesert","clftJCsChasm"]
        if loadZandi:
            pages += ["clftZandiVis","ZandiCrossLegs","ZandiDirections","ZandiDirections01","ZandiDirections02","ZandiDirections03"]
            pages += ["ZandiIdle","ZandiRubNose","ZandiScratchHead","ZandiTurnPage","ZandiAllFace","ZandiOpen01Face"]
            pages += ["ZandiOpen02Face","ZandiRand01Face","ZandiRand02Face","ZandiRand03Face","ZandiRand04Face","ZandiRand05Face"]
            pages += ["ZandiRes01aFace","ZandiRes01bFace","ZandiRes02aFace","ZandiRes02bFace","ZandiRes03aFace","ZandiRes03bFace"]
            pages += ["ZandiJC01aFace","ZandiJC01bFace","ZandiJC02aFace","ZandiJC02bFace","ZandiJC03aFace","ZandiJC03bFace"]
            pages += ["ZandiJC04aFace","ZandiJC04bFace","ZandiJC05aFace","ZandiJC05bFace","ZandiJC06aFace","ZandiJC06bFace"]
            pages += ["ZandiJC07aFace","ZandiJC07bFace"]
        else:
            print "Zandi seems to have stepped away from the Airstream. Hmmm..."
        if loadBook:
            pages += ["clftYeeshaBookVis","FemaleGetPersonalBook","MaleGetPersonalBook"]
        else:
            print "Zandi seems to have stepped away from the Airstream. Hmmm..."

        # Put in all the common pages
        pages += ["BookRoom","clftAtrusNote"]
        pages += ["FemaleClimbOffTreeLadder","FemaleGetOnTreeLadder","FemaleWindmillLockedCCW","FemaleWindmillLockedCW","FemaleWindmillStart"]
        pages += ["MaleClimbOffTreeLadder","MaleGetOnTreeLadder","MaleWindmillLockedCCW","MaleWindmillLockedCW","MaleWindmillStart"]
        pages += ["YeeshaVisionBlocked","YeeshaFinalVision"]

        PtPageInNode(pages)

        if loadTomahna:
            #now that Tomahna pages have loaded, reset its chronicle value back to no,
            #so subsequent linking will default to regular Cleft instead of Tomahna,
            #unless a Tomahna link is used, of course...
            entryTomahna.chronicleSetValue("no")
            entryTomahna.save()

        pass


    def OnFirstUpdate(self):
        pass
        #~ # test for first time to play the intro movie
        #~ vault = ptVault()
        #~ entry = vault.findChronicleEntry(kIntroPlayedChronicle)
        #~ if type(entry) != type(None):
            #~ # already played intro sometime in the past... just let 'em play
            #~ PtSendKIMessage(kEnableKIandBB,0)
        #~ else:
            #~ # make sure the KI and blackbar is still diabled
            #~ PtSendKIMessage(kDisableKIandBB,0)
            #~ # It's the first time... start the intro movie, just by loading the movie dialog
            #~ PtLoadDialog("IntroMovieGUI")


    def OnServerInitComplete(self):
        global loadTomahna
        global fissureDrop
        
        ageSDL = PtGetAgeSDL()
        
        # sets Tomahna SDL based on what is being loaded (thanks to chronicle val)
        # also settings previously contained in .fni files
        if loadTomahna:
            SDLVarName = "clftTomahnaActive"
            ageSDL[SDLVarName] = (1,)
            PtDebugPrint("Cleft.OnServerInitComplete: loadTomahna is 1, setting clftTomahnaActive SDL to 1")
            #PtFogSetDefLinear(start, end, density)
            PtFogSetDefLinear(0,0,0)
            PtSetClearColor(.4,.4,.5)

            SDLVarSceneBahro = "clftSceneBahroUnseen"
            boolSceneBahro = ageSDL[SDLVarSceneBahro][0]
            if boolSceneBahro:
                PtDebugPrint("Cleft.OnServerInitComplete: SDL says bahro hasn't played yet, paging in SceneBahro stuff...")
                PtPageInNode("clftSceneBahro")
            else:
                PtDebugPrint("Cleft.OnServerInitComplete: SDL says SceneBahro already played, will NOT page in")
                
            ageSDL.setNotify(self.key,SDLVarSceneBahro,0.0)
            
            SDLVarSceneYeesha = "clftSceneYeeshaUnseen"
            boolSceneYeesha = ageSDL[SDLVarSceneYeesha][0]
            if boolSceneYeesha:
                #PtDebugPrint("Cleft.OnServerInitComplete: SDL says Yeesha hasn't played yet, paging in SceneYeesha stuff...")
                #PtPageInNode("clftSceneYeesha")
                SDLVarOfficeDoor = "clftOfficeDoorClosed"
                boolOfficeDoor = ageSDL[SDLVarOfficeDoor][0]
                if boolOfficeDoor:
                    PtDebugPrint("Cleft.OnServerInitComplete: SDL says Yeesha will play and office door is shut, will open it")
                    ageSDL[SDLVarOfficeDoor] = (0,)
            else:
                PtDebugPrint("Cleft.OnServerInitComplete: SDL says SceneYeesha already played, will NOT page in")

        else:
            SDLVarName = "clftTomahnaActive"
            ageSDL[SDLVarName] = (0,)
            PtDebugPrint("Cleft.OnServerInitComplete: loadTomahna is 0, setting clftTomahnaActive SDL set to 0")
            PtFogSetDefLinear(0,0,0)
            PtSetClearColor(0,0,0)
        
        linkmgr = ptNetLinkingMgr()
        link = linkmgr.getCurrAgeLink()
        spawnPoint = link.getSpawnPoint()

        spTitle = spawnPoint.getTitle()
        spName = spawnPoint.getName()
        
        if spName == "LinkInPointFissureDrop":
            fissureDrop = 1
            #avatar.physics.suppress(false)
            avatar = 0
            try:
                avatar = PtGetLocalAvatar()
            except:
                print"failed to get local avatar"
                return
            avatar.avatar.registerForBehaviorNotify(self.key)
            cam = ptCamera()
            cam.disableFirstPersonOverride()
            cam.undoFirstPerson()
            PtDisableMovementKeys()
            PtSendKIMessage(kDisableEntireYeeshaBook,0)
            respFissureDropStart.run(self.key,avatar=PtGetLocalAvatar())


    def Load(self):        
        
        ageSDL = PtGetAgeSDL()

        # If both Kitchen and Office Doors are closed when linking into the age, this will open the Kitchen door.
        #  It prevents Player from being locked out of Kitchen/Office if both doors were left shut when Player was last there...
        
        SDLVarKitchenDoor = "clftKitchenDoorClosed"
        SDLVarOfficeDoor = "clftOfficeDoorClosed"
        
        boolKitchenDoor = ageSDL[SDLVarKitchenDoor][0]
        boolOfficeDoor = ageSDL[SDLVarOfficeDoor][0]
        
        if boolKitchenDoor and boolOfficeDoor:
            PtDebugPrint("Cleft.OnLoad: both Kitchen and Office doors are closed... setting Kitchen door SDL to open")
            ageSDL[SDLVarKitchenDoor] = (0,)
        else:
            PtDebugPrint("Cleft.OnLoad: either Kitchen and/or Office door is already open... leaving Kitchen door alone")

        pass


    def OnNotify(self,state,id,events):
        global fissureDrop

        if (id == respFissureDropMain.id):
            print "FISSUREDROP.OnNotify:  respFissureDropMain.id callback"
            if fissureDrop:
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                fissureDrop = 0
                avatar = PtGetLocalAvatar()
                avatar.avatar.unRegisterForBehaviorNotify(self.key)
                PtEnableMovementKeys()
                PtSendKIMessage(kEnableEntireYeeshaBook,0)


    def OnBehaviorNotify(self,type,id,state):
        global fissureDrop
   
        #PtDebugPrint("Cleft.OnBehaviorNotify(): %d" % (type))
        if type == PtBehaviorTypes.kBehaviorTypeLinkIn and not state:
            print "FISSUREDROP.OnBehaviorNotify: fissureDrop = %d" % (fissureDrop)
            if fissureDrop:
                PtDebugPrint("Cleft.OnBehaviorNotify(): will run respFissureDropMain now.")
                respFissureDropMain.run(self.key,avatar=PtGetLocalAvatar())
