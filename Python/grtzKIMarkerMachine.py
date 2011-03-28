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
Module: grtzKIMarkerMachine
Age: GreatZero
Date: Nov. 10, 2003
Author: Mark DeForest
This for the KI marker machine in the GreatZero near the link area
"""
MaxVersionNumber = 2
MinorVersionNumber = 4

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import string
import xLinkingBookDefs
from xPsnlVaultSDL import *

aKISlotAct = ptAttribActivator(1,"KI Slot activator")
aKIAvatarResp = ptAttribResponder(2, "Avatar Behavior resp",statelist=["Short","Longer"])
aKILogoResp = ptAttribResponder(3, "Show KI Logo resp")
aKILightsResp = ptAttribResponderList(4, "Light Responder List",statelist=["LightOn","LightOff"],byObject=1)
aKISoundsResp = ptAttribResponder(5, "Sound responder",statelist=["UpLoadMarkers","DownLoadMarkers","ShowMarkers"])

gGZPlaying = 0
gGZMarkerInRange = 0
gGZMarkerInRangeRepy = None
# what is the to get color
gMarkerToGetColor = 'off'
gMarkerGottenColor = 'off'
gMarkerToGetNumber = 0
gMarkerGottenNumber = 0

gMarkerTargetToGetNumber = 0

gAvatar = None

gIsDownloadingLastGame = 0

#--- -constants
kLightShiftOnDelaySeconds = 0.1
kLightShiftOnID = 100

kTimeInbetweenDownAndUpload = 1.5

kLightShiftOffDelaySeconds = 0.1
kLightShiftOffID = 200

kLightsOffSeconds = 2
kLightsOffDelayID = 10

kReEnableClickableSeconds = 8
kReEnableClickableID = 500
kNumGZMarkers = 40

gLightRespNames = [\
                "cRespKIMachLight01",\
                "cRespKIMachLight02",\
                "cRespKIMachLight03",\
                "cRespKIMachLight04",\
                "cRespKIMachLight05",\
                "cRespKIMachLight06",\
                "cRespKIMachLight07",\
                "cRespKIMachLight08",\
                "cRespKIMachLight09",\
                "cRespKIMachLight10",\
                "cRespKIMachLight11",\
                "cRespKIMachLight12",\
                "cRespKIMachLight13",\
                "cRespKIMachLight14",\
                "cRespKIMachLight15",\
            ]

#Utility classes
def ResetMarkerGame():
    "Resets marker game chronicle states"
    #This is used for when the KI or the grtzKIMarkerMachine entcounters a corrupted vault
    vault = ptVault()

    # First, reset the KI Marker Level
    entry = vault.findChronicleEntry(kChronicleKIMarkerLevel)
    if type(entry) != type(None):
        entry.chronicleSetValue("0")
        entry.save()

    # Next, reset the Marker's Aquired Data
    UpdateGZMarkers(kGZMarkerInactive)

    # Now reset the chronicle display settings
    resetString = "0 off:off 0:0"
    entry = vault.findChronicleEntry(kChronicleGZGames)
    if type(entry) != type(None):
        entry.chronicleSetValue(resetString)

    #Who knows what state we were in so we'll reset the CGZs as well!!!
    PtSendKIMessage(kMGStopCGZGame, 0)

    # Finally, update the KI display
    PtSendKIMessage(kGZUpdated,0)

def UpdateGZMarkers(markerStatus):
    "Stops the actual markers from being displayed in the game"
    statusTypes = (kGZMarkerInactive, kGZMarkerAvailable, kGZMarkerCaptured, kGZMarkerUploaded)

    if not markerStatus in statusTypes:
        PtDebugPrint("ERROR: grtzKIMarkerMachine.UpdateGZMarkers(): Invalid markerStatus: %s" %markerStatus)
        return

    vault = ptVault()
    entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
    if type(entry) != type(None):
        markers = entry.chronicleGetValue()
        resetValue = markerStatus * len(markers)
        entry.chronicleSetValue(resetValue)
        entry.save()
    else:
        markers = markerStatus * kNumGZMarkers
        vault.addChronicleEntry(kChronicleGZMarkersAquired,kChronicleGZMarkersAquiredType,markers)

class grtzKIMarkerMachine(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 206
        self.version = MaxVersionNumber
        PtDebugPrint("grtzKIMarkerMachine: Max version %d - minor version %d.1" % (MaxVersionNumber,MinorVersionNumber),level=kDebugDumpLevel)

    def OnNotify(self,state,id,events):
        "Notify from region sensor or from the KI"
        global gAvatar
        global gIsDownloadingLastGame
        global kNumGZMarkers
        PtDebugPrint("grtzKIMarkerMachine: Notify  state=%f, id=%d" % (state,id),level=kDebugDumpLevel)

        # if this is from the region sensor, then determine if this
        if id == aKISlotAct.id:
            avatar = PtFindAvatar(events)
            # disable the clickable, so that someone else doesn't try using it until we're done with it
            aKISlotAct.disable()
            gAvatar = avatar
            if state:
                # the avatar responder ... the avatar's hand is in the KI slot
                if PtDetermineKILevel() >= kNormalKI:
                    # Run the avatar behavior responder
                    aKIAvatarResp.run(self.key,avatar=avatar,state='Longer')
                else:
                    aKIAvatarResp.run(self.key,avatar=avatar,state='Short')
            if gAvatar != PtGetLocalAvatar():
                PtAtTimeCallback(self.key, kReEnableClickableSeconds, kReEnableClickableID)
        if id == aKIAvatarResp.id:
            # make sure this is our avatar
            if gAvatar != PtGetLocalAvatar():
                return
            # the avatar responder ... the avatar's hand is in the KI slot
            if PtDetermineKILevel() >= kNormalKI:
                # show the logo:
                aKILogoResp.run(self.key)
                # pop up their miniKI
                PtSendKIMessage(kKIShowMiniKI,0)
                # determine where they are in the GZMarker thing
                markerKILevel = PtDetermineKIMarkerLevel()

                if markerKILevel > kKIMarkerFirstLevel:
                    print "Making Sure Nexus Link Exists."
                    self.IUpdateNexusLink()

                #Don't try and get the game if we've got a normal KI marker level (i.e. finished with first two marker games)
                #It will clobber any marker game progress if we try!
                if markerKILevel >= kKIMarkerNormalLevel:
                    # they are finished with this... nothing to do
                    PtDebugPrint("grtzKIMarkerMachine: They're all done with this!",level=kDebugDumpLevel)
                    aKISlotAct.enable()
                    return
                
                
                self.IGetGZGame()
                gIsDownloadingLastGame = 0
                # is this the first time?
                if markerKILevel == kKIMarkerNotUpgraded:
                    PtDebugPrint("grtzKIMarkerMachine: Starting first GZ game",level=kDebugDumpLevel)
                    PtSendKIMessageInt(kUpgradeKIMarkerLevel,kKIMarkerFirstLevel)
                    self.IUploadGame1()
                    PtSendKIMessageInt(kGZUpdated, kNumGZMarkers)
                    #Ensure that the chronicle entry is setup correctly
                    #The case we're most concerned with is a vault corruption issue where the chronicle entry exists, but is not initialized correctly
                    #Since we're turning on for the first time, we'll enable each marker
                    vault = ptVault()
                    entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
                    if type(entry) == type(None):
                        # if there is none, then just add another entry - start off as active
                        markers = kGZMarkerAvailable * kNumGZMarkers
                        vault.addChronicleEntry(kChronicleGZMarkersAquired,kChronicleGZMarkersAquiredType,markers)
                    else:
                        markers = kGZMarkerAvailable * kNumGZMarkers
                        entry.chronicleSetValue(markers)
                        entry.save()

                    # after fun with lights then the activator will be enabled
                # are we working on the first game... is it done?
                elif markerKILevel == kKIMarkerFirstLevel:
                    if gMarkerGottenNumber >= gMarkerToGetNumber:
                        PtDebugPrint("grtzKIMarkerMachine: Done with first game",level=kDebugDumpLevel)
                        # they got them all... download them
                        PtSendKIMessageInt(kUpgradeKIMarkerLevel,kKIMarkerSecondLevel)
                        self.IDownloadGame1()
                        # after fun with lights then the activator will be enabled
                    else:
                        PtDebugPrint("grtzKIMarkerMachine: In the middle of the first game...",level=kDebugDumpLevel)
                        self.IShowCurrentGame()
                        aKISlotAct.enable()
                elif markerKILevel == kKIMarkerSecondLevel:
                    PtDebugPrint("grtzKIMarkerMachine: In the middle of the second game",level=kDebugDumpLevel)
                    if gMarkerGottenNumber >= gMarkerToGetNumber:
                        PtDebugPrint("grtzKIMarkerMachine: Done with second game",level=kDebugDumpLevel)
                        gIsDownloadingLastGame = 1
                        self.IDownloadGame2()
                        UpdateGZMarkers(kGZMarkerUploaded)
                        PtSendKIMessage(kGZUpdated,0)
                        # after fun with lights then the activator will be enabled
                    else:
                        PtDebugPrint("grtzKIMarkerMachine: In the middle of the second game...",level=kDebugDumpLevel)
                        self.IShowCurrentGame()
                        aKISlotAct.enable()
            else:
                # if they don't have a KI then do nothing...
                PtDebugPrint("grtzKIMarkerMachine: KI level not high enough",level=kDebugDumpLevel)
                aKISlotAct.enable()

    def IGetGZGame(self):
        global gGZPlaying
        global gMarkerGottenColor
        global gMarkerToGetColor
        global gMarkerGottenNumber
        global gMarkerToGetNumber
        vault = ptVault()
        # is there a chronicle for the GZ games?
        entry = vault.findChronicleEntry(kChronicleGZGames)
        error = 0
        if type(entry) != type(None):
            markerGameString = entry.chronicleGetValue()
            args = markerGameString.split()
            
            if len(args) == 3:
                try:
                    gGZPlaying = string.atoi(args[0])
                    colors = args[1].split(':')
                    outof = args[2].split(':')
                    if len(colors) != 2 or len(outof) != 2:
                        raise ValueError

                    gMarkerGottenColor = colors[0]
                    gMarkerToGetColor = colors[1]
                    gMarkerGottenNumber = string.atoi(outof[0])
                    gMarkerToGetNumber = string.atoi(outof[1])
                    return
                except:
                    PtDebugPrint("grtzKIMarkerMachine - error trying to read GZGames Chronicle",level=kErrorLevel)
                    error = 1
            else:
                PtDebugPrint("grtzKIMarkerMachine - error GZGames string formation error",level=kErrorLevel)
                error = 1
        gGZPlaying = 0
        gMarkerGottenColor = 'off'
        gMarkerToGetColor = 'off'
        gMarkerGottenNumber = 0
        gMarkerToGetNumber = 0
        #We'll save the game now (this will help reduce vault corruption issues (i.e. the 1515 bug).
        self.ISetGZGame()

        if error:
            # This is a vault corruption issue...  yes, very bad
            # We're going to re-initialize the marker games to give the user a chance to go on
            # Although I'm sure they'll still be upset!
            PtDebugPrint("grtzKIMarkerMachine - vault corruption error: RESETTING all Marker Game data!!!!", level=kErrorLevel)
            ResetMarkerGame()




    def IFlashGZGame(self):
        global gGZPlaying
        global gMarkerGottenColor
        global gMarkerToGetColor
        global gMarkerGottenNumber
        global gMarkerToGetNumber
        # flash the markers in the miniKI
        upstring = "%d %s:%s %d:%d" % (gGZPlaying,gMarkerGottenColor,gMarkerToGetColor,gMarkerGottenNumber,gMarkerToGetNumber)
        PtSendKIMessage(kGZFlashUpdate,upstring)

    def ISetGZGame(self):
        global gGZPlaying
        global gMarkerGottenColor
        global gMarkerToGetColor
        global gMarkerGottenNumber
        global gMarkerToGetNumber
        # set the markers in the miniKI
        vault = ptVault()
        # is there a chronicle for the GZ games?
        entry = vault.findChronicleEntry(kChronicleGZGames)
        upstring = "%d %s:%s %d:%d" % (gGZPlaying,gMarkerGottenColor,gMarkerToGetColor,gMarkerGottenNumber,gMarkerToGetNumber)
        if type(entry) != type(None):
            entry.chronicleSetValue(upstring)
            entry.save()
        else:
            # if there is none, then just add another entry
            vault.addChronicleEntry(kChronicleGZGames,kChronicleGZGamesType,upstring)

    def IUploadGame1(self):
        "Upload the first game of 15 GZMarkers"
        global gGZPlaying
        global gMarkerGottenColor
        global gMarkerToGetColor
        global gMarkerGottenNumber
        global gMarkerToGetNumber
        global gMarkerTargetToGetNumber
        PtDebugPrint("grtzKIMarkerMachine - uploading game 1 - shifting lights",level=kDebugDumpLevel)
        aKISoundsResp.run(self.key,state='UpLoadMarkers')
        gGZPlaying = 1
        gMarkerGottenColor = 'green'
        gMarkerToGetColor = 'greenlt'
        gMarkerGottenNumber = 0
        gMarkerToGetNumber = 0
        gMarkerTargetToGetNumber = 15
        PtAtTimeCallback(self.key, kLightShiftOnDelaySeconds, kLightShiftOnID)

    def IRefreshNextLightOn(self):
        "turn on a light and set miniKIlight on, also"
        global gMarkerToGetNumber
        global gMarkerTargetToGetNumber
        if gMarkerToGetNumber < gMarkerTargetToGetNumber:
            PtDebugPrint("grtzKIMarkerMachine - lighting light %d"%(gMarkerToGetNumber+1),level=kDebugDumpLevel)
            lidx = gMarkerToGetNumber
            aKILightsResp.run(self.key,state='LightOn',objectName=gLightRespNames[lidx])
            gMarkerToGetNumber += 1
            self.IFlashGZGame()
            PtAtTimeCallback(self.key, kLightShiftOnDelaySeconds, kLightShiftOnID)
        else:
            PtDebugPrint("grtzKIMarkerMachine - all done shifting",level=kDebugDumpLevel)
            gMarkerToGetNumber = gMarkerTargetToGetNumber
            self.ISetGZGame()
            PtSendKIMessage(kGZUpdated,0)
            PtAtTimeCallback(self.key, kLightsOffSeconds, kLightsOffDelayID)

    def IDownloadGame1(self):
        "Download the first games GZMarkers"
        aKISoundsResp.run(self.key,state='DownLoadMarkers')
        aKILightsResp.run(self.key,state='LightOn')
        PtAtTimeCallback(self.key, kLightShiftOffDelaySeconds, kLightShiftOffID)

    def IRefreshNextLightOff(self):
        "turn on a light and set miniKIlight on, also"
        global gMarkerToGetNumber
        global gMarkerTargetToGetNumber
        global gIsDownloadingLastGame
        if gMarkerToGetNumber > 0:
            gMarkerToGetNumber -= 1
            lidx = gMarkerToGetNumber
            aKILightsResp.run(self.key,state='LightOff',objectName=gLightRespNames[lidx])
            self.IFlashGZGame()
            PtAtTimeCallback(self.key, kLightShiftOffDelaySeconds, kLightShiftOffID)
        else:
            # need to start next thing... if there is one
            if not gIsDownloadingLastGame:
                self.IUploadGame2()
            else:
                # else that's all there is to do
                # wipe out the last game
                vault = ptVault()
                # is there a chronicle for the GZ games?
                entry = vault.findChronicleEntry(kChronicleGZGames)
                if type(entry) != type(None):
                    entry.chronicleSetValue("0")
                    entry.save()
                # they've made it to the next level
                PtSendKIMessageInt(kUpgradeKIMarkerLevel,kKIMarkerNormalLevel)
                # re-enable the activator
                aKISlotAct.enable()

    def IUploadGame2(self):
        "Upload the second game of 15 GZMarkers"
        global gGZPlaying
        global gMarkerGottenColor
        global gMarkerToGetColor
        global gMarkerGottenNumber
        global gMarkerToGetNumber
        global gMarkerTargetToGetNumber
        self.IUpdateNexusLink()
        PtDebugPrint("grtzKIMarkerMachine - uploading game 1 - shifting lights",level=kDebugDumpLevel)
        aKISoundsResp.run(self.key,state='UpLoadMarkers')
        gGZPlaying = 2
        gMarkerGottenColor = 'red'
        gMarkerToGetColor = 'redlt'
        gMarkerGottenNumber = 0
        gMarkerToGetNumber = 0
        gMarkerTargetToGetNumber = 15
        PtAtTimeCallback(self.key, kTimeInbetweenDownAndUpload, kLightShiftOnID)

    def IDownloadGame2(self):
        "Download the second games GZMarkers"
        aKILightsResp.run(self.key,state='LightOn')
        PtAtTimeCallback(self.key, kLightShiftOffDelaySeconds, kLightShiftOffID)

    def IShowCurrentGame(self):
        "display what they've gotten so far... sorta"
        if gMarkerGottenNumber == 0:
            PtDebugPrint("grtzKIMarkerMachine - turn all the lights on",level=kDebugDumpLevel)
            aKISoundsResp.run(self.key,state='ShowMarkers')
            aKILightsResp.run(self.key,state='LightOn')
        else:
            PtDebugPrint("grtzKIMarkerMachine - turn only %d lights on" % (gMarkerToGetNumber-gMarkerGottenNumber),level=kDebugDumpLevel)
            aKISoundsResp.run(self.key,state='ShowMarkers')
            for idx in range(gMarkerGottenNumber,gMarkerToGetNumber):
                aKILightsResp.run(self.key,state='LightOn',objectName=gLightRespNames[idx])
        PtAtTimeCallback(self.key, kLightsOffSeconds, kLightsOffDelayID)

    def IGetHoodLinkNode(self):
        vault = ptVault()
        folder = vault.getAgesIOwnFolder()
        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            if type(link) != type(None):
                info = link.getAgeInfo()
            if not info: continue
            ageName = info.getAgeFilename()
            if ageName == "Neighborhood":
                return link
        return None
    
    def IGetHoodInfoNode(self):
        link = self.IGetHoodLinkNode()
        if type(link) == type(None):
            return None
        info = link.getAgeInfo()
        return info

    def IUpdateNexusLink(self):
        childAgeFolder = self.IGetHoodInfoNode().getChildAgesFolder()
        contents = childAgeFolder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            name = link.getAgeInfo().getAgeFilename()
            if name == "GreatZero":
                GZSpawnPoints = link.getSpawnPoints()
                for SpawnPoint in GZSpawnPoints:
                    title = SpawnPoint.getTitle().lower()
                    name = SpawnPoint.getName().lower()
                    #print "Title: %s\nName: %s" % (title,name)
                    if title == "great zero" and name == "bigroomlinkinpoint":
                        print "grtzKIMarkerMachine: Nexus link already exists."
                        return
                #self.IDoCityLinksChron("BigRoomLinkInPoint")   # this will be used if we want to add this to the city book
                outerRoomSP = ptSpawnPointInfo("Great Zero","BigRoomLinkInPoint")
                #print "NewSpawnPointInfo\nName: %s\nTitle: %s" % (outerRoomSP.getName(), outerRoomSP.getTitle())
                link.addSpawnPoint(outerRoomSP)
                link.save()
                print "grtzKIMarkerMachine: Nexus link added."
                PtSendKIMessage(kKILocalChatStatusMsg,PtGetLocalizedString("KI.Messages.NexusLinkAdded"))
                PtDebugPrint("grtzKIMarkerMachine - setting new spawn point for GZ",level=kDebugDumpLevel)
                return
        PtDebugPrint("grtzKIMarkerMachine - error - could not find link to add spawnpoint to")

    def OnTimer(self,id):
        if id == kLightsOffDelayID:
            if gMarkerGottenNumber == 0:
                PtDebugPrint("grtzKIMarkerMachine - turn all the lights off",level=kDebugDumpLevel)
                aKILightsResp.run(self.key,state='LightOff')
            else:
                PtDebugPrint("grtzKIMarkerMachine - turn only %d lights off" % (gMarkerToGetNumber-gMarkerGottenNumber),level=kDebugDumpLevel)
                for idx in range(gMarkerGottenNumber,gMarkerToGetNumber):
                    aKILightsResp.run(self.key,state='LightOff',objectName=gLightRespNames[idx])
            aKISlotAct.enable()
        elif id == kLightShiftOnID:
            self.IRefreshNextLightOn()
        elif id == kLightShiftOffID:
            self.IRefreshNextLightOff()
        elif id == kReEnableClickableID:
            aKISlotAct.enable()


    def IDoCityLinksChron(self,agePanel):   # ganked from xLinkingBookGUIPopup.py
        CityLinks = []
        vault = ptVault()
        entryCityLinks = vault.findChronicleEntry("CityBookLinks")
        if type(entryCityLinks) != type(None):
            valCityLinks = entryCityLinks.chronicleGetValue()
            print "valCityLinks = ",valCityLinks
            CityLinks = valCityLinks.split(",")
            print "CityLinks = ",CityLinks
            if agePanel not in CityLinks:
                NewLinks = valCityLinks + "," + agePanel
                entryCityLinks.chronicleSetValue(NewLinks)
                entryCityLinks.save()
                print "grtzKIMarkerMachine.IDoCityLinksChron():  setting citylinks chron entry to include: ",agePanel
                valCityLinks = entryCityLinks.chronicleGetValue()
                CityLinks = valCityLinks.split(",")
                print "grtzKIMarkerMachine.IDoCityLinksChron():  citylinks now = ",CityLinks
            else:
                print "grtzKIMarkerMachine.IDoCityLinksChron():  do nothing, citylinks chron already contains: ",agePanel
        else:
            vault.addChronicleEntry("CityBookLinks",0,agePanel)
            print "grtzKIMarkerMachine.IDoCityLinksChron():  creating citylinks chron entry and adding: ",agePanel
        
        psnlSDL = xPsnlVaultSDL()
        GotBook = psnlSDL["psnlGotCityBook"][0]
        if not GotBook:
            psnlSDL["psnlGotCityBook"] = (1,)
            print "grtzKIMarkerMachine.IDoCityLinksChron():  setting SDL for city book to 1"