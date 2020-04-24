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
Module: nxusBookMachine
Age: nexus
Date: September, 2002
Author: Bill Slease
Handler for the nexus book machine.
"""

from Plasma import *
from PlasmaKITypes import *
from PlasmaNetConstants import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
from xPsnlVaultSDL import *

import xLocTools

import PlasmaControlKeys
import datetime
import random

# define the attributes that will be entered in max
NexusGUI = ptAttribGUIDialog(1, "The Nexus GUI")
actKISlot = ptAttribActivator(2, "Actvtr: KI Slot")
respKISlot = ptAttribResponder(3, "Rspndr: KI Slot")
camMachineOps = ptAttribSceneobject(4, "Camera: machine ops")
behMachineOps = ptAttribBehavior(5, "Behavior: machine ops")
respKISlotReturn = ptAttribResponder(6, "Rspndr: KI Slot Return") # onInit
respGUIOn = ptAttribResponder(7, "Rspndr: GUI On")
respGUIOff = ptAttribResponder(8, "Rspndr: GUI Off")
respBookSelect = ptAttribResponder(9, "Rspndr: Enable GetBook Btn")
#respBookGet = ptAttribResponder(10,"Rspndr: Get Book")
actLink = ptAttribActivator(11, "Actvtr: Link")
respLink = ptAttribResponder(12, "Rspndr: Link")
respBookRetract = ptAttribResponder(13, "Rspndr: Retract Book") # onInit
actGetBook = ptAttribActivator(14, "Actvtr: Get Book")
respGetBook = ptAttribResponder(15, "Rspndr: Get Book")
respButtonPress = ptAttribResponder(16, "Rspndr: GetBook Btn Press") # onInit
objlistLinkPanels = ptAttribSceneobjectList(17, "Objct: Link Panels")
respKISlotGlow = ptAttribResponder(18, "Rspndr: KI Slot Glow")

#------nexus machine GUI tags
kNexusDialogName = "NexusAgeDialog"

kIDBtnLinkCategory01 = 100
kIDTxtLinkCategory01 = 101
kIDBtnLinkCategory02 = 110
kIDTxtLinkCategory02 = 111
kIDBtnLinkCategory03 = 120
kIDTxtLinkCategory03 = 121
kIDBtnLinkCategory04 = 130
kIDTxtLinkCategory04 = 131

kCategoryCity = kIDBtnLinkCategory01
kCategoryPrivate = kIDBtnLinkCategory02
kCategoryPublic = kIDBtnLinkCategory03
kCategoryPersonal = kIDBtnLinkCategory04

kGUICategoryControls = ((kIDTxtLinkCategory01, kIDBtnLinkCategory01),
                        (kIDTxtLinkCategory02, kIDBtnLinkCategory02),
                        (kIDTxtLinkCategory03, kIDBtnLinkCategory03),
                        (kIDTxtLinkCategory04, kIDBtnLinkCategory04))

kIDBtnNeighborhoodCreate = 400
kIDBtnNeighborhoodSelect = 401
kIDTxtNeighborhoodName = 402
kIDTxtNeighborhoodInfo = 403

kIDBtnNeighborhoodPublic = 404
kIDTxtNeighborhoodPublic = 405

kIDBtnScrollUp = 500
kIDBtnScrollDn = 501

kIDTxtLinkDescription = 200

kIDBtnLinkSelectFirst = 210
kIDBtnLinkSelectLast = 280

kIDTxtLinkNameFirst = 211
kIDTxtLinkNameLast = 281

kIDBtnDeleteLinkFirst = 310
kIDBtnDeleteLinkLast = 380

#id for 8 link list controls
#for any row: (select button, name txt, info txt, delete button)
kGUILinkControls = [(210, 211, 212, 310),
                    (220, 221, 222, 320),
                    (230, 231, 232, 330),
                    (240, 241, 242, 340),
                    (250, 251, 252, 350),
                    (260, 261, 262, 360),
                    (270, 271, 272, 370),
                    (280, 281, 282, 380),
                    ]

kIDNameHeaderText = 600
kIDPopHeaderText = 601
kIDNameHeaderBtn = 610
kIDPopHeaderBtn = 611
kIDNameAscArrow = 620
kIDPopAscArrow = 621
kIDNameDescArrow = 630
kIDPopDescArrow = 631

#currently unused language controls
kIDEngCheckBox = 700
kIDFreCheckBox = 701
kIDGerCheckBox = 702
kIDEngCheck = 710
kIDFreCheck = 711
kIDGerCheck = 712
kIDEngText = 720
kIDFreText = 721
kIDGerText = 722

kLanguageControls = {kIDEngCheckBox: PtLanguage.kEnglish,
                     kIDEngCheck: PtLanguage.kEnglish,

                     kIDFreCheckBox: PtLanguage.kFrench,
                     kIDFreCheck: PtLanguage.kFrench,

                     kIDGerCheckBox: PtLanguage.kGerman,
                     kIDGerCheck: PtLanguage.kGerman,
                    }
#------colors
AgenGoldDk = ptColor(0.925, 0.84, 0.365, 1.0)
AgenGoldLt = ptColor(0.97, 0.937, 0.745, 1.0)
AgenGoldDkSoft = ptColor(0.925, 0.84, 0.365, 0.25)
AgenGoldLtSoft = ptColor(0.97, 0.937, 0.745, 0.25)

colorNormal = AgenGoldDk
colorSelected = AgenGoldLt
colorPresented = AgenGoldLt
colorDisabled = AgenGoldDkSoft

kNumDisplayFields = 8
kMaxDisplayableChars = 24 # the avg number of chars to display before tacking on an ellipsis: "..."

#special named link panels (other then 'LinkPanel_' + Age Filename)
kLinkPanels = {
'city' : {'LinkInPointFerry' : U'LinkPanel_Ferry Terminal',
        'LinkInPointDakotahAlley' : U'LinkPanel_Dakotah Alley',
        'LinkInPointPalace' : U'LinkPanel_Palace Alcove',
        'LinkInPointConcertHallFoyer' : U'LinkPanel_Concert Hall Foyer',
        'LinkInPointLibrary' : U'LinkPanel_Library Courtyard'
        },
'Cleft' : {
           #That would be conspicious, if Nexus allowed to link to rainy cleft, unless we belive in great Nexus-Maintainers-Bahro conspiracy
           'SpawnPointTomahna01' : U'LinkPanel_Tomahna',
           #Umm, why Nexus even has entry for some boring hole in ground on surface?  
           '' : U'LinkPanel_Cleft',
          },

'GreatZero' : {'' : U'LinkPanel_Great Zero Observation',
               'BigRoomLinkInPoint' : U'LinkPanel_GreatZero'
              },
'Neighborhood02' : {'' : U'LinkPanel_Kirel'
                   },
}


kHiddenPersonalAges = ["Personal", "Nexus", "Neighborhood", "city", "AvatarCustomization", "Cleft", "BaronCityOffice", "BahroCave", "PelletBahroCave", "Kveer", "Myst", "LiveBahroCaves", "LiveBahroCave"]
kHiddenCityLinks = ["islmPalaceBalcony03", "KadishGallery", "islmPalaceBalcony02", "islmDakotahRoof", ]
kHiddenAgesIfInvited = ["BahroCave", "PelletBahroCave", "Pellet Cave", "LiveBahroCave", "LiveBahroCaves", "Myst"]

#public ages SDL variables to be read from Vault on start (max. population, is link visible)
kAgeSdlVariables = {
'city' : ('MaxCityPop', 'nxusCityLinksVis'),
'GreatTreePub' : ('MaxPubPop', 'nxusShowPub'),
'guildPub' : ('MaxGuildPubPop', None),
'Neighborhood02' : ('MaxKirelPop', 'nxusShowKirel'),
'Kveer' : ('MaxKveerPublicPop', None),
}

kGuildPubs = ["Cartographers", "Greeters", "Maintainers", "Messengers", "Writers"]

#if possible, game will try to use those instances
kHardcodedInstances = {"GuildPub-Cartographers" : "35624301-841e-4a07-8db6-b735cf8f1f53",
                      "GuildPub-Greeters" : "381fb1ba-20a0-45fd-9bcb-fd5922439d05",
                      "GuildPub-Maintainers" : "e8306311-56d3-4954-a32d-3da01712e9b5",
                      "GuildPub-Messengers" : "9420324e-11f8-41f9-b30b-c896171a8712",
                      "GuildPub-Writers" : "5cf4f457-d546-47dc-80eb-a07cdfefa95d",
                      "Kveer" : "68e219e0-ee25-4df0-b855-0435584e29e2"}

#id for ages descriptions
kPublicAgesDescription = {
     'city' : ("Nexus.Messages.CityFull", "Nexus.Messages.CityPopulation"),
     'GreatTreePub' : ("Nexus.Messages.PubFull", "Nexus.Messages.PubPopulation"),
     'Neighborhood02' : ("Nexus.Messages.KirelFull", "Nexus.Messages.KirelPopulation"),
     'GuildPub-Cartographers' : ("Nexus.Messages.GuildPubFull", "Nexus.Messages.GuildPubPopulation"),
     'GuildPub-Greeters' : ("Nexus.Messages.GuildPubFull", "Nexus.Messages.GuildPubPopulation"),
     'GuildPub-Maintainers' : ("Nexus.Messages.GuildPubFull", "Nexus.Messages.GuildPubPopulation"),
     'GuildPub-Messengers' : ("Nexus.Messages.GuildPubFull", "Nexus.Messages.GuildPubPopulation"),
     'GuildPub-Writers' : ("Nexus.Messages.GuildPubFull", "Nexus.Messages.GuildPubPopulation"),
}

# hood sorting vars
kSortNone = 0
kSortNameAsc = 1
kSortNameDesc = 2
kSortPopAsc = 3
kSortPopDesc = 4

#controls for hood sorting var
kSortControlId = {
    kSortNameAsc : kIDNameAscArrow,
    kSortNameDesc : kIDNameDescArrow,
    kSortPopAsc : kIDPopAscArrow,
    kSortPopDesc : kIDPopDescArrow
}

#GUI state
kGUIDeactivated = 0
kWaitingOnGUIAnim = 1
kGUIActivated = 2

kChronicleVarType = 0

def Uni(string):
    "Converts a string to unicode, using latin-1 encoding if necessary"
    try:
        retVal = unicode(string)
        return retVal
    except UnicodeDecodeError:
        retVal = unicode(string, "latin-1")
        return retVal

class AgeInstance():
    def __init__(self, ageData):
        self.ageInfo = ageData[0]
        self.population = ageData[1]
        self.owners = ageData[2]

class AgeData():
    def __init__(self, ageFilename, defaultMaxPop, linkVisible):
        self.ageFilename = ageFilename
        self.maxPop = defaultMaxPop
        self.linkVisible = linkVisible
        self.instances = list()

class LinkListEntry():
    def __init__(self, displayName, displayInfo, description = U"", canDelete = False, isEnabled = True):
        self.untranslatedName = Uni(displayName)
        self.displayName = xLocTools.LocalizeAgeName(displayName)
        self.displayInfo = displayInfo
        self.description = description
        self.canDelete = canDelete
        self.isEnabled = isEnabled
        self.als = None

    def setLinkStruct(self, ageInfo, spawnPoint = None, linkRule = PtLinkingRules.kBasicLink):
        if isinstance(ageInfo, ptVaultAgeInfoNode):
            ageInfo = ageInfo.asAgeInfoStruct()

        self.als = ptAgeLinkStruct()
        self.als.setAgeInfo(ageInfo)
        self.als.setLinkingRules(linkRule)
        if spawnPoint is not None:
            self.als.setSpawnPoint(spawnPoint)

    def setChildAgeLinkStruct(self, ageInfo, parentFilename, spawnPoint = None):
        if isinstance(ageInfo, ptVaultAgeInfoNode):
            ageInfo = ageInfo.asAgeInfoStruct()

        self.als = ptAgeLinkStruct()
        self.als.setAgeInfo(ageInfo)
        self.als.setLinkingRules(PtLinkingRules.kChildAgeBook)
        self.als.setParentAgeFilename(parentFilename)
        if spawnPoint is not None:
            self.als.setSpawnPoint(spawnPoint)

class nxusBookMachine(ptModifier):
    "The Nexus python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5017
        version = 5
        self.version = version
        print "__init__nxusBookMachine v.", version
        random.seed()

        self.guiState = kGUIDeactivated
        self.getBookBtnUp = False
        self.gettingBook = False
        self.controlsEnabled = False
        self.animCount = 0
        self.dialogVisible = False

        self.currentStatusBarText = U""

        self.publicHoodSort = kSortNone #current hood list sorting method

        self.deleteCandidateId = None #id for hood to delete

        self.controlIdToAgeEntry = dict() #list of LinkListEntry entries assigned to control

        self.idCategorySelected = kIDBtnLinkCategory01
        self.idLinkSelected = None

        self.presentedBookAls = None

        self.indexDisplayStart = 0

        self.publicHoods = list()
        self.neighborhoodEntry = None
        self.publicAges = {
            'city' : AgeData(ageFilename = 'city', defaultMaxPop = 20, linkVisible = 1),
            'GreatTreePub' : AgeData(ageFilename = 'GreatTreePub', defaultMaxPop = 100, linkVisible = 0),
            'guildPub' : AgeData(ageFilename = '', defaultMaxPop = 0, linkVisible = 0),
            'Neighborhood02' : AgeData(ageFilename = 'Neighborhood02', defaultMaxPop = 100, linkVisible = 0),
            'Kveer' : AgeData(ageFilename = 'Kveer', defaultMaxPop = 100, linkVisible = 0),
            }

        self.categoryLinksList = {
            kCategoryCity : list(), #city links
            kCategoryPrivate : list(), #private
            kCategoryPublic : list(), #public
            kCategoryPersonal : list() #personal
        }

        self.showHoodLanguages = {
            PtLanguage.kEnglish : True,
            PtLanguage.kFrench : True,
            PtLanguage.kGerman : True
        }

    def OnFirstUpdate(self):
        "First update, load GUI dialog, give player PAL to Nexus"
        PtLoadDialog(kNexusDialogName, self.key, "Nexus")

        # move objects to correct starting position just in case
        respKISlotReturn.run(self.key, fastforward = 1)
        respBookRetract.run(self.key, fastforward = 1)
        respButtonPress.run(self.key, fastforward = 1)

        # hide all the linking panels in the machine - will draw appropriate when selected
        for objPanel in objlistLinkPanels.value:
            objPanel.draw.disable()

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        for (ageName, (maxPopVar, linkVisibleVar)) in kAgeSdlVariables.iteritems():
            #updating maximum population
            if maxPopVar is not None:
                PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing variable '%s'" % maxPopVar)
                try:
                    pop = ageSDL[maxPopVar][0]
                    self.publicAges[ageName].maxPop = pop
                except (KeyError, IndexError):
                    PtDebugPrint("Unable to get  '%s' from SDL, defaulting to %d" % (maxPopVar, self.publicAges[ageName].maxPop))

            #checking if link is visible
            if linkVisibleVar is not None:
                PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing variable '%s'" % linkVisibleVar)
                try:
                    visible = ageSDL[linkVisibleVar][0]
                    self.publicAges[ageName].linkVisible = visible
                except (KeyError, IndexError):
                    PtDebugPrint("Unable to get  '%s' from SDL, defaulting to %d" % (linkVisibleVar, self.publicAges[ageName].linkVisible))

        #special cases

        #GZ
        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing the show great zero toggle")
        try:
            self.showGreatZero = ageSDL["nxusShowGZ"][0]
        except (KeyError, IndexError):
            PtDebugPrint("Unable to get show great zero toggle from SDL, defaulting to false")
            self.showGreatZero = False

        #Guild Pub
        PtDebugPrint("nxusBookMachine.OnServerInitComplete(): Grabbing the show GuildPub var")
        guildPubEntry = self.publicAges['guildPub']

        psnlSDL = xPsnlVaultSDL()
        showGuildPub = psnlSDL["guildAlliance"][0]
        if showGuildPub:
            if PtIsInternalRelease():
                for guild in kGuildPubs:
                    filename = "GuildPub-%s" % guild
                    data = AgeData(filename, 0, 1)
                    data.guild = guild # for name formatting
                    self.publicAges[filename] = data
                    PtDebugPrint("nxusBookMachine.OnServerInitComplete() - showing guild pub: %s" % guild)
            else:
                guild = kGuildPubs[showGuildPub - 1]
                filename = "GuildPub-" + guild
                PtDebugPrint("nxusBookMachine.OnServerInitComplete() - member of guild: %s" % (guild))
                guildPubEntry.linkVisible = 1
                guildPubEntry.guild = guild
                guildPubEntry.ageFilename = filename
                #rename old entry
                self.publicAges[filename] = guildPubEntry

        del self.publicAges['guildPub']

        vault = ptVault()
        entry = vault.findChronicleEntry("GotLinkToKveerPublic")
        if entry is not None:
            entryValue = entry.chronicleGetValue()
            if entryValue == "yes":
                print "nxusBookMachine.OnServerInitComplete(): chron says you have the link to public Kveer, woo hoo"
                self.publicAges['Kveer'].linkVisible = True
        else:
            print "nxusBookMachine.OnServerInitComplete(): chron says no link to public Kveer yet, so sorry"


    def __del__(self):
        "destructor - get rid of any dialogs that we might have loaded"
        PtUnloadDialog(kNexusDialogName)

    def ICanCreateHood(self):
        hoodLink = self.IGetHoodLinkNode()
        if not hoodLink.getVolatile():
            return False # only let the player create a new hood if their book is volatile
        if PtIsInternalRelease():
            return True # we don't have the 1 day limitation on internal clients
        vault = ptVault()
        entry = vault.findChronicleEntry("LastHoodCreationTime")
        if entry is None:
            return True # we haven't created a hood before, so let them!

        temp = float(entry.chronicleGetValue())
        lastTime = datetime.date.fromtimestamp(temp)
        temp = PtGetDniTime()
        curTime = datetime.date.fromtimestamp(temp)

        oneDay = datetime.timedelta(days = 1)

        return curTime - lastTime > oneDay

    def gotPublicAgeList(self, ages):
        if not ages:
            PtDebugPrint("nxusBookMachine.gotPublicAgeList() - got an empty list, which we assume are hoods, clearing hood list")
            self.publicHoods = list()
            self.IUpdateLinks(kCategoryPublic)
            return

        hoods = list()
        tempInstances = dict()

        for age in ages:
            ageFilename = age[0].getAgeFilename()
            if ageFilename == "Neighborhood":
                # if the current population and number of owners is zero then don't display it
                #looks like it doesn't work (at least on Dirtsand)
                if age[2] != 0 or age[1] != 0:
                    hoods.append(AgeInstance(age))
            else:
                PtDebugPrint("nxusBookMachine.gotPublicAgeList() - got the list of %s instances" % ageFilename)
                try:
                    instances = tempInstances[ageFilename]
                except KeyError:
                    instances = list()
                    tempInstances[ageFilename] = instances

                instances.append(AgeInstance(age))

        if tempInstances:
            for (ageFilename, instances) in tempInstances.iteritems():
                try:
                    self.publicAges[ageFilename].instances = sorted(instances, key = lambda entry : entry.ageInfo.getAgeSequenceNumber())
                except KeyError:
                    PtDebugPrint("nxusBookMachine.gotPublicAgeList(): got age '%s', that wasn't expected" % ageFilename)
            self.IUpdateLinks(kCategoryCity)

        if hoods:
            self.publicHoods = hoods
            self.IUpdateLinks(kCategoryPublic)

    def ISortPublicHoods(self, hoods, hoodSort = kSortNone):
        # if the language is not English, French, or German, we assume it is English and treat it as such
        hoodsToSort = list()
        for hood in hoods:
            hoodLanguage = hood.ageInfo.getAgeLanguage()
            allowLanguage = self.showHoodLanguages.get(hoodLanguage, True)
            if allowLanguage:
                hoodsToSort.append(hood)

        if hoodSort == kSortNone:
            return hoodsToSort

        reverse = (hoodSort in (kSortNameDesc, kSortPopDesc))
        if hoodSort in (kSortNameAsc, kSortNameDesc):
            return sorted(hoodsToSort, key = lambda hood: hood.ageInfo.getDisplayName(), reverse = reverse)
        else:
            return sorted(hoodsToSort, key = lambda hood: hood.population, reverse = reverse)

    def IFindAgeLinkInFolder(self, folder, ageName):
        ageName = ageName.lower()
        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild().upcastToAgeLinkNode()
            if link is not None:
                info = link.getAgeInfo()
            else:
                link = content.getChild()
                info = link.upcastToAgeInfoNode()

            if info.getAgeFilename().lower() == ageName:
                return link
        return None

    def IFindAgeInfoInFolder(self, folder, ageName):
        ageName = ageName.lower()
        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            if link is not None:
                info = link.getAgeInfo()
            else:
                link = content.getChild()
                info = link.upcastToAgeInfoNode()

            if info.getAgeFilename().lower() == ageName:
                return info
        return None

    def IGetHoodLinkNode(self):
        vault = ptVault()
        folder = vault.getAgesIOwnFolder()
        return self.IFindAgeLinkInFolder(folder, 'Neighborhood')

    def IGetHoodInfoNode(self):
        vault = ptVault()
        folder = vault.getAgesIOwnFolder()
        return self.IFindAgeInfoInFolder(folder, 'Neighborhood')

    def IIsMyHoodPublic(self):
        hoodInfo = self.IGetHoodInfoNode()
        if hoodInfo is not None:
            return hoodInfo.isPublic()

    def IMakeHoodPublic(self):
        hoodInfo = self.IGetHoodInfoNode()
        if hoodInfo is not None:
            infoStruct = hoodInfo.asAgeInfoStruct()
            PtCreatePublicAge(infoStruct, self)


    def IMakeHoodPrivate(self):
        hoodInfo = self.IGetHoodInfoNode()
        if hoodInfo is not None:
            guid = hoodInfo.getAgeInstanceGuid()
            PtRemovePublicAge(guid, self)

    def IPublicAgeCreated(self, ageName):
        PtDebugPrint("IPublicAgeCreated: " + ageName)
        PtGetPublicAgeList(ageName, self)

    def IPublicAgeRemoved(self, ageName):
        PtDebugPrint("IPublicAgeRemoved: " + ageName)
        PtGetPublicAgeList(ageName, self)

    def IHoodCreated(self, ageInfo):
        PtDebugPrint("OnVaultNotify: Setting the new hood's language to %d " % PtGetLanguage())
        ageInfo.setAgeLanguage(PtGetLanguage())
        # save our creation time to the vault to prevent people from making too many hoods
        vault = ptVault()
        entry = vault.findChronicleEntry("LastHoodCreationTime")
        if entry is None:
            # not found... add current level chronicle
            vault.addChronicleEntry("LastHoodCreationTime", kChronicleVarType, str(PtGetDniTime()))
        else:
            entry.chronicleSetValue(str(PtGetDniTime()))
            entry.save()

    def IPushGetBookBtn(self):
        if self.getBookBtnUp:
            self.animCount += 1
            respButtonPress.run(self.key)
            self.getBookBtnUp = False

    def IRetractGetBookBtn(self):
        if not self.getBookBtnUp:
            self.animCount += 1
            respBookSelect.run(self.key) #retract button
            self.getBookBtnUp = True

    def IBookRetract(self):
        actLink.disable()
        self.animCount += 1
        respBookRetract.run(self.key)
        self.presentedBookAls = None

    def OnVaultNotify(self, event, tupdata):
        if event == PtVaultNotifyTypes.kRegisteredOwnedAge:
            ageLinkNode = tupdata[0] # should be a ptVaultAgeLinkNode
            ageInfo = ageLinkNode.getAgeInfo() # ptAgeInfoNode
            ageFilename = ageInfo.getAgeFilename()
            if (ageFilename == "Neighborhood"):
                self.IHoodCreated(ageInfo)
                self.IUpdateHoodLink()
            else:
                self.IUpdateLinks(kCategoryPrivate)

        elif event in (PtVaultNotifyTypes.kUnRegisteredOwnedAge, PtVaultNotifyTypes.kUnRegisteredVisitAge):
            PtDebugPrint("OnVaultNotify: A link was deleted, checking if we need to retract the book")
            ageInfo = tupdata[0].getAgeInfo()
            #is there any book presented
            if self.presentedBookAls is not None:
                presentedInfo = self.presentedBookAls.getAgeInfo()
                if ageInfo.getAgeInstanceGuid() == presentedInfo.getAgeInstanceGuid():
                    PtDebugPrint("OnVaultNotify: The currently displayed link was deleted, retracting book")
                    self.IBookRetract()

            if self.getBookBtnUp and self.idLinkSelected is not None:
                selectedInfo = self.controlIdToAgeEntry[self.idLinkSelected].als.getAgeInfo()
                if ageInfo.getAgeInstanceGuid() == selectedInfo.getAgeInstanceGuid():
                    PtDebugPrint("OnVaultNotify: The currently selected link was deleted, pushing button")
                    self.IPushGetBookBtn()

            self.IUpdateLinks()

        elif event == PtVaultNotifyTypes.kPublicAgeCreated:
            ageName = tupdata[0]
            self.IPublicAgeCreated(ageName)

        elif event == PtVaultNotifyTypes.kPublicAgeRemoved:
            ageName = tupdata[0]
            self.IPublicAgeRemoved(ageName)

    def OnVaultEvent(self, event, tupdata):
        if event == PtVaultCallbackTypes.kVaultNodeSaved:
            PtDebugPrint("nxuxBookMachine: kVaultNodeSaved event (id=%d,type=%d)" % (tupdata[0].getID(), tupdata[0].getType()), level = kDebugDumpLevel)

            # tupdata is ( ptVaultNode )
            if tupdata[0].getType() == PtVaultNodeTypes.kAgeInfoNode:
                if self.IGetHoodInfoNode().getID() == tupdata[0].getID():
                    self.IUpdateHoodLink()

    def IOnYesNoNotify(self, state, events):
        if self.deleteCandidateId is None: # user wants to create a new hood
            if state: # user answered yes
                self.IHideDisableButton(kIDBtnNeighborhoodCreate)
                link = self.IGetHoodLinkNode()
                if link:
                    link.setVolatile(1) # mark it volatile, but DON'T SAVE (we don't want the server to know, just the client)
                ptVault().createNeighborhood()
                #no need to update, it will happen on vault notify

        else: # user wants to delete a link
            if state:
                self.IDeleteLink()
                #links will be updated on vault notify

    def IOnActKISlot(self, state, events): #click on KI Slot
        kiLevel = PtDetermineKILevel()
        print "nxusBookMachine.OnNotify:\tplayer ki level is %d" % kiLevel
        if kiLevel < kNormalKI:
            respKISlot.run(self.key, events = events) #Insert KI
        elif state:
            for ageFilename in self.publicAges.keys():
                # Crummy server software (eg Cyan) might discard requests for hardcoded public ages,
                # leaving us dead in the water. BUT we would like to get information about that
                # age from the vault, if possible. Therefore, we ask anyway but don't rely on it.
                hardcoded = kHardcodedInstances.get(ageFilename)
                if hardcoded is not None:
                    ageData = self.publicAges[ageFilename]
                    if not ageData.instances:
                        ageInfo = ptAgeInfoStruct()
                        ageInfo.setAgeFilename(ageFilename)
                        ageInfo.setAgeInstanceGuid(hardcoded)
                        ageData.instances.append(AgeInstance((ageInfo, 0, 0)))
                PtGetPublicAgeList(ageFilename, self)
            PtGetPublicAgeList('Neighborhood', self)

            # set up the camera so when the one shot returns it gets set up right (one shot was fighting with python for camera control)
            virtCam = ptCamera()
            virtCam.undoFirstPerson()
            virtCam.disableFirstPersonOverride()
            respKISlotGlow.run(self.key, events = events) # insert hand, light slot, remove hand, notify triggerer and move KI slot, send notify

    def IOnKISlotGlow(self, state, events): # KI slot activated
        # switch to machine ops cam, fix player in idle behavior, show GUI
        avatar = PtGetLocalAvatar()
        behMachineOps.run(avatar)
        virtCam = ptCamera()
        virtCam.save(camMachineOps.sceneobject.getKey())
        avatar.draw.disable()
        if PtIsDialogLoaded(kNexusDialogName):
            NexusGUI.dialog.show()
            respGUIOn.run(self.key)
            self.guiState = kWaitingOnGUIAnim
        else:
            PtDebugPrint("nxusBookMachine.IOnKISlotGlow(): Nexus GUI not loaded")
            return
        PtEnableControlKeyEvents(self.key)
        PtSendKIMessage(kDisableKIandBB, 0)

    def IOnRespGUIOn(self, state, events):
        self.guiState = kGUIActivated

    def IOnRespGUIOff(self, state, events):
        self.guiState = kGUIDeactivated
        avatar = PtGetLocalAvatar()
        NexusGUI.dialog.hide()
        virtCam = ptCamera()
        virtCam.restore(camMachineOps.sceneobject.getKey())
        virtCam.enableFirstPersonOverride()
        avatar.draw.enable()
        behMachineOps.gotoStage(avatar, -1) # exit behavior
        PtDisableControlKeyEvents(self.key)
        PtSendKIMessage(kEnableKIandBB, 0)
        respKISlotReturn.run(self.key)
        self.controlsEnabled = True
        self.animCount = 0

        if self.presentedBookAls is not None:
            self.IBookRetract()

        if self.idLinkSelected is not None:
            self.ICancelLinkChoice()

    def IOnGetBookBtn(self, state, events):
        if self.animCount == 0:
            self.gettingBook = True
            self.IPushGetBookBtn()

    def IOnRespButtonPress(self, state, events):
        self.animCount -= 1
        if self.gettingBook:
            #if there is already book presented, so we need to retract it
            if self.presentedBookAls is not None:
                respBookRetract.run(self.key)
                actLink.disable()
            #book already retracted, get new book
            else:
                respGetBook.run(self.key)
                selectedAls = self.controlIdToAgeEntry[self.idLinkSelected].als
                self.presentedBookAls = selectedAls
                self.IDrawLinkPanel()
                
            self.animCount += 1


    def IOnRespBookRetract(self, state, events):
        if self.gettingBook:
            self.getBookAfterRetract = False
            selectedAls = self.controlIdToAgeEntry[self.idLinkSelected].als
            self.presentedBookAls = selectedAls
            self.IDrawLinkPanel()
            respGetBook.run(self.key)
        else:
            self.animCount -= 1
            self.gettingBook = False

    def IOnRespGetBook(self, state, events):
        actLink.enable()
        self.gettingBook = False
        self.animCount -= 1

    def IOnRespBookSelect(self, state, events):
        self.animCount -= 1
        self.getBookBtnUp = True

    def IOnActLink(self, state, events):
        if self.presentedBookAls is None:
            PtDebugPrint("Ignoring book link click, since no book is shown")
        else:
            respGUIOff.run(self.key)
            actGetBook.disable()
            self.controlsEnabled = False
            self.IDoLink()

    def OnNotify(self, state, id, events):
        "main non-GUI message handler"

        if not hasattr(self, 'onNotifyActions'):
            self.onNotifyActions = {
                - 1: self.IOnYesNoNotify, # callback from delete yes/no dialog (hopefully)
                ##################
                # Engage Machine Interface  #
                ##################
                actKISlot.id : self.IOnActKISlot, # click on KI Slot
                respKISlotGlow.id: self.IOnKISlotGlow, #KI slot activated.
                respGUIOn.id: self.IOnRespGUIOn,
                ##################
                # Exit Machine Interface  #
                ##################
                respGUIOff.id: self.IOnRespGUIOff, # catch callback from shut down GUI anim responder run by OnControlKeyEvent()
                ##################
                # Machine Operations  #
                ##################
                actGetBook.id: self.IOnGetBookBtn,
                respButtonPress.id: self.IOnRespButtonPress,
                respBookRetract.id: self.IOnRespBookRetract,
                respGetBook.id: self.IOnRespGetBook,
                respBookSelect.id: self.IOnRespBookSelect, # reenable GUI buttons now that getBookBtn is done animating
                ##################
                # Link Me  #
                ##################
                actLink.id: self.IOnActLink,
                respLink.id: None,
            }

        action = self.onNotifyActions.get(id)
        if action is not None:
            action(state, events)


    ##################
    # Exit Machine Interface  #
    ##################

    def OnControlKeyEvent(self, controlKey, activeFlag):
        "exit machine op mode"
        if controlKey in (PlasmaControlKeys.kKeyExitMode, PlasmaControlKeys.kKeyMoveBackward, PlasmaControlKeys.kKeyRotateLeft, PlasmaControlKeys.kKeyRotateRight):
            if self.guiState == kGUIActivated and self.controlsEnabled and self.animCount == 0:
                actGetBook.disable()
                respGUIOff.run(self.key)

    ##############
    # Handle GUI Clicks    #
    ##############

    def OnGUINotify(self, id, control, event):
        "Events from the Nexus GUI.."

        if id != NexusGUI.id:
            PtDebugPrint("nxusBookMachine.OnGUINotify():\tunexpected message id")

        ##################
        # Init Machine GUI  #
        ##################

        elif event == kShowHide:
            if not self.dialogVisible:
                self.dialogVisible = True
                self.IClearGUI()
                self.IUpdateHoodLink()
                self.IUpdateLinks()
                self.controlsEnabled = True

        #don't allow any click while self.controlsEnabled is false or there are any animations playing) 
        elif event == kAction and self.controlsEnabled and self.animCount == 0:
            ctrlID = control.getTagID()
            ##################
            # Link Select Buttons  #
            ##################
            if (ctrlID >= kIDBtnLinkSelectFirst and ctrlID <= kIDBtnLinkSelectLast) or ctrlID == kIDBtnNeighborhoodSelect:
                self.IRetractGetBookBtn()
                self.IChangeSelectedLink(ctrlID)

            ##################
            # Category Select Buttons  #
            ##################

            elif ctrlID >= kIDBtnLinkCategory01 and ctrlID <= kIDBtnLinkCategory04:
                #after category change currently selected link id is invalid, so disable button
                if self.idLinkSelected != kIDBtnNeighborhoodSelect:
                    self.IPushGetBookBtn()

                self.IChangeCategory(ctrlID)

            #############
            #   Scroll Buttons    #
            #############

            elif ctrlID == kIDBtnScrollUp and self.indexDisplayStart > 0:
                self.indexDisplayStart -= 1
                if self.idLinkSelected is not None and self.idLinkSelected != kIDBtnNeighborhoodSelect:
                    self.idLinkSelected += 10
                    if self.idLinkSelected > kIDBtnLinkSelectLast:  
                    # selected link scrolled off screen
                        self.ICancelLinkChoice()

                self.IUpdateGUILinkList()

            elif ctrlID == kIDBtnScrollDn:
                entryCount = len(self.categoryLinksList[self.idCategorySelected])
                #rhs value can be negative - but this shouldn't happen, since button should be disabled
                if self.indexDisplayStart < entryCount - kNumDisplayFields:
                    self.indexDisplayStart = self.indexDisplayStart + 1
                    if self.idLinkSelected is not None and self.idLinkSelected != kIDBtnNeighborhoodSelect:
                        self.idLinkSelected -= 10
                        if self.idLinkSelected < kIDBtnLinkSelectFirst: 
                        # selected link scrolled off screen
                            self.ICancelLinkChoice()
                    self.IUpdateGUILinkList()
                else:
                    PtDebugPrint("nxusBookMachine.OnGUINotify(): Tried to scroll to empty positions")

            #############
            #   Create a Hood     #
            #############

            elif ctrlID == kIDBtnNeighborhoodCreate:
                self.deleteCandidateId = None
                PtYesNoDialog(self.key, PtGetLocalizedString("Nexus.Messages.HoodCreate"))

            elif ctrlID == kIDBtnNeighborhoodPublic:
                if self.IIsMyHoodPublic():
                    self.IMakeHoodPrivate()
                else:
                    self.IMakeHoodPublic()
                # IUpdateLinks should be called automatically when the public age list updates

            ##################
            # Delete  Link Buttons  #
            ##################

            elif ctrlID >= kIDBtnDeleteLinkFirst and ctrlID <= kIDBtnDeleteLinkLast:
                self.deleteCandidateId = ctrlID - 100
                entry = self.controlIdToAgeEntry[self.deleteCandidateId]
                stringConfirm = PtGetLocalizedString("Nexus.Messages.LinkDelete", [entry.displayName])
                PtYesNoDialog(self.key, stringConfirm)

            #############
            #   Header sort buttons #
            #############

            elif ctrlID == kIDNameHeaderBtn or ctrlID == kIDNameAscArrow or ctrlID == kIDNameDescArrow:
                if self.publicHoodSort == kSortNameAsc:
                    self.publicHoodSort = kSortNameDesc
                else:
                    self.publicHoodSort = kSortNameAsc
                self.IUpdateLinks()

            elif ctrlID == kIDPopHeaderBtn or ctrlID == kIDPopAscArrow or ctrlID == kIDPopDescArrow:
                if self.publicHoodSort == kSortPopAsc:
                    self.publicHoodSort = kSortPopDesc
                else:
                    self.publicHoodSort = kSortPopAsc
                self.IUpdateLinks()

            #############
            #   Language checkboxes #
            #############

            elif ctrlID in kLanguageControls:
                try:
                    language = kLanguageControls[ctrlID]
                    self.showHoodLanguages[language] = not self.showHoodLanguages[language]
                    self.IUpdateLinks()
                except KeyError:
                    pass

        elif event == kInterestingEvent:
            if control is not None:
                if control.isInteresting():
                    ctrlID = control.getTagID()
                    try:
                        description = self.controlIdToAgeEntry[ctrlID].description
                        self.ISetDescriptionText(description, False)
                    except KeyError:
                        pass
                else:
                    self.ISetDescriptionText(self.currentStatusBarText, False)

    def IShowEnableButton(self, controlId):
        btn = ptGUIControlButton(NexusGUI.dialog.getControlFromTag(controlId))
        btn.show()
        btn.enable()

    def IHideDisableButton(self, controlId):
        btn = ptGUIControlButton(NexusGUI.dialog.getControlFromTag(controlId))
        btn.hide()
        btn.disable()

    def ISetDescriptionText(self, description, permanent = True):
        descrTxt = ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtLinkDescription))
        descrTxt.setStringW(description)
        if permanent:
            self.currentStatusBarText = description
            

    def IChangeSelection(self, oldSelection, newSelection, description = U""):
        #reenable old entry
        if oldSelection is not None:
            btnId = oldSelection
            txtId = oldSelection + 1
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(btnId)).enable()
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(txtId)).setForeColor(colorNormal)

        #disable and highlight new entry
        btnId = newSelection
        txtId = newSelection + 1
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(btnId)).disable()
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(txtId)).setForeColor(colorSelected)
        
        self.ISetDescriptionText(description)

    def IChangeCategory(self, newCategory):
        if newCategory == self.idCategorySelected:
            return

        self.IChangeSelection(self.idCategorySelected, newCategory)
        self.idCategorySelected = newCategory
        #update links with entries from new category
        self.IUpdateLinks()
        
        if self.presentedBookAls is not None and self.idLinkSelected != kIDBtnNeighborhoodSelect:
            self.IBookRetract()

    def IChangeSelectedLink(self, newSelection):
        if newSelection == self.idLinkSelected:
            return

        description = self.controlIdToAgeEntry[newSelection].description
        self.IChangeSelection(self.idLinkSelected, newSelection, description)
        self.idLinkSelected = newSelection
        
        if self.presentedBookAls is not None:
            self.IBookRetract()


    def IGetControlColor(self, controlId, enabled = True):
        if not enabled:
            return colorDisabled
        elif self.idLinkSelected == controlId:
            return colorSelected
        else:
            return colorNormal

    def IUpdateGuiEntry(self, idButton, idTxtName, idTxtInfo, linkEntry):
        self.controlIdToAgeEntry[idButton] = linkEntry
        # set color of name and info

        if self.idLinkSelected == idButton or not linkEntry.isEnabled:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idButton)).disable()
        else:
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(idButton)).enable()

        if self.idLinkSelected == idButton:
            self.ISetDescriptionText(linkEntry.description)


        displayName = linkEntry.displayName
        if len(displayName) > kMaxDisplayableChars:
            displayName = displayName[:kMaxDisplayableChars] + U"..."

        color = self.IGetControlColor(idButton, linkEntry.isEnabled)

        txtName = ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTxtName))
        txtName.setForeColor(color)
        txtName.setStringW(displayName)

        txtInfo = ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(idTxtInfo))
        txtInfo.setForeColor(color)
        txtInfo.setStringW(linkEntry.displayInfo)

    def IUpdateDeleteButton(self, idButton, enable):
        if enable:
            self.IShowEnableButton(idButton)
        else:
            self.IHideDisableButton(idButton)

    def IDisableLanguageControls(self):
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDEngText)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDFreText)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDGerText)).setString("")
        for id in kLanguageControls.keys():
            ptGUIControlButton(NexusGUI.dialog.getControlFromTag(id)).hide()

    def IToggleSortControls(self, enabled):
        active = kSortControlId.get(self.publicHoodSort)
        for id in kSortControlId.values():
            control = ptGUIControlButton(NexusGUI.dialog.getControlFromTag(id))
            if enabled and id == active:
                control.show()
                control.enable()
            else:
                control.hide()
                control.disable()

    def ICancelLinkChoice(self):
        self.IPushGetBookBtn()
        self.idLinkSelected = None
        self.ISetDescriptionText(U"")
        
        if self.presentedBookAls is not None:
            self.IBookRetract()

    def IClearGUI(self):
        # clear header titles and disable their buttons
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDNameHeaderText)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDPopHeaderText)).setString("")
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDNameHeaderBtn)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDPopHeaderBtn)).disable()

        self.IDisableLanguageControls()
        self.IClearEntryList()
        self.IToggleSortControls(False)

        # hide create hood button
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodCreate)).hide()

        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodPublic)).disable()
        ptGUIControlButton(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodPublic)).hide()
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodPublic)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDBtnNeighborhoodSelect)).setNotifyOnInteresting(1)

        self.ISetDescriptionText(U"")

        #highlight selected category
        for (txtId, btnId) in kGUICategoryControls:
            btn = ptGUIControlButton(NexusGUI.dialog.getControlFromTag(btnId))
            if self.idCategorySelected == btnId:
                color = colorSelected
                btn.disable()
            else:
                color = colorNormal
                btn.enable()
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(txtId)).setForeColor(color)

    def IClearEntryList(self):
        #reset scrolling
        self.indexDisplayStart = 0

        if self.idLinkSelected is not None and self.idLinkSelected != kIDBtnNeighborhoodSelect:
            self.ICancelLinkChoice()
        self.IHideDisableButton(kIDBtnScrollUp)
        self.IHideDisableButton(kIDBtnScrollDn)

        for (btnSelectId, txtNameId, txtInfoId, btnDeleteId) in kGUILinkControls:
            try:
                del self.controlIdToAgeEntry[btnSelectId]
            except KeyError:
                pass
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(txtNameId)).setString("")
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(txtInfoId)).setString("")

            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(btnSelectId)).setNotifyOnInteresting(1)

            self.IHideDisableButton(btnSelectId)
            self.IHideDisableButton(btnDeleteId)

    def IFormatGZCreateCoords(self, linkNode):
        dniCoords = linkNode.getCreateAgeCoords()
        if dniCoords is not None:
            coords = (dniCoords.getTorans(), dniCoords.getHSpans(), dniCoords.getVSpans())
        else:
            coords = (0, 0, 0)
        return U"%05d%   04d%   04d" % coords

    def IDeleteLink(self):
        if self.deleteCandidateId is None:
            PtDebugPrint("nxusBookMachine.IDeleteLink(): Tried to delete nothing. Should not happen")
            return

        vault = ptVault()
        cityLink = vault.getLinkToCity()
        linkInfo = self.controlIdToAgeEntry[self.deleteCandidateId].als
        ageInfo = linkInfo.getAgeInfo()
        #currently, we can only delete two types of entries:
        #city entry points and age invites 
        if ageInfo.getAgeFilename() == 'city':
            spawnPoint = linkInfo.getSpawnPoint()
            PtDebugPrint("Deleting city link to %s:%s" % (spawnPoint.getName(), spawnPoint.getTitle()))
            cityLink.removeSpawnPoint(spawnPoint)
            cityLink.save()

            #we deleted selected link
            if self.deleteCandidateId == self.idLinkSelected:
                self.ICancelLinkChoice()
                self.IUpdateLinks(kCategoryCity)

            if self.presentedBookAls is not None:
                presentedAgeGuid = self.presentedBookAls.getAgeInfo().getAgeInstanceGuid()
                presentedAgeSpawnName = self.presentedBookAls.getSpawnPoint().getName()
                PtDebugPrint("Trying to delete city link (%s,%s)" % (presentedAgeGuid, presentedAgeSpawnName))
                if (presentedAgeGuid == ageInfo.getAgeInstanceGuid() and
                    presentedAgeSpawnName == spawnPoint.getName()):
                    self.IBookRetract()
        else:
            guid = ageInfo.getAgeInstanceGuid()
            vault.unRegisterVisitAge(guid)
            #if link was presented, it will be retracted on vault notify

        self.IUpdateLinks()
        return

    def IDrawLinkPanel(self):
        if self.presentedBookAls is None:
            PtDebugPrint("nxusBookMachine.IDrawLinkPanel: trying to draw panel without selected book!")
        else:
            panelName = self.IGetLinkPanelName(self.presentedBookAls)
            PtDebugPrint("drawing link panel: %s" % (panelName))
            for objPanel in objlistLinkPanels.value:
                if objPanel.getName() == panelName:
                    objPanel.draw.enable()
                else:
                    objPanel.draw.disable()

    def IChoosePublicInstances(self):
        for (ageFilename, entry) in self.publicAges.iteritems():
            if entry.instances:
                #instance with lowest population (minimal load ballancing, if multiple public instanes ever go back)
                #minPop = min(entry.instances, key = lambda age: age.population)
                #but for now, lets just stick to oldest instance (with lowest node id)
                entry.selected = entry.instances[0]
            else:
                entry.selected = None

    #######################################
    # Methods for generating link list
    #######################################

    def IGetAeguraEntries(self):
        vault = ptVault()

        playerCityLink = vault.getLinkToCity() #link saved in player's vault
        publicCityData = self.publicAges['city'].selected #selected info for public city (AgeData)

        stringLinkInfo = self.IFormatGZCreateCoords(playerCityLink)

        spawnPoints = playerCityLink.getSpawnPoints()
        for spawnPoint in spawnPoints:
            displayName = spawnPoint.getTitle()
            # Don't display spawn points that shouldn't be shown
            if displayName in kHiddenCityLinks:
                continue


            if displayName == "Ferry Terminal":
                canDelete = False
            else:
                canDelete = True

            newEntry = LinkListEntry(displayName, stringLinkInfo, U"", canDelete, None)
            newEntry.setLinkStruct(publicCityData.ageInfo, spawnPoint) #create link to instance, set spawnPoint
            yield newEntry

    def IUpdateCityLinksList(self):
        self.IChoosePublicInstances() #make sure, that we selected prefered instances

        cityLinks = list()
        for ageData in self.publicAges.itervalues():
            if ageData.selected is None or not ageData.linkVisible:
                continue

            if ageData.maxPop == 0:
                # maxPop == 0 means don't show it
                description = U""
            else:
                #check if selected instance is full
                entryEnabled = (ageData.selected.population <= ageData.maxPop)

                #try to find translated description
                try:
                    (textFull, textPopulation) = kPublicAgesDescription[ageData.ageFilename]
                    if entryEnabled:
                        description = PtGetLocalizedString(textPopulation, [str(ageData.selected.population), str(ageData.maxPop)])
                    else:
                        description = PtGetLocalizedString(textFull)
                except KeyError:
                    description = U""

            #special case: Ae'gura multiple link points
            if ageData.ageFilename == 'city':
                for entry in self.IGetAeguraEntries():
                    entry.description = description
                    entry.isEnabled = entryEnabled
                    cityLinks.append(entry)
                continue

            #ptAgeInfoStruct for selected instance
            selectedInfo = ageData.selected.ageInfo

            #special case: ugly hack for K'veer
            if ageData.ageFilename == 'Kveer':
                displayName = "K'veer"
            #special case: and another one for GuildPub name
            elif hasattr(ageData, 'guild'):
                displayName = "The %s' Pub" % (ageData.guild)
            else:
                displayName = selectedInfo.getDisplayName()

            # just in case: no display name, use the filename
            if not displayName:
                PtDebugPrint("nxusBookMachine.IUpdateCityLinksList(): Empty display name for age '{}'".format(ageData.ageFilename))
                displayName = ageData.ageFilename

            #normal cases: just add link with default link spot
            stringLinkInfo = U"%05d%   04d%   04d" %(0,0,0) #temporary consistency hack. fixme
            newEntry = LinkListEntry(displayName, stringLinkInfo, description, False, entryEnabled)
            newEntry.setLinkStruct(selectedInfo) #create link to instance, use default spawnPoint
            cityLinks.append(newEntry)

        self.categoryLinksList[kCategoryCity] = cityLinks

    def IUpdatePublicLinksList(self):
        sortedHoods = self.ISortPublicHoods(self.publicHoods, self.publicHoodSort)

        hoodLinks = list()
        for hood in sortedHoods:
            displayName = hood.ageInfo.getDisplayName()
            stringLinkInfo = str(hood.population) #TODO: i10n
            description = hood.ageInfo.getAgeDescription()

            newEntry = LinkListEntry(displayName, stringLinkInfo, description)
            newEntry.setLinkStruct(hood.ageInfo) #create link to instance, use default spawnPoint
            hoodLinks.append(newEntry)

        self.categoryLinksList[kCategoryPublic] = hoodLinks

    def IGetGZLinkNode(self):
        childAgeFolder = self.IGetHoodInfoNode().getChildAgesFolder()
        contents = childAgeFolder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            name = link.getAgeInfo().getAgeFilename()
            if name.lower() == "greatzero":
                return link
        return None # not found

    def IGetGZLinks(self):
        if PtDetermineKIMarkerLevel() <= kKIMarkerFirstLevel:
            # if our KI level is too low (we didn't grab markers), don't show anything
            PtDebugPrint("nxusBookMachine.IUpdateLinks:\tHiding all GZ spawn points because you haven't found the markers yet")
            return

        GZLinkNode = self.IGetGZLinkNode()
        if GZLinkNode is None:
            PtDebugPrint("nxusBookMachine.IGetGZLinks:\tCouldn't find GZ link node, preventing it from showing")
        elif self.showGreatZero: # even if the link exists, don't show it if the vault says no
            GZSpawnpoints = GZLinkNode.getSpawnPoints()

            for spawnPoint in GZSpawnpoints: # [:] makes a copy of the list, so we can modify the original
                displayName = spawnPoint.getTitle()
                stringLinkInfo = self.IFormatGZCreateCoords(GZLinkNode)
                if 'default' in displayName.lower() or 'default' in spawnPoint.getName().lower():
                    continue # we hide the default spawn point, since they don't want it showing in our list
                else:
                    entry = LinkListEntry(displayName, stringLinkInfo)
                    entry.setChildAgeLinkStruct(GZLinkNode.getAgeInfo(), 'Neighborhood', spawnPoint) #create link to child age
                    yield entry

    def IUpdatePrivateLinksList(self):
        vault = ptVault()
        folder = vault.getAgesICanVisitFolder()
        privateList = list(self.IGetAgesFromFolder(folder, kHiddenAgesIfInvited, True))
        self.categoryLinksList[kCategoryPrivate] = privateList


    def IUpdatePersonalLinksList(self):
        vault = ptVault()

        personalList = list()
        if self.showGreatZero:
            personalList.extend(self.IGetGZLinks())
        folder = vault.getAgesIOwnFolder()
        personalList.extend(self.IGetAgesFromFolder(folder, kHiddenPersonalAges, False))
        self.categoryLinksList[kCategoryPersonal] = personalList

    def IGetAgesFromFolder(self, folder, hidden, private):
        if folder is None:
            PtDebugPrint("nxusBookMachine.IGetAgesFromFolder:\tlink folder type None")
            return

        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            if link is not None:
                info = link.getAgeInfo()
            else:
                # it might be an AgeInfoNode
                link = link.getChild()
                info = link.upcastToAgeInfoNode()

            if info is None:
                PtDebugPrint("nxusBookMachine: Can't find ageInfo from link", level = kErrorLevel)
                continue

            if isinstance(link, ptVaultAgeLinkNode) and link.getVolatile():
                continue

            name = link.getAgeInfo().getAgeFilename()

            #remove hidden entries
            if name in hidden:
                PtDebugPrint("Removing link for " + name + " since it's in our hidden list")
                continue

            #check, if default spawnPoint exists
            #spawnPoints = link.getSpawnPoints()
            #for spawnPoint in spawnPoints:
            #    if spawnPoint.getName().lower() == "linkinpointdefault":
            #        break
            #else:
            #    if name.lower() != "greatzero":
            #        PtDebugPrint("Removing link for " + name + " since you don't have the default link-in point")
            #        continue # if it doesn't have the default link-in point, don't let it show

            displayName = info.getDisplayName()

            stringLinkInfo = self.IFormatGZCreateCoords(link)

            entry = LinkListEntry(displayName, stringLinkInfo, canDelete = private)
            if private:
                entry.setLinkStruct(info, spawnPoint = None, linkRule = PtLinkingRules.kVisitBook)
            else:
                #if (thisAge == "Ercana" or thisAge == "AhnonayCathedral" or thisAge == "Ahnonay") #do some crazy stuff
                entry.setLinkStruct(info, spawnPoint = None, linkRule = PtLinkingRules.kOwnedBook) #create link to instance, use default spawnPoint 
            yield entry

    #######################################
    # GUI Updates
    #######################################

    def IShowCreateHoodControls(self):
        if self.ICanCreateHood():
            self.IShowEnableButton(kIDBtnNeighborhoodCreate)
        else:
            self.IHideDisableButton(kIDBtnNeighborhoodCreate)

        self.IHideDisableButton(kIDBtnNeighborhoodPublic)
        self.IHideDisableButton(kIDBtnNeighborhoodSelect)

        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodName)).setString("")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodInfo)).setString("")
        self.ISetDescriptionText(U"")

    def IUpdateHoodLink(self):
        hoodLink = self.IGetHoodLinkNode()
        if hoodLink is None:
            self.IShowCreateHoodControls()
            return

        info = hoodLink.getAgeInfo()
        if info is None or hoodLink.getVolatile():
            self.IShowCreateHoodControls()
            return

        displayName = info.getDisplayName()

        description = Uni(info.getAgeDescription())
        infoTxt = self.IFormatGZCreateCoords(hoodLink)
        self.IShowEnableButton(kIDBtnNeighborhoodPublic)
        self.IShowEnableButton(kIDBtnNeighborhoodSelect)

        self.hoodEntry = LinkListEntry(displayName, infoTxt, description, True, True)
        self.hoodEntry.setLinkStruct(info, spawnPoint = None, linkRule = PtLinkingRules.kOwnedBook) #create link to instance, use default spawnPoint

        self.IUpdateGuiEntry(kIDBtnNeighborhoodSelect, kIDTxtNeighborhoodName, kIDTxtNeighborhoodInfo, self.hoodEntry)

        if info.isPublic():
            hoodPubPriv = PtGetLocalizedString("Nexus.Neighborhood.MakePrivate")
        else:
            hoodPubPriv = PtGetLocalizedString("Nexus.Neighborhood.MakePublic")
        ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDTxtNeighborhoodPublic)).setStringW(hoodPubPriv)

    def IUpdateGUILinkList(self):
        if self.idCategorySelected == kCategoryPublic:
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDNameHeaderText)).setStringW(PtGetLocalizedString("Nexus.Headers.Name"))
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDPopHeaderText)).setStringW(PtGetLocalizedString("Nexus.Headers.Population"))
            # show our header sorting buttons
            self.IShowEnableButton(kIDNameHeaderBtn)
            self.IShowEnableButton(kIDPopHeaderBtn)
            #show current sort direction
            self.IToggleSortControls(True)
        else:
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDNameHeaderText)).setStringW(U"")
            ptGUIControlTextBox(NexusGUI.dialog.getControlFromTag(kIDPopHeaderText)).setStringW(U"")
            self.IToggleSortControls(False)

        ageList = self.categoryLinksList[self.idCategorySelected]

        if not ageList:
            return

        indexDisplayEnd = self.indexDisplayStart + min(len(ageList) - self.indexDisplayStart, kNumDisplayFields)

        if self.indexDisplayStart > 0:
            self.IShowEnableButton(kIDBtnScrollUp)
        else:
            self.IHideDisableButton(kIDBtnScrollUp)

        if len(ageList) > indexDisplayEnd:
            self.IShowEnableButton(kIDBtnScrollDn)
        else:
            self.IHideDisableButton(kIDBtnScrollDn)

        idTextbox = kIDTxtLinkNameFirst
        for i in range(self.indexDisplayStart, indexDisplayEnd):
            entry = ageList[i]
            self.IUpdateGuiEntry(idButton = idTextbox - 1 , idTxtName = idTextbox, idTxtInfo = idTextbox + 1,
                                    linkEntry = entry)
            self.IShowEnableButton(idTextbox - 1)
            self.IUpdateDeleteButton(idButton = idTextbox + 99, enable = entry.canDelete)
            idTextbox += 10

    def IUpdateLinks(self, categoryId = None):
        if not PtIsDialogLoaded(kNexusDialogName):
            PtDebugPrint("nxusBookMachine.IUpdateLinks: called without loaded dialog")
            return

        #return, if we update list, that is currently not displayed (list will be updated on display)
        if categoryId is not None and categoryId != self.idCategorySelected:
            return

        if self.idCategorySelected == kCategoryCity: # city links
            self.IUpdateCityLinksList()
        elif self.idCategorySelected == kCategoryPrivate: # private links
            self.IUpdatePrivateLinksList()
        elif self.idCategorySelected == kCategoryPublic: # public links
            self.IUpdatePublicLinksList()
        else:
            self.IUpdatePersonalLinksList()

        #assume that list have changed - clear it
        self.IClearEntryList()
        self.IUpdateGUILinkList()

    def IGetLinkPanelName(self, als):
        filename = als.getAgeInfo().getAgeFilename()
        PtDebugPrint("nxusBookMachine.getLinkPanelName(): Looking for link panel for '%s'" % filename)
        try:
            panels = kLinkPanels[filename]
            linkName = als.getSpawnPoint().getName()
            PtDebugPrint("nxusBookMachine.getLinkPanelName(): Found special case, trying for spawn point '%s'" % linkName)
            try:
                return panels[linkName]
            except KeyError:
                panel = panels['']
                PtDebugPrint("nxusBookMachine.getLinkPanelName(): Defaulting to '%s'" % panel)
                return panel
        except KeyError:
            panel = U"LinkPanel_" + filename
            PtDebugPrint("nxusBookMachine.getLinkPanelName(): Defaulting to '%s'" % panel)
            return panel

    def IDoLink(self):
        if self.presentedBookAls is None:
            ptDebugPrint("nxusBookMachine.IDoLink(): Tried to link without chosen book. This Is Bad Thing (R)")
        else:
            linkMgr = ptNetLinkingMgr()
            linkMgr.linkToAge(self.presentedBookAls)

    #TODO: Not revised. I'm not sure about this stuff... Is it needed?
    def DoErcanaAndAhnonayStuff(self, panel):
        print "nxusBookMachine.DoErcanaAndAhnonayStuff(): this age panel = ", panel
        if panel == "Ercana":
            ageFileName = "Ercana"
            ageInstanceName = "Er'cana"
        elif panel == "AhnonayCathedral" or panel == "Ahnonay":
            ageFileName = "AhnonayCathedral"
            ageInstanceName = "Ahnonay Cathedral"
        self.FindOrCreateGUIDChron(ageFileName)


    def FindOrCreateGUIDChron(self, ageFileName):
        print "FindOrCreateGUIDChron for: ", ageFileName
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

        pelletCaveGUID = ""
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("PelletBahroCave")
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            pelletCaveGUID = ageInfoNode.getAgeInstanceGuid()
            print "found pelletCaveGUID age chron, = ", pelletCaveGUID

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
            print "created pelletCaveGUID age chron, = ", pelletCaveGUID
