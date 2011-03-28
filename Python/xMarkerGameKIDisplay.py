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
Module: xMarkerGameKIDisplay
Age: global
Author: Tye Hooley
Date: Dec. 2007
This is the "new" python handler for Marker Games
"""
from PlasmaTypes import *
from PlasmaKITypes import *
from xMarkerGameUtils import *


#Default Values
kDefaultTimeLimit = 120
kDefaultGameType = PtMarkerGameTypes.kMarkerGameQuest


class xMarkerGameKIDisplay:
    "Houses the network and interface for displaying User-Created Marker Games in the KI"
    def __init__(self, KI, templateID = "", gameType = kDefaultGameType, gameName = "MarkerGame", cachedData = None ):
        #Set version info
        self.verMajor = 0
        self.verMinor = 1
        self.ver = "%i.%i" % (self.verMajor, self.verMinor)
        print "__init__(): xMarkerGameKIDisplay v%s" % self.ver

        #Load default marker game data
        self.gameData = MarkerGameData()

        #Store the KI's key... we use this for mini-game registration
        self.key = KI.key
        self.KI = KI

        #Setup default values for creating a game
        #This works as existing games will ignore these fields and return whatever is on the template!

        if gameName == "MarkerGame" or gameName == "":        
            playerName = PtGetLocalPlayer().getPlayerName()
            if playerName[-1] == "s":
                addToName = "'"
            else:
                addToName = "'s"
            gameName = str(PtGetLocalPlayer().getPlayerName()) + str(addToName) + " Marker Game"
        self.newGame = 1
        self.initialized = 0
        self.showMarkers = 0
        self.selectedMarker = -1

        #This is a hack, and I hope that nobody ever uses it unless they really know what they are doing!
        if cachedData != None:
            PtDebugPrint("DEBUG: xMarkerGameKIDisplay.__init__():\tLoading from cached data")
            self.gameData.copy(cachedData)
            return

        #if we have a templateID then we must store it as it will never be xmitted
        if templateID != "":
            PtDebugPrint("DEBUG: xMarkerGameKIDisplay.__init__():\tloading existing game templateID: %s" %templateID)
            self.gameData.data['svrGameTemplateID'] = templateID
            self.newGame = 0
        
        PtCreateMarkerGame(self.key, gameType, gameName, kDefaultTimeLimit , templateID)


    def __del__(self):
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.clearSelectedMarker()
        if not self.KI.markerGameManager.gameLoaded():
            mrkrDisplay.removeAllMarkers()



    #--------------------------------#
    # Incomming Game Server Messages #
    #--------------------------------#
    def registerTemplateCreated(self, msg):
        "Server sent a template created message"
        self.gameData.data['svrGameTemplateID'] = msg.templateID()
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.registerTemplateCreated():\tReceived template created msg for template: %s" %msg.templateID())
        self.SaveGameClientID(msg.getGameCli().gameID())
        self.NotifyGameReady()

    def registerPlayerJoin(self, msg):
        "Server reports a player has joined the game"
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.registerPlayerJoin():\tReceived Player Joined Message for player: %s" % msg.playerID())
        
        if msg.playerID() == PtGetLocalClientID():
            #Store the Game Client ID
            self.SaveGameClientID(msg.getGameCli().gameID())
            self.gameData.data['isPlayerJoined'] = 1   #we only need this if we actually try to start the game (as we'll use the same data)
            

    def registerMarker(self, msg):
        "received register marker message from the game server, and show the marker if we're editing"
        #Save the marker
        marker = MarkerData()
        marker.data['x']    = msg.x()
        marker.data['y']    = msg.y()
        marker.data['z']    = msg.z()
        marker.data['id']   = msg.markerId()
        marker.data['name'] = msg.name()
        marker.data['age']  = msg.age()

        self.gameData.data['markers'].append(marker)
        self.gameData.data['numMarkers'] += 1
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay():\tRegistered Marker: %s" %marker)

        #Show the marker if necessary
        if self.showMarkers:
            mrkrDisplay = ptMarkerMgr()
            ageName = PtGetAgeInfo().getAgeFilename()
            #Only add markers that exist within this age (only a concern for quest games)
            if marker.data['age'].lower() != unicode(ageName).lower():
                return
            mrkrDisplay.addMarker(marker.data['x'], marker.data['y'], marker.data['z'], marker.data['id'], 0)

    def registerMarkerCaptured(self, msg):
        "received a marker captured message from the game server, update our marker count"
        markerID = msg.markerId()
        mrkrDisplay = ptMarkerMgr()

        mrkrDisplay.captureQuestMarker(markerID,1)
        for marker in self.gameData.data['markers']:
            marker = marker.data
            if marker['id'] == markerID:
                marker['captured'] = 1
                break
        self.gameData.data['numCapturedMarkers'] += 1

    def registerGameName(self, msg):
        "sever sent a game name change"
        self.gameData.data['svrGameName'] = msg.name()
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.registerMarkerGameName():\tChange game name to: %s" % msg.name())
        #Store the Game Client ID
        self.SaveGameClientID(msg.getGameCli().gameID())
        self.NotifyGameReady()

    def registerGameType(self, msg):
        "server sent a game type message"
        self.gameData.data['svrGameTypeID'] = msg.gameType()
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.registerGameType(): Received Game type Message for game type: %s" % msg.gameType())
        #Store the Game Client ID
        self.SaveGameClientID(msg.getGameCli().gameID())
        self.NotifyGameReady()

    def registerDeleteMarker(self, msg):
        "received a marker delete message from the server, need to process it!"
        markerID = msg.markerId()
        #remove the marker from the display
        markerDisplay = ptMarkerMgr()
        markerDisplay.clearSelectedMarker()
        markerDisplay.removeMarker(markerID)

        #remove the marker from the internal marker list
        i = 0
        for x in self.gameData.data['markers']:
            if x.data['id'] == markerID:
                del self.gameData.data['markers'][i]
                self.selectedMarker = -1
                self.gameData.data['numMarkers'] -= 1
                PtDebugPrint("DEBUG: xMarkerGameKIDisplay.registerDeleteMarker():\tFound and deleted markerID: %s." %markerID)
                break
            i += 1

    def registerMarkerNameChanged(self, msg):
        "received a marker name changed message from the server, need to process it!"
        markerID = msg.markerId()
        #Update the marker data
        for marker in self.gameData.data['markers']:
            data = marker.data
            #Only add markers that exist within this age (only a concern for quest games)
            if data['id'] == markerID:
                data['name'] = msg.name()
                return
            

    def registerResetGame(self, msg):
        "received a marker game reset message"
        # normally this occurs on the game itself not the display, but often we have the same data set, so we'll have to update it here!
        self.gameData.data['numCapturedMarkers'] = 0
        
        #Reset Markers
        for marker in self.gameData.data['markers']:
            marker = marker.data
            if marker['captured']:
                marker['captured'] = 0

        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.registerResetGame():\tResetting KI Marker Display's game progress...")
        


    #--------------------------------#
    #    Other External Functions    #
    #--------------------------------#
    def isMyMsg(self, msg):
        "Returns if this is my message (i.e. I need to register it)"
        #Note this only works after the player has been joined and the template created
        if msg.getGameCli().gameID() == self.gameData.data['svrGameClientID']:
            return 1
        else:
            return 0

    def setGameName(self, name):
        "Sets the name of an existing game"
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.changeGameName(name)
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.setGameName():\tSending Game Name Change Request: %s" %name)

    def editMarkers(self):
        "Displays all the markers for editing purposes!"
        #Just in case we've got markers showing, we better delete them!
        self.showMarkers = 1
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.removeAllMarkers()
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.editMarkers():\tEntering Edit Mode: Displaying all markers")

        ageName = unicode(PtGetAgeInfo().getAgeFilename()).lower()
       
        #Update the marker display manager
        for marker in self.gameData.data['markers']:
            data = marker.data
            #Only add markers that exist within this age (only a concern for quest games)
            if data['age'].lower() != ageName:
                return
            mrkrDisplay.addMarker(data['x'], data['y'], data['z'], data['id'], 0)

    def exitEditMarkers(self):
        "exits the edit mode"
        if self.showMarkers ==0:
            #We were not editing!
            return
        self.showMarkers = 0
        self.selectedMarker = -1
        markerDisplay = ptMarkerMgr()
        markerDisplay.removeAllMarkers()

    def hideMarkers(self):
        "deletes all the markers in the marker display (i.e. done editing)."
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.removeAllMarkers()
        self.showMarkers = 0

    def addMarker(self, x, y, z, name):
        "Adds a marker to this game!"
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.addMarker(float(x), float(y), float(z), name, PtGetAgeInfo().getAgeFilename())
        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.addMarker():\tSending Add Marker Request")

    def deleteSelectedMarker(self):
        "Deletes the selected marker only if we're in edit show marker mode"
        if not self.showMarkers or self.selectedMarker < 0:
            PtDebugPrint("ERROR: xMarkerGameKIDisplay.deleteSelectedMarker():\tCould not delete the marker as there's no marker selected or we're not editing markers.")
            return

        #Get the id of the selected marker....
        markerID = self.gameData.data['markers'][self.selectedMarker].data['id']
        if type(markerID) == type(None) or markerID < 0:
            PtDebugPrint("ERROR: xMarkerGameKIDisplay.deleteSelectedMarker():\tCould not find the marker to delete!")
            return

        PtDebugPrint("DEBUG: xMarkerGameKIGameDisplay.deleteSelectedMarker():\tdeleting markerID: %s, with type: %s" %(markerID, type(markerID)))
        #Finally delete the marker...
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.deleteMarker(markerID)

    def setSelectedMarker(self, markerNum):
        "sets a marker as selected, if it exists"
        if not self.showMarkers:
            PtDebugPrint("ERROR: xMarkerGameKIDisplay.setSelectedMarker():\tMarkers are not being edited, cannot select markers!")
            return

        PtDebugPrint("DEBUG: xMarkerGameKIDisplay.setSelectedMarker():\tTrying to display markerNum: %s" %(markerNum))
        #Reset any displayed markers...        
        mrkrDisplay = ptMarkerMgr()
        if self.selectedMarker > -1:
            mrkrDisplay.clearSelectedMarker()
        
        self.selectedMarker = markerNum       

        if markerNum > -1:
            #get marker ID
            #Note: the ID is different than the markerNum
            markerID = self.gameData.data['markers'][markerNum].data['id']
            mrkrDisplay.setSelectedMarker(markerID)

    def getSelectedMarker(self):
        "returns the marker object, see xMarkerGameUtils.py for further details"
        markerNum = self.selectedMarker
        if markerNum < 0 or markerNum > self.gameData.data['numMarkers']:
            return None
        return self.gameData.data['markers'][markerNum]

    def setNameOfSelectedMarker(self, name):
        "Sets the name of the selected marker"
        marker = self.getSelectedMarker()
        if type(marker) == type(None):
            PtDebugPrint("xMarkerGameKIDisplay.setNameOfSelectedMarker():\tCould not find selected marker aborting marker name change!")
            return
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.changeMarkerName(marker.data['id'], name)


    def getNumMarkers(self):
        "returns the number of markers in the game"
        return self.gameData.data['numMarkers']

    def deleteGame(self):
        "deletes the game"
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.deleteGame()

    #--------------------------------#
    #    Internal-Only Functions     #
    #--------------------------------#
    def SaveGameClientID(self, id):
        "Stores the game client ID"
        #Since messages are asynchronous, we must check to see if we've got our client ID stored if not, we store it!
        #Note: WE CAN NEVER CHANGE THIS INSTANCE OF THE GAME TO A NEW INSTANCE
        # This object should be deleted; otherwise our clientIDs will not match!
        if self.gameData.data['svrGameClientID'] == self.gameData.default['svrGameClientID']:
            self.gameData.data['svrGameClientID'] = id
            PtDebugPrint("DEBUG: xMarkerGameKIDisplay.SaveGameClientID():\tRegistering game client ID: %s" % id)


    def NotifyGameReady(self):
        "If the game has been sucessfully created, send a KI message that the data is ready"
        if self.initialized:
            return

        data = self.gameData.data
        default = self.gameData.default

        if data['svrGameTemplateID']     == default['svrGameTemplateID'] or \
               data['svrGameName']       == default['svrGameName'] or \
               data['svrGameTypeID']     == default['svrGameTypeID'] or \
               data['svrGameClientID']   == default['svrGameClientID']:
                return
        
        if self.newGame:
            PtDebugPrint("DEBUG: xmarkerGameKIDisplay.NotifyGameReady():\tNotifying KI new game is ready!")
            self.KI.IFinishCreateMarkerFolder(data['svrGameName'], data['svrGameTemplateID'])
        else:
            PtDebugPrint("xMarkerGameKIDisplay.NotifyGameReady():\tGame Loaded sucessfully!")
            self.KI.IFinishDisplayCurrentMarkerGame()

        self.initialized = 1


            




