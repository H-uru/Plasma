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
Module: nxusBookMachine
Age: nexus
Date: September, 2002
Author: Bill Slease
Handler for the nexus book machine.
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
import PlasmaControlKeys
from xPsnlVaultSDL import *
import time

import xLocTools

import string
import whrandom

import xVisitorUtils

# define the attributes that will be entered in max
NexusGUI = ptAttribGUIDialog(1,"The Nexus GUI")
actKISlot = ptAttribActivator(2,"Actvtr: KI Slot")
respKISlot = ptAttribResponder(3,"Rspndr: KI Slot")
camMachineOps = ptAttribSceneobject(4,"Camera: machine ops")
behMachineOps = ptAttribBehavior(5, "Behavior: machine ops")
respKISlotReturn = ptAttribResponder(6,"Rspndr: KI Slot Return") # onInit
respGUIOn = ptAttribResponder(7,"Rspndr: GUI On")
respGUIOff = ptAttribResponder(8,"Rspndr: GUI Off")
respBookSelect = ptAttribResponder(9,"Rspndr: Enable GetBook Btn")
#respBookGet = ptAttribResponder(10,"Rspndr: Get Book")
actLink = ptAttribActivator(11,"Actvtr: Link")
respLink = ptAttribResponder(12,"Rspndr: Link")
respBookRetract = ptAttribResponder(13,"Rspndr: Retract Book") # onInit
actGetBook = ptAttribActivator(14,"Actvtr: Get Book")
respGetBook = ptAttribResponder(15,"Rspndr: Get Book")
respButtonPress = ptAttribResponder(16,"Rspndr: GetBook Btn Press") # onInit
objlistLinkPanels = ptAttribSceneobjectList(17,"Objct: Link Panels")
respKISlotGlow = ptAttribResponder(18,"Rspndr: KI Slot Glow")



# globals
#==============

kHiddenPersonalAges = ["Personal","Nexus","Neighborhood","city","AvatarCustomization","Cleft","BaronCityOffice","BahroCave","PelletBahroCave","Kveer","Myst","LiveBahroCaves","LiveBahroCave"]
kHiddenAgesIfVisitor = ["Kadish","Gira","Garrison","Teledahn"]
kHiddenCityLinks = ["islmPalaceBalcony03","KadishGallery","islmPalaceBalcony02","islmDakotahRoof"]
kHiddenAgesIfInvited = ["BahroCave","PelletBahroCave","Pellet Cave","LiveBahroCave","LiveBahroCaves","Myst"]

#------nexus machine GUI tags
kNexusDialogName="NexusAgeDialog"

kIDBtnLinkCategory01 = 100
kIDTxtLinkCategory01 = 101
kIDBtnLinkCategory02 = 110
kIDTxtLinkCategory02 = 111
kIDBtnLinkCategory03 = 120
kIDTxtLinkCategory03 = 121
kIDBtnLinkCategory04 = 130
kIDTxtLinkCategory04 = 131

kIDBtnNeighborhoodCreate = 400
kIDBtnNeighborhoodSelect = 401
kIDTxtNeighborhoodName = 402
kIDTxtNeighborhoodInfo = 403

kIDBtnNeighborhoodPublic = 404
kIDTxtNeighborhoodPublic = 405

kIDBtnScrollUp = 500
kIDBtnScrollDn = 501

kIDTxtLinkDescription = 200

kIDBtnLinkSelect01 = 210
kIDBtnLinkSelect02 = 220
kIDBtnLinkSelect03 = 230
kIDBtnLinkSelect04 = 240
kIDBtnLinkSelect05 = 250
kIDBtnLinkSelect06 = 260
kIDBtnLinkSelect07 = 270
kIDBtnLinkSelect08 = 280
kIDTxtLinkName01 = 211
kIDTxtLinkName02 = 221
kIDTxtLinkName03 = 231
kIDTxtLinkName04 = 241
kIDTxtLinkName05 = 251
kIDTxtLinkName06 = 261
kIDTxtLinkName07 = 271
kIDTxtLinkName08 = 281
kIDTxtLinkInfo01 = 212
kIDTxtLinkInfo02 = 222
kIDTxtLinkInfo03 = 232
kIDTxtLinkInfo04 = 242
kIDTxtLinkInfo05 = 252
kIDTxtLinkInfo06 = 262
kIDTxtLinkInfo07 = 272
kIDTxtLinkInfo08 = 282

kIDBtnDeleteLink01 = 310
kIDBtnDeleteLink02 = 320
kIDBtnDeleteLink03 = 330
kIDBtnDeleteLink04 = 340
kIDBtnDeleteLink05 = 350
kIDBtnDeleteLink06 = 360
kIDBtnDeleteLink07 = 370
kIDBtnDeleteLink08 = 380

kIDNameHeaderText = 600
kIDPopHeaderText = 601
kIDNameHeaderBtn = 610
kIDPopHeaderBtn = 611
kIDNameAscArrow = 620
kIDPopAscArrow = 621
kIDNameDescArrow = 630
kIDPopDescArrow = 631

kIDEngCheckBox = 700
kIDFreCheckBox = 701
kIDGerCheckBox = 702
kIDEngCheck = 710
kIDFreCheck = 711
kIDGerCheck = 712
kIDEngText = 720
kIDFreText = 721
kIDGerText = 722

#------colors
AgenGoldDk = ptColor(0.925,0.84,0.365,1.0)
AgenGoldLt = ptColor(0.97,0.937,0.745,1.0)
AgenGoldDkSoft = ptColor(0.925,0.84,0.365,0.25)
AgenGoldLtSoft = ptColor(0.97,0.937,0.745,0.25)

colorNormal = AgenGoldDk
colorSelected = AgenGoldLt
colorPresented = AgenGoldLt
colorDisabled = AgenGoldDkSoft

#-------misc
avatar = None
waitingOnGUIAnim = false
idLinkSelected = 0
idBookPresented = 0
boolGetBookBtnUp = false
boolGetBookAfterBtnPress = false
boolBookPresented = false
boolGetBookAfterBookRetract = false
idDeleteCandidateName = 0
idCategorySelected = kIDBtnLinkCategory01
boolLinking = false
boolGettingBook = false
indexDisplayStart = 0
dialogLoaded = 0
kNumDisplayFields = 8
kEmptyGuid = '0000000000000000'
kMaxDisplayableChars = 24 # the avg number of chars to display before tacking on an ellipsis: "..."

# the vars that can be randomized to any valid value for their type
kRandomizeVars = ["nb01ClockVis","nb01GardenFungusVis",
                "nb01DestructionCracksVis","nb01LanternsVis","nb01LampOption01Vis","nb01OldImager01Vis",
                "nb01OldImager02Vis","nb01WaterfallTorchesVis","nb01ResidenceAdditionsVis"]

# these vars have a specific set of values
# "name" : (0,1,2) <-- range of values
kVarRestrictions = {
    "nb01StainedWindowOption": (0,1,2)
    }

cityInstances = []
cityPopulations = []
curCityInstance = -1 # use the first one as default

pubInstance = None
pubPopulation = 0

kirelInstance = None
kirelPopulation = 0

#guildPubInstance = None
guildPubPopulation = 0
guildPubs = ["Cartographers","Greeters","Maintainers","Messengers","Writers"]
guildPubGUIDs = [   "35624301-841e-4a07-8db6-b735cf8f1f53","381fb1ba-20a0-45fd-9bcb-fd5922439d05",\
                    "e8306311-56d3-4954-a32d-3da01712e9b5","9420324e-11f8-41f9-b30b-c896171a8712",\
                    "5cf4f457-d546-47dc-80eb-a07cdfefa95d"]
kveerPublicPopulation = 0
#kveerPublicInstance = None
kveerPublicGUID = "68e219e0-ee25-4df0-b855-0435584e29e2"
publicHoods = []
publicHoodsPop = []
sortedHoods = []
sortedHoodsPop = []

kChronicleVarType = 0  # not currently used

boolNewHoodDlg = 0
boolGUIActivated = 0

fullNeighborhoodName = U"" # the full name for our neighborhood (because we truncate long names)
fullLinkName = [] # the full text for each of our displayed links (because we truncate long names)
untranslatedNeighborhoodName = U""
untranslatedName = [] # the untranslated form of the title (needed in some cases to find a linking panel or link node)

oldStatusBarText = U"" # when we mouse over and display the expanded text, we want to know what we had here last

maxCityPopulation = -1 # what is the maximum city population?
maxPubPopulation = -1 # what is the maximum pub population?
maxKirelPopulation = -1 # what is the maximum kirel population?
maxGuildPubPopulation = -1 # what is the maximum GuildPub population?
maxKveerPublicPopulation = -1 # what is the maximum public Kveer population?
showCityLinks = 1 # do we show city links other then the Ferry Terminal?
showGreatZero = 0 # do we show the great zero under our city links?
showPub = 0 # do we show the watcher's pub under our city links?
showKirel = 0 # do we show kirel under our city links?
showGuildPub = 0 # do we show one of the guild-pubs under our city links?
showKveerPublic = 0 # do we show the public Kveer under our city links?

# hood sorting vars
kSortNone = 0
kSortNameAsc = 1
kSortNameDesc = 2
kSortPopAsc = 3
kSortPopDesc = 4
publicHoodSort = kSortNone
showEnglishHoods = 1 # we default to showing all languages
showFrenchHoods = 1
showGermanHoods = 1

# these vars help us know what "city" links go to the city, the pub, or a child age (order is city->pub->children in the list)
numCityLinks = 0 # the number of links in the city we have
numPubLinks = 0 # the number of links to the pub we have
numKirelLinks = 0 # the number of links to kirel we have
numGuildPubLinks = 0 # the number of links to the GuildPubs we have
numKveerPublicLinks = 0 # the number of links to the public Kveer we have

childAgeLinkNodes = [] # the array of link nodes to the hood child ages

# this var helps us know what "personal" links go to the GZ, or your personal ages (order is GZ->personal in the list)
numGZLinks = 0 # the number of links to the great zero that we have
GZLinkNode = None # the link node to the Great Zero (if we have one)
GZSpawnpoints = [] # the GZ spawnpoints that are displayed

IsVisitorPlayer = true  #whether or not the player is a visitor

def Uni(string):
    "Converts a string to unicode, using latin-1 encoding if necessary"
    try:
        retVal = unicode(string)
        return retVal
    except:
        retVal = unicode(string,"latin-1")
        return retVal

