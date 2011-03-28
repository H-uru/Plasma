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
Module: ahnyPressurePlates
Age: Ahnonay
Date: April, 2007
Author: Derek Odell
Ahnonay Quab control
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import xLinkingBookDefs
import xRandom


# define the attributes that will be entered in max
zones                       = ptAttribActivator(1, "act: Zone Detectors")
respClockLights             = ptAttribResponderList(2, "resp: Clock Lights", statelist=["on","off"], byObject=1)
zoneObjects                 = ptAttribSceneobjectList(3, "obj: Zones")
SDLOccupied                 = ptAttribString(4, "str: SDL Occupied Zones")
SDLTrees                    = ptAttribString(5, "str: SDL Trees (optional)")
bookClickable               = ptAttribActivator(6, "act: Book Clickable")
SeekBehavior                = ptAttribBehavior(7, "beh: Smart Seek To Book")
Sphere                      = ptAttribDropDownList(8, "Which Sphere?", ("Sphere01", "Sphere02", "Sphere03", "Sphere04"))
respLinkResponder           = ptAttribResponder(9, "resp: Link To Cathedral")
respSphereRotate            = ptAttribResponder(10, "resp: Sphere Rotation SFX")

# globals
respLightList       = []
objZoneList         = []
LocalAvatar         = None
gLinkingBook        = None
treeToZoneKey       = [1,3,4,5,8,9,10,12,13,15,16,18,21,22,24]

