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
Module: Ahnonay.py
Age: Ahnonay
Date: June 2003
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaNetConstants import *
from xPsnlVaultSDL import *

spherePages  = ["Ahnonay_District_ahnySphere01",
                "Ahnonay_District_ahnySphere02",
                "Ahnonay_District_ahnySphere03",
                "Ahnonay_District_ahnySphere04",
                "Ahnonay_ahnySphere01",
                "Ahnonay_ahnySphere02",
                "Ahnonay_ahnySphere03",
                "Ahnonay_ahnySphere04"]

class Ahnonay(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5399
        self.version = 1

    def OnFirstUpdate(self):
        pass

    def OnServerInitComplete(self):
        agevault = ptAgeVault()
        ageinfo = agevault.getAgeInfo()
        guid = ageinfo.getAgeInstanceGuid()
        linkid = None
        locked = None
        volatile = None
        spawn = None
        owner = None
        myID = str(PtGetClientIDFromAvatarKey(PtGetLocalAvatar().getKey()))

        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("Personal")

        vault = ptVault()
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
                        if chron and chron.getName() == "AhnonayLink":
                            linkid = chron
                            print "Ahnonay.OnServerInitComplete(): Link Chron already exists: %s" % (linkid.getValue())
                        elif chron and chron.getName() == "AhnonayLocked":
                            locked = chron
                            print "Ahnonay.OnServerInitComplete(): Locked Chron already exists: %s" % (locked.getValue())
                        elif chron and chron.getName() == "AhnonayVolatile":
                            volatile = chron
                            print "Ahnonay.OnServerInitComplete(): Volatile Chron already exists: %s" % (volatile.getValue())
                        elif chron and chron.getName() == "AhnonaySpawnPoints":
                            spawn = chron
                            print "Ahnonay.OnServerInitComplete(): Spawn Chron already exists: %s" % (spawn.getValue())
                        elif chron and chron.getName() == "AhnonayOwner":
                            owner = chron
                    break

        if owner == None:
            print "I am not the age owner, and I don't have my own Ahnonay"
        elif owner.getValue() == myID:
            if linkid == None:
                print "Ahnonay.OnServerInitComplete(): Link Chron not found, creating"
                newNode = ptVaultChronicleNode(0)
                newNode.chronicleSetName("AhnonayLink")
                newNode.chronicleSetValue(guid)
                ageDataFolder.addNode(newNode)

            if locked == None:
                print "Ahnonay.OnServerInitComplete(): Locked Chron not found, creating"
                newNode = ptVaultChronicleNode(0)
                newNode.chronicleSetName("AhnonayLocked")
                newNode.chronicleSetValue("1")
                ageDataFolder.addNode(newNode)

            if volatile == None:
                print "Ahnonay.OnServerInitComplete(): Volatile Chron not found, creating"
                newNode = ptVaultChronicleNode(0)
                newNode.chronicleSetName("AhnonayVolatile")
                newNode.chronicleSetValue("0")
                ageDataFolder.addNode(newNode)

            if spawn == None:
                print "Ahnonay.OnServerInitComplete(): Spawn Chron not found, creating"
                newNode = ptVaultChronicleNode(0)
                newNode.chronicleSetName("AhnonaySpawnPoints")
                newNode.chronicleSetValue("Default,LinkInPointDefault")
                ageDataFolder.addNode(newNode)

            if volatile and linkid:
                if volatile.getValue() == "1" and guid != linkid.getValue():
                    print "Ahnonay.OnServerInitComplete(): In a new instance of Ahnonay so setting new vars"
                    linkid.setValue(guid)
                    locked.setValue("1")
                    volatile.setValue("0")
                    spawn.setValue("Default,LinkInPointDefault")
        else:
            print "I am not the age owner, but I do have my own Ahnonay"


        ageSDL = PtGetAgeSDL()
        sphere = ageSDL["ahnyCurrentSphere"][0]
        
        if sphere > 4:
            sphere = 1
            ageSDL["ahnyCurrentSphere"] = (1,)
        
        linkmgr = ptNetLinkingMgr()
        link = linkmgr.getCurrAgeLink()
        spawnPoint = link.getSpawnPoint()

        spTitle = spawnPoint.getTitle()
        spName = spawnPoint.getName()

        if spTitle == "SCSavePoint":
            if spName == "SaveClothPoint7" or spName == "SaveClothPoint8":
                print "linking to hub or hut"
                newSphere = 4
            else:
                offset = str(ageSDL["ahnyCurrentOffset"][0])
                print "Ahnonay.OnPageLoad(): Sphere0%s loaded with offset:%s" % (sphere, offset)
                newSphere = (int(sphere) - int(offset)) % 4
                if newSphere == 0:
                    newSphere = 4
        else:
            newSphere = sphere
            
        if newSphere == 1:
            PtPageInNode("Sphere01BuildingInterior")
            PtPageInNode("MaintRoom01")
            PtPageInNode("ahnySphere01")
        elif newSphere == 2:
            PtPageInNode("MaintRoom02")
            PtPageInNode("ahnySphere02")
        elif newSphere == 3:
            PtPageInNode("MaintRoom03")
            PtPageInNode("ahnySphere03")
        elif newSphere == 4:
            PtPageInNode("Vortex")
            PtPageInNode("Hub")
            PtPageInNode("MaintRoom04")
            PtPageInNode("EngineerHut")
            PtPageInNode("ahnySphere04")

    ###########################
    def OnPageLoad(self,what,who):
        global spherePages
        print "Ahnonay.OnPageLoad(): what=%s who=%s" % (what, who)

        if what == kLoaded:
            if who in spherePages: 
                ageSDL = PtGetAgeSDL()
                sphere = str(ageSDL["ahnyCurrentSphere"][0])
                offset = str(ageSDL["ahnyCurrentOffset"][0])
                print "Ahnonay.OnPageLoad(): Sphere0%s loaded with offset:%s" % (sphere, offset)
                
                linkmgr = ptNetLinkingMgr()
                link = linkmgr.getCurrAgeLink()
                spawnPoint = link.getSpawnPoint()

                spTitle = spawnPoint.getTitle()
                spName = spawnPoint.getName()

                if spTitle == "SCSavePoint":
                    if spName == "SaveClothPoint7" or spName == "SaveClothPoint8":
                        print "linking to hub or hut"
                        newSphere = 4
                    else:
                        newSphere = (int(sphere) - int(offset)) % 4
                        if newSphere == 0:
                            newSphere = 4
                    spawnPoint = spName + str(newSphere)
                    PtGetLocalAvatar().physics.warpObj(PtFindSceneobject(spawnPoint, "Ahnonay").getKey())
                else:
                    defaultLink = "LinkInPointSphere0%s" % (sphere)
                    PtGetLocalAvatar().physics.warpObj(PtFindSceneobject(defaultLink, "Ahnonay").getKey())

    ###########################
    def OnNotify(self,state,id,events):
        pass

    ###########################
    def OnBackdoorMsg(self, target, param):
        ageSDL = PtGetAgeSDL()
        if target == "sphere":
            if self.sceneobject.isLocallyOwned():
                ageSDL["ahnyCurrentSphere"] = (int(param),)