class nxusBookMachine(ptModifier):
    "The Nexus python code"
    def __init__(self):
        global IsVisitorPlayer
        ptModifier.__init__(self)
        self.id = 5017
        version = 5
        self.version = version
        print "__init__nxusBookMachine v.", version
        
        #Init visitor support
        IsVisitorPlayer = not PtIsSubscriptionActive()       
        PtLoadDialog(xVisitorUtils.kVisitorNagDialog)


    def OnFirstUpdate(self):
        "First update, load GUI dialog, give player PAL to Nexus"
        PtLoadDialog(kNexusDialogName, self.key, "Nexus")

        # move objects to correct starting position just in case
        respKISlotReturn.run(self.key,fastforward=1)
        respBookRetract.run(self.key,fastforward=1)
        respButtonPress.run(self.key,fastforward=1)

        # hide all the linking panels in the machine - will draw appropriate when selected
        for objPanel in objlistLinkPanels.value:
            #print "hiding link panel: ",objPanel.getName()
            objPanel.draw.disable()


    def OnServerInitComplete(self):
        global maxCityPopulation
        global maxPubPopulation
        global maxKirelPopulation
        global maxGuildPubPopulation
        global maxKveerPublicPopulation
        global showCityLinks
        global showGreatZero
        global showPub
        global showKirel
        global showGuildPub
        #global guildPubInstance
        global guildPubPopulation
        global showKveerPublic
        #global kveerPublicInstance
        global kveerPublicPopulation


        ageSDL = PtGetAgeSDL()
        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing max city population")
        try:
            maxCityPopulation = ageSDL["MaxCityPop"][0]
        except:
            PtDebugPrint("Unable to get max city population from SDL, defaulting to 20")
            maxCityPopulation = 20
        
        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing max pub population")
        try:
            maxPubPopulation = ageSDL["MaxPubPop"][0]
        except:
            PtDebugPrint("Unable to get max pub population from SDL, defaulting to 100")
            maxPubPopulation = 100

        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing max kirel population")
        try:
            maxKirelPopulation = ageSDL["MaxKirelPop"][0]
        except:
            PtDebugPrint("Unable to get max kirel population from SDL, defaulting to 100")
            maxKirelPopulation = 100

        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing max GuildPub population")
        try:
            maxGuildPubPopulation = ageSDL["MaxGuildPubPop"][0]
        except:
            PtDebugPrint("Unable to get max GuildPub population from SDL, defaulting to 100")
            maxGuildPubPopulation = 100

        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing max public Kveer population")
        try:
            maxKveerPublicPopulation = ageSDL["MaxKveerPublicPop"][0]
        except:
            PtDebugPrint("Unable to get max public Kveer population from SDL, defaulting to 100")
            maxKveerPublicPopulation = 100

        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing the show city links toggle")
        try:
            showCityLinks = ageSDL["nxusCityLinksVis"][0]
        except:
            PtDebugPrint("Unable to get show city links toggle from SDL, defaulting to true")
            showCityLinks = 1

        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing the show great zero toggle")
        try:
            showGreatZero = ageSDL["nxusShowGZ"][0]
        except:
            PtDebugPrint("Unable to get show great zero toggle from SDL, defaulting to false")
            showGreatZero = 0
        
        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing the show pub toggle")
        try:
            showPub = ageSDL["nxusShowPub"][0]
        except:
            PtDebugPrint("Unable to get show pub toggle from SDL, defaulting to false")
            showPub = 0

        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing the show Kirel toggle")
        try:
            showKirel = ageSDL["nxusShowKirel"][0]
        except:
            PtDebugPrint("Unable to get show kirel toggle from SDL, defaulting to false")
            showKirel = 0

        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing the show GuildPub var")
        try:
            psnlSDL = xPsnlVaultSDL()
            showGuildPub = psnlSDL["guildAlliance"][0]
            PtDebugPrint("'guildAlliance' SDL = %d" % (showGuildPub))
        except:
            PtDebugPrint("Unable to get show GuildPub toggle from SDL, defaulting to false")
            showGuildPub = 0
        if showGuildPub:
            guild = guildPubs[showGuildPub-1]
            PtDebugPrint("nxusBookMachine.OnServerInitComplete() - member of guild: %s" % (guild))

        vault = ptVault()
        entry = vault.findChronicleEntry("GotLinkToKveerPublic")
        if type(entry) != type(None):
            entryValue = entry.chronicleGetValue()
            if entryValue == "yes":
                print "nxusBookMachine.OnServerInitComplete(): chron says you have the link to public Kveer, woo hoo"
                showKveerPublic = 1
        else:
            print "nxusBookMachine.OnServerInitComplete(): chron says no link to public Kveer yet, so sorry"
        

    def __del__(self):
        "destructor - get rid of any dialogs that we might have loaded"
        PtUnloadDialog(kNexusDialogName)
        PtUnloadDialog(xVisitorUtils.kVisitorNagDialog)        
        
    def ICanCreateHood(self):
        hoodLink = self.IGetHoodLinkNode()
        if not hoodLink.getVolatile():
            return 0 # only let the player create a new hood if their book is volatile
        if PtIsInternalRelease():
            return 1 # we don't have the 1 day limitation on internal clients
        vault = ptVault()
        entry = vault.findChronicleEntry("LastHoodCreationTime")
        if type(entry) == type(None):
            return 1 # we haven't created a hood before, so let them!

        temp = float(entry.chronicleGetValue())
        lastTime = time.gmtime(temp)
        temp = PtGetDniTime()
        curTime = time.gmtime(temp)

        # compare years
        if lastTime[0] < curTime[0]:
            return 1
        if lastTime[0] == curTime[0]: # same year
            # compare months
            if lastTime[1] < curTime[1]:
                return 1
            if lastTime[1] == curTime[1]: # same month
                # compare days
                if lastTime[2] < curTime[2]:
                    return 1
        # less then a day has passed (or we went backwards in time), so don't let them create a new hood!
        return 0
    
    def gotPublicAgeList(self,ages):
        global cityInstances
        global cityPopulations
        global curCityInstance
        global publicHoods
        global publicHoodsPop
        global pubInstance
        global pubPopulation
        global kirelInstance
        global kirelPopulation

        if len(ages) == 0:
            PtDebugPrint("nxusBookMachine.gotPublicAgeList() - got an empty list, which we assume are hoods, clearing hood list")
            # there should always be city instances, so if we "get" an empty list, assume we wanted a hood list
            publicHoods = []
            publicHoodsPop = []
            self.IUpdateLinks()
            return

        if ages[0][0].getAgeFilename() == "city":
            PtDebugPrint("nxusBookMachine.gotPublicAgeList() - got the list of cities")
            cityInstances = []
            cityPopulations = []
            for age in ages:
                cityInstances.append(age[0])
                cityPopulations.append(age[1])
            # we're going to sort by sequence number so we always have the same order
            for i in range(0,len(cityInstances)-1):
                for j in range(i+1,len(cityInstances)):
                    if cityInstances[i].getAgeSequenceNumber() > cityInstances[j].getAgeSequenceNumber():
                        temp = cityInstances[i]
                        cityInstances[i] = cityInstances[j]
                        cityInstances[j] = temp
                        temp = cityPopulations[i]
                        cityPopulations[i] = cityPopulations[j]
                        cityPopulations[j] = temp
            # now we should have a list of city instances sorted by age sequence number in ascending order
        elif ages[0][0].getAgeFilename() == "Neighborhood":
            PtDebugPrint("nxusBookMachine.gotPublicAgeList() - got the list of hoods")
            publicHoods = []
            publicHoodsPop = []
            for age in ages:
                # if the current population and number of owners is zero then don't display it
                if age[2] == 0 and age[1] == 0:
                    continue
                publicHoods.append(age[0])
                publicHoodsPop.append(age[1])
            self.ISortPublicHoods()
        elif ages[0][0].getAgeFilename() == "GreatTreePub":
            PtDebugPrint("nxusBookMachine.gotPublicAgeList() - got the pub")
            # we only have one pub instance
            pubInstance = ages[0][0]
            pubPopulation = ages[0][1]

        elif ages[0][0].getAgeFilename() == "Neighborhood02":
            PtDebugPrint("nxusBookMachine.gotPublicAgeList() - got kirel")
            # we only have one kirel instance
            kirelInstance = ages[0][0]
            kirelPopulation = ages[0][1]

        if curCityInstance == -1:
            self.ISyncCurCityInstanceAndVault()

        self.IUpdateLinks()

    def publicAgeCreated(self,ageName):
        PtDebugPrint("publicAgeCreated: " + ageName);
        PtGetPublicAgeList(ageName, self);
    
    def publicAgeRemoved(self,ageName):
        PtDebugPrint("publicAgeRemoved: " + ageName);
        PtGetPublicAgeList(ageName, self);
    
    def ISwapPublicHoods(self,index1,index2):
        global publicHoods
        global publicHoodsPop

        hoodTemp = publicHoods[index1]
        popTemp = publicHoodsPop[index1]
        publicHoods[index1] = publicHoods[index2]
        publicHoodsPop[index1] = publicHoodsPop[index2]
        publicHoods[index2] = hoodTemp
        publicHoodsPop[index2] = popTemp
    
    def ISortPublicHoods(self):
        global publicHoods
        global publicHoodsPop
        global sortedHoods
        global sortedHoodsPop
        
        sortedHoods = []
        sortedHoodsPop = []

        if publicHoodSort == kSortNone and showEnglishHoods and showFrenchHoods and showGermanHoods:
            sortedHoods = publicHoods
            sortedHoodsPop = publicHoodsPop
            return
        if publicHoodSort == kSortNameAsc or publicHoodSort == kSortNameDesc:
            for i in range(len(publicHoods)-1):
                for j in range(i+1,len(publicHoods)):
                    if publicHoodSort == kSortNameAsc and publicHoods[i].getDisplayName() > publicHoods[j].getDisplayName():
                        self.ISwapPublicHoods(i,j)
                    elif publicHoodSort == kSortNameDesc and publicHoods[i].getDisplayName() < publicHoods[j].getDisplayName():
                        self.ISwapPublicHoods(i,j)
        elif publicHoodSort == kSortPopAsc or publicHoodSort == kSortPopDesc:
            for i in range(len(publicHoods)-1):
                for j in range(i+1,len(publicHoods)):
                    if publicHoodSort == kSortPopAsc and publicHoodsPop[i] > publicHoodsPop[j]:
                        self.ISwapPublicHoods(i,j)
                    elif publicHoodSort == kSortPopDesc and publicHoodsPop[i] < publicHoodsPop[j]:
                        self.ISwapPublicHoods(i,j)
        # now to copy the hoods in the languages we want to the sorted list
        # if the language is not English, French, or German, we assume it is English and treat it as such
        for i in range(len(publicHoods)):
            hoodLanguage = publicHoods[i].getAgeLanguage()
            if hoodLanguage == PtLanguage.kFrench and showFrenchHoods:
                sortedHoods.append(publicHoods[i])
                sortedHoodsPop.append(publicHoodsPop[i])
            elif hoodLanguage == PtLanguage.kGerman and showGermanHoods:
                sortedHoods.append(publicHoods[i])
                sortedHoodsPop.append(publicHoodsPop[i])
            elif not (hoodLanguage == PtLanguage.kFrench or hoodLanguage == PtLanguage.kGerman) and showEnglishHoods:
                sortedHoods.append(publicHoods[i])
                sortedHoodsPop.append(publicHoodsPop[i])
    
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
    
    def IGetGZLinkNode(self):
        childAgeFolder = self.IGetHoodInfoNode().getChildAgesFolder()
        contents = childAgeFolder.getChildNodeRefList()
        childAgeLinkNodes = []
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            name = link.getAgeInfo().getAgeFilename()
            if name.lower() == "greatzero":
                return link
        return None # not found
    
    def IGetGZInfoNode(self):
        link = self.IGetGZLinkNode()
        if type(link) == type(None):
            return None
        info = link.getAgeInfo()
        return info
    
    def IIsMyHoodPublic(self):
        global publicHoods
        infoNode = self.IGetHoodInfoNode()
        return infoNode.isPublic()
    
    def IMakeHoodPublic(self):
        hoodInfo = self.IGetHoodInfoNode()
        if type(hoodInfo) != type(None):
            infoStruct = hoodInfo.asAgeInfoStruct()
            PtCreatePublicAge(infoStruct,self)
    
    def IMakeHoodPrivate(self):
        hoodInfo = self.IGetHoodInfoNode()
        if type(hoodInfo) != type(None):
            guid = hoodInfo.getAgeInstanceGuid()
            PtRemovePublicAge(guid,self)

    def OnVaultNotify(self,event,tupdata):
        global boolBookPresented
        global boolGetBookAfterBookRetract
        global idCategorySelected
        global idBookPresented
        global actLink
        global respBookRetract
        global boolGetBookBtnUp
        
        if event==PtVaultNotifyTypes.kRegisteredOwnedAge:
            ageLinkNode = tupdata[0] # should be a ptVaultAgeLinkNode
            ageInfo = ageLinkNode.getAgeInfo() # ptAgeInfoNode
            ageFilename = ageInfo.getAgeFilename()
            if (ageFilename == "Neighborhood"):
                PtDebugPrint( "OnVaultNotify: A new neighborhood was created! Time to scramble...I mean randomize the SDL!" )
                self.IRandomizeNeighborhood(ageInfo)
                PtDebugPrint( "OnVaultNotify: Setting the new hood's language to "+str(PtGetLanguage()) )
                ageInfo.setAgeLanguage(PtGetLanguage())
                # save our creation time to the vault to prevent people from making too many hoods
                vault = ptVault()
                entry = vault.findChronicleEntry("LastHoodCreationTime")
                if type(entry) == type(None):
                    # not found... add current level chronicle
                    vault.addChronicleEntry("LastHoodCreationTime",kChronicleVarType,str(PtGetDniTime()))
                else:
                    entry.chronicleSetValue(str(PtGetDniTime()))
                    entry.save()

        elif event==PtVaultNotifyTypes.kUnRegisteredOwnedAge or event==PtVaultNotifyTypes.kUnRegisteredVisitAge:
            self.IUpdateLinks()
            PtDebugPrint( "OnVaultNotify: A link was deleted, checking if we need to retract the book" )
            try:
                clickedLinkName = ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag( idBookPresented+1 )).getString()
            except:
                clickedLinkName = None

            if boolBookPresented and not clickedLinkName: # retract book if the book we were showing was the one deleted
                PtDebugPrint( "OnVaultNotify: The currently displayed link was deleted, retracting book" )
                actLink.disable()
                idBookPresented = 0
                respBookRetract.run(self.key)
                boolBookPresented = false
                boolGetBookAfterBookRetract = false

            if boolGetBookBtnUp:
                respButtonPress.run(self.key)
                boolGetBookBtnUp = false
                self.IDisableGUIButtons() # reenable when button is done animating
        
        elif event == PtVaultNotifyTypes.kPublicAgeCreated:
            ageName = tupdata[0];
            self.publicAgeCreated(ageName);
        
        elif event == PtVaultNotifyTypes.kPublicAgeRemoved:
            ageName = tupdata[0];
            self.publicAgeRemoved(ageName);

    def OnVaultEvent(self,event,tupdata):
        if event == PtVaultCallbackTypes.kVaultNodeSaved:
            PtDebugPrint("nxuxBookMachine: kVaultNodeSaved event (id=%d,type=%d)" % (tupdata[0].getID(),tupdata[0].getType()),level=kDebugDumpLevel)
            # tupdata is ( ptVaultNode )
            if tupdata[0].getType() == PtVaultNodeTypes.kAgeInfoNode:
                if self.IGetHoodInfoNode().getID() == tupdata[0].getID():
                    self.IUpdateLinks()

    def IRandomizeNeighborhood(self,ageInfoNode):
        childRefList = ageInfoNode.getChildNodeRefList()
        for childRef in childRefList:
            child = childRef.getChild()
            if child.getType() == PtVaultNodeTypes.kSDLNode:
                sdl = child.upcastToSDLNode()
                dataRecord = sdl.getStateDataRecord()
                nameList = dataRecord.getVarList()
                for name in nameList:
                    # are we supposed to randomize this var?
                    if (name in kRandomizeVars):
                        var = dataRecord.findVar(name)
                        if var.getType() == PtSDLVarType.kBool: # there are only bool values right now
                            if whrandom.random() >= 0.5:
                                PtDebugPrint( 'IRandomizeNeighborhood: Setting var "' + name + '" to true' )
                                var.setBool(1)
                            else:
                                PtDebugPrint( 'IRandomizeNeighborhood: Setting var "' + name + '" to false' )
                                var.setBool(0)
                        else:
                            PtDebugPrint( 'IRandomizeNeighborhood: Var "' + name + '" has an unknown type! Not randomizing' )
                    elif (name in kVarRestrictions):
                        var = dataRecord.findVar(name)
                        value = int(whrandom.random() * len(kVarRestrictions[name]))
                        if (var.getType() == PtSDLVarType.kInt) or (var.getType() == PtSDLVarType.kByte):
                            PtDebugPrint( 'IRandomizeNeighborhood: Setting var "' + name + '" to ' + str(kVarRestrictions[name][value]) )
                            var.setInt(kVarRestrictions[name][value])
                        else:
                            PtDebugPrint( 'IRandomizeNeighborhood: Var "' + name + '" has an unknown type: '+str(var.getType())+' Not randomizing' )
                sdl.setStateDataRecord(dataRecord)
                sdl.saveAll()
                return;

    def OnTimer(self, id):
        if id == 99:
            PtFadeLocalAvatar(1)

    def OnNotify(self,state,id,events):
        "main non-GUI message handler"
        global avatar
        global waitingOnGUIAnim
        global idLinkSelected
        global idBookPresented
        global boolGetBookBtnUp
        global boolGetBookAfterBtnPress
        global boolBookPresented        
        global boolGetBookAfterBookRetract        
        global boolLinking
        global boolGettingBook
        global boolNewHoodDlg
        global boolGUIActivated
        
        if type(avatar) == type(None):
            PtDebugPrint("OnNotify: Initing avatar with PtGetLocalAvatar()")
            avatar = PtGetLocalAvatar()
        
        if id==(-1):
            # callback from delete yes/no dialog (hopefully)
            if boolNewHoodDlg: # user wants to create a new hood
                boolNewHoodDlg = 0
                if state: # user answered yes
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).hide()
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).disable()
                    link = self.IGetHoodLinkNode()
                    if link:
                        link.setVolatile(1) # mark it volatile, but DON'T SAVE (we don't want the server to know, just the client)
                    ptVault().createNeighborhood()
            else: # user wants to delete a link
                if state:
                    self.IDeleteLink()
            return
            
        # in no other cases do we want to take action on state = 0 events
        if not state:
            return
            
        ##################
        # Engage Machine Interface  #
        ##################

        # click on KI Slot...insert hand, light slot, remove hand, notify triggerer and move KI slot
        if id==actKISlot.id:
            avatar = PtFindAvatar(events)
            kiLevel = PtDetermineKILevel()
            print "nxusBookMachine.OnNotify:\tplayer ki level is",kiLevel
            if kiLevel < kNormalKI:
                respKISlot.run(self.key,events=events)
            else:
                PtGetPublicAgeList('city',self)
                PtGetPublicAgeList('GreatTreePub',self)
                PtGetPublicAgeList('Neighborhood02',self)
                PtGetPublicAgeList('Neighborhood',self)

                # set up the camera so when the one shot returns it gets set up right (one shot was fighting with python for camera control)
                virtCam = ptCamera()
                virtCam.undoFirstPerson()
                virtCam.disableFirstPersonOverride()
                respKISlotGlow.run(self.key,events=events)
            return
        # KI slot activated...switch to machine ops cam, fix player in idle behavior, show GUI
        if id==respKISlotGlow.id:
            behMachineOps.run(avatar)
            virtCam = ptCamera()
            virtCam.save(camMachineOps.sceneobject.getKey())
            avatar.draw.disable()
            #PtFadeLocalAvatar(1)
            #PtAtTimeCallback(self.key, 1, 99)
            if ( PtIsDialogLoaded(kNexusDialogName) ):
                NexusGUI.dialog.show()
                respGUIOn.run(self.key)
                waitingOnGUIAnim = true
            PtEnableControlKeyEvents(self.key)
            PtSendKIMessage(kDisableKIandBB,0)
        if id==respGUIOn.id:
            waitingOnGUIAnim = false
            boolGUIActivated = 1

        ##################
        # Exit Machine Interface  #
        ##################

        # catch callback from shut down GUI anim responder run by OnControlKeyEvent()
        if id==respGUIOff.id:
            boolGUIActivated = 0
            NexusGUI.dialog.hide()
            virtCam = ptCamera()
            virtCam.restore(camMachineOps.sceneobject.getKey())
            virtCam.enableFirstPersonOverride()
            avatar.draw.enable()
            #PtFadeLocalAvatar(0)
            behMachineOps.gotoStage(avatar,-1) # exit behavior
            PtDisableControlKeyEvents(self.key)
            PtSendKIMessage(kEnableKIandBB,0)
            respKISlotReturn.run(self.key)
            if boolBookPresented:
                respBookRetract.run(self.key)
                actLink.disable()
                boolBookPresented = false
                boolGetBookAfterBookRetract = false
            if boolGetBookBtnUp:
                boolGetBookAfterBtnPress = false
                respButtonPress.run(self.key)
                boolGetBookBtnUp = false
            if idLinkSelected:
                idLinkSelected = 0
                self.IUpdateLinks()
            if idBookPresented:
                idBookPresented = 0
                self.IUpdateLinks()
            return
            
        ##################
        # Machine Operations  #
        ##################

        if id==actGetBook.id:
            boolGettingBook = true
            
            #depress the button
            respButtonPress.run(self.key)
            boolGetBookBtnUp = false
            
            linkName = self.IGetUntranslatedName(idLinkSelected+1)
            if linkName == "": # this shouldn't happen, but just in case, we won't grab the book
                boolGetBookAfterBtnPress = false
                boolGetBookAfterBookRetract = false
                idLinkSelected = 0
                idBookPresented = 0
                self.IUpdateLinks()
                self.IDisableGUIButtons()
                return
            
            # determine next action
            if idBookPresented:
                boolGetBookAfterBtnPress = false
                boolGetBookAfterBookRetract = true
            else:
                boolGetBookAfterBtnPress = true
                boolGetBookAfterBookRetract = false
                
            idBookPresented = idLinkSelected
            print "idBookPresented = ",idBookPresented
            idLinkSelected = 0
            self.IUpdateLinks()

            # disable GUI Ops until Get Book Operations completed
            self.IDisableGUIButtons()
            return
        if id==respButtonPress.id:
            if boolGetBookAfterBtnPress:
                respGetBook.run(self.key)
                self.IDrawLinkPanel()
                boolBookPresented = true
            elif boolGetBookAfterBookRetract:
                respBookRetract.run(self.key)
                actLink.disable()
            else:
                self.IUpdateLinks() # reenable GUI buttons now that button is done animating
            return
        if id==respBookRetract.id and boolGetBookAfterBookRetract:
            respGetBook.run(self.key)
            self.IDrawLinkPanel()
            boolBookPresented = true
            return
        if id==respGetBook.id:
            # reenable GUI buttons
            self.IUpdateLinks()
            actLink.enable()
            boolGettingBook = false
            return

        if id==respBookSelect.id:
            self.IUpdateLinks() # reenable GUI buttons now that getBookBtn is done animating

        ##################
        # Link Me  #
        ##################

        if id==actLink.id:
            if idBookPresented == 0:
                PtDebugPrint("Ignoring link clickable since no book is shown")
                return
            respGUIOff.run(self.key)
            boolLinking = true
            actGetBook.disable()
            self.IDisableGUIButtons()
            respGUIOff.run(self.key)
            self.ILink()
            return
        if id==respLink.id:
            return
            

    ##################
    # Exit Machine Interface  #
    ##################

    def OnControlKeyEvent(self,controlKey,activeFlag):
        "exit machine op mode"
        global boolGUIActivated
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            if not boolGettingBook and boolGUIActivated:
                actGetBook.disable()
                self.IDisableGUIButtons()
                respGUIOff.run(self.key)
            return
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            if not boolGettingBook and boolGUIActivated:
                actGetBook.disable()
                self.IDisableGUIButtons()
                respGUIOff.run(self.key)
            return
        
    ##############
    # Handle GUI Clicks    #
    ##############

    def OnGUINotify(self,id,control,event):
        "Events from the Nexus GUI.."
        global idLinkSelected
        global boolGetBookBtnUp
        global idDeleteCandidateName
        global boolBookPresented
        global boolGetBookAfterBookRetract
        global idBookPresented
        global idCategorySelected
        global boolGetBookAfterBtnPress        
        global indexDisplayStart
        global dialogLoaded
        global curCityInstance
        global boolNewHoodDlg
        global oldStatusBarText
        global publicHoodSort
        global showEnglishHoods
        global showFrenchHoods
        global showGermanHoods
        global IsVisitorPlayer

        if id!=NexusGUI.id:
            print "nxusBookMachine.OnGUINotify():\tunexpected message id"
            return
            
        ##################
        # Init Machine GUI  #
        ##################

        if event == kDialogLoaded:
            dialogLoaded = 1
            return
            
        if event == kShowHide:
            #print "dialog loaded...initializing with player's KI info"
            dialogLoaded = 1
            self.IUpdateLinks()
            return
            

        if event == kAction:
            ctrlID = control.getTagID()
            print "you clicked on control ID:",ctrlID
            print "Pre Category: %i, Pre link: %i" %(idCategorySelected,idLinkSelected)

            ##################
            # Link Select Buttons  #
            ##################

            if ( ctrlID >= kIDBtnLinkSelect01 and ctrlID <= kIDBtnLinkSelect08 ) or ( ctrlID == kIDBtnNeighborhoodSelect ):
                #Prevent visitors from visiting the city...
                if IsVisitorPlayer and idCategorySelected == kIDBtnLinkCategory01 and ctrlID != kIDBtnNeighborhoodSelect:  
                    PtDebugPrint ("nxusBookMachine.OnGUINotify()-->Visitors do not have access to the city")
                    PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                    return


                if idBookPresented:
                    actLink.disable()
                    idBookPresented = 0
                    respBookRetract.run(self.key)
                    boolBookPresented = false
                    boolGetBookAfterBookRetract = false

                idLinkSelected = ctrlID
                self.IUpdateLinks()

                if not boolGetBookBtnUp:
                    respBookSelect.run(self.key)
                    boolGetBookBtnUp = true
                    self.IDisableGUIButtons() # reenable when button is done animating
                return
                
                
            ##################
            # Delete  Link Buttons  #
            ##################
                
            if ctrlID >= kIDBtnDeleteLink01 and ctrlID <= kIDBtnDeleteLink08:
                idDeleteCandidateName = ctrlID - 99
                ageName = self.IGetFullName(idDeleteCandidateName)
                stringConfirm = PtGetLocalizedString("Nexus.Messages.LinkDelete", [ageName])
                boolNewHoodDlg = 0
                PtYesNoDialog(self.key,stringConfirm)
                return
                            
            ##################
            # Category Select Buttons  #
            ##################

            if ctrlID >= kIDBtnLinkCategory01 and ctrlID <= kIDBtnLinkCategory04:
                #Visitors do not have access to private ages, make sure to show the nag dialog
                if IsVisitorPlayer and ctrlID == kIDBtnLinkCategory02:
                    PtDebugPrint ("nxusBookMachine.OnGUINotify()-->Visitors do not have access to private ages")
                    PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                    return
    

                # defensive coding vs. button mashers
                if idLinkSelected != kIDBtnNeighborhoodSelect:
                    actLink.disable()
                    
                indexDisplayStart = 0 # reset scrolling
                    
                idCategorySelected = ctrlID
                idLinkSelected = 0
                if boolGetBookBtnUp and idLinkSelected != kIDBtnNeighborhoodSelect:
                    respButtonPress.run(self.key)
                    self.IDisableGUIButtons() # disable while button animates
                    boolGetBookBtnUp = false
                    boolGetBookAfterBtnPress = false
                    return # update links (enable GUI) after button animates
                if idBookPresented and idBookPresented != kIDBtnNeighborhoodSelect:
                    respBookRetract.run(self.key)
                    boolBookPresented = false
                    boolGetBookAfterBookRetract = false                
                    idBookPresented = 0
                self.IUpdateLinks()
                return
                
            #############
            #   Scroll Buttons    #
            #############
        
            if ctrlID == kIDBtnScrollUp and indexDisplayStart > 0:
                indexDisplayStart = indexDisplayStart - 1                   
                if idLinkSelected >= kIDBtnLinkSelect01 and idLinkSelected <= kIDBtnLinkSelect08:
                    idLinkSelected = idLinkSelected + 10
                    if idLinkSelected > ( kIDBtnLinkSelect01 + ( kNumDisplayFields - 1 ) * 10 ):  # selected link scrolled off screen
                        idLinkSelected = 0
                        if boolGetBookBtnUp:
                            respButtonPress.run(self.key)
                            self.IDisableGUIButtons() # disable while button animates
                            boolGetBookBtnUp = false
                            boolGetBookAfterBtnPress = false
                if idBookPresented >= kIDBtnLinkSelect01 and idBookPresented <= kIDBtnLinkSelect08:
                    idBookPresented = idBookPresented + 10
                    if idBookPresented > ( kIDBtnLinkSelect01 + ( kNumDisplayFields - 1 ) * 10 ):  # presented book scrolled off screen
                        boolBookPresented = false
                        boolGetBookAfterBookRetract = false                
                        idBookPresented = 0
                        respBookRetract.run(self.key)
                self.IUpdateLinks()
                return

            if ctrlID == kIDBtnScrollDn:
                indexDisplayStart = indexDisplayStart + 1
                if idLinkSelected >= kIDBtnLinkSelect01 and idLinkSelected <= kIDBtnLinkSelect08:
                    idLinkSelected = idLinkSelected - 10
                    if idLinkSelected < kIDBtnLinkSelect01: # selected link scrolled off screen
                        idLinkSelected = 0
                        if boolGetBookBtnUp:
                            respButtonPress.run(self.key)
                            self.IDisableGUIButtons() # disable while button animates
                            boolGetBookBtnUp = false
                            boolGetBookAfterBtnPress = false
                if idBookPresented >= kIDBtnLinkSelect01 and idBookPresented <= kIDBtnLinkSelect08:
                    idBookPresented = idBookPresented - 10
                    print idBookPresented
                    if idBookPresented < kIDBtnLinkSelect01: # presented book scrolled off screen
                        boolBookPresented = false
                        boolGetBookAfterBookRetract = false                
                        idBookPresented = 0
                        respBookRetract.run(self.key)
                self.IUpdateLinks()
                return
                
            #############
            #   Create a Hood     #
            #############
        
            if ctrlID == kIDBtnNeighborhoodCreate:
                #Only Explorers can create Neighborhoods
                if not PtIsSubscriptionActive():
                    PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                    return
                
                boolNewHoodDlg = 1
                PtYesNoDialog(self.key,PtGetLocalizedString("Nexus.Messages.HoodCreate"))
                return
            
            if ctrlID == kIDBtnNeighborhoodPublic:
                if self.IIsMyHoodPublic():
                    self.IMakeHoodPrivate()
                else:
                    self.IMakeHoodPublic()
                # IUpdateLinks should be called automatically when the public age list updates
                return
                
            #############
            #   Header sort buttons #
            #############
        
            if ctrlID == kIDNameHeaderBtn or ctrlID == kIDNameAscArrow or ctrlID == kIDNameDescArrow:
                if publicHoodSort == kSortNameAsc:
                    publicHoodSort = kSortNameDesc
                else:
                    publicHoodSort = kSortNameAsc
                self.ISortPublicHoods()
                self.IUpdateLinks()
            
            if ctrlID == kIDPopHeaderBtn or ctrlID == kIDPopAscArrow or ctrlID == kIDPopDescArrow:
                if publicHoodSort == kSortPopAsc:
                    publicHoodSort = kSortPopDesc
                else:
                    publicHoodSort = kSortPopAsc
                self.ISortPublicHoods()
                self.IUpdateLinks()
                
            #############
            #   Language checkboxes #
            #############
        
            if ctrlID == kIDEngCheckBox or ctrlID == kIDEngCheck:
                showEnglishHoods = not showEnglishHoods
                self.ISortPublicHoods()
                self.IUpdateLinks()
            
            if ctrlID == kIDFreCheckBox or ctrlID == kIDFreCheck:
                showFrenchHoods = not showFrenchHoods
                self.ISortPublicHoods()
                self.IUpdateLinks()
            
            if ctrlID == kIDGerCheckBox or ctrlID == kIDGerCheck:
                showGermanHoods = not showGermanHoods
                self.ISortPublicHoods()
                self.IUpdateLinks()
        
        if event == kInterestingEvent:
            if type(control) != type(None):
                if control.isInteresting():
                    oldStatusBarText = ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).getStringW()
                    if self.IGetFullName(control.getTagID()) == "Garrison":
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW("Gahreesen")
                    else:   
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(self.IGetFullName(control.getTagID()))
                else:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(oldStatusBarText)
                    oldStatusBarText = U""

    def IUpdateLinks(self):
        global fullNeighborhoodName
        global fullLinkName
        global untranslatedNeighborhoodName
        global untranslatedName
        global numGZLinks
        global GZLinkNode
        global GZSpawnpoints
        
        ######################
        ##
        ##  initialize the GUI
        ##
        ######################
        
        if not dialogLoaded:
            return
        
        fullLinkName = []
        untranslatedName = []
        self.IDisableGUIButtons()

        # set category names
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkCategory01)).setForeColor(colorNormal)        
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkCategory02)).setForeColor(colorNormal)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkCategory03)).setForeColor(colorNormal)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkCategory04)).setForeColor(colorNormal)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idCategorySelected+1)).setForeColor(colorSelected)

        # clear header titles and disable their buttons
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDNameHeaderText)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDPopHeaderText)).setString("")
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameHeaderBtn)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopHeaderBtn)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameAscArrow)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameDescArrow)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopAscArrow)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopDescArrow)).hide()

        # hide the checkboxes
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDEngText)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDFreText)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDGerText)).setString("")
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDEngCheckBox)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDFreCheckBox)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDGerCheckBox)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDEngCheck)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDFreCheck)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDGerCheck)).hide()

        # enable/disable category selection buttons
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory01)).enable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory02)).enable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory03)).enable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory04)).enable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idCategorySelected)).disable()
        # clear link names - fill in below
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName01)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName02)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName03)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName04)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName05)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName06)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName07)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkName08)).setString("")
        # clear link info - fill in below
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo01)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo02)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo03)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo04)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo05)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo06)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo07)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo08)).setString("")

        # disable interesting notify if we are showing city links or instances
        if idCategorySelected == kIDBtnLinkCategory01:
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect01)).setNotifyOnInteresting(0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect02)).setNotifyOnInteresting(0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect03)).setNotifyOnInteresting(0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect04)).setNotifyOnInteresting(0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect05)).setNotifyOnInteresting(0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect06)).setNotifyOnInteresting(0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect07)).setNotifyOnInteresting(0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect08)).setNotifyOnInteresting(0)
        else:
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect01)).setNotifyOnInteresting(1)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect02)).setNotifyOnInteresting(1)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect03)).setNotifyOnInteresting(1)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect04)).setNotifyOnInteresting(1)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect05)).setNotifyOnInteresting(1)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect06)).setNotifyOnInteresting(1)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect07)).setNotifyOnInteresting(1)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect08)).setNotifyOnInteresting(1)

        # hide delete buttons - show/enable below
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink01)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink02)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink03)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink04)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink05)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink06)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink07)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink08)).hide()
        # hide scroll buttons - show/enable below
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).hide()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).hide()
        # hide create hood button
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).hide()
        
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodPublic)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodPublic)).hide()
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodPublic)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodSelect)).setNotifyOnInteresting(1)

        
        ######################
        ##
        ##  my neighborhood button
        ##
        ######################
        
        # set color of name and info
        if idBookPresented == kIDBtnNeighborhoodSelect:
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodName)).setForeColor(colorPresented)        
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodInfo)).setForeColor(colorPresented)
        else:
            if idLinkSelected == kIDBtnNeighborhoodSelect:
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodName)).setForeColor(colorSelected)        
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodInfo)).setForeColor(colorSelected)
            else:
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodName)).setForeColor(colorNormal)        
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodInfo)).setForeColor(colorNormal)
                
        # write my hood name, info (coords) and description...enable/disable selection button
        vault = ptVault()
        if type(vault) == type(None):
            PtDebugPrint("nxusBookMachine.IUpdateLinks:\tplayer vault type None")
            return
        hoodFound = 0
        PAL = vault.getAgesIOwnFolder()
        PtAssert( PAL, "vault.getAgesIOwnFolder return bad" )
        contents = PAL.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            if type(link) != type(None):
                info = link.getAgeInfo()
            else:
                link = content.getChild()
                info = link.upcastToAgeInfoNode()
            if not info: continue
            ageName = info.getAgeFilename()
            if ageName == "Neighborhood":
                hoodFound = 1
                if self.ICanCreateHood():
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).show()
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).enable()
                
                hoodLink = self.IGetHoodLinkNode()
                if not hoodLink.getVolatile():
                    # if the hood link is not volatile, then they can go
                    untranslatedNeighborhoodName = Uni(info.getDisplayName())
                    stringHoodName = xLocTools.LocalizeAgeName(info.getDisplayName())
                    fullNeighborhoodName = stringHoodName
                    if len(stringHoodName) > kMaxDisplayableChars:
                        stringHoodName = stringHoodName[:kMaxDisplayableChars] + U"..."
                    dniCoords = link.getCreateAgeCoords()
                    try:
                        stringHoodInfo = U"%05d%   04d%   04d" %(dniCoords.GetTorans(),dniCoords.getHSpans(),dniCoords.getVSpans())
                    except:
                        stringHoodInfo = U"%05d%   04d%   04d" %(0,0,0)
                    stringHoodDesc = Uni(info.getAgeDescription())
                    if idBookPresented == kIDBtnNeighborhoodSelect:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodSelect)).disable()
                    else:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodSelect)).enable()
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodPublic)).enable()
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodPublic)).show()
                    if self.IIsMyHoodPublic():
                        stringHoodPubPriv = PtGetLocalizedString("Nexus.Neighborhood.MakePrivate")
                    else:
                        stringHoodPubPriv = PtGetLocalizedString("Nexus.Neighborhood.MakePublic")
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodName)).setStringW(stringHoodName)
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodInfo)).setStringW(stringHoodInfo)
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(stringHoodDesc)
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodPublic)).setStringW(stringHoodPubPriv)
                else:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodName)).setString("")
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodInfo)).setString("")
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setString("")

        if not hoodFound:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).enable()
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodName)).setString("")
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodInfo)).setString("")
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setString("")

        if not ( idLinkSelected == kIDBtnNeighborhoodSelect or idBookPresented == kIDBtnNeighborhoodSelect ):
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setString("")

        ######################
        ##
        ##  category specific links
        ##
        ######################

        # city links have become to different to handle here...giving them their own function
        if idCategorySelected == kIDBtnLinkCategory01: # city links
            self.IUpdateCityLinks()
            return
        # get appropriate list of links depending on category selected
        folder = None
        if idCategorySelected == kIDBtnLinkCategory02: # private links
            folder = vault.getAgesICanVisitFolder()
        elif idCategorySelected == kIDBtnLinkCategory03: # public links
            self.IUpdatePublicHoods()
            return
        elif idCategorySelected == kIDBtnLinkCategory04: # personal links
            folder = vault.getAgesIOwnFolder()
        if type(folder) == type(None):
            PtDebugPrint("nxusBookMachine.IUpdateLinks:\tlink folder type None")
            return
        contents = folder.getChildNodeRefList()
        
        GZLinkNode = None
        GZSpawnpoints = []
        
        # enable/disable scrolling
        if indexDisplayStart > 0:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).enable()
        numLinks = len(contents)

        if idCategorySelected == kIDBtnLinkCategory02:
            for content in contents[:]: # [:] makes a copy of the list, so we can modify the original
                link = content.getChild()
                link = link.upcastToAgeLinkNode()
                name = link.getAgeInfo().getAgeFilename()
                if name in kHiddenAgesIfInvited:
                    PtDebugPrint("Removing link for " + name + " since it can't be used as an invite")
                    contents.remove(content)
                    numLinks -= 1
                    continue
        
        if idCategorySelected == kIDBtnLinkCategory04: # filter out links that we don't have the default spawn point for
            for content in contents[:]: # [:] makes a copy of the list, so we can modify the original
                link = content.getChild()
                link = link.upcastToAgeLinkNode()
                name = link.getAgeInfo().getAgeFilename()
                if (name in kHiddenPersonalAges) or (name in kHiddenAgesIfVisitor and IsVisitorPlayer):
                    PtDebugPrint("Removing link for " + name + " since it's in our hidden list")
                    contents.remove(content)
                    numLinks -= 1
                    continue
                
                spawnPoints = link.getSpawnPoints()
                defaultFound = false
                for spawnPoint in spawnPoints:
                    if spawnPoint.getName().lower() == "linkinpointdefault":
                        defaultFound = true
                        break                    
                if not defaultFound and name.lower() != "greatzero":
                    info = link.getAgeInfo()
                    PtDebugPrint("Removing link for " + name + " since you don't have the default link-in point")
                    contents.remove(content) # if it doesn't have the default link-in point, don't let it show
                    numLinks -= 1
            
            # GZ is in our personal links, so update our globals
            numGZLinks = 0 # reset the numGZLinks var until we can go through the spawnpoint list
            GZLinkNode = self.IGetGZLinkNode()
            if type(GZLinkNode) == type(None):
                PtDebugPrint("nxusBookMachine.IUpdateLinks:\tCouldn't find GZ link node, preventing it from showing")
            elif showGreatZero: # even if the link exists, don't show it if the vault says no
                GZSpawnpoints = GZLinkNode.getSpawnPoints()
                
                for spawnpoint in GZSpawnpoints[:]: # [:] makes a copy of the list, so we can modify the original
                    title = spawnpoint.getTitle().lower()
                    name = spawnpoint.getName().lower()
                    if (title.find("default") != -1) or (name.find("default") != -1):
                        GZSpawnpoints.remove(spawnpoint) # we hide the default spawn point, since they don't want it showing in our list
                    elif PtDetermineKIMarkerLevel() <= kKIMarkerFirstLevel:
                        # if our KI level is too low (we didn't grab markers), don't show anything
                        PtDebugPrint("nxusBookMachine.IUpdateLinks:\tHiding all GZ spawn points because you haven't found the markers yet")
                        GZSpawnpoints.remove(spawnpoint)
                    else:
                        numGZLinks += 1
                        numLinks += 1
        
        # write link names, info, set appropriate text color, enable/show related delete buttons
        idTextbox = kIDTxtLinkName01
        index = -1
        
        # if we are showing personal links, tack the GZ on first
        if (idCategorySelected == kIDBtnLinkCategory04) and (numGZLinks > 0): # only show GZ links in the personal category
            for spawnpoint in GZSpawnpoints: # already cleaned out for us, depending on a lot of state
                index = index + 1
                if index < indexDisplayStart:
                    continue
                if index > ( indexDisplayStart + kNumDisplayFields - 1 ):
                    break
                
                # setup color and clickability of the link
                if idBookPresented == (idTextbox - 1):
                    # make presented book unselectable
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox - 1)).disable()
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox + 1)).setForeColor(colorPresented)
                else:
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox - 1)).enable()
                    if idLinkSelected == idTextbox-1:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox + 1)).setForeColor(colorSelected)
                    else:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox + 1)).setForeColor(colorNormal)

                untranslatedName.append(Uni(spawnpoint.getTitle()))
                displayName = xLocTools.LocalizeAgeName(spawnpoint.getTitle())
                fullLinkName.append(displayName)
                if len(displayName) > kMaxDisplayableChars:
                    displayName = displayName[:kMaxDisplayableChars] + U"..."
                dniCoords = GZLinkNode.getCreateAgeCoords()
                try:
                    stringLinkInfo = U"%05d%   04d%   04d" %(dniCoords.GetTorans(),dniCoords.getHSpans(),dniCoords.getVSpans())
                except:
                    stringLinkInfo = U"%05d%   04d%   04d" %(0,0,0)

                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW(stringLinkInfo)

                idTextbox = idTextbox + 10
        
        # now go through all the "normal" link nodes and add them to the list
        for content in contents:
            linkNode = content.getChild()
            linkNode = linkNode.upcastToAgeLinkNode()
            if type(linkNode) != type(None):
                info = linkNode.getAgeInfo()
            else:
                # it muight be an AgeInfoNode
                linkNode = content.getChild()
                info = linkNode.upcastToAgeInfoNode()
            if type(info) == type(None):
                PtDebugPrint("nxusBookMachine: Can't find ageInfo from link",level = kErrorLevel)
                numLinks -= 1
                continue
            # don't display links that are set volatile
            if isinstance(linkNode, ptVaultAgeLinkNode) and linkNode.getVolatile():
                numLinks -= 1
                continue

            index = index + 1
            templink = content.getChild()
            templink = templink.upcastToAgeLinkNode()
            tempname = templink.getAgeInfo().getAgeFilename()

            if index < indexDisplayStart:
                continue
            if index > ( indexDisplayStart + kNumDisplayFields - 1 ):
                break
                
            if idBookPresented == ( idTextbox-1 ):
                # make presented book unselectable
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
            else:
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                if idLinkSelected == idTextbox-1:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                    if idCategorySelected == kIDBtnLinkCategory02 or idCategorySelected == kIDBtnLinkCategory03: # neighborhood links
                        # write hood description
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(Uni(info.getAgeDescription()))
                else:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)

            untranslatedName.append(Uni(info.getDisplayName()))
            displayName = xLocTools.LocalizeAgeName(info.getDisplayName())
            fullLinkName.append(displayName)
            if len(displayName) > kMaxDisplayableChars:
                displayName = displayName[:kMaxDisplayableChars] + U"..."
            
            if displayName == "Garrison":
                displayName = "Gahreesen"

            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
            dniCoords = linkNode.getCreateAgeCoords()
            try:
                stringLinkInfo = U"%05d%   04d%   04d" %(dniCoords.GetTorans(),dniCoords.getHSpans(),dniCoords.getVSpans())
            except:
                stringLinkInfo = U"%05d%   04d%   04d" %(0,0,0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW(stringLinkInfo)

            # show/enable associated delete button
            if ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).getStringW() == xLocTools.LocalizeAgeName("Ferry Terminal"):
                idTextbox = idTextbox + 10
            elif idCategorySelected == kIDBtnLinkCategory03 or idCategorySelected == kIDBtnLinkCategory04: # can't delete public hoods or personal links
                idTextbox = idTextbox + 10
            else:
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox+99)).show()
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox+99)).enable()
                idTextbox = idTextbox + 10

        # update the scroll down button
        print "NUMLINKS: %d" % (numLinks)

        if numLinks > ( kNumDisplayFields + indexDisplayStart ):
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).enable()

    def IUpdatePublicHoods(self):
        global publicHoods
        global publicHoodsPop
        global sortedHoods
        global sortedHoodsPop

        # show our header sorting buttons
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDNameHeaderText)).setStringW(PtGetLocalizedString("Nexus.Headers.Name"))
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDPopHeaderText)).setStringW(PtGetLocalizedString("Nexus.Headers.Population"))
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameHeaderBtn)).enable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopHeaderBtn)).enable()
        
        # show the language filter checkboxes
        # Disabling all language filters since we are not localizing for now
        """ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDEngText)).setString("English")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDFreText)).setString("Francais")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDGerText)).setString("Deutsch")
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDEngCheckBox)).show()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDEngCheckBox)).enable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDFreCheckBox)).show()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDFreCheckBox)).enable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDGerCheckBox)).show()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDGerCheckBox)).enable()
        # show the checks depending on the status
        if showEnglishHoods:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDEngCheck)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDEngCheck)).enable()
        if showFrenchHoods:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDFreCheck)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDFreCheck)).enable()
        if showGermanHoods:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDGerCheck)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDGerCheck)).enable()"""

        # show the correct arrow depending on our sort method
        if publicHoodSort == kSortNameAsc:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameAscArrow)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameAscArrow)).enable()
        elif publicHoodSort == kSortNameDesc:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameDescArrow)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameDescArrow)).enable()
        elif publicHoodSort == kSortPopAsc:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopAscArrow)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopAscArrow)).enable()
        elif publicHoodSort == kSortPopDesc:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopDescArrow)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopDescArrow)).enable()

        # enable/disable scrolling
        if indexDisplayStart > 0:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).enable()
        numLinks = len(sortedHoods)
        print "NUMLINKS: %d" % (numLinks)
        if numLinks > ( kNumDisplayFields + indexDisplayStart ):
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).enable()
            
        # write link names, info, set appropriate text color, enable/show related delete buttons
        idTextbox = kIDTxtLinkName01
        index = 0
        for hood in sortedHoods:
            if index < indexDisplayStart:
                index = index + 1
                continue
            if index > ( indexDisplayStart + kNumDisplayFields - 1 ):
                break

            if idBookPresented == ( idTextbox-1 ):
                # make presented book unselectable
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
            else:
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                if idLinkSelected == idTextbox-1:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                else:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)

            untranslatedName.append(Uni(hood.getDisplayName()))
            displayName = xLocTools.LocalizeAgeName(hood.getDisplayName())
            fullLinkName.append(displayName)
            if len(displayName) > kMaxDisplayableChars:
                displayName = displayName[:kMaxDisplayableChars] + U"..."

            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
            stringLinkInfo = str(sortedHoodsPop[index])
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setString(stringLinkInfo)
            idTextbox = idTextbox + 10
            index = index + 1

    def IUpdateCityLinks(self):
        global curCityInstance
        global numCityLinks
        global numGZLinks
        global GZLinkNode
        global childAgeLinkNodes
        global numPubLinks
        global numKirelLinks
        global numGuildPubLinks
        global numKveerPublicLinks

        if len(cityPopulations) == 0:
            return

        self.ISyncCurCityInstanceAndVault()

        # find my city link and its associated spawn points
        vault = ptVault()
        if type(vault) == type(None):
            PtDebugPrint("nxusBookMachine.IUpdateCityLinks:\tWARNING player vault type None")
            return
        cityLink = vault.getLinkToCity()
        spawnpoints = cityLink.getSpawnPoints()

        # Don't display spawn points that shouldn't be shown
        for sp in spawnpoints[:]:
            t = sp.getTitle()
            if t in kHiddenCityLinks or t == "Default":
                spawnpoints.remove(sp)
        
        numCityLinks = len(spawnpoints)
        
        cityDisabled = (cityPopulations[curCityInstance] >= maxCityPopulation)
        pubDisabled = (pubPopulation >= maxPubPopulation)
        kirelDisabled = (kirelPopulation >= maxKirelPopulation)
        guildPubDisabled = (guildPubPopulation >= maxGuildPubPopulation)
        kveerPublicDisabled = (kveerPublicPopulation >= maxKveerPublicPopulation)
        
        # write link names, info, set appropriate text color, enable/show related delete buttons
        idTextbox = kIDTxtLinkName01
        index = -1
        for spawnpoint in spawnpoints:
            index = index + 1
            if index < indexDisplayStart:
                continue
            if index > ( indexDisplayStart + kNumDisplayFields - 1 ):
                break

            # setup color and clickability of the link
            if idBookPresented == ( idTextbox-1 ):
                # make presented book unselectable
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
            else:
                if cityDisabled:
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                else:
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                if idLinkSelected == idTextbox-1:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                else:
                    if cityDisabled:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorDisabled)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorDisabled)
                    else:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)
            
            untranslatedName.append(Uni(spawnpoint.getTitle()))
            displayName = xLocTools.LocalizeAgeName(spawnpoint.getTitle())
            fullLinkName.append(displayName)
            if len(displayName) > kMaxDisplayableChars:
                displayName = displayName[:kMaxDisplayableChars] + U"..."
            
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
            dniCoords = cityLink.getCreateAgeCoords()
            try:
                stringLinkInfo = U"%05d%   04d%   04d" %(dniCoords.GetTorans(),dniCoords.getHSpans(),dniCoords.getVSpans())
            except:
                stringLinkInfo = U"%05d%   04d%   04d" %(0,0,0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW(stringLinkInfo)

            # show/enable associated delete button
            if not showCityLinks: # we aren't showing city links, so reset this box
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setString("")
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setString("")
                # we don't increment the textbox id since we will be re-using this box for the next link (if there is one)
                # and we decrement the index because it will be incremented at the beginning of the loop
                index = index - 1
                numCityLinks -= 1 # we didn't show this link, decrement
            else:
                if ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).getStringW() == xLocTools.LocalizeAgeName("Ferry Terminal"):
                    idTextbox = idTextbox + 10
                else:
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox+99)).show()
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox+99)).enable()
                    idTextbox = idTextbox + 10
        
        # var so we know if they have selected the pub link, and therefore need to display the pub population
        pubIsSelected = 0
        numPubLinks = 0
        
        # now handle the pub link
        if showPub and not (type(pubInstance) == type(None)):
            numPubLinks = 1
            index = index + 1
            print "idLinkSelected: ", idLinkSelected, " idTextbox-1: ", idTextbox-1
            if (index >= indexDisplayStart) and (index <= (indexDisplayStart + kNumDisplayFields - 1)):
                # in range
                # setup color and clickability of the link
                if idBookPresented == ( idTextbox-1 ):
                    # make presented book unselectable
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
                    pubIsSelected = 1
                else:
                    if pubDisabled:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    else:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                    if idLinkSelected == idTextbox-1:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                        pubIsSelected = 1
                    else:
                        if pubDisabled:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorDisabled)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorDisabled)
                        else:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)
                
                untranslatedName.append(Uni(pubInstance.getDisplayName()))
                displayName = xLocTools.LocalizeAgeName(pubInstance.getDisplayName())
                fullLinkName.append(displayName)
                if len(displayName) > kMaxDisplayableChars:
                    displayName = displayName[:kMaxDisplayableChars] + U"..."
                
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW("")
                
                # do NOT show or enable the delete button, but go to the next textbox
                idTextbox = idTextbox + 10

        # var so we know if they have selected the kirel link, and therefore need to display the kirel population
        kirelIsSelected = 0
        numKirelLinks = 0
        
        # now handle the kirel link
        if showKirel and not (type(kirelInstance) == type(None)):
            numKirelLinks = 1
            index = index + 1
            print "idLinkSelected: ", idLinkSelected, " idTextbox-1: ", idTextbox-1
            if (index >= indexDisplayStart) and (index <= (indexDisplayStart + kNumDisplayFields - 1)):
                # in range
                # setup color and clickability of the link
                if idBookPresented == ( idTextbox-1 ):
                    # make presented book unselectable
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
                    kirelIsSelected = 1
                else:
                    if kirelDisabled:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    else:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                    if idLinkSelected == idTextbox-1:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                        kirelIsSelected = 1
                    else:
                        if kirelDisabled:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorDisabled)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorDisabled)
                        else:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)
                
                untranslatedName.append(Uni(kirelInstance.getDisplayName()))
                displayName = xLocTools.LocalizeAgeName(kirelInstance.getDisplayName())
                fullLinkName.append(displayName)
                if len(displayName) > kMaxDisplayableChars:
                    displayName = displayName[:kMaxDisplayableChars] + U"..."
                
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW("")
                
                # do NOT show or enable the delete button, but go to the next textbox
                idTextbox = idTextbox + 10

        # var so we know if they have selected the guild-pub link, and therefore need to display the kirel population - tho we can't
        guildPubIsSelected = 0
        numGuildPubLinks = 0
        
        # now handle the GuildPub link
        gp = None
        if showGuildPub:
            gp = self.IGetAgeInfo("guildPub")
        if (type(gp) != type(None)) and maxGuildPubPopulation:
            numGuildPubLinks = 1
            index = index + 1
            print "idLinkSelected: ", idLinkSelected, " idTextbox-1: ", idTextbox-1
            if (index >= indexDisplayStart) and (index <= (indexDisplayStart + kNumDisplayFields - 1)):
                # in range
                # setup color and clickability of the link
                if idBookPresented == ( idTextbox-1 ):
                    # make presented book unselectable
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
                    guildPubIsSelected = 1
                else:
                    if guildPubDisabled:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    else:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                    if idLinkSelected == idTextbox-1:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                        guildPubIsSelected = 1
                    else:
                        if guildPubDisabled:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorDisabled)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorDisabled)
                        else:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)
                
                untranslatedName.append(Uni(gp.getDisplayName()))
                displayName = xLocTools.LocalizeAgeName(gp.getDisplayName())
                guild = guildPubs[showGuildPub-1]
                displayName = "The %s' Pub" % (guild)
                fullLinkName.append(displayName)
                if len(displayName) > kMaxDisplayableChars:
                    displayName = displayName[:kMaxDisplayableChars] + U"..."
                #displayName = "GuildPub-" + guildPubs[showGuildPub-1]
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW("")
                
                # do NOT show or enable the delete button, but go to the next textbox
                idTextbox = idTextbox + 10

        # var so we know if they have selected the public Kveer link, and therefore need to display the public Kveer population - tho we can't
        kveerPublicIsSelected = 0
        numKveerPublicLinks = 0

        # now handle the public Kveer link
        kv = None
        if showKveerPublic:
            kv = self.IGetAgeInfo("kveerPublic")
        if (type(kv) != type(None)) and maxKveerPublicPopulation:
            numKveerPublicLinks = 1
            index = index + 1
            print "idLinkSelected: ", idLinkSelected, " idTextbox-1: ", idTextbox-1
            if (index >= indexDisplayStart) and (index <= (indexDisplayStart + kNumDisplayFields - 1)):
                # in range
                # setup color and clickability of the link
                if idBookPresented == ( idTextbox-1 ):
                    # make presented book unselectable
                    ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
                    kveerPublicIsSelected = 1
                else:
                    if kveerPublicDisabled:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                    else:
                        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                    if idLinkSelected == idTextbox-1:
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                        kveerPublicIsSelected = 1
                    else:
                        if kveerPublicDisabled:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorDisabled)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorDisabled)
                        else:
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)

                untranslatedName.append(Uni(kv.getDisplayName()))
                displayName = xLocTools.LocalizeAgeName(kv.getDisplayName())
                if displayName == "Kveer":
                    displayName = "K'veer"
                fullLinkName.append(displayName)
                if len(displayName) > kMaxDisplayableChars:
                    displayName = displayName[:kMaxDisplayableChars] + U"..."
                #displayName = "GuildPub-" + guildPubs[showGuildPub-1]
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW("")
                
                # do NOT show or enable the delete button, but go to the next textbox
                idTextbox = idTextbox + 10

        # now we handle any leftover children
        for linkNode in childAgeLinkNodes:
            index = index + 1
            if index < indexDisplayStart:
                continue
            if index > ( indexDisplayStart + kNumDisplayFields - 1 ):
                break

            # setup color and clickability of the link
            if idBookPresented == ( idTextbox-1 ):
                # make presented book unselectable
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).disable()
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorPresented)        
                ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorPresented)
            else:
                ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox-1)).enable()
                if idLinkSelected == idTextbox-1:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorSelected)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorSelected)
                else:
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setForeColor(colorNormal)        
                    ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setForeColor(colorNormal)

            untranslatedName.append(Uni(linkNode.getAgeInfo().getDisplayName()))
            displayName = xLocTools.LocalizeAgeName(linkNode.getAgeInfo().getDisplayName())
            fullLinkName.append(displayName)
            if len(displayName) > kMaxDisplayableChars:
                displayName = displayName[:kMaxDisplayableChars] + U"..."
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox)).setStringW(displayName)

            dniCoords = linkNode.getCreateAgeCoords()
            try:
                stringLinkInfo = U"%05d%   04d%   04d" %(dniCoords.GetTorans(),dniCoords.getHSpans(),dniCoords.getVSpans())
            except:
                stringLinkInfo = U"%05d%   04d%   04d" %(0,0,0)
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTextbox+1)).setStringW(stringLinkInfo)

            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox+99)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idTextbox+99)).enable()
            idTextbox = idTextbox + 10
        
        # update the "status bar" text
        if pubIsSelected:
            if pubDisabled:
                pubPopString = PtGetLocalizedString("Nexus.Messages.PubFull")
            else:
                pubPopString = PtGetLocalizedString("Nexus.Messages.PubPopulation", [str(pubPopulation), str(maxPubPopulation)])
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(pubPopString)
        elif kirelIsSelected:
            if kirelDisabled:
                kirelPopString = PtGetLocalizedString("Nexus.Messages.KirelFull")
            else:
                kirelPopString = PtGetLocalizedString("Nexus.Messages.KirelPopulation", [str(kirelPopulation), str(maxKirelPopulation)])
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(kirelPopString)
        elif guildPubIsSelected:
            print "guildPub is selected, but no pop data available"
            pass
