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
class Enum:
    """Enum base class"""
    def __init__(self):
        """None"""
        pass

class EnumValue:
    """A basic enumeration value"""
    def __init__(self):
        """None"""
        pass

class PtAIMsgType:
    """(none)"""
    kUnknown = 0
    kBrainCreated = 1
    kArrivedAtGoal = 2

class PtAccountUpdateType:
    """(none)"""
    kCreatePlayer = 1
    kDeletePlayer = 2
    kUpgradePlayer = 3
    kActivePlayer = 4
    kChangePassword = 5

class PtBehaviorTypes:
    """(none)"""
    kBehaviorTypeFall = 8192
    kBehaviorTypeIdle = 32
    kBehaviorTypeWalkingJump = 2
    kBehaviorTypeSidestepRight = 4096
    kBehaviorTypeRunningJump = 4
    kBehaviorTypeLinkIn = 65536
    kBehaviorTypeAnyJump = 7
    kBehaviorTypeRunningImpact = 8
    kBehaviorTypeWalk = 64
    kBehaviorTypeRun = 128
    kBehaviorTypeTurnRight = 1024
    kBehaviorTypeWalkBack = 256
    kBehaviorTypeMovingTurnLeft = 16384
    kBehaviorTypeGroundImpact = 16
    kBehaviorTypeStandingJump = 1
    kBehaviorTypeTurnLeft = 512
    kBehaviorTypeAnyImpact = 24
    kBehaviorTypeSidestepLeft = 2048
    kBehaviorTypeMovingTurnRight = 32768
    kBehaviorTypeLinkOut = 131072

class PtBookEventTypes:
    """(none)"""
    kNotifyImageLink = 0
    kNotifyShow = 1
    kNotifyHide = 2
    kNotifyNextPage = 3
    kNotifyPreviousPage = 4
    kNotifyCheckUnchecked = 5
    kNotifyClose = 6

class PtBrainModes:
    """(none)"""
    kGeneric = 0
    kLadder = 1
    kSit = 2
    kEmote = 3
    kAFK = 4
    kNonGeneric = 5

class PtButtonNotifyTypes:
    """(none)"""
    kNotifyOnUp = 0
    kNotifyOnDown = 1
    kNotifyOnUpAndDown = 2

class PtCCRPetitionType:
    """(none)"""
    kGeneralHelp = 0
    kBug = 1
    kFeedback = 2
    kExploit = 3
    kHarass = 4
    kStuck = 5
    kTechnical = 6

class PtEventType:
    """(none)"""
    kCollision = 1
    kPicked = 2
    kControlKey = 3
    kVariable = 4
    kFacing = 5
    kContained = 6
    kActivate = 7
    kCallback = 8
    kResponderState = 9
    kMultiStage = 10
    kSpawned = 11
    kClickDrag = 12
    kOfferLinkingBook = 14
    kBook = 15

class PtGUIMultiLineDirection:
    """(none)"""
    kLineStart = 1
    kLineEnd = 2
    kBufferStart = 3
    kBufferEnd = 4
    kOneBack = 5
    kOneForward = 6
    kOneWordBack = 7
    kOneWordForward = 8
    kOneLineUp = 9
    kOneLineDown = 10
    kPageUp = 11
    kPageDown = 12

class PtGameScoreTypes:
    """(none)"""
    kFixed = 0
    kAccumulative = 1
    kAccumAllowNegative = 2

class PtJustify:
    """(none)"""
    kLeftJustify = 0
    kCenter = 1
    kRightJustify = 2

class PtLOSObjectType:
    """(none)"""
    kClickables = 0
    kCameraBlockers = 1
    kCustom = 2
    kShootable = 3

class PtLOSReportType:
    """(none)"""
    kReportHit = 0
    kReportMiss = 1
    kReportHitOrMiss = 2

class PtLanguage:
    """(none)"""
    kEnglish = 0
    kFrench = 1
    kGerman = 2
    kSpanish = 3
    kItalian = 4
    kJapanese = 5
    kNumLanguages = 6

class PtMarkerMsgType:
    """(none)"""
    kMarkerCaptured = 0

class PtMovieEventReason:
    """(none)"""
    kMovieDone = 0

class PtMultiStageEventType:
    """(none)"""
    kEnterStage = 1
    kBeginingOfLoop = 2
    kAdvanceNextStage = 3
    kRegressPrevStage = 4

class PtNotificationType:
    """(none)"""
    kActivator = 0
    kVarNotification = 1
    kNotifySelf = 2
    kResponderFF = 3
    kResponderChangeState = 4

class PtNotifyDataType:
    """(none)"""
    kNumber = 1
    kKey = 2

class PtSDLReadWriteOptions:
    """(none)"""
    kTimeStampOnRead = 16
    kDirtyOnly = 1
    kSkipNotificationInfo = 2
    kBroadcast = 4

class PtSDLVarType:
    """(none)"""
    kInt = 0
    kFloat = 1
    kBool = 2
    kString32 = 3
    kKey = 4
    kStateDescriptor = 5
    kCreatable = 6
    kDouble = 7
    kTime = 8
    kByte = 9
    kShort = 10
    kVector3 = 50
    kPoint3 = 51
    kRGB = 52
    kRGBA = 53
    kQuaternion = 54
    kNone = -1

class PtScoreRankGroups:
    """(none)"""
    kIndividual = 0
    kNeighborhood = 1

class PtScoreTimePeriods:
    """(none)"""
    kOverall = 0
    kYear = 1
    kMonth = 2
    kDay = 3

class PtStatusLogFlags:
    """(none)"""
    kDebugOutput = 32
    kFilledBackground = 1
    kAppendToLast = 2
    kDontWriteFile = 4
    kDeleteForMe = 8
    kTimestamp = 64
    kStdout = 128
    kTimeInSeconds = 256
    kAlignToTop = 16
    kTimeAsDouble = 512

