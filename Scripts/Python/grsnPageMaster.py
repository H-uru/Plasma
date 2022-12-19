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
Module: grsnPageMaster.py
Age: Garrison
Date: December 2006
manages page loading for public vs. non-public Garrisons
"""

from Plasma import *
from PlasmaTypes import *


IsPublic = 0


class grsnPageMaster(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 50100
        self.version = 1

        global IsPublic

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
            PtDebugPrint("grsnPageMaster.__init__(): Garrison version = public")
        else:
            PtDebugPrint("grsnPageMaster.__init__(): Garrison version = Yeesha")

        pages = []

        # Add the common pages
        ## actually, we'll just set these to load automatically in the .age file
        #pages += ["grsnWellInner","grsnWellFirstFloorRooms","grsnWellCameras"]

        # for public version, add any pages specific only for that
        if IsPublic:
            pass
        # for non-public version, add all the remaining pages
        else:
            pages += ["grsnWellOccluders","grsnWellSecondFloorGearRoom","grsnElevator","grsnExterior"]
            pages += ["grsnVeranda","grsnVerandaExterior","grsnObsRoom01Imager","grsnObsRoom02Imager","grsnPrison","grsnPrisonTunnels"]
            pages += ["grsnTeamRoom01","grsnTeamRoom02","grsnTrainingCenterHalls","grsnTrainingCenterMudRooms","grsnTrainingCntrLinkRm"]
            pages += ["TrnCtrControlRoom01","TrnCtrControlRoom02","trainingCenterObservationRooms","NexusBlackRoom","NexusWhiteRoom"]
            pages += ["WallRoom","grsnWallRoomClimbingPhys"]
            pages += ["FemaleElevatorArrivingBottom","FemaleElevatorArrivingTop","FemaleElevatorLeavingBottom","FemaleElevatorLeavingTop"]
            pages += ["FemaleLandingRoll","FemaleReadyIdle","FemaleReadyJump","FemaleTubeFall"]
            pages += ["FemaleWallClimbDismountDown","FemaleWallClimbDismountLeft","FemaleWallClimbDismountRight","FemaleWallClimbDismountUp"]
            pages += ["FemaleWallClimbDown","FemaleWallClimbFallOff","FemaleWallClimbIdle","FemaleWallClimbLeft"]
            pages += ["FemaleWallClimbMountDown","FemaleWallClimbMountLeft","FemaleWallClimbMountRight","FemaleWallClimbMountUp"]
            pages += ["FemaleWallClimbRelease","FemaleWallClimbRight","FemaleWallClimbUp"]
            pages += ["MaleElevatorArrivingBottom","MaleElevatorArrivingTop","MaleElevatorLeavingBottom","MaleElevatorLeavingTop"]
            pages += ["MaleLandingRoll","MaleReadyIdle","MaleReadyJump","MaleTubeFall"]
            pages += ["MaleWallClimbDismountDown","MaleWallClimbDismountLeft","MaleWallClimbDismountRight","MaleWallClimbDismountUp"]
            pages += ["MaleWallClimbDown","MaleWallClimbFallOff","MaleWallClimbIdle","MaleWallClimbLeft"]
            pages += ["MaleWallClimbMountDown","MaleWallClimbMountLeft","MaleWallClimbMountRight","MaleWallClimbMountUp"]
            pages += ["MaleWallClimbRelease","MaleWallClimbRight","MaleWallClimbUp"]
            
        PtPageInNode(pages)


    def OnFirstUpdate(self):
        pass

    
    def OnServerInitComplete(self):
        pass


    def Load(self):
        pass


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        pass


    def OnNotify(self,state,id,events):
        pass

