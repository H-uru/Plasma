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
Module: ahnySaveCloth
Age: Most post-prime ages
Date: January 2004
Author: Adam Van Ornum
Sets a save point
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
Activator = ptAttribActivator(1,"Activator: Cloth Clickable")
OneShotResp = ptAttribResponder(2, "Resp: One Shot")
clothID  = ptAttribString(3,"save cloth ID")

avatar          = None
link            = None
whereAmI        = 0
gotSC           = 0
sdlSC           = ""

class ahnySaveCloth(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5424
        self.version = 1
        print "DEBUG: ahnySaveCloth.__init__: v.", self.version

    def OnFirstUpdate(self):
        global sdlSC
        global gotSC
        global link
        global whereAmI

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
                            link = chron.getValue()

        agevault = ptAgeVault()
        ageinfo = agevault.getAgeInfo()
        guid = ageinfo.getAgeInstanceGuid()

        if guid == link:
            ageSDL = PtGetAgeSDL()
            sphere = ageSDL["ahnyCurrentSphere"][0]
            
            linkmgr = ptNetLinkingMgr()
            curLink = linkmgr.getCurrAgeLink()
            spawnPoint = curLink.getSpawnPoint()

            spTitle = spawnPoint.getTitle()
            spName = spawnPoint.getName()

            if spTitle == "SCSavePoint":
                if spName == "SaveClothPoint7" or spName == "SaveClothPoint8":
                    print "linking to hub or hut"
                    whereAmI = 4
                else:
                    offset = str(ageSDL["ahnyCurrentOffset"][0])
                    print "Ahnonay.OnPageLoad(): Sphere0%s loaded with offset:%s" % (sphere, offset)
                    whereAmI = (int(sphere) - int(offset)) % 4
                    if whereAmI == 0:
                        whereAmI = 4
            else:
                whereAmI = sphere
            print "ahnySaveCloth.OnServerInitComplete(): I am age owner in %d" % (whereAmI)

        # SaveCloth SDL stuff, for use with POTS symbols
        sdlSC = "ahnyGotSaveCloth" + clothID.value
        try:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(sdlSC,1,1)
            ageSDL.sendToClients(sdlSC)
            ageSDL.setNotify(self.key,sdlSC,0.0)
            gotSC = ageSDL[sdlSC][0]
            print "ahnySaveCloth.OnServerInitComplete():\t found sdl: ",sdlSC,", which = ",gotSC
        except:
            print "ERROR.  Couldn't find sdl: ",sdlSC,", defaulting to 0"

        ageSDL.setFlags("ahnyCurrentSphere",1,1)
        ageSDL.sendToClients("ahnyCurrentSphere")
        ageSDL.setNotify(self.key,"ahnyCurrentSphere",0.0)

        ageSDL.setFlags("ahnyCurrentOffset",1,1)
        ageSDL.sendToClients("ahnyCurrentOffset")
        ageSDL.setNotify(self.key,"ahnyCurrentOffset",0.0)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global gotSC
        global sdlSC

        if VARname != sdlSC:
            return
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("xSaveCloth.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[sdlSC][0]))
        gotSC = ageSDL[sdlSC][0]

    def OnNotify(self,state,id,events):
        global avatar
        global whereAmI
        global link

        print "DEBUG: ahnySaveCloth::onNotify, id ",id

        if not state:
            return

        if id == Activator.id:
            #Activator.disable()
            avatar = PtFindAvatar(events)
            OneShotResp.run(self.key, avatar=PtFindAvatar(events)) # run the oneshot

        elif id == OneShotResp.id and avatar == PtGetLocalAvatar():
            agevault = ptAgeVault()
            ageinfo = agevault.getAgeInfo()
            guid = ageinfo.getAgeInstanceGuid()

            if guid == link:
                print "I'm the age owner, setting spawnpoint"
                #Activator.enable()
                
                ageSDL = PtGetAgeSDL()
                sphere = ageSDL["ahnyCurrentSphere"][0]
                offset = (sphere - whereAmI) % 4
                print "ahnySaveCloth.OnNotify: Offset = %d" % (offset)
                ageSDL["ahnyCurrentOffset"] = (offset,)

                agevault = ptAgeVault()
                ageinfo = agevault.getAgeInfo()
                spawn = None

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
                                if chron and chron.getName() == "AhnonaySpawnPoints":
                                    spawn = chron.getValue().split(";")
                                    newSpawn = "%s;SCSavePoint,SaveClothPoint%s" % (spawn[0],clothID.value)
                                    print newSpawn
                                    chron.setValue(newSpawn) 
                                    if not gotSC:
                                        ageSDL = PtGetAgeSDL()
                                        ageSDL[sdlSC] = (1,)
                                    return

                print "ahnySaveCloth.OnNotify(): ERROR: couldn't find chron node"


            else:
                print "I'm not the age owner, so I don't do anything."








            '''
            vault = ptVault()
            myAges = vault.getAgesIOwnFolder()
            myAges = myAges.getChildNodeRefList()
            for ageInfo in myAges:
                link = ageInfo.getChild()
                link = link.upcastToAgeLinkNode()
                info = link.getAgeInfo()
                if not info:
                    continue
                ageName = info.getAgeFilename()
                spawnPoints = link.getSpawnPoints()
    
                if ageName == "Ahnonay":
                    ahnySDL = info.getAgeSDL()
                    ahnyRecord = ahnySDL.getStateDataRecord()
                    currentCloth = ahnyRecord.findVar("ahnyCurrentSaveCloth")
                    currentSphere = ahnyRecord.findVar("ahnyCurrentSphere")
                    if (clothID.value):
                        clothNumber = int(clothID.value)
                        activeSphere = currentSphere.getInt(0)

                        agevault = ptAgeVault()
                        currentage = int(agevault.getAgeInfo().getAgeFilename()[-1])
                        print "currently in sphere:", currentage

                        currentpos = (currentage - activeSphere) % 4

                        if clothNumber > 6 and clothNumber < 25:
                            clothOffset = clothNumber % 6
                            if clothOffset == 0:
                                clothOffset = 6
                        else:
                            clothOffset = clothNumber
                        
                        currentCloth.setInt(clothOffset,0)
                        currentCloth.setInt(currentpos,1)
                        print"current save cloth updated to number", clothOffset, " from cloth value of", clothNumber
                        print"current save position updated to:", currentpos
                    else:
                        print"missing sphere identifier string!"
                    ahnySDL.setStateDataRecord(ahnyRecord)
                    ahnySDL.save()
                    return
            '''

        else:
            PtDebugPrint("ERROR: ahnySaveCloth.OnNotify: Error trying to access the Vault.")

