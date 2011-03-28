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
Module: xSaveCloth
Age: Most post-prime ages
Date: January 2004
Author: Adam Van Ornum
Sets a save point
"""

from Plasma import *
from PlasmaTypes import *


# define the attributes that will be entered in max
Activator = ptAttribActivator(1,"Activator: Cloth Clickable")
OneShotResp = ptAttribResponder(2,"Resp: One Shot")
AnimResp = ptAttribResponder(3,"Anim responder",netForce=1)
soSpawnpoint = ptAttribSceneobject(4,"Spawn point scene object")
numSC = ptAttribInt(5,"SaveCloth #:")


# globals
CAM_DIVIDER = "~"
sdlBase = "GotSaveCloth"
#sdls = ["ercaGotSaveCloth1","ercaGotSaveCloth2","ercaGotSaveCloth3","ercaGotSaveCloth4",\
#        "ercaGotSaveCloth5","ercaGotSaveCloth6","ercaGotSaveCloth7"]
sdlSC = ""
gotSC = 0

# constants
kMaxSC = 7


class xSaveCloth(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5324
        self.version = 2
        print "DEBUG: xSaveCloth.__init__: v.", self.version


    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        global sdlSC
        global gotSC

        linkmgr = ptNetLinkingMgr()
        link = linkmgr.getCurrAgeLink()
        spawnPoint = link.getSpawnPoint()

        spTitle = spawnPoint.getTitle()
        spName = spawnPoint.getName()

        print "spawned into", spName, ", this save cloth handles", soSpawnpoint.value.getName()
        if spTitle.endswith("SavePoint") and spName == soSpawnpoint.value.getName():
            print "restoring camera stack for save point", spTitle, spName

            # Restore camera stack
            camstack = spawnPoint.getCameraStack()
            print "camera stack:", camstack
            if camstack != "":
                PtClearCameraStack()
                camlist = camstack.split(CAM_DIVIDER)

                age = PtGetAgeName()
                for x in camlist:
                    print "adding camera: |" + str(x) + "|"
                    try:
                        PtRebuildCameraStack(x, age)
                    except:
                        PtDebugPrint("ERROR: xSaveCloth.OnServerInitComplete: problem rebuilding camera stack...continuing")
            # Done restoring camera stack

        
        # SaveCloth SDL stuff, for use with POTS symbols
        if not (numSC.value > 0 and numSC.value <= kMaxSC):
            print "xSaveCloth.OnServerInitComplete(): ERROR! invalid save cloth # of ",numSC.value,", specified in MAX component.  Please revise..."
            return
        ageName = PtGetAgeName()
        if ageName == "Ercana":
            sdlPre = "erca"
#        UNCOMMENT THIS WHEN AHNONAY IS READY
#        elif ageName == "Ahnonay":
#            sdlPre = "ahny"
        else:
            print "xSaveCloth.py not updated for this age's SDL.  Ignoring SaveCloth SDL stuff..."
            return
        sdlSC = sdlPre + sdlBase + str(numSC.value)
        try:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(sdlSC,1,1)
            ageSDL.sendToClients(sdlSC)
            ageSDL.setNotify(self.key,sdlSC,0.0)
            gotSC = ageSDL[sdlSC][0]
            #print "xSaveCloth.OnServerInitComplete():\t found sdl: ",sdlSC,", which = ",gotSC
        except:
            print "ERROR.  Couldn't find sdl: ",sdlSC,", defaulting to 0"


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global gotSC

        if VARname != sdlSC:
            return
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("xSaveCloth.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d" % (VARname,SDLname,tag,ageSDL[sdlSC][0]))
        gotSC = ageSDL[sdlSC][0]


    def OnNotify(self,state,id,events):
        global ClothInUse
        global CAM_DIVIDER

        if not state:
            return

        if id == Activator.id:
            Activator.disable()
            OneShotResp.run(self.key, events=events) # run the oneshot
            return

        elif id == OneShotResp.id:
            vault = ptVault()
            if vault.amOwnerOfCurrentAge() and PtWasLocallyNotified(self.key):
                PtDebugPrint("DEBUG: xSaveCloth.OnNotify: am owner of current age, getting save point")
                #if 1:
                try:
                # Save the camera stack
                    camstack = ""
                    numcams = PtGetNumCameras()
                    for x in range(numcams):
                        camstack += (PtGetCameraNumber(x+1) + CAM_DIVIDER)
                    camstack = camstack[:-1]

                    print "camera stack:", camstack

                    # camera stack is now being saved as a spawn point on the owned age link
                    vault = ptVault()
                    ainfo = ptAgeInfoStruct()
                    savepoint = None
                    ainfo.setAgeFilename(PtGetAgeName())
                    agelink = vault.getOwnedAgeLink(ainfo)

                    if type(agelink) != type(None):
                        spawnpoints = agelink.getSpawnPoints()

                        for sp in spawnpoints:
                            if sp.getTitle() == "SCSavePoint":
                                savepoint = sp
                                break

                        if type(savepoint) != type(None):
                            agelink.removeSpawnPoint(savepoint.getName())
                            
                        savepoint = ptSpawnPointInfo("SCSavePoint", soSpawnpoint.value.getName())
                        savepoint.setCameraStack(camstack)

                        agelink.addSpawnPoint(savepoint)
                        agelink.save()
                    # Done saving the camera stack
                except:
                    PtDebugPrint("ERROR: xSaveCloth.OnNotify: error occurred doing the whole save point thing")

            AnimResp.run(self.key, events=events)

            if soSpawnpoint.sceneobject.isLocallyOwned():
                if not gotSC:
                    ageSDL = PtGetAgeSDL()
                    ageSDL[sdlSC] = (1,)

        elif id == AnimResp.id:
            Activator.enable()
            
        else:
            PtDebugPrint("ERROR: xSaveCloth.OnNotify: Error trying to access the Vault.")


