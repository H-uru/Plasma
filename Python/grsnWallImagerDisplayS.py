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
# Include Plasma code
from Plasma import *
from PlasmaTypes import *

# for save/load
import cPickle

## COMMENTED OUT by Jeff due to the re-write in the garrison wall

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################
southWall = ptAttribSceneobjectList(2,"South Wall",byObject=1)
##############################################################
# grsnWallImagerDisplayS
##############################################################

ReceiveInit = false

## for team light responders
kTeamLightsOn = 0
kTeamLightsOff = 1

## game states

kWaiting    = 0
kNorthSit   = 1
kSouthSit   = 2
kNorthSelect = 3
kSouthSelect = 4
kNorthReady = 5
kSouthReady = 6
kNorthPlayerEntry = 7
kSouthPlayerEntry = 8
kGameInProgress = 9
kNorthWin = 10
kSouthWin = 11
kSouthQuit = 12
kNorthQuit = 13


class grsnWallImagerDisplayS(ptResponder):
   
    # constants
    
    def __init__(self):
        "construction"
        PtDebugPrint("grsnWallImagerDisplayS::init begin")
        ptResponder.__init__(self)
        self.id = 52397
        self.version = 1
        PtDebugPrint("grsnWallImagerDisplayS::init end")        
"""
    def OnServerInitComplete(self):
        global ReceiveInit
        
        PtDebugPrint("grsnWallPython::OnServerInitComplete")        
        solo = true
        if len(PtGetPlayerList()):
            solo = false
            ReceiveInit = true
            return
        else:
            print"solo in climbing wall"
            
        
    def OnClimbingWallInit(self,type,state,value):
        global ReceiveInit
        
        print"grsnClimbingWall::OnClimbingWallInit type ",type," state ",state," value ",value
        if (ReceiveInit == false):
            print"failed to receive init"
            return
        if (type == ptClimbingWallMsgType.kEndGameState):
            ReceiveInit = false
            print "finished receiving total game state"
        
        if (type == ptClimbingWallMsgType.kTotalGameState):
            print "begin receiving total game state"
        
        elif (type == ptClimbingWallMsgType.kAddBlocker and state > 0 and value == 0):
            southWall.value[state].runAttachedResponder(kTeamLightsOn)

    def OnClimbingWallEvent(self,type,state,value):
        
        if (type == ptClimbingWallMsgType.kAddBlocker and value == false):            #display wall settings
            southWall.value[state].runAttachedResponder(kTeamLightsOn)
            print"Imager display S drawing wall index",state
                    
        elif (type == ptClimbingWallMsgType.kRemoveBlocker and value == false):
            southWall.value[state].runAttachedResponder(kTeamLightsOff)
            print"Imager display S clearing wall index",state
        
        elif (type == ptClimbingWallMsgType.kNewState):
            if (state == ptClimbingWallMsgState.kSouthSit or state == ptClimbingWallMsgState.kNorthSit ):
                #clear wall settings
                i = 0
                while (i < 171):
                    southWall.value[i].runAttachedResponder(kTeamLightsOff)
                    i = i + 1
"""
