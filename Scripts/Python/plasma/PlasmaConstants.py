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

# NOTE: This stub file was generated automatically from Plasma's Python interface.
# Do not edit this file manually.
# To change any of the docstrings or function signatures,
# edit the corresponding C++ glue code in pfPython.
# If the Python interface has changed, regenerate these stubs
# by running the following call in the in-game Python console:
# >>> __import__("generate_stubs").run()

from __future__ import annotations
from typing import *

class Enum:
    """Enum base class"""
    def __init__(self):
        pass

class EnumValue:
    """A basic enumeration value"""
    def __init__(self):
        pass

class PtAIMsgType:
    kUnknown = 0
    kBrainCreated = 1
    kArrivedAtGoal = 2

class PtAccountUpdateType:
    kCreatePlayer = 1
    kDeletePlayer = 2
    kUpgradePlayer = 3
    kActivePlayer = 4
    kChangePassword = 5

class PtBehaviorTypes:
    kBehaviorTypeStandingJump = 1
    kBehaviorTypeWalkingJump = 2
    kBehaviorTypeRunningJump = 4
    kBehaviorTypeAnyJump = 7
    kBehaviorTypeRunningImpact = 8
    kBehaviorTypeGroundImpact = 16
    kBehaviorTypeAnyImpact = 24
    kBehaviorTypeIdle = 32
    kBehaviorTypeWalk = 64
    kBehaviorTypeRun = 128
    kBehaviorTypeWalkBack = 256
    kBehaviorTypeTurnLeft = 512
    kBehaviorTypeTurnRight = 1024
    kBehaviorTypeSidestepLeft = 2048
    kBehaviorTypeSidestepRight = 4096
    kBehaviorTypeFall = 8192
    kBehaviorTypeMovingTurnLeft = 16384
    kBehaviorTypeMovingTurnRight = 32768
    kBehaviorTypeLinkIn = 65536
    kBehaviorTypeLinkOut = 131072

class PtBookEventTypes:
    kNotifyImageLink = 0
    kNotifyShow = 1
    kNotifyHide = 2
    kNotifyNextPage = 3
    kNotifyPreviousPage = 4
    kNotifyCheckUnchecked = 5
    kNotifyClose = 6

class PtBrainModes:
    kGeneric = 0
    kLadder = 1
    kSit = 2
    kSitOnGround = 3
    kEmote = 4
    kAFK = 5
    kNonGeneric = 6

class PtButtonNotifyTypes:
    kNotifyOnUp = 0
    kNotifyOnDown = 1
    kNotifyOnUpAndDown = 2

class PtCCRPetitionType:
    kGeneralHelp = 0
    kBug = 1
    kFeedback = 2
    kExploit = 3
    kHarass = 4
    kStuck = 5
    kTechnical = 6

class PtConfirmationResult:
    Cancel = 0
    No = 0
    OK = 1
    Quit = 1
    Yes = 1
    Logout = 62

class PtConfirmationType:
    OK = 0
    ConfirmQuit = 1
    ForceQuit = 2
    YesNo = 3

class PtEventType:
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

class PtFontFlags:
    kFontBold = 1
    kFontItalic = 2
    kFontShadowed = 4

class PtGUIMultiLineDirection:
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
    kFixed = 0
    kAccumulative = 1
    kAccumAllowNegative = 2

class PtJustify:
    kLeftJustify = 0
    kCenter = 1
    kRightJustify = 2

class PtLOSObjectType:
    kClickables = 0
    kCameraBlockers = 1
    kCustom = 2
    kShootable = 3

class PtLOSReportType:
    kReportHit = 0
    kReportMiss = 1
    kReportHitOrMiss = 2

class PtLanguage:
    kEnglish = 0
    kFrench = 1
    kGerman = 2
    kSpanish = 3
    kItalian = 4
    kJapanese = 5
    kDutch = 6
    kRussian = 7
    kPolish = 8
    kCzech = 9
    kNumLanguages = 10

class PtMarkerMsgType:
    kMarkerCaptured = 0

class PtMovieEventReason:
    kMovieDone = 0

class PtMultiStageEventType:
    kEnterStage = 1
    kBeginingOfLoop = 2
    kAdvanceNextStage = 3
    kRegressPrevStage = 4

class PtNotificationType:
    kActivator = 0
    kVarNotification = 1
    kNotifySelf = 2
    kResponderFF = 3
    kResponderChangeState = 4

class PtNotifyDataType:
    kFloat = 1
    kKey = 2
    kInt = 3
    kNull = 4

class PtSDLReadWriteOptions:
    kDirtyOnly = 1
    kSkipNotificationInfo = 2
    kBroadcast = 4
    kTimeStampOnRead = 16

class PtSDLVarType:
    kNone = -1
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

class PtScoreRankGroups:
    kIndividual = 0
    kNeighborhood = 1

class PtScoreTimePeriods:
    kOverall = 0
    kYear = 1
    kMonth = 2
    kDay = 3

class PtStatusLogFlags:
    kFilledBackground = 1
    kAppendToLast = 2
    kDontWriteFile = 4
    kDeleteForMe = 8
    kAlignToTop = 16
    kDebugOutput = 32
    kTimestamp = 64
    kStdout = 128
    kTimeInSeconds = 256
    kTimeAsDouble = 512