#            if guildPubDisabled:
#                gp = "GuildPub-" + guildPubs[showGuildPub-1]
#                guildPubPopString = PtGetLocalizedString("Nexus.Messages.GuildPubFull", [gp])
#            else:
#                #guildPubPopString = PtGetLocalizedString("Nexus.Messages.GuildPubPopulation", [str(guildPubPopulation), str(maxGuildPubPopulation)])
#                guildPubPopString = PtGetLocalizedString("Nexus.Messages.GuildPubPopulation", ["*", "*"])
#            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(guildPubPopString)
        elif kveerPublicIsSelected:
            print "kveerPublic is selected, but no pop data available"
            pass
        else:
            if cityDisabled:
                cityPopString = PtGetLocalizedString("Nexus.Messages.CityFull")
            else:
                cityPopString = PtGetLocalizedString("Nexus.Messages.CityPopulation", [str(cityPopulations[curCityInstance]), str(maxCityPopulation)])
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription)).setStringW(cityPopString)
        
        # update scroll buttons
        numLinks = numCityLinks + len(childAgeLinkNodes) + numPubLinks + numKirelLinks + numGuildPubLinks + numKveerPublicLinks
        print "NUMLINKS: %d" % (numLinks)

        if indexDisplayStart > 0:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).enable()
        if numLinks > ( kNumDisplayFields + indexDisplayStart ):
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).show()
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).enable()
    
    def IGetFullName(self,controlID):
        "Get the full link name, because the one displayed may be truncated"
        global fullLinkName
        global fullNeighborhoodName
        
        if controlID == kIDTxtNeighborhoodName or controlID == kIDBtnNeighborhoodSelect:
            return fullNeighborhoodName
        
        # the id #s range from 211 to 281 (for text boxes) or 210 to 280 (for buttons), incrementing in 10s
        # we take advantage of integer division so that (211-210)/10 == (210-210)/10
        # indexDisplayStart contains the offset in case we are scrolling, with 0 being at the very top of the list (so no offset)
        index = controlID - 210
        # now index is from 00 to 70 (or 01 to 71 if text box)
        index /= 10
        # now index is from 0 to 7
        if index < len(fullLinkName):
            return fullLinkName[index]
        else:
            return U""
    
    def IGetUntranslatedName(self,controlID):
        global untranslatedName
        global untranslatedNeighborhoodName
        
        if controlID == kIDTxtNeighborhoodName or controlID == kIDBtnNeighborhoodSelect:
            return untranslatedNeighborhoodName
        
        # the id #s range from 211 to 281 (for text boxes) or 210 to 280 (for buttons), incrementing in 10s
        # we take advantage of integer division so that (211-210)/10 == (210-210)/10
        # indexDisplayStart contains the offset in case we are scrolling, with 0 being at the very top of the list (so no offset)
        index = controlID - 210
        # now index is from 00 to 70 (or 01 to 71 if text box)
        index /= 10
        # now index is from 0 to 7
        try:
            return untranslatedName[index]
        except IndexError:
            return U""

    def ILink(self):
        global curCityInstance
        global cityInstances
        global GZLinkNode
        global GZSpawnpoints
        
        vault = ptVault()
        if type(vault) == type(None):
            PtDebugPrint("nxusBookMachine.ILink:\tplayer vault type None")
            return
        
        if idBookPresented == 0:
            PtDebugPrint("Ignoring link request since idBookPresented is 0 (which is invalid)")
            return

        # my neighborhood link
        if idBookPresented == kIDBtnNeighborhoodSelect:
            folder = vault.getAgesIOwnFolder()
            PtAssert( folder, "vault.getAgesIOwnFolder return bad" )
            contents = folder.getChildNodeRefList()
            for content in contents:
                link = content.getChild()
                link = link.upcastToAgeLinkNode()
                if type(link) != type(None):
                    info = link.getAgeInfo()
                else:
                    link = content.getChild()
                    info = link.upcastToAgeInfoNode()
                if type(info) == type(None): continue
                if info.getAgeFilename() == "Neighborhood":
                    als = link.asAgeLinkStruct()
                    als.setLinkingRules( PtLinkingRules.kOwnedBook )
                    linkMgr = ptNetLinkingMgr()
                    linkMgr.linkToAge(als)
                    return
            print "nxusBookMachine.ILink:\tERROR--couldn't find link to player's neighborhood in player's Vault"
            return

        # get appropriate list of links depending on category selected
        folder = None
        if idCategorySelected == kIDBtnLinkCategory01: # city links
            self.ILinkToCity()
            return
        elif idCategorySelected == kIDBtnLinkCategory02: # private links
            folder = vault.getAgesICanVisitFolder()
        elif idCategorySelected == kIDBtnLinkCategory03: # public links
            self.ILinkToPublicHood()
            return
        elif idCategorySelected == kIDBtnLinkCategory04: # personal links
            folder = vault.getAgesIOwnFolder()
        if type(folder) == type(None):
            PtDebugPrint("nxusBookMachine.ILink:\tWARNING link folder type None")
            return
        contents = folder.getChildNodeRefList()
        
        # see if they want to go to the GZ
        if (idCategorySelected == kIDBtnLinkCategory04):
            index = idBookPresented + 1
            index = index - 210
            index /= 10 # index is now from 0 to 7
            index += indexDisplayStart
            PtDebugPrint("nxusBookMachine.ILink():\tindex: "+str(index)+" numGZLinks: "+str(numGZLinks))
            
            if (index < numGZLinks):
                # going to the GZ!
                info = self.IGetGZInfoNode()
                if type(info) == type(None):
                    PtDebugPrint("nxusBookMachine.ILink():\tWARNING--unable to find GZ info node, aborting link")
                    return
                
                if type(GZLinkNode) == type(None):
                    PtDebugPrint("nxusBookMachine.ILink():\tWARNING--GZ link node is bad, aborting link")
                    return
                if index >= len(GZSpawnpoints):
                    PtDebugPrint("nxusBookMachine.ILink():\tWARNING--GZ spawnpoint index out of range, aborting link")
                    return
                
                als = GZLinkNode.asAgeLinkStruct()
                als.setSpawnPoint(GZSpawnpoints[index])
                als.setLinkingRules(PtLinkingRules.kChildAgeBook)
                als.setParentAgeFilename("Neighborhood")
                
                print "-- linking --"
                linkMgr = ptNetLinkingMgr()
                linkMgr.linkToAge(als)
                return

        # do the link
        clickedLinkName = self.IGetUntranslatedName(idBookPresented + 1)
        PtDebugPrint( "nxusBookMachine.ILink():\tattempting link to %s" % (clickedLinkName))
        if not clickedLinkName:
            PtDebugPrint("nxusBookMachine.ILink():\tWARNING--Link name is empty, aborting link")
            return
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            if type(link) != type(None):
                info = link.getAgeInfo()
            else:
                link = content.getChild()
                info = link.upcastToAgeInfoNode()
            if type(info) == type(None): continue
            if idCategorySelected == kIDBtnLinkCategory01: # city links
                thisLinkName = info.getAgeFilename()
            else:
                thisLinkName = info.getDisplayName()
            if thisLinkName == clickedLinkName:
                print "-- found link node --"
                thisAge = info.getAgeFilename()
                if (thisAge == "Ercana" or thisAge == "AhnonayCathedral" or thisAge == "Ahnonay") and (idCategorySelected != kIDBtnLinkCategory02): #  check to make sure it's a non-private link
                    self.DoErcanaAndAhnonayStuff(thisAge)
                if isinstance(link,ptVaultAgeLinkNode):
                    als = link.asAgeLinkStruct()
                else:
                    als = ptAgeLinkStruct()
                    als.setAgeInfo(info.asAgeInfoStruct())
                if idCategorySelected == kIDBtnLinkCategory02: # private links
                    als.setLinkingRules( PtLinkingRules.kVisitBook )
                elif idCategorySelected == kIDBtnLinkCategory04: # personal links
                    als.setLinkingRules( PtLinkingRules.kOwnedBook )
                else:
                    als.setLinkingRules( PtLinkingRules.kBasicLink ) # public hood links
                print "-- linking --"
                linkMgr = ptNetLinkingMgr()
                linkMgr.linkToAge(als)
                return
                
        PtDebugPrint( "nxusBookMachine.ILink():\tWARNING--couldn't find link to %s in player's Vault" % (clickedLinkName))
        return
    
    def ILinkToCity(self):
        vault = ptVault()
        if type(vault) == type(None):
            PtDebugPrint("nxusBookMachine.ILinkToCity:\tplayer vault type None")
            return
        index = idBookPresented + 1
        index = index - 210
        index /= 10 # index is now from 0 to 7, but...
        index += indexDisplayStart  # we need to account for scrolling down, as Kveer (and any additional links) requires it
        PtDebugPrint("nxusBookMachine.ILinkToCity:\tindex: "+str(index)+" numCityLinks: "+str(numCityLinks)+" numPubLinks: "+str(numPubLinks)+" numKirelLinks: "+str(numKirelLinks)+" numGuildPubLinks: "+str(numGuildPubLinks)+" numKveerPublicLinks: "+str(numKveerPublicLinks))

        if index < numCityLinks: # it's a city link
            spawnpointtitle = self.IGetUntranslatedName(idBookPresented + 1)
            PtDebugPrint("nxusBookMachine.ILinkToCity():\tattempting link to city's %s" % (spawnpointtitle))
            folder = vault.getAgesIOwnFolder()
            contents = folder.getChildNodeRefList()
            for content in contents:
                link = content.getChild()
                link = link.upcastToAgeLinkNode()
                if type(link) != type(None):
                    info = link.getAgeInfo()
                else:
                    link = content.getChild()
                    info = link.upcastToAgeInfoNode()
                if type(info) == type(None): continue
                thisLinkName = info.getAgeFilename()
                if thisLinkName == U"city":
                    print "-- found link node --"
                    if isinstance(link,ptVaultAgeLinkNode):
                        als = link.asAgeLinkStruct()
                    else:
                        als = ptAgeLinkStruct()
                        als.setAgeInfo(info.asAgeInfoStruct())
                    if not curCityInstance == -1: # we aren't using the default city
                        als.getAgeInfo().copyFrom(cityInstances[curCityInstance])
                    als.setLinkingRules( PtLinkingRules.kBasicLink )
                    spawnpoints = link.getSpawnPoints()
                    foundspawnpoint = false
                    for spawnpoint in spawnpoints:
                        if spawnpoint.getTitle() == spawnpointtitle:
                            print "-- found spawn point --"
                            als.setSpawnPoint( spawnpoint )
                            foundspawnpoint = true
                    if not foundspawnpoint:
                        PtDebugPrint("nxusBookMachine.ILinkToCity():\tWARNING couldn't find city spawn point for %s" % (spawnpointtitle))
                        return
                    print "-- linking --"
                    linkMgr = ptNetLinkingMgr()
                    linkMgr.linkToAge(als)
                    return
        elif index < numCityLinks + numPubLinks: # it's a pub link
            ageName = self.IGetUntranslatedName(idBookPresented + 1)
            PtDebugPrint("nxusBookMachine.ILinkToCity():\tattempting link to %s" % (ageName))
            if not ageName:
                PtDebugPrint("Link name is empty, aborting link")
                return
            if type(pubInstance) == type(None):
                PtDebugPrint("Pub instance is none, aborting link")
                return;
            info = ptAgeInfoStruct()
            info.copyFrom(pubInstance)
            als = ptAgeLinkStruct()
            als.setAgeInfo(info)
            als.setLinkingRules(PtLinkingRules.kBasicLink)
            print "-- linking --"
            linkMgr = ptNetLinkingMgr()
            linkMgr.linkToAge(als)
            return
        elif index < numCityLinks + numPubLinks + numKirelLinks: # it's a kirel link
            ageName = self.IGetUntranslatedName(idBookPresented + 1)
            PtDebugPrint("nxusBookMachine.ILinkToCity():\tattempting link to %s" % (ageName))
            if not ageName:
                PtDebugPrint("Link name is empty, aborting link")
                return
            if type(kirelInstance) == type(None):
                PtDebugPrint("Kirel instance is none, aborting link")
                return;
            info = ptAgeInfoStruct()
            info.copyFrom(kirelInstance)
            als = ptAgeLinkStruct()
            als.setAgeInfo(info)
            als.setLinkingRules(PtLinkingRules.kBasicLink)
            print "-- linking --"
            linkMgr = ptNetLinkingMgr()
            linkMgr.linkToAge(als)
            return
        elif index < numCityLinks + numPubLinks + numKirelLinks + numGuildPubLinks: # it's a GuildPub link
            ageName = self.IGetUntranslatedName(idBookPresented + 1)
            if ageName[:1] == " ":
                ageName = ageName[1:]   #remove the stupid blank first space
            PtDebugPrint("nxusBookMachine.ILinkToCity():\tattempting link to %s" % (ageName))
            if not ageName:
                PtDebugPrint("Link name is empty, aborting link")
                return
            gp = self.IGetAgeInfo("guildPub")
            if type(gp) == type(None):
                PtDebugPrint("GuildPub instance is none, aborting link")
                return;
            info = ptAgeInfoStruct()
            info.copyFrom(gp)
            als = ptAgeLinkStruct()
            als.setAgeInfo(info)
            als.setLinkingRules(PtLinkingRules.kBasicLink)
            print "-- linking --"
            linkMgr = ptNetLinkingMgr()
            linkMgr.linkToAge(als)
            return
        elif index < numCityLinks + numPubLinks + numKirelLinks + numGuildPubLinks + numKveerPublicLinks: # it's a public Kveer link
            ageName = self.IGetUntranslatedName(idBookPresented + 1)
            if ageName[:1] == " ":
                ageName = ageName[1:]   #remove the stupid blank first space
            PtDebugPrint("nxusBookMachine.ILinkToCity():\tattempting link to %s" % (ageName))
            if not ageName:
                PtDebugPrint("Link name is empty, aborting link")
                return
            kv = self.IGetAgeInfo("kveerPublic")
            if type(kv) == type(None):
                PtDebugPrint("public Kveer instance is none, aborting link")
                return;
            info = ptAgeInfoStruct()
            info.copyFrom(kv)
            als = ptAgeLinkStruct()
            als.setAgeInfo(info)
            als.setLinkingRules(PtLinkingRules.kBasicLink)
            print "-- linking --"
            linkMgr = ptNetLinkingMgr()
            linkMgr.linkToAge(als)
            return        
        else: # it's a child age link
            ageName = self.IGetUntranslatedName(idBookPresented + 1)
            PtDebugPrint("nxusBookMachine.ILinkToCity():\tattempting link to %s" % (ageName))
            if not ageName:
                PtDebugPrint("Link name is empty, aborting link")
                return
            for linkNode in childAgeLinkNodes:
                info = linkNode.getAgeInfo()
                if ageName == info.getDisplayName():
                    als = linkNode.asAgeLinkStruct()
                    als.setLinkingRules( PtLinkingRules.kChildAgeBook )
                    als.setParentAgeFilename("Neighborhood")
                    print "-- linking --"
                    linkMgr = ptNetLinkingMgr()
                    linkMgr.linkToAge(als)
                    return
        PtDebugPrint("nxusBookMachine.ILinkToCity():\tWARNING--couldn't find link in player's Vault")
        return

    def ILinkToPublicHood(self):
        global publicHoods

        clickedLinkName = self.IGetUntranslatedName(idBookPresented+1)
        PtDebugPrint("nxusBookMachine.ILinkToPublicHood():\tattempting link to %s" % (clickedLinkName))
        if not clickedLinkName:
            PtDebugPrint("Link name is empty, aborting link")
            return

        for hood in publicHoods:
            thisLinkName = hood.getDisplayName()
            if thisLinkName == clickedLinkName:
                als = ptAgeLinkStruct()
                als.setAgeInfo(hood)
                als.setLinkingRules(PtLinkingRules.kBasicLink)
                linkMgr = ptNetLinkingMgr()
                linkMgr.linkToAge(als)
                break

    def IDisableGUIButtons(self):
        # disable delete buttons
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink01)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink02)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink03)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink04)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink05)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink06)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink07)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnDeleteLink08)).disable()
        # disable link select buttons  buttons
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect01)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect02)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect03)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect04)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect05)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect06)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect07)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkSelect08)).disable()
        # disable hood buttons
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodSelect)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodPublic)).disable()
        # disable category buttons
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory01)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory02)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory03)).disable()        
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnLinkCategory04)).disable()       
        # disable scroll buttons
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollDn)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnScrollUp)).disable()
        # disable header buttons
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameHeaderBtn)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopHeaderBtn)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameAscArrow)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameDescArrow)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopAscArrow)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopDescArrow)).disable()
        # disable checkboxes
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDEngCheckBox)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDFreCheckBox)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDGerCheckBox)).disable()
        
        # disable interesting notifys
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo01)).setNotifyOnInteresting(0)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo02)).setNotifyOnInteresting(0)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo03)).setNotifyOnInteresting(0)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo04)).setNotifyOnInteresting(0)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo05)).setNotifyOnInteresting(0)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo06)).setNotifyOnInteresting(0)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo07)).setNotifyOnInteresting(0)
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkInfo08)).setNotifyOnInteresting(0)
        return
        
    def IDeleteLink(self):
        global boolGetBookAfterBookRetract
        global idLinkSelected
        global idBookPresented
        global boolBookPresented
        global boolGetBookBtnUp
        global boolGetBookAfterBtnPress
        
        actLink.disable()
        
        # determine who we're trying to delete
        stringDeleteCandidateName = self.IGetUntranslatedName(idDeleteCandidateName)
        vault = ptVault()
        if type(vault) == type(None):
            PtDebugPrint("nxusBookMachine.IDeleteLink:\tplayer vault type None")
            return
        
        # do the deletion
        vault = ptVault()
        if idCategorySelected == kIDBtnLinkCategory01: # city links
            cityLink = vault.getLinkToCity()
            spawnPoints = cityLink.getSpawnPoints()
            deletionCompleted = 0
            for spawnPoint in spawnPoints:
                if spawnPoint.getTitle()==stringDeleteCandidateName:
                    PtDebugPrint("Deleting link to "+stringDeleteCandidateName)
                    cityLink.removeSpawnPoint(spawnPoint)
                    cityLink.save()
                    deletionCompleted = 1
                    break
            if not deletionCompleted: # this must be a child age link
                for linkNode in childAgeLinkNodes:
                    info = linkNode.getAgeInfo()
                    if stringDeleteCandidateName == info.getDisplayName():
                        PtDebugPrint("Deleting link to "+stringDeleteCandidateName)
                        guid = info.getAgeInstanceGuid()
                        vault.unRegisterVisitAge(guid)
                        break
        elif idCategorySelected == kIDBtnLinkCategory02: # private links
            folder = vault.getAgesICanVisitFolder()
            contents = folder.getChildNodeRefList()
            for content in contents:
                linkNode = content.getChild()
                linkNode = linkNode.upcastToAgeLinkNode()
                if type(linkNode) == type(None):
                    linkNode = content.getChild()
                    info = linkNode.upcastToAgeInfoNode()
                else:
                    info = linkNode.getAgeInfo()
                if type(info) == type(None): continue
                if stringDeleteCandidateName == info.getDisplayName():
                    PtDebugPrint("Deleting link to "+stringDeleteCandidateName)
                    guid = info.getAgeInstanceGuid()
                    vault.unRegisterVisitAge(guid)
                    break
        else:
            PtDebugPrint("nxusBookMachine.IDeleteLink:\tdeletion of this link type not supported")
            
        # update GUI data and machine state
        if ( idDeleteCandidateName-1 ) == idBookPresented:
            respBookRetract.run(self.key)
            boolGetBookAfterBookRetract = false
            boolBookPresented = false
            idBookPresented = 0
        if ( idDeleteCandidateName-1 ) == idLinkSelected:
            respButtonPress.run(self.key)
            boolGetBookBtnUp = false
            boolGetBookAfterBtnPress = false
            idLinkSelected = 0
            return # update links (enable GUI) after button animates
        if ( idDeleteCandidateName-1) < idLinkSelected:
            idLinkSelected = idLinkSelected - 10
        if ( idDeleteCandidateName-1) < idBookPresented:
            idBookPresented = idBookPresented - 10
            
        self.IUpdateLinks()
        return
        
    def IDrawLinkPanel(self):
        # first hide them all
        for objPanel in objlistLinkPanels.value:
            #print "hiding link panel: ",objPanel.getName()
            objPanel.draw.disable()

        # now draw correct panel
        if ( idBookPresented == kIDBtnNeighborhoodSelect ) or ( idCategorySelected == kIDBtnLinkCategory03 ): # Neighborhoods
            panelName = U"LinkPanel_Neighborhood"
        elif idCategorySelected == kIDBtnLinkCategory01: # City links
            temp = self.IGetUntranslatedName(idBookPresented+1)
            if temp == "Tokotah Alley": # some genius had this thing misspelled as dakota for a LONG time, so we hack it in here
                panelName = U"LinkPanel_Dakotah Alley"
            elif temp == " The Watcher's Pub": # apparently this name has a space preceeding it, AND isn't named the same as the linking panel... go figure
                panelName = U"LinkPanel_" + "GreatTreePub"
            elif temp == " Kirel": # apparently this name has a space preceeding it, AND isn't named the same as the linking panel... go figure
                panelName = U"LinkPanel_" + "Kirel"
            elif temp[:10] == " GuildPub-":
                panelName = U"LinkPanel_GuildPub-" + guildPubs[showGuildPub-1]
            elif temp == " Kveer" or temp == " K'veer":
                panelName = U"LinkPanel_" + "Kveer"
            else:
                panelName = U"LinkPanel_" + self.IGetUntranslatedName(idBookPresented+1)
        else:
            # gonna have to dig...
            clickedName = self.IGetUntranslatedName(idBookPresented+1)
            panelName = clickedName
            vault = ptVault()
            if idCategorySelected == kIDBtnLinkCategory02: # private links
                folder = vault.getAgesICanVisitFolder()
            else: # personal links
                folder = vault.getAgesIOwnFolder()
            contents = folder.getChildNodeRefList()
            for content in contents:
                linkNode = content.getChild()
                linkNode = linkNode.upcastToAgeLinkNode()
                if type(linkNode) != type(None):
                    info = linkNode.getAgeInfo()
                else:
                    linkNode = content.getChild()
                    info = linkNode.upcastToAgeInfoNode()
                if type(info) == type(None): continue
                if clickedName == "Great Zero": #super hack to the rescue!
                    panelName = U"LinkPanel_" + "GreatZero"
                    break
                if clickedName == info.getDisplayName():
                    panelName = U"LinkPanel_" + Uni(info.getAgeFilename())
                    break
            
        PtDebugPrint( "drawing link panel: %s" % (panelName) )
        for objPanel in objlistLinkPanels.value:
            PtDebugPrint("name: "+objPanel.getName())
            if objPanel.getName() == panelName:
                objPanel.draw.enable()
                return
        PtDebugPrint( "nxusBookMachine.IDrawLinkPanel():\tERROR: couldn't find link panel: %s" % (panelName) )
        return
        
    def ISyncCurCityInstanceAndVault(self):
        global curCityInstance
        
        if len(cityInstances):
            minCityGUID = cityInstances[0].getAgeInstanceGuid()
            curCityInstance = 0
            for i in range(0,len(cityInstances)):
                if cityInstances[i].getAgeInstanceGuid() < minCityGUID:
                    minCityGUID = cityInstances[i].getAgeInstanceGuid()
                    curCityInstance = i
            PtDebugPrint("ISyncCurCityInstanceAndVault(): Picked guid: "+str(minCityGUID))

    
    def IGetAgeInfo(self,age):
        if age == "guildPub":
            #global guildPubInstance
            #global guildPubPopulation
            gpubGUID = guildPubGUIDs[showGuildPub-1]
            gpInfo = ptAgeInfoStruct()
            guild = guildPubs[showGuildPub-1]
            gp = "GuildPub-" + guild
            gpInfo.setAgeFilename(gp)
            gpInfo.setAgeInstanceName(gp)
            gpInfo.setAgeInstanceGuid(gpubGUID)
            gpLink = ptAgeLinkStruct()
            gpLink.setAgeInfo(gpInfo)
            gpInfo = gpLink.getAgeInfo()
            guildPubInstance = gpInfo
            #guildPubPopulation = ???   #no current way to get that data
            return guildPubInstance
        elif age == "kveerPublic":
            #global kveerPublicInstance
            #global kveerPublicPopulation
            kveerInfo = ptAgeInfoStruct()
            kveerInfo.setAgeFilename("Kveer")
            kveerInfo.setAgeInstanceName("K'veer")
            kveerInfo.setAgeInstanceGuid(kveerPublicGUID)
            kveerLink = ptAgeLinkStruct()
            kveerLink.setAgeInfo(kveerInfo)
            kveerInfo = kveerLink.getAgeInfo()
            kveerPublicInstance = kveerInfo
            #kveerPublicPopulation = ???   #no current way to get that data
            return kveerPublicInstance
        else:
            print "nxusBookMachine.IGetAgeInfo():  ERROR!  Don't recognize age: ",age


    def DoErcanaAndAhnonayStuff(self,panel):
        print "nxusBookMachine.DoErcanaAndAhnonayStuff(): this age panel = ",panel
        if panel == "Ercana":
            ageFileName = "Ercana"
            ageInstanceName = "Er'cana"
        elif panel == "AhnonayCathedral" or panel == "Ahnonay":
            ageFileName = "AhnonayCathedral"
            ageInstanceName = "Ahnonay Cathedral"
        self.FindOrCreateGUIDChron(ageFileName)


    def FindOrCreateGUIDChron(self,ageFileName):
        print "FindOrCreateGUIDChron for: ",ageFileName
        GUIDChronFound = 0
        ageDataFolder = None
        
        vault = ptVault()
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename(ageFileName)
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            ageInfoChildren = ageInfoNode.getChildNodeRefList()
            for ageInfoChildRef in ageInfoChildren:
                ageInfoChild = ageInfoChildRef.getChild()
                folder = ageInfoChild.upcastToFolderNode()
                if folder and folder.folderGetName() == "AgeData":
                    ageDataFolder = folder
                    ageDataChildren = folder.getChildNodeRefList()
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "PelletCaveGUID":
                            GUIDChronFound = 1
                            print "found pellet cave GUID: ", chron.getValue()
                            return
                    #return

        pelletCaveGUID = ""
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("PelletBahroCave")
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            pelletCaveGUID = ageInfoNode.getAgeInstanceGuid()
            print "found pelletCaveGUID age chron, = ",pelletCaveGUID
        
        if not ageDataFolder:
            print "no ageDataFolder..."
            ageStruct = ptAgeInfoStruct()
            ageStruct.setAgeFilename(ageFileName)
            ageLinkNode = vault.getOwnedAgeLink(ageStruct)
            if ageLinkNode:
                print "got ageLinkNode, created AgeData folder"
                ageInfoNode = ageLinkNode.getAgeInfo()
                ageDataFolder = ptVaultFolderNode(0)
                ageDataFolder.folderSetName("AgeData")
                ageInfoNode.addNode(ageDataFolder)

        if not GUIDChronFound:
            print "creating PelletCave GUID chron"
            newNode = ptVaultChronicleNode(0)
            newNode.chronicleSetName("PelletCaveGUID")
            newNode.chronicleSetValue(pelletCaveGUID)
            ageDataFolder.addNode(newNode)
            print "created pelletCaveGUID age chron, = ",pelletCaveGUID