#====================================
class ahnyPressurePlates(ptModifier):
    ###########################
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5947
        version = 1
        self.version = version
        print "__init__ahnyPressurePlates v%d " % (version)

    ###########################
    def OnFirstUpdate(self):
        global respLightList
        global objZoneList
        global treeToZoneKey

        try:
            ageSDL = PtGetAgeSDL()
            ageSDL[SDLOccupied.value][0]
        except:
            print "ahnyPressurePlates.OnFirstUpdate(): ERROR --- Cannot find the Ahnonay Age SDL"
            ageSDL[SDLOccupied.value] = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)

        ageSDL.setFlags("ahnyCurrentSphere",1,1)
        ageSDL.sendToClients("ahnyCurrentSphere")
        ageSDL.setNotify(self.key,"ahnyCurrentSphere",0.0)

        ageSDL.setFlags(SDLOccupied.value,1,1)
        ageSDL.sendToClients(SDLOccupied.value)
        ageSDL.setNotify(self.key,SDLOccupied.value,0.0)

        if not len(PtGetPlayerList()):
            ageSDL[SDLOccupied.value] = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)


        for light in respClockLights.value:
            thisLight = light.getName()
            respLightList.append(thisLight)

        for zone in zoneObjects.value:
            thisZone = zone.getName()
            objZoneList.append(thisZone)

        if respLightList != []:
            idx = 0
            for occupants in ageSDL[SDLOccupied.value]:
                if occupants:
                    respClockLights.run(self.key, state='on', objectName=respLightList[idx])
                idx += 1
        if Sphere.value == "Sphere02":
            try:
                ageSDL[SDLTrees.value][0]
            except:
                print "ahnyPressurePlates.OnFirstUpdate(): ERROR --- Cannot find the Ahnonay Age SDL"
                ageSDL[SDLTrees.value] = (1,1,1,1,1,1,1,1,1,1,1,1,1,1,1)

            ageSDL.setFlags(SDLTrees.value,1,1)
            ageSDL.sendToClients(SDLTrees.value)
            ageSDL.setNotify(self.key,SDLTrees.value,0.0)
            
            occupiedZones = list(ageSDL[SDLOccupied.value])
            occupiedTrees = list(ageSDL[SDLTrees.value])
            for index in treeToZoneKey:
                if occupiedZones[index] == 0 and occupiedTrees[treeToZoneKey.index(index)] == 1:
                    respClockLights.run(self.key, state='on', objectName=respLightList[index], netForce=1 )
            ageSDL[SDLOccupied.value] = tuple(occupiedZones)


    ###########################
    def OnServerInitComplete(self):
        pass

    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname == "ahnyCurrentSphere" and respSphereRotate.value != []:
            print "ahnyPressurePlates.OnSDLNotify(): playing audio SFX"
            respSphereRotate.run(self.key)

    ###########################
    def OnNotify(self,state,id,events):
        global respLightList
        global objZoneList
        global LocalAvatar
        
        #print "ahnyPressurePlates.OnNotify: state=%s id=%d events=" % (state, id), events

        if id == zones.id:
            for event in events:
                if (event[0] == kCollisionEvent) and self.sceneobject.isLocallyOwned():
                    ageSDL = PtGetAgeSDL()
                    region = event[3]
                    regName = region.getName()
                    for zone in zoneObjects.value:
                        zoneName = zone.getName()
                        if zoneName == regName:
                            ageSDL = PtGetAgeSDL()
                            index = objZoneList.index(zoneName)
                            occupiedZones = list(ageSDL[SDLOccupied.value])
                            #print "Zone: %s Index: %d Occupied: %s" % (zoneName,index,str(occupiedZones))
                            if event[1] == 1: #We are entering
                                if occupiedZones[index] != 255: #avoid overflow
                                    occupiedZones[index]= occupiedZones[index] + 1
                                if respLightList != [] and occupiedZones[index]==1: # if we are now equal to one run the responder 
                                    respClockLights.run(self.key, state='on', objectName=respLightList[index], netForce=1 )
                                print "%s - enter %s" % (str(occupiedZones), str(index))
                            else: #this should be exiting
                                if occupiedZones[index] != 0: #only subtract if we are not zero don't want to overflow
                                    occupiedZones[index] = occupiedZones[index] -1
                                if Sphere.value == "Sphere02" and index in treeToZoneKey:
                                    if ((not ageSDL[SDLTrees.value][treeToZoneKey.index(index)]) and( occupiedZones[index] == 0)):
                                        if respLightList != []:
                                            respClockLights.run(self.key, state='off', objectName=respLightList[index], netForce=1)
                                else:

                                    if (respLightList != []) and (occupiedZones[index] == 0):# 
                                        respClockLights.run(self.key, state='off', objectName=respLightList[index] , netForce=1)
                                print "%s - exit %s" % (str(occupiedZones), str(index))
                            ageSDL[SDLOccupied.value] = tuple(occupiedZones)
                            #print "Occupied: %s" % (str(occupiedZones))

        # is it a clickable book on a pedestal?
        elif id == bookClickable.id and PtFindAvatar(events) == PtGetLocalAvatar() and state:
            PtToggleAvatarClickability(false)
            bookClickable.disable()
            LocalAvatar = PtFindAvatar(events)
            SeekBehavior.run(LocalAvatar)

        # is it the seek behavior because we clicked on a book ourself?    
        elif id == SeekBehavior.id and PtFindAvatar(events) == PtGetLocalAvatar():
            for event in events:
                if event[0] == kMultiStageEvent and event[2] == kEnterStage: # Smart seek completed. Exit multistage, and show GUI.
                    SeekBehavior.gotoStage(LocalAvatar, -1) 
                    self.IShowBook()

        else:
            for event in events:
                # is it from the OpenBook? (we only have one book to worry about)
                if event[0] == PtEventType.kBook:
                    print "ahnyPressurePlates: BookNotify  event=%d, id=%d" % (event[1],event[2])
                    if event[1] == PtBookEventTypes.kNotifyImageLink:
                        if event[2] >= xLinkingBookDefs.kFirstLinkPanelID or event[2] == xLinkingBookDefs.kBookMarkID:
                            print "ahnyPressurePlates:Book: hit linking panel %s" % (event[2])
                            self.HideBook(1)

                            ageSDL = PtGetAgeSDL()
                            print ageSDL[SDLOccupied.value]
                            if self.RegionsEmpty():
                                print "Sphere rotating"
                                ageSDL = PtGetAgeSDL()
                                currentSphere = ageSDL["ahnyCurrentSphere"][0]
                                if currentSphere == 3 or currentSphere == 4:
                                    ageSDL["ahnyCurrentSphere"] = (1,)
                                else:
                                    ageSDL["ahnyCurrentSphere"] = ((currentSphere + 1),)
                            else:
                                print "Sphere staying put"

                            respLinkResponder.run(self.key, avatar=PtGetLocalAvatar(),netPropagate=0)

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
                                    currentSphere = ahnyRecord.findVar("ahnyCurrentSphere")
                                    if (sphere.value == "1"):
                                        currentSphere.setInt(2,0)
                                    elif (sphere.value == "2"):
                                        currentSphere.setInt(3,0)
                                    elif (sphere.value == "3"):
                                        currentSphere.setInt(1,0)
                                    elif (sphere.value == "4"):
                                        currentSphere.setInt(1,0)
                                    else:
                                        print"missing sphere identifier string!"
                                    ahnySDL.setStateDataRecord(ahnyRecord)
                                    ahnySDL.save()
                                    print"advanced from sphere ",sphere.value
                                    return
                            '''
        
                    elif event[1] == PtBookEventTypes.kNotifyShow:
                        print "ahnyLinkBookGUIPopup:Book: NotifyShow"
                        PtSendKIMessage(kEnableKIandBB,0)

                    elif event[1] == PtBookEventTypes.kNotifyHide:
                        print "ahnyLinkBookGUIPopup:Book: NotifyHide"
                        PtToggleAvatarClickability(true)
                        bookClickable.enable()

    ###########################
    def RegionsEmpty(self):
        ageSDL = PtGetAgeSDL()
        occupantList = list(ageSDL[SDLOccupied.value])

        if Sphere.value == "Sphere01":
            quabs = ageSDL["ahnyQuabs"][0]
            if quabs:
                print "ahnyPressurePlates: not all quabs kicked off"
                return false
        elif Sphere.value == "Sphere02":
            treeList = list(ageSDL[SDLTrees.value])
            for tree in treeList:
                if tree:
                    print "ahnyPressurePlates: not all trees knocked over"
                    return false

        for zone in occupantList[1:]:
            if zone:
                print "ahnyPressurePlates: some zones still occupied"
                return false

        if occupantList[0] == 1:
            return true

        print "ahnyPressurePlates: book zone still occupied"
        return false

    ###########################
    def IShowBook(self):
        global gLinkingBook

        try:
            params = xLinkingBookDefs.xAgeLinkingBooks["AhnonayCathedral"]

            if len(params) == 6:
                sharable,width,height,stampdef,bookdef,gui = params
            elif len(params) == 5:
                sharable,width,height,stampdef,bookdef = params
                gui = "BkBook"
            else:
                return

            PtSendKIMessage(kDisableKIandBB,0)
            bookdef = bookdef.replace("%s", "")
            gLinkingBook = ptBook(bookdef,self.key)
            gLinkingBook.setSize( width, height )
            gLinkingBook.setGUI(gui)
            gLinkingBook.show(1)

        except LookupError:
            print "ahnyLinkBookGUIPopup: could not find age AhnonayCathedral's linking panel"

    ###########################
    def HideBook(self, islinking = 0):
        global gLinkingBook
        
        PtToggleAvatarClickability(true) # enable me as clickable
        if gLinkingBook:
            gLinkingBook.hide()
        
















"""

    # utility functions:


    ###########################
    def IGetAgeFilename(self):
        try:
            name = xLinkingBookDefs.xLinkDestinations[TargetAge.value][0]
        except:
            PtDebugPrint("IGetAgeFilename(): " + TargetAge.value + " is missing from the xLinkDestinations table, attempting to use it as the value")
            name = TargetAge.value
        return name

    ###########################
    def IGetAgeInstanceName(self):
        try:
            name = xLinkingBookDefs.xLinkDestinations[TargetAge.value][0]
        except:
            PtDebugPrint("IGetAgeInstanceName(): " + TargetAge.value + " is missing from the xLinkDestinations table, attempting to use it as the value")
            name = TargetAge.value
        return name


    ###########################
    def IGetAgeSpawnPoint(self):
        try:
            name = xLinkingBookDefs.xLinkDestinations[TargetAge.value][1]
        except:
            PtDebugPrint("IGetAgeSpawnPoint(): " + TargetAge.value + " is missing from the xLinkDestinations table, attempting to use an empty string as the value")
            name = ""
        return name

"""
