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
Module: xMarkerGameManager
Age: global
Author: Tye Hooley
Date: Dec. 2007
Updated:  Quest Games-->May 2007
This is the "new" python handler for Marker Games
"""
from PlasmaTypes import *
from PlasmaKITypes import *

from PlasmaGameConstants import *
from PlasmaGame import *
from xEnum import *

from xMarkerGameUtils import *
import grtzMarkerGames

# Chronicle Entires
kChronMGGameGUID = "MGGameGUID"

# Constants
kCGZGottenColor = 'yellow'
kCGZToGetColor  = 'yellowlt'
kUCGottenColor  = 'purple'
kUCToGetColor   = 'purplelt'



class MarkerGameManager:
    "Houses the logic and interface for Marker Games, intended to be inited by the KI (i.e. xKI)"
    def __init__(self, KI, existingGame=None):
        version = 1
        subVersion = 3

        self.version = "%s.%s" % (version,subVersion)
        PtDebugPrint("__init__: Marker Game Manager version: v.%s" %self.version)


        #Tye: We really don't want to have a handle on the KI, we want to pass messages to keep things compartmentalized...
        #But it's left here just in case there is no other way.  
        #TODO: When finished coding see if this can be removed!
        self.key = KI.key  #We need to register the KI to receive messages, so this is necessary
        PtDebugPrint("__init__: Registering Marker Game Manager with key: %s" %self.key)
        self.isNewGame = 0
        self.pendingReset = 0

        if existingGame != None:
            self.gameData = chronicleMarkerGameData(existingGame.gameData)
        else:
            #Load any existing marker game that is stored in the chronicle...
            self.gameData = chronicleMarkerGameData()
        
        #~self.gameData.printData()

        self.queuedGame = -1
        self.clientUpdatedMarker = 0

        #Now that we've got the player's data loaded, we had better start any previously played games....
        if self.gameData.data['svrGameTemplateID'] != self.gameData.default['svrGameTemplateID']:
            # Start a CGZ game
            if self.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCGZ:
                PtDebugPrint("DEBUG: __init__: Player has existing Marker game, Loading game: %s" %self.gameData.data['svrGameTemplateID'])
                self.createCGZMarkerGame(self.gameData.data['CGZGameNum'])
            # Start all other User-created marker games
            else:
                PtDebugPrint("DEBUG: __init__: Loading User-Created Marker Game: %s" %self.gameData.data['svrGameTemplateID'])
                if existingGame:
                    PtDebugPrint("DEBUG: __init__: Game already loaded, loading markers and starting the game!")
                    #we've already loaded the game, now, after we register the markers, we just start playing!
                    mrkrDisplay = ptMarkerMgr()
                    mrkrDisplay.removeAllMarkers()
                    
                    ageName = unicode(PtGetAgeInfo().getAgeFilename()).lower()
                    for marker in self.gameData.data['markers']:
                        data = marker.data
                        #Only add markers that exist within this age (only a concern for quest games)
                        if data['age'].lower() != ageName:
                            continue
                        if data['captured'] == 0:
                            mrkrDisplay.addMarker(data['x'], data['y'], data['z'], data['id'], data['captured'])

                    self.StartGame()
                else:
                    #Need to load the game's marker states from the server, we'll just re-instanciate a game to intiate the process
                    self.loadExistingUCMarkerGame()
                

    def __del__(self):
        return
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.removeAllMarkers()

    def gameLoaded(self):
        "Lets callers know if a game is loaded/playing."
        #this is implemented as the xMarkerGameManager is loaded regardless if a game is instanciated
        if self.queuedGame > -1:
            return 1
        if self.gameData.data['svrGameTemplateID'] != self.gameData.default['svrGameTemplateID']:
            return 1
        return 0
    
    def loadExistingUCMarkerGame(self):
        "initiates the client load of the user created marker states from the server"
        PtDebugPrint("DEBUG: xMarkerGame.loadUCMarkerGame():\tLoading the User-Created Marker Game: %s"%self.gameData.data['svrGameName'])
        #To be safe, we should delete any existing markers in the marker manager display.  We'll load them later...
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.removeAllMarkers()


        templateID = self.gameData.data['svrGameTemplateID']
        gameType = self.gameData.data['svrGameTypeID']
        gameName = self.gameData.data['svrGameName']

        #Clear out settings, just to be safe--->and store them!
        self.gameData.initDefaultValues()        
        self.gameData.data['svrGameTemplateID'] = templateID
        self.gameData.data['svrGameTypeID'] = gameType
        self.gameData.save()

        PtCreateMarkerGame(self.key, gameType, gameName , 0, templateID)


    def createCGZMarkerGame(self, gameNum):
        "Initiates the CGZ Marker Game creation process"
        PtDebugPrint("DEBUG: xMarkerGame.createCGZMarkerGame():\tDispatching a new CGZ marker game #%s"%gameNum)
        #Get existing game....  We're starting a new game, we better get rid of the old one!
        existingGame = self.gameData.data['svrGameClientID']
        templateID = ""
        if existingGame != self.gameData.default['svrGameClientID']:
            #We've got to load the game again!
            #We must save the templateID as it will not be xmitted again!
            templateID = self.gameData.data['svrGameTemplateID']

        #To be safe, we should delete any existing markers in the marker manager display.  We'll load them later...
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.removeAllMarkers()

        PtDebugPrint("xMarkerGameManager.CreateCGZMarkerGame():\tCreating Marker Game Number: %i" %gameNum)
        if gameNum < 0 or gameNum > len(grtzMarkerGames.mgs):
            PtDebugPrint("xMarkerGameManager.CreateCGZMarkerGame():\tCannot create game: Invalid Game Number: %s" %gameNum)
            return

        #Save some important settings!
        self.gameData.initDefaultValues()
        self.gameData.data['CGZGameNum'] = gameNum  #store this as there's not much hope of us getting it later....
        self.gameData.data['svrGameTemplateID'] = templateID
        self.gameData.save()

        #Create the game! (callback is: registerPlayerJoin)
        PtCreateMarkerGame(self.key, PtMarkerGameTypes.kMarkerGameCGZ, grtzMarkerGames.GetCGZGameName(gameNum), 0, templateID)

    def captureMarker(self,markerID):
        "Informs server of a captured marker event (note: actions take place in the callback from the server: registerMarkerCaptured)"
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        if server == None:
            PtDebugPrint("ERROR: xMarkerGameManager.captureMarker():\tCould not connect game client to mini-game server to capture marker!")
            return
        #~PtDebugPrint("DEBUG: xMarkerGameManager.captureMarker():\tRegistering Marker #%s as Captured" %markerID)
        self.clientUpdatedMarker += 1
        server.captureMarker(markerID)


    def stopCGZGame(self):
        "Call this to stop a CGZ marker game, it automatically checks for win conditiions"
        gameNum = self.gameData.data['CGZGameNum']
        #check for End Game
        finishTime = 0.0
        if self.gameData.data['numMarkers'] == self.gameData.data['numCapturedMarkers'] and self.gameData.data['numMarkers'] > 0:
            PtDebugPrint("DEBUG: xMarkerGameManager.stopCGZGame():\tGame Completed!  ---Checking for new best score---")
            endTime = PtGetDniTime()
            startTime,bestTime = grtzMarkerGames.GetGameTime(grtzMarkerGames.GetCGZGameName(gameNum))
            finishTime = endTime - startTime
 
        #Update Score Display
        PtDebugPrint("DEBUG: xMarkerGameManager.stopCGZGame(): Attempting to Register score: %s for game: %s" %(finishTime, gameNum))
        grtzMarkerGames.UpdateScore(self.gameData.data['CGZGameNum'], 0, finishTime)

        #Delete Game from the server
        try:
            PtDebugPrint("DEBUG: xMarkerGameManager.stopCGZGame():\t----Dispatching stop CGZ Marker Game----")
            server = GetGameClient(self.gameData.data['svrGameClientID'])
            server.deleteGame()
        except:
            #if we could not process previous statements, we had better clean up a little!
            self.gameData.initDefaultValues()
            self.gameData.save()

        #Everything else is done in the registerDeleteGame

    def stopGame(self):
        "stops an existing User-created marker game"
        if not self.gameLoaded():
            PtDebugPrint("ERROR: xMarkerGameManager.stopGame():\tCould not stop the game as it was not loaded!")
            return

        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.pauseGame()

    def resetGame(self):
        "resets a user-created marker game"
        if not self.gameLoaded():
            PtDebugPrint("ERROR: xMarkerGameManager.stopGame():\tCould not reset the game as it was not loaded!")
            return
        #resets only happen when the game is paused, so we'll do that first and then we'll reset once finished
        self.pendingReset = 1

        #Reset the server
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.pauseGame()


    def deleteGame(self):
        "deletes an existing User created marker game"
        if not self.gameLoaded():
            PtDebugPrint("ERROR: xMarkerGameManager.stopGame():\tCould not delete the game as it was not loaded!")
            return
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        server.deleteGame()


    #--------------------------------#
    # Incomming Game Server Messages #
    #--------------------------------#
    def registerPlayerJoin(self, msg):
        "Server sent a register player join message, make sure to update our data structure"
        PtDebugPrint("DEBUG: xMarkerGameManager.registerPlayerJoin():\tReceived Player Joined Message for player: %s" %msg.playerID())
        if msg.playerID() == PtGetLocalClientID():
            #Store the Game Client ID
            self.gameData.data['isPlayerJoined'] = 1
            self.SaveGameClientID(msg.getGameCli().gameID())
            self.gameData.save()
            self.StartGame()


    def registerTemplateCreated(self, msg):
        "Server sent a template created message"
        self.gameData.data['svrGameTemplateID'] = msg.templateID()
        PtDebugPrint("DEBUG: xMarkerGameManager.registerTemplateCreated():\tReceived template created msg for template: %s" %msg.templateID())
        #This message is only sent for brand new games (i.e. old games never send template registrations)
        #Make sure we store this data....
        self.isNewGame = 1
        #Store the Game Client ID
        self.SaveGameClientID(msg.getGameCli().gameID())
        self.gameData.save()
        self.StartGame()


    def registerGameType(self, msg):
        "sever sent a game type message"
        self.gameData.data['svrGameTypeID'] = msg.gameType()
        PtDebugPrint("DEBUG: xMarkerGameManager.registerGameType(): Received Game type Message for game type: %s" %msg.gameType())
        #Store the Game Client ID
        self.SaveGameClientID(msg.getGameCli().gameID())
        self.gameData.save()
        self.StartGame()


    def registerMarkerGameOver(self, msg):
        "NOT IMPLEMENTED"
        #~print "Tye: DEBUG: WE'VE recv'd game over message!!!"
        return
    



    def registerMarker(self, msg):
        "received register marker message from the game server, display the marker if we're in the same age"
        ageName = PtGetAgeInfo().getAgeFilename()

        x = msg.x()
        y = msg.y()
        z = msg.z()
        id = msg.markerId()

        #Create local marker storage as we may need to display the data....
        #Note: Markers are not "saved" in the chronicle as they reside on the mini-game server.
        #      Thus there is no: self.gameData.save() here...
        marker = MarkerData()
        marker.data['x']    = x
        marker.data['y']    = y
        marker.data['z']    = z
        marker.data['id']   = id
        marker.data['name'] = msg.name()
        marker.data['age']  = msg.age()

        self.gameData.data['markers'].append(marker)
        self.gameData.data['numMarkers'] += 1
        PtDebugPrint("DEBUG: xMarkerGameManager():\tRegistered Marker: %s" %marker)

        if msg.age().lower() != unicode(ageName).lower():
            #PtDebugPrint("DEBUG: xMarkerGameManager.registerMarker():\tMarker exists in a different age, bypassing marker manager display update")
            PtDebugPrint("****> Register Marker #%s: (%s,%s,%s), ageName = %s\t\thidden, wrong age" %(id,x,y,z,msg.age()))
            #~print "\t\tCurrent Age: %s" %ageName
            #~print "\t\tMarker's Age: %s" %msg.age()
            return
        PtDebugPrint("DEBUG: xMarkerGameManager():\tRegistered Marker #%s: (%s,%s,%s), ageName = %s" %(id,x,y,z,msg.age()))

        #Update the marker display manager
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.addMarker(x, y, z, id, 0)


    def registerMarkerCaptured(self, msg):
        "received a marker captured message from the game server, update our marker count"
        markerID = msg.markerId()
        mrkrDisplay = ptMarkerMgr()

        self.gameData.data['numCapturedMarkers'] += 1
        self.gameData.save()

        output = ""
        if self.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCGZ:
            gameNum = grtzMarkerGames.GetCurrentCGZGame()

            if gameNum >=0 and gameNum < len(grtzMarkerGames.mgs):
                try:
                    if self.clientUpdatedMarker:
                        self.clientUpdatedMarker -= 1
                        markers = grtzMarkerGames.mgs[gameNum][0][4]
                        output =  "Found marker: '%s'" % markers[markerID][0]
                        PtSendKIMessage(kKILocalChatStatusMsg, output)
                    #Hide the markers...
                    mrkrDisplay.captureQuestMarker(markerID,1)
                    #End game check is in the Marker Scope GUI
                except:
                    PtDebugPrint("ERROR: xMarkerGameManager.captureMarker():\tCould not get data for CGZ marker captured message!")
        else:
            mrkrDisplay.captureQuestMarker(markerID,1)
            for marker in self.gameData.data['markers']:
                marker = marker.data
                if marker['id'] == markerID:
                    marker['captured'] = 1
                    if self.clientUpdatedMarker:
                        self.clientUpdatedMarker -= 1
                        output = "Found marker: '%s'" %marker['name']
                        PtSendKIMessage(kKILocalChatStatusMsg, output)
                        #check for completion of game...
                        if self.gameData.data['numCapturedMarkers'] >= self.gameData.data['numMarkers']:
                            PtSendKIMessage(kKILocalChatStatusMsg, PtGetLocalizedString("KI.MarkerGame.FinishedGame", [self.gameData.data['svrGameName']]))
                    #TODO: eventually this will need to be different for Hold games as they change colors...
                    #Quest game: Hide the marker....
                    mrkrDisplay.captureQuestMarker(markerID,1)                        
                    break

        #Setup the KI display
        self.UpdateKIMarkerDisplay()


    def registerDeleteGame(self, msg):
        "received a delete game message from the server; make sure to update our internal data"
        PtDebugPrint("DEBUG: xMarkerGameManager.registerDeleteGame():\t---Received Delete Game message, resetting game data---")
        #Reset game data
        self.gameData.initDefaultValues()
        self.gameData.save()

        # Reset KI's Marker Display
        self.UpdateKIMarkerDisplay()

        # Delete all markers from the Marker Manager Display
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.removeAllMarkers()
        self.isNewGame = 0

        #Start any queued game
        if self.queuedGame > -1:
            PtDebugPrint("DEBUG: xMarkerGameManager.registerDeleteGame():\t---Starting Queued game: %s---"%self.queuedGame)
            #TODO: setup for user created marker games...
            self.createCGZMarkerGame(self.queuedGame)
            self.queuedGame = -1

    def registerGameName(self, msg):
        "Saves the game name as dictated by the server"
        #save the name
        self.gameData.data['svrGameName'] = msg.name()
        self.gameData.save()
        PtDebugPrint("DEBUG: xMarkerGameManager.registerMarkerGameName():\tChange game name to: %s" % msg.name())

        #Store the Game Client ID
        self.SaveGameClientID(msg.getGameCli().gameID())
        self.gameData.save()
        self.StartGame()      

    def registerPauseGame(self, msg):
        "pauses the current game"
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        #We could be trying to reset the game
        if self.pendingReset:
            server.resetGame()
            self.pendingReset = 0
        else:
            #We delete the game as we already have the game loaded in the display manager...
            self.registerResetGame("")


    def registerResetGame(self, msg):
        "Resets all the progress in a game. The server doesn't send any state changes, we have to do it manually here!"
        #reset game state
        #if we could not process previous statements, we had better clean up a little!
        self.gameData.initDefaultValues()
        self.gameData.save()

        # Delete all markers from the Marker Manager Display
        mrkrDisplay = ptMarkerMgr()
        mrkrDisplay.removeAllMarkers()

        #Setup the KI display
        self.UpdateKIMarkerDisplay()

        PtDebugPrint("DEBUG: xMarkerGameManager.registerResetGame():\tResetting game's current progress...")
        
        

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
            PtDebugPrint("DEBUG: xMarkerGameManager.SaveGameClientID():\tRegistering game client ID: %s" % id)

    
    def PlayCGZMarkerGame(self, gameNum):
        "Prepares and starts a CGZ Marker Game"
        PtDebugPrint("DEBUG: xMarkerGameManager.PlayCGZMarkerGame():\tServer finished creating CGZ marker game, adding markers and starting the game")
        #Save Important Marker Data
        markers = grtzMarkerGames.mgs[gameNum][0][4]
        self.gameData.data['numMarkers'] = len(markers)
        self.gameData.data['numCapturedMarkers'] = 0
        self.gameData.save()

        #Add Markers....
        PtDebugPrint("DEBUG: xMarkerGameManager.PlayCGZMarkerGame():\tAdding Markers for game #%s" %gameNum)
        server = GetGameClient(self.gameData.data['svrGameClientID'])
        if self.isNewGame == 1:
            index = 0
            for marker in markers:
                server.addMarker(float(marker[1]), float(marker[2]), float(marker[3]), marker[0], marker[4])
                PtDebugPrint("-----| Creating Marker #%s: (%s,%s,%s) ageName = %s" % (index, marker[1], marker[2], marker[3], marker[4]))
                index += 1


        PtDebugPrint("DEBUG: xMarkerGameManager.PlayCGZMarkerGame():\tSending CGZ Start Game Message to the server")
        #Start the game on the server
        server.startGame()

        #Set the KI's marker display
        self.UpdateKIMarkerDisplay()


    def StartGame(self):
        "A registration message was received; check to see if we have enough info to start the marker game"
        if self.gameData.data['isPlayerJoined'] and self.gameData.data['svrGameTemplateID'] != self.gameData.default['svrGameTemplateID']:
            #This only works for CGZ games right now....
            if self.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCGZ:
                #Make sure that we can start the game (i.e. all necessary messages have arrived)
                #Make sure that the CGZ game is valid....
                if self.gameData.data['CGZGameNum'] < 0 and self.gameData.data['CGZGameNum'] > len(grtzMarkerGames.mgs):
                    PtDebugPrint("ERROR: xMarkerGameManager.registerPlayerJoin():\tCannot register player to an invalid CGZ game number: %s" %self.gameData.data['CGZGameNum'])
                    return
                #Now we really can start the game!!!
                PtDebugPrint("DEBUG: xMarkerGameManager.StartGame():\t----STARTING CGZ GAME----")
                self.PlayCGZMarkerGame(self.gameData.data['CGZGameNum'])
                return
            else:
                #all other user-created games
                server = GetGameClient(self.gameData.data['svrGameClientID'])
                PtDebugPrint("DEBUG: xMarkerGameManager.StartGame():\tSending Start Game Message to the server")
                #Start the game on the server
                server.startGame()

                self.UpdateKIMarkerDisplay()
                

    def UpdateKIMarkerDisplay(self):
        "Updates the KI's Marker Display with the game's current progress"
        numMarkers = self.gameData.data['numMarkers']
        numCapturedMarkers = self.gameData.data['numCapturedMarkers']
        
        # -1 is our default, if either one is set to default, assuming the value is 0 for the purposes of display
        if numMarkers < 0:
            numMarkers = 0
        if numCapturedMarkers < 0:
            numCapturedMarkers = 0


        if self.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCGZ:
            gottenColor = kCGZGottenColor
            toGetColor  = kCGZToGetColor
        else:
            gottenColor = kUCGottenColor
            toGetColor  = kUCToGetColor
        
        #   Format = GZGame GottenColor:ToGetColor GottenNum:ToGetNum
        #       Note: GZGame is NOT the CGZ Game Num, so we'll send -1 so the KI will ignore it
        msg = "%d %s:%s %d:%d" % (-1, gottenColor, toGetColor, numCapturedMarkers, numMarkers)
        #print "reported captured msg: ", msg
        PtSendKIMessage(kGZFlashUpdate, msg)


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
    

    def OnBackdoorMsg(self, target, param):
        if target.lower() == "cgz":
            if param.lower() == "solve":
                print "xMarkerGameManager.OnBackdoorMsg(cgz, solve):\t*@#[ CHEAT ACTIVATED! ]#@*"
                print "xMarkerGameManager.solveCGZGame(cgz, solve):\t***[ Sending capture messages to all markers ]***"
                
                numMarkers = self.gameData.data['numMarkers']
                for i in range(numMarkers):
                    self.captureMarker(i)



