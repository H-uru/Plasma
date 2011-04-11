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
def PtCreateMarkerGame(callbackKey, gameType, gameName = "", timeLimit = 0, templateId = ""):
    """Creates a new Marker game with the specified callback key, game type (from PtMarkerGameTypes), time limit (in ms), and template id (guid string)"""
    pass

def PtCreateTTTGame(callbackKey, numPlayers):
    """Creates a new TicTacToe game with the specified callback key and number of players (1 or 2)"""
    pass

def PtGetGameCli(gameID):
    """Returns a ptGameCli associated with the specified id"""
    pass

def PtGetGameIDs():
    """Returns a list of game IDs that the player is currently joined to"""
    pass

def PtGetGameNameByTypeID(guid):
    """Returns the name of the game represented by guid passed in as a string"""
    pass

def PtIsBlueSpiralGame(typeID):
    """Returns true if the specifed typeID (guid as a string) is a BlueSpiral game"""
    pass

def PtIsClimbingWallGame(typeID):
    """Returns true if the specifed typeID (guid as a string) is a ClimbingWall game"""
    pass

def PtIsHeekGame(typeID):
    """Returns true if the specifed typeID (guid as a string) is a Heek game"""
    pass

def PtIsMarkerGame(typeID):
    """Returns true if the specifed typeID (guid as a string) is a Marker game"""
    pass

def PtIsTTTGame(typeID):
    """Returns true if the specifed typeID (guid as a string) is a TicTacToe game"""
    pass

def PtIsVarSyncGame(typeID):
    """Returns true if the specifed typeID (guid as a string) is a VarSync game"""
    pass

def PtJoinCommonBlueSpiralGame(callbackKey, gameID):
    """Joins a common BlueSpiral game with the specified ID. If one doesn't exist, it creates it"""
    pass

def PtJoinCommonClimbingWallGame(callbackKey, gameID):
    """Joins a common ClimbingWall game with the specified ID. If one doesn't exist, it creates it"""
    pass

def PtJoinCommonHeekGame(callbackKey, gameID):
    """Joins a common Heek game with the specified ID. If one doesn't exist, it creates it"""
    pass

def PtJoinCommonTTTGame(callbackKey, gameID, numPlayers):
    """Joins a common TicTacToe game with the specified ID. If one doesn't exist, it creates it with the specified number of players"""
    pass

def PtJoinCommonVarSyncGame(callbackKey):
    """Joins the common VarSync game. If one doesn't exist, it creates it"""
    pass

def PtJoinGame(callbackKey, gameID):
    """Sends a join request to the specified game. Messages are sent to the callback key"""
    pass

