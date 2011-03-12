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
class PtBlueSpiralMsgTypes:
    """(none)"""
    kBlueSpiralGameStarted = 8
    kBlueSpiralClothOrder = 4
    kBlueSpiralSuccessfulHit = 5
    kBlueSpiralGameWon = 6
    kBlueSpiralGameOver = 7

class PtClimbingWallMsgTypes:
    """(none)"""
    kClimbingWallNumBlockersChanged = 4
    kClimbingWallReadyMsg = 5
    kClimbingWallBlockersChanged = 6
    kClimbingWallPlayerEntered = 7
    kClimbingWallSuitMachineLocked = 8
    kClimbingWallGameOver = 9

class PtClimbingWallReadyTypes:
    """(none)"""
    kClimbingWallReadyNumBlockers = 0
    kClimbingWallReadyBlockers = 1

class PtGameCliInviteErrors:
    """(none)"""
    kGameInviteSuccess = 0
    kGameInviteErrNotOwner = 1
    kGameInviteErrAlreadyInvited = 2
    kGameInviteErrAlreadyJoined = 3
    kGameInviteErrGameStarted = 4
    kGameInviteErrGameOver = 5
    kGameInviteErrGameFull = 6
    kGameInviteErrNoJoin = 7

class PtGameCliMsgTypes:
    """(none)"""
    kGameCliPlayerJoinedMsg = 0
    kGameCliPlayerLeftMsg = 1
    kGameCliInviteFailedMsg = 2
    kGameCliOwnerChangeMsg = 3
    kGameCliTTTMsg = 4
    kGameCliHeekMsg = 5
    kGameCliMarkerMsg = 6
    kGameCliBlueSpiralMsg = 7
    kGameCliClimbingWallMsg = 8
    kGameCliVarSyncMsg = 9

class PtGameMgrMsgTypes:
    """(none)"""
    kGameMgrInviteReceivedMsg = 1
    kGameMgrInviteRevokedMsg = 2

class PtHeekCountdownStates:
    """(none)"""
    kHeekCountdownStart = 0
    kHeekCountdownStop = 1
    kHeekCountdownIdle = 2

class PtHeekGameChoice:
    """(none)"""
    kHeekGameChoiceRock = 0
    kHeekGameChoicePaper = 1
    kHeekGameChoiceScissors = 2

class PtHeekGameSeq:
    """(none)"""
    kHeekGameSeqCountdown = 0
    kHeekGameSeqChoiceAnim = 1
    kHeekGameSeqGameWinAnim = 2

class PtHeekLightStates:
    """(none)"""
    kHeekLightOn = 0
    kHeekLightOff = 1
    kHeekLightFlash = 2

class PtHeekMsgTypes:
    """(none)"""
    kHeekPlayGame = 4
    kHeekGoodbye = 5
    kHeekWelcome = 6
    kHeekDrop = 7
    kHeekSetup = 8
    kHeekLightState = 9
    kHeekInterfaceState = 10
    kHeekCountdownState = 11
    kHeekWinLose = 12
    kHeekGameWin = 13
    kHeekPointUpdate = 14

class PtMarkerGameTypes:
    """(none)"""
    kMarkerGameQuest = 0
    kMarkerGameCGZ = 1
    kMarkerGameCapture = 2
    kMarkerGameCaptureAndHold = 3

class PtMarkerMsgTypes:
    """(none)"""
    kMarkerTemplateCreated = 4
    kMarkerTeamAssigned = 5
    kMarkerGameType = 6
    kMarkerGameStarted = 7
    kMarkerGamePaused = 8
    kMarkerGameReset = 9
    kMarkerGameOver = 10
    kMarkerGameNameChanged = 11
    kMarkerTimeLimitChanged = 12
    kMarkerGameDeleted = 13
    kMarkerMarkerAdded = 14
    kMarkerMarkerDeleted = 15
    kMarkerMarkerNameChanged = 16
    kMarkerMarkerCaptured = 17

class PtTTTGameResult:
    """(none)"""
    kTTTGameResultWinner = 0
    kTTTGameResultTied = 1
    kTTTGameResultGave = 2
    kTTTGameResultError = 3

class PtTTTMsgTypes:
    """(none)"""
    kTTTGameStarted = 4
    kTTTGameOver = 5
    kTTTMoveMade = 6

class PtVarSyncMsgTypes:
    """(none)"""
    kVarSyncNumericVarCreated = 8
    kVarSyncStringVarChanged = 4
    kVarSyncNumericVarChanged = 5
    kVarSyncAllVarsSent = 6
    kVarSyncStringVarCreated = 7