class ptGameCliMsg:
    """Message from the game server from a game"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptBlueSpiralMsg(ptGameCliMsg):
    """Base class for BlueSpiral game messages"""
    def __init__(self):
        """None"""
        pass

    def getBlueSpiralMsgType(self):
        """Returns the type of the BlueSpiral message (see PtBlueSpiralMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalBlueSpiralMsg(self):
        """Returns this message as the BlueSpiral message it really is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptBlueSpiralClothOrderMsg(ptBlueSpiralMsg):
    """BlueSpiral message received when the game is started and the cloth order is set"""
    def __init__(self):
        """None"""
        pass

    def getBlueSpiralMsgType(self):
        """Returns the type of the BlueSpiral message (see PtBlueSpiralMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def order(self):
        """Returns a list of numbers indicating the correct order to hit the clothes in"""
        pass

    def upcastToFinalBlueSpiralMsg(self):
        """Returns this message as the BlueSpiral message it really is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptGameCli:
    """Base class for all game client interfaces"""
    def __init__(self):
        """None"""
        pass

    def gameID(self):
        """Returns the ID number for this game"""
        pass

    def gameTypeID(self):
        """Returns the game type ID for this game (as a guid string)"""
        pass

    def invitePlayer(self,playerID):
        """Invites the specified player to join the game"""
        pass

    def leaveGame(self):
        """Leaves this game"""
        pass

    def name(self):
        """Returns the name of the game"""
        pass

    def playerCount(self):
        """Returns the current number of players"""
        pass

    def uninvitePlayer(self,playerID):
        """Revokes the invitation for the specified player"""
        pass

    def upcastToBlueSpiralGame(self):
        """Returns this game client as a ptBlueSpiralGame"""
        pass

    def upcastToClimbingWallGame(self):
        """Returns this game client as a ptClimbingWallGame"""
        pass

    def upcastToHeekGame(self):
        """Returns this game client as a ptHeekGame"""
        pass

    def upcastToMarkerGame(self):
        """Returns this game client as a ptMarkerGame"""
        pass

    def upcastToTTTGame(self):
        """Returns this game client as a ptTTTGame"""
        pass

    def upcastToVarSyncGame(self):
        """Returns this game client as a ptVarSyncGame"""
        pass

class ptBlueSpiralGame(ptGameCli):
    """Game client for the BlueSpiral game"""
    def __init__(self):
        """None"""
        pass

    def gameID(self):
        """Returns the ID number for this game"""
        pass

    def gameTypeID(self):
        """Returns the game type ID for this game (as a guid string)"""
        pass

    def hitCloth(self,clothNum):
        """Tells the server you hit the specified cloth"""
        pass

    def invitePlayer(self,playerID):
        """Invites the specified player to join the game"""
        pass

    def leaveGame(self):
        """Leaves this game"""
        pass

    def name(self):
        """Returns the name of the game"""
        pass

    def playerCount(self):
        """Returns the current number of players"""
        pass

    def startGame(self):
        """Starts a new game"""
        pass

    def uninvitePlayer(self,playerID):
        """Revokes the invitation for the specified player"""
        pass

    def upcastToBlueSpiralGame(self):
        """Returns this game client as a ptBlueSpiralGame"""
        pass

    def upcastToClimbingWallGame(self):
        """Returns this game client as a ptClimbingWallGame"""
        pass

    def upcastToHeekGame(self):
        """Returns this game client as a ptHeekGame"""
        pass

    def upcastToMarkerGame(self):
        """Returns this game client as a ptMarkerGame"""
        pass

    def upcastToTTTGame(self):
        """Returns this game client as a ptTTTGame"""
        pass

    def upcastToVarSyncGame(self):
        """Returns this game client as a ptVarSyncGame"""
        pass

class ptBlueSpiralGameOverMsg(ptBlueSpiralMsg):
    """BlueSpiral message received when the timer runs out, someone hits the wrong cloth, or the game is restarted (before a game start msg in that last case)"""
    def __init__(self):
        """None"""
        pass

    def getBlueSpiralMsgType(self):
        """Returns the type of the BlueSpiral message (see PtBlueSpiralMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalBlueSpiralMsg(self):
        """Returns this message as the BlueSpiral message it really is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptBlueSpiralGameStartedMsg(ptBlueSpiralMsg):
    """BlueSpiral message received when someone starts the game (or when you join a game that is running)"""
    def __init__(self):
        """None"""
        pass

    def getBlueSpiralMsgType(self):
        """Returns the type of the BlueSpiral message (see PtBlueSpiralMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def startSpin(self):
        """Returns true if you are supposed to start spinning the door thingy"""
        pass

    def upcastToFinalBlueSpiralMsg(self):
        """Returns this message as the BlueSpiral message it really is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptBlueSpiralGameWonMsg(ptBlueSpiralMsg):
    """BlueSpiral message received when the last cloth is successfully hit"""
    def __init__(self):
        """None"""
        pass

    def getBlueSpiralMsgType(self):
        """Returns the type of the BlueSpiral message (see PtBlueSpiralMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalBlueSpiralMsg(self):
        """Returns this message as the BlueSpiral message it really is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptBlueSpiralSuccessfulHitMsg(ptBlueSpiralMsg):
    """BlueSpiral message received when a cloth is hit in the correct order"""
    def __init__(self):
        """None"""
        pass

    def getBlueSpiralMsgType(self):
        """Returns the type of the BlueSpiral message (see PtBlueSpiralMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalBlueSpiralMsg(self):
        """Returns this message as the BlueSpiral message it really is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptClimbingWallMsg(ptGameCliMsg):
    """Base class for ClimbingWall game messages"""
    def __init__(self):
        """None"""
        pass

    def getClimbingWallMsgType(self):
        """Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalClimbingWallMsg(self):
        """Returns this message as the ClimbingWall msg it is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptClimbingWallBlockersChangedMsg(ptClimbingWallMsg):
    """ClimbingWall message received when the blocker state changes"""
    def __init__(self):
        """None"""
        pass

    def blockersSet(self):
        """Returns an array of blocker indicies denoting which blockers are set"""
        pass

    def getClimbingWallMsgType(self):
        """Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def localOnly(self):
        """Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"""
        pass

    def teamNumber(self):
        """The team that this message is for"""
        pass

    def upcastToFinalClimbingWallMsg(self):
        """Returns this message as the ClimbingWall msg it is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptClimbingWallGame(ptGameCli):
    """Game client for the ClimbingWall game"""
    def __init__(self):
        """None"""
        pass

    def changeBlocker(self,teamNumber, blockerNumber, added):
        """Changes the specified marker's state for the specified team"""
        pass

    def changeNumBlockers(self,amountToAdjust):
        """Adjusts the number of blockers we are playing with"""
        pass

    def finishedGame(self):
        """Tells the server you reached the top of the wall"""
        pass

    def gameID(self):
        """Returns the ID number for this game"""
        pass

    def gameTypeID(self):
        """Returns the game type ID for this game (as a guid string)"""
        pass

    def invitePlayer(self,playerID):
        """Invites the specified player to join the game"""
        pass

    def leaveGame(self):
        """Leaves this game"""
        pass

    def name(self):
        """Returns the name of the game"""
        pass

    def panic(self):
        """Tells the server you are panicking and want your blockers reset"""
        pass

    def playerCount(self):
        """Returns the current number of players"""
        pass

    def playerEntered(self,teamNumber):
        """Tells the server that you are trying to play the game for the specified team"""
        pass

    def ready(self,readyType, teamNumber):
        """Marks the specified team as ready for the specified type (See PtClimbingWallReadyTypes)"""
        pass

    def reset(self):
        """Attempts to reset the game's control panel"""
        pass

    def uninvitePlayer(self,playerID):
        """Revokes the invitation for the specified player"""
        pass

    def upcastToBlueSpiralGame(self):
        """Returns this game client as a ptBlueSpiralGame"""
        pass

    def upcastToClimbingWallGame(self):
        """Returns this game client as a ptClimbingWallGame"""
        pass

    def upcastToHeekGame(self):
        """Returns this game client as a ptHeekGame"""
        pass

    def upcastToMarkerGame(self):
        """Returns this game client as a ptMarkerGame"""
        pass

    def upcastToTTTGame(self):
        """Returns this game client as a ptTTTGame"""
        pass

    def upcastToVarSyncGame(self):
        """Returns this game client as a ptVarSyncGame"""
        pass

class ptClimbingWallGameOverMsg(ptClimbingWallMsg):
    """ClimbingWall message received when the game is over"""
    def __init__(self):
        """None"""
        pass

    def getClimbingWallMsgType(self):
        """Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def localOnly(self):
        """Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"""
        pass

    def team1Blockers(self):
        """Returns an array of blocker indicies denoting which blockers team 1 set"""
        pass

    def team2Blockers(self):
        """Returns an array of blocker indicies denoting which blockers team 2 set"""
        pass

    def teamWon(self):
        """The team that won the game"""
        pass

    def upcastToFinalClimbingWallMsg(self):
        """Returns this message as the ClimbingWall msg it is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptClimbingWallNumBlockersChangedMsg(ptClimbingWallMsg):
    """ClimbingWall message received when the number of blockers is changed"""
    def __init__(self):
        """None"""
        pass

    def getClimbingWallMsgType(self):
        """Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def localOnly(self):
        """Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"""
        pass

    def newBlockerCount(self):
        """Returns the number of blockers this game is current running with"""
        pass

    def upcastToFinalClimbingWallMsg(self):
        """Returns this message as the ClimbingWall msg it is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptClimbingWallPlayerEnteredMsg(ptClimbingWallMsg):
    """ClimbingWall message received when you successfully enter the suit machine"""
    def __init__(self):
        """None"""
        pass

    def getClimbingWallMsgType(self):
        """Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalClimbingWallMsg(self):
        """Returns this message as the ClimbingWall msg it is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptClimbingWallReadyMsg(ptClimbingWallMsg):
    """ClimbingWall message received when the ready state of the teams is changed"""
    def __init__(self):
        """None"""
        pass

    def getClimbingWallMsgType(self):
        """Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def localOnly(self):
        """Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"""
        pass

    def readyType(self):
        """The type of ready message this represents (see PtClimbingWallReadyTypes)"""
        pass

    def team1Ready(self):
        """Whether team 1 is ready or not"""
        pass

    def team2Ready(self):
        """Whether team 2 is ready or not"""
        pass

    def upcastToFinalClimbingWallMsg(self):
        """Returns this message as the ClimbingWall msg it is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptClimbingWallSuitMachineLockedMsg(ptClimbingWallMsg):
    """ClimbingWall message received when the locked state of the suit machines is changed"""
    def __init__(self):
        """None"""
        pass

    def getClimbingWallMsgType(self):
        """Returns the type of the ClimbingWall message (see PtClimbingWallMsgTypes)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def localOnly(self):
        """Returns true if we are only supposed to adjust our stuff locally, and not net-prop it"""
        pass

    def team1MachineLocked(self):
        """Whether team 1's suit machine is locked or not"""
        pass

    def team2MachineLocked(self):
        """Whether team 2's suit machine is locked or not"""
        pass

    def upcastToFinalClimbingWallMsg(self):
        """Returns this message as the ClimbingWall msg it is"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptGameCliInviteFailedMsg(ptGameCliMsg):
    """Game client message when an invite failed message is received"""
    def __init__(self):
        """None"""
        pass

    def error(self):
        """Returns the error value (See PtGameCliInviteErrors)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def inviteeID(self):
        """Returns the invitee's ID number"""
        pass

    def operationID(self):
        """Returns the operation's ID number"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptGameCliOwnerChangeMsg(ptGameCliMsg):
    """Game client message when a owner change message is received"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def ownerID(self):
        """Returns the owner's ID number"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptGameCliPlayerJoinedMsg(ptGameCliMsg):
    """Game client message when a player joined message is received"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def playerID(self):
        """Returns the player's ID number"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptGameCliPlayerLeftMsg(ptGameCliMsg):
    """Game client message when a player left message is received"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def playerID(self):
        """Returns the player's ID number"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptGameMgrMsg:
    """Message from the game manager"""
    def __init__(self):
        """None"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameMgrMsgTypes)"""
        pass

    def upcastToInviteReceivedMsg(self):
        """Returns this message as a ptGameMgrInviteReceivedMsg"""
        pass

    def upcastToInviteRevokedMsg(self):
        """Returns this message as a ptGameMgrInviteRevokedMsg"""
        pass

class ptGameMgrInviteReceivedMsg(ptGameMgrMsg):
    """Game manager message when an invite is received"""
    def __init__(self):
        """None"""
        pass

    def gameTypeID(self):
        """Returns the game type ID (as a guid string)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameMgrMsgTypes)"""
        pass

    def inviterID(self):
        """Returns the inviter's ID number"""
        pass

    def newGameID(self):
        """Returns the new game's ID number"""
        pass

    def upcastToInviteReceivedMsg(self):
        """Returns this message as a ptGameMgrInviteReceivedMsg"""
        pass

    def upcastToInviteRevokedMsg(self):
        """Returns this message as a ptGameMgrInviteRevokedMsg"""
        pass

class ptGameMgrInviteRevokedMsg(ptGameMgrMsg):
    """Game manager message when an invite is received"""
    def __init__(self):
        """None"""
        pass

    def gameTypeID(self):
        """Returns the game type ID (as a guid string)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameMgrMsgTypes)"""
        pass

    def inviterID(self):
        """Returns the inviter's ID number"""
        pass

    def newGameID(self):
        """Returns the new game's ID number"""
        pass

    def upcastToInviteReceivedMsg(self):
        """Returns this message as a ptGameMgrInviteReceivedMsg"""
        pass

    def upcastToInviteRevokedMsg(self):
        """Returns this message as a ptGameMgrInviteRevokedMsg"""
        pass

class ptHeekMsg(ptGameCliMsg):
    """Base class for Heek game messages"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekCountdownStateMsg(ptHeekMsg):
    """Heek message received by game admin when the countdown state needs to change"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def state(self):
        """Returns state the countdown should be switched to (see PtHeekCountdownStates)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekDropMsg(ptHeekMsg):
    """Heek message received when another player's position needs to be reset/modified"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def position(self):
        """Returns player position to cleanup and dump"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekGame(ptGameCli):
    """Game client for the Heek game"""
    def __init__(self):
        """None"""
        pass

    def choose(self,choice):
        """Makes the specified move (see PtHeekGameChoice)"""
        pass

    def gameID(self):
        """Returns the ID number for this game"""
        pass

    def gameTypeID(self):
        """Returns the game type ID for this game (as a guid string)"""
        pass

    def invitePlayer(self,playerID):
        """Invites the specified player to join the game"""
        pass

    def leaveGame(self):
        """Leaves this game (puts us into "observer" mode"""
        pass

    def name(self):
        """Returns the name of the game"""
        pass

    def playGame(self,position, points, name):
        """Requests to start playing the game in the specified position"""
        pass

    def playerCount(self):
        """Returns the current number of players"""
        pass

    def sequenceFinished(self,sequence):
        """Tells the server that the specified animation finished (see PtHeekGameSeq)"""
        pass

    def uninvitePlayer(self,playerID):
        """Revokes the invitation for the specified player"""
        pass

    def upcastToBlueSpiralGame(self):
        """Returns this game client as a ptBlueSpiralGame"""
        pass

    def upcastToClimbingWallGame(self):
        """Returns this game client as a ptClimbingWallGame"""
        pass

    def upcastToHeekGame(self):
        """Returns this game client as a ptHeekGame"""
        pass

    def upcastToMarkerGame(self):
        """Returns this game client as a ptMarkerGame"""
        pass

    def upcastToTTTGame(self):
        """Returns this game client as a ptTTTGame"""
        pass

    def upcastToVarSyncGame(self):
        """Returns this game client as a ptVarSyncGame"""
        pass

class ptHeekGameWinMsg(ptHeekMsg):
    """Heek message received by game admin when a game is won"""
    def __init__(self):
        """None"""
        pass

    def choice(self):
        """Returns the choice that won (see PtHeekGameChoice)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekGoodbyeMsg(ptHeekMsg):
    """Heek message received when the server processes leave request"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekInterfaceStateMsg(ptHeekMsg):
    """Heek message received when your interface buttons need to enable or disable"""
    def __init__(self):
        """None"""
        pass

    def buttonsEnabled(self):
        """Returns whether your buttons should be enabled"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekLightStateMsg(ptHeekMsg):
    """Heek message received when one of your local lights needs to change state"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def lightNum(self):
        """Returns the index of the light this refers to"""
        pass

    def state(self):
        """Returns state the light should be switched to (see PtHeekLightStates)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekPlayGameMsg(ptHeekMsg):
    """Heek message received when the server processes your play game request"""
    def __init__(self):
        """None"""
        pass

    def enableButtons(self):
        """Returns true if we should enable the buttons at the place we sat down"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def isPlaying(self):
        """Returns true if the server accepted the play game request"""
        pass

    def isSinglePlayer(self):
        """Returns true if you are the only player at the table"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekPointUpdateMsg(ptHeekMsg):
    """Heek message received when the number of points you have needs to be changed"""
    def __init__(self):
        """None"""
        pass

    def displayUpdate(self):
        """Returns whether you should display a message to the user"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def points(self):
        """Returns your new amount of points"""
        pass

    def rank(self):
        """Returns your new rank"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekSetupMsg(ptHeekMsg):
    """Heek message for setting up each position's state"""
    def __init__(self):
        """None"""
        pass

    def buttonState(self):
        """Returns whether the buttons are enabled or not"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def lightOn(self):
        """Returns a list of bools representing lights on or off"""
        pass

    def position(self):
        """Returns the position this message is for"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekWelcomeMsg(ptHeekMsg):
    """Heek message received when a new player sits down"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def name(self):
        """Returns the new player's name"""
        pass

    def points(self):
        """Returns the new player's points"""
        pass

    def rank(self):
        """Returns the new player's rank"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptHeekWinLoseMsg(ptHeekMsg):
    """Heek message received when the round is over and you won or lost"""
    def __init__(self):
        """None"""
        pass

    def choice(self):
        """Returns the choice that won or lost (see PtHeekGameChoice)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getHeekMsgType(self):
        """Returns the type of the Heek message (see PtHeekMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalHeekMsg(self):
        """Returns this message as the Heek message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def win(self):
        """Returns true if you won"""
        pass

class ptMarkerGame(ptGameCli):
    """Game client for the Marker game"""
    def __init__(self):
        """None"""
        pass

    def addMarker(self,x, y, z, name = "", age = ""):
        """Adds a marker to the game. Age is ignored in a non-quest game. Won't work if you're not the owner/creator"""
        pass

    def captureMarker(self,markerId):
        """Captures the specified marker"""
        pass

    def changeGameName(self,newName):
        """Changes the name of the game. Won't work if you're not the game owner/creator"""
        pass

    def changeMarkerName(self,markerId, newName):
        """Changes the name of the specified marker. Won't work if you're not the game owner/creator"""
        pass

    def changeTimeLimit(self,newTimeLimit):
        """Changes the time limit on the game (in ms). Won't work if you're not the game owner/creator, or if it's a quest game"""
        pass

    def deleteGame(self):
        """Tells the server to delete the game. Won't work if you're not the game owner/creator"""
        pass

    def deleteMarker(self,markerId):
        """Deletes the specified marker from the game. Won't work if you're not the game owner/creator"""
        pass

    def gameID(self):
        """Returns the ID number for this game"""
        pass

    def gameTypeID(self):
        """Returns the game type ID for this game (as a guid string)"""
        pass

    def invitePlayer(self,playerID):
        """Invites the specified player to join the game"""
        pass

    def leaveGame(self):
        """Leaves this game"""
        pass

    def name(self):
        """Returns the name of the game"""
        pass

    def pauseGame(self):
        """Pauses the game. Won't work on MP games if you're not the owner/creator"""
        pass

    def playerCount(self):
        """Returns the current number of players"""
        pass

    def resetGame(self):
        """Resets the game. Won't work on MP games if you're not the owner/creator"""
        pass

    def startGame(self):
        """Starts the game. Won't work on MP games if you're not the owner/creator"""
        pass

    def uninvitePlayer(self,playerID):
        """Revokes the invitation for the specified player"""
        pass

    def upcastToBlueSpiralGame(self):
        """Returns this game client as a ptBlueSpiralGame"""
        pass

    def upcastToClimbingWallGame(self):
        """Returns this game client as a ptClimbingWallGame"""
        pass

    def upcastToHeekGame(self):
        """Returns this game client as a ptHeekGame"""
        pass

    def upcastToMarkerGame(self):
        """Returns this game client as a ptMarkerGame"""
        pass

    def upcastToTTTGame(self):
        """Returns this game client as a ptTTTGame"""
        pass

    def upcastToVarSyncGame(self):
        """Returns this game client as a ptVarSyncGame"""
        pass

class ptMarkerMsg(ptGameCliMsg):
    """Base class for Marker game messages"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerGameDeletedMsg(ptMarkerMsg):
    """Marker message received when the game is deleted"""
    def __init__(self):
        """None"""
        pass

    def failed(self):
        """Returns whether the delete succeeded or not"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerGameNameChangedMsg(ptMarkerMsg):
    """Marker message received when the game name is changed"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def name(self):
        """Returns the new game name"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerGameOverMsg(ptMarkerMsg):
    """Marker message received when the server determines the game is over (usually via timeout)"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerGamePausedMsg(ptMarkerMsg):
    """Marker message received when the game is paused by the owner"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def timeLeft(self):
        """Returns the amount of time left on the server clock"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerGameResetMsg(ptMarkerMsg):
    """Marker message received when the game is reset by the owner"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerGameStartedMsg(ptMarkerMsg):
    """Marker message received when the game is started by the owner"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerGameTypeMsg(ptMarkerMsg):
    """Marker message received when you are assigned a team number"""
    def __init__(self):
        """None"""
        pass

    def gameType(self):
        """Returns the type of the game you just joined"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerMarkerAddedMsg(ptMarkerMsg):
    """Marker message received when a marker is added to the game"""
    def __init__(self):
        """None"""
        pass

    def age(self):
        """Returns the age the marker was created in"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def markerId(self):
        """Returns the id number of the marker"""
        pass

    def name(self):
        """Returns the name of the marker"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def x(self):
        """Returns x coord of the marker"""
        pass

    def y(self):
        """Returns y coord of the marker"""
        pass

    def z(self):
        """Returns z coord of the marker"""
        pass

class ptMarkerMarkerCapturedMsg(ptMarkerMsg):
    """Marker message received when a marker is captured"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def markerId(self):
        """Returns id of the marker which was captured"""
        pass

    def team(self):
        """Returns the team number of the team that captured it (0 for no team, or a quest game)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerMarkerDeletedMsg(ptMarkerMsg):
    """Marker message received when a marker is deleted"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def markerId(self):
        """Returns id of the marker that was deleted"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerMarkerNameChangedMsg(ptMarkerMsg):
    """Marker message received when the name of a marker is changed"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def markerId(self):
        """Returns id of the marker who's name was changed"""
        pass

    def name(self):
        """Returns the new name"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerTeamAssignedMsg(ptMarkerMsg):
    """Marker message received when you are assigned a team number"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def teamNumber(self):
        """Returns the number of the team you were assigned to"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerTemplateCreatedMsg(ptMarkerMsg):
    """Marker message received when a quest game template is created"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def templateID(self):
        """Returns the ID number of the template that was created"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptMarkerTimeLimitChangedMsg(ptMarkerMsg):
    """Marker message received when the game name is changed"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getMarkerMsgType(self):
        """Returns the type of the Marker message (see PtMarkerMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def timeLimit(self):
        """Returns the new time limit (in ms)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalMarkerMsg(self):
        """Returns this message as the Marker message it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptTTTGame(ptGameCli):
    """Game client for the TicTacToe game"""
    def __init__(self):
        """None"""
        pass

    def gameID(self):
        """Returns the ID number for this game"""
        pass

    def gameTypeID(self):
        """Returns the game type ID for this game (as a guid string)"""
        pass

    def invitePlayer(self,playerID):
        """Invites the specified player to join the game"""
        pass

    def leaveGame(self):
        """Leaves this game"""
        pass

    def makeMove(self,row, col):
        """Makes a move in the specified spot"""
        pass

    def name(self):
        """Returns the name of the game"""
        pass

    def playerCount(self):
        """Returns the current number of players"""
        pass

    def showBoard(self):
        """Prints the current board layout to the console"""
        pass

    def uninvitePlayer(self,playerID):
        """Revokes the invitation for the specified player"""
        pass

    def upcastToBlueSpiralGame(self):
        """Returns this game client as a ptBlueSpiralGame"""
        pass

    def upcastToClimbingWallGame(self):
        """Returns this game client as a ptClimbingWallGame"""
        pass

    def upcastToHeekGame(self):
        """Returns this game client as a ptHeekGame"""
        pass

    def upcastToMarkerGame(self):
        """Returns this game client as a ptMarkerGame"""
        pass

    def upcastToTTTGame(self):
        """Returns this game client as a ptTTTGame"""
        pass

    def upcastToVarSyncGame(self):
        """Returns this game client as a ptVarSyncGame"""
        pass

class ptTTTMsg(ptGameCliMsg):
    """Base class for TicTacToe game messages"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getTTTMsgType(self):
        """Returns the type of the TTT message (see PtTTTMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalTTTMsg(self):
        """Returns this message as the TTT msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptTTTGameOverMsg(ptTTTMsg):
    """TicTacToe message received when the game is over"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getTTTMsgType(self):
        """Returns the type of the TTT message (see PtTTTMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def result(self):
        """Returns the result of the game (see PtTTTGameResult)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalTTTMsg(self):
        """Returns this message as the TTT msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def winnerID(self):
        """Returns the winner's ID"""
        pass

class ptTTTGameStartedMsg(ptTTTMsg):
    """TicTacToe message received when the game is started"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getTTTMsgType(self):
        """Returns the type of the TTT message (see PtTTTMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalTTTMsg(self):
        """Returns this message as the TTT msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def yourTurn(self):
        """Returns true if you are the first player (and therefore it's your turn)"""
        pass

class ptTTTMoveMadeMsg(ptTTTMsg):
    """TicTacToe message received when someone makes a move"""
    def __init__(self):
        """None"""
        pass

    def col(self):
        """Returns the col index of the move (1..3)"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getTTTMsgType(self):
        """Returns the type of the TTT message (see PtTTTMsgTypes)"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def playerID(self):
        """Returns the the ID of the player that just moved"""
        pass

    def row(self):
        """Returns the row index of the move (1..3)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalTTTMsg(self):
        """Returns this message as the TTT msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptVarSyncMsg(ptGameCliMsg):
    """Base class for VarSync game messages"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def getVarSyncMsgType(self):
        """Returns the type of the VarSync message (see PtVarSyncMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalVarSyncMsg(self):
        """Returns this message as the VarSync msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptVarSyncAllVarsSentMsg(ptVarSyncMsg):
    """VarSync message received after the last var is sent to you when you join the game, or request a list of vars"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def getVarSyncMsgType(self):
        """Returns the type of the VarSync message (see PtVarSyncMsgTypes)"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalVarSyncMsg(self):
        """Returns this message as the VarSync msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

class ptVarSyncGame(ptGameCli):
    """Game client for the VarSync game"""
    def __init__(self):
        """None"""
        pass

    def createNumericVar(self,varName, value):
        """Attempts to create a new numeric variable and set it to the specified number (clipped to double)"""
        pass

    def createStringVar(self,varName, value):
        """Attempts to create a new string variable and set it to the specified string (clipped to 255 chars)"""
        pass

    def gameID(self):
        """Returns the ID number for this game"""
        pass

    def gameTypeID(self):
        """Returns the game type ID for this game (as a guid string)"""
        pass

    def invitePlayer(self,playerID):
        """Invites the specified player to join the game"""
        pass

    def leaveGame(self):
        """Leaves this game"""
        pass

    def name(self):
        """Returns the name of the game"""
        pass

    def playerCount(self):
        """Returns the current number of players"""
        pass

    def requestAllVars(self):
        """Requests all the vars the server knows about"""
        pass

    def setNumericVar(self,varID, value):
        """Attempts to set a numeric variable to the specified number (clipped to double)"""
        pass

    def setStringVar(self,varID, value):
        """Attempts to set a string variable to the specified string (clipped to 255 chars)"""
        pass

    def uninvitePlayer(self,playerID):
        """Revokes the invitation for the specified player"""
        pass

    def upcastToBlueSpiralGame(self):
        """Returns this game client as a ptBlueSpiralGame"""
        pass

    def upcastToClimbingWallGame(self):
        """Returns this game client as a ptClimbingWallGame"""
        pass

    def upcastToHeekGame(self):
        """Returns this game client as a ptHeekGame"""
        pass

    def upcastToMarkerGame(self):
        """Returns this game client as a ptMarkerGame"""
        pass

    def upcastToTTTGame(self):
        """Returns this game client as a ptTTTGame"""
        pass

    def upcastToVarSyncGame(self):
        """Returns this game client as a ptVarSyncGame"""
        pass

class ptVarSyncNumericVarChangedMsg(ptVarSyncMsg):
    """VarSync message received when a numeric variable's value changes"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def getVarSyncMsgType(self):
        """Returns the type of the VarSync message (see PtVarSyncMsgTypes)"""
        pass

    def id(self):
        """Returns the id of the var that changed"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalVarSyncMsg(self):
        """Returns this message as the VarSync msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def value(self):
        """Returns the variable's new value"""
        pass

class ptVarSyncNumericVarCreatedMsg(ptVarSyncMsg):
    """VarSync message received when a numeric variable is created and assigned an id"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def getVarSyncMsgType(self):
        """Returns the type of the VarSync message (see PtVarSyncMsgTypes)"""
        pass

    def id(self):
        """Returns the id assigned to this variable"""
        pass

    def name(self):
        """Returns the name of the var that was created"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalVarSyncMsg(self):
        """Returns this message as the VarSync msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def value(self):
        """Returns the variable's new value"""
        pass

class ptVarSyncStringVarChangedMsg(ptVarSyncMsg):
    """VarSync message received when a string variable's value changes"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def getVarSyncMsgType(self):
        """Returns the type of the VarSync message (see PtVarSyncMsgTypes)"""
        pass

    def id(self):
        """Returns the id of the var that changed"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalVarSyncMsg(self):
        """Returns this message as the VarSync msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def value(self):
        """Returns the variable's new value"""
        pass

class ptVarSyncStringVarCreatedMsg(ptVarSyncMsg):
    """VarSync message received when a string variable is created and assigned an id"""
    def __init__(self):
        """None"""
        pass

    def getGameCli(self):
        """Returns the game client associated with this message"""
        pass

    def getType(self):
        """Returns the type of the message (see PtGameCliMsgTypes)"""
        pass

    def getVarSyncMsgType(self):
        """Returns the type of the VarSync message (see PtVarSyncMsgTypes)"""
        pass

    def id(self):
        """Returns the id that was assigned to this variable"""
        pass

    def name(self):
        """Returns the name of the var that was created"""
        pass

    def upcastToFinalGameCliMsg(self):
        """Returns this message as the game client message it is (player joined, player left, invite failed, or owner change)"""
        pass

    def upcastToFinalVarSyncMsg(self):
        """Returns this message as the VarSync msg it is"""
        pass

    def upcastToGameMsg(self):
        """Returns this message as the base class of message for the game it is associated with (ttt, heek, marker, etc)"""
        pass

    def value(self):
        """Returns the variable's new value"""
        pass

