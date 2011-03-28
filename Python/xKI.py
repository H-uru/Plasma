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
"""Module: xKI
Age: global
Author: Mark DeForest
Date: April 18, 2002
This is the python handler for the entire KI
===> changed to ptVault 10/18/2002
===> the new look (start 01/07/2003)
--- Prologue I phased
===> KI re-initialization after account update
"""

MaxVersionNumber = 58
MinorVersionNumber = 52

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *

import xKIExtChatCommands
import time
import string
import xCensor
import xLinkingBookDefs
import xBookGUIs
import whrandom
import re   #used for CoD Fix
import os   #used for saving pictures locally
import glob #used for saving pictures locally
import math

import xLocTools
import xEnum
from xMarkerGameManager import * #Logic for Marker Games
from xMarkerGameKIDisplay import * #Support to display user-created marker game details within the KI


# personal age SDL helper
from xPsnlVaultSDL import *

kJalakMiniIconBtn = 1200
kJalakRandomBtn = 1201
kJalakExtremeBtn = 1202
kJalakWallToggleBtn = 1203
kJalakColumnsLowBtn = 1204
kJalakColumnsMedBtn = 1205
kJalakColumnsHighBtn = 1206
kJalakRampBtn = 1207
kJalakSphereBtn = 1208
kJalakBigBoxBtn = 1209
kJalakLilBoxBtn = 1210
kJalakRectangleBtn = 1211
kJalakDestroyBtn = 1212
JalakBtnStates = [   str(kJalakRandomBtn),str(kJalakExtremeBtn),str(kJalakWallToggleBtn),str(kJalakColumnsLowBtn),\
                str(kJalakColumnsMedBtn),str(kJalakColumnsHighBtn),str(kJalakRampBtn),str(kJalakSphereBtn),\
                str(kJalakBigBoxBtn),str(kJalakLilBoxBtn),str(kJalakRectangleBtn),str(kJalakDestroyBtn)]

# define the attributes that will be entered in max
KIBlackbar = ptAttribGUIDialog(1,"The Blackbar dialog")
KIMini = ptAttribGUIDialog(2,"The KIMini dialog")
KIYesNo = ptAttribGUIDialog(3,"The KIYesNo dialog")
BigKI = ptAttribGUIDialog(5,"The BIG KI (Mr. BigStuff)")
#==>DEAD# PictureSmall = ptAttribDynamicMap(6, "The Small picture DynamicMap")
NewItemAlert = ptAttribGUIDialog(7,"The new item alert dialog")
#==>DEAD# PictureObject = ptAttribSceneobject(8, "The Small picture sceneobject")
KIListModeDialog = ptAttribGUIDialog(9, "The list mode dialog")
KIPictureExpanded = ptAttribGUIDialog(10, "The Picture expanded dialog")
KIJournalExpanded = ptAttribGUIDialog(11, "The journal entry expanded dialog")
KIOnAnim = ptAttribAnimation(12,"Turn on/off the KI on animation")
KIOnResp = ptAttribResponder(13,"Turn On responder")
KIOffResp = ptAttribResponder(14,"Turn Off responder")
#==>DEAD# JournalIcon = ptAttribMaterial(15,"Journal Icon map")
#==>DEAD# SoundIcon = ptAttribMaterial(16,"Sound Icon map")
KIPlayerExpanded = ptAttribGUIDialog(17, "The player expanded dialog")
KIMicroBlackbar = ptAttribGUIDialog(18, "The micro Blackbar dialog")
KIMicro = ptAttribGUIDialog(19, "The micro KI dialog")
KINanoBlackBar = ptAttribGUIDialog(20, "The nano Blackbar dialog")
KIVolumeExpanded = ptAttribGUIDialog(21, "The volume control dialog")
KIAgeOwnerExpanded = ptAttribGUIDialog(22, "The Age Owner settings dialog")
KIRateIt = ptAttribGUIDialog(23, "The Rate It dialog")
KISettings = ptAttribGUIDialog(24, "The KI settings dialog")
KIMarkerFolderExpanded = ptAttribGUIDialog(27, "The Marker Folder dialog")
KIMarkerFolderPopupMenu = ptAttribGUIPopUpMenu(28, "The MarkerFolder Time Popup Menu")
KIQuestionNote = ptAttribGUIDialog(29, "The Question Note dialog")
KIMarkerTypePopupMenu = ptAttribGUIPopUpMenu(30, "The MarkerFolder Type Popup Menu")
KICreateMarkerGameGUI = ptAttribGUIDialog(31, "The Marker Game Creation GUI")
KIMarkerGameGUIOpen = ptAttribResponder(32, "Marker Game GUI Open Responder")
KIMarkerGameGUIClose = ptAttribResponder(33, "Marker Game GUI Close Responder")
KIJalakMiniIconOn = ptAttribResponder(34, "Jalak KIMini icon 'on/off' resp",['on','off'])
KIJalakGUIDialog = ptAttribGUIDialog(35, "The Jalak GUI dialog")
KIJalakGUIOpen = ptAttribResponder(36, "Jalak GUI 'open' resp")
KIJalakGUIClose = ptAttribResponder(37, "Jalak GUI 'close' resp")
KIJalakBtnLights = ptAttribResponder(38, "Jalak GUI btn lights resp",statelist=JalakBtnStates,netForce=0)


# globals
KIGUIInitialized = 0
IAmAdmin = 0
UserList = []
PlayerInfoName = None
IKIDisabled = 0
IKIHardDisabled = 0
IminiKIWasUp = 0
WaitingForAnimation=0
LastPrivatePlayerID = None
ToReplyToLastPrivatePlayerID = None
CCRConversationInProgress = 0
WeAreTakingAPicture = 0
kImageDirectory = U'KIimages'
kImageFileNameTemplate = U'KIimage'
ChatLogFile = None
ISawTheKIAtleastOnce = 0
IsPlayingLookingAtKIMode = 0
listLightResps = ["respKILightOff","respKILightOn"]
lightOn = 0
lightStop = 0
KILightObjectName = "RTOmniKILight"
AgeName = ""
JalakGUIState = 0
kHideAgesHackList = ["BahroCave","PelletBahroCave","Pellet Cave","LiveBahroCave","LiveBahroCaves"]


#======Phased globals
PhasedKICreateNotes = 1
PhasedKICreateImages = 1
PhasedKIShareYeeshaBook = 1
PhasedKIInterAgeChat = 1
PhasedKINeighborsInDPL = 1
PhasedKIBuddies = 1
PhasedKIPlayMarkerGame = 1  #Disabled until Marker Games are completed
PhasedKICreateMarkerGame = 1 #Disabled until Marker Games are completed
PhasedKISendNotes = 1
PhasedKISendImages = 1
PhasedKISendMarkerGame = 1 #Disabled until Marker Games are completed
PhasedKIShowMarkerGame = 1 #Disabled until Marker Games are completed

#=====timers used
kFadeTimer=1
kBKITODCheck=2
kAlertHideTimer=3
kTakeSnapShot=4
kMarkerGameTimer=5
kDumpLogsTimer=6
kLightStopID=7
kJalakBtnDelayTimer=8

#===== KI limits
kMaxPictures = 15
kMaxNotes = 30
kMaxMarkerFolders = 10
kMaxMarkers = 1000
kMaxRecentPlayerListSize = 50

NumberOfPictures = 0
NumberOfNotes = 0
NumberOfMarkerFolders = 0
NumberOfMarkers = 0

kJournalTextSize = 2048

#=================
# KI level globals
#=================
theKILevel = kNanoKI       # assume that we are starting out as a micro
PrivateChatChannel = 0
theCensorLevel = 0
OnlyGetPMsFromBuddies = 0
OnlyAllowBuddiesOnRequest = 0
gKIMarkerLevel = 0

kContentListScrollSize = 5
gAlreadyCheckedCGZGame = 0


#=================
# Chonicle entries not in PlasmaKITypes
kChronicleFontSize = "PlayerKIFontSize"
kChronicleFontSizeType = 2
kChronicleFadeTime = "PlayerKIFadeTime"
kChronicleFadeTimeType = 2
kChronicleOnlyPMs = "PlayerKIOnlyPMsBuddies"
kChronicleOnlyPMsType = 2
kChronicleBuddiesOnRequest = "PlayerKIBuddiesOnRequest"
kChronicleBuddiesOnRequestType = 2
kChronCGZPlaying = "CGZPlaying"
# ==============
# BlackBar globals
#----Controls
kMiniMaximizeRGID=34
kExitButtonID=4
kPlayerBookCBID=15
kBBCCRButtonID=200
kmicroChatButton=100

kRolloverLeftID = 998
kRolloverRightID = 999

# ==============
# miniKI globals
#----Controls
kChatCaretID=12
kChatEditboxID=5
kChatDisplayArea=70
kFolderPlayerList=30
kPlayerList=31

kminiToggleBtnID=1
kminiPutAwayID=4
kminiTakePicture=60
kminiMuteAll=61
kminiPrivateToggle=62
kminiCreateJournal=63
kminiDragBar=50
kminiChatScrollUp=51
kminiChatScrollDown=52
kminiPlayerListUp=53
kminiPlayerListDown=54
#-
kmini7Indicator1=71
kmini7Indicator2=72
kmini7Indicator3=73
kmini7Indicator4=74
#-
kminiMarkerIndicator01 = 601
kminiMarkerIndicatorLast = 625
gMarkerColors = {
                'off'       : 0.0,\
                'redlt'     : 1.5,\
                'red'       : 3.5,\
                'yellowlt'  : 6.0,\
                'yellow'    : 8.5,\
                'purplelt'  : 11.0,\
                'purple'    : 13.5,\
                'greenlt'   : 16.0,\
                'green'     : 18.5,\
            }

kminiGZDrip = 700
kminiGZActive = 701
kminiGZMarkerGameActive = 702
kminiGZMarkerInRange = 703
kminiMGNewMarker = 1010
kminiMGNewGame = 1011
kminiMGInactive = 1012

#----
kMaxChatSize = 2048
kMaxNumChatItems = 50
kStartNumChatItems = 9
kStartOffScreenLine = 0
kChatBlankLine = "  \n"
kChatSelfMsg=1
kChatBroadcastMsg=2
kChatPrivateMsg=3
kChatAdminBroadcastMsg=4
kChatAdminPrivateMsg=5
kChatPrivateMsgSelf=6
kChatOfferLink=7
kChatSystemMessage=8
kChatInterAge=9
kChatInterAgeSelf=10
kChatCCRMessage=11
kChatCCRMessageSelf=12
kChatCCRMessageFromPlayer=13
#----fading the lists
kFadeNotActive=0
kFadeFullDisp=1
kFadeDoingFade=2
kFadeStopping=3
FadeMode = kFadeNotActive
MiniKIFirstTimeShow = 1
FadeEnableFlag = 1
CurrentFadeTick = 0
#-- fadeTime stuff
TicksOnFull = 30
kFadeTimeMax = 120
kFullTickTime = 1.0
TicksOnFade = 4
kFadeTickTime = 0.2
OriginalForeAlpha = 1.0
OriginalSelectAlpha = 1.0
OriginalminiKICenter = None
LastminiKICenter = None
#-- font size stuff
FontSizeList = [ 7, 8, 10, 12, 14 ]
# ====================================
# BigKI dialog globals
#----globals
PreviousTime = "20:20"
TimeBlinker = 1
gFeather = 0
#
# this a list of playerfolder and players, all one giant list
BKPlayerList = []

BKPlayerSelected = None
PreviouslySelectedPlayer = None
#
#==Folder list
BKJournalFolderDict = {}
BKJournalListOrder = []
BKJournalFolderSelected = 0
BKJournalFolderTopLine = 0
BKPlayerFolderDict = {}
BKPlayerListOrder = []
BKPlayerFolderSelected = 0
BKPlayerFolderTopLine = 0
BKConfigFolderDict = {}
BKConfigListOrder = [ PtGetLocalizedString("KI.Config.Settings") ]
BKConfigDefaultListOrder = [ PtGetLocalizedString("KI.Config.Settings") ]
BKConfigFolderSelected = 0
BKConfigFolderTopLine = 0
BKFolderLineDict = BKJournalFolderDict
BKFolderListOrder = BKJournalListOrder
BKFolderSelected = BKJournalFolderSelected        # index into BKFolderListOrder
BKFolderTopLine = BKJournalFolderTopLine         # index into BKFolderListOrder
BKFolderSelectChanged = 0   # boolean whether the folder selection has changed
BKIncomingFolder = None
#
BKNewItemsInInbox = 0
FolderOfDevices = None

#
#==Content list
BKCurrentContent = None
BKContentList = []
BKContentListTopLine = 0
#

#==Editing
BKInEditMode = 0
BKEditContent = None
BKEditField = -1
BKGettingPlayerID = 0

#-- Right hand side panel mode
kBKListMode = 1
kBKJournalExpanded = 2
kBKPictureExpanded = 3
kBKPlayerExpanded = 4
kBKVolumeExpanded = 5
kBKAgeOwnerExpanded = 6
kBKKIExpanded = 7
kBKMarkerListExpanded = 8
kBKQuestionNote = 9
BKRightSideMode = kBKListMode
#----Controls
kBKToggleMiniID=14
kBKIPutAwayID=4
kBKFolderUpLine=66
kBKFolderDownLine=67
kBKLMUpButton=110
kBKLMDownButton=111
kBKDisabledPeopleButton=300
kBKDisabledGearButton=301
#-----fields
kBKICurAgeNameID=60
kBKICurTimeID=61
kBKIGPS1TextID=62
kBKIGPS2TextID=63
kBKIGPS3TextID=64
kBKPlayerListID=65
kBKRadioModeID=68
kBKPlayerName=200
kBKPlayerID = 201
kBKIPelletDrop=400
kBKNeighborhoodAndID=59
#--LeftSide controls
kBKIIncomingBtn = 70
kBKIFolderLineBtn01 = 71
kBKIFolderLineBtnLast = 78
kBKIToPlayerButton = 80
kBKIToIncomingButton = 81
kBKIToFolderButton01 = 82
kBKIToFolderButtonLast = 88
#---fields
kBKIPlayerLine = 90
kBKIIncomingLine = 91
kBKIFolderLine01 = 92
kBKIFolderLineLast = 98
#--- fields when mouse over long-ified
kLONGBKIIncomingLine = 591
kLONGBKIFolderLine01 = 592
kLONGBKIFolderLineLast = 598
#===RightSide controls===
#--List mode of entries
kBKIListModeCreateBtn = 80
kBKIListModeLineBtn01 = 81
kBKIListModeLineBtnLast = 89
#---fields
kBKILMTitleCreateLine = 101
kBKILMOffsetLine01 = 110
kBKILMOffsetLineLast = 190
kBKILMIconPictureOffset=0
kBKILMTitleOffset=1
kBKILMDateOffset=2
kBKILMFromOffset=3
kBKILMIconJournalOffset=4
kBKILMIconPersonOffset=5
#++++++++++
#--Journal expanded controls
kBKIJRNTitleButton=80
kBKIJRNNoteButton=81
kBKIJRNDeleteButton=82
#---edit areas
kBKIJRNTitleEdit=85
kBKIJRNNoteEdit=86
#---fields
kBKIJRNAgeName=90
kBKIJRNDate=91
kBKIJRNTitle=92
kBKIJRNNote=93
kBKIJRNFrom=94
kBKIJRNSentDate=95
#--------
#++++++++++
#--Picture expanded controls
kBKIPICTitleButton=80
kBKIPICDeleteButton=82
#---edit areas
kBKIPICTitleEdit=85
#---fields
kBKIPICAgeName=90
kBKIPICDate=91
kBKIPICTitle=92
kBKIPICImage=93
kBKIPICFrom=94
kBKIPICSentDate=95
#---constants
kBKIImageStartX = 112
kBKIImageStartY = 212
#--------
#--Player expanded controls
kBKIPLYPlayerIDEditBox=80
kBKIPLYDeleteButton=82
#---fields
kBKIPLYName=90
kBKIPLYID=91
kBKIPLYDetail=92
#--------
#--KI Settings controls
kBKIKIFontSize=80
kBKIKIFadeTime=81
kBKIKIOnlyPM=90
kBKIKIBuddyCheck=91
kBKIKISettingsText=570
kBKIKIFontSizeText=580
kBKIKIFadeTimeText=581
kBKIKIOnlyPMText=590
#--------
#--Volume expanded controls
kBKISoundFXVolSlider=80
xBKIMusicVolSlider=81
xBKIVoiceVolSlider=82
kBKIAmbienceVolSlider=83
kBKIMicLevelSlider=84
kBKIGUIVolSlider=85
#---------
#--Age Owner Settings controls
kBKAgeOwnerTitleTB=90
kBKAgeOwnerTitleBtn=91
kBKAgeOwnerTitleEditbox=92
kBKAgeOwnerStatusTB=93
kBKAgeOwnerMakePublicTB=94
kBKAgeOwnerMakePublicBtn=95
kBKAgeOwnerGUIDTB=96
kBKAgeOwnerDescription=99
kBKAgeOwnerDescriptionTitle = 510

BKAgeOwnerEditDescription = 0

#===more BK edit globals
#--- edit IDs  [ [textbox,button,editbox], ...]
kBKEditIDtextbox=0
kBKEditIDbutton=1
kBKEditIDeditbox=2
BKEditFieldIDs = [ [kBKIJRNTitle,kBKIJRNTitleButton,kBKIJRNTitleEdit],\
                    [kBKIJRNNote,kBKIJRNNoteButton,kBKIJRNNoteEdit],\
                    [kBKIPICTitle,kBKIPICTitleButton,kBKIPICTitleEdit] ]
kBKEditFieldJRNTitle = 0
kBKEditFieldJRNNote = 1
kBKEditFieldPICTitle = 2

#++++++===========+++++++
#------colors
Clear = ptColor(0,0,0,0)
AgenBlueDk = ptColor(0.65,0.6353, 0.745,1.0)
AgenGreenLt = ptColor(0.8745,1.0,0.85,1.0)
AgenGreenDk = ptColor(0.65,0.745,0.6353,1.0)

DniYellow = ptColor(0.851,0.812,0.576,1.0)      # 217,207,147
DniCyan = ptColor(0.576,0.867,0.851,1.0)       # 147,221,217
DniBlue = ptColor(0.780,0.706,0.870,1.0)        # 199,180,222
DniRed = ptColor(1.0,0.216,0.380,1.0)           # 255,055,097
DniGreen = ptColor(0.698,0.878,0.761,1.0)     # 178,224,194
DniGreenDk = ptColor(0.0,0.596,0.211,1.0)       # 0, 152, 54
#DniPurple = ptColor(0.819,0.698,0.878,1.0)          # 209,178,224
DniPurple = ptColor(0.878,0.698,0.819,1.0)          # 224,178,209
DniWhite = ptColor().white()

DniShowRed = ptColor(1.0,0.851,0.874,1.0)        # 255, 217, 223
DniHideBlue = ptColor(0.780,0.706,0.870,0.3)        # 255, 217, 223

DniColorShowBtn = DniShowRed
DniColorGhostBtn = DniHideBlue

ChatMessageColor = DniWhite
ChatHeaderBroadcastColor = DniBlue
ChatHeaderPrivateColor = DniYellow
ChatHeaderCCRColor = DniCyan
ChatHeaderStatusColor = DniBlue
ChatHeaderErrorColor = DniRed
ChatHeaderNeighborsColor = DniPurple
ChatHeaderBuddiesColor = DniGreen

DniSelectableColor = DniGreen
DniSelectedColor = DniWhite
DniStaticColor = DniBlue

# ====================================
# Yes/No dialog globals
#----Controls
kYesNoTextID=12
kYesButtonID=10
kNoButtonID=11
kYesButtonTextID = 60
kNoButtonTextID = 61
kYesNoLogoutButtonID = 62
kYesNoLogoutTextID = 63
#----globals
kYNQuit=0
kYNDelete=1
kYNOfferLink=2
kYNOutside=3
kYNKIFull=4
kYNWanaPlay=5
kYNNoReason=6
YNWhatReason=kYNQuit
YNOutsideSender = None
# =====================================
# PlayerBook dialog globals
#======= book sharing stuff
kNotOffering = 0
kOfferee = 1
kOfferer = 2
# who offered a link for us
OfferLinkFromWho = None
OfferedBookMode = 0
BookOfferer = None
PsnlOfferedAgeInfo = None
#--- global ptBook object
YeeshaBook = None
IsYeeshaBookEnabled = 1
IsEntireYeeshaBookEnabled = 1

# =====================================
# New Item alert dialog globals
#---------
AlertTimerActive = 0
kAlertTimeDefault = 10.0
kMaxBookAlertTime = 20.0
AlertTimeToUse = kAlertTimeDefault
kAlertKIAlert = 60
kAlertBookAlert = 61

# =====================================
# Marker Folder expanded dialog globals
#---------
kMarkerFolderTitleText = 80
kMarkerFolderTitleBtn = 81
kMarkerFolderTitleEB = 82
kMarkerFolderOwner = 84
kMarkerFolderStatus = 85
kMarkerFolderInvitePlayer = 90
kMarkerFolderEditStartGame = 93
kMarkerFolderPlayEndGame = 94
kMarkerFolderInvitePlayerTB = 100
kMarkerFolderEditStartGameTB = 103
kMarkerFolderPlayEndGameTB = 104

kMarkerFolderGameTimeTitleTB = 199
kMarkerFolderGameTimeTB = 200
kMarkerFolderTimePullDownBtn = 201
kMarkerFolderTimeArrow = 202
kMarkerFolderGameTypeTB = 210
kMarkerFolderTypePullDownBtn = 211
kMarkerFolderTypeArrow = 212
kMarkerFolderDeleteBtn = 150

kMarkerFolderToranIcon = 220
kMarkerFolderHSpanIcon = 221
kMarkerFolderVSpanIcon = 222
kMarkerFolderToranTB = 224
kMarkerFolderHSpanTB = 225
kMarkerFolderVSpanTB = 226

kMarkerFolderMarkListbox = 300
kMarkerFolderMarkerListUpBtn = 301
kMarkerFolderMarkerListDownBtn = 302
kMarkerFolderMarkerTextTB = 310
kMarkerFolderMarkerTextBtn = 311
kMarkerFolderMarkerTextEB = 312
#-------------------------
# Create Marker Game Dialog 
#-------------------------
kCreateMarkerGameNameEB = 1000
kMarkerGameType1 = 1001
kMarkerGameType2 = 1002
kMarkerGameType3 = 1003
kCreateMarkerGameSubmitBT = 1004
kCreateMarkerGameCancelBT = 1005
kMarkerGameLabel1 = 1006
kMarkerGameLabel2 = 1007
kMarkerGameLabel3 = 1008
kMarkerGameDefaultColor = ""
kMarkerGameSelectedColor = ""
kSelectedMGType = 0
kMarkerGameStates = {kMarkerGameType1:0,'UNKNOWN':1,kMarkerGameType2:2,kMarkerGameType3:3}
kMGKILevel = 0
#-------------------------
# Upload Pellet Score Button 
#-------------------------
kPelletScoreButton = 1020
kPelletImager = ""

# ptPlayer's of who is being added or in the game
#MarkerPlayerList = []
# marker game state
kMGNotActive = 0
kMGGameCreation = 1
kMGGameOn = 2
MarkerGameState = kMGNotActive
#MarkerGameMaster = 0
WorkingMarkerFolder = None
CurrentPlayingMarkerGame = None
kMFOverview = 1
kMFEditing = 2
kMFEditingMarker = 3
kMFPlaying = 4
MFdialogMode = kMFOverview

MarkerGameTimeID = 0
MarkerJoinRequests = []

# are we playing the GZ game... if so then which one (1 or 2)
gGZPlaying = 0
gGZMarkerInRange = 0
gGZMarkerInRangeRepy = None
# what is the to get color
gMarkerToGetColor = 'off'
gMarkerGottenColor = 'off'
gMarkerToGetNumber = 0
gMarkerGottenNumber = 0

#message waiting system globals:
kMessageWait = xEnum.Enum("createMarker, changeMarkerName")

# =====================================
# Jalak GUI constants
kSphere = "Sphere"
kLilBox = "LilBox"
kBigBox = "BigBox"
kRamp = "Ramp"
kRect = "Rect"
kJalakBtnDelaySeconds = 0.4

# Jalak GUI globals
jlakGUIButtons = []
kJalakPythonComponent = "cPythField"
JalakScript = None

#=====================================
# Dev globals
#---------
# ONLY TURN THIS ON DURING DEVELOPMENT, it should never used in a release!
kInternalDev = 0  # Removes those annoying "Uru has been updated..." admin messages


# =====================================
# Question Note dialog globals
#---------
kQNTitle = 100
kQNMessage = 101
kQNAcceptBtn = 110
kQNAcceptText = 120
kQNDeclineBtn = 111
kQNDeclineText = 121

AmICCR = 0


# default keys that we ignore... like Escape and enter
DefaultKeyIgnoreList = [ 10,27 ]

# ======================================
# localization helpers
kMarkerFolderPopupMenu = [
    (PtGetLocalizedString("KI.MarkerGame.Popup1Min"),60),
    (PtGetLocalizedString("KI.MarkerGame.Popup2Min"),120),
    (PtGetLocalizedString("KI.MarkerGame.Popup5Min"),300),
    (PtGetLocalizedString("KI.MarkerGame.Popup10Min"),600),
]

kOKDialogDict = { # "code": translation
        "":                                                                         PtGetLocalizedString("KI.Errors.EmptyError"),       #01
        "TERMINATED: Server LogOff. Reason: Logged In Elsewhere":                   PtGetLocalizedString("KI.Errors.LoggedInElsewhere"),#02
        "TERMINATED: Server LogOff. Reason: Timed Out":                             PtGetLocalizedString("KI.Errors.TimedOut"),         #03
        "TERMINATED: Server LogOff. Reason: Not Authenticated":                     PtGetLocalizedString("KI.Errors.NotAuthenticated"), #04
        "TERMINATED: Server LogOff. Reason: Kicked Off":                            PtGetLocalizedString("KI.Errors.KickedOff"),        #05
        "TERMINATED: Server LogOff. Reason: Unknown Reason":                        PtGetLocalizedString("KI.Errors.UnknownReason"),    #06
        "TERMINATED: Server LogOff. Reason: CCRs must use a protected lobby":       PtGetLocalizedString("KI.Errors.CCRLobby"),         #07
        "TERMINATED: Server LogOff. Reason: CCRs must have internal client code":   PtGetLocalizedString("KI.Errors.CCRInternal"),      #08
        "TERMINATED: Server LogOff. Reason: UNKNOWN REASON CODE":                   PtGetLocalizedString("KI.Errors.UnknownReason2"),   #09
        "SERVER SILENCE":               PtGetLocalizedString("KI.Errors.ServerSilence"),            #10
        "BAD VERSION":                  PtGetLocalizedString("KI.Errors.OldVersion"),               #11
	    "Player Disabled":              PtGetLocalizedString("KI.Errors.PlayerDisabled"),           #12
	    "CAN'T FIND AGE":               PtGetLocalizedString("KI.Errors.CantFindAge"),              #13
	    "AUTH RESPONSE FAILED":         PtGetLocalizedString("KI.Errors.AuthFailed"),               #14
        "AUTH TIMEOUT":                 PtGetLocalizedString("KI.Errors.AuthTimeout"),              #15
        "SDL Desc Problem":             PtGetLocalizedString("KI.Errors.SDLDescProblem"),           #16
        "Unspecified error":            PtGetLocalizedString("KI.Errors.UnspecifiedError"),         #17
		"Failed to send msg":           PtGetLocalizedString("KI.Errors.FailedToSendMsg"),          #18
		"Authentication timed out":     PtGetLocalizedString("KI.Errors.AuthenticationTimedOut"),   #19
		"Peer timed out":               PtGetLocalizedString("KI.Errors.PeerTimedOut"),             #20
		"Server silence":               PtGetLocalizedString("KI.Errors.ServerSilence2"),           #21
		"Protocol version mismatch":    PtGetLocalizedString("KI.Errors.ProtocolVersionMismatch"),  #22
		"Auth failed":                  PtGetLocalizedString("KI.Errors.AuthFailed2"),              #23
		"Failed to create player":      PtGetLocalizedString("KI.Errors.FailedToCreatePlayer"),     #24
		"Invalid error code":           PtGetLocalizedString("KI.Errors.InvalidErrorCode"),         #25
		"linking banned":               PtGetLocalizedString("KI.Errors.LinkingBanned"),            #26
		"linking restored":             PtGetLocalizedString("KI.Errors.LinkingRestored"),          #27
		"silenced":                     PtGetLocalizedString("KI.Errors.Silenced"),                 #28
		"unsilenced":                   PtGetLocalizedString("KI.Errors.Unsilenced"),               #29
}

kChatPetitionCommands = {
    PtGetLocalizedString("KI.Commands.Petition"):           PtCCRPetitionType.kGeneralHelp,
    PtGetLocalizedString("KI.Commands.PetitionGeneral"):    PtCCRPetitionType.kGeneralHelp,
    PtGetLocalizedString("KI.Commands.PetitionBug"):        PtCCRPetitionType.kBug,
    PtGetLocalizedString("KI.Commands.PetitionFeedback"):   PtCCRPetitionType.kFeedback,
    PtGetLocalizedString("KI.Commands.PetitionExploit"):    PtCCRPetitionType.kExploit,
    PtGetLocalizedString("KI.Commands.PetitionHarass"):     PtCCRPetitionType.kHarass,
    PtGetLocalizedString("KI.Commands.PetitionStuck"):      PtCCRPetitionType.kStuck,
    PtGetLocalizedString("KI.Commands.PetitionTechnical"):  PtCCRPetitionType.kTechnical
}


def CMPplayerOnline(playerA,playerB):
    elPlayerA = playerA.getChild()
    elPlayerB = playerB.getChild()
    if type(elPlayerA) != type(None) and type(elPlayerB) != type(None):
        if elPlayerA.getType() == PtVaultNodeTypes.kPlayerInfoNode and elPlayerB.getType() == PtVaultNodeTypes.kPlayerInfoNode:
            elPlayerA = elPlayerA.upcastToPlayerInfoNode()
            elPlayerB = elPlayerB.upcastToPlayerInfoNode()
            if elPlayerA.playerIsOnline() and elPlayerB.playerIsOnline():
                return cmp(elPlayerA.playerGetName(),elPlayerB.playerGetName())
            if elPlayerA.playerIsOnline():
                return -1
            if elPlayerB.playerIsOnline():
                return 1
            return cmp(elPlayerA.playerGetName(),elPlayerB.playerGetName())
    return 0

def CMPNodeDate(nodeA,nodeB):
    elNodeA = nodeA.getChild()
    elNodeB = nodeB.getChild()
    if type(elNodeA) != type(None) and type(elNodeB) != type(None):
        if elNodeA.getModifyTime() > elNodeB.getModifyTime():
            return -1
        else:
            return 1
    return 0

class xKI(ptModifier):
    "The KI dialog modifier, includes Blackbar, miniKI, YesNo"
    def __init__(self):
        global FolderOfDevices
        global ChatLogFile
        ptModifier.__init__(self)
        self.id = 199
        self.version = MaxVersionNumber
        self.isChatting = 0
        FolderOfDevices = DeviceFolder(PtGetLocalizedString("KI.Folders.Devices"))
        PtDebugPrint("__xKI: Max version %d - minor version %d.a" % (MaxVersionNumber,MinorVersionNumber))
        #
        self.pendingMGmessage = None

        #Message for user create marker game waiting screen...
        self.pendingMGaction = None

        self.autoShout = 0  #The switch whether or not autoshout is is enabled (it's off by default)

        self.ImagerMap = {}
       

    def OnInit(self):
        PtLoadDialog("KIBlackBar",self.key)
        PtLoadDialog("KINanoBlackBar",self.key)
        PtLoadDialog("KIMicroBlackBar",self.key)
        if not PtIsSinglePlayerMode():
            PtLoadDialog("KIMicro",self.key)
            PtLoadDialog("KIMini",self.key)
            PtLoadDialog("KIMain",self.key)
            PtLoadDialog("KIListMode",self.key)
            PtLoadDialog("KIJournalExpanded",self.key)
            PtLoadDialog("KIPictureExpanded",self.key)
            PtLoadDialog("KIPlayerExpanded",self.key)
            PtLoadDialog("KIAgeOwnerSettings",self.key)
            PtLoadDialog("KISettings",self.key)
            PtLoadDialog("KIMarkerFolder",self.key)
            PtLoadDialog("KIMarkerTimeMenu",self.key)
            PtLoadDialog("KIQuestionNote",self.key)
            PtLoadDialog("KIMarkerTypeMenu",self.key)
        PtLoadDialog("KIYesNo",self.key)
        PtLoadDialog("KINewItemAlert",self.key)
        # the options menu has its own handler
        PtLoadDialog("OptionsMenuGUI")
        PtLoadDialog("IntroBahroBgGUI")

        # Preload the options dialog's dialogs
        PtLoadDialog("OptionsMenuGUI")
        PtLoadDialog("KIHelp")
        PtLoadDialog("KIHelpMenu")
        PtLoadDialog("KeyMapDialog")
        PtLoadDialog("GameSettingsDialog")
        PtLoadDialog("CalibrationGUI")
        PtLoadDialog("TrailerPreviewGUI")
        PtLoadDialog("KeyMap2Dialog")
        PtLoadDialog("AdvancedGameSettingsDialog")
        PtLoadDialog("OptionsHelpGUI")

        # Preload the books (except bkBook which is loaded by pfJournalDialog)
        PtLoadDialog("bkNotebook")
        PtLoadDialog("bkBahroRockBook")

        PtLoadDialog("YeeshaPageGUI")
        
        #Load Marker Game GUI 
        PtLoadDialog("KIMiniMarkers",self.key)

        self.markerGameManager = None
        self.markerGameDisplay = None


    def OnFirstUpdate(self):
        "First update, load our dialogs"
        global AmICCR
        global ChatLogFile
                
        # create the dnicoordinate keeper
        self.dnicoords = ptDniCoordinates()
        # to start with we will use randized numbers instead of the real thing
        ChatLogFile = ptStatusLog()
        try:
            AmICCR = ptCCRMgr().getLevel()
            PtDebugPrint("xKI: CCR level set at %d" % (AmICCR),level=kDebugDumpLevel)
        except:
            PtDebugPrint("xKI: error - not CCR ",level=kDebugDumpLevel)
            AmICCR = 0
        
        xBookGUIs.LoadAllBookGUIs()
        
        logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoLogoutTextID))
        logoutText.hide()
        logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesNoLogoutButtonID))
        logoutButton.hide()


    def OnAccountUpdate(self, updateType, result, playerID):
        global KIGUIInitialized
        #
        # Due to the way the KI is started (at load time), it is necessary
        # to re-initialize once the playerID has updated.
        #
        if updateType == PtAccountUpdateType.kActivePlayer and result == 0 and playerID !=0:
            PtDebugPrint("xKI: active player set. clear to re-init KI GUI", level=kDebugDumpLevel)
            KIGUIInitialized = 0
        elif updateType == PtAccountUpdateType.kChangePassword:
            if result == 0:
                self.IDoStatusChatMessage(PtGetLocalizedString("KI.Messages.PasswordChangeSuccess"), netPropagate = 0)
            else:
                self.IDoStatusChatMessage(PtGetLocalizedString("KI.Messages.PasswordChangeFailure"), netPropagate = 0)


    def OnServerInitComplete(self):
        global AgeName
        if self.markerGameDisplay != None:
            self.markerGameDisplay = None

        #Update the marker game manager
        if self.markerGameManager == None:  
            #Game is initialized on account update (so ignore if player is not selected)
            PtDebugPrint("xKI.OnServerInitComplete():\tERROR: Could not find marker manger, re-creating a new one")
            self.markerGameManager = MarkerGameManager(self)
        else:  
            #Loading new age, re-load marker game manager
            ageName = PtGetAgeInfo().getAgeFilename()
            if ageName.lower() != "startup":
                PtDebugPrint("DEBUG: xKI.OnServerInitComplete():\tRe-loading Marker Game Manager!")
                self.markerGameManager = MarkerGameManager(self)

        #Force any open KIs to close... we don't want to leave them open if we log out and are now logging back in!
        if not PtIsSinglePlayerMode():
            self.IminiPutAwayKI()

        self.CheckKILight()

        AgeName = PtGetAgeName()
        print "xKI.OnServerInitComplete(): age = ",AgeName

        #set up Jalak GUI
        if AgeName == "Jalak":
            print "loading Jalak GUI dialog"
            PtLoadDialog("jalakControlPanel",self.key)
            KIJalakMiniIconOn.run(self.key,state="on",netPropagate=0)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).show()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
            self.IAlertKIStart()
        else:
            KIJalakMiniIconOn.run(self.key,state="off",netPropagate=0,fastforward=1)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).hide()

    def CheckKILight(self):
        timeRemaining = self.GetKILightChron()
        if not timeRemaining:
            print "had KI light, but it's currently off"
            self.DoKILight(0,1)
        elif timeRemaining > 0:
            print "have KI light, time remaining = ",timeRemaining
            self.DoKILight(1,1,timeRemaining)
            self.SetKILightChron(0)
        else:
            print "no KI light"


    def GetKILightChron(self):
        vault = ptVault()
        entry = vault.findChronicleEntry("KILightStop")
        if type(entry) != type(None):
            entryValue = entry.chronicleGetValue()
            remaining = string.atoi(entryValue)
            return remaining
        else:
            print "no KI light"
            return -1


    def SetKILightChron(self,remaining):
        vault = ptVault()
        entry = vault.findChronicleEntry("KILightStop")
        if type(entry) != type(None):
            entryValue = entry.chronicleGetValue()
            oldVal = string.atoi(entryValue)
            if remaining == oldVal:
                return
            print "set KI light chron to: ",remaining
            entry.chronicleSetValue("%d" % (remaining))
            entry.save()


    def DoKILight(self,state,ff,remaining=0):
        global lightOn
        global lightStop

        thisResp = listLightResps[state]
        LocalAvatar = PtGetLocalAvatar()
        avatarKey = LocalAvatar.getKey()
        avatarObj = avatarKey.getSceneObject()
        respList = avatarObj.getResponders()
        if len(respList) > 0:
            PtDebugPrint("xKI.DoKILight(): ...responder list:")
            for resp in respList:
                PtDebugPrint("       %s" % (resp.getName()))
                if resp.getName() == thisResp:
                    PtDebugPrint("found KI light resp: %s" % (thisResp))
                    atResp = ptAttribResponder(42)
                    atResp.__setvalue__(resp)
                    atResp.run(self.key,avatar=LocalAvatar,fastforward=ff)
                    if state:
                        PtAtTimeCallback(self.key,remaining,kLightStopID)
                        print "xKI.DoKILight(): light was on in previous age, turning on for remaining ",remaining," seconds"
                        curTime = PtGetDniTime()
                        lightStop = (remaining + curTime)
                        lightOn = 1
                    else:
                        print "xKI.DoKILight(): light is shut off, updating chron"
                        self.SetKILightChron(remaining)
                        lightOn = 0
                        PtSetLightAnimStart(avatarKey, KILightObjectName, false)
                    break
        else:
            PtDebugPrint("dsntKILightMachine.DoKILight():  ERROR! couldn't find any responders")


    def BeginAgeUnLoad(self,avObj):
        ageName = PtGetAgeName()
        if ageName == "Descent":
            return
        elif ageName == "Jalak":
            #ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            #ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).hide()
            if JalakGUIState:
                self.JalakGUIToggle(1)
        if not lightOn:
            return
        
        local = 0
        try:
            local = PtGetLocalAvatar()
        except:
            print"xKI.BeginAgeUnLoad()-->\tfailed to get local avatar"
            return
        if (local == avObj): 
            print "xKI.BeginAgeUnLoad()-->\tavatar page out"
            curTime = PtGetDniTime()
            timeRemaining = (lightStop - curTime)
            if timeRemaining > 0:
                self.DoKILight(0,1,timeRemaining)
                LocalAvatar = PtGetLocalAvatar()
                avatarKey = LocalAvatar.getKey()
                PtSetLightAnimStart(avatarKey, KILightObjectName, false)


    def __del__(self):
        "the destructor - unload any dialogs we loaded"
        PtUnloadDialog("KINanoBlackBar")
        PtUnloadDialog("KIMicroBlackBar")
        if not PtIsSinglePlayerMode():
            PtUnloadDialog("KIMicro")
            PtUnloadDialog("KIMini")
            PtUnloadDialog("KIMain")
            PtUnloadDialog("KIListMode")
            PtUnloadDialog("KIJournalExpanded")
            PtUnloadDialog("KIPictureExpanded")
            PtUnloadDialog("KIPlayerExpanded")
            PtUnloadDialog("KIAgeOwnerSettings")
            PtUnloadDialog("KISettings")
            PtUnloadDialog("KIMarkerFolder")
            PtUnloadDialog("KIMarkerTimeMenu")
            PtUnloadDialog("KIQuestionNote")
            PtUnloadDialog("KIMarkerTypeMenu")
        PtUnloadDialog("KIYesNo")
        PtUnloadDialog("KINewItemAlert")
        PtUnloadDialog("OptionsMenuGUI")
        PtUnloadDialog("IntroBahroBgGUI")
#########
## dialogs that are used
##        PtUnloadDialog("KIVolumeSettings")
##        PtUnloadDialog("KIRateIt")

        PtUnloadDialog("OptionsMenuGUI")
        PtUnloadDialog("KIHelp")
        PtUnloadDialog("KIHelpMenu")
        PtUnloadDialog("KeyMapDialog")
        PtUnloadDialog("GameSettingsDialog")
        PtUnloadDialog("CalibrationGUI")
        PtUnloadDialog("TrailerPreviewGUI")
        PtUnloadDialog("KeyMap2Dialog")
        PtUnloadDialog("AdvancedGameSettingsDialog")
        PtUnloadDialog("OptionsHelpGUI")

        PtUnloadDialog("bkNotebook")
        PtUnloadDialog("bkBahroRockBook")

        PtUnloadDialog("YeeshaPageGUI")

        PtUnloadDialog("KIMiniMarkers")

        if AgeName == "Jalak":
            print "unloading Jalak GUI dialog"
            KIJalakMiniIconOn.run(self.key,state="off",netPropagate=0,fastforward=1)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).hide()
            PtUnloadDialog("jalakControlPanel")


    def ILocalizeQuitNoDialog(self):
        yesButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesButtonTextID))
        noButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kNoButtonTextID))
        yesButton.setStringW(PtGetLocalizedString("KI.YesNoDialog.QuitButton"))
        noButton.setStringW(PtGetLocalizedString("KI.YesNoDialog.NoButton"))

    def ILocalizeYesNoDialog(self):
        yesButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesButtonTextID))
        noButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kNoButtonTextID))
        yesButton.setStringW(PtGetLocalizedString("KI.YesNoDialog.YESButton"))
        noButton.setStringW(PtGetLocalizedString("KI.YesNoDialog.NoButton"))

    def OnDefaultKeyCaught(self,ch,isDown,isRepeat,isShift,isCtrl,keycode):
        "get the left over keystrokes, that no one wanted"
        global DefaultKeyIgnoreList
        global IKIDisabled
        if ord(ch) != 0:
            if not ord(ch) in DefaultKeyIgnoreList:
                if isDown and not isCtrl and not isRepeat:
                    if not IKIDisabled and not PtIsEnterChatModeKeyBound():
                        self.IEnterChatMode(1,firstChar=ch)
            #else:
            #    PtDebugPrint("xKI:DEFAULTKEY not used char of = %d"%(ord(ch)),level=kDebugDumpLevel)
        #else:
            #if isDown:
            #    PtDebugPrint("xKI:DEFAULTKEY not used - keycode = %d"%(keycode),level=kDebugDumpLevel)
    def SelectMarkerType(self, dlgObj, tagID):
        global kSelectedMGType
        
        if kSelectedMGType != tagID and tagID and kSelectedMGType != 0:
            print "KI: Old Marker Game Type ID:", kSelectedMGType
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kSelectedMGType)).enable()
            self.ChangeMarkerTypeColor(dlgObj, kSelectedMGType)

        kSelectedMGType = tagID
        print "KI: Selecting New Game Type:", kSelectedMGType
        ptGUIControlButton(dlgObj.dialog.getControlFromTag(kSelectedMGType)).disable()
        self.ChangeMarkerTypeColor(dlgObj, tagID)

    def ChangeMarkerTypeColor (self, dlgObj, tagID):
        currentColor = ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID+5)).getForeColor()

        if currentColor == kMarkerGameDefaultColor:
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID+5)).setForeColor(kMarkerGameSelectedColor)
        elif currentColor == kMarkerGameSelectedColor:
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID+5)).setForeColor(kMarkerGameDefaultColor)
        
    def initMarkerGameGUI (self, dlgObj):
        global kSelectedMGType
        kSelectedMGType = kMarkerGameType1
        if kMGKILevel == 2:
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType1)).disable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType2)).enable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType2)).show()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel2)).show()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType3)).enable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType3)).show()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel3)).show()
        elif kMGKILevel == 1:
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType1)).disable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType2)).enable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType2)).show()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel2)).show()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType3)).hide()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel3)).hide()
        elif kMGKILevel == 0:
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType1)).disable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType2)).hide()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel2)).hide()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kMarkerGameType3)).hide()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel3)).hide()

        ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel1)).setForeColor(kMarkerGameSelectedColor)
        ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel2)).setForeColor(kMarkerGameDefaultColor)
        ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kMarkerGameLabel3)).setForeColor(kMarkerGameDefaultColor)
       
        playerName = PtGetLocalPlayer().getPlayerName()
        if playerName[-1] == "s":
            addToName = "'"
        else:
            addToName = "'s"
        gameName = str(PtGetLocalPlayer().getPlayerName()) + str(addToName) + " Marker Game"

        ptGUIControlEditBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kCreateMarkerGameNameEB)).setString(gameName)


    def JalakGUIInit(self):
        #print "xKI.JalakGUIInit()"
        global jlakGUIButtons
        global JalakScript

        jlakRandom = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakRandomBtn))
        jlakExtreme = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakExtremeBtn))
        jlakWall = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakWallToggleBtn))
        jlakAllLow = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakColumnsLowBtn))
        jlakAllMed = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakColumnsMedBtn))
        jlakAllHigh = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakColumnsHighBtn))
        jlakRamp = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakRampBtn))
        jlakSphere = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakSphereBtn))
        jlakBigBox = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakBigBoxBtn))
        jlakLilBox = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakLilBoxBtn))
        jlakRect = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakRectangleBtn))
        jlakDestroy = ptGUIControlButton(KIJalakGUIDialog.dialog.getControlFromTag(kJalakDestroyBtn))

        jlakGUIButtons = [  jlakRandom,jlakExtreme,jlakWall,jlakAllLow,jlakAllMed,jlakAllHigh,\
                            jlakRamp,jlakSphere,jlakBigBox,jlakLilBox,jlakRect,jlakDestroy]

        obj = PtFindSceneobject("JalakDONOTTOUCH", "Jalak")
        pythonScripts = obj.getPythonMods()
        for script in pythonScripts:
            if script.getName() == kJalakPythonComponent:
                JalakScript = script
                print "xKI.JalakGUIInit(): found Jalak's python component: ",kJalakPythonComponent
                return
        print "xKI.JalakGUIInit():  ERROR! did NOT find Jalak's python component: ",kJalakPythonComponent


    def JalakGUIToggle(self,ff=0):
        print "xKI.JalakGUIToggle()"
        global JalakGUIState
        ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
        if AgeName != "Jalak":
            JalakGUIState = 0
            return
        if JalakGUIState:
            JalakGUIState = 0
            KIJalakGUIClose.run(self.key,netPropagate=0,fastforward=ff)
            #KIJalakGUIDialog.dialog.hide()
            #KIJalakGUIDialog.dialog.disable()
            if ff:
                PtHideDialog("jalakControlPanel")
            #KIBlackbar.dialog.show()
        else:
            #User cannot be busy doing some other task
            if WeAreTakingAPicture or WaitingForAnimation:
                PtDebugPrint("DEBUG: xKI.JalakGUIToggle():\tAborting request for Jalak GUI: user is busy!")
                return
            # only those that have garrison KI can create game
            if theKILevel <= kMicroKI or IKIDisabled:
                PtDebugPrint("DEBUG: xKI.JalakGUIToggle():\tAborting request for Jalak GUI: user does not have the KI!")
                return
            JalakGUIState = 1
            #KIJalakGUIDialog.dialog.show()
            PtShowDialog("jalakControlPanel")
            #KIJalakGUIDialog.dialog.enable()
            KIJalakGUIOpen.run(self.key,netPropagate=0)
            #KIBlackbar.dialog.hide()


    #def JalakGUIActive(self):
        #pass
        #print "xKI.JalakGUIActive()"


    def OnNotify(self,state,id,events):
        global WaitingForAnimation
        global OfferedBookMode
        global BookOfferer
        global YeeshaBook
        global IsYeeshaBookEnabled
        PtDebugPrint("xKI: Notify  state=%f, id=%d" % (state,id),level=kDebugDumpLevel)
        # is it a notification from the scene input interface or PlayerBook?
        for event in events:
            if event[0] == kOfferLinkingBook: # this can come locally (to re-draw the panel after someone has accepted or rejected it) or...
                PtDebugPrint("got offer book notification",level=kDebugDumpLevel)
                if theKILevel == kMicroKI or PtIsSinglePlayerMode():
                    plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                else:
                    plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                # make sure we get the events back is someone else used the PlayerBook
                if (event[2] == -999): # if the offerer is recinding the book offer, hide the panel, or...
                    # if waiting for offeree to accept
                    if OfferedBookMode == kOfferee:
                        # then take done the offered playerbook
                        YeeshaBook.hide()
                        PtToggleAvatarClickability(true)
                        plybkCB.setChecked(0)
                    # else -     otherwise they were too late.
                    OfferedBookMode = kNotOffering
                    BookOfferer = None
                    PtDebugPrint("Offerer is rescinding the book offer",level=kDebugDumpLevel)
                    PtToggleAvatarClickability(true)
                    return
                elif (event[2] == 999): # the book is being offered by someone else
                    BookOfferer = event[1]
                    avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                    if ptVault().getIgnoreListFolder().playerlistHasPlayer(avID):
                        OfferedBookMode = kNotOffering
                        PtNotifyOffererLinkRejected(avID) 
                        BookOfferer = None
                        PtToggleAvatarClickability(true)
                        return
                    else:
                        OfferedBookMode = kOfferee
                        PtDebugPrint("offered book by ",BookOfferer.getName(),level=kDebugDumpLevel)
                        self.IShowYeeshaBook()
                        PtToggleAvatarClickability(false)
                        return
                        
            # is it from the YeeshaBook? (we only have one book to worry about)
            elif event[0] == PtEventType.kBook:
                PtDebugPrint("xKI: BookNotify  event=%d, id=%d" % (event[1],event[2]),level=kDebugDumpLevel)
                if theKILevel == kMicroKI or PtIsSinglePlayerMode():
                    plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                else:
                    plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                if event[1] == PtBookEventTypes.kNotifyImageLink:
                    if event[2] == xLinkingBookDefs.kYeeshaBookShareID:
                        if IsYeeshaBookEnabled:
                            # need to rescind any offer if we are trying to offer someone else
                            PtClearOfferBookMode()
                            if (OfferedBookMode == kNotOffering):
                                # take done the playerbook
                                YeeshaBook.hide()
                                PtToggleAvatarClickability(true)
                                plybkCB.setChecked(0)
                                # prepare to choose who to offer the link to
                                PtSetOfferBookMode(self.key, "Personal", "Relto")
                                # just use the default link in point for others (not into the hut!)
                    elif event[2] == xLinkingBookDefs.kYeeshaBookLinkID:
                        if IsYeeshaBookEnabled:
                            PtDebugPrint("xKI:Book: hit linking panel",level=kDebugDumpLevel)
                            YeeshaBook.hide()
                            plybkCB.setChecked(0)
                            if (OfferedBookMode == kOfferee):
                                PtDebugPrint("accepted link, notifying offerer of such",level=kDebugDumpLevel)
                                OfferedBookMode = kNotOffering
                                avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                                PtNotifyOffererLinkAccepted(avID)
                                PtNotifyOffererLinkCompleted(avID)
                                BookOfferer = None
                            else:
                                # last chance check to see if they are on a ladder or something
                                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                                if IsEntireYeeshaBookEnabled and ( curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit ):
                                    # we're linking to .... our (err... the players) personal age!
                                    linkmgr = ptNetLinkingMgr()
                                    linkmgr.linkToMyPersonalAgeWithYeeshaBook()
                                    PtToggleAvatarClickability(true)
                                    # bye-bye
                    elif event[2] >= xLinkingBookDefs.kYeeshaPageStartID:
                        whatpage = event[2] - xLinkingBookDefs.kYeeshaPageStartID
                        sdlvar = xLinkingBookDefs.xYeeshaPages[whatpage][0]
                        self.IToggleYeeshaPageSDL(sdlvar,1)
                elif event[1] == PtBookEventTypes.kNotifyShow:
                    pass
                elif event[1] == PtBookEventTypes.kNotifyHide:
                    PtDebugPrint("xKI:Book: NotifyHide",level=kDebugDumpLevel)
                    PtToggleAvatarClickability(true)
                    plybkCB.setChecked(0)
                    if(OfferedBookMode == kOfferee):
                        PtDebugPrint("rejecting link, notifying offerer of such",level=kDebugDumpLevel)
                        OfferedBookMode = kNotOffering
                        avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                        PtNotifyOffererLinkRejected(avID) 
                        BookOfferer = None
                elif event[1] == PtBookEventTypes.kNotifyNextPage:
                    pass
                elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                    pass
                elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                    if event[2] >= xLinkingBookDefs.kYeeshaPageStartID:
                        whatpage = event[2] - xLinkingBookDefs.kYeeshaPageStartID
                        sdlvar = xLinkingBookDefs.xYeeshaPages[whatpage][0]
                        self.IToggleYeeshaPageSDL(sdlvar,0)
                # nothing more to do, when there is a book event
                return
        #~ PtDebugPrint("xKI: Notify  state=%f, id=%d" % (state,id))
        if state:
            # is it one of the responders that are displaying the bigKI
            if id == KIOnResp.id:
                self.IBigKIShowMode()
                WaitingForAnimation = 0
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
                toggleCB.enable()
            elif id == KIOffResp.id:
                BigKI.dialog.hide()
                WaitingForAnimation = 0
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
                toggleCB.enable()
        if id == KIMarkerGameGUIClose.id:
            PtHideDialog("KIMiniMarkers")
        if id == KIJalakGUIClose.id:
            #print "callback from Jalak GUI close"
            PtHideDialog("jalakControlPanel")
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
            #KIJalakGUIDialog.dialog.hide()
            #KIJalakGUIDialog.dialog.disable()
        elif id == KIJalakGUIOpen.id:
            #print "callback from Jalak GUI open"
            KIJalakGUIDialog.dialog.enable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
            #self.JalakGUIActive()
        elif id == KIJalakBtnLights.id:
            btnID = string.atoi(KIJalakBtnLights.getState())
            self.SendJalakBtnHit(btnID)
            PtAtTimeCallback(self.key,kJalakBtnDelaySeconds,kJalakBtnDelayTimer)


    def OnPageLoad(self,what,room):
        "delay showing stuff until something is paged in"
        global KIGUIInitialized
        global FolderOfDevices
        global BKPlayerSelected
        global gGZMarkerInRange
        global gGZMarkerInRangeRepy
        global kPelletImager
        
        if not KIGUIInitialized:
            KIGUIInitialized = 1
            self.ISetupKI()

        # when we upload the page, then the device we have must be gone
        if what == kUnloaded:
            FolderOfDevices.removeAll()
            # remove any selected player... sorry, they'll have to re-select 'em.
            if theKILevel == kNormalKI and not PtIsSinglePlayerMode():
                BKPlayerSelected = None
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                sendToField.setString(" ")
            if gGZMarkerInRange:
                gGZMarkerInRange = 0
                gGZMarkerInRangeRepy = None
                self.IRefreshMiniKIMarkerDisplay()
                NewItemAlert.dialog.hide()
                kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                kialert.hide()
            if kPelletImager != "":
                kPelletImager = ""
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kPelletScoreButton)).hide()

    def ISetupKI(self):
        "All logic necessary to setup the KI for a given player"
        global KIGUIInitialized
        global IsYeeshaBookEnabled
        global IsEntireYeeshaBookEnabled
        global gGZMarkerInRange
        global gGZMarkerInRangeRepy
        global BKPlayerList
        global BKPlayerSelected
        global PreviouslySelectedPlayer
        global BKJournalFolderDict
        global BKJournalListOrder
        global BKJournalFolderSelected
        global BKJournalFolderTopLine
        global BKPlayerFolderDict
        global BKPlayerListOrder
        global BKPlayerFolderSelected
        global BKPlayerFolderTopLine
        global BKConfigFolderDict
        global BKConfigFolderSelected
        global BKConfigFolderTopLine
        global BKFolderLineDict
        global BKFolderListOrder
        global BKFolderSelected
        global BKFolderTopLine
        global BKFolderSelectChanged
        global BKIncomingFolder
        global BKNewItemsInInbox
        global BKCurrentContent
        global BKContentList
        global BKContentListTopLine

        BKPlayerList = []
        BKPlayerSelected = None
        PreviouslySelectedPlayer = None

        BKJournalFolderDict = {}
        BKJournalListOrder = []
        BKJournalFolderSelected = 0
        BKJournalFolderTopLine = 0
        BKPlayerFolderDict = {}
        BKPlayerListOrder = []
        BKPlayerFolderSelected = 0
        BKPlayerFolderTopLine = 0
        BKConfigFolderDict = {}
        BKConfigFolderSelected = 0
        BKConfigFolderTopLine = 0
        BKFolderLineDict = BKJournalFolderDict
        BKFolderListOrder = BKJournalListOrder
        BKFolderSelected = BKJournalFolderSelected
        BKFolderTopLine = BKJournalFolderTopLine
        BKFolderSelectChanged = 0
        BKIncomingFolder = None

        BKNewItemsInInbox = 0
        BKCurrentContent = None
        BKContentList = []
        BKContentListTopLine = 0

        IsYeeshaBookEnabled = 1
        IsEntireYeeshaBookEnabled = 1

        self.IDetermineCensorLevel()
        self.IDetermineKILevel()
        self.IDetermineKIFlags()
        self.IDetermineGZ()
        
        # Hide all dialogs first, then we'll show the one we want
        KINanoBlackBar.dialog.hide()
        KIMicroBlackbar.dialog.hide()
        KIMicro.dialog.hide()
        KIMini.dialog.hide()
        KIBlackbar.dialog.hide()
        BigKI.dialog.hide()
        self.IEnterChatMode(0)
        if theKILevel == kNanoKI:
            PtDebugPrint("Its a nanoKI",level=kDebugDumpLevel)
            # show the microBlackbar
            KINanoBlackBar.dialog.show()      # show the BlackBar as soon as its loaded
        
        # if microKI or singlePlayer (stays at micro)
        elif PtIsSinglePlayerMode():
            PtDebugPrint("Its a microKI",level=kDebugDumpLevel)
            # show the microBlackbar
            KIMicroBlackbar.dialog.show()      # show the BlackBar as soon as its loaded
            # but nothing else... even if they have NormalKI

        elif theKILevel == kMicroKI:
            PtDebugPrint("Its a microKI",level=kDebugDumpLevel)
            # show the microBlackbar
            KIMicroBlackbar.dialog.show()      # show the BlackBar as soon as its loaded
            # show the microKI
            KIMicro.dialog.show()
        elif theKILevel == kNormalKI:
            PtDebugPrint("Its a normalKI",level=kDebugDumpLevel)
            KIBlackbar.dialog.show()      # show the BlackBar as soon as its loaded
            self.IClearBBMini()
            # check for unseen messages
            self.ICheckInboxForUnseen()

        self.IminiPutAwayKI()

        modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kBKRadioModeID))
        modeselector.setValue(0)

        self.IBigKIRefreshFolders()
        self.IBigKIRefreshFolderDisplay()
        self.IBigKIRefreshContentList()
        self.IBigKIRefreshContentListDisplay()

        self.IBigKIChangeMode(kBKListMode)
        #Clear out any existing marker game
        self.markerGameManager = MarkerGameManager(self)

#Tye: this function and it's callers should be removed!
    def ICheckIfCGZShouldBeRunning(self):
        global gAlreadyCheckedCGZGame
        if not gAlreadyCheckedCGZGame:
            gAlreadyCheckedCGZGame = 1
            vault = ptVault()
            # is there a chronicle for the GZ games?
            entry = vault.findChronicleEntry(kChronCGZPlaying)
            if type(entry) != type(None):
                gCurrentMarkerGame = entry.chronicleGetValue()
                if gCurrentMarkerGame != "":
                    Mmgr = ptMarkerMgr()
                    # if there is a game running, close it down
                    if not Mmgr.isGameRunning():
                        PtDebugPrint("xKI::IDetermineEndGame - but it wasn't running in MarkTag world! - Restarting",level=kDebugDumpLevel)
                        curGameFolder = self.IFindGameInFolder(self.IFindHiddenFolder(),gCurrentMarkerGame)
                        if curGameFolder:
                            #markerRefs = curGameFolder.getChildNodeRefList()
                            #for markerRef in markerRefs:
                            #    if markerRef.beenSeen():
                            #        print "^^^^seen"
                            #    else:
                            #        print "^^^^NOTseen"
                            Mmgr.setWorkingMarkerFolder(curGameFolder)
                            Mmgr.createGame(120,PtMarkerMsgGameType.kGameTypeQuest,[])
                            Mmgr.startGame()
                        else:
                            #print "****xKI - could not find current game folder"
                            pass
                    else:
                        #print "*****xKI - current game already running"
                        pass
                else:
                    #print "*****xKI - there is no current game"
                    pass
    def IFindHiddenFolder(self):
        #== find the folder where the markerfolders might be
        # search thru the age journal folders
        vault = ptVault()
        jfolder = None
        master_agefolder = vault.getAgeJournalsFolder()
        if type(master_agefolder) != type(None):
            agefolderRefs = master_agefolder.getChildNodeRefList()
            for agefolderRef in agefolderRefs:
                agefolder = agefolderRef.getChild()
                agefolder = agefolder.upcastToFolderNode()
                if type(agefolder) != type(None):
                    # look for the Hidden folder
                    if self.IsFolderHidden(agefolder):
                        jfolder = agefolder
                        break
        return jfolder
    def IFindGameInFolder(self,folder,gameName):
        "find the marker game in the folder"
        if folder:
            # need to try to find the game
            folderRefs = folder.getChildNodeRefList()
            for jref in folderRefs:
                jnode = jref.getChild()
                jnode = jnode.upcastToMarkerGameNode()
                # is it a marker folder list?
                if type(jnode) != type(None):
                    # is it named the right one?
                    if jnode.getGameName() == gameName:
                        print "Found %s marker game in folder %s" % (folder.folderGetName(),jnode.folderGetName())
                        return jnode
        return None
#Tye: The previous two functions may not be necessary! (please check)


    def OnGUINotify(self,id,control,event):
        "Events from the Blackbar and KIMini dialog..."
        # micro to miniKI globals
        global theKILevel
        # miniKI globals
        global WaitingForAnimation
        global OriginalForeAlpha
        global OriginalSelectAlpha
        global PlayerInfoName
        global MiniKIFirstTimeShow
        global OriginalminiKICenter
        global LastminiKICenter
        global BKPlayerList
        global TicksOnFull
        global FadeEnableFlag
        global OnlyGetPMsFromBuddies
        global OnlyAllowBuddiesOnRequest
        global WorkingMarkerFolder
        global MarkerGameTimeID
        # bigKI globals
        global BKRightSideMode
        global BKInEditMode
        global BKCurrentContent
        global BKFolderSelected
        global BKFolderSelectChanged
        global BKFolderTopLine
        global BKJournalFolderSelected
        global BKJournalFolderTopLine
        global BKPlayerFolderSelected
        global BKPlayerFolderTopLine
        global BKConfigFolderSelected
        global BKConfigFolderTopLine
        global BKContentList
        global BKContentListTopLine
        global BKPlayerSelected
        global BKFolderLineDict
        global BKFolderListOrder
        global BKGettingPlayerID
        global MFdialogMode
        # Yes/No globals
        global YNWhatReason
        global OfferLinkFromWho
        global YNOutsideSender
        #Marker Game Globals
        global kMarkerGameDefaultColor
        global kMarkerGameSelectedColor
        global BKAgeOwnerEditDescription
        PtDebugPrint("xKI::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel)
###############################################
##
##  BlackBar processing
##
###############################################
        if id == KIBlackbar.id:
            if event == kDialogLoaded:
                # test to see what level the KI is at
                #~ self.IDetermineKILevel()
                #~ if theKILevel == kNormalKI:
                    #~ control.show()      # show the BlackBar as soon as its loaded
                    #~ self.IClearBBMini()
                pass
            elif event == kAction or event == kValueChanged:
                # test to see which control had the event
                bbID = control.getTagID()
                if bbID == kMiniMaximizeRGID:
                    if control.getValue() == 0:
                        if PtIsDialogLoaded("KIMini"):
                            KIMini.dialog.show()
                    elif control.getValue() == -1:
                        if PtIsDialogLoaded("KIMini"):
                            KIMini.dialog.hide()
                elif bbID == kExitButtonID:
                    yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
                    yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
                    self.ILocalizeQuitNoDialog()
                    logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoLogoutTextID))
                    logoutText.show()
                    logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesNoLogoutButtonID))
                    logoutButton.show()
                    KIYesNo.dialog.show()
                elif bbID == kPlayerBookCBID:
                    if control.isChecked():
                        curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                        if IsEntireYeeshaBookEnabled and ( curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit ):
                            if not WaitingForAnimation:
                                self.IShowYeeshaBook()
                            else:
                                control.setChecked(0)
                        else:
                            # we can't let them use the book now
                            control.setChecked(0)
                elif bbID == kBBCCRButtonID:
                    PtShowDialog("OptionsMenuGUI")
                else:
                    PtDebugPrint("xBlackbar: Don't know this control  bbID=%d" % (bbID),level=kDebugDumpLevel)
                    pass
            elif event == kInterestingEvent:
                plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                try:
                    curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                    if IsEntireYeeshaBookEnabled and ( curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit ):
                        PtDebugPrint("xBlackbar: interesting - show playerbook",level=kDebugDumpLevel)
                        plybkCB.show()
                    else:
                        PtDebugPrint("xBlackbar: interesting - on ladder - hide playerbook",level=kDebugDumpLevel)
                        plybkCB.hide()
                except NameError:
                    if IsEntireYeeshaBookEnabled:
                        PtDebugPrint("xBlackbar: interesting - show playerbook",level=kDebugDumpLevel)
                        plybkCB.show()
                    else:
                        PtDebugPrint("xBlackbar: interesting - on ladder - hide playerbook",level=kDebugDumpLevel)
                        plybkCB.hide()
###############################################
##
##  nano BlackBar processing
##
###############################################
        elif id == KINanoBlackBar.id:
            PtDebugPrint("nanoBlackbar::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kDialogLoaded:
                pass
            elif event == kAction or event == kValueChanged:
                # test to see which control had the event
                bbID = control.getTagID()
                if bbID == kExitButtonID:
                    yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
                    yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
                    self.ILocalizeQuitNoDialog()
                    logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoLogoutTextID))
                    logoutText.show()
                    logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesNoLogoutButtonID))
                    logoutButton.show()
                    KIYesNo.dialog.show()
                elif bbID == kBBCCRButtonID:
                    PtShowDialog("OptionsMenuGUI")
                else:
                    PtDebugPrint("xnanoBlackbar: Don't know this control  bbID=%d" % (bbID),level=kDebugDumpLevel)
                    pass
###############################################
##
##  micro BlackBar processing
##
###############################################
        elif id == KIMicroBlackbar.id:
            PtDebugPrint("microBlackbar::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kDialogLoaded:
                if PtIsSinglePlayerMode():
                    chatBtn = ptGUIControlButton(KIMicroBlackbar.dialog.getControlFromTag(kmicroChatButton))
                    chatBtn.hide()
                rollBtn = ptGUIControlButton(KIMicroBlackbar.dialog.getControlFromTag(kRolloverLeftID))
                rollBtn.setNotifyOnInteresting(1)
                rollBtn = ptGUIControlButton(KIMicroBlackbar.dialog.getControlFromTag(kRolloverRightID))
                rollBtn.setNotifyOnInteresting(1)
            elif event == kAction or event == kValueChanged:
                # test to see which control had the event
                bbID = control.getTagID()
                if bbID == kExitButtonID:
                    yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
                    yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
                    self.ILocalizeQuitNoDialog()
                    logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoLogoutTextID))
                    logoutText.show()
                    logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesNoLogoutButtonID))
                    logoutButton.show()
                    KIYesNo.dialog.show()
                elif bbID == kPlayerBookCBID:
                    if control.isChecked():
                        curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                        if IsEntireYeeshaBookEnabled and ( curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit ):
                            if not WaitingForAnimation:
                                self.IShowYeeshaBook()
                            else:
                                # sorry, can't click now, put back to unselected
                                control.setChecked(0)
                        else:
                            # we can't let them use the book now
                            control.setChecked(0)
                elif bbID == kBBCCRButtonID:
                    PtShowDialog("OptionsMenuGUI")
                else:
                    PtDebugPrint("xmicroBlackbar: Don't know this control  bbID=%d" % (bbID),level=kDebugDumpLevel)
                    pass
            elif event == kInterestingEvent:
                plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                try:
                    curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                    if IsEntireYeeshaBookEnabled and ( curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit ):
                        PtDebugPrint("xmicroBlackbar: interesting - show playerbook",level=kDebugDumpLevel)
                        plybkCB.show()
                    else:
                        PtDebugPrint("xmicroBlackbar: interesting - on ladder - hide playerbook",level=kDebugDumpLevel)
                        plybkCB.hide()
                except NameError:
                    # no local avatar, just look at Yeeshabook enable flag
                    if IsEntireYeeshaBookEnabled:
                        PtDebugPrint("xmicroBlackbar: interesting - show playerbook",level=kDebugDumpLevel)
                        plybkCB.show()
                    else:
                        PtDebugPrint("xmicroBlackbar: interesting - on ladder - hide playerbook",level=kDebugDumpLevel)
                        plybkCB.hide()
###############################################
##
##  microKI processing
##
###############################################
        elif id == KIMicro.id:
            PtDebugPrint("microKI::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kDialogLoaded:
                # Fill in the listbox so that the test is near the enter box
                chatarea = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kChatDisplayArea))
                chatarea.lock()         # make the chat display immutable
                chatarea.unclickable()  # make the chat display non-clickable
                chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
                chatarea.disableScrollControl()
                # we need to show the button to hide it
                btnUp = ptGUIControlButton(KIMicro.dialog.getControlFromTag(kminiChatScrollUp))
                btnUp.show()
                btnUp.hide()
                # set the edit box buffer size to something larger
                chatedit = ptGUIControlEditBox(KIMicro.dialog.getControlFromTag(kChatEditboxID))
                chatedit.setStringSize(500)
                chatedit.setChatMode(1)
            elif event == kShowHide:
                if control.isEnabled():
#### don't automatically show faded elements on show
#                    self.IKillFadeTimer()
#                    self.IStartFadeTimer()
                    if not self.isChatting:
                        self.IFadeCompletely()
                    pass
            elif event == kAction or event == kValueChanged:
                ctrlID = control.getTagID()
                #~ PtDebugPrint("xKImicro:: tagID=%d, event=%d control=" % (ctrlID,event),control )
                if ctrlID == kChatEditboxID:
                    if not control.wasEscaped() and control.getString() != "":
                        self.ISendRTChat(control.getString())
                    self.IEnterChatMode(0)
                elif ctrlID == kChatDisplayArea:
                    self.IKillFadeTimer()
                    self.IStartFadeTimer()
            elif event == kFocusChange:
                # make sure if they are chatting then get the focus back
                if self.isChatting:
                    KIMicro.dialog.setFocus(KIMicro.dialog.getControlFromTag(kChatEditboxID))
###############################################
##
##  miniKI processing
##
###############################################
        elif id == KIMini.id:
            if event == kDialogLoaded:
                # get the original position of the miniKI
                dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kminiDragBar))
                OriginalminiKICenter = dragbar.getObjectCenter()
                #~ print "miniKI center is x,y,z  %g,%g,%g" % (OriginalminiKICenter.getX(),OriginalminiKICenter.getY(),OriginalminiKICenter.getZ())
                # retreive the original alpha's
                fore = control.getForeColor()
                OriginalForeAlpha = fore.getAlpha()
                sel = control.getSelectColor()
                OriginalSelectAlpha = sel.getAlpha()
                # Fill in the listbox so that the test is near the enter box
                chatarea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kChatDisplayArea))
                chatarea.lock()         # make the chat display immutable
                chatarea.unclickable()  # make the chat display non-clickable
                chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
                # hide the chat scroll buttons (should be nothing in chat area yet anyhow)
                chatarea.disableScrollControl()
                # we need to show the button to hide it
                btnUp = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiChatScrollUp))
                btnUp.show()
                privateChbox = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiPrivateToggle))
                privateChbox.disable()
                # set the edit box buffer size to something larger
                chatedit = ptGUIControlEditBox(KIMini.dialog.getControlFromTag(kChatEditboxID))
                chatedit.setStringSize(500)
                chatedit.setChatMode(1)
                # default the marker tag stuff to be off
                btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZDrip))
                btnmt.hide()
                btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZActive))
                btnmt.hide()
                btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZMarkerGameActive))
                btnmt.hide()
                btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZMarkerInRange))
                btnmt.hide()
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiMGNewMarker)).hide()
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiMGNewGame)).hide()
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiMGInactive)).hide()
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiMGInactive)).disable()
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kPelletScoreButton)).hide()
                # set the color to the off color
                for mcbid in range(kminiMarkerIndicator01,kminiMarkerIndicatorLast+1):
                    mcb = ptGUIControlProgress(KIMini.dialog.getControlFromTag(mcbid))
                    mcb.setValue(gMarkerColors['off'])
            elif event == kShowHide:
                if control.isEnabled():
                    if kPelletImager != "":
                        ptGUIControlButton(KIMini.dialog.getControlFromTag(kPelletScoreButton)).show()
                    if MiniKIFirstTimeShow:
                        # set the font size and fade time from chronicle
                        self.IDetermineFontSize()
                        self.IDetermineFadeTime()
                        # if we are chatting then just let it happen
                        if not self.isChatting:
                            self.IEnterChatMode(0)
                            self.IFadeCompletely()
                        MiniKIFirstTimeShow = 0
                    self.IRefreshPlayerList()
                    self.IRefreshPlayerListDisplay()
                    self.IRefreshMiniKIMarkerDisplay()
#                    # turn off the bloody blinking if they are going to look at their KI
#                    if NewItemAlert.isEnabled()
#                        NewItemAlert.dialog.hide()
#                        kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
#                        kialert.hide()
#### don't automatically show faded elements on show
#                    self.IKillFadeTimer()
#                    # only start the fade timer when not chatting
#                    if not self.isChatting:
#                        self.IStartFadeTimer()
                else:
                    self.IEnterChatMode(0)
                    self.IClearBBMini()
            elif event == kAction or event == kValueChanged:
                ctrlID = control.getTagID()
                PtDebugPrint("xKImini:: tagID=%d, event=%d control=" % (ctrlID,event),control,level=kDebugDumpLevel )
                if ctrlID == kChatEditboxID:
                    if not control.wasEscaped() and control.getString() != "":
                        self.ISendRTChat(control.getString())
                    self.IEnterChatMode(0)
                    self.IStartFadeTimer()
                elif ctrlID == kPlayerList:
                    # make sure they don't click outside what's there
                    plyrsel = control.getSelection()
                    if plyrsel >= control.getNumElements():
                        control.setSelection(0)
                        plyrsel = 0
                    # place selected player in SendTo textbox
                    sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                    caret = ptGUIControlTextBox(KIMini.dialog.getControlFromTag(kChatCaretID))
                    caret.setString(">")
                    privateChbox = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiPrivateToggle))
                    privateChbox.setChecked(0)
                    PlayerInfoName = None   # assume they are going to pick a non-agemember
                    if plyrsel == -1:
                        BKPlayerSelected = None
                    else:
                        BKPlayerSelected = BKPlayerList[plyrsel]
                        # if its a device folder or just a string
                        if isinstance(BKPlayerSelected,DeviceFolder) or type(BKPlayerSelected) == type(""):
                            # then can't be sent to
                            pass
                        elif isinstance(BKPlayerSelected,Device):
                            sendToField.setString(BKPlayerSelected.name)
                        # is it a specific player info node?
                        elif isinstance(BKPlayerSelected,ptVaultNodeRef):
                            plyrInfonode = BKPlayerSelected.getChild()
                            plyrInfo = plyrInfonode.upcastToPlayerInfoNode()
                            # its an content.. should be a player
                            if type(plyrInfo) != type(None):
                                sendToField.setString(plyrInfo.playerGetName())
                                # set private caret
                                caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt")+unicode(plyrInfo.playerGetName())+U" >")
                                privateChbox.setChecked(1)
                            else:
                                BKPlayerSelected = None
                        elif isinstance(BKPlayerSelected,ptPlayer):
                            sendToField.setString(BKPlayerSelected.getPlayerName())
                            caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt")+unicode(BKPlayerSelected.getPlayerName())+U" >")
                            PlayerInfoName = BKPlayerSelected
                            privateChbox.setChecked(1)
                        elif isinstance(BKPlayerSelected,ptVaultPlayerInfoListNode):
                            # its a player list, display its name
                            fldrType = BKPlayerSelected.folderGetType()
                            # if it is other than the All Players folder
                            if fldrType != PtVaultStandardNodes.kAgeMembersFolder:
                                # if its a list of age owners... must be list of neighbors
                                if fldrType == PtVaultStandardNodes.kAgeOwnersFolder:
                                    fldrType = PtVaultStandardNodes.kHoodMembersFolder
                                caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt")+xLocTools.FolderIDToFolderName(fldrType)+U" >")
                            # since we are going to a folder then... not private and no player selected
                            privateChbox.setChecked(0)
                            BKPlayerSelected = None
                        elif isinstance(BKPlayerSelected,MarkerPlayer):
                            caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt")+unicode(BKPlayerSelected.player.getPlayerName())+U" >")
                            privateChbox.setChecked(1)
                            BKPlayerSelected = None
                        elif isinstance(BKPlayerSelected,MarkerGame):
                            caret.setStringW(PtGetLocalizedString("KI.Chat.MarkerTOAllTeams"))
                            privateChbox.setChecked(0)
                            BKPlayerSelected = None
                        elif isinstance(BKPlayerSelected,DPLBranchStatusLine):
                            if type(CurrentPlayingMarkerGame) != type(None):
                                if BKPlayerSelected == CurrentPlayingMarkerGame.greenTeamDPL:
                                    caret.setStringW(PtGetLocalizedString("KI.Chat.MarkerTOGreenTeam"))
                                elif BKPlayerSelected == CurrentPlayingMarkerGame.redTeamDPL:
                                    caret.setStringW(PtGetLocalizedString("KI.Chat.MarkerTORedTeam"))
                            privateChbox.setChecked(0)
                            BKPlayerSelected = None
                        else:
                            BKPlayerSelected = None
                    if type(BKPlayerSelected) == type(None):
                        sendToField.setString(" ")
                    self.IBigKISetToButtons()
                    # don't keep the focus, we don't need it
                    if self.isChatting:
                        chatedit = ptGUIControlEditBox(KIMini.dialog.getControlFromTag(kChatEditboxID))
                        KIMini.dialog.setFocus(chatedit.getKey())
                    # they're playing with the player list... kill the fade timer
                    self.IKillFadeTimer()
                    self.IStartFadeTimer()
                elif ctrlID == kminiPutAwayID:
                    self.IminiPutAwayKI()
                elif ctrlID == kminiToggleBtnID:
                    self.IminiToggleKISize()
                elif ctrlID == kminiTakePicture:
                    self.IminiTakePicture()
                elif ctrlID == kminiCreateJournal:
                    self.IminiCreateJournal()
                elif ctrlID == kminiMuteAll:
                    # hit the mute button... set mute depending on control
                    audio = ptAudioControl()
                    if control.isChecked():
                        audio.muteAll()
                    else:
                        audio.unmuteAll()
                elif ctrlID == kminiPlayerListUp:
                    # scroll the Player list up one line
                    playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
                    self.IScrollUpListbox(playerlist,kminiPlayerListUp,kminiPlayerListDown)
                elif ctrlID == kminiPlayerListDown:
                    # scroll the player list down one line
                    playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
                    self.IScrollDownListbox(playerlist,kminiPlayerListUp,kminiPlayerListDown)
                elif ctrlID == kminiGZMarkerInRange:
                    PtDebugPrint("miniKI::GetMarkerButton hit!",level=kDebugDumpLevel )
                    self.ICaptureGZMarker()
                    self.IRefreshMiniKIMarkerDisplay()
                elif ctrlID == kChatDisplayArea:
                    self.IKillFadeTimer()
                    self.IStartFadeTimer()
                elif ctrlID == kminiMGNewMarker:
                    self.ICreateAMarker()
                elif ctrlID == kminiMGNewGame:
                    self.ICreateMarkerGame()
                elif ctrlID == kJalakMiniIconBtn:
                    if AgeName == "Jalak":
                        self.JalakGUIToggle()
                    else:
                        ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
                elif ctrlID == kPelletScoreButton:
                    self.IUploadPelletScore()

            elif event == kFocusChange:
                # make sure if they are chatting then get the focus back
                if self.isChatting:
                    # if the bigKI is up then let the focus go where it wants
                    # ...otherwise put the focus back to the chat line
                    if not BigKI.dialog.isEnabled():
                        KIMini.dialog.setFocus(KIMini.dialog.getControlFromTag(kChatEditboxID))
                    else:
                        if not BKInEditMode:
                            KIMini.dialog.setFocus(KIMini.dialog.getControlFromTag(kChatEditboxID))
###############################################
##
##  BigKI processing
##
###############################################
        elif id == BigKI.id:
            PtDebugPrint("BigKI::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kDialogLoaded:
                # hide stuff until needed
                #~ self.IBigKIHideToButtons()
                BKInEditMode = 0
                # need to put animation at off position, so there is no pop with the animation plays
                KIOnAnim.animation.skipToTime(1.5)
                # if single player mode... can't have this
                if PtIsSinglePlayerMode():
                    modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kBKRadioModeID))
                    modeselector.setValue(0)
                    modeselector.disable()
                else:
                    pdisable = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKDisabledPeopleButton))
                    pdisable.disable()
                    gdisable = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKDisabledGearButton))
                    gdisable.disable()
                self.IBigKISetNotifyForLong()
            elif event == kShowHide:
                if control.isEnabled():
                    # hide all the long guys
                    self.IBigKIHideLongFolderNames()
                    # make sure that fields are filled
                    self.IBigKISetStatics()
                    self.IBigKISetChanging()
                    # refresh the player list
                    self.IRefreshPlayerList()
                    self.IRefreshPlayerListDisplay()
                    # make the miniKI fields stay up
                    self.IKillFadeTimer()
                    # refresh the folders and their contents
                    self.IBigKIRefreshFolders()
                    self.IBigKIRefreshFolderDisplay()
                    self.IBigKIShowBigKI()
                else:
                    # else if the BigKI is being disabled
                    self.IStartFadeTimer()
                    pass
            elif event == kAction or event == kValueChanged:
                bkID = control.getTagID()
                PtDebugPrint("BigKI::OnGUINotify event=%d controlID=%d" % (event,bkID),level=kDebugDumpLevel )
                # was it one of the folder buttons?
                if bkID >= kBKIIncomingBtn and bkID <= kBKIFolderLineBtnLast:
                    if BKFolderLineDict is BKConfigFolderDict:
                        BKFolderSelected = bkID-kBKIIncomingBtn+BKFolderTopLine
                        self.IShowSelectedConfig()
                    else:
                        oldselect = BKFolderSelected
                        BKFolderSelected = bkID-kBKIIncomingBtn+BKFolderTopLine
                        if oldselect != BKFolderSelected:
                            BKFolderSelectChanged = 1
                        else:
                            BKFolderSelectChanged = 0
                        self.IBigKIChangeMode(kBKListMode)
                # was it scroll folders up a line?
                elif bkID == kBKFolderUpLine:
                    if BKFolderTopLine > 0:
                        BKFolderTopLine -= 1;
                        self.IBigKIRefreshFolderDisplay()
                        self.IBigKISetToButtons()
                # was it scroll folders down a line?
                elif bkID == kBKFolderDownLine:
                    BKFolderTopLine += 1;
                    self.IBigKIRefreshFolderDisplay()
                    self.IBigKISetToButtons()
                elif bkID == kBKLMUpButton:
                    if BKRightSideMode == kBKListMode:
                        if BKContentListTopLine > 0:
                            BKContentListTopLine -= kContentListScrollSize;
                            if BKContentListTopLine < 0:
                                BKContentListTopLine = 0
                            self.IBigKIRefreshContentListDisplay()
                elif bkID == kBKLMDownButton:
                    if BKRightSideMode == kBKListMode:
                        BKContentListTopLine += kContentListScrollSize;
                        self.IBigKIRefreshContentListDisplay()
                elif bkID >= kBKIToFolderButton01 and bkID <= kBKIToFolderButtonLast:
                    # determine which folder
                    tofolderNum = bkID-kBKIToFolderButton01+BKFolderTopLine+1
                    # if they are in an expanded mode, then they can move the element to another folder
                    if BKRightSideMode != kBKListMode and type(BKCurrentContent) != type(None):
                        # move the CurrentContent to the selected folder
                        if isinstance(BKCurrentContent,ptPlayer):
                            # add to new folder
                            try:
                                newfoldername = BKFolderListOrder[tofolderNum]
                                newfolder = BKFolderLineDict[newfoldername]
                                if type(newfolder) != type(None):
                                    if newfolder.getType()==PtVaultNodeTypes.kAgeInfoNode:
                                        #make sure they are not trying to add themselves
                                        localplayer = PtGetLocalPlayer()
                                        if BKCurrentContent.getPlayerID() != localplayer.getPlayerID():
                                            self.IInviteToVisit(BKCurrentContent.getPlayerID(),newfolder)
                                    elif newfolder.getType()==PtVaultNodeTypes.kPlayerInfoListNode:
                                        #make sure they are not trying to add themselves
                                        localplayer = PtGetLocalPlayer()
                                        if BKCurrentContent.getPlayerID() != localplayer.getPlayerID():
                                            newfolder.playerlistAddPlayer(BKCurrentContent.getPlayerID())
                            except (IndexError,KeyError):
                                # if there was an error, then just display the whatever was already selected
                                tofolderNum = BKFolderSelected
                        else:
                            oldfolder = BKCurrentContent.getParent()
                            theElement = BKCurrentContent.getChild()
                            if type(theElement) != type(None):
                                # add to new folder
                                try:
                                    newfoldername = BKFolderListOrder[tofolderNum]
                                    newfolder = BKFolderLineDict[newfoldername]
                                    if type(newfolder) != type(None):
                                        if newfolder.getType()==PtVaultNodeTypes.kAgeInfoNode:
                                            theElement = theElement.upcastToPlayerInfoNode()
                                            #make sure they are not trying to add themselves
                                            localplayer = PtGetLocalPlayer()
                                            if type(theElement) != type(None) and theElement.playerGetID() != localplayer.getPlayerID():
                                                self.IInviteToVisit(theElement.playerGetID(),newfolder)
                                        elif newfolder.getType()==PtVaultNodeTypes.kPlayerInfoListNode:
                                            theElement = theElement.upcastToPlayerInfoNode()
                                            #make sure they are not trying to add themselves
                                            localplayer = PtGetLocalPlayer()
                                            if type(theElement) != type(None) and theElement.playerGetID() != localplayer.getPlayerID():
                                                theElement = theElement.upcastToPlayerInfoNode()
                                                newfolder.playerlistAddPlayer(theElement.playerGetID())
                                        else:
                                            copynoderef = newfolder.addNode(theElement)
#####====> needs to be fixed when we can just copy the saver
                                            #~ # copy from data to new noderef
                                            #~ copynoderef.copyFrom(BKCurrentContent)
#####====> needs to be fixed when we can just copy the saver
                                            BKCurrentContent = copynoderef
                                            if type(oldfolder) != type(None):
                                                # only remove old one if it was not a player (from a playerlist)
                                                oldfolder.removeNode(theElement)
                                except (IndexError,KeyError):
                                    # if there was an error, then just display the whatever was already selected
                                    tofolderNum = BKFolderSelected
                        # Leave it at the folder they are on
                        BKFolderSelectChanged = 1
                        self.IBigKIChangeMode(kBKListMode)
                        # they could have copied a player... refresh list
                        self.IRefreshPlayerList()
                        self.IRefreshPlayerListDisplay()
                elif bkID == kBKRadioModeID:
                    # save the previous selected and topline
                    if BKFolderLineDict is BKJournalFolderDict:
                        BKJournalFolderSelected = BKFolderSelected
                        BKJournalFolderTopLine = BKFolderTopLine
                    elif BKFolderLineDict is BKPlayerFolderDict:
                        BKPlayerFolderSelected = BKFolderSelected
                        BKPlayerFolderTopLine = BKFolderTopLine
                    elif BKFolderLineDict is BKConfigFolderDict:
                        BKConfigFolderSelected = BKFolderSelected
                        BKConfigFolderTopLine = BKFolderTopLine
                    modeselect = control.getValue()
                    if modeselect == 0:
                        # journal mode
                        BKFolderLineDict = BKJournalFolderDict
                        BKFolderListOrder = BKJournalListOrder
                        BKFolderSelected = BKJournalFolderSelected
                        BKFolderTopLine = BKJournalFolderTopLine
                    elif modeselect == 1:
                        # player list mode
                        BKFolderLineDict = BKPlayerFolderDict
                        BKFolderListOrder = BKPlayerListOrder
                        BKFolderSelected = BKPlayerFolderSelected
                        BKFolderTopLine = BKPlayerFolderTopLine
                    else:
                        # configuration mode
                        BKFolderLineDict = BKConfigFolderDict
                        BKFolderListOrder = BKConfigListOrder
                        BKFolderSelected = BKConfigFolderSelected
                        BKFolderTopLine = BKConfigFolderTopLine
                    # reset the topline and selection
                    self.IBigKIRefreshFolderDisplay()
                    if modeselect == 0 and (BKRightSideMode == kBKPictureExpanded or BKRightSideMode == kBKJournalExpanded or BKRightSideMode == kBKMarkerListExpanded):
                        # if we're switchin' to journal mode and already in PictureExpanded, then must be picture taking
                        self.IBigKIInvertToFolderButtons()
                    else:
                        # are we switching to the configuration mode?
                        if modeselect == 2:
                            # display the config screen that is selected
                            self.IShowSelectedConfig()
                           
                        else:
                            # otherwise... make sure its in listmode
                            self.IBigKIChangeMode(kBKListMode)
                elif bkID == kBKIToPlayerButton:
                    if type(BKCurrentContent) != type(None) and type(BKPlayerSelected) != type(None):
                        sendElement = BKCurrentContent.getChild()
                        toplayerbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKIToPlayerButton))
                        if type(sendElement) != type(None):
                            if isinstance(BKPlayerSelected,DeviceFolder):
                                # can't send to a folder
                                pass
                            elif isinstance(BKPlayerSelected,Device):
                                PtDebugPrint("xKI: send to device %s of type %s" % (BKPlayerSelected.name,BKPlayerSelected.type),level=kDebugDumpLevel)
                                # do something here to send the element to the device
                                PtDebugPrint("xKI: send to device",level=kDebugDumpLevel)

                                if self.ImagerMap.has_key(BKPlayerSelected.name):
                                    notify = ptNotify(self.key)
                                    notify.clearReceivers()
                                    notify.addReceiver(self.ImagerMap[BKPlayerSelected.name])
                                    notify.netPropagate(1)
                                    notify.netForce(1)
                                    notify.setActivate(1.0)
                                    sname = "Upload=%s" % (BKPlayerSelected.name)
                                    notify.addVarNumber(sname, sendElement.getID())
                                    notify.send()
                                
                                toplayerbtn.hide()
                            elif isinstance(BKPlayerSelected,ptVaultNode):
                                if BKPlayerSelected.getType()==PtVaultNodeTypes.kPlayerInfoListNode:
                                    PtDebugPrint("xKI: we are sending a message to the entire %s folder" % (BKPlayerSelected.folderGetName()),level=kDebugDumpLevel)
                                    plyrreflist = BKPlayerSelected.getChildNodeRefList()
                                    for plyrref in plyrreflist:
                                        plyr = plyrref.getChild()
                                        plyr = plyr.upcastToPlayerInfoNode()
                                        if type(plyr) != type(None):
                                            # test to not send it to ourselves.
                                            sendElement.sendTo(plyr.playerGetID())
                                elif BKPlayerSelected.getType()==PtVaultNodeTypes.kPlayerInfoNode:
                                    sendElement.sendTo(BKPlayerSelected.playerGetID())
                                else:
                                    self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.CantSend"))
                                toplayerbtn.hide()
                            elif isinstance(BKPlayerSelected,ptVaultNodeRef):
                                plyrElement = BKPlayerSelected.getChild()
                                if type(plyrElement) != type(None) and plyrElement.getType()==PtVaultNodeTypes.kPlayerInfoNode:
                                    plyrElement = plyrElement.upcastToPlayerInfoNode()
                                    sendElement.sendTo(plyrElement.playerGetID())
                                else:
                                    self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.PlayerNotFound"))
                                toplayerbtn.hide()
                            elif isinstance(BKPlayerSelected,ptPlayer):
                                sendElement.sendTo(BKPlayerSelected.getPlayerID())
                                toplayerbtn.hide()
                            else:
                                self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.UnknownPlayerType"))
                        else:
                            self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.BadJournalElement"))
            elif event == kInterestingEvent:
                if type(control) != type(None):
                    shortTB = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(control.getTagID()+21))
                    longTB = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(control.getTagID()+521))
                    if shortTB.getStringJustify() == kRightJustify and control.isInteresting():
                        # switch to long versions
                        longTB.setForeColor(shortTB.getForeColor())
                        longTB.setString(shortTB.getString())
                        shortTB.hide()
                        longTB.show()
                    else:
                        shortTB.show()
                        longTB.hide()
###############################################
##
##  List Mode dialog processing
##
###############################################
        elif id == KIListModeDialog.id:
            PtDebugPrint("KIListMode::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kAction or event == kValueChanged:
                lmID = control.getTagID()
                if lmID >= kBKIListModeLineBtn01 and lmID <= kBKIListModeLineBtnLast:
                    # find out which button to what content and selected it
                    whichone = lmID - kBKIListModeLineBtn01 + BKContentListTopLine
                    if whichone < len(BKContentList):
                        theContent = BKContentList[whichone]
                        if type(theContent) != type(None):
                            BKCurrentContent = theContent
                            if isinstance(BKCurrentContent,QuestionNote):
                                nextmode = kBKQuestionNote
                                self.IBigKIChangeMode(nextmode)
                            elif isinstance(BKCurrentContent,ptPlayer):
                                nextmode = kBKPlayerExpanded
                                self.IBigKIChangeMode(nextmode)
                            else:
                                theElement = theContent.getChild()
                                if type(theElement) != type(None):
                                    datatype = theElement.getType()
                                    if datatype == PtVaultNodeTypes.kTextNoteNode:
                                        nextmode = kBKJournalExpanded
                                    elif datatype == PtVaultNodeTypes.kImageNode:
                                        nextmode = kBKPictureExpanded
                                    elif datatype == PtVaultNodeTypes.kPlayerInfoNode:
                                        nextmode = kBKPlayerExpanded
                                    elif datatype == PtVaultNodeTypes.kMarkerGameNode:
                                        nextmode = kBKMarkerListExpanded
                                    else:
                                        BKCurrentContent = None
                                        nextmode = kBKListMode
                                    self.IBigKIChangeMode(nextmode)
                                else:
                                    PtDebugPrint("xBigKI: ListMode - content is None for element!",level=kErrorLevel)
                elif lmID == kBKIListModeCreateBtn:
                    if BKFolderLineDict is BKPlayerFolderDict:
                        BKGettingPlayerID = 1
                        self.IBigKIChangeMode(kBKPlayerExpanded)
                    else:
                        self.IBigKICreateJournalNote()
                        self.IBigKIChangeMode(kBKJournalExpanded)
                        self.IBigKIDisplayCurrentContentJournal()
                        self.IBigKIEnterEditMode(kBKEditFieldJRNTitle)
            #~ elif event == kShowHide:
                #~ if control.isEnabled():
                    #~ # refresh the display of the currently selected content element
                    #~ self.IBigKINewContentList()
###############################################
##
##  Picture Expanded dialog processing
##
###############################################
        elif id == KIPictureExpanded.id:
            PtDebugPrint("KIPicture::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kDialogLoaded:
                editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[kBKEditFieldPICTitle][kBKEditIDeditbox]))
                editbox.hide()
            elif event == kShowHide:
                if control.isEnabled():
                    # refresh the display of the currently selected content element
                    self.IBigKIDisplayCurrentContentImage()
            elif event == kAction or event == kValueChanged:
                peID = control.getTagID()
                if peID == kBKIPICTitleButton:
                    if self.IsContentMutable(BKCurrentContent):
                        self.IBigKIEnterEditMode(kBKEditFieldPICTitle)
                elif peID == kBKIPICDeleteButton:
                    YNWhatReason = kYNDelete
                    elem = BKCurrentContent.getChild()
                    elem = elem.upcastToImageNode()
                    if type(elem) != type(None):
                        pictitle = elem.imageGetTitle()
                    else:
                        pictitle = "<unknown>"
                    yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
                    yesText.setStringW(PtGetLocalizedString("KI.Messages.DeletePicture", [xCensor.xCensor(pictitle,theCensorLevel)]))
                    self.ILocalizeYesNoDialog()
                    KIYesNo.dialog.show()
                # see if it is one of the editboxes
                elif peID == kBKIPICTitleEdit:
                    self.IBigKISaveEdit(1)
            elif event == kFocusChange:
                if self.IsContentMutable(BKCurrentContent):
                    self.IBigKICheckFocusChange()
###############################################
##
##  Journal Expanded (Text) dialog processing
##
###############################################
        elif id == KIJournalExpanded.id:
            PtDebugPrint("KIJournal::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kDialogLoaded:
                editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[kBKEditFieldJRNTitle][kBKEditIDeditbox]))
                editbox.hide()
            elif event == kShowHide:
                if control.isEnabled():
                    # refresh the display of the currently selected content element
                    self.IBigKIDisplayCurrentContentJournal()
            elif event == kAction or event == kValueChanged:
                jeID = control.getTagID()
                # see if it is one of the buttons
                if jeID == kBKIJRNTitleButton:
                    if self.IsContentMutable(BKCurrentContent):
                        self.IBigKIEnterEditMode(kBKEditFieldJRNTitle)
                elif jeID == kBKIJRNNoteButton:
                    if self.IsContentMutable(BKCurrentContent):
                        self.IBigKIEnterEditMode(kBKEditFieldJRNNote)
                elif jeID == kBKIJRNDeleteButton:
                    YNWhatReason = kYNDelete
                    elem = BKCurrentContent.getChild()
                    elem = elem.upcastToTextNoteNode()
                    if type(elem) != type(None):
                        jrntitle = elem.noteGetTitle()
                    else:
                        jrntitle = "<unknown>"
                    yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
                    yesText.setStringW(PtGetLocalizedString("KI.Messages.DeleteJournal", [xCensor.xCensor(jrntitle,theCensorLevel)]))
                    self.ILocalizeYesNoDialog()
                    KIYesNo.dialog.show()
                # see if it is one of the editboxes
                elif jeID == kBKIJRNTitleEdit or kBKIJRNNoteEdit:
                    if self.IsContentMutable(BKCurrentContent):
                        self.IBigKISaveEdit(1)
            elif event == kFocusChange:
                if self.IsContentMutable(BKCurrentContent):
                    if type(control) != type(None):
                        # if we are changing focus to the multiline, then we are entering edit mode
                        jeID = control.getTagID()
                        if jeID == kBKIJRNNote:
                            self.IBigKIEnterEditMode(kBKEditFieldJRNNote)
                            return
                    self.IBigKICheckFocusChange()
            else:
                pass
###############################################
##
##  Player list Expanded dialog processing
##
##############################################
        elif id == KIPlayerExpanded.id:
            PtDebugPrint("KIJournal::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kShowHide:
                if control.isEnabled():
                    # refresh the display of the currently selected content element
                    self.IBigKIDisplayCurrentContentPlayer()
            elif event == kAction or event == kValueChanged:
                plID = control.getTagID()
                # see if it is one of the buttons
                if plID == kBKIPLYDeleteButton:
                    # delete button
                    YNWhatReason = kYNDelete
                    elem = BKCurrentContent.getChild()
                    elem = elem.upcastToPlayerInfoNode()
                    if type(elem) != type(None):
                        plyrname = elem.playerGetName()
                    else:
                        plyrname = "<unknown>"
                    # use the name in the folder list
                    try:
                        pfldname = BKFolderListOrder[BKFolderSelected]
                    except LookupError:
                        pfldname = "<unknown>"
                    yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
                    yesText.setStringW(PtGetLocalizedString("KI.Messages.DeletePlayer", [xCensor.xCensor(plyrname,theCensorLevel),pfldname]))
                    self.ILocalizeYesNoDialog()
                    KIYesNo.dialog.show()
                elif plID == kBKIPLYPlayerIDEditBox:
                    # editbox event
                    self.IBigKICheckSavePlayer()
            elif event == kFocusChange:
                if BKGettingPlayerID:
                    if KIPlayerExpanded.dialog.isEnabled():
                        plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYPlayerIDEditBox))
                        plyIDedit.focus()
                        KIPlayerExpanded.dialog.setFocus(plyIDedit.getKey())
                    else:
                        BKGettingPlayerID = 0
                        self.IBigKIChangeMode(kBKListMode)
###############################################
##
##  KI Setting Expanded dialog processing
##
###############################################
        elif id == KISettings.id:
            PtDebugPrint("KISettings::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kShowHide:
                if control.isEnabled():
                    # set the text fields
                    tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kBKIKISettingsText))
                    tfield.setStringW(PtGetLocalizedString("KI.Config.Settings"))
                    tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kBKIKIFontSizeText))
                    tfield.setStringW(PtGetLocalizedString("KI.Config.FontSize"))
                    tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kBKIKIFadeTimeText))
                    tfield.setStringW(PtGetLocalizedString("KI.Config.ChatFadeTime"))
                    tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kBKIKIOnlyPMText))
                    tfield.setStringW(PtGetLocalizedString("KI.Config.OnlyBuddies"))
                    # refresh the volume settings
                    self.IRefreshKISettings()
                    pass
                else:
                    #... this would be a good place to save settings out
                    self.ISaveFontSize()
                    self.ISaveFadeTime()
                    self.ISaveKIFlags()
            elif event == kAction or event == kValueChanged:
                kiID = control.getTagID()
                if kiID == kBKIKIFontSize:
                    slidePerFont = float(control.getMax()-control.getMin()+1.0)/float(len(FontSizeList))
                    fontIndex = int(control.getValue()/slidePerFont + 0.25)
                    if fontIndex >= len(FontSizeList):
                        fontIndex = len(FontSizeList)-1
                    self.ISetFontSize(FontSizeList[fontIndex])
                elif kiID == kBKIKIFadeTime:
                    slidePerTime = float(control.getMax()-control.getMin())/float(kFadeTimeMax)
                    TicksOnFull = int(control.getValue()/slidePerTime + 0.25)
                    PtDebugPrint("KISettings: FadeTime set to %d"%(TicksOnFull),level=kDebugDumpLevel)
                    if TicksOnFull == kFadeTimeMax:
                        # disable the fade all together
                        FadeEnableFlag = 0
                        PtDebugPrint("KISettings: FadeTime disabled",level=kDebugDumpLevel)
                    else:
                        FadeEnableFlag = 1
                        PtDebugPrint("KISettings: FadeTime enabled",level=kDebugDumpLevel)
                elif kiID == kBKIKIOnlyPM:
                    OnlyGetPMsFromBuddies = control.isChecked()
                elif kiID == kBKIKIBuddyCheck:
                    OnlyAllowBuddiesOnRequest = control.isChecked()
            else:
                # don't know... must be an error
                pass
###############################################
##
##  Volume Setting Expanded dialog processing
##
###############################################
        elif id == KIVolumeExpanded.id:
            PtDebugPrint("KIVolume::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kShowHide:
                if control.isEnabled():
                    # refresh the volume settings
                    self.IRefreshVolumeSettings()
                    pass
                else:
                    # the volume setting menu is being hide
                    #... this would be a good place to save settings out
                    pass
            elif event == kAction or event == kValueChanged:
                plID = control.getTagID()
                audio = ptAudioControl()
                # see if it is one of the buttons
                if plID == kBKISoundFXVolSlider:
                    # Sound FX volume change
                    setting = control.getValue()
                    PtDebugPrint("SoundFX being changed to %g (into %g)" % (setting,setting/10),level=kDebugDumpLevel)
                    audio.setSoundFXVolume(setting/10)
                elif plID == xBKIMusicVolSlider:
                    # Music volume change
                    setting = control.getValue()
                    PtDebugPrint("Music being changed to %g (into %g)" % (setting,setting/10),level=kDebugDumpLevel)
                    audio.setMusicVolume(setting/10)
                elif plID == xBKIVoiceVolSlider:
                    # voice volume change
                    setting = control.getValue()
                    PtDebugPrint("Voice being changed to %g (into %g)" % (setting,setting/10),level=kDebugDumpLevel)
                    audio.setVoiceVolume(setting/10)
                elif plID == kBKIAmbienceVolSlider:
                    # ambience volume change
                    setting = control.getValue()
                    PtDebugPrint("Ambience being changed to %g (into %g)" % (setting,setting/10),level=kDebugDumpLevel)
                    audio.setAmbienceVolume(setting/10)
                elif plID == kBKIMicLevelSlider:
                    # microphone level change
                    setting = control.getValue()
                    PtDebugPrint("MicLevel being changed to %g (into %g)" % (setting,setting/10),level=kDebugDumpLevel)
                    audio.setMicLevel(setting/10)
                elif plID == kBKIGUIVolSlider:
                    # microphone level change
                    setting = control.getValue()
                    PtDebugPrint("MicLevel being changed to %g (into %g)" % (setting,setting/10),level=kDebugDumpLevel)
                    audio.setGUIVolume(setting/10)
                else:
                    # don't know... must be an error
                    pass
###############################################
##
##  Age Owner Setting Expanded dialog processing (this is really owner setting dialog
##
###############################################
        elif id == KIAgeOwnerExpanded.id:
            PtDebugPrint("KIAgeOwner::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kShowHide:
                if control.isEnabled():
                    # localize text fields
                    tfield = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerDescriptionTitle))
                    tfield.setStringW(PtGetLocalizedString("KI.Config.Description"))
                    # refresh the Age Owner settings
                    titleedit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleEditbox))
                    titleedit.hide()
                    self.IRefreshAgeOwnerSettings()
                else:
                    pass
            elif event == kAction or event == kValueChanged:
                plID = control.getTagID()
                if plID == kBKAgeOwnerMakePublicBtn:
                    # currently we can't make ages public or private
                    PtDebugPrint("KIAgeOwner: make public button hit",level=kDebugDumpLevel )
                    try:
                        vault = ptVault()
                        myAge = BKConfigFolderDict[BKConfigListOrder[BKFolderSelected]]
                        myAgeStruct = myAge.asAgeInfoStruct()
                        makePublic = 1
                        if myAge.isPublic():
                            makePublic = 0
                            PtDebugPrint("KIAgeOwner: making %s private"%(myAge.getDisplayName()),level=kDebugDumpLevel )
                        else:
                            PtDebugPrint("KIAgeOwner: making %s public"%(myAge.getDisplayName()),level=kDebugDumpLevel )
                        vault.setAgePublic(myAgeStruct, makePublic)
                        # let the refresh re-enable the public button
                        control.disable()
                    except AttributeError:
                        PtDebugPrint("KIAgeOwner: change public/private error",level=kErrorLevel )
                elif plID == kBKAgeOwnerTitleBtn:
                    PtDebugPrint("KIAgeOwner: change title button hit",level=kDebugDumpLevel )
                    # disable the button that was hit
                    control.disable()
                    # and enable the editbox and disable the textbox of the title
                    title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleTB))
                    title.hide()
                    titleedit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleEditbox))
                    try:
                        # get the selected age config setting
                        myAge = BKConfigFolderDict[BKConfigListOrder[BKFolderSelected]]
                        titleedit.setString(myAge.getAgeUserDefinedName())
                    except LookupError:
                        titleedit.setString("")
                    titleedit.show()
                    titleedit.end()
                    KIAgeOwnerExpanded.dialog.setFocus(titleedit.getKey())
                elif plID == kBKAgeOwnerTitleEditbox:
                    PtDebugPrint("KIAgeOwner: edit field set",level=kDebugDumpLevel )
                    self.ISaveUserNameFromEdit(control)
            elif event == kFocusChange:
                PtDebugPrint("KIAgeOwner: focus change",level=kDebugDumpLevel )
                # see if the editbox is enabled and something other than that is getting the focus
                # and not the button
                titleedit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleEditbox))
                if titleedit.isVisible():
                    if type(control) == type(None) or (control.getTagID() != kBKAgeOwnerTitleEditbox and control.getTagID() != kBKAgeOwnerTitleBtn):
                        # save editbox
                        self.ISaveUserNameFromEdit(titleedit)
                if type(control) != type(None):
                    # need to check if the decription was updated
                    plID = control.getTagID()
                    if plID == kBKAgeOwnerDescription:
                        BKAgeOwnerEditDescription = 1
                        PtDebugPrint("KIAgeOwner: start edit of description",level=kDebugDumpLevel)
                    else:
                        if BKAgeOwnerEditDescription:
                            # save
                            descript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerDescription))
                            myAge = BKConfigFolderDict[BKConfigListOrder[BKFolderSelected]]
                            if type(myAge) != type(None):
                                PtDebugPrint("KIAgeOwner: age description updated for %s"%(myAge.getDisplayName()),level=kDebugDumpLevel)
                                myAge.setAgeDescription(descript.getString())
                                myAge.save()
                            else:
                                PtDebugPrint("KIAgeOwner: neighborhood is None while trying to update description",level=kDebugDumpLevel)
                        BKAgeOwnerEditDescription = 0
                else:
                    if BKAgeOwnerEditDescription:
                        # save
                        descript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerDescription))
                        myAge = BKConfigFolderDict[BKConfigListOrder[BKFolderSelected]]
                        if type(myAge) != type(None):
                            PtDebugPrint("KIAgeOwner: age description updated for %s"%(myAge.getDisplayName()),level=kDebugDumpLevel)
                            buf = descript.getEncodedBuffer()
                            myAge.setAgeDescription(str(buf))
                            myAge.save()
                        else:
                            PtDebugPrint("KIAgeOwner: neighborhood is None while trying to update description",level=kDebugDumpLevel)
                    BKAgeOwnerEditDescription = 0

###############################################
##
##  Yes/No dialog processing
##
###############################################
        elif id == KIYesNo.id:
            if event == kAction or event == kValueChanged:
                ynID = control.getTagID()
                if YNWhatReason == kYNQuit:
                    if ynID == kYesButtonID:
                        PtConsole("app.quit")
                    elif ynID == kNoButtonID:
                        KIYesNo.dialog.hide()
                        logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoLogoutTextID))
                        logoutText.hide()
                        logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesNoLogoutButtonID))
                        logoutButton.hide()
                    elif ynID == kYesNoLogoutButtonID:
                        KIYesNo.dialog.hide()
                        logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoLogoutTextID))
                        logoutText.hide()
                        logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesNoLogoutButtonID))
                        logoutButton.hide()
                        
                        # clear out all chat on micro KI
                        chatarea = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kChatDisplayArea))
                        chatarea.setString("")
                        chatarea.moveCursor(PtGUIMultiLineDirection.kBufferStart)
                        KIMicro.dialog.refreshAllControls()
                        
                        #clear out all chat on mini KI
                        chatarea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kChatDisplayArea))
                        chatarea.setString("")
                        chatarea.moveCursor(PtGUIMultiLineDirection.kBufferStart)
                        KIMini.dialog.refreshAllControls()

                        linkmgr = ptNetLinkingMgr()
                        ageLink = ptAgeLinkStruct()
                        
                        ageInfo = ptAgeInfoStruct()
                        ageInfo.setAgeFilename("StartUp")
                        
                        spawnPoint = ptSpawnPointInfo()
                        spawnPoint.setName("LinkInPointDefault")
                        
                        ageLink.setAgeInfo(ageInfo)
                        ageLink.setSpawnPoint(spawnPoint)
                        ageLink.setLinkingRules(PtLinkingRules.kBasicLink)
                        linkmgr.linkToAge(ageLink)
                        
                elif YNWhatReason == kYNDelete:
                    if ynID == kYesButtonID:
                        # remove the current element
                        if type(BKCurrentContent) != type(None):
                            del_folder = BKCurrentContent.getParent()
                            del_elem = BKCurrentContent.getChild()
                            if type(del_folder) != type(None) and type(del_elem) != type(None):
                                # are we removing a visitor from an age we own?
                                tfolder = del_folder.upcastToFolderNode()
                                if type(tfolder) != type(None) and tfolder.folderGetType() == PtVaultStandardNodes.kCanVisitFolder:
                                    PtDebugPrint("xKI:del: revoking visitor",level=kDebugDumpLevel)
                                    del_elem = del_elem.upcastToPlayerInfoNode()
                                    # need to refind the folder that has the ageInfo in it
                                    agefoldername = BKFolderListOrder[BKFolderSelected]
                                    agefolder = BKFolderLineDict[agefoldername]
                                    self.IRevokeToVisit(del_elem.playerGetID(),agefolder)
                                # or are we removing a player from a player list?
                                elif del_folder.getType()==PtVaultNodeTypes.kPlayerInfoListNode and del_elem.getType()==PtVaultNodeTypes.kPlayerInfoNode:
                                    # yes... see if it is one of the special neighborhood player list
                                    # ... how about visitors?
                                    PtDebugPrint("xKI:del: removing player from folder",level=kDebugDumpLevel)
                                    del_folder = del_folder.upcastToPlayerInfoListNode()
                                    del_elem = del_elem.upcastToPlayerInfoNode()
                                    # then use the player list remove playerID
                                    del_folder.playerlistRemovePlayer(del_elem.playerGetID())
                                    # remove the selected player... probably is this one!
                                    BKPlayerSelected = None
                                    sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                                    sendToField.setString(" ")
                                # or are we just removing a journal entry
                                else:
                                    # see if this is a marker folder game that is being deleted
                                    if del_elem.getType() == PtVaultNodeTypes.kMarkerGameNode:
                                        # Delete all markers from the Marker Manager Display
                                        mrkrDisplay = ptMarkerMgr()
                                        if not self.markerGameManager.gameLoaded():
                                            mrkrDisplay.removeAllMarkers()
                                        #Delete the game!
                                        if self.markerGameDisplay == None:
                                            PtDebugPrint("ERROR: xKI.OnGUINotify(delete):\tCannot delete marker game as it is not loaded!")
                                            return
                                        self.markerGameDisplay.deleteGame()
                                        #if we were playing the game we must make sure it's reset as well!
                                        if self.markerGameDisplay.gameData.data['svrGameTemplateID'] == self.markerGameManager.gameData.data['svrGameTemplateID']:
                                            self.markerGameManager.deleteGame()


                                    BKCurrentContent = None
                                    del_folder.removeNode(del_elem)
                                    PtDebugPrint("xKI:del: deleting element from folder",level=kDebugDumpLevel)
                            else:
                                PtDebugPrint("xKI: tried to delete bad vaultnode or delete from bad folder",level=kErrorLevel)
                            # pick what to display
                            self.IBigKIChangeMode(kBKListMode)
                            # they could have removed a player... refresh list
                            self.IRefreshPlayerList()
                            self.IRefreshPlayerListDisplay()
                    elif ynID == kNoButtonID:
                        pass                    
                    YNWhatReason = kYNQuit
                    KIYesNo.dialog.hide()
                elif YNWhatReason == kYNOfferLink:
                    YNWhatReason = kYNQuit
                    KIYesNo.dialog.hide()
                    if ynID == kYesButtonID:
                        if type(OfferLinkFromWho) != type(None):
                            # we're linking to .... their personal age
                            PtDebugPrint("xKI: Linking to offered age %s" % (OfferLinkFromWho.getDisplayName()),level=kDebugDumpLevel)
                            link = ptAgeLinkStruct()
                            link.setLinkingRules(PtLinkingRules.kBasicLink)
                            link.setAgeInfo(OfferLinkFromWho)
                            ptNetLinkingMgr().linkToAge(link)
                            OfferLinkFromWho = None
                            # bye-bye
                    elif ynID == kNoButtonID:
                        pass
                    OfferLinkFromWho = None
                elif YNWhatReason == kYNOutside:
                    YNWhatReason = kYNQuit
                    KIYesNo.dialog.hide()
                    if type(YNOutsideSender) != type(None):
                        # start building the notify message to go back to the orignator
                        note = ptNotify(self.key)
                        note.clearReceivers()
                        note.addReceiver(YNOutsideSender)
                        note.netPropagate(0)
                        note.netForce(0)
                        if ynID == kYesButtonID:
                            # good return
                            note.setActivate(1)
                            note.addVarNumber("YesNo",1)
                        elif ynID == kNoButtonID:
                            # bad return
                            note.setActivate(0)
                            note.addVarNumber("YesNo",0)
                        note.send()
                    YNOutsideSender = None
                elif YNWhatReason == kYNKIFull:
                    KIYesNo.dialog.hide()
                    # put back what was changed
                    yesButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesButtonID))
                    yesButton.show()
                    yesBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesButtonTextID))
                    yesBtnText.show()                    
                    noBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kNoButtonTextID))
                    noBtnText.setStringW(PtGetLocalizedString("KI.YesNoDialog.NOButton"))
                    YNWhatReason = kYNQuit
                else:
                    # make sure that it get hidden... even if we didn't know the reason why its up
                    YNWhatReason = kYNQuit
                    KIYesNo.dialog.hide()
                    YNOutsideSender = None
            elif event == kExitMode:
                # make sure that it get hidden... even if we didn't know the reason why its up
                YNWhatReason = kYNQuit
                KIYesNo.dialog.hide()
                YNOutsideSender = None
###############################################
##
##  Rate It dialog processing
##
###############################################
        elif id == KIRateIt.id:
            if event == kAction or event == kValueChanged:
                ynID = control.getTagID()
                # do <whatever!>
###############################################
##
##  New Item Alert processing
##
###############################################
        elif id == NewItemAlert.id:
            if event == kDialogLoaded:
                kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                kialert.disable()
                kialert.hide()
                bookalert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
                bookalert.disable()
                bookalert.hide()
                pass
            elif event == kShowHide:
                if control.isEnabled():
                    # showing... start the timer to hide it later
                    self.IAlertStartTimer()
                else:
                    # hiding... nothing really to do here
                    pass
###############################################
##  Create Marker Game Dialog
##
###############################################
        elif id == KICreateMarkerGameGUI.id:
            if control:
                tagID = control.getTagID()
            if event == kDialogLoaded:
                kMarkerGameDefaultColor = ptGUIControlTextBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kMarkerGameLabel1)).getForeColor()
                kMarkerGameSelectedColor = ptGUIControlTextBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kMarkerGameLabel1)).getSelectColor()
                print "xKI: Marker Default Color: ", kMarkerGameDefaultColor
                print "xKI: Marker Selected Color: ", kMarkerGameSelectedColor
                print "xKI: Marker Game Dialog Loaded"
            elif event == kShowHide:
                self.initMarkerGameGUI(KICreateMarkerGameGUI)
                print "xKI: Marker Game Dialog is showing or hiding."
            elif event == kAction or event == kValueChanged:
                if tagID == kMarkerGameType1 or tagID == kMarkerGameType2 or tagID == kMarkerGameType3:
                    self.SelectMarkerType(KICreateMarkerGameGUI, tagID)
                elif tagID == kCreateMarkerGameCancelBT:
                    KIMarkerGameGUIClose.run(self.key,netPropagate=0)
                elif kCreateMarkerGameSubmitBT:
                    markerGameNameText = ptGUIControlEditBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kCreateMarkerGameNameEB)).getString()
                    markerGameName = xCensor.xCensor(markerGameNameText,xCensor.xRatedPG)
                    try:
                        markerGameType = kMarkerGameStates[kSelectedMGType]
                    except:
                        markerGameType = 0
                        print "xKI: Couldn't find marker game type, so setting it to Quest Mode."
                    #Create the marker game display...
                    #We will await a return KI message and then, once finished, will call IFinishCreateMarkerFolder()
                    self.markerGameDisplay = xMarkerGameKIDisplay(self, "", markerGameType, markerGameName)
                    KIMarkerGameGUIClose.run(self.key,netPropagate=0)
###############################################
##
##  Marker Folder expanded dialog processing
##
###############################################
        elif id == KIMarkerFolderExpanded.id:
            #we do this just as a safety precaution, prolly not necessary...
            self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionLoading")

            if event == kDialogLoaded:
                # set the time field pull down
                typeField = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTimeTB))
                typeField.setString(kMarkerFolderPopupMenu[MarkerGameTimeID][0])
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    titleedit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleEB))
                    titleedit.hide()
                    markeredit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextEB))
                    markeredit.hide()
                    self.IBigKIDisplayCurrentContentMarkerFolder()
                else:
                    if type(self.markerGameDisplay) != type(None) and type(self.markerGameDisplay.gameData) != type(None):
                        #We may still be adding markers, so check to see if we're in edit mode before destroying the loaded marker game!
                        if not self.markerGameDisplay.showMarkers:
                            self.markerGameDisplay = None
            elif event == kAction or event == kValueChanged:
                mfldrID = control.getTagID()
                if mfldrID == kMarkerFolderEditStartGame:
                    PtDebugPrint("xKI:Marker: Hit the MF Edit/Start game button",level=kDebugDumpLevel)
                    if MFdialogMode == kMFOverview:
                        #------------------------------#
                        # this is the Edit game button #
                        #------------------------------#
                        PtDebugPrint("xKI:Marker: must be edit game button",level=kDebugDumpLevel)
                        if self.markerGameDisplay == None:
                            PtDebugPrint("ERROR: xKI.OnGuiNotify(kMarkerFolderEditStartGame):\tCannot locate the game, aborting edit game request")
                            return
                        self.IShowMarkerGameLoading()
                        self.markerGameDisplay.editMarkers()
                        self.ISetWorkingToCurrentMarkerFolder()                        
                    elif MFdialogMode == kMFEditing:
                        #---------------------------------#                        
                        # this is the Done Editing button #
                        #---------------------------------#
                        PtDebugPrint("xKI:Marker: must be done editing button",level=kDebugDumpLevel)
                        self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionSaving")
                        self.IShowMarkerGameLoading()
                        self.markerGameDisplay.exitEditMarkers()
                        self.IResetWorkingMarkerFolder()                        
                        #ptMarkerMgr().hideMarkersLocal()
                    elif MFdialogMode == kMFPlaying:
                        #----------------------------------------------------------#
                        # this is the stop game button and start button in capture #
                        #----------------------------------------------------------#
                        PtDebugPrint("xKI:Marker: must be start game button",level=kDebugDumpLevel)
                        if self.markerGameManager.gameLoaded():
                            #display a waiting screen...
                            self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionEndingGame")
                            self.IShowMarkerGameLoading()
                            #Stop the game...
                            self.markerGameManager.stopGame()

                        """
                        #TODO: This is left for historic reasons, please delete (i.e. when multiplayer markers get implemented)                        
                        workingMF = ptMarkerMgr().getWorkingMarkerFolder()
                        if type(workingMF) != type(None):
                            if workingMF.getGameType() == PtMarkerMsgGameType.kGameTypeQuest:
                                ptMarkerMgr().endGame()
                            else:
                                ptMarkerMgr().startGame()
                                # hide the bigKI so the master can play!
                                self.IminiToggleKISize()
                        """
                    elif MFdialogMode == kMFEditingMarker:
                        #--------------------#
                        # Save marker button #
                        #--------------------#
                        # Should already be saved, just clear selection for now
                        self.markerGameDisplay.setSelectedMarker(-1)
                        self.IBKCheckContentRefresh(BKCurrentContent)
                elif mfldrID == kMarkerFolderPlayEndGame:
                    PtDebugPrint("xKI:Marker: Hit the MF Play/End game button",level=kDebugDumpLevel)
                    if MFdialogMode == kMFOverview:
                        #---------------------------------------------------------#
                        # this is the Play game button (create a game in Capture) #
                        #---------------------------------------------------------#
                        PtDebugPrint("xKI:Marker: must be play game button",level=kDebugDumpLevel)
                        self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionPrepareGame")
                        self.IShowMarkerGameLoading()
                        self.markerGameManager = MarkerGameManager(self, self.markerGameDisplay)

                        """
                        #TODO: This is left for historic reasons, please delete (i.e. when multiplayer markers get implemented)
                        element = self.ISetWorkingToCurrentMarkerFolder()
                        if element:
                            # create the actual game
                            ptMarkerMgr().createGame(element.getRoundLength(),element.getGameType(),[])
                            self.IRefreshPlayerList()
                            self.IRefreshPlayerListDisplay()
                        """
                    elif MFdialogMode == kMFEditing:
                        #-------------------------------#
                        # this is the Add marker button #
                        #-------------------------------#
                        PtDebugPrint("xKI:Marker: must be add marker button",level=kDebugDumpLevel)
                        self.ICreateAMarker()
                    elif MFdialogMode == kMFPlaying:
                        #-----------------------------#
                        # this is the End Game button #
                        #-----------------------------#
                        PtDebugPrint("xKI:Marker: must be end game button",level=kDebugDumpLevel)
                        #display a waiting screen...
                        self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionResetGame")
                        self.IShowMarkerGameLoading()
                        self.markerGameManager.resetGame()

                        """
                        workingMF = ptMarkerMgr().getWorkingMarkerFolder()
                        if type(workingMF) != type(None):
                            if workingMF.getGameType() == PtMarkerMsgGameType.kGameTypeQuest:
                                # end the current game
                                ptMarkerMgr().endGame()
                                # reset all the seen flags in the game
                                markerRefs = workingMF.getChildNodeRefList()
                                for markerRef in markerRefs:
                                    markerRef.unsetSeen()
                                # restart the game
                                element = self.ISetWorkingToCurrentMarkerFolder()
                                if element:
                                    # create the actual game
                                    ptMarkerMgr().createGame(element.getRoundLength(),element.getGameType(),[])
                                self.IBKCheckContentRefresh(BKCurrentContent)
                            else:
                                self.IDoStatusChatMessage(PtGetLocalizedString("KI.MarkerGame.PrematureEnding"),netPropagate=1)
                                ptMarkerMgr().endGame()
                        """
                    elif MFdialogMode == kMFEditingMarker:
                        #---------------#
                        # remove marker #
                        #---------------#
                        self.markerGameDisplay.deleteSelectedMarker()

                elif mfldrID == kMarkerFolderMarkListbox:
                    if self.markerGameDisplay != None:
                        markerlistSelectable = 1
                        if self.markerGameDisplay.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameQuest:
                            # if we are playing, then the marker list is not selectable
                            if self.markerGameManager.gameData.data['svrGameTemplateID'] == self.markerGameDisplay.gameData.data['svrGameTemplateID']:
                                markerlistSelectable = 0
                        if markerlistSelectable:
                            markerSel = control.getSelection()
                            self.markerGameDisplay.setSelectedMarker(markerSel)
                            self.IBKCheckContentRefresh(BKCurrentContent)
                        
                elif mfldrID == kMarkerFolderInvitePlayer:
                    PtDebugPrint("xKI:Marker: inviting a player",level=kDebugDumpLevel)
                    if type(WorkingMarkerFolder) != type(None):
                        userlbx = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
                        iselect = userlbx.getSelection()
                        if iselect >= 0 and iselect < len(BKPlayerList):
                            PtDebugPrint("xKI:Marker: valid player selection",level=kDebugDumpLevel)
                            toplyr = BKPlayerList[iselect]
                            if isinstance(toplyr, ptPlayer):
                                # make sure that its not a repeat
                                newplayer = 1
                                for mplayer in WorkingMarkerFolder.invitedPlayers:
                                    if mplayer.ID == toplyr.getPlayerID():
                                        newplayer = 0
                                        break
                                if newplayer:
                                    PtDebugPrint("xKI:Marker: inviting player %s [%d] to the game" %(toplyr.getPlayerName(),toplyr.getPlayerID()),level=kDebugDumpLevel)
                                    WorkingMarkerFolder.invitedPlayers.append(MarkerPlayer(toplyr.getPlayerID()))
                                    ptMarkerMgr().invitePlayers([long(toplyr.getPlayerID())])
                                    self.IRefreshPlayerList()
                                    self.IRefreshPlayerListDisplay()
                            elif isinstance(toplyr, ptVaultPlayerInfoListNode):
                                PtDebugPrint("xKI:Marker: playerinfo list",level=kDebugDumpLevel)
                                # if this is Age Players...
                                plistnode = toplyr.upcastToPlayerInfoListNode()
                                ageplayers = self.IRemoveCCRPlayers(PtGetPlayerListDistanceSorted())
                                inviting = []
                                if type(plistnode) == type(None):
                                    PtDebugPrint("xKI:InvitePlayers: folder is AgePlayers",level=kDebugDumpLevel)
                                    # then tell the game manager to invite everyone in the age
                                    for aplayer in ageplayers:
                                        inviting.append(long(aplayer.getPlayerID()))
                                        WorkingMarkerFolder.invitedPlayers.append(MarkerPlayer(aplayer.getPlayerID()))
                                # look for buddies or neighbors
                                elif plistnode.folderGetType() == PtVaultStandardNodes.kBuddyListFolder:
                                    PtDebugPrint("xKI:InvitePlayers: folder is BuddyList",level=kDebugDumpLevel)
                                    # search through all the buddies, find ones in this age
                                    buddies = ptVault().getBuddyListFolder()
                                    budlist = buddies.getChildNodeRefList()
                                    for bud in budlist:
                                        buddy = bud.getChild().upcastToPlayerInfoNode()
                                        for aplayer in ageplayers:
                                            if aplayer.getPlayerID() == buddy.playerGetID():
                                                inviting.append(long(aplayer.getPlayerID()))
                                                WorkingMarkerFolder.invitedPlayers.append(MarkerPlayer(aplayer.getPlayerID()))
                                elif plistnode.folderGetType() == PtVaultStandardNodes.kAgeOwnersFolder:
                                    PtDebugPrint("xKI:InvitePlayers: folder is Neighbors",level=kDebugDumpLevel)
                                    # search through all the neighbors, find ones in this age
                                    neighbors = self.IGetNeighbors()
                                    if type(neighbors) != type(None):
                                        neighborlist = neighbors.getChildNodeRefList()
                                        for neighbor in neighborlist:
                                            hoody = neighbor.getChild().upcastToPlayerInfoNode()
                                            for aplayer in ageplayers:
                                                if aplayer.getPlayerID() == hoody.playerGetID():
                                                    inviting.append(long(aplayer.getPlayerID()))
                                                    WorkingMarkerFolder.invitedPlayers.append(MarkerPlayer(aplayer.getPlayerID()))
                                # if any players were found to invite.. then invite them
                                if len(inviting) > 0:
                                    ptMarkerMgr().invitePlayers(inviting)
                                self.IRefreshPlayerList()
                                self.IRefreshPlayerListDisplay()
                            elif isinstance(toplyr,kiFolder):
                                ageplayers = self.IRemoveCCRPlayers(PtGetPlayerListDistanceSorted())
                                inviting = []
                                PtDebugPrint("xKI:InvitePlayers: folder is AgePlayers",level=kDebugDumpLevel)
                                # then tell the game manager to invite everyone in the age
                                for aplayer in ageplayers:
                                    inviting.append(long(aplayer.getPlayerID()))
                                    WorkingMarkerFolder.invitedPlayers.append(MarkerPlayer(aplayer.getPlayerID()))
                                # if any players were found to invite.. then invite them
                                if len(inviting) > 0:
                                    ptMarkerMgr().invitePlayers(inviting)
                                self.IRefreshPlayerList()
                                self.IRefreshPlayerListDisplay()
                            else:
                                PtDebugPrint("xKI:Marker: unknown class type",level=kDebugDumpLevel)
                elif mfldrID == kMarkerFolderTitleBtn:
                    PtDebugPrint("KIMarkerFolder: change title button hit",level=kDebugDumpLevel )
                    # disable the button that was hit
                    control.disable()
                    # and enable the editbox and disable the textbox of the title
                    title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleText))
                    titleedit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleEB))
                    titleedit.setString(title.getString())
                    title.hide()
                    titleedit.show()
                    titleedit.end()
                    KIMarkerFolderExpanded.dialog.setFocus(titleedit.getKey())
                elif mfldrID == kMarkerFolderTitleEB:
                    PtDebugPrint("KIMarkerFolder: edit field set",level=kDebugDumpLevel )
                    self.ISaveMarkerFolderNameFromEdit(control)
                elif mfldrID == kMarkerFolderMarkerTextBtn:
                    PtDebugPrint("KIMarkerFolder: change marker text button hit",level=kDebugDumpLevel )
                    # disable the button that was hit
                    control.disable()
                    # and enable the editbox and disable the textbox of the title
                    title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextTB))
                    titleedit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextEB))
                    titleedit.setString(title.getString())
                    title.hide()
                    titleedit.show()
                    titleedit.end()
                    KIMarkerFolderExpanded.dialog.setFocus(titleedit.getKey())
                elif mfldrID == kMarkerFolderMarkerTextEB:
                    PtDebugPrint("KIMarkerFolder: edit field set",level=kDebugDumpLevel )
                    self.pendingMGmessage = kMessageWait.changeMarkerName
                    self.ISaveMarkerTextFromEdit(control)
                elif mfldrID == kMarkerFolderTimePullDownBtn or mfldrID == kMarkerFolderTimeArrow:
                    KIMarkerFolderPopupMenu.menu.show()
                elif mfldrID == kMarkerFolderTypePullDownBtn or mfldrID == kMarkerFolderTypeArrow:
                    KIMarkerTypePopupMenu.menu.show()
                elif mfldrID == kMarkerFolderDeleteBtn:
                    YNWhatReason = kYNDelete
                    elem = BKCurrentContent.getChild()
                    elem = elem.upcastToMarkerGameNode()
                    if type(elem) != type(None):
                        mftitle = elem.getGameName()
                    else:
                        mftitle = "<unknown>"
                    yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
                    yesText.setStringW(PtGetLocalizedString("KI.Messages.DeletePicture", [xCensor.xCensor(mftitle,theCensorLevel)]))
                    self.ILocalizeYesNoDialog()
                    KIYesNo.dialog.show()
            elif event == kFocusChange:
                PtDebugPrint("KIMarkerFolder: focus change",level=kDebugDumpLevel )
                # see if the editbox is enabled and something other than that is getting the focus
                # and not the button
                titleedit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleEB))
                if titleedit.isVisible():
                    if type(control) == type(None) or (control.getTagID() != kMarkerFolderTitleEB and control.getTagID() != kMarkerFolderTitleBtn):
                        # save editbox
                        self.ISaveMarkerFolderNameFromEdit(titleedit)
                # see if the editbox is enabled and something other than that is getting the focus
                # and not the button
                if MFdialogMode == kMFEditingMarker:
                    titleedit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextEB))
                    if titleedit.isVisible():
                        if type(control) == type(None) or (control.getTagID() != kMarkerFolderMarkerTextEB and control.getTagID() != kMarkerFolderMarkerTextBtn):
                            # save editbox
                            self.ISaveMarkerTextFromEdit(titleedit)
        elif id == KIMarkerFolderPopupMenu.id:
            if event == kDialogLoaded:
                # load the popup memu with items
                for menuItem in kMarkerFolderPopupMenu:
                    KIMarkerFolderPopupMenu.menu.addNotifyItem(menuItem[0])
            elif event == kAction:
                menuID = control.getTagID()
                # set the type field
                MarkerGameTimeID = menuID
                typeField = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTimeTB))
                typeField.setString(kMarkerFolderPopupMenu[MarkerGameTimeID][0])
                # save the current marker folder to this type of game
                #TODO -- Need to set things through the xMarkerGameManager...  prolly need another callback for the KI update.
                #        We'll wait until we actually implement multiplayer marker games
                if type(BKCurrentContent) != type(None):
                    element = BKCurrentContent.getChild()
                    if type(element) != type(None):
                        datatype = element.getType()
                        if datatype == PtVaultNodeTypes.kMarkerGameNode:
                            element = element.upcastToMarkerGameNode()
                            if element:
                                element.setRoundLength(kMarkerFolderPopupMenu[MarkerGameTimeID][1])
                                element.save()
            elif event == kExitMode:
                if KIMarkerFolderPopupMenu.menu.isEnabled():
                    KIMarkerFolderPopupMenu.menu.hide()
                elif KIMarkerTypePopupMenu.menu.isEnabled():
                    KIMarkerTypePopupMenu.menu.hide()
        elif id == KIMarkerTypePopupMenu.id:
            #These should never happen as you can no longer change game types on the fly, this is a server limitation.
            #Althogh this has been left in here for historic reasons and may one day be functional....
            if event == kDialogLoaded:
                # load the popup memu with items
                #KIMarkerTypePopupMenu.menu.addNotifyItemW(PtGetLocalizedString("KI.MarkerGame.CaptureGame"))
                #KIMarkerTypePopupMenu.menu.addNotifyItemW(PtGetLocalizedString("KI.MarkerGame.HoldGame"))
                KIMarkerTypePopupMenu.menu.addNotifyItemW(PtGetLocalizedString("KI.MarkerGame.QuestGame"))
            elif event == kAction:
                pass

            elif event == kExitMode:
                if KIMarkerFolderPopupMenu.menu.isEnabled():
                    KIMarkerFolderPopupMenu.menu.hide()
                elif KIMarkerTypePopupMenu.menu.isEnabled():
                    KIMarkerTypePopupMenu.menu.hide()
###############################################
##
##  Question Note (Text) dialog processing
##
###############################################
        elif id == KIQuestionNote.id:
            PtDebugPrint("KIQuestion::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel )
            if event == kShowHide:
                if control.isEnabled():
                    # refresh the display of the currently selected content element
                    self.IBigKIDisplayCurrentQuestionNote()
            elif event == kAction or event == kValueChanged:
                qnID = control.getTagID()
                # see if it is one of the buttons
                if qnID == kQNAcceptBtn:
                    try:
                        BKCurrentContent.YesAction()
                        self.IBKCheckFolderRefresh(ptVault().getInbox())
                        # pick what to display
                        self.IBigKIChangeMode(kBKListMode)
                    except AttributeError:
                        pass
                elif qnID == kQNDeclineBtn:
                    try:
                        BKCurrentContent.NoAction()
                        self.IBKCheckFolderRefresh(ptVault().getInbox())
                        # pick what to display
                        self.IBigKIChangeMode(kBKListMode)
                    except AttributeError:
                        pass
            else:
                pass

        elif id == KIJalakGUIDialog.id:
            #PtDebugPrint("KIJalakGUIDialog.OnGUINotify(): id=%d, event=%d control=" % (id,event),control)
            if event == kDialogLoaded:
                self.JalakGUIInit()
            elif event == kShowHide:
                if control.isEnabled():
                    print "show/hide"
                    #control.show()
            elif event == kAction or event == kValueChanged:
                if type(control) != type(None):
                    tagID = control.getTagID()
                    btn = str(tagID)
                    if btn in JalakBtnStates:
                        KIJalakBtnLights.run(self.key,state=btn,netPropagate=0)
                        self.SetJalakGUIButtons(0)


    def SetJalakGUIButtons(self,state):
        for btn in jlakGUIButtons:
            if state:
                btn.enable()
            else:
                btn.disable()


    def SendJalakBtnHit(self,btnID):
        if btnID == kJalakRandomBtn:
            self.SendNote('self.AutoColumns(%d)' % (3))
        elif btnID == kJalakExtremeBtn:
            self.SendNote('self.AutoColumns(%d)' % (4))
        elif btnID == kJalakWallToggleBtn:
            self.SendNote('self.ToggleWall()')
        elif btnID == kJalakColumnsLowBtn:
            self.SendNote('self.AutoColumns(%d)' % (0))
        elif btnID == kJalakColumnsMedBtn:
            self.SendNote('self.AutoColumns(%d)' % (1))
        elif btnID == kJalakColumnsHighBtn:
            self.SendNote('self.AutoColumns(%d)' % (2))
        elif btnID == kJalakRampBtn:
            self.SendNote('self.DropWidget("%s")' % (kRamp))
        elif btnID == kJalakSphereBtn:
            self.SendNote('self.DropWidget("%s")' % (kSphere))
        elif btnID == kJalakBigBoxBtn:
            self.SendNote('self.DropWidget("%s")' % (kBigBox))
        elif btnID == kJalakLilBoxBtn:
            self.SendNote('self.DropWidget("%s")' % (kLilBox))
        elif btnID == kJalakRectangleBtn:
            self.SendNote('self.DropWidget("%s")' % (kRect))
        elif btnID == kJalakDestroyBtn:
            self.SendNote('self.ResetWidgets()')


    def SendNote(self,ExtraInfo):
        note = ptNotify(self.key)
        note.clearReceivers()                
        note.addReceiver(JalakScript)
        note.netPropagate(0)
        note.netForce(0)
        note.setActivate(1.0)
        note.addVarNumber(str(ExtraInfo),1.0)
        note.send()


    def OnKIMsg(self,command,value):
        "On receipt of a KI message"
        global TicksOnFull
        global IAmAdmin
        global IKIDisabled
        global IKIHardDisabled
        global IminiKIWasUp
        global YNWhatReason
        global YNOutsideSender
        global FolderOfDevices
        global theKILevel
        global FadeMode
        global FadeEnableFlag
        global PrivateChatChannel
        global PhasedKICreateNotes
        global PhasedKICreateImages
        global PhasedKIShareYeeshaBook
        global PhasedKIInterAgeChat
        global PhasedKINeighborsInDPL
        global PhasedKIBuddies
        global PhasedKIPlayMarkerGame
        global PhasedKICreateMarkerGame
        global PhasedKISendNotes
        global PhasedKISendImages
        global PhasedKISendMarkerGame
        global PhasedKIShowMarkerGame
        global IsYeeshaBookEnabled
        global IsEntireYeeshaBookEnabled
        global gGZMarkerInRange
        global gGZMarkerInRangeRepy
        global gMarkerToGetColor
        global gMarkerGottenColor
        global gMarkerToGetNumber
        global gMarkerGottenNumber
        global kPelletImager

        PtDebugPrint("xKI: KIMsg: command = %d value ="%(command),value,level=kDebugDumpLevel)
        if command == kEnterChatMode:
            #~ if not IKIDisabled and not BigKI.dialog.isEnabled():
            if not IKIDisabled and not PtIsSinglePlayerMode():
                self.IEnterChatMode(1)
        elif command == kSetChatFadeDelay:
            TicksOnFull = value
        elif command == kSetTextChatAdminMode:
            IAmAdmin = value
        elif command == kDisableKIandBB:
            PtDebugPrint("xKI: Disable KI",level=kDebugDumpLevel)
            IKIDisabled = 1
            IKIHardDisabled = 1
            if theKILevel == kNanoKI:
                KINanoBlackBar.dialog.hide()
            elif PtIsSinglePlayerMode():
                KIMicroBlackbar.dialog.hide()
            elif theKILevel == kMicroKI:
                KIMicroBlackbar.dialog.hide()
                if KIMicro.dialog.isEnabled():
                    IminiKIWasUp = 1
                    KIMicro.dialog.hide()
                else:
                    IminiKIWasUp = 0
            else:
                KIBlackbar.dialog.hide()
                if KIMini.dialog.isEnabled():
                    IminiKIWasUp = 1
                    KIMini.dialog.hide()
                else:
                    IminiKIWasUp = 0
                if not WaitingForAnimation:
                    # hide all dialogs that are on the right side of the bigKI
                    KIListModeDialog.dialog.hide()
                    KIPictureExpanded.dialog.hide()
                    KIJournalExpanded.dialog.hide()
                    KIPlayerExpanded.dialog.hide()
                    # then hide the bigKI
                    BigKI.dialog.hide()
                    # set animation back to dot on screen
                    KIOnAnim.animation.skipToTime(1.5)
            # if an outsider has a Yes/No up, then tell them No!
            if YNWhatReason == kYNOutside:
                if type(YNOutsideSender) != type(None):
                    # start building the notify message to go back to the orignator
                    note = ptNotify(self.key)
                    note.clearReceivers()
                    note.addReceiver(YNOutsideSender)
                    note.netPropagate(0)
                    note.netForce(0)
                    # bad return
                    note.setActivate(0)
                    note.addVarNumber("YesNo",0)
                    note.send()
                YNOutsideSender = None
            # hide the playerbook and reset the checkbox for playerbook on the blackbar
            if YeeshaBook:
                YeeshaBook.hide()
            PtToggleAvatarClickability(true)
            plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
            plybkCB.setChecked(0)
            YNWhatReason = kYNQuit
            KIYesNo.dialog.hide()
        elif command == kEnableKIandBB:
            PtDebugPrint("xKI: Enable KI",level=kDebugDumpLevel)
            IKIDisabled = 0
            IKIHardDisabled = 0
            if theKILevel == kNanoKI:
                KINanoBlackBar.dialog.show()
            elif PtIsSinglePlayerMode():
                KIMicroBlackbar.dialog.show()
            elif theKILevel == kMicroKI:
                KIMicroBlackbar.dialog.show()
                if IminiKIWasUp:
                    KIMicro.dialog.show()
            else:
                KIBlackbar.dialog.show()
                if IminiKIWasUp:
                    self.IClearBBMini(0)
                    KIMini.dialog.show()
        # these are the temp disables from the avatar system
        # and only hide the black bars
        elif command == kTempDisableKIandBB:
            PtDebugPrint("xKI: TEMP Disable KI",level=kDebugDumpLevel)
            IKIDisabled = 1
            if theKILevel == kNanoKI:
                KINanoBlackBar.dialog.hide()
            elif theKILevel == kMicroKI or PtIsSinglePlayerMode():
                KIMicroBlackbar.dialog.hide()
            else:
                KIBlackbar.dialog.hide()
            # hide the playerbook and reset the checkbox for playerbook on the blackbar
            if YeeshaBook:
                YeeshaBook.hide()
            PtToggleAvatarClickability(true)
            plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
            plybkCB.setChecked(0)
        elif command == kTempEnableKIandBB:
            PtDebugPrint("xKI: TEMP Enable KI",level=kDebugDumpLevel)
            if not IKIHardDisabled:
                IKIDisabled = 0
                if theKILevel == kNanoKI:
                    KINanoBlackBar.dialog.showNoReset()
                elif theKILevel == kMicroKI or PtIsSinglePlayerMode():
                    KIMicroBlackbar.dialog.showNoReset()
                else:
                    KIBlackbar.dialog.showNoReset()
        elif command == kYesNoDialog:
            YNWhatReason = kYNOutside
            YNOutsideSender = value[1]
            yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
            yesText.setStringW(value[0])
            self.ILocalizeYesNoDialog()
            KIYesNo.dialog.show()
        elif command == kAddPlayerDevice:
            PtDebugPrint("KI: add device %s to player list" % (value),level=kDebugDumpLevel)
            print "Value:", value
            if value.find("<p>") != -1:
                kPelletImager = value.rstrip("<p>")
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kPelletScoreButton)).show()
                print "kPelletImager", kPelletImager
                return
        # KI shortcut commands
            try:
                FolderOfDevices.index(Device(value))
                #~ PtDebugPrint("xBigKI: Device %s already in list, don't need to do anything" % (value))
            except ValueError:
                FolderOfDevices.append(Device(value))
                self.IRefreshPlayerList()
                self.IRefreshPlayerListDisplay()
        elif command == kRemovePlayerDevice:
            if value.find("<p>") != -1:
                kPelletImager = ""
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kPelletScoreButton)).hide()
                return
        # KI shortcut commands            
            #~ print "KI: remove device %s from player list" % (value)
            try:
                FolderOfDevices.remove(Device(value))
            except ValueError:
                #~ PtDebugPrint("xBigKI: removing device %s when it was not there" % (value))
                pass
            self.IRefreshPlayerList()
            self.IRefreshPlayerListDisplay()
        elif command == kUpgradeKILevel:
            if value >= kLowestKILevel and value <= kHighestKILevel:
                if value > theKILevel:
                    PtDebugPrint("xKI: Upgrading from KIlevel %d to new KI level of %d" % (theKILevel,value),level=kWarningLevel)
                    # we're upgradin'!
                    # first find out what to turn off
                    self.IRemoveKILevel(theKILevel,upgrading=1)
                    # set it in the chronicle
                    theKILevel = value
                    self.IUpdateKILevelChronicle()
                    # next bring up the new KI level
                    self.IWearKILevel(theKILevel)
                else:
                    PtDebugPrint("xKI: Ignoring - trying to upgrading from KIlevel %d to new KI level of %d" % (theKILevel,value),level=kWarningLevel)
                    # make sure they are wearing what they should be (as far as PersonalBook and KI
                    self.IMakeSureWeWereKILevel()
                    pass
            else:
                PtDebugPrint("xKI: Invalid KI level %d" % (value),level=kErrorLevel)
                pass
        elif command == kDowngradeKILevel:
            if value == theKILevel:
                PtDebugPrint("xKI: Remove KI level of %d" % (value),level=kWarningLevel)
                # we're downgrading
                if value == kMicroKI:
                    # goin' from micro back to nano... then
                    # unwear the micro
                    self.IRemoveKILevel(kMicroKI)
                    # set it in the chronicle
                    theKILevel = kNanoKI
                    self.IUpdateKILevelChronicle()
                    # wear the nano
                    self.IWearKILevel(theKILevel)
                elif value == kNormalKI:
                    # goin' from normal back to micro... then
                    # unwear the new KI
                    self.IRemoveKILevel(kNormalKI)
                    # set it in the chronicle
                    theKILevel = kMicroKI
                    self.IUpdateKILevelChronicle()
                    # show the micro
                    self.IWearKILevel(theKILevel)
                else:
                    PtDebugPrint("xKI: Ignoring - can't remove to any lower than %d" % (value),level=kWarningLevel)
            else:
                PtDebugPrint("xKI: Ignoring - trying to remove KILevel %d, but currently at %d" % (value,theKILevel),level=kWarningLevel)
        elif command == kRateIt:
            # value = (chronicleName,message,onceFlag)
            # just set the text and show the dialog for now...
            rateText = ptGUIControlTextBox(KIRateIt.dialog.getControlFromTag(kYesNoTextID))
            rateText.setStringW(value[1])
            KIRateIt.dialog.show()
        elif command == kSetPrivateChatChannel:
            PrivateChatChannel = value
        elif command == kUnsetPrivateChatChannel:
            PrivateChatChannel = 0
        elif command == kStartBookAlert:
            self.IAlertBookStart()
        elif command == kStartKIAlert:
            self.IAlertKIStart()
        elif command == kUpdatePelletScore:
            self.IUpdatePelletScore()
            self.IAlertKIStart()
        #
        # KI shortcut commands
        elif command == kMiniBigKIToggle:
            if not PtIsSinglePlayerMode():
                self.IminiToggleKISize()
        elif command == kKIPutAway:
            if not PtIsSinglePlayerMode():
                self.IminiPutAwayKI()
        elif command == kChatAreaPageUp:
            self.IminiChatAreaPageUp()
        elif command == kChatAreaPageDown:
            self.IminiChatAreaPageDown()
        elif command == kChatAreaGoToBegin:
            self.IminiChatAreaGoToBegin()
        elif command == kChatAreaGoToEnd:
            self.IminiChatAreaGoToEnd()
        elif command == kKITakePicture:
            if not PtIsSinglePlayerMode():
                self.IminiTakePicture()
        elif command == kKICreateJournalNote:
            if not PtIsSinglePlayerMode():
                self.IminiCreateJournal()
        elif command == kKIToggleFade:
            if self.IsChatFaded():
                #then activate
                self.IKillFadeTimer()
                self.IStartFadeTimer()
            else:
                # then un-activate
                self.IFadeCompletely()
        elif command == kKIToggleFadeEnable:
            self.IKillFadeTimer()
            if FadeEnableFlag:
                FadeEnableFlag = 0
            else:
                FadeEnableFlag = 1
            self.IStartFadeTimer()
        elif command == kKIChatStatusMsg:
            self.IDoStatusChatMessage(value)
        elif command == kKILocalChatStatusMsg:
            self.IDoStatusChatMessage(value,netPropagate=0)
        elif command == kKILocalChatErrorMsg:
            self.IDoErrorChatMessage(value)
        elif command == kKIUpSizeFont:
            self.IUpFontSize()
        elif command == kKIDownSizeFont:
            self.IDownFontSize()
        elif command == kKIOpenYeehsaBook:
            nm = ptNetLinkingMgr()
            if theKILevel >= kMicroKI and not IKIDisabled and not WaitingForAnimation and nm.isEnabled():
                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                if IsEntireYeeshaBookEnabled and ( curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit ):
                    self.IShowYeeshaBook()
                    if theKILevel == kMicroKI:
                        plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                    else:
                        plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                    plybkCB.setChecked(1)
        elif command == kKIOpenKI:
            if not PtIsSinglePlayerMode():
                if not WaitingForAnimation:
                    # toggle between putaway,mini,big,putaway
                    if not KIMini.dialog.isEnabled():
                        self.IminiPutAwayKI(1)
                    elif not BigKI.dialog.isEnabled():
                        # check to see if we can just unfade the miniKI display
                        if not FadeEnableFlag and self.IsChatFaded():  #fix for when fading is disabled
                            self.IminiToggleKISize()
                        elif FadeEnableFlag and self.IsChatFaded():
                            #then activate
                            self.IKillFadeTimer()
                            self.IStartFadeTimer()
                        else:
                            self.IminiToggleKISize()
                    else:
                        self.IminiPutAwayKI()
        elif command == kKIShowCCRHelp:
            # hide the Yeesha book if open
            if YeeshaBook:
                YeeshaBook.hide()
            PtToggleAvatarClickability(true)
            if theKILevel == kMicroKI:
                plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                plybkCB.setChecked(0)
            elif theKILevel > kMicroKI:
                plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kPlayerBookCBID))
                plybkCB.setChecked(0)
            # no try to do things at the same time!
            if not WaitingForAnimation and not IKIDisabled:
                # show the options menu dialog
                PtShowDialog("OptionsMenuGUI")
        elif command == kKIPhasedAllOn:
            PhasedKICreateNotes = 1
            PhasedKICreateImages = 1
            PhasedKIShareYeeshaBook = 1
            PhasedKIInterAgeChat = 1
            PhasedKINeighborsInDPL = 1
            PhasedKIBuddies = 1
            PhasedKIPlayMarkerGame = 1
            PhasedKICreateMarkerGame = 1
            PhasedKISendNotes = 1
            PhasedKISendImages = 1
            PhasedKISendMarkerGame = 1
            PhasedKIShowMarkerGame = 1
        elif command == kKIPhasedAllOff:
            PhasedKICreateNotes = 0
            PhasedKICreateImages = 0
            PhasedKIShareYeeshaBook = 0
            PhasedKIInterAgeChat = 0
            PhasedKINeighborsInDPL = 0
            PhasedKIBuddies = 0
            PhasedKIPlayMarkerGame = 0
            PhasedKICreateMarkerGame = 0
            PhasedKISendNotes = 0
            PhasedKISendImages = 0
            PhasedKISendMarkerGame = 0
            PhasedKIShowMarkerGame = 0
        elif command == kKIOKDialog or command == kKIOKDialogNoQuit:
            reasonField = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
            try:
                localized = kOKDialogDict[value]
            except KeyError:
                localized = U"UNTRANSLATED: "+unicode(value)
            reasonField.setStringW(localized)
            noButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kNoButtonID))
            noButton.hide()
            noBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kNoButtonTextID))
            noBtnText.hide()
            yesBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesButtonTextID))
            yesBtnText.setStringW(PtGetLocalizedString("KI.YesNoDialog.OKButton"))
            YNWhatReason = kYNQuit
            if command == kKIOKDialogNoQuit:
                YNWhatReason = kYNNoReason
            KIYesNo.dialog.show()
        elif command == kDisableYeeshaBook:
            IsYeeshaBookEnabled = 0
        elif command == kEnableYeeshaBook:
            IsYeeshaBookEnabled = 1
        elif command == kQuitDialog:
            yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
            yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
            self.ILocalizeQuitNoDialog()
            logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoLogoutTextID))
            logoutText.show()
            logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesNoLogoutButtonID))
            logoutButton.show()
            KIYesNo.dialog.show()
        elif command == kDisableEntireYeeshaBook:
            IsEntireYeeshaBookEnabled = 0
        elif command == kEnableEntireYeeshaBook:
            IsEntireYeeshaBookEnabled = 1
        elif command == kGZUpdated:
            if value != 0:
                # setting the max in the GZ marker chronicle
                vault = ptVault()
                # is there a chronicle for the GZ games?
                entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
                if type(entry) != type(None):
                    markers = entry.chronicleGetValue()
                    if len(markers) < value:
                        # need to increase the capacity of the markers - start as active
                        markers += kGZMarkerAvailable * (value - len(markers))
                        entry.chronicleSetValue(markers)
                        entry.save()
                else:
                    # if there is none, then just add another entry - start off as active
                    markers = kGZMarkerAvailable * value
                    vault.addChronicleEntry(kChronicleGZMarkersAquired,kChronicleGZMarkersAquiredType,markers)
            self.IDetermineKILevel()
            self.IDetermineGZ()
            self.IRefreshMiniKIMarkerDisplay()
        elif command == kGZFlashUpdate:
            #This msg no longer is just for the marker machine to update the KI
            #It now supports all External updates
            #All other external updates should use -1 for the GZGame (i.e. the first parameter)
            try:
                args = value.split()
                GZGame = string.atoi(args[0])
            except:
                PtDebugPrint("xKI.OnKIMsg(kGZFlashUpdate):\tCannot Update Marker Display, Invalid Parameters: %s" %value)
                return

            if GZGame == -1:
                self.IGZFlashUpdate(value)
            else:
                self.IDetermineKILevel()
                if gKIMarkerLevel > kKIMarkerNotUpgraded and gKIMarkerLevel < kKIMarkerNormalLevel:
                    self.IGZFlashUpdate(value)
            self.IRefreshMiniKIMarkerDisplay()

        elif command == kGZInRange:
            # only say markers are in range if we have more markers to get
            if gMarkerToGetNumber > gMarkerGottenNumber:
                gGZMarkerInRange = value[0]
                gGZMarkerInRangeRepy = value[1]
                self.IRefreshMiniKIMarkerDisplay()
                if not KIMini.dialog.isEnabled():
                    NewItemAlert.dialog.show()
                    kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                    kialert.show()
        elif command == kGZOutRange:
            gGZMarkerInRange = 0
            gGZMarkerInRangeRepy = None
            self.IRefreshMiniKIMarkerDisplay()
            NewItemAlert.dialog.hide()
            kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
            kialert.hide()
        elif command == kUpgradeKIMarkerLevel:
            self.IUpgradeKIMarkerLevel(value)
            self.IRefreshMiniKIMarkerDisplay()
        elif command == kKIShowMiniKI:
            if not PtIsSinglePlayerMode() and theKILevel >= kNormalKI:
                self.IClearBBMini(0)    # set the checkbox, which will show the dialog... chain reaction

        #-------------------------#
        # CGZ Marker Game KI Msgs #
        #-------------------------#
        elif command == kMGStartCGZGame:
            #KI message to start a CGZ Marker Game
            PtDebugPrint("xKI.OnKIMsg():\tCreating a CGZ Marker Game with (game number=%s)" %(value))

            if type(value) != type(None):
                self.markerGameManager.createCGZMarkerGame(value)
            else: 
                PtDebugPrint("xKI.OnKIMsg():\tERROR: Invalid game parameter, aborting game creation!")
        elif command == kMGStopCGZGame:
            #So far, this is only called by the grtzMarkerScopeGUI, as most of the time xMarkerGameManager has control
            #Although this may change when other marker games are added (please revisit)
            if self.markerGameManager != None:
                self.markerGameManager.stopCGZGame()

        #----------------------------------#
        # User-Created Marker Game KI Msgs #
        #----------------------------------#
        elif command == kKICreateMarker:
            self.ICreateAMarker()
        elif command == kKICreateMarkerFolder:
            self.ICreateMarkerGame()

        elif command == kFriendInviteSent:
            if value == 0:
                self.IDoStatusChatMessage(PtGetLocalizedString("KI.Messages.InviteSuccess"), netPropagate=0)
            else:
                if value == 30:
                    msg = PtGetLocalizedString("KI.Errors.InvitesDisabled")
                else:
                    msg = "Error occured trying to send invite: " + str(value)
                self.IDoErrorChatMessage(msg)

        elif command == kRegisterImager:
            self.ImagerMap[value[0]] = value[1]


    def OnGameCliMsg(self, msg):
        global MFdialogMode
        msgType = msg.getType()
        #-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#
        #     Game Client Messages    #
        #-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#
        if msgType == PtGameCliMsgTypes.kGameCliPlayerJoinedMsg:
            joinMsg = msg.upcastToFinalGameCliMsg()
            
            #TODO: Currently we bail if we have an incomming client join msg, BUT
            #If we're a user-created game, we need to do some bookkeeping...
            #This will definitely need work later on (i.e. for multi-player games)!
            if joinMsg.playerID() != PtGetLocalClientID():
                return

            if self.markerGameDisplay != None:
                self.markerGameDisplay.registerPlayerJoin(joinMsg)
            else:
                self.markerGameManager.registerPlayerJoin(joinMsg)
            return
        #elif msgType == PtGameCliMsgTypes.kGameCliPlayerLeftMsg:
        #elif msgType == PtGameCliMsgTypes.kGameCliInviteFailedMsg:
        #elif msgType == PtGameCliMsgTypes.kGameCliOwnerChangeMsg:

        #-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#
        #       Marker Messages       #
        #-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#
        #Bail out if it's not a marker message (we process other messages above)
        if msgType != PtGameCliMsgTypes.kGameCliMarkerMsg:
            return
        msg = msg.upcastToGameMsg()
        msgType = msg.getMarkerMsgType()
        finalMsg = msg.upcastToFinalMarkerMsg()

    #Template Created
        if msgType == PtMarkerMsgTypes.kMarkerTemplateCreated:
            if self.markerGameDisplay != None:
                self.markerGameDisplay.registerTemplateCreated(finalMsg)
            else:
                self.markerGameManager.registerTemplateCreated(finalMsg)
        #elif msgType == PtMarkerMsgTypes.kMarkerTeamAssigned:
    #Game Started
        elif msgType == PtMarkerMsgTypes.kMarkerGameStarted:
            if type(self.markerGameDisplay) != type(None) and self.markerGameDisplay.isMyMsg(finalMsg):
                self.IBKCheckContentRefresh(BKCurrentContent)
    #Game Paused        
        elif msgType == PtMarkerMsgTypes.kMarkerGamePaused:
            if self.markerGameManager.isMyMsg(finalMsg):
                self.markerGameManager.registerPauseGame(finalMsg)
            self.IBKCheckContentRefresh(BKCurrentContent)
    #Game Reset
        elif msgType == PtMarkerMsgTypes.kMarkerGameReset:
            self.markerGameManager.registerResetGame(finalMsg)
            self.markerGameDisplay.registerResetGame(finalMsg)
            self.IBKCheckContentRefresh(BKCurrentContent)


                
    #Game Over        
        elif msgType == PtMarkerMsgTypes.kMarkerGameOver:
            self.markerGameManager.registerMarkerGameOver(finalMsg)
    #Game Name Changed
        elif msgType == PtMarkerMsgTypes.kMarkerGameNameChanged:
            if self.markerGameDisplay != None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay.registerGameName(finalMsg)
                return
            self.markerGameManager.registerGameName(finalMsg)
        #elif msgType == PtMarkerMsgTypes.kMarkerTimeLimitChanged:
    #Game Deleted        
        elif msgType == PtMarkerMsgTypes.kMarkerGameDeleted:
            if self.markerGameDisplay != None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay = None                   
            self.markerGameManager.registerDeleteGame(finalMsg)
    #Marker Added            
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerAdded:
            # here and in other functions...
            if self.markerGameDisplay != None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay.registerMarker(finalMsg)
                    if type(self.pendingMGmessage) != type(None) and self.pendingMGmessage == kMessageWait.createMarker and BigKI.dialog.isEnabled():
                        self.pendingMGmessage = None
                        self.ISetWorkingToCurrentMarkerFolder()
                    return
            self.pendingMGmessage = None
            self.markerGameManager.registerMarker(finalMsg)
    #Marker Deleted            
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerDeleted:
            if self.markerGameDisplay != None and self.markerGameDisplay.isMyMsg(finalMsg):
                self.markerGameDisplay.registerDeleteMarker(finalMsg)
                self.IBKCheckContentRefresh(BKCurrentContent)
    #Marker Name Changed
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerNameChanged:
            if type(self.markerGameDisplay) != type(None) and self.markerGameDisplay.isMyMsg(finalMsg):
                self.markerGameDisplay.registerMarkerNameChanged(finalMsg)
                if type(self.pendingMGmessage) != type(None) and self.pendingMGmessage == kMessageWait.changeMarkerName:
                    if MFdialogMode == kMFEditingMarker:
                        self.IBKCheckContentRefresh(BKCurrentContent)
            self.pendingMGmessage = None
    #Marker Captured
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerCaptured:
            if type(self.markerGameDisplay) != type(None) and self.markerGameDisplay.isMyMsg(finalMsg):
                if self.markerGameManager.gameData.data['svrGameTemplateID'] != self.markerGameDisplay.gameData.data['svrGameTemplateID']:
                    self.markerGameDisplay.registerMarkerCaptured(finalMsg)

            if self.markerGameManager.gameLoaded() and self.markerGameManager.isMyMsg(finalMsg):
                self.markerGameManager.registerMarkerCaptured(finalMsg)
    #Game Type
        elif msgType == PtMarkerMsgTypes.kMarkerGameType:
            if self.markerGameDisplay != None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay.registerGameType(finalMsg)
                return
            self.markerGameManager.registerGameType(finalMsg)

    def OnBackdoorMsg(self, target, param):
        global kInternalDev

        global BKFolderLineDict
        global BKFolderListOrder
        global BKFolderSelected
        global BKFolderTopLine

        if target == "ki" and param == "refresh":
            BKFolderLineDict = BKJournalFolderDict
            BKFolderListOrder = BKJournalListOrder
            BKFolderSelected = BKJournalFolderSelected
            BKFolderTopLine = BKJournalFolderTopLine

            self.IBigKIRefreshFolders()
            self.IBigKIRefreshFolderDisplay()
            self.IBigKIRefreshContentList()
            self.IBigKIRefreshContentListDisplay()

            self.IBigKIChangeMode(kBKListMode)
        
        if target.lower() == "cgz":  #All CGZ Marker Game messages are passed on to the appropriate scripts
            self.markerGameManager.OnBackdoorMsg(target, param)
        elif target.lower() == "togglebuildnotify" or target.lower() == "devbliss":  # Removes those annoying "Uru has been updated..." messsages
            kInternalDev = abs(kInternalDev - 1)
            if kInternalDev:
                print "/t****[   Developer Bliss is now ON    ]****"
                print "/t****[ Blocking Build Notify Messages ]****"
            else:
                print "/t****[   Developer Bliss is now OFF   ]****"
                print "/t****[ Allowing Build Notify Messages ]****"

    def IRemoveKILevel(self,level,upgrading=0):
        "Undo those parts of the KI to undo this level"
        if level == kNanoKI:
            KINanoBlackBar.dialog.hide()
        elif level == kMicroKI:
            if not upgrading:
                # unwear the new Yeesha book
                avatar = PtGetLocalAvatar()
                gender = avatar.avatar.getAvatarClothingGroup()
                if gender > kFemaleClothingGroup:
                    gender = kMaleClothingGroup
                avatar.netForce(1)
                if gender == kFemaleClothingGroup:
                    avatar.avatar.removeClothingItem("FAccPlayerBook")
                else:
                    avatar.avatar.removeClothingItem("MAccPlayerBook")
                avatar.avatar.saveClothing() # save any clothing changes
            # change the display to be the normal KI
            KIMicroBlackbar.dialog.hide()
            if not PtIsSinglePlayerMode():
                KIMicro.dialog.hide()
        elif level == kNormalKI:
            # goin' from normal back to micro... then
            # unwear the new KI
            avatar = PtGetLocalAvatar()
            gender = avatar.avatar.getAvatarClothingGroup()
            if gender > kFemaleClothingGroup:
                gender = kMaleClothingGroup
            avatar.netForce(1)
            if gender == kFemaleClothingGroup:
                avatar.avatar.removeClothingItem("FAccKI")
            else:
                avatar.avatar.removeClothingItem("MAccKI")
            avatar.avatar.saveClothing() # save any clothing changes
            if not PtIsSinglePlayerMode():
                # Fill in the listbox so that the test is near the enter box
                chatarea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kChatDisplayArea))
                chatarea.lock()         # make the chat display immutable
                chatarea.unclickable()  # make the chat display non-clickable
                chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
                chatarea.disableScrollControl()
                # switch the display of the KI to the micro
                KIBlackbar.dialog.hide()
                KIMini.dialog.hide()
                # hide all dialogs that are on the right side of the bigKI
                KIListModeDialog.dialog.hide()
                KIPictureExpanded.dialog.hide()
                KIJournalExpanded.dialog.hide()
                KIPlayerExpanded.dialog.hide()
                # then hide the bigKI
                BigKI.dialog.hide()
                # set animation back to dot on screen
                KIOnAnim.animation.skipToTime(1.5)

    def IWearKILevel(self,level):
        "Wear a set level"
        # next bring up the new KI level
        if level == kNanoKI:
            # wear the nano
            KINanoBlackBar.dialog.show()
        elif level == kMicroKI:
            # wear the new Yeesha book
            avatar = PtGetLocalAvatar()
            gender = avatar.avatar.getAvatarClothingGroup()
            if gender > kFemaleClothingGroup:
                gender = kMaleClothingGroup
            avatar.netForce(1)
            if gender == kFemaleClothingGroup:
                avatar.avatar.wearClothingItem("FAccPlayerBook")
            else:
                avatar.avatar.wearClothingItem("MAccPlayerBook")
            avatar.avatar.saveClothing() # save any clothing changes
            # show the micro
            KIMicroBlackbar.dialog.show()
            self.IClearBBMini()
            if not PtIsSinglePlayerMode():
                KIMicro.dialog.show()
                self.IEnterChatMode(0)
            # set alert that the Yeesha book is now available
            ### self.IAlertBookStart(time=kMaxBookAlertTime)
        elif level == kNormalKI:
            # goin' from micro to normal... then
            # wear the new KI
            avatar = PtGetLocalAvatar()
            gender = avatar.avatar.getAvatarClothingGroup()
            if gender > kFemaleClothingGroup:
                gender = kMaleClothingGroup
            avatar.netForce(1)
            if gender == kFemaleClothingGroup:
                avatar.avatar.wearClothingItem("FAccKI")
            else:
                avatar.avatar.wearClothingItem("MAccKI")
            avatar.avatar.saveClothing() # save any clothing changes
            if not PtIsSinglePlayerMode():
                # change the display to be the normal KI
                KIBlackbar.dialog.show()
                self.IClearBBMini()
                # set animation back to dot on screen
                KIOnAnim.animation.skipToTime(1.5)
                # set alert that the KI is now available
                self.IAlertKIStart()
                # look for anything new in the Inbox
                self.ICheckInboxForUnseen()
                # refresh the folders, which will create the age journal for this age
                self.IBigKIRefreshFolders()
            else:
                # show the micro
                KIMicroBlackbar.dialog.show()
                self.IClearBBMini()

    def IMakeSureWeWereKILevel(self):
        "Wear a set level"
        # next bring up the new KI level
        if theKILevel == kNanoKI:
            # nothing to extra to wear
            pass
        elif theKILevel == kMicroKI:
            # we should be wearing the new Yeesha book
            try:
                avatar = PtGetLocalAvatar()
                gender = avatar.avatar.getAvatarClothingGroup()
                if gender > kFemaleClothingGroup:
                    gender = kMaleClothingGroup
                avatar.netForce(1)
                if gender == kFemaleClothingGroup:
                    avatar.avatar.wearClothingItem("FAccPlayerBook")
                else:
                    avatar.avatar.wearClothingItem("MAccPlayerBook")
            except NameError:
                pass
### don't save for now... until there is a better time to do the save
#            avatar.avatar.saveClothing() # save any clothing changes
            # show the micro
        elif theKILevel == kNormalKI:
            # we should be wearing a KI
            try:
                avatar = PtGetLocalAvatar()
                gender = avatar.avatar.getAvatarClothingGroup()
                if gender > kFemaleClothingGroup:
                    gender = kMaleClothingGroup
                avatar.netForce(1)
                if gender == kFemaleClothingGroup:
                    avatar.avatar.wearClothingItem("FAccPlayerBook")
                    avatar.avatar.wearClothingItem("FAccKI")
                else:
                    avatar.avatar.wearClothingItem("MAccPlayerBook")
                    avatar.avatar.wearClothingItem("MAccKI")
            except NameError:
                pass
### don't save for now... until there is a better time to do the save
#            avatar.avatar.saveClothing() # save any clothing changes

    def OnRTChat(self,player,message,flags):
        "On receipt of RTChat message"
        if type(message) != type(None):
            cflags = ChatFlags(flags)
            # make sure that its a channel that we are on (but only if a broadcast message)
            if cflags.broadcast:
                if cflags.channel != PrivateChatChannel:
                    # its a private channel message that we can't listen to
                    return
            # if we are using the nano KI, then only private and admin message to get thru!
            if theKILevel == kNanoKI:
                if cflags.private or cflags.admin:
                    pass
                else:
                    return
            # censor message to players taste
            message = xCensor.xCensor(message,theCensorLevel)

            # Make sure the incoming message is from an approved source
            vault = ptVault()
            ignores = vault.getIgnoreListFolder()
            if type(ignores) != type(None):
                if ignores.playerlistHasPlayer(player.getPlayerID()):  #Discard Message
                    return

            #Everything's okay, send message....
            self.IAddRTChat(player,message,cflags,forceKI=not ISawTheKIAtleastOnce)

            # see if they are AFK and the message was directly to them, send back they're state to sender
            try:
                if IKIDisabled or PtGetLocalAvatar().avatar.getCurrentMode() == PtBrainModes.kAFK and cflags.private and not cflags.toSelf:
                    plist = [player]
                    myself = PtGetLocalPlayer()
                    afkself = ptPlayer(myself.getPlayerName()+str(PtGetLocalizedString("KI.Chat.AFK")),myself.getPlayerID())
                    PtSendRTChat(afkself, plist, " ",cflags.flags)
            except NameError:
                pass

    def OnTimer(self,id):
        "Timer event, for the fade stuff"
        global FadeMode
        global CurrentFadeTick
        global BKRightSideMode
        global LastminiKICenter
        global OriginalminiKICenter
        global BKJournalFolderDict
        global BKPlayerFolderSelected
        global BKPlayerFolderTopLine
        global BKFolderLineDict
        global BKFolderListOrder
        global BKFolderSelected
        global BKFolderTopLine
        global WeAreTakingAPicture
        #PtDebugPrint("xKI:OnTimer id=%d  FadeMode=%d" % (id,FadeMode) )
        if id == kFadeTimer:
            if PtIsSinglePlayerMode():
                return
            # if we are fading then fade a tick
            if FadeMode == kFadeFullDisp:
                CurrentFadeTick -= 1
                #print "CurrentFadeTick = %d" % (CurrentFadeTick)
                if CurrentFadeTick > 0:
                    # setup call for next second
                    PtAtTimeCallback(self.key,kFullTickTime,kFadeTimer)
                else:
                    FadeMode = kFadeDoingFade
                    CurrentFadeTick = TicksOnFade
                    # setup call for next second
                    PtAtTimeCallback(self.key,kFadeTickTime,kFadeTimer)
            elif FadeMode == kFadeDoingFade:
                CurrentFadeTick -= 1
                if CurrentFadeTick > 0:
                    # fade a little
                    if theKILevel < kNormalKI:
                        mKIdialog = KIMicro.dialog
                    else:
                        mKIdialog = KIMini.dialog
                    mKIdialog.setForeColor(-1,-1,-1,OriginalForeAlpha*CurrentFadeTick/TicksOnFade)
                    mKIdialog.setSelectColor(-1,-1,-1,OriginalSelectAlpha*CurrentFadeTick/TicksOnFade)
                    mKIdialog.refreshAllControls()
                    # setup call for next second
                    PtAtTimeCallback(self.key,kFadeTickTime,kFadeTimer)
                else:
                    # completely fade out
                    self.IFadeCompletely()
            elif FadeMode == kFadeStopping:
                FadeMode = kFadeNotActive
        elif id == kBKITODCheck:
            # update the time of day if the BigKI is up
            if BigKI.dialog.isEnabled():
                self.IBigKISetChanging()
####===> for testing only
#            # play with the lights just for a test
#            # pick random indicator
#            indicator = whrandom.randrange(kminiMarkerIndicator01,kminiMarkerIndicatorLast+1)
#            # pick random light
#            lcolor = whrandom.choice(gMarkerColors.values())
#            # light it
#            mcb = ptGUIControlProgress(KIMini.dialog.getControlFromTag(indicator))
#            mcb.setValue(lcolor)
####===> for testing only
        elif id == kMarkerGameTimer:
            if type(CurrentPlayingMarkerGame) != type(None):
                CurrentPlayingMarkerGame.updateGameTime()
                PtAtTimeCallback(self.key,1,kMarkerGameTimer)
        elif id == kAlertHideTimer:
            self.IAlertStop()
        elif id == kTakeSnapShot:
            print "*-*-*- Start screen capture -*-*-*-*"
            PtStartScreenCapture(self.key)
        elif id == kDumpLogsTimer:
            if (PtDumpLogs(self.logDumpDest)):
                self.IDoStatusChatMessage(PtGetLocalizedString("KI.Messages.LogDumpSuccess", [self.logDumpDest]),netPropagate=0)
            else:
                self.IDoStatusChatMessage(PtGetLocalizedString("KI.Messages.LogDumpFailed", [self.logDumpDest]),netPropagate=0)
        elif id == kLightStopID:
            self.DoKILight(0,0)
        elif id == kJalakBtnDelayTimer:
            self.SetJalakGUIButtons(1)


    def OnScreenCaptureDone(self,image):
        global BKRightSideMode
        global LastminiKICenter
        global OriginalminiKICenter
        global BKJournalFolderDict
        global BKPlayerFolderSelected
        global BKPlayerFolderTopLine
        global BKFolderLineDict
        global BKFolderListOrder
        global BKFolderSelected
        global BKFolderTopLine
        global WeAreTakingAPicture
        global kImageDirectory
        global kImageFileNameTemplate


        "The screen capture image is ready to stow"
        print "#-#-#- capture screen received -#-#-#-#"
        # create a journal image using a screen shot
        self.IBigKICreateJournalImage(image)
        # only show the KI if there isn't a dialog in the way
        if not PtIsGUIModal():
            # make sure that we are in journal mode
            if BKFolderLineDict is BKJournalFolderDict:
                pass
            else:
                modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kBKRadioModeID))
                modeselector.setValue(0)
            # set things up so that when the bigKI shows it goes into edit mode
            if BKRightSideMode != kBKPictureExpanded:
                # make sure if a different is showing, hide it
                self.IBigKIHideMode()
            # now it "show the picture expanded" mode
            BKRightSideMode = kBKPictureExpanded
            # reset the topline and selection
            self.IBigKIRefreshFolderDisplay()
            # setup to edit the title of the picture (err... the caption)
            self.IBigKIEnterEditMode(kBKEditFieldPICTitle)
            # show the big stuff
            BigKI.dialog.show()
            # was just the miniMI showing (not the big KI?)
            if type(LastminiKICenter) == type(None):
                #~ print "only mini KI should have been showing"
                if type(OriginalminiKICenter) != type(None):
                    dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kminiDragBar))
                    LastminiKICenter = dragbar.getObjectCenter()
                    dragbar.setObjectCenter(OriginalminiKICenter)
                    dragbar.anchor()
            KIMini.dialog.show()
        else:
            # if we aren't supposed to display the ki, at least flash it so they know something happened
            self.IAlertKIStart()
        
        WeAreTakingAPicture = 0

        basePath = PtGetUserPath() + U"\\" + kImageDirectory + U"\\"
        gImageFileSearch = ((basePath + kImageFileNameTemplate) + U'*.jpg')
        if not PtCreateDir(basePath):
            print U"xKI::OnScreenCaptureDone(): Unable to create \"" + basePath + "\" directory. Image not saved to disk."
            return
        
        found = 0
        gCurrentImageFilename = None
        gLastImageFileNumber = 0

        while (not found):
            gLastImageFileNumber += 1
            tryName = ((basePath + kImageFileNameTemplate) + (U'%04d.jpg' % gLastImageFileNumber))
            if PtFileExists(tryName):
                pass
            else:
                found = 1
        
        print U"xKI::OnScreenCaptureDone(): Saving image to \"" + tryName + "\""
        image.saveAsJPEG(tryName, 90)

    def OnMemberUpdate(self):
        "The userlist has been updated, get a fresh copy"
        PtDebugPrint("xKI:OnMemberUpdate - refresh player list",level=kDebugDumpLevel)
        # make sure that the mini-me-KI is loaded before trying to set it up
        if PtIsDialogLoaded("KIMini"):
            self.IRefreshPlayerList()
            self.IRefreshPlayerListDisplay()

    def OnRemoteAvatarInfo(self,player):
        "We be checkin' out this player :-)"
        global PlayerInfoName
        global BKPlayerSelected
        if theKILevel < kNormalKI:
            return
        avatarSet = 0
        if type(player) == type(0):
            pass
        elif isinstance(player,ptPlayer):
            PlayerInfoName = player
            avatarSet = 1
            BKPlayerSelected = player
            sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
            sendToField.setString(PlayerInfoName.getPlayerName())
            self.IBigKISetToButtons()
            # find the player in the DPL and select them
            for pidx in range(len(BKPlayerList)):
                if isinstance(BKPlayerList[pidx],ptPlayer) and BKPlayerList[pidx] == player:
                    playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
                    playerlist.setSelection(pidx)
                    # also set the caret for the chat
                    caret = ptGUIControlTextBox(KIMini.dialog.getControlFromTag(kChatCaretID))
                    caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt")+unicode(player.getPlayerName())+U" >")
                    privateChbox = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiPrivateToggle))
                    privateChbox.setChecked(1)
                    break
        if avatarSet:
            if not KIMini.dialog.isEnabled():
                KIMini.dialog.show()
            self.IKillFadeTimer()
            self.IStartFadeTimer()

    def OnVaultNotify(self,event,tupdata):
        "A high level player vault event"
        global theKILevel
        PtDebugPrint("xKI:OnVaultNotify recvd. Event=%d and data= " % (event),tupdata,level=kDebugDumpLevel)
        try:
            test = tupdata[0]
        except:
            PtDebugPrint("xKI:OnVaultNotify: Bailing, we don't have any age data!")
            return


        if PtIsDialogLoaded("KIMain"):
            if event == PtVaultNotifyTypes.kRegisteredOwnedAge or event == PtVaultNotifyTypes.kUnRegisteredOwnedAge or event == PtVaultNotifyTypes.kRegisteredVisitAge or event == PtVaultNotifyTypes.kUnRegisteredVisitAge:
                #PtDebugPrint("xKI: kRegisteredOwnedAge event (id=%d,type=%d)" % (tupdata[0].getID(),tupdata[0].getType()),level=kDebugDumpLevel)
                if theKILevel > kMicroKI:
                    # a new owned age is added... beter refresh its folders
                    if isinstance(tupdata[0],ptVaultAgeLinkNode):
                        # ... better see if it is the neighborhood
                        ownedAge = tupdata[0].getAgeInfo()
                        if type(ownedAge) != type(None):
                            if self.IIsAgeMyNeighborhood(ownedAge):
                                self.IBigKIRefreshHoodStatics(ownedAge)
                                # new set of neighbors to display in the DPL
                                self.IRefreshPlayerList()
                                self.IRefreshPlayerListDisplay()
                            # re-build the player folder list 'cause number of lists may have changed
                            self.IBigKIRefreshFolders()
                            self.IBigKIRefreshFolderDisplay()
                            self.IBigKIRefreshContentList()
                            self.IBigKIRefreshContentListDisplay()
                            self.IRefreshAgeOwnerSettings()
                        else:
                            PtDebugPrint("xKI:kRegisteredOwnedAge: Error - where is the stinking ageInfo! ",level=kErrorLevel)
                    else:
                        PtDebugPrint("xKI:kRegisteringOwnedAge: Error - unknown tuple data type! ",level=kErrorLevel)
            else:
                PtDebugPrint("xKI: OnVaultNotify - unknown event! %d" % (event),level=kWarningLevel)
        else:
            PtDebugPrint("xKI: OnVaultNotify - bigKI dialog was not loaded... waiting...",level=kDebugDumpLevel)

    def OnAgeVaultEvent(self,event,tupdata):
        "A low level age vault event"
        PtDebugPrint("xKI:OnAgeVaultEvent recvd. Event=%d and data= " % (event),tupdata,level=kDebugDumpLevel)
        self.HandleVaultTypeEvents(event,tupdata)

    def OnVaultEvent(self,event,tupdata):
        "A low level player vault event"
        PtDebugPrint("xKI:OnVaultEvent recvd. Event=%d and data= " % (event),tupdata,level=kDebugDumpLevel)
        self.HandleVaultTypeEvents(event,tupdata)

    def HandleVaultTypeEvents(self,event,tupdata):
        global theKILevel
        # make sure that the bigKI dialog is loaded before trying to update it
        if PtIsDialogLoaded("KIMain"):
            if event == PtVaultCallbackTypes.kVaultConnected:
                PtDebugPrint("xKI: kVaultConnected event",level=kDebugDumpLevel)
                # tupdata is ()
                pass
            elif event == PtVaultCallbackTypes.kVaultDisconnected:
                PtDebugPrint("xKI: kVaultDisconnected event",level=kDebugDumpLevel)
                pass
            elif event == PtVaultCallbackTypes.kVaultNodeSaved:
                PtDebugPrint("xKI: kVaultNodeSaved event (id=%d,type=%d)" % (tupdata[0].getID(),tupdata[0].getType()),level=kDebugDumpLevel)
                # tupdata is ( ptVaultNode )
                # if a player info has changed then reget player info stuff
                if tupdata[0].getType() == PtVaultNodeTypes.kPlayerInfoNode:
                    self.IRefreshPlayerList()
                    self.IRefreshPlayerListDisplay()
                elif tupdata[0].getType() == PtVaultNodeTypes.kAgeInfoNode:
                    # an age info was updated... refresh everything of the like
                    self.IBigKISetStatics()
                    self.IBigKIRefreshFolders()
                    self.IBigKIOnlySelectedToButtons()
                    self.IRefreshAgeOwnerSettings()
#TODO: Need to re-enable/remodel this at some point                
#~                elif tupdata[0].getType() == PtVaultNodeTypes.kMarkerNode:
#~                    if BKRightSideMode == kBKMarkerListExpanded:
#~                        ptMarkerMgr().setSelectedMarker(tupdata[0].getID())
#~                        self.IBigKIDisplayCurrentContentMarkerFolder()
                self.IBigKIRefreshContentList()
                self.IBigKIRefreshContentListDisplay()
            elif event == PtVaultCallbackTypes.kVaultNodeInitialized:
                PtDebugPrint("xKI: kVaultNodeInitialized event (id=%d,type=%d)" % (tupdata[0].getID(),tupdata[0].getType()),level=kDebugDumpLevel)
                # tupdata is ( ptVaultNode )
                if theKILevel > kMicroKI:
                    node = tupdata[0]
                    self.IBKCheckElementRefresh(tupdata[0])
            elif event == PtVaultCallbackTypes.kVaultNodeAdded:
                PtDebugPrint("xKI: kVaultNodeAdded event",level=kDebugDumpLevel)
                pass
            elif event == PtVaultCallbackTypes.kVaultNodeRefAdded:
                PtDebugPrint("xKI: kVaultNodeRefAdded event (childID=%d,parentID=%d)" % (tupdata[0].getChildID(),tupdata[0].getParentID()),level=kDebugDumpLevel)
                # tupdata is ( ptVaultNodeRef )
                if theKILevel > kMicroKI:
                    folder = tupdata[0].getParent()
                    folder = folder.upcastToFolderNode()
                    # if the parent of this ref is the inbox, then its incoming mail
                    if type(folder) != type(None) and folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
                        #Tye: This is a hack as the beenSeen() function has not been implemented yet!
                        #Since the function always returns true, we'll force the alert to happen with every message!
                        #Please remove when the status of the beenSeen() function has changed.
                        self.IAlertKIStart()  

                        if not tupdata[0].beenSeen():
                            if OnlyGetPMsFromBuddies:
                                vault = ptVault()
                                inbox = vault.getInbox()
                                buddies = vault.getBuddyListFolder()
                                if buddies.playerlistHasPlayer(tupdata[0].getSaverID()):
                                    # then show alert
                                    self.IAlertKIStart()
                            else:
                                ### no check needed- on with the show
                                # then show alert
                                self.IAlertKIStart()
                        
                    child = tupdata[0].getChild()
                    child = child.upcastToFolderNode()
                    if type(child) != type(None):
                        # the thing being added is a folder... re-get the folders
                        PtDebugPrint("xKI- adding a folder... refresh folder list",level=kDebugDumpLevel)
                        self.IBigKIRefreshFolders()
                    self.IBKCheckFolderRefresh(folder)
            elif event == PtVaultCallbackTypes.kVaultRemovingNodeRef:
                PtDebugPrint("xKI: kVaultRemovingNodeRef event (childID=%d,parentID=%d)" % (tupdata[0].getChildID(),tupdata[0].getParentID()),level=kDebugDumpLevel)
                # tupdata is ( ptVaultNodeRef )
                pass
            elif event == PtVaultCallbackTypes.kVaultNodeRefRemoved:
                PtDebugPrint("xKI: kVaultNodeRefRemoved event (childID,parentID) ",tupdata,level=kDebugDumpLevel)
                # tupdata is ( childID, parentID )
                if theKILevel > kMicroKI:
                    if BKRightSideMode == kBKMarkerListExpanded:
                        self.IBigKIDisplayCurrentContentMarkerFolder()
                    self.IBKCheckFolderRefresh()
            elif event == PtVaultCallbackTypes.kVaultOperationFailed:
                PtDebugPrint("xKI: kVaultOperationFailed event  (operation,resultCode) ",tupdata,level=kDebugDumpLevel)
                #tupdata is ( operation, resultCode )
                pass
            else:
                PtDebugPrint("xBigKI: OnVaultEvent - unknown event! %d" % (event),level=kWarningLevel)
        else:
            PtDebugPrint("xBigKI: OnVaultEvent - bigKI dialog was not loaded... waiting...",level=kDebugDumpLevel)

    def OnCCRMsg(self,msgtype,message,ccrplayerid):
        "A CCR message!"
        global CCRConversationInProgress
        PtDebugPrint("xKI: CCR message recv'd type=%d,ccr=%d,msg=%s" % (msgtype,ccrplayerid,message),level=kDebugDumpLevel)
        if msgtype == kCCRBeginCommunication:
            self.IAddRTChat(None,PtGetLocalizedString("KI.CCR.ConversationStarted"),kChatCCRMessage)
            CCRConversationInProgress = ccrplayerid
            self.IAddRTChat(None,message,kChatCCRMessage)
        elif msgtype == kCCREndCommunication:
            self.IAddRTChat(None,PtGetLocalizedString("KI.CCR.ConversationEnded"),kChatCCRMessage)
            CCRConversationInProgress = 0
        elif msgtype == kCCRChat:
            self.IAddRTChat(None,message,kChatCCRMessage)
        elif msgtype == kCCRReturnChatMsg:
            daPlayer = ptPlayer(str(PtGetLocalizedString("KI.Chat.CCRFromPlayer", [str(ccrplayerid)])),ccrplayerid)
            self.IAddRTChat(daPlayer,message,kChatCCRMessageFromPlayer)
        else:
            PtDebugPrint("xKI - unknown CCR message of type %d" % (msgtype),level=kWarningLevel)
        if PtGetLocalAvatar().avatar.getCurrentMode():
            if CCRConversationInProgress:
                # CCRConversationInProgress also holds the CCR playerid
                PtSendChatToCCR("Player is AFK",CCRConversationInProgress)

    def OnMarkerMsg(self,msgType,tupdata):
        "process Marker messages"
        #Currently the only one we get is the marker captured message!
        if msgType == PtMarkerMsgType.kMarkerCaptured:
            # tupdata:
            #   [0] = markerID
            PtDebugPrint("DEBUG: xKI.OnMarkerMsg(): MarkerDisplay reports a marker received a collision.  MarkerID = %s" %tupdata[0], level=kDebugDumpLevel)
            if self.markerGameDisplay != None:
                return
            self.markerGameManager.captureMarker(tupdata[0])

    def IDetermineCensorLevel(self):
        "Set the CensorLevel"
        global theCensorLevel
        # assume that they have none...
        theCensorLevel = xCensor.xRatedPG
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleCensorLevel)
        if type(entry) == type(None):
            # not found... add current level chronicle
            vault.addChronicleEntry(kChronicleCensorLevel,kChronicleCensorLevelType,"%d" % (theCensorLevel))
        else:
            theCensorLevel = string.atoi(entry.chronicleGetValue())
        PtDebugPrint("xKI: the censor level is %d" % (theCensorLevel),level=kWarningLevel)
    def ISaveCensorLevel(self):
        "Set the Censor level in the chronicle"
        global theCensorLevel
        # assume that they have none...
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleCensorLevel)
        if type(entry) != type(None):
            entry.chronicleSetValue("%d" % (theCensorLevel))
            entry.save()
        else:
            vault.addChronicleEntry(kChronicleCensorLevel,kChronicleCensorLevelType,"%d" % (theCensorLevel))
        PtDebugPrint("xKI: Saving Censor level of %d" % (theCensorLevel),level=kWarningLevel)

    def IDetermineKILevel(self):
        "Set the KILevel"
        global theKILevel
        global gKIMarkerLevel
        global gFeather
        # assume that they have none...
        theKILevel = kNanoKI
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleKILevel)
        if type(entry) == type(None):
            # not found... add current level chronicle
            vault.addChronicleEntry(kChronicleKILevel,kChronicleKILevelType,"%d" % (theKILevel))
        else:
            oldLevel = string.atoi(entry.chronicleGetValue())
            if oldLevel >= kLowestKILevel and oldLevel <= kHighestKILevel:
                theKILevel = oldLevel
        PtDebugPrint("xKI: the KI level is %d" % (theKILevel),level=kWarningLevel)
        # set the KIMarkerLevel
        #   assume no level
        gKIMarkerLevel = 0
        entry = vault.findChronicleEntry(kChronicleKIMarkerLevel)
        if type(entry) == type(None):
            # not found... add current level chronicle
            vault.addChronicleEntry(kChronicleKIMarkerLevel,kChronicleKIMarkerLevelType,"%d" % (gKIMarkerLevel))
        else:
            try:
                gKIMarkerLevel = string.atoi(entry.chronicleGetValue())                
            except:
                PtDebugPrint("ERROR: xKI.IDetermineKILevel(): Chronicle entry error with the KI's Marker Level, resetting to the default value")
                entry.chronicleSetValue("%d" % (gKIMarkerLevel))
                entry.save()
                
        PtDebugPrint("xKI: the KIMarker level is %d" % (gKIMarkerLevel),level=kWarningLevel)
        entry = vault.findChronicleEntry("feather")
        if type(entry) == type(None):
            # not found... add current level chronicle
            gFeather = 0
        else:
            try:
                gFeather = string.atoi(entry.chronicleGetValue())
            except ValueError:
                gFeather = 0

    def IUpgradeKIMarkerLevel(self,newLevel):
        "upgrade the KIMarker level to something"
        global gKIMarkerLevel
        PtDebugPrint("xKI: KIMarker going from %d to %d" % (gKIMarkerLevel,newLevel),level=kWarningLevel)
        if theKILevel > kMicroKI:
            if newLevel > gKIMarkerLevel:
                gKIMarkerLevel = newLevel
                vault = ptVault()
                entry = vault.findChronicleEntry(kChronicleKIMarkerLevel)
                if type(entry) == type(None):
                    # not found... add current level chronicle
                    PtDebugPrint("xKI: KIMarker level not found - set to %d" % (gKIMarkerLevel),level=kWarningLevel)
                    vault.addChronicleEntry(kChronicleKIMarkerLevel,kChronicleKIMarkerLevelType,"%d" % (gKIMarkerLevel))
                else:
                    PtDebugPrint("xKI: KIMarker upgrading existing level to %d" % (gKIMarkerLevel),level=kWarningLevel)
                    entry.chronicleSetValue("%d" % (gKIMarkerLevel))
                    entry.save()

    def IDetermineFontSize(self):
        "Set the FontSize from saved"
        # assume that they have none...
        fontSize = self.IGetFontSize()
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleFontSize)
        if type(entry) == type(None):
            # not found... add current level chronicle
            vault.addChronicleEntry(kChronicleFontSize,kChronicleFontSizeType,"%d" % (fontSize))
        else:
            fontSize = string.atoi(entry.chronicleGetValue())
            self.ISetFontSize(fontSize)
        PtDebugPrint("xKI: the Saved Font Size is %d" % (fontSize),level=kWarningLevel)
    def ISaveFontSize(self):
        "Set the FontSize from saved"
        # assume that they have none...
        fontSize = self.IGetFontSize()
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleFontSize)
        if type(entry) != type(None):
            entry.chronicleSetValue("%d" % (fontSize))
            entry.save()
        else:
            vault.addChronicleEntry(kChronicleFontSize,kChronicleFontSizeType,"%d" % (fontSize))
        PtDebugPrint("xKI: Saving Font Size of %d" % (fontSize),level=kWarningLevel)

    def IDetermineFadeTime(self):
        "Set the FadeTime from saved"
        global TicksOnFull
        global FadeEnableFlag
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleFadeTime)
        if type(entry) == type(None):
            # not found... add current level chronicle
            vault.addChronicleEntry(kChronicleFadeTime,kChronicleFadeTimeType,"%d" % (TicksOnFull))
        else:
            TicksOnFull = string.atoi(entry.chronicleGetValue())
            if TicksOnFull == kFadeTimeMax:
                # disable the fade all together
                FadeEnableFlag = 0
                self.IKillFadeTimer()
                PtDebugPrint("KIDeterineFadeTime: FadeTime disabled",level=kWarningLevel)
            else:
                FadeEnableFlag = 1
        PtDebugPrint("xKI: the Saved Fade Time is %d" % (TicksOnFull),level=kWarningLevel)
    def ISaveFadeTime(self):
        "Set the FadeTime from saved"
        global TicksOnFull
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleFadeTime)
        if type(entry) != type(None):
            entry.chronicleSetValue("%d" % (TicksOnFull))
            entry.save()
        else:
            vault.addChronicleEntry(kChronicleFadeTime,kChronicleFadeTimeType,"%d" % (TicksOnFull))
        PtDebugPrint("xKI: Saving Fade Time of %d" % (TicksOnFull),level=kWarningLevel)

    def IDetermineKIFlags(self):
        "Sets the KI flags from the saved chronicle"
        global OnlyGetPMsFromBuddies
        global OnlyAllowBuddiesOnRequest
        vault = ptVault()
        # Only gets PMs and KIMail from Buddies
        entry = vault.findChronicleEntry(kChronicleOnlyPMs)
        if type(entry) == type(None):
            # not found.... then add to chronicle
            vault.addChronicleEntry(kChronicleOnlyPMs,kChronicleOnlyPMsType,"%d" % (OnlyGetPMsFromBuddies))
        else:
            OnlyGetPMsFromBuddies = string.atoi(entry.chronicleGetValue())
        # Only allow people to be buddies if I say so
        entry = vault.findChronicleEntry(kChronicleBuddiesOnRequest)
        if type(entry) == type(None):
            # not found.... then add to chronicle
            vault.addChronicleEntry(kChronicleBuddiesOnRequest,kChronicleBuddiesOnRequestType,"%d" % (OnlyAllowBuddiesOnRequest))
        else:
            OnlyAllowBuddiesOnRequest = string.atoi(entry.chronicleGetValue())
    def ISaveKIFlags(self):
        "Save the KIFlags"
        global OnlyGetPMsFromBuddies
        global OnlyAllowBuddiesOnRequest
        vault = ptVault()
        # Only gets PMs and KIMail from Buddies
        entry = vault.findChronicleEntry(kChronicleOnlyPMs)
        if type(entry) != type(None):
            entry.chronicleSetValue("%d" % (OnlyGetPMsFromBuddies))
            entry.save()
        else:
            vault.addChronicleEntry(kChronicleOnlyPMs,kChronicleOnlyPMsType,"%d" % (OnlyGetPMsFromBuddies))
        # Only allow people to be buddies if I say so
        entry = vault.findChronicleEntry(kChronicleBuddiesOnRequest)
        if type(entry) != type(None):
            entry.chronicleSetValue("%d" % (OnlyAllowBuddiesOnRequest))
            entry.save()
        else:
            vault.addChronicleEntry(kChronicleBuddiesOnRequest,kChronicleBuddiesOnRequestType,"%d" % (OnlyAllowBuddiesOnRequest))

    def IDetermineGZ(self):
        "Update the GZ globals from chronicle"
        global gGZPlaying
        global gMarkerToGetColor
        global gMarkerGottenColor
        global gMarkerToGetNumber
        global gMarkerGottenNumber
        if gKIMarkerLevel > kKIMarkerNotUpgraded:
            if gKIMarkerLevel < kKIMarkerNormalLevel:
                vault = ptVault()
                # is there a chronicle for the GZ games?
                entry = vault.findChronicleEntry(kChronicleGZGames)
                error = 0
                if type(entry) != type(None):
                    gameString = entry.chronicleGetValue()
                    print "xKI:GZ - game string is: %s" % (gameString)
                    args = gameString.split()
                    if len(args) == 3:
                        try:
                            gGZPlaying = string.atoi(args[0])
                            colors = args[1].split(':')
                            outof = args[2].split(':')

                            #Check for corrupted entry
                            if len(colors) != 2 or len(outof) !=2:
                                print "xKI.GZ DetermineGZ: Invalid color field or marker field: %s" %(gameString)
                                raise ValueError                     


                            #Check for invalid entry                
                            if (colors[0] == 'red' or colors[0] == 'green') and string.atoi(outof[1]) > 15:
                                print "xKI.GZ DetermineGZ: Invalid marker number entry (i.e. 1515 bug): %s" %(gameString)
                                raise ValueError

                            gMarkerGottenColor = colors[0]
                            gMarkerToGetColor = colors[1]
                            gMarkerGottenNumber = string.atoi(outof[0])
                            gMarkerToGetNumber = string.atoi(outof[1])

                            return
                        except:
                            print "xKI:GZ - error trying to read GZGames Chronicle"
                            error = 1
                    else:
                        print "xKI:GZ - error GZGames string formation error"
                        error = 1
                # if bad update for any reason, reset to null game
                gGZPlaying = 0
                gMarkerToGetColor = 'off'
                gMarkerGottenColor = 'off'
                gMarkerToGetNumber = 0
                gMarkerGottenNumber = 0
                
                #Reset marker games if a corrupted vault occurred
                if error:
                    PtDebugPrint("xKI - vault corruption error: RESETTING all Marker Game data!!!!", level=kErrorLevel)
                    import grtzKIMarkerMachine
                    grtzKIMarkerMachine.ResetMarkerGame()

            else:
                # can't be playing a GZGame!
                gGZPlaying = 0
                # we've done the CGZs, clear them only if there is no game running
                if MarkerGameState == kMGNotActive or type(CurrentPlayingMarkerGame) == type(None):
                    gMarkerToGetColor = 'off'
                    gMarkerGottenColor = 'off'
                    gMarkerToGetNumber = 0
                    gMarkerGottenNumber = 0
        else:
            # else set globals to default off state
            gGZPlaying = 0
            gMarkerToGetColor = 'off'
            gMarkerGottenColor = 'off'
            gMarkerToGetNumber = 0
            gMarkerGottenNumber = 0

    def IGZFlashUpdate(self,gameString):
        "Update the GZ globals from value - flash display (no save)"
        global gGZPlaying
        global gMarkerToGetColor
        global gMarkerGottenColor
        global gMarkerToGetNumber
        global gMarkerGottenNumber
        print "xKI:GZ FLASH - game string is %s" % (gameString)
        args = gameString.split()
        if len(args) == 3:
            try:
                GZPlaying = string.atoi(args[0])
                colors = args[1].split(':')
                outof = args[2].split(':')

                #Check for corrupted entry
                if len(colors) != 2 or len(outof) !=2:
                    print "xKI.GZ Flash: Invalid color field or marker field: %s" %(gameString)
                    raise ValueError

                MarkerGottenColor = colors[0]
                MarkerToGetColor = colors[1]
                MarkerGottenNumber = string.atoi(outof[0])
                MarkerToGetNumber = string.atoi(outof[1])

                #Check for invalid entry                
                if (colors[0] == 'red' or colors[0] == 'green') and MarkerToGetNumber > 15:
                    print "xKI.GZ Flash: Invalid marker number entry (i.e. 1515 bug): %s" %(gameString)
                    raise ValueError

                
    
                if GZPlaying != -1: # Make sure we have a valid GZ game, otherwise don't update as we're probably playing some other marker game
                    gGZPlaying = GZPlaying

                gMarkerGottenColor = MarkerGottenColor
                gMarkerToGetColor = MarkerToGetColor
                gMarkerGottenNumber = MarkerGottenNumber
                gMarkerToGetNumber = MarkerToGetNumber
                return
            except:
                print "xKI:GZ FLASH - Error trying to read GZGames input string....  Checking Chronicle for corruption"
        else:
            print "xKI:GZ FLASH - Error GZGames string formation error.... Checking Chronicle for corruption"

        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleGZGames)
        if type(entry) != type(None):
            if gameString == entry.chronicleGetValue():
                print "xKI:GZ Flash - ****Error: Vault Corrupted: trying to gracefully reset to a default state****"
                import grtzKIMarkerMachine
                grtzKIMarkerMachine.ResetMarkerGame()
                return


        print "xKI:GZ Flash - Error: Chronicle not corrupted, proceeding (this implies an error in code somewhere)!"


    def IUpdateGZGamesChonicles(self):
        "Update the GZ chronicle variable"
        global gGZPlaying
        global gMarkerToGetColor
        global gMarkerGottenColor
        global gMarkerToGetNumber
        global gMarkerGottenNumber
        if gGZPlaying:
            vault = ptVault()
            # is there a chronicle for the GZ games?
            entry = vault.findChronicleEntry(kChronicleGZGames)
            upstring = "%d %s:%s %d:%d" % (gGZPlaying,gMarkerGottenColor,gMarkerToGetColor,gMarkerGottenNumber,gMarkerToGetNumber)
            if type(entry) != type(None):
                entry.chronicleSetValue(upstring)
                entry.save()
            else:
                # if there is none, then just add another entry
                vault.addChronicleEntry(kChronicleGZGames,kChronicleGZGamesType,upstring)

    def IUpdateKILevelChronicle(self):
        "Update the KILevelChronicle to the new KI level"
        vault = ptVault()
        # find the current version (if there is one)
        entry = vault.findChronicleEntry(kChronicleKILevel)
        if type(entry) != type(None):
            entry.chronicleSetValue("%d" % (theKILevel))
            entry.save()
        else:
            # if there is none, then just add another entry
            vault.addChronicleEntry(kChronicleKILevel,kChronicleKILevelType,"%d" % (theKILevel))

    def IGetNeighborhood(self):
        "find the neighborhood for this player"
        try:
            return ptVault().getLinkToMyNeighborhood().getAgeInfo()
        except AttributeError:
            PtDebugPrint("xKI: neighborhood was not found",level=kDebugDumpLevel)
            return None

    def IGetNeighbors(self):
        "find our lovely neighbors"
        try:
            return self.IGetNeighborhood().getAgeOwnersFolder()
        except AttributeError:
            PtDebugPrint("xKI: list of neighbors was not found",level=kDebugDumpLevel)
            return None

    def IGetAgeFileName(self,ageInfo=None):
        "returns the .age file name of the age"
        if type(ageInfo) == type(None):
            ageInfo = PtGetAgeInfo()
        if type(ageInfo) != type(None):
            return ageInfo.getAgeFilename()
        else:
            return "?UNKNOWN?"

    def IGetAgeInstanceName(self,ageInfo=None):
        "Returns the name of the age's instance name"
        if type(ageInfo) == type(None):
            ageInfo = PtGetAgeInfo()
        if type(ageInfo) != type(None):
            if ageInfo.getAgeInstanceName() == "D'ni-Rudenna":
                # see if it can stay this name
                sdl = xPsnlVaultSDL()
                if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                    pass
                else:
                    return "???"
            if ageInfo.getAgeInstanceName() == "Ae'gura":
                return "D'ni-Ae'gura"
            return self.IFilterAgeName(xLocTools.LocalizeAgeName(ageInfo.getAgeInstanceName()))
        else:
            return "?UNKNOWN?"

    def IIsChildAge(self, ageInfo):
        if ageInfo:
            parent = ageInfo.getParentAgeLink()
            if parent:
                parentinfo = parent.getAgeInfo()
                if parentinfo:
                    return 1
        return 0

    def IGetAgeDisplayName(self,ageInfo=None):
        "Returns the way the name should be displayed"
        isSubAge = 0
        agevault = ptAgeVault()
        if type(ageInfo) == type(None):
            ageInfo = agevault.getAgeInfo()

        isChildAge = self.IIsChildAge(ageInfo)
            
        if type(ageInfo) != type(None):
            if ageInfo.getAgeFilename() != "Neighborhood":
                subAges = agevault.getSubAgesFolder()
                if subAges and subAges.getChildNodeCount() > 0:
                    isSubAge = 1
            
            if ageInfo.getAgeInstanceName() == "D'ni-Rudenna":
                # see if it can stay this name
                sdl = xPsnlVaultSDL()
                if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                    return "D'ni-Rudenna"
                else:
                    return "???"
            if ageInfo.getAgeInstanceName() == "Ae'gura" or ageInfo.getAgeFilename() == "city":
                if isChildAge:
                    return "D'ni-Ae'gura'"
                return "D'ni-Ae'gura"
            
            if ageInfo.getAgeFilename() == "spyroom":
                return "Old Spy Room"

            if ageInfo.getAgeFilename() == "philRelto":
                return "Phil's Relto"
            
            if ageInfo.getAgeFilename() == "GuildPub-Cartographers":
                return "The Cartographers' Pub"
            if ageInfo.getAgeFilename() == "GuildPub-Greeters":
                return "The Greeters' Pub"
            if ageInfo.getAgeFilename() == "GuildPub-Maintainers":
                return "The Maintainers' Pub"
            if ageInfo.getAgeFilename() == "GuildPub-Messengers":
                return "The Messengers' Pub"
            if ageInfo.getAgeFilename() == "GuildPub-Writers":
                return "The Writers' Pub"

            if ageInfo.getAgeFilename() == "Kveer":
                return "K'veer"

            if ageInfo.getAgeFilename() in kHideAgesHackList:
                return "???"

            # don't include the persons name on D'ni locations
            iname = ageInfo.getAgeInstanceName()
            if iname.startswith("D'ni"):
                return iname

            if isChildAge or isSubAge:
                localizeName = ageInfo.getAgeInstanceName()
                if isChildAge:
                    localizeName += "'"
            else:
                localizeName = ageInfo.getDisplayName()

            return self.IFilterAgeName(xLocTools.LocalizeAgeName(localizeName))
            
        else:
            return "?UNKNOWN?"

    def IFilterAgeName(self,ageName):
        "filter and lastminute switching of the age names... can be anywhere in string"
        #print "AgeName input as %s" % (ageName)
        if ageName.find("Garrison") != -1:
            ageName = ageName.replace("Garrison", "Gahreesen")
        if ageName.find("Personal") != -1:
            ageName = ageName.replace("Personal", "Relto")
        if ageName.find("Garden") != -1:
            ageName = ageName.replace("Garden", "Eder Kemo")
        if ageName == "city":
            ageName = "D'ni-Ae'gura"
        elif ageName == "GreatZero" or ageName == "Great Zero":
            ageName = "D'ni-Rezeero"
        elif ageName == "Gira":
            ageName = "Eder Gira"
        elif ageName == "AhnonayCathedral":
            ageName = "Ahnonay Cathedral"
        elif ageName == "EderDelin":
            ageName = "Eder Delin"
        elif ageName == "EderTsogal":
            ageName = "Eder Tsogal"
        elif ageName == "spyroom":
            ageName = ageName.replace("spyroom", "Old Spy Room")
        elif ageName == "philRelto":
            ageName = ageName.replace("philRelto", "Phil's Relto")
        elif ageName == "GuildPub-Cartographers":
            ageName = ageName.replace("GuildPub-Cartographers", "The Cartographers' Pub")
        elif ageName == "GuildPub-Greeters":
            ageName = ageName.replace("GuildPub-Greeters", "The Greeters' Pub")
        elif ageName == "GuildPub-Maintainers":
            ageName = ageName.replace("GuildPub-Maintainers", "The Maintainers' Pub")
        elif ageName == "GuildPub-Messengers":
            ageName = ageName.replace("GuildPub-Messengers", "The Messengers' Pub")
        elif ageName == "GuildPub-Writers":
            ageName = ageName.replace("GuildPub-Writers", "The Writers' Pub")
        elif ageName == "Kveer":
            ageName = ageName.replace("Kveer", "K'veer")
        #print "AgeName output as %s" % (ageName)
        return ageName


    def ICanAgeBeMadePublic(self,ageInfo):
        "Returns true(1) if the age in question can be made public, otherwise returns 0"
        try:
            if self.IIsAgeMyNeighborhood(ageInfo):
                return 1
        except AttributeError:
            pass
        return 0

    def ICanAgeInviteVistors(self,ageInfo,link):
        "Returns true(1) if the age in question can have invited vistors, otherwise returns 0"
        try:
            if ageInfo.getAgeFilename() == "Personal":
                return 0
            if ageInfo.getAgeFilename() == "Nexus":
                return 0
            if ageInfo.getAgeFilename() == "Cleft":
                return 0
            if ageInfo.getAgeFilename() == "AvatarCustomization":
                return 0
            if string.lower(ageInfo.getAgeFilename()) == "city":
                return 0
            if ageInfo.getAgeFilename() == "BahroCave":
                return 0
            if (ageInfo.getAgeFilename() == "LiveBahroCave" or ageInfo.getAgeFilename() == "LiveBahroCaves"):
                return 0
            if ageInfo.getAgeFilename() == "BaronCityOffice":
                return 0
            if ageInfo.getAgeFilename() == "ErcanaCitySilo":
                return 0
            if ageInfo.getAgeFilename() == "GreatZero":
                return 0
            if ageInfo.getAgeFilename() == "Shaft":
                return 0
        except AttributeError:
            pass
        # make sure that the age has not been deleted
        if link.getVolatile():
            return 0
        # make sure they have a default link to this age
        # if the age doesn't have a default link then they don't have full access yet
        spawnPoints = link.getSpawnPoints()
        for spawnlink in spawnPoints:
            if spawnlink.getTitle() == "Default":
                return 1
        return 0

    def ICanConfigAge(self,ageInfo):
        "Returns true(1) if the age in question can be config'd, otherwise returns 0"
#==== Age co-owner list - only neigborhoods are configurable
#        try:
#            if ageInfo.getAgeFilename() == "Personal":
#                return 0
#            if ageInfo.getAgeFilename() == "Nexus":
#                return 0
#            if ageInfo.getAgeFilename() == "Cleft":
#                return 0
#            if ageInfo.getAgeFilename() == "AvatarCustomization":
#                return 0
#            if string.lower(ageInfo.getAgeFilename()) == "city":
#                return 0
#        except AttributeError:
#            pass
#        return 1
#==== Age co-owner list - only neigborhoods are configurable
        if ageInfo.getAgeFilename() == "Neighborhood":
            return 1
        return 0

    def IConvertAgeName(self,ageName):
        if ageName == "Cleft":
            return "D'ni-Riltagamin"
        if ageName == "BahroCave":
            # see if it can stay this name
            sdl = xPsnlVaultSDL()
            if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                return "D'ni-Rudenna"
            else:
                return "???"
        if ageName == "BaronCityOffice":
            return "D'ni-Ae'gura"
        if ageName == "city":
            return "D'ni-Ae'gura"
        if ageName == "Ae'gura":
            return "D'ni-Ae'gura"
        if ageName == "Personal":
            return "Relto"
        if ageName == "Garden":
            return "Eder Kemo"
        if ageName == "Gira":
            return "Eder Gira"
        if ageName == "Garrison":
             return "Gahreesen"
        if ageName == "ErcanaCitySilo":
            return "D'ni-Ashem'en"
        if ageName == "GreatZero":
            return "D'ni-Rezeero"
        if ageName == "Shaft":
            return "D'ni-Tiwah"
        if ageName == "spyroom":
            return "Old Spy Room"
        if ageName == "philRelto":
            return "Phil's Relto"
        if ageName == "GuildPub-Cartographers":
            return "The Cartographers' Pub"
        if ageName == "GuildPub-Greeters":
            return "The Greeters' Pub"
        if ageName == "GuildPub-Maintainers":
            return "The Maintainers' Pub"
        if ageName == "GuildPub-Messengers":
            return "The Messengers' Pub"
        if ageName == "GuildPub-Writers":
            return "The Writers' Pub"
        if ageName == "AhnonayCathedral":
            return "Ahnonay Cathedral"
        if ageName == "Kveer":
            return "K'veer"
        return ageName

    def IIsAgeMyNeighborhood(self,ageInfo):
        "Return true(1) if the age in question is our neighborhood. otherwise return 0"
        try:
            hoodGUID = ptVault().getLinkToMyNeighborhood().getAgeInfo().getAgeInstanceGuid()
            if type(hoodGUID) != type("") or hoodGUID == "":
                PtDebugPrint("xKI: neighborhood GUID not valid",level=kWarningLevel)
                # can't trust this test, try a different one
                if ageInfo.getAgeFilename() == "Neighborhood":
                    return 1
            else:
                if ageInfo.getAgeInstanceGuid() == hoodGUID:
                    return 1
        except AttributeError:
            pass
        return 0

    def IUpdateKIUsage(self):
        "update the KI usage globals"
        global NumberOfPictures
        global NumberOfNotes
        global NumberOfMarkerFolders
        global NumberOfMarkers
        usage = ptVault().getKIUsage()
        NumberOfPictures = usage[0]
        NumberOfNotes = usage[1]
        NumberOfMarkerFolders = usage[2]
        try:
            NumberOfMarkers = self.markerGameDisplay.gameData.data['numMarkers']
        except:
            NumberOfMarkers = -1

    def ICanTakePicture(self):
        "determine whether they've reached limit or not"
        self.IUpdateKIUsage()
        if kMaxPictures == -1 or NumberOfPictures < kMaxPictures:
            return 1
        return 0
    def ICanMakeNote(self):
        "determine whether they've reached limit or not"
        self.IUpdateKIUsage()
        if kMaxNotes == -1 or NumberOfNotes < kMaxNotes:
            return 1
        return 0
    def ICanMakeMarkerFolder(self):
        "determine whether they've reached limit or not"
        self.IUpdateKIUsage()
        if kMaxMarkerFolders == -1 or NumberOfMarkerFolders < kMaxMarkerFolders:
            return 1
        return 0
    def ICanMakeMarker(self):
        "determine whether they've reached limit or not"
        self.IUpdateKIUsage()
        if kMaxMarkers == -1 or NumberOfMarkers < kMaxMarkers:
            return 1
        return 0
       

    def ICreateMarkerGame(self):
        "The first step towards creating a user-created marker game"
        global theKILevel
        global IKIDisabled
        global WeAreTakingAPicture
        global WaitingForAnimation
        global PhasedKICreateMarkerGame
        global gKIMarkerLevel
        
        # if single player mode... can't have this
        if PtIsSinglePlayerMode():
            return
        
        # must have clearance
        if not PhasedKICreateMarkerGame or gKIMarkerLevel < kKIMarkerNormalLevel:
            PtDebugPrint("DEBUG: xKI.ICreateMarkerGame():\tAborting create marker game request, user does not have sufficient privileges!")
            return

        #User cannot be busy doing some other task
        if WeAreTakingAPicture or WaitingForAnimation:
            PtDebugPrint("DEBUG: xKI.ICreateMarkerGame():\tAborting create marker game request: user is busy!")
            return

        # only those that have garrison KI can create game
        if theKILevel <= kMicroKI or IKIDisabled:
            PtDebugPrint("DEBUG: xKI.ICreateMarkerGame():\tAborting create marker game request: User does not have the KI!")
            return

        if self.markerGameManager.gameLoaded():
            PtDebugPrint("DEBUG: xKI.ICreateMarkerGame():\tAborting create marker game request: A game is already in progress!")
            self.IAddRTChat(None,PtGetLocalizedString("KI.MarkerGame.createErrorExistingGame"),kChatSystemMessage)
            return

        if not self.ICanMakeMarkerFolder():
            PtDebugPrint("DEBUG: xKI.ICreateMarkerGame():\tAborting create marker game request: Marker Games are FULL!")
            self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullMarkerGames"))
            return

        
        self.IBigKIHideBigKI()
        PtShowDialog("KIMiniMarkers")
        KIMarkerGameGUIOpen.run(self.key,netPropagate=0)
    

    def IFinishCreateMarkerFolder(self, gameName, gameGUID):
        "Finishes creating the marker game folder after the asynchronous mini-game server registers the parameters"
        #Tye: Check these variables, not sure if all of them are being used....
        global LastminiKICenter
        global BKFolderTopLine
        global BKJournalFolderTopLine
        global BKFolderSelected
        global BKJournalFolderSelected
        global BKCurrentContent

        #Get the current age journal folder....
        load = 0
        while load < 2:
            try:
                journal = BKJournalFolderDict[self.IGetAgeInstanceName()]
    
                #If we don't have a age journal folder, we do need to create one!!!
                if type(journal) == type(None):
                    raise
                load = 2
            except:
                if load == 1:
                    # We failed two times in a row, it's hopeless...  
                    # TODO: Or we could try to actually make the folder ourselves....
                    PtDebugPrint("ERROR: xKI.IFinishCreateMarkerFolder():\tCould not load Age Journal Folder, Marker Game Creation FAILED!")
                    return
                
                load += 1
                self.IBigKIRefreshFolders()

        # hide the blackbar (even if its already hid)
        KIBlackbar.dialog.hide()
        
        # put the toggle button back to bigKI
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
        toggleCB.setChecked(1)

        # make sure that we are in journal mode
        modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kBKRadioModeID))
        modeselector.setValue(0)
        
        # make sure that the age folder is selected
        BKFolderTopLine = BKJournalFolderTopLine = 0      # scroll back to the top
        BKFolderSelected = BKJournalFolderSelected = BKJournalListOrder.index(self.IGetAgeInstanceName())
        self.IBigKIRefreshFolderDisplay()
 
        # create the marker game node
        PtDebugPrint("DEBUG: xKI.IFinishCreateMarkerFolder():\tCreating Vault Node with name: %s, and GUID=%s" %(gameName, gameGUID))
        markerGameNode = ptVaultMarkerGameNode()
        markerGameNode.setGameName(gameName)
        markerGameNode.setGameGuid(gameGUID)
        BKCurrentContent = journal.addNode(markerGameNode)

        # change to display current content
        self.IBigKIChangeMode(kBKMarkerListExpanded)
        # bring up the bigKI if not visible
        if BigKI.dialog.isEnabled():
            self.IBigKIShowMode()
        else:
            # need to make the miniKI come up over the bigKI
            KIMini.dialog.hide()
            # show the big stuff
            BigKI.dialog.show()
            # need to make the miniKI come up over the bigKI
            KIMini.dialog.show()
        # was just the miniMI showing (not the big KI?)
        if type(LastminiKICenter) == type(None):
            #~ print "only mini KI should have been showing"
            if type(OriginalminiKICenter) != type(None):
               dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kminiDragBar))
               LastminiKICenter = dragbar.getObjectCenter()
               dragbar.setObjectCenter(OriginalminiKICenter)
               dragbar.anchor()


    def ICreateAMarker(self):
        global IKIDisabled
        global YNWhatReason
        # if single player mode... can't have this
        if PtIsSinglePlayerMode():
            return

        #Set this just in case we're in the GUI!
        self.pendingMGmessage = kMessageWait.createMarker 

        if not WeAreTakingAPicture and not WaitingForAnimation:
            # only those that have garrison KI can create game
            if theKILevel > kMicroKI and not IKIDisabled:
                if type(self.markerGameDisplay) != type(None) and type(self.markerGameDisplay.gameData) != type(None):
                    self.IUpdateKIUsage() 
                    if self.ICanMakeMarker() and self.markerGameDisplay.showMarkers:
                        mgData = self.markerGameDisplay.gameData.data
                        markerName = mgData['svrGameName'] + " marker"
                        try:
                            ava = PtGetLocalAvatar()
                            avaCoord = ava.position()
                            x = avaCoord.getX()
                            y = avaCoord.getY()
                            z = avaCoord.getZ()

                            self.markerGameDisplay.addMarker(x, y, z, markerName)
                            PtDebugPrint("DEBUG: xKI.ICreateAMarker():\tCreating marker at: (%s,%s,%s)" % (x,y,z))
                        except:
                            PtDebugPrint("ERROR: xKI.ICreateAMarker():\tMarker Creation FAILED!")
                            return
                        
                    else:
                        self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullMarkers"))


    def IShowKIFullErrorMsg(self, msg):
        "displays a 'ok' dialog error message"
        global YNWhatReason

        # put up some kinda error message
        YNWhatReason = kYNKIFull
        reasonField = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesNoTextID))
        reasonField.setStringW(msg)
        yesButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kYesButtonID))
        yesButton.hide()
        yesBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kYesButtonTextID))
        yesBtnText.hide()
        noBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kNoButtonTextID))
        noBtnText.setStringW(PtGetLocalizedString("KI.YesNoDialog.OKButton"))
        KIYesNo.dialog.show()

    def ISetWorkingToCurrentMarkerFolder(self):
        "sets the BKCurrent to be the working marker folder (if it is one)"
        global WorkingMarkerFolder
        global BKCurrentContent
        
        if self.markerGameDisplay == None:
            PtDebugPrint("ERROR: xKI.ISetWorkingToCurrentMarkerFolder():\tCannot set working game, as the game isn't loaded!")
            return None

        game = self.markerGameDisplay.gameData
        if game == None:
            PtDebugPrint("ERROR: xKI.ISetWorkingToCurrentMarkerFolder():\tCannot set working game, as there is no game data! ")
            return None

        if BKCurrentContent == None:
            PtDebugPrint("ERROR: xKI.ISetWorkingToCurrentMarkerFolder():\tCannot set working game, as there is no vault folder!")
            return None

        element = BKCurrentContent.getChild()
        if element == None:
            PtDebugPrint("ERROR: xKI.ISetWorkingToCurrentMarkerFolder():\tCannot set working game, as there is no vault node!")
            return None

        datatype = element.getType()
        if datatype != PtVaultNodeTypes.kMarkerGameNode:
            PtDebugPrint("ERROR: xKI.ISetWorkingToCurrentMarkerFolder():\tCannot set working game, as the vault node is of the wrong type!")
            return None
        element = element.upcastToMarkerGameNode()
        
        if element == None:
            PtDebugPrint("ERROR: xKI.ISetWorkingToCurrentMarkerFolder():\tCannot set working game, as the vault node is empty!")
            return None
          
        # refresh the DPL
        self.IRefreshPlayerList()
        self.IRefreshPlayerListDisplay()
        # refresh the current screen
        self.IBKCheckContentRefresh(BKCurrentContent)
        return element        

        """
        #TODO: DELETE ME!, left for historical reasons remove once multiplayer marker games have been implmemented
        MGmgr = ptMarkerMgr()
        # reset the player list
        WorkingMarkerFolder = MarkerGame(PtGetLocalPlayer().getPlayerID())
        # if we were looking at a marker game folder, hide the markers in case they had them up
        MGmgr.hideMarkersLocal()
        # set as the current working Marker Folder
        if type(BKCurrentContent) != type(None):
            element = BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kMarkerGameNode:
                    element = element.upcastToMarkerGameNode()
                    if element:

                        MGmgr.setWorkingMarkerFolder(element)
                        # refresh the DPL
                        self.IRefreshPlayerList()
                        self.IRefreshPlayerListDisplay()
                        # refresh the current screen
                        self.IBKCheckContentRefresh(BKCurrentContent)
                        return element
        return None
        """

    def IGetCurrentMarkerFolder(self):
        "sets the BKCurrent to be the working marker folder (if it is one)"
        global BKCurrentContent
        # find current 
        if type(BKCurrentContent) != type(None):
            element = BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kMarkerGameNode:
                    element = element.upcastToMarkerGameNode()
                    if element:
                        return element
        return None

    def IResetWorkingMarkerFolder(self):
        "resets the working marker folder to None"
        global WorkingMarkerFolder
        MGmgr = ptMarkerMgr()

        #Don't delete any markers necessary for an existing game!
        if not self.markerGameManager.gameLoaded(): 
            MGmgr.hideMarkersLocal()
       
        WorkingMarkerFolder = None
        self.markerGameDisplay = None
        # refresh the DPL
        self.IRefreshPlayerList()
        self.IRefreshPlayerListDisplay()
        # refresh the current screen
        self.IBKCheckContentRefresh(BKCurrentContent)

##############################################################
##
##  playerBook (sorta) private methods
##
##############################################################
    def IShowYeeshaBook(self):
        "Show yeesha book with extras depending state of many things"
        global theKILevel
        global IKIDisabled
        global YeeshaBook
        global PhasedKIShareYeeshaBook
        global IsYeeshaBookEnabled
        if theKILevel >= kMicroKI and not IKIDisabled and not WaitingForAnimation:
            # first make sure that the BigKI or the mini KI is not up
            if BigKI.dialog.isEnabled() or KIMini.dialog.isEnabled():
                self.IminiPutAwayKI()
            startOpen = 0
            if IsYeeshaBookEnabled:
                if OfferedBookMode == kNotOffering:
                    if PhasedKIShareYeeshaBook and not PtIsSinglePlayerMode():
                        YeeshaBDef = xLinkingBookDefs.xYeeshaBookBase + self.IGetYeeshaPageDefs()
                    else:
                        YeeshaBDef = xLinkingBookDefs.xYeeshaBookNoShare + self.IGetYeeshaPageDefs()
                else:
                    YeeshaBDef = xLinkingBookDefs.xYeeshaBookNoShare
                    startOpen = 1
            else:
                YeeshaBDef = xLinkingBookDefs.xYeeshaBookBroke + self.IGetYeeshaPageDefs()
            YeeshaBook = ptBook(YeeshaBDef,self.key)
            YeeshaBook.setSize( xLinkingBookDefs.YeeshaBookSizeWidth, xLinkingBookDefs.YeeshaBookSizeHeight )
            YeeshaBook.show(startOpen)
            PtToggleAvatarClickability(false)

    #this def needs to be called whenever the Personal Age GUI is drawn
    def IGetYeeshaPageDefs(self):
        "YeeshaBook - returns ptBook definition string for pages"
        pagedef = ''
        vault = ptVault()
        if type(vault) != type(None): #is the Vault online?
            psnlSDL = vault.getPsnlAgeSDL()
            if psnlSDL:
                for sdlvar,page in xLinkingBookDefs.xYeeshaPages:
                    FoundValue = psnlSDL.findVar(sdlvar)
                    if type(FoundValue) != type(None):
                        PtDebugPrint("xKI: The previous value of the SDL variable %s is %d" % (sdlvar, FoundValue.getInt()),level=kDebugDumpLevel)
                        state = FoundValue.getInt() % 10
                        if state != 0:
                            active = 1
                            
                            if state == 2 or state == 3:
                                active = 0
                            try:
                                pagedef += page % (active)
                            except LookupError:
                                pagedef += "<pb><pb>Bogus page %s" % (sdlvar)
            else:
                PtDebugPrint("xKI: Error trying to access the Chronicle psnlSDL. psnlSDL = %s" % ( psnlSDL),level=kErrorLevel)
        else:
            PtDebugPrint("xKI: Error trying to access the Vault. Can't access YeeshaPageChanges chronicle.",level=kErrorLevel)
        return pagedef

    def IToggleYeeshaPageSDL(self,varname,on):
        vault = ptVault()
        if type(vault) != type(None): #is the Vault online?
            psnlSDL = vault.getPsnlAgeSDL()
            if psnlSDL:
                ypageSDL = psnlSDL.findVar(varname)
                if ypageSDL:
                    size, state = divmod(ypageSDL.getInt(), 10)
                    value = None
                    if state == 1 and not on:
                        value = 3
                    elif state == 3 and on:
                        value = 1
                    elif state == 2 and on:
                        value = 4
                    elif state == 4 and not on:
                        value = 2
                    # else - all other combinations are already correct
                    if value != None:
                        PtDebugPrint("KI:Book: setting %s to %d" % (varname,value),level=kDebugDumpLevel)
                        ypageSDL.setInt( (size * 10) + value )
                        vault.updatePsnlAgeSDL(psnlSDL)

##############################################################
##
##  miniKI (sorta) private methods
##
##############################################################

    def IminiChatAreaPageUp(self):
        if PtIsSinglePlayerMode():
            return
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        self.IKillFadeTimer()
        self.IStartFadeTimer()
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        chatarea.moveCursor(PtGUIMultiLineDirection.kPageUp)

    def IminiChatAreaPageDown(self):
        if PtIsSinglePlayerMode():
            return
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        self.IKillFadeTimer()
        self.IStartFadeTimer()
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        chatarea.moveCursor(PtGUIMultiLineDirection.kPageDown)

    def IminiChatAreaGoToBegin(self):
        if PtIsSinglePlayerMode():
            return
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        self.IKillFadeTimer()
        self.IStartFadeTimer()
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        chatarea.moveCursor(PtGUIMultiLineDirection.kBufferStart)

    def IminiChatAreaGoToEnd(self):
        if PtIsSinglePlayerMode():
            return
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        self.IKillFadeTimer()
        self.IStartFadeTimer()
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)

    def IGetFontSize(self):
        "Returns the current font size"
        if PtIsSinglePlayerMode():
            return 0
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        MiniChatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        return MiniChatarea.getFontSize()

    def ISetFontSize(self,fontSize):
        "Sets all the controls that are affected by the font size"
        PtDebugPrint("ISetFontSize: Setting font size to %d"%(fontSize),level=kWarningLevel)
        if PtIsSinglePlayerMode():
            return
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        MiniChatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        MiniChatarea.setFontSize(fontSize)
        MiniChatarea.refresh()
        MicroChatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        MicroChatarea.setFontSize(fontSize)
        MicroChatarea.refresh()
        #~ MiniChatedit = ptGUIControlEditBox(mKIdialog.getControlFromTag(kChatEditboxID))
        #~ MiniChatedit.setFontSize(fontSize)
        #~ MiniChatedit.refresh()
        notearea = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(kBKIJRNNote))
        notearea.setFontSize(fontSize)
        notearea.refresh()
        ownernotes = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerDescription))
        ownernotes.setFontSize(fontSize)
        ownernotes.refresh()

    def IUpFontSize(self):
        "set the default font size for several dialogs"
        if PtIsSinglePlayerMode():
            return
        size = self.IGetFontSize()
        for i in range(len(FontSizeList)-1):
            if size <= FontSizeList[i]:
                size = FontSizeList[i+1]
                break
        self.ISetFontSize(size)
        self.ISaveFontSize()
        self.IRefreshKISettings()

    def IDownFontSize(self):
        "set the default font size for several dialogs"
        if PtIsSinglePlayerMode():
            return
        size = self.IGetFontSize()
        for i in range(len(FontSizeList)-1,0,-1):
            if size >= FontSizeList[i]:
                size = FontSizeList[i-1]
                break
        self.ISetFontSize(size)
        self.ISaveFontSize()
        self.IRefreshKISettings()

    def IStartFadeTimer(self):
        "start the fade timer"
        global FadeMode
        global CurrentFadeTick
        global FadeEnableFlag
        #~ PtDebugPrint("xminiKI: StartFadeTimer  FadeMode = %d" % (FadeMode))
        if PtIsSinglePlayerMode():
            return
        #if theKILevel < kNormalKI:
        #    return
        if not FadeEnableFlag:
            return
        # only start fade if the big KI is not up
        if PtIsSinglePlayerMode() or not BigKI.dialog.isEnabled():
            if FadeMode == kFadeNotActive:
                PtAtTimeCallback(self.key,kFullTickTime,kFadeTimer)
            FadeMode = kFadeFullDisp
            CurrentFadeTick = TicksOnFull

    def IKillFadeTimer(self):
        "reset fade timer thingy"
        global FadeMode
        global CurrentFadeTick
        #~ PtDebugPrint("xminiKI: KillFadeTimer  FadeMode=%d" % (FadeMode))
        if PtIsSinglePlayerMode():
            return
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        if FadeMode != kFadeNotActive:
            FadeMode = kFadeStopping
        CurrentFadeTick = TicksOnFull
        mKIdialog.setForeColor(-1,-1,-1,OriginalForeAlpha)
        mKIdialog.setSelectColor(-1,-1,-1,OriginalSelectAlpha)
        # un-hide the dialogs that we were fading
        if theKILevel == kNormalKI:
            playerlist = ptGUIControlListBox(mKIdialog.getControlFromTag(kPlayerList))
            playerlist.show()
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        chatarea.enableScrollControl()
        mKIdialog.refreshAllControls()

    def IFadeCompletely(self):
        "fade out the miniKI lists... completely"
        global FadeMode
        if PtIsSinglePlayerMode():
            return
        if theKILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        # only fade if the big KI is _not_ enabled... otherwise bring back to life
        if BigKI.dialog.isEnabled():
            mKIdialog.setForeColor(-1,-1,-1,OriginalForeAlpha)
            mKIdialog.setSelectColor(-1,-1,-1,OriginalSelectAlpha)
            mKIdialog.refreshAllControls()
        else:
            # completely fade out
            mKIdialog.setForeColor(-1,-1,-1,0)
            mKIdialog.setSelectColor(-1,-1,-1,0)
            mKIdialog.refreshAllControls()
            # hide the dialogs that we were fading
            if theKILevel == kNormalKI:
                playerlist = ptGUIControlListBox(mKIdialog.getControlFromTag(kPlayerList))
                playerlist.hide()
            chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
            
            #Tye: Disabled this temporarily to fix the scroll reset issues...
            #It should be re-enabled once the chat fade bug has been fixed!
            #chatarea.disableScrollControl()

            # set to inactive
            FadeMode = kFadeNotActive

    def IsChatFaded(self):
        if PtIsSinglePlayerMode():
            return 0
        # do we have clearance
        if theKILevel >= kMicroKI:
            # is the bigKI hidden and the miniKI showing
            if not BigKI.dialog.isEnabled() and (KIMini.dialog.isEnabled() or KIMicro.dialog.isEnabled()):
                # toggle the fade mode
                if FadeMode == kFadeNotActive:
                    return 1

        return 0

    def IClearBBMini(self,value=-1):
        global theKILevel
        if theKILevel < kNormalKI:
            pass
        else:
            mmRG = ptGUIControlRadioGroup(KIBlackbar.dialog.getControlFromTag(kMiniMaximizeRGID))
            mmRG.setValue(value)

    def IEnterChatMode(self,entering,firstChar=None):
        "This will enter or leave chat mode (being able to type in a text message"
        global LastminiKICenter
        global LastPrivatePlayerID
        global ToReplyToLastPrivatePlayerID
        # if single player mode... don't enter chat mode
        if PtIsSinglePlayerMode():
            return
        if theKILevel == kNanoKI:
            mKIdialog = KIMicro.dialog
            KIMicro.dialog.show()
        elif theKILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
            KIMicro.dialog.show()
        else:
            mKIdialog = KIMini.dialog
        caret = ptGUIControlTextBox(mKIdialog.getControlFromTag(kChatCaretID))
        chatedit = ptGUIControlEditBox(mKIdialog.getControlFromTag(kChatEditboxID))
        if entering:
            # the nanoPeople can't chat ... yet
            if theKILevel == kNanoKI and CCRConversationInProgress == 0:
                return
            self.isChatting = 1
            # make the chat edit box visible
            if not KIMini.dialog.isEnabled():
                self.IClearBBMini(0)    # set the checkbox, which will show the dialog... chain reaction
            if firstChar:
                chatedit.setString(firstChar)
                chatedit.end()
            else:
                chatedit.clearString()
            chatedit.show()
            caret.show()
            mKIdialog.setFocus(chatedit.getKey())
            #chatedit.focus()
            # save the reply playerID when they start typing
            ToReplyToLastPrivatePlayerID = LastPrivatePlayerID
            self.IKillFadeTimer()
        else:
            # make the chat edit box invisible
            caret.hide()
            chatedit.hide()
            self.isChatting = 0
            self.IStartFadeTimer()

    def IGetPlayersInChatDistance(self,minPlayers=8):
        "Returns a list of players that are within chat distance"
        plyrList = []
        ageplayers = PtGetPlayerListDistanceSorted()
        for ap in ageplayers:
            if ap.getPlayerID() != 0:
                if ap.getDistanceSq() < PtMaxListenDistSq():
                    plyrList.append(ap)
                else:
                    if len(plyrList) <= minPlayers:
                        plyrList.append(ap)
        return plyrList

    def IFindGroupOfBuddies(self,PList):
        "Returns a list of players from a buddy list"
        outList = []
        for bud in PList:
            if isinstance(bud, ptVaultNodeRef):
                # then send to player that might be in another age
                ebud = bud.getChild()
                ebud = ebud.upcastToPlayerInfoNode()
                if type(ebud) != type(None):
                    if ebud.playerIsOnline():
                        outList.append(ptPlayer(ebud.playerGetName(),ebud.playerGetID()))
        return outList

    def ISendRTChat(self,message):
        global BKPlayerList
        global LastPrivatePlayerID
        global PhasedKIInterAgeChat
        cflags = ChatFlags(0)
        cflags.toSelf = 1
        listenerOnly = 1
        betterTellEm = 0
        goesToFolder = None
        # if we were in AFK Mode then exit it:
        if PtGetLocalAvatar().avatar.getCurrentMode() == PtBrainModes.kAFK:
            PtAvatarExitAFK()
        # any special commands
        message = self.ICheckChatCommands(message)
        if not message:
            return
        if IAmAdmin:
            cflags.admin = 1
        # get any selection players in userlist
        userlbx = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
        privateChbox = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiPrivateToggle))
        iselect = userlbx.getSelection()
        selPlyrList = []


        if message.startswith(str(PtGetLocalizedString("KI.Commands.ChatAllAge"))):
            listenerOnly = 0
            selPlyrList = []
            message = message[len(PtGetLocalizedString("KI.Commands.ChatAllAge"))+1:]
        elif message.startswith(PtGetLocalizedString("KI.Commands.ChatReply")):
            if type(ToReplyToLastPrivatePlayerID) == type(None):
                self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.NoOneToReply"),kChatSystemMessage)
                return  # short circuit the rest of this
            # if a local private message, then check to see if they are still here
            if not ToReplyToLastPrivatePlayerID[2]:
                # get the agemembers
                agemembers = self.IRemoveCCRPlayers(PtGetPlayerListDistanceSorted())
                hasplayer = 0
                for member in agemembers:
                    if member.getPlayerID() == ToReplyToLastPrivatePlayerID[1]:
                        hasplayer = 1
                        break
                if not hasplayer:
                    LastPrivatePlayerID = None
                    self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.LeftTheAge", [str(ToReplyToLastPrivatePlayerID[0])]),kChatSystemMessage)
                    return  # short circuit the rest of this
            else:
                # check in the recents folder to see if they are still online
                vault = ptVault()
                PIKA = vault.getPeopleIKnowAboutFolder()
                if type(PIKA) != type(None):
                    # only search it if it actually has the player in question
                    if PIKA.playerlistHasPlayer(ToReplyToLastPrivatePlayerID[1]):
                        PIKArefs = PIKA.getChildNodeRefList()
                        for PIKAref in PIKArefs:
                            PIKAelem = PIKAref.getChild()
                            PIKAelem = PIKAelem.upcastToPlayerInfoNode()
                            if type(PIKAelem) != type(None):
                                if PIKAelem.playerGetID() == ToReplyToLastPrivatePlayerID[1]:
                                    if not PIKAelem.playerIsOnline():
                                        LastPrivatePlayerID = None
                                        self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.LeftTheGame", [str(ToReplyToLastPrivatePlayerID[0])]),kChatSystemMessage)
                                        return  # short circuit the rest of this
                                    break
            message = message[len(PtGetLocalizedString("KI.Commands.ChatReply"))+1:]
            # forget whatever they selected, they really want to reply to the last private message
            selPlyrList = []
            selPlyrList.append(ptPlayer(ToReplyToLastPrivatePlayerID[0],ToReplyToLastPrivatePlayerID[1]))
            cflags.private = 1
            # were they from a different Age?
            if ToReplyToLastPrivatePlayerID[2]:
                cflags.interAge = 1
                message = "<<"+self.IGetAgeDisplayName()+">>"+message
        elif message.startswith(str(PtGetLocalizedString("KI.Commands.ChatPrivate"))):
            # then parse for sending private message
            pwords = message.split()
            foundBuddy = 0
            # make sure its still just a /p
            if len(pwords) > 1 and pwords[0] == str(PtGetLocalizedString("KI.Commands.ChatPrivate")):
                # try to find their buddy in the DPL online lists
                for daPlayer in BKPlayerList:
                    if isinstance(daPlayer, ptPlayer):
                        plyrName = daPlayer.getPlayerName()
                        if pwords[1] == plyrName:
                            selPlyrList.append(daPlayer)
                            cflags.private = 1
                            # remove the '/p buddyname' from the message
                            message = message[message.find(pwords[1])+len(pwords[1])+1:]
                            foundBuddy = 1
                            # add to recents folder
                            self.IAddPlayerToRecents(daPlayer.getPlayerID())
                            break
                        # see if they start the same (name might have spaces!)
                        if plyrName.startswith(pwords[1]):
                            mrest = message[len(PtGetLocalizedString("KI.Commands.ChatPrivate"))+1:]
                            if mrest.startswith(plyrName):
                                selPlyrList.append(daPlayer)
                                cflags.private = 1
                                # remove the '/p buddyname' from the message
                                message = mrest[len(plyrName)+1:]
                                foundBuddy = 1
                                # add to recents folder
                                self.IAddPlayerToRecents(daPlayer.getPlayerID())
                                break
                    elif isinstance(daPlayer, ptVaultNodeRef):
                        # then send to player that might be in another age
                        eplyr = daPlayer.getChild()
                        eplyr = eplyr.upcastToPlayerInfoNode()
                        if type(eplyr) != type(None):
                            plyrName = eplyr.playerGetName()
                            if pwords[1] == plyrName:
                                selPlyrList.append(ptPlayer(eplyr.playerGetName(),eplyr.playerGetID()))
                                cflags.private = 1
                                cflags.interAge = 1
                                # add age where we came from
                                message = "<<"+self.IGetAgeDisplayName()+">>"+message[message.find(pwords[1])+len(pwords[1])+1:]
                                foundBuddy = 1
                                # add to recents folder
                                self.IAddPlayerToRecents(eplyr.playerGetID())
                                break
                            # see if they start the same (name might have spaces!)
                            if plyrName.startswith(pwords[1]):
                                mrest = message[len(PtGetLocalizedString("KI.Commands.ChatPrivate"))+1:]
                                if mrest.startswith(plyrName):
                                    selPlyrList.append(ptPlayer(eplyr.playerGetName(),eplyr.playerGetID()))
                                    cflags.private = 1
                                    cflags.interAge = 1
                                    # add age where we came from
                                    message = "<<"+self.IGetAgeDisplayName()+">>"+mrest[len(plyrName)+1:]
                                    foundBuddy = 1
                                    # add to recents folder
                                    self.IAddPlayerToRecents(eplyr.playerGetID())
                                    break
            if not foundBuddy:
                PtDebugPrint("xKI:SendRTChat: /p command can't find player %s"%(pwords[1]),level=kDebugDumpLevel)
                self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.CannotFindBuddy", [pwords[1]]),kChatSystemMessage)
                return
        elif message.startswith(str(PtGetLocalizedString("KI.Commands.ChatNeighbors"))):
            cflags.neighbors = 1
            message = message[len(PtGetLocalizedString("KI.Commands.ChatNeighbors"))+1:]
            neighbors = self.IGetNeighbors()
            if type(neighbors) != type(None):
                selPlyrList = self.IFindGroupOfBuddies(neighbors.getChildNodeRefList())
            else:
                selPlyrList = []
            if len(selPlyrList) == 0:
                self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in your neighbor list"]),kChatSystemMessage)
                return
            # yup, so set the message of where we were
            else:
                cflags.interAge = 1
                # add age where we came from
                message = "<<"+self.IGetAgeDisplayName()+">>"+message
                goesToFolder = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)
        elif message.startswith(str(PtGetLocalizedString("KI.Commands.ChatBuddies"))):
            vault = ptVault()
            buddies = vault.getBuddyListFolder()
            message = message[len(PtGetLocalizedString("KI.Commands.ChatBuddies"))+1:]
            if type(buddies) != type(None):
                selPlyrList = self.IFindGroupOfBuddies(buddies.getChildNodeRefList())
            else:
                selPlyrList = []
            if len(selPlyrList) == 0:
                self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in your buddy list"]),kChatSystemMessage)
                return
            # yup, so set the message of where we were
            else:
                cflags.interAge = 1
                # add age where we came from
                message = "<<"+self.IGetAgeDisplayName()+">>"+message
                goesToFolder = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)
        else:
            if iselect >= 0 and iselect < len(BKPlayerList):
                toplyr = BKPlayerList[iselect]
                if isinstance(toplyr, ptPlayer):
                    selPlyrList.append(toplyr)
                    cflags.private = 1
                    # add to recents folder
                    self.IAddPlayerToRecents(toplyr.getPlayerID())
                    PtDebugPrint("xKI:SendRTChat: private message to %s"%(toplyr.getPlayerName()),level=kDebugDumpLevel)
                elif isinstance(toplyr, ptVaultNodeRef):
                    # then send to player that might be in another age
                    eplyr = toplyr.getChild()
                    eplyr = eplyr.upcastToPlayerInfoNode()
                    if type(eplyr) != type(None):
                        if not eplyr.playerIsOnline():
                            self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.WentOffline", [eplyr.playerGetName()]),kChatSystemMessage)
                            return  # short circuit the rest of this
                        selPlyrList.append(ptPlayer(eplyr.playerGetName(),eplyr.playerGetID()))
                        # if the player is in the same age, then just send private message
                        cflags.private = 1
                        # add to recents folder
                        self.IAddPlayerToRecents(eplyr.playerGetID())
                        if PtValidateKey(PtGetAvatarKeyFromClientID(eplyr.playerGetID())):
                            # they're in the age
                            pass
                        else:
                            # oops, they're not in this age... send interAge
                            cflags.interAge = 1
                            # add age where we came from
                            message = "<<"+self.IGetAgeDisplayName()+">>"+message
                elif isinstance(toplyr,ptVaultPlayerInfoListNode):
                    # its a player list, display its name
                    fldrType = toplyr.folderGetType()
                    # if its a list of age owners... must be list of neighbors
                    if fldrType == PtVaultStandardNodes.kAgeOwnersFolder:
                        fldrType = PtVaultStandardNodes.kHoodMembersFolder
                        cflags.neighbors = 1
                    selPlyrList = self.IFindGroupOfBuddies(toplyr.getChildNodeRefList())
                    # was there anyone online to send to?
                    if len(selPlyrList) == 0:
                        self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in list"]),kChatSystemMessage)
                        return
                    # yup, so set the message of where we were
                    else:
####===> not sure how to do this, but only interage when the player is in a different age
####===> ... I guess that would mean two messages to be sent
                        cflags.interAge = 1
                        # add age where we came from
                        message = "<<"+self.IGetAgeDisplayName()+">>"+message
                        goesToFolder = xLocTools.FolderIDToFolderName(fldrType)
                elif isinstance(toplyr,kiFolder):
                    # else we are just spazzin' to everyone, but only those within listening distance
                    if self.autoShout:
                        listenerOnly = 0
                        selPlyrList = []
                    else:
                        listenerOnly = 1
                        selPlyrList = self.IGetPlayersInChatDistance()
                        ageplayers = PtGetPlayerListDistanceSorted()
                        if len(ageplayers) > 0 and len(selPlyrList) == 0:
                            betterTellEm = 1
                elif isinstance(toplyr,MarkerPlayer):
                    selPlyrList.append(toplyr.player)
                    cflags.private = 1
                    PtDebugPrint("xKI:SendRTChat: private message to %s"%(toplyr.player.getPlayerName()),level=kDebugDumpLevel)
                elif isinstance(toplyr,MarkerGame):
                    for mplayer in toplyr.greenTeamPlayers + toplyr.redTeamPlayers:
                        selPlyrList.append(mplayer.player)
                    goesToFolder = PtGetLocalizedString("KI.Chat.MarkerAllTeams")
                elif isinstance(toplyr,DPLBranchStatusLine):
                    if type(CurrentPlayingMarkerGame) != type(None):
                        if toplyr == CurrentPlayingMarkerGame.greenTeamDPL:
                            for mplayer in CurrentPlayingMarkerGame.greenTeamPlayers:
                                selPlyrList.append(mplayer.player)
                            goesToFolder = PtGetLocalizedString("KI.Chat.MarkerGreenTeam")
                        elif toplyr == CurrentPlayingMarkerGame.redTeamDPL:
                            for mplayer in CurrentPlayingMarkerGame.redTeamPlayers:
                                selPlyrList.append(mplayer.player)
                            goesToFolder = PtGetLocalizedString("KI.Chat.MarkerRedTeam")
                else:
                    # error - unknown player type
                    pass
            else:
                # error - nothing selected
                pass
        # add in our chat channel
        cflags.channel = PrivateChatChannel
        if len(selPlyrList) == 0 and listenerOnly:
            # Don't send... cause its going to noone
            # ...do we want to display an error message... might be annoying
            if betterTellEm:
                self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.NoOneListening"),kChatSystemMessage)
            pass
        else:
            if not PhasedKIInterAgeChat and cflags.interAge:
                self.IAddRTChat(None,PtGetLocalizedString("KI.Chat.InterAgeNotAvailable"),kChatSystemMessage)
                return
            PtSendRTChat(PtGetLocalPlayer(), selPlyrList, message,cflags.flags)
        sender = PtGetLocalPlayer()
        if cflags.private:
            sender = selPlyrList[0]
        if cflags.interAge:
            if goesToFolder:
                sender = ptPlayer(str(goesToFolder),0)
            else:
                sender = selPlyrList[0]
        elif goesToFolder:
            sender = ptPlayer(str(goesToFolder),0)
        self.IAddRTChat(sender,message,cflags)

    def ICheckChatCommands(self,chatmessage):
        "Check for any chat commands at front of chat message. Returns message"
        global ChatLogFile
        global AmICCR
        global gFeather
        # if we are a CCR then look for CCR special commands
        if AmICCR:
            if string.lower(chatmessage).startswith("//begin"):
                pid,ccrmsg = self.IGetPIDMsg(chatmessage[len("//begin"):])
                if pid:
                    res = ptCCRMgr().beginCommunication(pid,ccrmsg)
                    if res >= 0:
                        self.IAddRTChat(None,"<BEGIN(%u)>"%(pid)+ccrmsg,kChatCCRMessageSelf)
                    else:
                        self.IAddRTChat(None,self.ICCRErrorMsg(res),kChatSystemMessage)
                else:
                    self.IAddRTChat(None,"Player ID needed",kChatSystemMessage)
                return None
            if string.lower(chatmessage).startswith("//send"):
                pid,ccrmsg = self.IGetPIDMsg(chatmessage[len("//send"):])
                if pid:
                    res = ptCCRMgr().sendCommunication(pid,ccrmsg)
                    if res >= 0:
                        self.IAddRTChat(None,"<SEND(%u)>"%(pid)+ccrmsg,kChatCCRMessageSelf)
                    else:
                        self.IAddRTChat(None,self.ICCRErrorMsg(res),kChatSystemMessage)
                else:
                    self.IAddRTChat(None,"Player ID needed",kChatSystemMessage)
                return None
            if string.lower(chatmessage).startswith("//end"):
                pid,ccrmsg = self.IGetPIDMsg(chatmessage[len("//end"):])
                if pid:
                    res = ptCCRMgr().endCommunication(pid)
                    if res >= 0:
                        self.IAddRTChat(None,"<END(%u)>"%(pid),kChatCCRMessageSelf)
                    else:
                        self.IAddRTChat(None,self.ICCRErrorMsg(res),kChatSystemMessage)
                else:
                    self.IAddRTChat(None,"Player ID needed",kChatSystemMessage)
                return None
            if string.lower(chatmessage).startswith("//system"):
                ccrmsg = chatmessage[len("//system"):]
                ptCCRMgr().systemMessage(ccrmsg)
                self.IAddRTChat(None,"<SYSTEM>"+ccrmsg,kChatCCRMessageSelf)
                return None
        for petCommand in kChatPetitionCommands.keys():
            try:
                if string.lower(chatmessage).startswith(petCommand):
                    #petmessage = chatmessage[len(petCommand):]
                    #PtSendPetitionToCCR(petmessage,str(kChatPetitionCommands[petCommand]),str(PtGetLocalizedString("KI.CCR.PetitionTitle")))
                    #self.IAddRTChat(None,PtGetLocalizedString("KI.CCR.PetitionSent", [string.capitalize(petCommand[1:]),petmessage]),kChatCCRMessageSelf)
                    #
                    # no petition commands from the chat line... yet
                    self.IAddRTChat(None,PtGetLocalizedString("KI.Errors.CommandError", [chatmessage]),kChatSystemMessage)
                    return None
            except UnicodeDecodeError:
                self.IAddRTChat(None,PtGetLocalizedString("KI.Errors.TextOnly"),kChatSystemMessage)
                return None
        if string.lower(chatmessage).startswith(str(PtGetLocalizedString("KI.Commands.CCR"))):
            if CCRConversationInProgress:
                ccrmessage = chatmessage[len(PtGetLocalizedString("KI.Commands.CCR")):]
                # CCRConversationInProgress also holds the CCR playerid
                PtSendChatToCCR(ccrmessage,CCRConversationInProgress)
                self.IAddRTChat(None,ccrmessage,kChatCCRMessageSelf)
                return None
            else:
                self.IAddRTChat(None,PtGetLocalizedString("KI.CCR.NoCCR"),kChatSystemMessage)
                return None
        if string.lower(chatmessage) == str(PtGetLocalizedString("KI.Commands.ChatClearAll")):
            chatareaU = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kChatDisplayArea))
            chatareaM = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kChatDisplayArea))
            chatareaU.clearBuffer()
            chatareaM.clearBuffer()
            return None
        if string.lower(chatmessage).startswith(PtGetLocalizedString("KI.Commands.ChatStartLog")):
            if type(ChatLogFile) == type(None):
                ChatLogFile = ptStatusLog()
            ChatLogFile.open("Chat.log",30, int(PtStatusLogFlags.kAppendToLast) + int(PtStatusLogFlags.kTimestamp))
            self.IDoStatusChatMessage(PtGetLocalizedString("KI.Chat.LogStarted"),netPropagate=0)
            return None
        if string.lower(chatmessage).startswith(PtGetLocalizedString("KI.Commands.ChatStopLog")):
            if type(ChatLogFile) != type(None):
                if ChatLogFile.isOpen():
                    self.IDoStatusChatMessage(PtGetLocalizedString("KI.Chat.LogStopped"),netPropagate=0)
                ChatLogFile.close()
            return None
        if string.lower(chatmessage).startswith(PtGetLocalizedString("KI.Commands.AddBuddy")):
            pid,msg = self.IGetPIDMsg(chatmessage[len(PtGetLocalizedString("KI.Commands.AddBuddy")):])
            if pid:
                localplayer = PtGetLocalPlayer()
                if pid != localplayer.getPlayerID():
                    vault = ptVault()
                    buddies = vault.getBuddyListFolder()
                    if type(buddies) != type(None):
                        if buddies.playerlistHasPlayer(pid):
                            self.IAddRTChat(None,PtGetLocalizedString("KI.Player.AlreadyAdded"),kChatSystemMessage)
                        else:
                            buddies.playerlistAddPlayer(pid)
                            self.IDoStatusChatMessage(PtGetLocalizedString("KI.Player.Added"),netPropagate=0)
                else:
                    self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NotYourself"),kChatSystemMessage)
            else:
                self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NumberOnly"),kChatSystemMessage)
            return None
        if string.lower(chatmessage).startswith(PtGetLocalizedString("KI.Commands.RemoveBuddy")):
            pid,ccrmsg = self.IGetPIDMsg(chatmessage[len(PtGetLocalizedString("KI.Commands.RemoveBuddy")):])
            if pid:
                vault = ptVault()
                buddies = vault.getBuddyListFolder()
                if type(buddies) != type(None):
                    if buddies.playerlistHasPlayer(pid):
                        buddies.playerlistRemovePlayer(pid)
                        self.IDoStatusChatMessage(PtGetLocalizedString("KI.Player.Removed"),netPropagate=0)
                    else:
                        self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NotFound"),kChatSystemMessage)
            else:
                # check the ignore list to see if they are there by name
                vault = ptVault()
                buddies = vault.getBuddyListFolder()
                if type(buddies) != type(None):
                    buddyrefs = buddies.getChildNodeRefList()
                    theName = string.lstrip(chatmessage[len(PtGetLocalizedString("KI.Commands.RemoveBuddy")):])
                    for plyr in buddyrefs:
                        if isinstance(plyr,ptVaultNodeRef):
                            PLR = plyr.getChild()
                            PLR = PLR.upcastToPlayerInfoNode()
                            # its an element.. should be a player
                            if type(PLR) != type(None) and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                if theName.startswith(PLR.playerGetName()):
                                    # found them
                                    buddies.playerlistRemovePlayer(PLR.playerGetID())
                                    self.IDoStatusChatMessage(PtGetLocalizedString("KI.Player.Removed"),netPropagate=0)
                                    return None
                self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NumberOnly"),kChatSystemMessage)
            return None
        if string.lower(chatmessage).startswith(PtGetLocalizedString("KI.Commands.Ignore")):
            pid,ccrmsg = self.IGetPIDMsg(chatmessage[len(PtGetLocalizedString("KI.Commands.Ignore")):])
            if pid:
                localplayer = PtGetLocalPlayer()
                if pid != localplayer.getPlayerID():
                    vault = ptVault()
                    ignores = vault.getIgnoreListFolder()
                    if type(ignores) != type(None):
                        if ignores.playerlistHasPlayer(pid):
                            self.IAddRTChat(None,PtGetLocalizedString("KI.Player.AlreadyAdded"),kChatSystemMessage)
                        else:
                            ignores.playerlistAddPlayer(pid)
                            self.IDoStatusChatMessage(PtGetLocalizedString("KI.Player.Added"),netPropagate=0)
                else:
                    self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NotYourself"),kChatSystemMessage)
            else:
                self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NumberOnly"),kChatSystemMessage)
            return None
        if string.lower(chatmessage).startswith(str(PtGetLocalizedString("KI.Commands.Unignore"))):
            pid,ccrmsg = self.IGetPIDMsg(chatmessage[len(PtGetLocalizedString("KI.Commands.Unignore")):])
            if pid:
                vault = ptVault()
                ignores = vault.getIgnoreListFolder()
                if type(ignores) != type(None):
                    if ignores.playerlistHasPlayer(pid):
                        ignores.playerlistRemovePlayer(pid)
                        self.IDoStatusChatMessage(PtGetLocalizedString("KI.Player.Removed"),netPropagate=0)
                    else:
                        self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NotFound"),kChatSystemMessage)
            else:
                # check the ignore list to see if they are there by name
                vault = ptVault()
                ignores = vault.getIgnoreListFolder()
                if type(ignores) != type(None):
                    ignorerefs = ignores.getChildNodeRefList()
                    theName = string.lstrip(chatmessage[len(PtGetLocalizedString("KI.Commands.Unignore")):])
                    for plyr in ignorerefs:
                        if isinstance(plyr,ptVaultNodeRef):
                            PLR = plyr.getChild()
                            PLR = PLR.upcastToPlayerInfoNode()
                            # its an element.. should be a player
                            if type(PLR) != type(None) and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                if theName.startswith(PLR.playerGetName()):
                                    # found them
                                    ignores.playerlistRemovePlayer(PLR.playerGetID())
                                    self.IDoStatusChatMessage(PtGetLocalizedString("KI.Player.Removed"),netPropagate=0)
                                    return None
                self.IAddRTChat(None,PtGetLocalizedString("KI.Player.NumberOnly"),kChatSystemMessage)
            return None
        if string.lower(chatmessage).startswith(str(PtGetLocalizedString("KI.Commands.AutoShout"))):
            self.autoShout = abs(self.autoShout - 1)
            if self.autoShout:
                self.IAddRTChat(None, PtGetLocalizedString("KI.Messages.AutoShoutEnabled"), kChatBroadcastMsg)
            else:
                self.IAddRTChat(None, PtGetLocalizedString("KI.Messages.AutoShoutDisabled"), kChatBroadcastMsg)
            return None
        if string.lower(chatmessage).startswith(str(PtGetLocalizedString("KI.Commands.DumpLogs"))):
            destination = chatmessage[len(PtGetLocalizedString("KI.Commands.DumpLogs")):]
            destination = destination.strip() # remove whitespace
            if destination == "":
                self.IAddRTChat(None,PtGetLocalizedString("KI.Errors.MalformedLogDumpCmd"),kChatSystemMessage)
                return None
            # we are using a timer here so that we can print out some last status messages to the log before
            # the log is dumped to it's new home
            print "-- Logs dumped to: \"" + destination + "\" at " + time.strftime("%d %b %Y %H:%M:%S (GMT)", time.gmtime())
            self.logDumpDest = destination # so the timer can get at it
            PtAtTimeCallback(self.key,0.25,kDumpLogsTimer)
            return None
        if string.lower(chatmessage).startswith(str(PtGetLocalizedString("KI.Commands.DumpLog"))):
            # cause people are too damn lazy to type the s character
            destination = chatmessage[len(PtGetLocalizedString("KI.Commands.DumpLog")):]
            destination = destination.strip() # remove whitespace
            if destination == "":
                self.IAddRTChat(None,PtGetLocalizedString("KI.Errors.MalformedLogDumpCmd"),kChatSystemMessage)
                return None
            # we are using a timer here so that we can print out some last status messages to the log before
            # the log is dumped to it's new home
            print "-- Logs dumped to: \"" + destination + "\" at " + time.strftime("%d %b %Y %H:%M:%S (GMT)", time.gmtime())
            self.logDumpDest = destination # so the timer can get at it
            PtAtTimeCallback(self.key,0.25,kDumpLogsTimer)
            return None
        if string.lower(chatmessage).startswith(str(PtGetLocalizedString("KI.Commands.ChangePassword"))):
            newpassword = chatmessage[len(PtGetLocalizedString("KI.Commands.ChangePassword")):].strip()
            if newpassword == "":
                self.IAddRTChat(None, PtGetLocalizedString("KI.Errors.BadPassword"), kChatSystemMessage)
                return None
            elif len(newpassword) > 15:
                self.IAddRTChat(None, PtGetLocalizedString("KI.Errors.PasswordTooLong"), kChatSystemMessage)
                return None
            PtChangePassword(newpassword)
            return None
        if string.lower(chatmessage).startswith(str(PtGetLocalizedString("KI.Commands.SendFriendInvite"))):
            commands = chatmessage[len(PtGetLocalizedString("KI.Commands.SendFriendInvite")):].strip().split(" ", 1)
            emailaddr = commands[0]
            toName = None
            
            if len(commands) == 2:
                toName = xCensor.xCensor(commands[1], theCensorLevel)
            if emailaddr == "":
                self.IAddRTChat(None, PtGetLocalizedString("KI.Errors.MissingEmailAddress"), kChatSystemMessage)
                return None
            elif len(emailaddr) > 63:
                self.IAddRTChat(None, PtGetLocalizedString("KI.Errors.EmailAddressTooLong"), kChatSystemMessage)
                return None
            
            if toName and len(toName) > 0:
                PtSendFriendInvite(emailaddr, toName)
            else:
                PtSendFriendInvite(emailaddr)
            
            return None
        if string.lower(chatmessage).startswith(str("/savecolumns")) and AgeName == "Jalak":
            fName = chatmessage[13:].strip()
            if fName:
                fName = fName + ".txt"
            else:
                fName = "JalakColumns.txt"
            self.SendNote('self.SaveColumns("%s")' % (fName))
            return None
        if string.lower(chatmessage).startswith(str("/loadcolumns")) and AgeName == "Jalak":
            fName = chatmessage[13:].strip()
            if fName:
                fName = fName + ".txt"
            else:
                fName = "JalakColumns.txt"
            self.SendNote('self.LoadColumns("%s")' % (fName))
            return None
        if PtIsInternalRelease() and chatmessage == "/revisitcleft":
            # find the cleft chronicle and delete it
            vault = ptVault()
            chron = vault.findChronicleEntry("CleftSolved")
            if type(chron) != type(None):
                chronFolder = vault.getChronicleFolder()
                if type(chronFolder) != type(None):
                    chronFolder.removeNode(chron)
            return None
        if PtIsInternalRelease() and chatmessage == "/restart":
            # find the cleft chronicle and delete it
            vault = ptVault()
            chron = vault.findChronicleEntry("InitialAvCustomizationsDone")
            if type(chron) != type(None):
                chronFolder = vault.getChronicleFolder()
                if type(chronFolder) != type(None):
                    chronFolder.removeNode(chron)
            chron = vault.findChronicleEntry("IntroPlayed")
            if type(chron) != type(None):
                chronFolder = vault.getChronicleFolder()
                if type(chronFolder) != type(None):
                    chronFolder.removeNode(chron)
            chron = vault.findChronicleEntry("CleftSolved")
            if type(chron) != type(None):
                chronFolder = vault.getChronicleFolder()
                if type(chronFolder) != type(None):
                    chronFolder.removeNode(chron)
            return None
        if chatmessage == "/look":
            plist = self.IRemoveCCRPlayers(self.IGetPlayersInChatDistance(minPlayers=-1))
            people = "nobody in particular"
            if len(plist) > 0:
                people = ""
                for p in plist:
                    people += p.getPlayerName() + ", "
                people = people[:-2]
            loc = self.IGetAgeFileName()
            see = ""
            exits = "North and West"
            if loc == "city":
                see = "  You see the remnants of a great civilization, ready to be rebuilt. Where are the flying monkeys?\n"
                exits = "NorthWest and South"
            elif loc == "Personal":
                see = "  You see a small hut... looks deserted.\n"
                exits = "... well, there are no exits"
            elif loc == "Teledahn":
                see = "  You see 'shrooms everywhere! Big ones, small ones. Are they edible?\n"
                exits = "East"
            elif loc == "Nexus":
                see = "  You see a jukebox like machine.\n"
                exits = "... well, there are no exits"
            elif loc == "Garden":
                see = "  You see bugs.   BUGS! I hate bugs.\n"
                exits = "North and South"
            elif loc == "EderTsogal":
                see = "  You see grass, water and things floating in the air (not feathers).\n"
                exits = "North. But you'll have to climb or fly to get there"
            elif loc == "Dereno":
                see = "  Ah, Dah-Ree-Toe. You see... well, if someone would clean those stupid windows you could see a *lot*. Have I been here before? Maybe all pods just look the same.\n"
                exits = "SouthWest and East but they are both blocked"
            elif loc == "BahroCave":
                see = "  You see a darkly lit cavern. Strange images on the wall next to you, flickering in the subdued light.\nBe afraid. Be very afraid!\n"
                exits = "North, West and East... but they are blocked by a large hole in the floor"
            elif loc == "Minkata":
                see = "  You see sand and dust in all directions. Above you there is a filtered sun or two... or more.\nSomewhere there is a horse with no name.\n"
                exits = "to the east. Nine days away"
            elif loc == "Cleft":
                see = "  You see sand for as far as the eye can see. Gonna need a vehicle of some sort.\n"
                exits = "... well, I don't know. Maybe you can ask the old man (if he ever stops listening to that music!)"
                people = "an old man. Ok, maybe he's not standing. BTW, wasn't he on M*A*S*H?"
            self.IAddRTChat(None,"%s:\n%s  Standing near you is %s.\n  There are exits to the %s."%(self.IGetAgeDisplayName(),see,people,exits),0)
            return None
        if string.lower(chatmessage).startswith("/go ") or chatmessage == "/go":
            self.IAddRTChat(None,"Put one foot in front of the other and eventually you will get there.",0)
            return None
        if string.lower(chatmessage).startswith("/get feather"):
            loc = self.IGetAgeFileName()
            if loc == "Gira":
                if gFeather < 7:
                    self.IAddRTChat(None,"You pick up a plain feather and put it in your pocket. I know you didn't see yourself do that... trust me, you have a feather in your pocket.",0)
                    gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if type(entry) == type(None):
                        vault.addChronicleEntry("feather",1,"%d" % (gFeather))
                    else:
                        entry.chronicleSetValue("%d" % (gFeather))
                        entry.save()
                else:
                    self.IAddRTChat(None,"You can only carry seven plain feathers.",0)
            elif loc == 'EderDelin':
                if gFeather == 7:
                    self.IAddRTChat(None,"You search... and find the 'Red' feather and put it in your pocket.",0)
                    gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if type(entry) == type(None):
                        vault.addChronicleEntry("feather",1,"%d" % (gFeather))
                    else:
                        entry.chronicleSetValue("%d" % (gFeather))
                        entry.save()
                elif gFeather > 7:
                    self.IAddRTChat(None,"You search... but find no other feathers.",0)
                else:
                    self.IAddRTChat(None,"You search... but then suddenly stop when you realize that you are missing seven plain feathers.",0)
            elif loc == 'Dereno':
                if gFeather == 8:
                    self.IAddRTChat(None,"You search... and find the 'Blue' feather and put it in your pocket.",0)
                    gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if type(entry) == type(None):
                        vault.addChronicleEntry("feather",1,"%d" % (gFeather))
                    else:
                        entry.chronicleSetValue("%d" % (gFeather))
                        entry.save()
                elif gFeather > 8:
                    self.IAddRTChat(None,"You search... but find no other feathers.",0)
                else:
                    self.IAddRTChat(None,"You search... but then suddenly stop when you realize that you are missing the 'Red' feather.",0)
            elif loc == 'Payiferen':
                if gFeather == 9:
                    self.IAddRTChat(None,"You search... and find the 'Black' feather and put it in your pocket.",0)
                    gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if type(entry) == type(None):
                        vault.addChronicleEntry("feather",1,"%d" % (gFeather))
                    else:
                        entry.chronicleSetValue("%d" % (gFeather))
                        entry.save()
                elif gFeather > 9:
                    self.IAddRTChat(None,"You search... but find no other feathers.",0)
                else:
                    self.IAddRTChat(None,"You search... but then suddenly stop when you realize that you are missing the 'Blue' feather.",0)
            elif loc == 'Ercana':
                if gFeather == 10:
                    self.IAddRTChat(None,"You search... and find the 'Silver' feather and put it in your pocket.",0)
                    gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if type(entry) == type(None):
                        vault.addChronicleEntry("feather",1,"%d" % (gFeather))
                    else:
                        entry.chronicleSetValue("%d" % (gFeather))
                        entry.save()
                elif gFeather > 10:
                    self.IAddRTChat(None,"You search... but find no other feathers.",0)
                else:
                    self.IAddRTChat(None,"You search... but then suddenly stop when you realize that you are missing the 'Black' feather.",0)
            elif loc == 'Jalak':
                if gFeather == 11:
                    self.IAddRTChat(None,"You search... and find the 'Duck' feather and put it in your pocket.",0)
                    gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if type(entry) == type(None):
                        vault.addChronicleEntry("feather",1,"%d" % (gFeather))
                    else:
                        entry.chronicleSetValue("%d" % (gFeather))
                        entry.save()
                elif gFeather > 11:
                    self.IAddRTChat(None,"You search... but find no other feathers.",0)
                else:
                    self.IAddRTChat(None,"You search... but then suddenly stop when you realize that you are missing the 'Silver' feather.",0)
            elif loc == 'Ahnonay':
                if gFeather == 12:
                    self.IAddRTChat(None,"You search... and find a large 'Rukh' feather (how could you have missed it?) and put it in your pocket.",0)
                    gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if type(entry) == type(None):
                        vault.addChronicleEntry("feather",1,"%d" % (gFeather))
                    else:
                        entry.chronicleSetValue("%d" % (gFeather))
                        entry.save()
                elif gFeather > 12:
                    self.IAddRTChat(None,"You search... but find no other feathers.",0)
                else:
                    self.IAddRTChat(None,"You search... but then suddenly stop when you realize that you are missing the 'Duck' feather.",0)
            else:
                self.IAddRTChat(None,"There are no feathers here.",0)
            return None
        if chatmessage == "/look in pocket":
            if gFeather:
                if gFeather == 1:
                    self.IAddRTChat(None,"You see a feather!",0)
                else:
                    pfeathers = gFeather
                    if pfeathers > 7:
                        pfeathers = 7
                    pOut = "You see %d plain feathers" % (pfeathers)
                    if gFeather>7:
                        pOut += " and a 'Red' feather"
                    if gFeather>8:
                        pOut += " and a 'Blue' feather"
                    if gFeather>9:
                        pOut += " and a 'Black' feather"
                    if gFeather>10:
                        pOut += " and a 'Silver' feather"
                    if gFeather>11:
                        pOut += " and a 'Duck' feather"
                    if gFeather>12:
                        pOut += " and a large 'Rukh' feather (sticking out of your pocket)"
                    pOut += "."
                    self.IAddRTChat(None,pOut,0)
            else:
                self.IAddRTChat(None,"There is nothing there but lint.",0)
            return None
        if string.lower(chatmessage).startswith("/fly"):
            self.IAddRTChat(None,"You close your eyes, you feel light headed and the ground slips away from your feet... Then you open your eyes and WAKE UP! (Ha, you can only dream about flying.)",0)
            return None
        if string.lower(chatmessage).startswith("/get "):
            if chatmessage[-1:] == "s":
                v = "are"
            else:
                v = "is"
            self.IAddRTChat(None,"The %s %s too heavy to lift. Maybe you should stick to feathers." % (chatmessage[len("/get "):],v),0)
            return None
        #
        # The check for emote commands should be last
        # search message for emote commmand (could embedd into message)
        if chatmessage.startswith('/'):
            # find the emote at the start of the message
            words = chatmessage.split()
            try:
                emote = xKIExtChatCommands.xChatEmoteXlate[unicode(string.lower(words[0][1:]))]
                PtEmoteAvatar(emote[0])
                if PtGetLanguage() == PtLanguage.kEnglish:
                    # find out what gender they are
                    avatar = PtGetLocalAvatar()
                    gender = avatar.avatar.getAvatarClothingGroup()
                    if gender > kFemaleClothingGroup:
                        gender = kMaleClothingGroup
                    hisher = PtGetLocalizedString("KI.EmoteStrings.His")
                    if gender == kFemaleClothingGroup:
                        hisher = PtGetLocalizedString("KI.EmoteStrings.Her")
                    statusMsg = PtGetLocalizedString(emote[1], [PtGetLocalPlayer().getPlayerName(),hisher])
                else:
                    statusMsg = PtGetLocalizedString(emote[1], [PtGetLocalPlayer().getPlayerName()])
                self.IDoStatusChatMessage(statusMsg)
                chatmessage = chatmessage[len(words[0]):]
                if chatmessage == "":
                    return None
                return chatmessage[1:]
            except LookupError:
                try:                    
                    command = xKIExtChatCommands.xChatExtendedChat[unicode(string.lower(words[0][1:]))]

                    if type(command) == type(""):
                        # see if there is text afterwords to be passed as arguments to the console command
                        args = chatmessage[len(words[0]):]
                        PtConsole(command+args)
                    else:
                        # must be a functions
                        try:
                            args = chatmessage[len(words[0])+1:]
                            if args != "":
                                try:
                                    retDisp = command(args)
                                except TypeError:
                                    retDisp = command()
                                    return args
                            else:
                                retDisp = command()
                            if (type(retDisp) == type(U"")) or (type(retDisp) == type("")):
                                self.IDoStatusChatMessage(retDisp,netPropagate=0)
                            elif type(retDisp) == type(()):
                                if retDisp[0]:
                                    self.IAddRTChat(None,retDisp[1],kChatSystemMessage)
                                else:
                                    self.IDoStatusChatMessage(retDisp[1],netPropagate=0)
                        except:
                            PtDebugPrint("xKI: chat command function did not run",command,level=kErrorLevel)
                    return None
                except LookupError:                    
                    # make sure that its not one of the special handled commands
                    if unicode(string.lower(words[0])) in xKIExtChatCommands.xChatSpecialHandledCommands:
                        # let the later processing handle these commands
                        return chatmessage
                    else:
                        # if they miss typed then error message
                        self.IAddRTChat(None,PtGetLocalizedString("KI.Errors.CommandError", [chatmessage]),kChatSystemMessage)
                    
                    return None
        return chatmessage

    def IGetPIDMsg(self,message):
        lmsg = message.split()
        try:
            pid = long(string.atoi(lmsg[0]))
            omsg = message[message.index(lmsg[0])+len(lmsg[0]):]
            return pid,omsg
        except IndexError:
            return 0, ""
        except ValueError:
            # try to find their buddy in the DPL online lists
            for daPlayer in BKPlayerList:
                if isinstance(daPlayer, ptPlayer):
                    plyrName = daPlayer.getPlayerName()
                    if lmsg[0] == plyrName:
                        # remove the '/p buddyname' from the message
                        message = message[message.find(lmsg[0])+len(lmsg[0])+1:]
                        return daPlayer.getPlayerID(),message
                    # see if they start the same (name might have spaces!)
                    if plyrName.startswith(lmsg[0]):
                        if message.startswith(plyrName):
                            # remove the '/p buddyname' from the message
                            message = message[len(plyrName)+1:]
                        return daPlayer.getPlayerID(),message
            # otherwise if none found...
            return 0, ""

    def ICCRErrorMsg(self,res):
        if res == -1:
            return "Unknown error"
        elif res == -2:
            return "Not Authorized"
        elif res == -3:
            return "Nil Local Avatar"
        elif res == -4:
            return " CCR Already Allocated"
        elif res == -5:
            return " Networking Is Disabled"
        elif res == -6:
            return "Can't Find Player"
        elif res == -7:
            return "Invalid Level"
        elif res == -8:
            return "Player Not In Age"

    def IAddRTChat(self,player,message,cflags,forceKI=1):
        "Add message to the RTChat listbox"
        global LastPrivatePlayerID
        global ChatLogFile
        global kInternalDev
        if PtIsSinglePlayerMode():
            return
        
        message = str(message) # HACK, so we handle unicode (this isn't really the best way)

        #This is to get rid of any annoying log filling admin messages that tell you to log out and then log back in!
        #They're so ANNOYING!
        if kInternalDev:
            if message.find("Uru has been updated.") > -1:
                return
        
        print "xKI:IAddRTChat: message=%s"%(message),player,cflags
        
        # Begin Fix for CoD --> Character of Doom
        (message, RogueCount) = re.subn('[\x00-\x08\x0a-\x1f]', '', message)
        # End Fix for CoD
        
        if theKILevel == kMicroKI or theKILevel == kNanoKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        pretext = U""
        headerColor = ChatHeaderBroadcastColor
        bodyColor = ChatMessageColor
        if isinstance(cflags, ChatFlags):
            if cflags.status:
                bodyColor = ChatHeaderStatusColor
                player = None
            elif cflags.interAge:
                if cflags.private:
                    headerColor = ChatHeaderPrivateColor
                    # force the forcing the of the KI when its a private message
                    forceKI = 1
                else:
                    if cflags.neighbors:
                        headerColor = ChatHeaderNeighborsColor
                    else:
                        headerColor = ChatHeaderBuddiesColor
                if cflags.toSelf:
                    pretext = PtGetLocalizedString("KI.Chat.InterAgeSendTo")
                    if message[:2] == "<<":
                        try:
                            idx = message.index(">>")
                            message = message[idx+2:]
                        except ValueError:
                            pass
                else:
                    if not self.IIfOnlyBuddyCheck(player.getPlayerID()):
                        return
                    pretext = PtGetLocalizedString("KI.Chat.InterAgeMsgRecvd")
                    # force the forcing the of the KI when its a private message
                    forceKI = 1
                    if message[:2] == "<<":
                        try:
                            idx = message.index(">>")
                            player = ptPlayer(str(PtGetLocalizedString("KI.Chat.InterAgePlayerRecvd", [player.getPlayerName(),message[2:idx]])),player.getPlayerID())
                            message = message[idx+2:]
                        except ValueError:
                            pass
                    # save the playerid for reply
                    if cflags.private:
                        LastPrivatePlayerID = (player.getPlayerName(),player.getPlayerID(),1)
                        # add to recents folder
                        self.IAddPlayerToRecents(player.getPlayerID())
            elif cflags.admin:
                if cflags.private:
                    headerColor = ChatHeaderErrorColor
                    pretext = PtGetLocalizedString("KI.Chat.PrivateMsgRecvd")
                    # force the forcing the of the KI when its a private message
                    forceKI = 1
                else:
                    headerColor = ChatHeaderCCRColor
                    # force the forcing the of the KI when its a private message
                    forceKI = 1
                    PtDebugPrint("xKI: an admin broadcast message!",level=kWarningLevel)
            elif cflags.broadcast:
                if cflags.toSelf:
                    headerColor = ChatHeaderBroadcastColor
                    pretext = PtGetLocalizedString("KI.Chat.BroadcastSendTo")
                else:
                    headerColor = ChatHeaderBroadcastColor
                    pretext = PtGetLocalizedString("KI.Chat.BroadcastMsgRecvd")
                    # add to recents folder
                    self.IAddPlayerToRecents(player.getPlayerID())
            elif cflags.private:
                if cflags.toSelf:
                    headerColor = ChatHeaderPrivateColor
                    pretext = PtGetLocalizedString("KI.Chat.PrivateSendTo")
                else:
                    if not self.IIfOnlyBuddyCheck(player.getPlayerID()):
                        return
                    headerColor = ChatHeaderPrivateColor
                    pretext = PtGetLocalizedString("KI.Chat.PrivateMsgRecvd")
                    # force the forcing the of the KI when its a private message
                    forceKI = 1
                    # save the playerid for reply
                    LastPrivatePlayerID = (player.getPlayerName(),player.getPlayerID(),0)
                    # add to recents folder
                    self.IAddPlayerToRecents(player.getPlayerID())
        else:
            # else the cflags is just a number
            if cflags == kChatSystemMessage:
                headerColor = ChatHeaderErrorColor
                pretext = PtGetLocalizedString("KI.Chat.ErrorMsgRecvd")
            elif cflags == kChatCCRMessage:
                headerColor = ChatHeaderCCRColor
                pretext = PtGetLocalizedString("KI.Chat.CCRMsgRecvd")
            elif cflags == kChatCCRMessageSelf:
                headerColor = ChatHeaderCCRColor
                pretext = PtGetLocalizedString("KI.Chat.CCRSendTo")
            elif cflags == kChatCCRMessageFromPlayer:
                headerColor = ChatHeaderCCRColor
            else:
                headerColor = ChatHeaderBroadcastColor
                pretext = PtGetLocalizedString("KI.Chat.BroadcastMsgRecvd")
        # make sure the miniKI is up, if needs to be forced up
        if forceKI and not IKIDisabled and not mKIdialog.isEnabled():
            mKIdialog.show()
        if type(player) != type(None):
            chatHeaderFormatted = pretext + unicode(player.getPlayerName()) + U":"
            chatMessageFormatted =  U" " + unicode(message)
        else:
            # must be a system type or error message
            chatHeaderFormatted = pretext
            if pretext == U"":
                chatMessageFormatted = unicode(message)
            else:
                chatMessageFormatted = " " + unicode(message)
        
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kChatDisplayArea))
        chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
        chatarea.insertStringW(U"\n")
        chatarea.insertColor(headerColor)
        
        #Added unicode support here!
        chatarea.insertStringW(chatHeaderFormatted)
        chatarea.insertColor(bodyColor)
        
        #Added unicode support here!
        chatarea.insertStringW(chatMessageFormatted)
        chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
        # see if we're logging
        if type(ChatLogFile) != type(None) and ChatLogFile.isOpen():
            ChatLogFile.write(chatHeaderFormatted[0:]+chatMessageFormatted)
        # see if we've overflowed the max lines
        if chatarea.getBufferSize() > kMaxChatSize:
            # remove from the start
            while chatarea.getBufferSize() > kMaxChatSize and chatarea.getBufferSize() > 0:
                PtDebugPrint("xKImini: max chat buffer size reached. Removing top line", level=kDebugDumpLevel)
                chatarea.deleteLinesFromTop(1)

            #chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
        # if this is the micro version then duplicate in miniKIs so its there when they switch
        if theKILevel == kMicroKI:
            chatarea2 = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kChatDisplayArea))
            chatarea2.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            chatarea2.insertStringW(U"\n")
            chatarea2.insertColor(headerColor)
            
            #Added unicode support here!
            chatarea2.insertStringW(chatHeaderFormatted)
            chatarea2.insertColor(bodyColor)
            
            #Added unicode support here!
            chatarea2.insertStringW(chatMessageFormatted)
            chatarea2.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            # see if we've overflowed the max lines
            if chatarea2.getBufferSize() > kMaxChatSize:
                # remove from the start
                while chatarea2.getBufferSize() > kMaxChatSize and chatarea2.getBufferSize() > 0:
                    PtDebugPrint("xKImini: max chat buffer size reached. Removing top line", level=kDebugDumpLevel)
                    chatarea2.deleteLinesFromTop(1)

                #chatarea2.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
        # set the scroll buttons (hide/show)
        mKIdialog.refreshAllControls()
        if not self.isChatting:
            self.IKillFadeTimer()
            self.IStartFadeTimer()

    def IIfOnlyBuddyCheck(self, playerID):
        "determine if there is a buddy check and if so, are they a buddy?"
        global OnlyGetPMsFromBuddies
        if OnlyGetPMsFromBuddies:
            # look thru their buddy list to see if this player is there
            buddies = ptVault().getBuddyListFolder()
            if type(buddies) != type(None):
                return buddies.playerlistHasPlayer(playerID)
            # if we can't find their buddies, better safe than sorry
            return 0
        return 1

    def IAddPlayerToRecents(self,playerID):
        global kMaxRecentPlayerListSize
        
        # check in the recents folder to see if they are still online
        vault = ptVault()
        PIKAfolder = vault.getPeopleIKnowAboutFolder()
        if PIKAfolder:
            PIKA = PIKAfolder.upcastToPlayerInfoListNode()
            if type(PIKA) != type(None):
                # only search it if it actually has the player in question
                if not PIKA.playerlistHasPlayer(playerID):
                    PIKA.playerlistAddPlayer(playerID)

                    childRefList = PIKAfolder.getChildNodeRefList()
                    numPeople = len(childRefList)
                    
                    if numPeople > kMaxRecentPlayerListSize:
                        peopleToRemove = []
                        numToRemove = numPeople - kMaxRecentPlayerListSize
                        for i in range(numToRemove):
                            peopleToRemove.append(childRefList[i].getChild())

                        for person in peopleToRemove:
                            PIKAfolder.removeNode(person)


    def IDoStatusChatMessage(self,statusMessage,netPropagate=1):
        "Send a status chat message to everyone in listen distance for now"
        cflags = ChatFlags(0)
        cflags.toSelf = 1
        cflags.status = 1
        statusMessage = str(statusMessage) # HACK for unicode

        if netPropagate:
            plyrList = self.IGetPlayersInChatDistance()
            if len(plyrList) > 0:
                PtSendRTChat(PtGetLocalPlayer(), plyrList, statusMessage,cflags.flags)
        
        self.IAddRTChat(None,statusMessage,cflags)

    def IDoErrorChatMessage(self,errorMessage):
        "Send a status chat message to everyone in listen distance for now"
        self.IAddRTChat(None,errorMessage,kChatSystemMessage)

    def IScrollUpListbox(self,control,upbtnID,downbtnID):
        "Try to scroll the listbox up one line"
        if PtIsSinglePlayerMode():
            return
        if theKILevel == kMicroKI or theKILevel == kNanoKI:
            return
        elif theKILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        currPos = control.getScrollPos()
        if currPos < control.getScrollRange():
            PtDebugPrint("xKI: Scroll listbox UP from %d to %d" % (currPos,currPos+1),level=kDebugDumpLevel)
            control.setScrollPos(currPos+1)
        else:
            PtDebugPrint("xKI: Scroll listbox UP - No! currPos=%d and range=%d" % (currPos,control.getScrollRange()),level=kDebugDumpLevel)
            pass
        self.ICheckScrollButtons(control,upbtnID,downbtnID)
        mKIdialog.refreshAllControls()
        self.IKillFadeTimer()
        self.IStartFadeTimer()

    def IScrollDownListbox(self,control,upbtnID,downbtnID):
        "Try to scroll the listbox down one line"
        if PtIsSinglePlayerMode():
            return
        if theKILevel == kMicroKI or theKILevel == kNanoKI:
            return
        elif theKILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        currPos = control.getScrollPos()
        if currPos > 0:
            PtDebugPrint("xKI: Scroll Chat area DOWN from %d to %d" % (currPos,currPos-1),level=kDebugDumpLevel)
            control.setScrollPos(currPos-1)
        else:
            PtDebugPrint("xKI: Scroll Chat area DOWN - No! currPos=%d" % (currPos),level=kDebugDumpLevel)
            pass
        self.ICheckScrollButtons(control,upbtnID,downbtnID)
        mKIdialog.refreshAllControls()
        self.IKillFadeTimer()
        self.IStartFadeTimer()

    def ICheckScrollButtons(self,control,upbtnID,downbtnID):
        "Check to see if the scroll buttons should be visible or not"
        if PtIsSinglePlayerMode():
            return
        if theKILevel == kMicroKI or theKILevel == kNanoKI:
            return
        elif theKILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        currentPos = control.getScrollPos()
        PtDebugPrint("xKI: Check scroll currPos=%d and range=%d" % (currentPos,control.getScrollRange()),level=kDebugDumpLevel)
        try:
            dbtn = ptGUIControlButton(mKIdialog.getControlFromTag(downbtnID))
            if currentPos == 0:
                # then hide down arrow
                dbtn.hide()
            else:
                dbtn.show()
            ubtn = ptGUIControlButton(mKIdialog.getControlFromTag(upbtnID))
            if currentPos >= control.getScrollRange():
                ubtn.hide()
            else:
                ubtn.show()
        except KeyError:
            pass

    def IRefreshPlayerList(self,forceSmall=0):
        "Refresh the player list"
        global BKPlayerList
        global FolderOfDevices
        global PreviouslySelectedPlayer
        global MarkerGameState
        global MarkerPlayerList
        global PhasedKINeighborsInDPL
        global PhasedKIBuddies

        if PtIsSinglePlayerMode():
            return

        PtDebugPrint("xBigKI: refresh playerlist",level=kDebugDumpLevel)
        playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
        select = playerlist.getSelection()
        if select >= 0 and select < len(BKPlayerList):
            PreviouslySelectedPlayer = BKPlayerList[select]
            #Vault node refs change frequently so we cannot do a straight comparison
            #Altering to use the child Node's ID...  it is supposed to be unique
            if type(PreviouslySelectedPlayer) == ptVaultNodeRef:
                PtDebugPrint("xKI.IRefreshPlayerList() Player selected getting the vault node ID", level=kDebugDumpLevel)
                PreviouslySelectedPlayer = PreviouslySelectedPlayer.getChild().getID()

        else:
            PreviouslySelectedPlayer = None
        BKPlayerList = []
        vault = ptVault()
        # if the bigKI is showing
        if BigKI.dialog.isEnabled() and not forceSmall:
            # get the AgeMember and Buddy folders and fill in
            agemembers = kiFolder(PtVaultStandardNodes.kAgeMembersFolder)
            if type(agemembers) != type(None):
                BKPlayerList.append(agemembers)
                BKPlayerList += self.IRemoveCCRPlayers(PtGetPlayerListDistanceSorted())
            else:
                BKPlayerList.append("?NOAgeMembers?")
            if PhasedKIBuddies:
                buddies = vault.getBuddyListFolder()
                if type(buddies) != type(None):
                    BKPlayerList.append(buddies)
                    BKPlayerList += self.IRemoveOfflinePlayers(buddies.getChildNodeRefList())
                else:
                    BKPlayerList.append("?NOBuddies?")
            if PhasedKINeighborsInDPL:
                neighbors = self.IGetNeighbors()
                if type(neighbors) != type(None):
                    BKPlayerList.append(neighbors)
                    onlinePlayers = self.IRemoveOfflinePlayers(neighbors.getChildNodeRefList())

                    localplayer = PtGetLocalPlayer()
                    for idx in range(len(onlinePlayers)):
                        PLR = onlinePlayers[idx].getChild().upcastToPlayerInfoNode()
                        if PLR.playerGetID() == localplayer.getPlayerID():
                            del onlinePlayers[idx]
                            break
                    BKPlayerList += onlinePlayers
                else:
                    BKPlayerList.append("NEIGHBORS")
            if type(FolderOfDevices) != type(None) and len(FolderOfDevices) > 0:
                BKPlayerList.append(FolderOfDevices)
                for device in FolderOfDevices:
                    BKPlayerList.append(device)
            # is there an activate game
            if MarkerGameState == kMGNotActive:
                # no, but are we working on one?
                if type(WorkingMarkerFolder) != type(None):
                    markerGame = ptMarkerMgr().getWorkingMarkerFolder()
                    if type(markerGame) != type(None):
                        BKPlayerList.append(markerGame)
                        if markerGame.getGameType() != PtMarkerMsgGameType.kGameTypeQuest:
                            BKPlayerList += WorkingMarkerFolder.invitedPlayers
            else: 
                if type(CurrentPlayingMarkerGame) != type(None):
                    # add marker game to the DPL
                    if CurrentPlayingMarkerGame.gameType == PtMarkerMsgGameType.kGameTypeQuest:
                        pass
                        #TODO: what's this??? -- looks like it's commented out anyway....
                        ### don't display the quest game in the DPL
                        # markerGame = ptMarkerMgr().getWorkingMarkerFolder()
                        # if type(markerGame) != type(None):
                        # BKPlayerList.append(markerGame)
                    else:
                        CurrentPlayingMarkerGame.addToDPLPlaying(BKPlayerList)
                        # add any players that have not joined but were invited
                        if MarkerGameState == kMGGameCreation and type(WorkingMarkerFolder) != type(None):
                            inviteTitlePosted = 0
                            for invitePlayer in WorkingMarkerFolder.invitedPlayers:
                                if not invitePlayer.isJoined:
                                    if not inviteTitlePosted:
                                        BKPlayerList.append("Invited")
                                        inviteTitlePosted = 1
                                    BKPlayerList.append(invitePlayer)
        # else is the big KI is not showing
        else:
            # get the AgeMember and Buddy folders and fill in
            agemembers = kiFolder(PtVaultStandardNodes.kAgeMembersFolder)
            if type(agemembers) != type(None):
                BKPlayerList.append(agemembers)
                BKPlayerList += self.IRemoveCCRPlayers(PtGetPlayerListDistanceSorted())
            else:
                BKPlayerList.append("?NOAgeMembers?")
            if PhasedKIBuddies:
                buddies = vault.getBuddyListFolder()
                if type(buddies) != type(None):
                    BKPlayerList.append(buddies)
                    BKPlayerList += self.IRemoveOfflinePlayers(buddies.getChildNodeRefList())
                else:
                    BKPlayerList.append("?NOBuddies?")
            if PhasedKINeighborsInDPL:
                neighbors = self.IGetNeighbors()
                if type(neighbors) != type(None):
                    BKPlayerList.append(neighbors)
                    onlinePlayers = self.IRemoveOfflinePlayers(neighbors.getChildNodeRefList())

                    localplayer = PtGetLocalPlayer()
                    for idx in range(len(onlinePlayers)):
                        PLR = onlinePlayers[idx].getChild().upcastToPlayerInfoNode()
                        if PLR.playerGetID() == localplayer.getPlayerID():
                            del onlinePlayers[idx]
                            break
                    BKPlayerList += onlinePlayers
                else:
                    BKPlayerList.append("NEIGHBORS")
            
            # is there an activate game
            if MarkerGameState == kMGNotActive:
                # no, but are we working on one?
                if type(WorkingMarkerFolder) != type(None):
                    markerGame = ptMarkerMgr().getWorkingMarkerFolder()
                    if type(markerGame) != type(None):
                        BKPlayerList.append(markerGame)
                        if markerGame.getGameType() != PtMarkerMsgGameType.kGameTypeQuest:
                            BKPlayerList += WorkingMarkerFolder.invitedPlayers
            else:
                if type(CurrentPlayingMarkerGame) != type(None):
                    # if a quest then just add the name to the list
                    if CurrentPlayingMarkerGame.gameType == PtMarkerMsgGameType.kGameTypeQuest:
                        pass
                        ### don't display the quest game in the DPL
                        # markerGame = ptMarkerMgr().getWorkingMarkerFolder()
                        # if type(markerGame) != type(None):
                        # BKPlayerList.append(markerGame)
                    else:
                        # add marker game to the DPL
                        CurrentPlayingMarkerGame.addToDPLPlaying(BKPlayerList)
                        # add any players that have not joined but were invited
                        if MarkerGameState == kMGGameCreation and type(WorkingMarkerFolder) != type(None):
                            inviteTitlePosted = 0
                            for invitePlayer in WorkingMarkerFolder.invitedPlayers:
                                if not invitePlayer.isJoined:
                                    if not inviteTitlePosted:
                                        BKPlayerList.append("Invited")
                                        inviteTitlePosted = 1
                                    BKPlayerList.append(invitePlayer)

    def IRemoveOfflinePlayers(self, playerlist):
        "Remove all the offline players in this list... returns result list"
        onlinelist = []
        ignores = ptVault().getIgnoreListFolder()
        for plyr in playerlist:
            if isinstance(plyr,ptVaultNodeRef):
                PLR = plyr.getChild()
                PLR = PLR.upcastToPlayerInfoNode()
                # its an element.. should be a player
                if type(PLR) != type(None) and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                    if PLR.playerIsOnline():
                        if not ignores.playerlistHasPlayer(PLR.playerGetID()):
                            onlinelist.append(plyr)
        return onlinelist

    def IRemoveCCRPlayers(self, playerlist):
        "Remove all the CCR players in this list... returns result list"
        nonCCRlist = []
        for plyr in playerlist:
            if isinstance(plyr,ptPlayer):
                if not plyr.isCCR() and not plyr.isServer():
                    nonCCRlist.append(plyr)
        return nonCCRlist

    def IRefreshPlayerListDisplay(self):
        # now we can display this mess
        global BKPlayerList
        global BKPlayerSelected
        global FolderOfDevices
        global PreviouslySelectedPlayer
        global MarkerGameState
        global MarkerPlayerList
        if PtIsSinglePlayerMode():
            playerup = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiPlayerListUp))
            playerup.hide()
            playerdown = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiPlayerListDown))
            playerdown.hide()
            return
        PtDebugPrint("xKI: refresh playerlist display",level=kDebugDumpLevel)
        playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
        scrollPos = playerlist.getScrollPos()
        playerlist.lock()
        playerlist.clearAllElements()
        newselection = -1    # assume no selection
        
        idx = 0
        for plyr in BKPlayerList:
            if isinstance(plyr,DeviceFolder):
                playerlist.closeBranch()
                playerlist.addBranch(string.upper(plyr.name),1)
            elif isinstance(plyr,Device):
                playerlist.addStringWithColor(plyr.name,DniSelectableColor,kSelectUseGUIColor)
            elif isinstance(plyr,ptVaultNodeRef):
                PLR = plyr.getChild()
                PLR = PLR.upcastToPlayerInfoNode()
                # its an element.. should be a player
                if type(PLR) != type(None) and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                    if PLR.playerIsOnline():
                        playerlist.addStringWithColor(PLR.playerGetName(),DniSelectableColor,kSelectUseGUIColor)
                    else:
                        playerlist.addStringWithColor(PLR.playerGetName(),AgenBlueDk,kSelectDetermined)
                else:
                    PtDebugPrint("xBigKI: unknown player element type %d" % (PLR.getType()),level=kErrorLevel)
            elif isinstance(plyr,ptPlayer):
                preText = " "
                postText = " "
                if plyr.getPlayerID() != 0:
                    if plyr.getDistanceSq() < PtMaxListenDistSq():
                        preText = ">"
                        postText = "<"
                if plyr.getPlayerName() != "":
                    playerlist.addStringWithColor(preText+plyr.getPlayerName()+postText,DniSelectableColor,kSelectUseGUIColor)
                else:
                    if plyr.getPlayerID() != 0:
                        playerlist.addStringWithColor(preText+"[ID:%08d]"%(plyr.getPlayerID())+postText,DniSelectableColor,kSelectDetermined)
                    else:
                        playerlist.addStringWithColor(preText+"?unknown user?"+postText,DniSelectableColor,kSelectDetermined)
            elif isinstance(plyr,kiFolder):
                playerlist.closeBranch()
                playerlist.addBranchW(string.upper(plyr.name),1)
            elif isinstance(plyr,ptVaultPlayerInfoListNode):
                # its a player list, display its name
                fldrType = plyr.folderGetType()
                # if its a list of age owners... must be list of neighbors
                if fldrType == PtVaultStandardNodes.kAgeOwnersFolder:
                    fldrType = PtVaultStandardNodes.kHoodMembersFolder
                playerlist.closeBranch()
                playerlist.addBranch(string.upper(xLocTools.FolderIDToFolderName(fldrType)),1)
            elif isinstance(plyr,ptVaultMarkerGameNode):
                # its a marker list, display its name
                playerlist.closeBranch()
                playerlist.addBranch(plyr.folderGetName(),1)
            elif isinstance(plyr,MarkerPlayer):
                if type(plyr.player) != type(None):
                    if plyr.player.getPlayerName() != "":
                        pcolor = AgenBlueDk
                        if plyr.isJoined:
                            if plyr.team == PtMarkerMsgTeam.kGreenTeam:
                                pcolor = DniGreenDk
                            elif plyr.team == PtMarkerMsgTeam.kRedTeam:
                                pcolor = DniRed
                        playerlist.addStringWithColor(plyr.player.getPlayerName()+plyr.scoreText,pcolor,kSelectUseGUIColor)
                else:
                    playerlist.addStringWithColor("?offline userID[%d]?"%(plyr.player.getPlayerID()),AgenBlueDk,kSelectDetermined)
            elif isinstance(plyr,MarkerGame):
                # its a marker list, display its name
                playerlist.closeBranch()
                playerlist.addBranch(plyr.gameName,1)
            elif isinstance(plyr,DPLBranchStatusLine):
                if plyr.closePrev:
                    playerlist.closeBranch()
                plyr.position = idx
                playerlist.addBranch(plyr.text,1)
            elif isinstance(plyr,DPLStatusLine):
                plyr.position = idx
                if type(plyr.color) != type(None):
                    clr = plyr.color
                else:
                    clr = DniSelectableColor
                playerlist.addStringWithColor(plyr.text,clr,kSelectUseGUIColor)
            elif type(plyr) == type(""):
                playerlist.closeBranch()
                playerlist.addBranch(plyr,1)
            else:
                PtDebugPrint("xBigKI: unknown list type ",plyr,level=kErrorLevel)
                pass
            #~ except TypeError:
                #~ PtDebugPrint("xBigKI: Unknown player list type ",plyr)
            # try to find out if it is the selected one
            # ...was there something selected?
            if type(PreviouslySelectedPlayer) != type(None):
                PtDebugPrint("xKI: a previously selected player",PreviouslySelectedPlayer, level=kDebugDumpLevel)

                #Fix for vaultNodeRef comparisons (they no longer work)!
                if type(PreviouslySelectedPlayer) == long and type(plyr) == ptVaultNodeRef:
                    plyr = plyr.getChild().getID()  #Set to the ID and now we can test!                    

                # was it the same class?
                if PreviouslySelectedPlayer.__class__ == plyr.__class__:
                    PtDebugPrint("xKI: Match class - previous player matches class", level=kDebugDumpLevel)
                    # and finlly, was it the same object
                    if PreviouslySelectedPlayer == plyr:
                        PtDebugPrint("xKI: Match object - previous player matches object, setting to %d"%(idx), level=kDebugDumpLevel)
                        newselection = idx
                        # found it... stop looking
                        PreviouslySelectedPlayer = None
                    else:
                        PtDebugPrint("xKI: Not Match object - previous does not match object", level=kDebugDumpLevel)
                else:
                    PtDebugPrint("xKI: Not Match class - previous does not match class", level=kDebugDumpLevel)
            idx += 1
        # if there is no selection
        if newselection == -1:
            # then select the first item in the list
            newselection = 0
            # put the caret back to regular prompt
            caret = ptGUIControlTextBox(KIMini.dialog.getControlFromTag(kChatCaretID))
            caret.setString(">")
        PtDebugPrint("xKI:mini: setting new selection to %d"%(newselection),level=kDebugDumpLevel)
        playerlist.setSelection(newselection)
        PreviouslySelectedPlayer = None

        # re-establish the selection they had before
        playerlist.setScrollPos(scrollPos)
        playerlist.unlock()

        self.ICheckScrollButtons(playerlist,kminiPlayerListUp,kminiPlayerListDown)

        # set the SendTo button
        sendToButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKIToPlayerButton))
        if type(BKPlayerSelected) == type(None):
            sendToButton.hide()
        else:
            # else make sure that the person is still here
            if isinstance(BKPlayerSelected,DeviceFolder):
                # this shouldn't happen
                BKPlayerSelected = None
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                sendToField.setString("  ")
                sendToButton.hide()
            elif isinstance(BKPlayerSelected,Device):
                # see if the device is still in range
                try:
                    FolderOfDevices.index(BKPlayerSelected)
                except ValueError:
                    # no longer in the list of devices... then remove
                    BKPlayerSelected = None
                    sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                    sendToField.setString("  ")
                    sendToButton.hide()

    def IRefreshMiniKIMarkerDisplay(self):
        global PhasedKICreateMarkerGame
        "refresh the display on the miniKI indicator bars"
        print "xKI:GZ: Refreshing MarkerDisplay  %d:%d" % (gMarkerGottenNumber,gMarkerToGetNumber)
        if theKILevel > kMicroKI and not PtIsSinglePlayerMode():
            if ((gMarkerGottenNumber == gMarkerToGetNumber) and ((gMarkerToGetNumber % 25) == 0)):
                xMyMaxMarkers = gMarkerToGetNumber
                xMyGotMarkers = gMarkerGottenNumber
            else:
                xMyGotMarkers = (gMarkerGottenNumber % 25)
                if (gMarkerGottenNumber >= (math.floor((gMarkerToGetNumber / 25)) * 25)):
                    xMyMaxMarkers = (gMarkerToGetNumber % 25)
                else:
                    xMyMaxMarkers = 25
            for mcbid in range(kminiMarkerIndicator01,kminiMarkerIndicatorLast+1):
                mcb = ptGUIControlProgress(KIMini.dialog.getControlFromTag(mcbid))
                markerNumber = mcbid - kminiMarkerIndicator01 + 1
                try:
                    if ((not gKIMarkerLevel) or (markerNumber > xMyMaxMarkers)):
                        mcb.setValue(gMarkerColors['off'])
                    elif ((markerNumber <= xMyMaxMarkers) and (markerNumber > xMyGotMarkers)):
                        mcb.setValue(gMarkerColors[gMarkerToGetColor])
                    else:
                        mcb.setValue(gMarkerColors[gMarkerGottenColor])
                except LookupError:
                    print "xKI:GZ - couldn't find color - defaulting to off"
                    mcb.setValue(gMarkerColors['off'])
            btnmtDrip = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZDrip))
            btnmtActive = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZActive))
            btnmtPlaying = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZMarkerGameActive))
            btnmtInRange = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiGZMarkerInRange))

            btnmgNewMarker = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiMGNewMarker))
            btnmgNewGame = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiMGNewGame))
            btnmgInactive = ptGUIControlButton(KIMini.dialog.getControlFromTag(kminiMGInactive))
            

            if gKIMarkerLevel:
#### for now Beam drip is off
#               btnmtDrip.show()
#### for now Beam drip is off
                btnmtDrip.hide()
                if gMarkerToGetNumber > gMarkerGottenNumber:
                    if gGZMarkerInRange:
                        btnmtInRange.show()
                        btnmtPlaying.hide()
                        btnmtActive.hide()
                    else:
                        btnmtInRange.hide()
                        btnmtPlaying.show()
                        btnmtActive.hide()
                else:
                    btnmtPlaying.hide()
                    btnmtInRange.hide()
                    btnmtActive.show()
            else:
                btnmtDrip.hide()
                btnmtActive.hide()
                btnmtPlaying.hide()
                btnmtInRange.hide()
            
            #Added code for checking if we should be displaying Marker Game Gui options.
            if self.markerGameManager != None and self.markerGameManager.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCGZ:
                playingCGZ = 1 
            else:
                playingCGZ = 0

            if gKIMarkerLevel >= kKIMarkerNormalLevel and PhasedKICreateMarkerGame and not playingCGZ:
                btnmtDrip.hide()
                btnmtActive.hide()
                btnmtPlaying.hide()
                btnmtInRange.hide()
                
                try:
                    showMarkers = self.markerGameDisplay.showMarkers
                except:
                    showMarkers = 0

                try:
                    selectedMarker = self.markerGameDisplay.selectedMarker
                except :
                    selectedMarker = -1
                
                try:
                    gameLoaded = self.markerGameManager.gameLoaded()
                except:
                    gameLoaded = 0
                
                if gameLoaded:
                    btnmgNewMarker.hide()
                    btnmgNewGame.hide()
                    btnmgInactive.show()

                elif showMarkers and selectedMarker < 0:
                    btnmgNewMarker.show()
                    btnmgNewGame.hide()
                    btnmgInactive.hide()

                else:
                    btnmgNewMarker.hide()
                    btnmgNewGame.show()
                    btnmgInactive.hide()
              
            else:      
                btnmgNewMarker.hide()
                btnmgNewGame.hide()
                btnmgInactive.hide()

    def IRefreshKISettings(self):
        "refresh the KI config settings for the dialog"
        global TicksOnFull
        global OnlyGetPMsFromBuddies
        global OnlyAllowBuddiesOnRequest
        fontSizeSlider = ptGUIControlKnob(KISettings.dialog.getControlFromTag(kBKIKIFontSize))
        fontSize = self.IGetFontSize()
        # find place in font table
        whichFont = 0
        for fs in FontSizeList:
            if fontSize <= fs:
                break
            whichFont += 1
        if whichFont >= len(FontSizeList):
            whichFont = len(FontSizeList)-1
        slidePerFont = float(fontSizeSlider.getMax()-fontSizeSlider.getMin()+1.0)/float(len(FontSizeList))
        FSslider = int(slidePerFont * whichFont + 0.25)
        fontSizeSlider.setValue(FSslider)
        
        fadeTimeSlider = ptGUIControlKnob(KISettings.dialog.getControlFromTag(kBKIKIFadeTime))
        slidePerTime = float(fadeTimeSlider.getMax()-fadeTimeSlider.getMin())/float(kFadeTimeMax)
        if not FadeEnableFlag:
            TicksOnFull = kFadeTimeMax
        FTslider = slidePerTime * TicksOnFull
        fadeTimeSlider.setValue(FTslider)

        onlyPMCheckbox = ptGUIControlCheckBox(KISettings.dialog.getControlFromTag(kBKIKIOnlyPM))
        onlyPMCheckbox.setChecked(OnlyGetPMsFromBuddies)
###==> when buddy control is enabled
#        buddyCheckCheckbox = ptGUIControlCheckBox(KISettings.dialog.getControlFromTag(kBKIKIBuddyCheck))
#        buddyCheckCheckbox.setChecked(OnlyAllowBuddiesOnRequest)
###==> when buddy control is enabled

    def IRefreshVolumeSettings(self):
        "refresh the volume settings to the current settings"
        audio = ptAudioControl()
        soundFX = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kBKISoundFXVolSlider))
        setting = audio.getSoundFXVolume()
        #~ print "SoundFX volume is %g (into %g)" % (setting,setting*10)
        soundFX.setValue(setting*10)

        music = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(xBKIMusicVolSlider))
        setting = audio.getMusicVolume()
        #~ print "Music volume is %g (into %g)" % (setting,setting*10)
        music.setValue(setting*10)
        
        voice = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(xBKIVoiceVolSlider))
        setting = audio.getVoiceVolume()
        #~ print "Voice volume is %g (into %g)" % (setting,setting*10)
        voice.setValue(setting*10)
        
        ambience = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kBKIAmbienceVolSlider))
        setting = audio.getAmbienceVolume()
        #~ print "Ambience volume is %g (into %g)" % (setting,setting*10)
        ambience.setValue(setting*10)
        
        miclevel = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kBKIMicLevelSlider))
        setting = audio.getMicLevel()
        #~ print "MicLevel is %g (into %g)" % (setting,setting*10)
        miclevel.setValue(setting*10)

        guivolume = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kBKIGUIVolSlider))
        setting = audio.getGUIVolume()
        #~ print "Ambience volume is %g (into %g)" % (setting,setting*10)
        guivolume.setValue(setting*10)

    def IRefreshAgeOwnerSettings(self):
        "refresh the Age Owner neighborhood settings"
        global BKRightSideMode
        global BKConfigFolderDict
        global BKConfigListOrder
        # make sure that we are actually going to display (or is it just an update)
        if BigKI.dialog.isEnabled() and BKRightSideMode == kBKAgeOwnerExpanded:
            try:
                # get the selected age config setting
                myAge = BKConfigFolderDict[BKConfigListOrder[BKFolderSelected]]
            except LookupError:
                myAge = None
            if type(myAge) != type(None):
                czar = myAge.getCzar()
                title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleTB))
                title.setString(self.IGetAgeDisplayName(myAge))
                titlebtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleBtn))
                titlebtn.enable()                
                titleedit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleEditbox))
                titleedit.hide()
                status = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerStatusTB))
                #guid = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerGUIDTB))
                #guid.setString("[ID:%s]"%(myAge.getAgeInstanceGuid()))
                visitors = myAge.getCanVisitFolder()
                owners = myAge.getAgeOwnersFolder()
                numvisitors = visitors.getChildNodeCount()
                numowners = owners.getChildNodeCount()
                vsess = "s"
                if numvisitors == 1:
                    vsess = ""
                osess = "s"
                if numowners == 1:
                    osess = ""
                #
                # for now, you can't make the age public or private... can only be done in the Nexus
                makepublicTB = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerMakePublicTB))
                makepublicBtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerMakePublicBtn))
                makepublicBtn.disable()
                makepublicTB.hide()
                makepublicTB.setString(" ")
                status.setStringW(PtGetLocalizedString("KI.Neighborhood.AgeOwnedStatus", [str(numowners),str(osess),str(numvisitors),str(vsess)]))
                descript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerDescription))
                encoded = buffer(myAge.getAgeDescription())
                descript.setEncodedBuffer(encoded)

    def IminiToggleKISize(self):
        "Toggle between mini and big KI"
        global theKILevel
        global IKIDisabled
        global WaitingForAnimation
        global OriginalminiKICenter
        global LastminiKICenter
        if theKILevel > kMicroKI and (not IKIDisabled or BigKI.dialog.isEnabled()):
            if IKIDisabled and BigKI.dialog.isEnabled():
                self.IminiPutAwayKI()
                return
            if not WaitingForAnimation:
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
                if BigKI.dialog.isEnabled():
                    # hide the big KI part
                    self.IBigKIHideBigKI()
                    # can't be in chatting mode
                    self.IEnterChatMode(0)
                    KIBlackbar.dialog.show()
                    if type(LastminiKICenter) != type(None):
                        dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kminiDragBar))
                        dragbar.setObjectCenter(LastminiKICenter)
                        dragbar.unanchor()
                        LastminiKICenter = None
                    # refresh the player list, because it will be the shorten version
                    self.IRefreshPlayerList(forceSmall=1)
                    self.IRefreshPlayerListDisplay()
                    toggleCB.setChecked(0)
                else:
                    # if there is nothing showing, then just bring up the miniKI
                    if not KIMini.dialog.isEnabled():
                        self.IClearBBMini(0)    # set the checkbox, which will show the dialog... chain reaction
                    else:
                        WaitingForAnimation = 1
                        # hide the miniKI and blackbar
                        KIBlackbar.dialog.hide()
                        # hide the miniKI so that it is shown on top of the bigKI
                        KIMini.dialog.hide()
                        # can't be in chatting mode
                        self.IEnterChatMode(0)
                        # show the big stuff
                        BigKI.dialog.show()
                        # save current location and snap back to original
                        if type(OriginalminiKICenter) != type(None):
                            dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kminiDragBar))
                            LastminiKICenter = dragbar.getObjectCenter()
                            PtDebugPrint("xKI:distance to original = %f"%(LastminiKICenter.distance(OriginalminiKICenter)),level=kDebugDumpLevel)
                            # if they are close, then snap it to original
                            if LastminiKICenter.distance(OriginalminiKICenter) < 0.027:
                                LastminiKICenter = OriginalminiKICenter
                            dragbar.setObjectCenter(OriginalminiKICenter)
                            dragbar.anchor()
                        KIMini.dialog.show()
                        toggleCB.setChecked(1)

    def IminiPutAwayKI(self, forceOpen = 0):
        global theKILevel
        global IKIDisabled
        global OriginalminiKICenter
        global LastminiKICenter
        global ISawTheKIAtleastOnce
        if theKILevel > kMicroKI and (not IKIDisabled or KIMini.dialog.isEnabled()):
            if KIMini.dialog.isEnabled():
                # put away the KI (both mini and big)
                KIMini.dialog.hide()
                # and put back where it used to be
                if type(LastminiKICenter) != type(None):
                    dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kminiDragBar))
                    dragbar.setObjectCenter(LastminiKICenter)
                    dragbar.unanchor()
                    LastminiKICenter = None
                if BigKI.dialog.isEnabled():
                    # hide the big KI part
                    self.IBigKIHideBigKI()
                KIBlackbar.dialog.show()
                self.IClearBBMini(-1)
                # put the toggle button back to miniKI
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
                toggleCB.setChecked(0)
                ISawTheKIAtleastOnce = 1
            else:
                # else if already disabled, then bring up the miniKI
                if forceOpen:
                    self.IClearBBMini(0)    # set the checkbox, which will show the dialog... chain reaction

    def IminiTakePicture(self):
        global theKILevel
        global IKIDisabled
        global WeAreTakingAPicture
        global YNWhatReason
        global PhasedKICreateImages
        if PhasedKICreateImages:
            if not WeAreTakingAPicture and not WaitingForAnimation:
                # NOTE: we are ignoring the IKIDisabled flag here because we want to be able to take sshots even
                # if the KI is disabled by certain GUIs. Hopefully this won't cause any weird issues, but since
                # it's only a screenshot, not much can go wrong (famous last words...)
                if theKILevel > kMicroKI:
                    if self.ICanTakePicture():
                        WeAreTakingAPicture = 1
                        # if a modal dialog is showing, just snap the shot because the KI is
                        # already out of the way
                        if not PtIsGUIModal():
                            # hide everything to take a picture
                            KIBlackbar.dialog.hide()
                            KIMini.dialog.hide()
                            self.IBigKIHideMode()
                            BigKI.dialog.hide()
                            # put the toggle button back to bigKI
                            toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
                            toggleCB.setChecked(1)
                        # wait a second and take the picture!
                        PtAtTimeCallback(self.key,0.25,kTakeSnapShot)
                    else:
                        # put up some kinda error message
                        self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullImages"))


    def IminiCreateJournal(self):
        global theKILevel
        global IKIDisabled
        global OriginalminiKICenter
        global LastminiKICenter
        global BKRightSideMode
        global WeAreTakingAPicture
        global YNWhatReason
        global PhasedKICreateNotes
        if PhasedKICreateNotes:
            if not WeAreTakingAPicture and not WaitingForAnimation:
                if theKILevel > kMicroKI and not IKIDisabled:
                    if self.ICanMakeNote():
                        # hide the blackbar (even if its already hid)
                        KIBlackbar.dialog.hide()
                        # put the toggle button back to bigKI
                        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
                        toggleCB.setChecked(1)
                        # create a journal entry
                        self.IBigKICreateJournalNote()
                        # make sure that we are in journal mode
                        modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kBKRadioModeID))
                        modeselector.setValue(0)
                        # set things up so that when the bigKI shows it goes into edit mode
                        if BKRightSideMode != kBKJournalExpanded:
                            # make sure if a different is showing, hide it
                            self.IBigKIHideMode()
                        # now it "show the journal expanded" mode
                        BKRightSideMode = kBKJournalExpanded
                        # reset the topline and selection
                        self.IBigKIRefreshFolderDisplay()
                        self.IBigKIDisplayCurrentContentJournal()
                        # setup to edit the title of the journal (err... the caption)
                        self.IBigKIEnterEditMode(kBKEditFieldJRNTitle)
                        if BigKI.dialog.isEnabled():
                            self.IBigKIShowMode()
                        else:
                            # need to make the miniKI come up over the bigKI
                            KIMini.dialog.hide()
                            # show the big stuff
                            BigKI.dialog.show()
                            # need to make the miniKI come up over the bigKI
                            KIMini.dialog.show()
                        # was just the miniMI showing (not the big KI?)
                        if type(LastminiKICenter) == type(None):
                            #~ print "only mini KI should have been showing"
                            if type(OriginalminiKICenter) != type(None):
                                dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kminiDragBar))
                                LastminiKICenter = dragbar.getObjectCenter()
                                dragbar.setObjectCenter(OriginalminiKICenter)
                                dragbar.anchor()
                        #~ KIMini.dialog.show()
                    else:
                        # put up some kinda error message
                        self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullNotes"))

    def ICaptureGZMarker(self):
        global gGZMarkerInRange
        global gGZMarkerInRangeRepy
        global gMarkerToGetNumber
        global gMarkerGottenNumber
        # make sure there is room for the capture marker
        if gGZPlaying and gMarkerToGetNumber > gMarkerGottenNumber:
            # set the marker status to 'gotten'
            #   ...in the GZ marker chronicle
            vault = ptVault()
            # is there a chronicle for the GZ games?
            entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
            if type(entry) != type(None):
                markers = entry.chronicleGetValue()
                markerIdx = gGZMarkerInRange - 1
                if markerIdx >= 0 and markerIdx < len(markers):
                    # Set the marker to "captured"
                    if len(markers)-(markerIdx+1) != 0:
                        markers = markers[:markerIdx] + kGZMarkerCaptured + markers[-(len(markers)-(markerIdx+1)):]
                    else:
                        markers = markers[:markerIdx] + kGZMarkerCaptured
                    entry.chronicleSetValue(markers)
                    entry.save()
                    # update the marker Gotten count
                    totalGotten = markers.count(kGZMarkerCaptured)
                    if gKIMarkerLevel > kKIMarkerFirstLevel:
                        # if this is the second wave of markers (or beyond)
                        totalGotten -= 15
                        if totalGotten < 0:
                            totalGotten = 0
                    if totalGotten > gMarkerToGetNumber:
                        totalGotten = gMarkerToGetNumber
                    gMarkerGottenNumber = totalGotten
                    # save update to chronicle
                    self.IUpdateGZGamesChonicles()
                else:
                    PtDebugPrint("xminiKI:CaptureGZMarker: invalid marker serial number of %d" % (gGZMarkerInRange) )
                    return
            else:
                PtDebugPrint("xminiKI:CaptureGZMarker: no chronicle entry found" )
                return
            # start building the notify message to go back to the orignator
            if type(gGZMarkerInRangeRepy) != type(None):
                note = ptNotify(self.key)
                note.clearReceivers()
                note.addReceiver(gGZMarkerInRangeRepy)
                note.netPropagate(0)
                note.netForce(0)
                note.setActivate(1)
                note.addVarNumber("Captured",1)
                note.send()
            gGZMarkerInRangeRepy = None
            gGZMarkerInRange = 0

##############################################################
##
##  bigKI (sorta) private methods
##
##############################################################
    def IBigKIShowBigKI(self):
        global WaitingForAnimation
        global IsPlayingLookingAtKIMode
        WaitingForAnimation = 1
        curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
        toggleCB.disable()
        if curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit:
            PtDebugPrint("xKI:ShowKI enter LookingAtKI mode",level=kDebugDumpLevel)
            PtAvatarEnterLookingAtKI()
            IsPlayingLookingAtKIMode = 1
        PtDisableMovementKeys()
        KIOnResp.run(self.key,netPropagate=0)
        if (gKIMarkerLevel == kKIMarkerNormalLevel):
            sdl = xPsnlVaultSDL()
            if (not sdl['GPSEnabled'][0]):
                PtDebugPrint('xKI:ShowKI check calibration', level=kDebugDumpLevel)
                try:
                    self.ICheckCalibrationProgress()
                except:
                    PtDebugPrint("ERROR: xKI.IBigKIShowBigKI():\tCouldn't execute self.ICheckCalibrationProgress()")


    def IBigKIHideBigKI(self):
        global WaitingForAnimation
        global IsPlayingLookingAtKIMode
        WaitingForAnimation = 1
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kminiToggleBtnID))
        toggleCB.disable()
        self.IBigKIHideMode()
        # make sure we were in 
        if IsPlayingLookingAtKIMode:
            PtDebugPrint("xKI:HideKI exit LookingAtKI mode",level=kDebugDumpLevel)
            PtAvatarExitLookingAtKI()
        IsPlayingLookingAtKIMode = 0
        PtEnableMovementKeys()
        KIOffResp.run(self.key,netPropagate=0)

    def IBigKIShowMode(self):
        global BKRightSideMode
        global BKCurrentContent
        global BKGettingPlayerID
        global PhasedKISendMarkerGame

        if BigKI.dialog.isEnabled():
            self.IResetListModeArrows()
            if BKRightSideMode == kBKListMode:
                KIListModeDialog.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                BKCurrentContent = None
                BKGettingPlayerID = 0
            elif BKRightSideMode == kBKJournalExpanded:
                KIJournalExpanded.dialog.show()
                if self.IsContentMutable(BKCurrentContent):
                    self.IBigKIInvertToFolderButtons()
                else:
                    self.IBigKIOnlySelectedToButtons()
                BKGettingPlayerID = 0
            elif BKRightSideMode == kBKPictureExpanded:
                KIPictureExpanded.dialog.show()
                if self.IsContentMutable(BKCurrentContent):
                    self.IBigKIInvertToFolderButtons()
                else:
                    self.IBigKIOnlySelectedToButtons()
                BKGettingPlayerID = 0
            elif BKRightSideMode == kBKPlayerExpanded:
                KIPlayerExpanded.dialog.show()
                # if the expanded player is ourselves, then no move buttons!
                localplayer = PtGetLocalPlayer()
                if type(BKCurrentContent) != type(None):
                    if isinstance(BKCurrentContent, ptPlayer):
                        if BKCurrentContent.getPlayerID() == localplayer.getPlayerID():
                            self.IBigKIOnlySelectedToButtons()
                            return
                    # else assume that its a plVaultNodeRef
                    else:
                        elem = BKCurrentContent.getChild()
                        if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                            elem = elem.upcastToPlayerInfoNode()
                            if elem.playerGetID() == localplayer.getPlayerID():
                                self.IBigKIOnlySelectedToButtons()
                                return
                self.IBigKIInvertToFolderButtons()
            elif BKRightSideMode == kBKVolumeExpanded:
                KIVolumeExpanded.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                BKCurrentContent = None
                BKGettingPlayerID = 0
            elif BKRightSideMode == kBKKIExpanded:
                KISettings.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                BKCurrentContent = None
                BKGettingPlayerID = 0
            elif BKRightSideMode == kBKAgeOwnerExpanded:
                KIAgeOwnerExpanded.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                BKCurrentContent = None
                BKGettingPlayerID = 0
            elif BKRightSideMode == kBKMarkerListExpanded:
                KIMarkerFolderExpanded.dialog.show()
                if PhasedKISendMarkerGame and self.IsContentMutable(BKCurrentContent):
                    self.IBigKIInvertToFolderButtons()
                else:
                    self.IBigKIOnlySelectedToButtons()
                BKGettingPlayerID = 0
            elif BKRightSideMode == kBKQuestionNote:
                KIQuestionNote.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                BKGettingPlayerID = 0

    def IBigKIHideMode(self):
        global BKRightSideMode
        if BKRightSideMode == kBKListMode:
            KIListModeDialog.dialog.hide()
        elif BKRightSideMode == kBKJournalExpanded:
            KIJournalExpanded.dialog.hide()
        elif BKRightSideMode == kBKPictureExpanded:
            KIPictureExpanded.dialog.hide()
        elif BKRightSideMode == kBKPlayerExpanded:
            KIPlayerExpanded.dialog.hide()
        elif BKRightSideMode == kBKVolumeExpanded:
            KIVolumeExpanded.dialog.hide()
        elif BKRightSideMode == kBKKIExpanded:
            KISettings.dialog.hide()
        elif BKRightSideMode == kBKAgeOwnerExpanded:
            KIAgeOwnerExpanded.dialog.hide()
        elif BKRightSideMode == kBKMarkerListExpanded:
            KIMarkerFolderExpanded.dialog.hide()
        elif BKRightSideMode == kBKQuestionNote:
            KIQuestionNote.dialog.hide()

    def IBigKIChangeMode(self,newmode):
        global BKRightSideMode
        if newmode != BKRightSideMode:
            self.IBigKIHideMode()
            BKRightSideMode = newmode
            self.IBigKIShowMode()
        elif newmode == kBKListMode:
            # might just be a selection change
            self.IBigKIOnlySelectedToButtons()

    def IBigKISetToButtons(self):
        global BKRightSideMode
        global PhasedKISendNotes
        if BKRightSideMode == kBKListMode:
            self.IBigKIOnlySelectedToButtons()
        elif BKRightSideMode == kBKJournalExpanded:
            if self.IsContentMutable(BKCurrentContent):
                self.IBigKIInvertToFolderButtons()
            else:
                self.IBigKIOnlySelectedToButtons()
        elif BKRightSideMode == kBKPictureExpanded:
            if self.IsContentMutable(BKCurrentContent):
                self.IBigKIInvertToFolderButtons()
            else:
                self.IBigKIOnlySelectedToButtons()
        elif BKRightSideMode == kBKPlayerExpanded:
            localplayer = PtGetLocalPlayer()
            if type(BKCurrentContent) != type(None):
                if isinstance(BKCurrentContent, ptPlayer):
                    if BKCurrentContent.getPlayerID() == localplayer.getPlayerID():
                        self.IBigKIOnlySelectedToButtons()
                        return
                # else assume that its a plVaultNodeRef
                else:
                    elem = BKCurrentContent.getChild()
                    if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                        elem = elem.upcastToPlayerInfoNode()
                        if elem.playerGetID() == localplayer.getPlayerID():
                            self.IBigKIOnlySelectedToButtons()
                            return
            self.IBigKIInvertToFolderButtons()
        elif BKRightSideMode == kBKVolumeExpanded:
            self.IBigKIOnlySelectedToButtons()
        elif BKRightSideMode == kBKKIExpanded:
            self.IBigKIOnlySelectedToButtons()
        elif BKRightSideMode == kBKAgeOwnerExpanded:
            self.IBigKIOnlySelectedToButtons()
        elif BKRightSideMode == kBKMarkerListExpanded:
            if PhasedKISendMarkerGame and MFdialogMode != kMFPlaying and self.IsContentMutable(BKCurrentContent):
                self.IBigKIInvertToFolderButtons()
            else:
                self.IBigKIOnlySelectedToButtons()
        elif BKRightSideMode == kBKQuestionNote:
            self.IBigKIOnlySelectedToButtons()

    def IBigKISetNotifyForLong(self):
        for id in range(kBKIIncomingBtn,kBKIFolderLineBtnLast):
            overBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(id))
            overBtn.setNotifyOnInteresting(1)

    def IBigKIHideLongFolderNames(self):
        for id in range(kLONGBKIIncomingLine,kLONGBKIFolderLineLast+1):
            longTB = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(id))
            longTB.hide()

    def IResetListModeArrows(self):
        # hide up/down scroll buttons (this should be in expanded mode)
        upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKLMUpButton))
        upbtn.hide()
        dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKLMDownButton))
        dwnbtn.hide()

    def IBigKIOnlySelectedToButtons(self):
        toplayerbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKIToPlayerButton))
        toplayerbtn.hide()
        self.IBigKIRefreshFolderDisplay()
        # hide all the buttons
        for id in range(kBKIToIncomingButton,kBKIToFolderButtonLast+1):
            tofolder = ptGUIControlButton(BigKI.dialog.getControlFromTag(id))
            tofolder.hide()
        self.IBigKINewContentList()

    def IBigKICanShowSendToPlayer(self):
        # make sure there is a Selected player
        if type(BKPlayerSelected) == type(None):
            return false
        # make sure that its something that can be sent to a player
        if BKRightSideMode == kBKPlayerExpanded or BKRightSideMode == kBKVolumeExpanded or BKRightSideMode == kBKAgeOwnerExpanded:
            return false
        if BKRightSideMode == kBKJournalExpanded:
            if not PhasedKISendNotes:
                return false
        if BKRightSideMode == kBKPictureExpanded:
            if not PhasedKISendImages:
                return false
        # make sure that its not ourselves
        if isinstance(BKPlayerSelected,ptVaultNodeRef):
            plyrElement = BKPlayerSelected.getChild()
            if type(plyrElement) != type(None) and plyrElement.getType()==PtVaultNodeTypes.kPlayerInfoNode:
                plyrElement = plyrElement.upcastToPlayerInfoNode()
                if plyrElement.playerGetID() == PtGetLocalClientID():
                    return false
        # then finally it must ok
        return true

    def IBigKIInvertToFolderButtons(self):
        global BKRightSideMode
        # setup 'toplayer' button
        toplayerbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKIToPlayerButton))
        if self.IBigKICanShowSendToPlayer():
            toplayerbtn.show()
        else:
            toplayerbtn.hide()
        selectedButton = BKFolderSelected - BKFolderTopLine + kBKIToIncomingButton
        for id in range(kBKIToIncomingButton,kBKIToFolderButtonLast+1):
            tofolder = ptGUIControlButton(BigKI.dialog.getControlFromTag(id))
            if id == selectedButton:
                tofolder.hide()
            else:
                # but don't show on elements that are not there or immutable
                if id - kBKIToIncomingButton <= len(BKFolderListOrder)-1-BKFolderTopLine:
                    try:
                        if self.IsFolderContentsMutable(BKFolderLineDict[BKFolderListOrder[id-kBKIToIncomingButton+BKFolderTopLine]]):
                            # assume that we are going to show the bugger
                            tofolder.show()
####=====> needs attention
### need to be able to determine if the player is already an owner of the age
#### if so, then no need to allow them to visit then, they can come anytime
                            #~ # one more check... if they are on a neighborhood member, then they can't move to the invite folders
                            #~ daFolder = BKFolderLineDict[BKFolderListOrder[id-kBKIToIncomingButton+BKFolderTopLine]]
                            #~ daFolderType = daFolder.folderGetType()
                            #~ if daFolderType==PtVaultStandardNodes.kCanVisitFolder:
                                #~ if type(BKCurrentContent) != type(None):
                                    #~ elem = BKCurrentContent.getChild()
                                    #~ if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                        #~ elem = elem.upcastToPlayerInfoNode()
                                        #~ neighbors = self.IGetNeighbors()
                                        #~ if type(neighbors) != type(None) and neighbors.playerlistHasPlayer(elem.playerGetID()):
                                            #~ tofolder.hide()
#=====> needs attention
                        else:
                            tofolder.hide()
                    except LookupError:
                        tofolder.hide()
                else:
                    tofolder.hide()

    def IsFolderContentsMutable(self,folder):
        "determines whether the folder in question can be changed"
        # make sure there is a real folder there to play with
        if type(folder) == type(None) or not isinstance(folder,ptVaultNode):
            return 0
        # if its not really a folder but an AgeInfoNode, then its for the canVisit player list
        if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
            return 1
        if folder.getType() != PtVaultNodeTypes.kPlayerInfoListNode and folder.getType() != PtVaultNodeTypes.kFolderNode:
            return 0
        # check for the incoming folder
        if folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
            return 0
        # check against agemembers folder
        if folder.folderGetType() == PtVaultStandardNodes.kAgeMembersFolder:
            return 0
        # check for the neighborhood members folder
        if folder.folderGetType() == PtVaultStandardNodes.kHoodMembersFolder:
            return 0
        # check for neighbor hood can visit (is actually half mutable- they can delete)
        if folder.folderGetType() == PtVaultStandardNodes.kAgeOwnersFolder:
            return 0
        # none of the special folders... then mutable
        return 1

    def IsFolderHidden(self,agefolder):
        if agefolder.folderGetName() == "Hidden":
            return 1
        return 0

    def IsContentMutable(self,noderef):
        "determines whether the content noderef is mutable"
        # get its parent folder
        if isinstance(noderef,ptVaultNodeRef):
            folder = BKCurrentContent.getParent()
            if folder:
                folder = folder.upcastToFolderNode()
                if folder:
                    if folder.folderGetType() == PtVaultStandardNodes.kGlobalInboxFolder:
                        return 0
        return 1

    def IBigKISetStatics(self):
        "Initialize the static string fields"
        agetext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKICurAgeNameID))
        ageName = self.IGetAgeDisplayName().replace("(null)", "").strip()
        PtDebugPrint("xKI: displaying age name of %s"%(ageName),level=kDebugDumpLevel)
        agetext.setStringW(ageName)
        playertext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKPlayerName))
        idtext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKPlayerID))
        localplayer = PtGetLocalPlayer()
        playertext.setString(localplayer.getPlayerName())
        if not PtIsSinglePlayerMode():
            idtext.setString("[ID:%08d]"%(localplayer.getPlayerID()))
        else:
            idtext.setString("  ")
        self.IUpdatePelletScore()
        self.IBigKIRefreshHoodStatics()

    def IBigKIRefreshHoodStatics(self,neighborhood=None):
        neighbortext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKNeighborhoodAndID))
        if not PtIsSinglePlayerMode():
            # if a neighborhood was not specified then get the one from the player's vault
            if not neighborhood:
                neighborhood = self.IGetNeighborhood()
            if type(neighborhood) != type(None):
                neighborName = xLocTools.LocalizeAgeName(neighborhood.getDisplayName())
                if neighborName == U"":
                    neighborName = PtGetLocalizedString("KI.Neighborhood.NoName")
                neighbortext.setStringW(PtGetLocalizedString("KI.Neighborhood.BottomLine", [xLocTools.MemberStatusString(),neighborName]))
            else:
                neighbortext.setStringW(PtGetLocalizedString("KI.Neighborhood.None"))
        else:
            neighbortext.setString("  ")

    def IBigKISetChanging(self):
        "Set the changing fields to current settings"
        global PreviousTime
        global TimeBlinker
        # use the D'ni time for this age
        dnitime = PtGetDniTime()
        if dnitime:
            tuptime = time.gmtime(dnitime)
            if TimeBlinker:
                curtime = unicode(time.strftime(str(PtGetLocalizedString("Global.Formats.DateTime")),tuptime))
                TimeBlinker = 0
            else:
                curtime = unicode(time.strftime(str(PtGetLocalizedString("Global.Formats.DateTime")),tuptime))
                TimeBlinker = 1
        else:
            curtime = PtGetLocalizedString("KI.Errors.TimeBroke")
        if curtime != PreviousTime:
            timetext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKICurTimeID))
            timetext.setStringW(curtime)
            PreviousTime = curtime
        # get the gps dni coordinates holders on the screen
        gps1 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIGPS1TextID))
        gps2 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIGPS2TextID))
        gps3 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIGPS3TextID))
        # set the GPS numbers
        # later on.... we will use the real stuff
        self.dnicoords.update()
        if gKIMarkerLevel == kKIMarkerNormalLevel:
            sdl = xPsnlVaultSDL()
            if sdl["GPSEnabled"][0]:
                gps1.setString("%d" % (self.dnicoords.getTorans()))
                gps2.setString("%d" % (self.dnicoords.getHSpans()))
                gps3.setString("%d" % (self.dnicoords.getVSpans()))
            else:
                gps1.setString("%d" % (0))
                gps2.setString("%d" % (0))
                gps3.setString("%d" % (0))
        else:
            gps1.setString("%d" % (0))
            gps2.setString("%d" % (0))
            gps3.setString("%d" % (0))
        PtAtTimeCallback(self.key,5,kBKITODCheck)

    def IBKCheckFolderRefresh(self,folder=None,content=None):
        "check to see if this a folder we're concerned about and needs to be refreshed"
        global BKCurrentContent
        if type(folder) != type(None):
            if folder.getType() == PtVaultNodeTypes.kPlayerInfoListNode:
                self.IRefreshPlayerList()
                self.IRefreshPlayerListDisplay()
        else:
            # if its none, then just check everything
            self.IRefreshPlayerList()
            self.IRefreshPlayerListDisplay()
        # only need to check refresh if we are actaully using the bigKI
        if theKILevel > kMicroKI:
            self.IBigKIRefreshContentList()
            self.IBigKIRefreshContentListDisplay()
        #~ PIKA = ki.getStandardFolder(kKIPlayerPeopleIKnowAboutFolder)
        #~ if folder == PIKA:
            #~ self.IRefreshPlayerList()
            #~ self.IRefreshPlayerListDisplay()
            #~ return

    def IBKCheckContentRefresh(self,content):
        "check to see if the content.element they are looking at changed underneath them"
        global BKRightSideMode
        global BKCurrentContent
        if type(BKCurrentContent) != type(None) and content == BKCurrentContent:
            #~ print "xBigKI - refresh match on content"
            if BKRightSideMode == kBKListMode:
                self.IBigKIRefreshContentListDisplay()
            elif BKRightSideMode == kBKJournalExpanded:
                self.IBigKIDisplayCurrentContentJournal()
            elif BKRightSideMode == kBKPictureExpanded:
                self.IBigKIDisplayCurrentContentImage()
            elif BKRightSideMode == kBKPlayerExpanded:
                self.IBigKIDisplayCurrentContentPlayer()
            elif BKRightSideMode == kBKMarkerListExpanded:
                self.IBigKIDisplayCurrentContentMarkerFolder()
        else:
            #~ PtDebugPrint("xBigKI - no match on content refresh callback")
            pass

    def IBKCheckElementRefresh(self,element):
        "check to see if the content.element they are looking at changed underneath them"
        global BKRightSideMode
        global BKCurrentContent
        if type(BKCurrentContent) != type(None):
            if isinstance(BKCurrentContent,ptVaultNodeRef) and element == BKCurrentContent.getChild():
                #~ print "xBigKI - refresh match on content"
                if BKRightSideMode == kBKListMode:
                    self.IBigKIRefreshContentListDisplay()
                elif BKRightSideMode == kBKJournalExpanded:
                    self.IBigKIDisplayCurrentContentJournal()
                elif BKRightSideMode == kBKPictureExpanded:
                    self.IBigKIDisplayCurrentContentImage()
                elif BKRightSideMode == kBKPlayerExpanded:
                    self.IBigKIDisplayCurrentContentPlayer()
                elif BKRightSideMode == kBKMarkerListExpanded:
                    self.IBigKIDisplayCurrentContentMarkerFolder()

    def IRemoveIgnoredPlayers(self, playerlist):
        "Remove all the offline players in this list... returns result list"
        nonIgnoredlist = []
        ignores = ptVault().getIgnoreListFolder()
        for plyr in playerlist:
            if isinstance(plyr,ptVaultNodeRef):
                PLR = plyr.getChild()
                PLR = PLR.upcastToPlayerInfoNode()
                # its an element.. should be a player
                if type(PLR) != type(None) and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                    if not ignores.playerlistHasPlayer(PLR.playerGetID()):
                        nonIgnored.append(plyr)
        return nonIgnored

    def IBigKIRefreshFolders(self):
        "Refresh the Folders list for the Inbox and the Age Journal folders"
        # need to remember selected and what position in the list we are at
        global BKJournalFolderDict
        global BKJournalListOrder
        global BKPlayerFolderDict
        global BKPlayerListOrder
        global BKConfigFolderDict
        global BKConfigListOrder
        global BKFolderLineDict
        global BKFolderListOrder
        global BKFolderSelected
        global BKFolderTopLine
        global PhasedKINeighborsInDPL
        global PhasedKIBuddies
        vault = ptVault()
        ageVault = ptAgeVault()
        #
        # get the journal folder stuffs
        #
        if not BKJournalFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder)):
            infolder = vault.getInbox()
            if type(infolder) != type(None):
                BKJournalListOrder.insert(0,xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder))
                BKJournalFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder)] = infolder
        # get the age journal folders and add any new ones
        master_agefolder = vault.getAgeJournalsFolder()
        if type(master_agefolder) != type(None):
            agefolderRefs = master_agefolder.getChildNodeRefList()
            for agefolderRef in agefolderRefs:
                agefolder = agefolderRef.getChild()
                agefolder = agefolder.upcastToFolderNode()
                if type(agefolder) != type(None):
                    if not self.IsFolderHidden(agefolder):
                        agefoldername = agefolder.folderGetName()
                        if agefoldername == "":
                            agefoldername = "[invalid]"
                        agefoldername = self.IFilterAgeName(agefoldername)
                        if agefoldername in kHideAgesHackList:
                            continue
                        if not BKJournalFolderDict.has_key(agefoldername):
                            # a new one... add
                            BKJournalListOrder.append(agefoldername)
                        BKJournalFolderDict[agefoldername] = agefolder
            # make sure the current age is at the top of the list
            try:
                line = BKJournalListOrder.index(self.IGetAgeInstanceName())
                if line != 1:
                    #~ print "re-adjust to the top"
                    # its not at the top of the list... put at the top
                    BKJournalListOrder.remove(self.IGetAgeInstanceName())
                    BKJournalListOrder.insert(1,self.IGetAgeInstanceName())
                    # if they are looking at a journal entry then put them in list mode
                    if BKRightSideMode == kBKJournalExpanded or BKRightSideMode == kBKPictureExpanded or BKRightSideMode == kBKMarkerListExpanded:
                        # put 'em in list mode
                        self.IBigKIChangeMode(kBKListMode)
            except ValueError:
                # create one?... yes
                # but not for AvatarCustomization Age
                ageName = self.IGetAgeFileName().lower()
                if ageName != "startup" and ageName != "avatarcustomization" and ageName != "unknown age" and self.IGetAgeInstanceName() != "?unknown?":
                    vault = ptVault()
                    entry = vault.findChronicleEntry("CleftSolved")
                    cleftSolved = 0
                    if type(entry) != type(None):
                        if entry.chronicleGetValue() == "yes":
                            cleftSolved = 1
                    if self.IGetAgeInstanceName() != "D'ni-Riltagamin" or cleftSolved:
                        instAgeName = self.IGetAgeInstanceName()
                        createAgeFolder = 1

                        agefolderRefs = master_agefolder.getChildNodeRefList()
                        for agefolderRef in agefolderRefs:
                            agefolder = agefolderRef.getChild()
                            agefolder = agefolder.upcastToFolderNode()
                            if type(agefolder) != type(None) and agefolder.getFolderNameW() == instAgeName:
                                createAgeFolder = 0
                                break
                        
                        if instAgeName and len(instAgeName) > 0 and createAgeFolder:
                            nfolder = ptVaultFolderNode(0)
                            if type(nfolder) != type(None):
                                nfolder.setFolderNameW(self.IGetAgeInstanceName())
                                nfolder.folderSetType(PtVaultStandardNodes.kAgeTypeJournalFolder)
                                # add to the master age folder folder
                                master_agefolder.addNode(nfolder)
                                # then add to list of folders
                                #...actually, let the folder refresh add 'em in
                            else:
                                PtDebugPrint("xKI: could not create folder for %s" % (self.IGetAgeInstanceName()),level=kErrorLevel)
        else:
            PtDebugPrint("xKI: could not find the master Age jounal folder",level=kErrorLevel)
        #
        # refresh the player list folders
        #
        #   remove all the player lists, so they can be regen'd
        BKPlayerFolderDict.clear()
        del BKPlayerListOrder[:]
        agemembers = kiFolder(PtVaultStandardNodes.kAgeMembersFolder)
        if type(agemembers) != type(None):
            if not BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder)):
                # a new one... add
                BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder))
            BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder)] = agemembers
            PtDebugPrint("xBigKI: updating agemembers ",level=kDebugDumpLevel)
        else:
            PtDebugPrint("xBigKI: AgeMembers folder is missing!",level=kWarningLevel)
        if PhasedKIBuddies:
            buddies = vault.getBuddyListFolder()
            if type(buddies) != type(None):
                if not BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)):
                    # a new one... add
                    BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder))
                BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)] = buddies
            else:
                PtDebugPrint("xBigKI: Buddies folder is missing!",level=kWarningLevel)
        if PhasedKINeighborsInDPL:
            # update the neighborhood folders
            self.IBigKIRefreshNeighborFolders()
        # update the Recent people folder
        PIKA = vault.getPeopleIKnowAboutFolder()
        if type(PIKA) != type(None):
            if not BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder)):
                # a new one... add
                BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder))
            BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder)] = PIKA
        else:
            PtDebugPrint("xBigKI: PeopleIKnowAbout folder is missing!",level=kWarningLevel)
        ignores = vault.getIgnoreListFolder()
        if type(ignores) != type(None):
            if not BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder)):
                # a new one... add
                BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder))
            BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder)] = ignores
        else:
            PtDebugPrint("xBigKI: People I'm ignoring folder is missing!")
        # ==== age visitors ====
        # add in folder separator
        visSep = SeparatorFolder(PtGetLocalizedString("KI.Folders.VisLists"))
        BKPlayerListOrder.append(visSep.name)
        BKPlayerFolderDict[visSep.name] = visSep
        self.IBigKIRefreshAgeVisitorFolders()
        # ====Age owners ===
        self.IBigKIRefreshAgesOwnedFolder()

    def IBigKIRefreshNeighborFolders(self):
        "refresh just the neighborhood folders"
        global BKPlayerFolderDict
        global BKPlayerListOrder
        neighborhood = self.IGetNeighborhood()
        try:
            neighbors = neighborhood.getAgeOwnersFolder()
            if not BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)):
                # a new one... add
                BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder))
            PtDebugPrint("xKI: got the neighbors player folder",level=kDebugDumpLevel)
            BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)] = neighbors
        except AttributeError:
            PtDebugPrint("xBigKI: Neighborhood is missing!",level=kWarningLevel)

    def IBigKIRefreshAgeVisitorFolders(self):
        "refresh the vistor folders for the ages that I own"
        global BKPlayerFolderDict
        global BKPlayerListOrder
        vault = ptVault()
        try:
            myAgesFolder = vault.getAgesIOwnFolder()
            listOfMyAgeLinks = myAgesFolder.getChildNodeRefList()
            for myAgeLinkRef in listOfMyAgeLinks:
                myAgeLink = myAgeLinkRef.getChild()
                myAgeLink = myAgeLink.upcastToAgeLinkNode()
                myAge = myAgeLink.getAgeInfo()
                if type(myAge) != type(None):
                    if self.ICanAgeInviteVistors(myAge,myAgeLink) and (myAge.getAgeFilename() not in kHideAgesHackList) and (myAge.getAgeFilename() != "Myst"):
                        PtDebugPrint("xKI: refreshing visitor list for %s"%(self.IGetAgeDisplayName(myAge)))
                        foldername = xCensor.xCensor(PtGetLocalizedString("KI.Config.OwnerVisitors", [self.IGetAgeDisplayName(myAge)]),theCensorLevel)
                        if not BKPlayerFolderDict.has_key(foldername):
                            # a new one... add
                            PtDebugPrint("xKI: adding visitor list for %s"%(self.IGetAgeDisplayName(myAge)))
                            BKPlayerListOrder.append(foldername)
                        BKPlayerFolderDict[foldername] = myAge
                else:
                    PtDebugPrint("xKI: age info for %s is not ready yet"%(myAgeLink.getUserDefinedName()),level=kErrorLevel)
        except AttributeError:
            PtDebugPrint("xKI: error finding age visitors folder",level=kErrorLevel)

    def IBigKIRefreshAgesOwnedFolder(self):
        "refresh the config folder listing for all the ages I own"
        global BKConfigFolderDict
        global BKConfigListOrder
        global BKConfigDefaultListOrder
        # first get rid of all the age config entries, in case one of the got deleted
        BKConfigFolderDict.clear()
        del BKConfigListOrder[:]
        for config in BKConfigDefaultListOrder:
            BKConfigListOrder.append(config)
        vault = ptVault()
        try:
            myAgesFolder = vault.getAgesIOwnFolder()
            listOfMyAgeLinks = myAgesFolder.getChildNodeRefList()
            for myAgeLinkRef in listOfMyAgeLinks:
                myAgeLink = myAgeLinkRef.getChild()
                myAgeLink = myAgeLink.upcastToAgeLinkNode()
                myAge = myAgeLink.getAgeInfo()
                if type(myAge) != type(None):
                    if self.ICanConfigAge(myAge) and (myAge.getAgeFilename() not in kHideAgesHackList) and (myAge.getAgeFilename() != "Myst"):
                        PtDebugPrint("xKI: refreshing owner config for Age %s"%(self.IGetAgeDisplayName(myAge)),level=kDebugDumpLevel)
                        configname = xCensor.xCensor(PtGetLocalizedString("KI.Config.OwnerConfig", [self.IGetAgeDisplayName(myAge)]),theCensorLevel)
                        if not BKConfigFolderDict.has_key(configname):
                            # a new one... add
                            PtDebugPrint("xKI: adding owner config for Age %s"%(self.IGetAgeDisplayName(myAge)),level=kDebugDumpLevel)
                            BKConfigListOrder.append(configname)
                        BKConfigFolderDict[configname] = myAge
                else:
                    PtDebugPrint("xKI:(AgeOwnerRefresh) age info for %s is not ready yet"%(myAgeLink.getUserDefinedName()),level=kErrorLevel)
        except AttributeError:
            PtDebugPrint("xKI:(AgeOwnerRefresh) error finding age folder",level=kErrorLevel)
        

    def IBigKIRefreshFolderDisplay(self):
        "Refresh the display of the folders and the selection"
        global BKFolderLineDict
        global BKFolderListOrder
        global BKFolderSelected
        global BKFolderTopLine
        # refresh the display of the folders
        id = kBKIIncomingLine
        if len(BKFolderListOrder) != 0:
            # make sure that it is a valid index
            if BKFolderTopLine >= len(BKFolderListOrder):
                BKFolderTopLine = len(BKFolderListOrder)-1
            # if the selected is off the screen to the top then (but only in list mode)
# Need to note when the BKFolderSelected has changed... need to refresh the content display
            if BKRightSideMode == kBKListMode:
                if BKFolderSelected < BKFolderTopLine:
                    BKFolderSelected = BKFolderTopLine
                if BKFolderSelected > BKFolderTopLine+(kBKIFolderLineLast-kBKIIncomingLine):
                    BKFolderSelected = BKFolderTopLine+(kBKIFolderLineLast-kBKIIncomingLine)
                if BKFolderSelected > BKFolderTopLine+len(BKFolderListOrder[BKFolderTopLine:])-1:
                    BKFolderSelected = BKFolderTopLine+len(BKFolderListOrder[BKFolderTopLine])-1
            selectedFolder = BKFolderSelected - BKFolderTopLine + kBKIIncomingLine
            for foldername in BKFolderListOrder[BKFolderTopLine:]:
                folderfield = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(id))
                LONGfolderfield = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(id+500))
                buttonid = id-kBKIFolderLine01+kBKIFolderLineBtn01
                folderbutton = ptGUIControlButton(BigKI.dialog.getControlFromTag(buttonid))
                # make sure its not a separator folder
                if foldername in BKFolderLineDict and isinstance(BKFolderLineDict[foldername],SeparatorFolder):
                    # don't allow them to click on this one
                    folderbutton.hide()
                    # and left justifiy it
                    folderfield.setStringJustify(kLeftJustify)
                    # and color it as a static field (sorta)
                    folderfield.setForeColor(DniStaticColor)
                else:
                    folderbutton.show()
                    # make sure its right justify (default)
                    folderfield.setStringJustify(kRightJustify)
                    # set the color of the folder field
                    if id == selectedFolder:
                        folderfield.setForeColor(DniSelectedColor)
                        LONGfolderfield.setForeColor(DniSelectedColor)
                    else:
                        folderfield.setForeColor(DniSelectableColor)
                        LONGfolderfield.setForeColor(DniSelectableColor)
                folderfield.setStringW(foldername)
                LONGfolderfield.setStringW(foldername)
                id += 1
                if id > kBKIFolderLineLast:
                    break
        # set the up and down buttons if needed
        upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKFolderUpLine))
        if BKFolderTopLine > 0:
            upbtn.show()
        else:
            upbtn.hide()
        dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKFolderDownLine))
        if id > kBKIFolderLineLast:         # have we filled up the listbox?
            # yes, then we need down arrow
            dwnbtn.show()
        else:
            dwnbtn.hide()

        # if there are more folder lines, fill them out to be blank
        #   and disable the button fields while you're at it
        for tagid in range(id,kBKIFolderLineLast+1):
            folderfield = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(tagid))
            folderfield.setForeColor(DniSelectableColor)
            folderfield.setString(" ")
            buttonid = tagid-kBKIFolderLine01+kBKIFolderLineBtn01
            folderbutton = ptGUIControlButton(BigKI.dialog.getControlFromTag(buttonid))
            folderbutton.hide()

    def IBigKINewContentList(self):
        "reget the contents of the folder that is selected, if we are in list mode"
        global BKRightSideMode
        global BKContentList
        global BKContentListTopLine
        # reget the contents of the selected folder
        try:
            foldername = BKFolderListOrder[BKFolderSelected]
            folder = BKFolderLineDict[foldername]
            if type(folder) != type(None):
                if isinstance(folder,ptVaultNode):
                    if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                        try:
                            BKContentList = folder.getCanVisitFolder().getChildNodeRefList()
                        except AttributeError:
                            BKContentList = []
                    else:
                        BKContentList = folder.getChildNodeRefList()
                    self.IBigKIProcessContentList(removeInboxStuff=1)
                    if BKFolderSelectChanged:
                        BKContentListTopLine = 0
                elif isinstance(folder,kiFolder):
                    BKContentList = self.IRemoveCCRPlayers(PtGetPlayerListDistanceSorted())
                    self.IBigKIProcessContentList(removeInboxStuff=1)
                    if BKFolderSelectChanged:
                        BKContentListTopLine = 0
                else:
                    # this really shouldn't happen becuase the user should not be able to click on these kind of folders
                    BKContentList = []
        except (IndexError,KeyError):
            del BKContentList[:]
        self.IBigKIRefreshContentListDisplay()

    def IBigKIRefreshContentList(self):
        "display the contents of the folder that is selected, if we are in list mode"
        global BKRightSideMode
        global BKContentList
        global BKContentListTopLine
        # reget the contents of the selected folder
        try:
            foldername = BKFolderListOrder[BKFolderSelected]
            folder = BKFolderLineDict[foldername]
            if type(folder) != type(None):
                if isinstance(folder,ptVaultNode):
                    if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                        try:
                            BKContentList = folder.getCanVisitFolder().getChildNodeRefList()
                        except AttributeError:
                            BKContentList = []
                    else:
                        BKContentList = folder.getChildNodeRefList()
                    self.IBigKIProcessContentList()
                elif isinstance(folder,kiFolder):
                    BKContentList = self.IRemoveCCRPlayers(PtGetPlayerListDistanceSorted())
                    self.IBigKIProcessContentList()
            else:
                del BKContentList[:]
        except LookupError:
            pass

    def IBigKIProcessContentList(self,removeInboxStuff=0):
        "Do extra processing on content folder list"
        global BKContentList
        global OnlyGetPMsFromBuddies
        # start with nothing in the removelist (remove from current content list)
        removelist = []
        # if player list
        if BKFolderLineDict is BKPlayerFolderDict:
            ignores = ptVault().getIgnoreListFolder()
            # make sure there are some players to process
            if len(BKContentList) > 0:
                # if this is a ptPlayer
                if isinstance(BKContentList[0],ptPlayer):
                    for idx in range(len(BKContentList)):
                        player = BKContentList[idx]
                        if isinstance(player,ptPlayer):
                            if ignores.playerlistHasPlayer(player.getPlayerID()):
                                # put into the removelist
                                removelist.insert(0,idx)
                            else:
                                # its a good player
                                pass
                        else:
                            # not really a player, then remove from the list
                            removelist.insert(0,idx)
                else:
                    # sort the list of players - online up front
                    BKContentList.sort(CMPplayerOnline)
                    # remove all the no-named players and ignored people
                    #~ PtDebugPrint("xKI: START: there are %d players in the this list" % (len(BKContentList)))
                    for idx in range(len(BKContentList)):
                        ref = BKContentList[idx]
                        elem = ref.getChild()
                        if type(elem) != type(None):
                            if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                elem = elem.upcastToPlayerInfoNode()
                                if elem.playerGetName() == "":
                                    #~ PtDebugPrint("xKI: removing %d from content internal list" % (elem.playerGetID()))
                                    # put in them in reverse order in the removelist
                                    removelist.insert(0,idx)
                                # check if they are in the ignore list
                                elif ignores.playerlistHasPlayer(elem.playerGetID()):
                                    # get parent... in some folders the player has to be still visible
                                    parent = ref.getParent()
                                    if parent: parent = parent.upcastToFolderNode()
                                    if type(parent) != type(None):
                                        # make sure this is not the IgnoreList
                                        if parent.folderGetType() != PtVaultStandardNodes.kIgnoreListFolder:
                                            # put in them in reverse order in the removelist
                                            removelist.insert(0,idx)
                                elif (AmICCR and elem.playerGetCCRLevel() > ptCCRMgr().getLevel()) or elem.playerGetCCRLevel() > 0:
                                    removelist.insert(0,idx)
                                else:
                                    # its a good player
                                    pass
                            else:
                                #~ PtDebugPrint("xKI: not a player? its a %d" % (elem.getType()))
                                removelist.insert(0,idx)
                        else:
                            #~ PtDebugPrint("xKI: player, but there is not a node?")
                            removelist.insert(0,idx)
            #~ PtDebugPrint("xKI: END: there are %d players in the this list" % (len(BKContentList)))
        elif BKFolderListOrder[BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
            # look for KIMail from non-Buddies if they only want KIMail from buddies
            vault = ptVault()
            inbox = vault.getInbox()
            buddies = vault.getBuddyListFolder()
            ignores = vault.getIgnoreListFolder()

            for idx in range(len(BKContentList)):
                ref = BKContentList[idx]
                if type(ref) != type(None):
                    if type(ref.getSaver()) == type(None) or ref.getSaverID() == 0:
                        print "Tye: They still haven't fixed getSaver() and getSaverID()!"
                        continue

                    if (OnlyGetPMsFromBuddies and not buddies.playerlistHasPlayer(ref.getSaverID())) or ignores.playerlistHasPlayer(ref.getSaverID()):
                        PtDebugPrint("xKI:remove from inbox because from %s"%(ref.getSaver().playerGetName()),level=kWarningLevel)
                        # remove from our list
                        removelist.insert(0,idx)
                        # only remove from inbox when
                        if removeInboxStuff:
                            PtDebugPrint("xKI:REALLY removed from inbox because from %s, this time"%(ref.getSaver().playerGetName()),level=kWarningLevel)
                            # remove from inbox... not sure how this is going to work!
                            element = ref.getChild()
                            inbox.removeNode(element)
        if len(removelist):
            PtDebugPrint("xKI: removing %d contents from being displayed" % (len(removelist)),level=kWarningLevel)
        for removeidx in removelist:
            del BKContentList[removeidx]

        if BKFolderListOrder[BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
            BKContentList = MarkerJoinRequests + BKContentList
            # also add in the GlobalInbox stuff here
            vault = ptVault()
            ginbox = vault.getGlobalInbox()
            if type(ginbox) != type(None):
                #PtDebugPrint("xKI: Global inbox found",level=kWarningLevel)
                BKContentList = ginbox.getChildNodeRefList() + BKContentList
                BKContentList.sort(CMPNodeDate)

        removelist = []
        for contentidx in range(len(BKContentList)):
            content = BKContentList[contentidx]
            if isinstance(content, ptVaultNodeRef):
                element = content.getChild()
                if type(element) != type(None):
                    if element.getType() == PtVaultNodeTypes.kFolderNode:
                        removelist.insert(0,contentidx)
        for removeidx in removelist:
            del BKContentList[removeidx]

    def IBigKIRefreshContentListDisplay(self):
        "display the contents of the folder that is selected, if we are in list mode"
        global BKRightSideMode
        global BKContentList
        global BKContentListTopLine
        if BKRightSideMode == kBKListMode:
            createfield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(kBKILMTitleCreateLine))
            createBtn = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(kBKIListModeCreateBtn))
            try:
                if BKFolderLineDict is BKPlayerFolderDict:
                    # there might be something different here in the future...
                    if BKFolderListOrder[BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder):
                        createfield.setStringW(PtGetLocalizedString("KI.Player.CreateBuddyTitle"))
                        createBtn.show()
                    else:
                        createfield.setString(" ")
                        createBtn.hide()
                elif BKFolderListOrder[BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
                    createfield.setString(" ")
                    createBtn.hide()
                else:
                    createfield.setString(" ")
                    createBtn.hide()
            except IndexError:
                createfield.setString(" ")
                createBtn.hide()
                if len(BKFolderListOrder) != 0:
                    PtDebugPrint("xKI: Index error BKFolderSelected=%d and list=" % (BKFolderSelected),BKFolderListOrder,level=kWarningLevel)
                return
            id = kBKILMOffsetLine01
            if len(BKContentList) != 0:
                # make sure that the top
                if BKContentListTopLine >= len(BKContentList):
                    BKContentListTopLine = len(BKContentList)-1
                for content in BKContentList[BKContentListTopLine:]:
                    if type(content) != type(None):
                        # put on the list line
                        contentIconJ = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(id+kBKILMIconJournalOffset))
                        contentIconAva = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(id+kBKILMIconPersonOffset))
                        contentIconP = ptGUIControlDynamicText(KIListModeDialog.dialog.getControlFromTag(id+kBKILMIconPictureOffset))
                        contentTitle = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(id+kBKILMTitleOffset))
                        contentDate = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(id+kBKILMDateOffset))
                        contentFrom = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(id+kBKILMFromOffset))
                        if isinstance(content,QuestionNote):
                            contentIconJ.show()
                            contentIconP.hide()
                            contentIconAva.hide()
                            contentTitle.setForeColor(DniSelectableColor)
                            contentTitle.setString(xCensor.xCensor(content.title,theCensorLevel))
                            contentTitle.show()
                            contentDate.setString(" ")
                            contentDate.hide()
                            contentFrom.setForeColor(DniSelectableColor)
                            contentFrom.setFontSize(10)
                            contentFrom.setString(xCensor.xCensor(content.game.master.player.getPlayerName(),theCensorLevel))
                            contentFrom.show()
                            # find the button to enable it
                            lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((id-100)/10)+kBKIListModeCreateBtn))
                            lmbutton.show()
                            id += 10
                            if id > kBKILMOffsetLineLast:
                                break
                        elif isinstance(content,ptPlayer):
                            contentIconAva.show()
                            contentIconJ.hide()
                            contentIconP.hide()
                            contentTitle.setForeColor(DniSelectableColor)
                            contentTitle.setString(xCensor.xCensor(content.getPlayerName(),theCensorLevel))
                            contentTitle.show()
                            contentDate.hide()
                            contentFrom.setForeColor(DniSelectableColor)
                            contentFrom.setFontSize(10)
                            contentFrom.setString(self.IGetAgeInstanceName())
                            contentFrom.show()
                            # find the button to enable it
                            lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((id-100)/10)+kBKIListModeCreateBtn))
                            lmbutton.show()
                            id += 10
                            if id > kBKILMOffsetLineLast:
                                break
                        else:
                            element = content.getChild()
                            if type(element) != type(None):
                                if element.getType() == PtVaultNodeTypes.kTextNoteNode:
                                    element = element.upcastToTextNoteNode()
                                    contentIconJ.show()
                                    contentIconP.hide()
                                    contentIconAva.hide()
                                elif element.getType() == PtVaultNodeTypes.kImageNode:
                                    element = element.upcastToImageNode()
                                    contentIconJ.hide()
                                    contentIconAva.hide()
                                    if contentIconP.getNumMaps() > 0:
                                        dynmap = contentIconP.getMap(0)
                                        image = element.imageGetImage()
                                        dynmap.clearToColor(ptColor(.1,.1,.1,.1))
                                        if type(image) != type(None):
                                            dynmap.drawImage(kBKIImageStartX,kBKIImageStartY,image,0)
                                        else:
                                            pass
                                        dynmap.flush()
                                    contentIconP.show()
                                elif element.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                    element = element.upcastToPlayerInfoNode()
                                    contentIconAva.show()
                                    contentIconJ.hide()
                                    contentIconP.hide()
                                elif element.getType() == PtVaultNodeTypes.kMarkerGameNode:
                                    element = element.upcastToMarkerGameNode()
                                    # no icon for marker folder... yet
                                    contentIconAva.hide()
                                    contentIconJ.hide()
                                    contentIconP.hide()
                                elif element.getType() == PtVaultNodeTypes.kFolderNode:
                                    continue
                                else:
                                    contentIconAva.hide()
                                    contentIconJ.hide()
                                    contentIconP.hide()
                                if isinstance(element,ptVaultPlayerInfoNode):
                                    # if its a player then use the title for the player name
                                    contentTitle.setForeColor(DniSelectableColor)
                                    contentTitle.setString(xCensor.xCensor(element.playerGetName(),theCensorLevel))
                                    contentTitle.show()
                                    contentDate.hide()
                                    contentFrom.setForeColor(DniSelectableColor)
                                    contentFrom.setFontSize(10)
                                    if element.playerIsOnline():
                                        contentFrom.setString(self.IConvertAgeName(element.playerGetAgeInstanceName()))
                                    else:
                                        contentFrom.setString("  ")
                                    contentFrom.show()
                                else:
                                    # else its an image or a text note
                                    if content.getSaverID() == 0:
                                        # must be from the DRC!
                                        contentTitle.setForeColor(DniStaticColor)
                                        contentDate.setForeColor(DniStaticColor)
                                    else:
                                        contentTitle.setForeColor(DniSelectableColor)
                                        contentDate.setForeColor(DniSelectableColor)
                                    
                                    if isinstance(element,ptVaultImageNode):
                                        preText = ""
                                        #if not content.beenSeen():
                                        #    preText = "*"
                                        contentTitle.setString(preText+xCensor.xCensor(element.imageGetTitle(),theCensorLevel))
                                    elif isinstance(element,ptVaultTextNoteNode):
                                        preText = ""
                                        #if not content.beenSeen():
                                        #    preText = "*"
                                        contentTitle.setString(preText+xCensor.xCensor(element.noteGetTitle(),theCensorLevel))

                                    elif isinstance(element,ptVaultMarkerGameNode):
                                        contentTitle.setString(xCensor.xCensor(element.getGameName(),theCensorLevel))

#~Disabled ??? as we'll now allow to click on the title but still not display the game...                                        
#~                                        if PhasedKIShowMarkerGame and gKIMarkerLevel >= kKIMarkerNormalLevel:
#~                                            contentTitle.setString(xCensor.xCensor(element.getGameName(),theCensorLevel))
#~                                        else:
#~                                            contentTitle.setString("???")


                                    else:
                                        #We'll assume that it's still downloading due to network lag and hope for the best!
                                        contentTitle.setString("--[Downloading]--")
                                        contentTitle.setForeColor(DniYellow)

                                        PtDebugPrint("xKI: error - unknown data type in content list. type=%d"%(element.getType()),element,level=kErrorLevel)
                                    contentTitle.show()
                                    try:
                                        tuptime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
                                        curtime = time.strftime(str(PtGetLocalizedString("Global.Formats.Date")),tuptime)
                                    except:
                                        curtime = ""

                                    contentDate.setString(curtime)
                                    contentDate.show()
                                    
                                    sender = content.getSaver()
                                    # see if the saver was us
                                    localplayer = PtGetLocalPlayer()
                                    if type(sender) != type(None) and localplayer.getPlayerID() != sender.playerGetID():
                                        if content.getSaverID() == 0:
                                            # must be from the DRC!
                                            contentFrom.setForeColor(DniStaticColor)
                                            contentFrom.setFontSize(13)
                                            contentFrom.setString("DRC")
                                        else:
                                            contentFrom.setForeColor(DniSelectableColor)
                                            contentFrom.setFontSize(10)
                                            contentFrom.setString(sender.playerGetName())
                                        contentFrom.show()
                                    else:
                                        if content.getSaverID() == 0:
                                            # must be from the DRC!
                                            contentFrom.setString("DRC")
                                            contentFrom.show()
                                        else:
                                            contentFrom.setString("  ")
                                            contentFrom.hide()
                                # find the button to enable it
                                lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((id-100)/10)+kBKIListModeCreateBtn))
                                lmbutton.show()
                                id += 10
                                if id > kBKILMOffsetLineLast:
                                    break
                            else:
                                PtDebugPrint("bigKI: no element inside the content. Doh!",level=kErrorLevel)
                    else:
                        PtDebugPrint("bigKI: no content, even though the folder said it was!",level=kErrorLevel)
            else:
                #~ PtDebugPrint("Nothing in this folder (%s)" % (BKFolderListOrder[BKFolderSelected]))
                pass
            # set the up and down buttons if needed
            upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKLMUpButton))
            if BKContentListTopLine > 0:
                upbtn.show()
            else:
                upbtn.hide()
            dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKLMDownButton))
            if id > kBKILMOffsetLineLast:         # have we filled up the listbox?
                # yes, then we need down arrow
                dwnbtn.show()
            else:
                dwnbtn.hide()
            # if there are more content lines, fill them out to be blank
            #   and disable the button fields while you're at it
            for tagid in range(id,kBKILMOffsetLineLast+10,10):
                iconpic = ptGUIControlDynamicText(KIListModeDialog.dialog.getControlFromTag(tagid+kBKILMIconPictureOffset))
                iconpic.hide()
                iconjrn = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(tagid+kBKILMIconJournalOffset))
                iconjrn.hide()
                iconava = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(tagid+kBKILMIconPersonOffset))
                iconava.hide()
                titlefield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagid+kBKILMTitleOffset))
                titlefield.hide()
                datefield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagid+kBKILMDateOffset))
                datefield.hide()
                fromfield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagid+kBKILMFromOffset))
                fromfield.hide()
                # find the button to disable it
                lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((tagid-100)/10)+kBKIListModeCreateBtn))
                lmbutton.hide()

    def IBigKIDisplayCurrentContentJournal(self):
        "Display a text journal note entry"
        jrnAgeName = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kBKIJRNAgeName))
        jrnAgeName.hide()
        jrnDate = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kBKIJRNDate))
        jrnDate.hide()
        jrnTitle = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kBKIJRNTitle))
        jrnTitle.hide()
        jrnNote = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(kBKIJRNNote))
        jrnNote.hide()
        jrnNote.setBufferLimit(kJournalTextSize)
        jrnDeleteBtn = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kBKIJRNDeleteButton))
        jrnDeleteBtn.hide()
        jrnTitleBtn = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kBKIJRNTitleButton))
        if type(BKCurrentContent) != type(None):
            if self.IsContentMutable(BKCurrentContent):
                jrnDeleteBtn.show()
                jrnNote.unlock()
                if BKInEditMode and BKEditField == kBKEditFieldJRNTitle:
                    pass
                else:
                    jrnTitleBtn.show()
            else:
                jrnNote.lock()
                jrnTitleBtn.hide()
            element = BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kTextNoteNode:
                    element = element.upcastToTextNoteNode()
                    # display the content on the screen
                    jrnAgeName.setString(self.IConvertAgeName(xCensor.xCensor(element.getCreateAgeName(),theCensorLevel)))
                    jrnAgeName.show()
                    tuptime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
                    curtime = time.strftime(str(PtGetLocalizedString("Global.Formats.Date")),tuptime)
                    jrnDate.setString(curtime)
                    jrnDate.show()
                    if BKInEditMode and BKEditField == kBKEditFieldJRNTitle:
                        pass
                    else:
                        jrnTitle.setString(xCensor.xCensor(element.noteGetTitle(),theCensorLevel))
                        jrnTitle.show()
                    if BKInEditMode and BKEditField == kBKEditFieldJRNNote:
                        pass
                    else:
                        encoded = buffer(xCensor.xCensor(element.noteGetText(),theCensorLevel))
                        jrnNote.setEncodedBuffer(encoded)
                        jrnNote.show()
                    self.IBigKISetSeen(BKCurrentContent)
                    # if this came from someone else (and its in the Incoming folder?)...
                    # ... then set the SendTo field so we can reply to them
                    self.ICheckContentForSender(BKCurrentContent)
                else:
                    PtDebugPrint("xBigKI: Display current content - wrong element type %d" % (datatype),level=kErrorLevel)
            else:
                PtDebugPrint("xBigKI: Display current content - element is None",level=kErrorLevel)
        else:
            PtDebugPrint("xBigKI: Display current content - BKCurrentContent is None",level=kErrorLevel)

    def IBigKIDisplayCurrentContentImage(self):
        "Display an image(picture) entry"
        picAgeName = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kBKIPICAgeName))
        picAgeName.hide()
        picDate = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kBKIPICDate))
        picDate.hide()
        picTitle = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kBKIPICTitle))
        picTitle.hide()
        picImage = ptGUIControlDynamicText(KIPictureExpanded.dialog.getControlFromTag(kBKIPICImage))
        picImage.hide()
        picDeleteBtn = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kBKIPICDeleteButton))
        picDeleteBtn.hide()
        picTitleBtn = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kBKIPICTitleButton))
        if type(BKCurrentContent) != type(None):
            if self.IsContentMutable(BKCurrentContent):
                picDeleteBtn.show()
                if BKInEditMode and BKEditField == kBKEditFieldPICTitle:
                    pass
                else:
                    picTitleBtn.show()
            else:
                picTitleBtn.hide()
            element = BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kImageNode:
                    element = element.upcastToImageNode()
                    # display the content on the screen
                    picAgeName.setString(self.IConvertAgeName(xCensor.xCensor(element.getCreateAgeName(),theCensorLevel)))
                    picAgeName.show()
                    tuptime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
                    curtime = time.strftime(str(PtGetLocalizedString("Global.Formats.Date")),tuptime)
                    picDate.setString(curtime)
                    picDate.show()
                    if BKInEditMode and BKEditField == kBKEditFieldPICTitle:
                        pass
                    else:
                        picTitle.setString(xCensor.xCensor(element.imageGetTitle(),theCensorLevel))
                        picTitle.show()
                    if picImage.getNumMaps() > 0:
                        dynmap = picImage.getMap(0)
                        image = element.imageGetImage()
                        dynmap.clearToColor(ptColor(.1,.1,.1,.3))
                        if type(image) != type(None):
                            dynmap.drawImage(kBKIImageStartX,kBKIImageStartY,image,0)
                        else:
                            dynmap.fillRect(kBKIImageStartX,kBKIImageStartY,kBKIImageStartX+800,kBKIImageStartY+600,ptColor(.2,.2,.2,.1))
                        dynmap.flush()
                    picImage.show()
                    self.IBigKISetSeen(BKCurrentContent)
                    # if this came from someone else (and its in the Incoming folder?)...
                    # ... then set the SendTo field so we can reply to them
                    self.ICheckContentForSender(BKCurrentContent)
                else:
                    PtDebugPrint("xBigKI: Display current content - wrong element type %d" % (datatype),level=kErrorLevel)
            else:
                PtDebugPrint("xBigKI: Display current content - element is None",level=kErrorLevel)
        else:
            PtDebugPrint("xBigKI: Display current content - BKCurrentContent is None",level=kErrorLevel)

    def IBigKIDisplayCurrentContentPlayer(self):
        "Display a player element entry"
        global BKPlayerSelected
        global BKGettingPlayerID
        plyName = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYName))
        plyName.hide()
        plyID = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYID))
        plyID.hide()
        plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYPlayerIDEditBox))
        plyIDedit.hide()
        plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYDetail))
        plyDetail.hide()
        plyDeleteBtn = ptGUIControlButton(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYDeleteButton))
        plyDeleteBtn.hide()
        # are we asking for a player id number?
        if BKGettingPlayerID:
            #~ print "setup for getting the player id number"
            plyName.setStringW(PtGetLocalizedString("KI.Player.EnterID"))
            plyName.show()
            plyIDedit.setString("")
            plyIDedit.show()
            plyIDedit.focus()
            KIPlayerExpanded.dialog.setFocus(plyIDedit.getKey())
        else:
            #~ print "displaying the expanded view of a player"
            if type(BKCurrentContent) != type(None):
                if isinstance(BKCurrentContent,ptPlayer):
                    # display the content on the screen
                    plyName.setString(xCensor.xCensor(BKCurrentContent.getPlayerName(),theCensorLevel))
                    plyName.show()
                    idtext = "%08d" % (BKCurrentContent.getPlayerID())
                    plyID.setString(idtext)
                    plyID.show()
                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.InAge", [self.IGetAgeDisplayName()]))
                    plyDetail.show()
                    sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                    BKPlayerSelected = BKCurrentContent
                    sendToField.setString(BKCurrentContent.getPlayerName())
                else:
                    element = BKCurrentContent.getChild()
                    if type(element) != type(None):
                        datatype = element.getType()
                        if datatype == PtVaultNodeTypes.kPlayerInfoNode:
                            element = element.upcastToPlayerInfoNode()
                            # display the content on the screen
                            plyName.setString(xCensor.xCensor(element.playerGetName(),theCensorLevel))
                            plyName.show()
                            idtext = "%08d" % (element.playerGetID())
                            plyID.setString(idtext)
                            plyID.show()
                            if element.playerIsOnline():
                                if element.playerGetAgeInstanceName() == "Cleft":
                                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.InCleft"))
                                elif element.playerGetAgeInstanceName() == "AvatarCustomization":
                                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.InCloset"))
                                else:
                                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.InAge", [self.IConvertAgeName(element.playerGetAgeInstanceName())]))
                            else:
                                plyDetail.setStringW(PtGetLocalizedString("KI.Player.Offline"))
                            plyDetail.show()
                            # determine if this player can be removed from this folder
                            folder = BKCurrentContent.getParent()
                            if folder: folder = folder.upcastToFolderNode()
                            if folder and self.IsFolderContentsMutable(folder):
                                plyDeleteBtn.show()
                            sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                            BKPlayerSelected = BKCurrentContent
                            sendToField.setString(element.playerGetName())
                        else:
                            PtDebugPrint("xBigKI: Display current content - wrong element type %d" % (datatype),level=kErrorLevel)
                    else:
                        PtDebugPrint("xBigKI: Display current content - element is None",level=kErrorLevel)
            else:
                PtDebugPrint("xBigKI: Display current content - BKCurrentContent is None",level=kErrorLevel)

    def IBigKIDisplayCurrentContentMarkerFolder(self):
        "Prepares to display a marker game, as we may need to load it"
        
        #Ensure that we can even view this game....
        if not PhasedKIPlayMarkerGame or not PhasedKIShowMarkerGame or gKIMarkerLevel < kKIMarkerNormalLevel:
            #Let the user know the KI isn't configured to view this game...
            self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionUpgradeKI")
            self.IShowMarkerGameLoading()
            return
      
        #Initialize the markerGameDisplay to the currently selected game
        #But first, we need to ensure that we've got everything necessary!
        if type(BKCurrentContent) == type(None):
            PtDebugPrint("ERROR: xKI.IBigKIDisplayCurrentContentMarkerFolder():\tCould not find the current content selected!")
            return
        
        element = BKCurrentContent.getChild()
        if type(element) == type(None):
            PtDebugPrint("ERROR: xKI.IBigKIDisplayCurrentContentMarkerFolder():\tCould not find the current content child node!")
            return
            
        datatype = element.getType()
        if datatype != PtVaultNodeTypes.kMarkerGameNode:
            PtDebugPrint("ERROR: xKI.IBigKIDisplayCurrentContentMarkerFolder():\tCannot process this node, wrong data type: %s!" %element.getType())
            return

        element = element.upcastToMarkerGameNode()
        PtDebugPrint("DEBUG: xKI.IBigKIDisplayCurrentContentMarkerFolder():\tStarting Marker Game KI Display Manager, loading game: %s guid: %s" %(element.getGameName(), element.getGameGuid()))


        #Now that we're here there's two possibilities: 1) we've just created a game and trying to display it, 2) we've clicked on an existing game
        if self.markerGameDisplay != None:
            if element.getGameGuid() == self.markerGameDisplay.gameData.data['svrGameTemplateID']: 
                # just display the details!
                self.IFinishDisplayCurrentMarkerGame()
                return
       
        #wait, if the currently played game is the game we're editing, we must load it's data
        if self.markerGameManager != None and self.markerGameManager.gameLoaded():
            if self.markerGameManager.gameData.data['svrGameTemplateID'] == element.getGameGuid():
                self.markerGameDisplay = xMarkerGameKIDisplay(self, cachedData = self.markerGameManager.gameData.data)
                self.IFinishDisplayCurrentMarkerGame()
                return

        #display a waiting screen...
        self.IShowMarkerGameLoading()

        # Else we've got to load the existing game...
        self.markerGameDisplay = xMarkerGameKIDisplay(self, element.getGameGuid())
        # All other info should be loaded later after the display has completed!


    def IShowMarkerGameLoading(self):
        "Displays the marker game loading screen for actions that take too long to complete..."
        #Disable all controls until we need them!
        mrkfldTitle = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleText))
        mrkfldTitle.hide()
        mrkfldStatus = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderStatus))
        mrkfldStatus.hide()
        mrkfldOwner = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderOwner))
        mrkfldOwner.hide()
        # hide the up down buttons for the marker list scroll, let the scroll control turn them back on
        mrkfldMLUpBtn = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerListUpBtn))
        mrkfldMLDownBtn = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerListDownBtn))
        mrkfldMLUpBtn.hide()
        mrkfldMLDownBtn.hide()

       
        mbtnInvitePlayer = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderInvitePlayer))
        mbtnInvitePlayer.hide()
        mbtnEditStart = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderEditStartGame))
        mbtnInvitePlayer.hide()
        mbtnPlayEnd = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderPlayEndGame))
        mbtnPlayEnd.hide()

        mtbInvitePlayer = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderInvitePlayerTB))
        mtbInvitePlayer.hide()
        mtbEditStart = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderEditStartGameTB))
        mtbEditStart.hide()
        mtbPlayEnd = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderPlayEndGameTB))
        mtbPlayEnd.hide()

        mrkfldTitleBtn = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleBtn))
        mrkfldTitleBtn.hide()
        mbtnDelete = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderDeleteBtn))
        mbtnDelete.hide()
        mbtnGameTimePullD = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTimePullDownBtn))
        mbtnGameTimePullD.hide()
        mbtnGameTypePullD = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTypePullDownBtn))
        mbtnGameTypePullD.hide()
        mbtnGameTypePullD.disable()
        mbtnGameTimeArrow = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTimeArrow))
        mbtnGameTimeArrow.hide()
        mbtnGameTypeArrow = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTypeArrow))
        mbtnGameTypeArrow.hide()
        mtbGameTime = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTimeTB))
        mtbGameTime.hide()
        mtbGameTimeTitle = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTimeTitleTB))
        mtbGameTimeTitle.hide()
        #mtbGameTimeTitle.setStringW(PtGetLocalizedString("KI.MarkerGame.Time"))
        mtbGameType = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTypeTB))
        mtbGameType.hide()
        mlbMarkerList = ptGUIControlListBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkListbox))
        mlbMarkerList.hide()
        mlbMarkerTextTB = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextTB))
        mlbMarkerTextTB.hide()
        mbtnMarkerText = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextBtn))
        mbtnMarkerText.hide()
        mbtnToran = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderToranIcon))
        mbtnToran.disable()
        mbtnHSpan = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderHSpanIcon))
        mbtnHSpan.disable()
        mbtnVSpan = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderVSpanIcon))
        mbtnVSpan.disable()
        mtbToran = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderToranTB))
        mtbToran.hide()
        mtbHSPan = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderHSpanTB))
        mtbHSPan.hide()
        mtbVSpan = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderVSpanTB))
        mtbVSpan.hide()
        #Whew done disabling all controls!

        #Show the status....
        if self.pendingMGaction != None:
            msg = "----" + self.pendingMGaction + "----"
        else:
            msg = "Please Wait"

        mrkfldTitle.setStringW(msg)
        mrkfldTitle.show()
        mrkfldTitle.refresh()


    def IFinishDisplayCurrentMarkerGame(self):
        "Displays the loaded marker game to the KI"
        global WorkingMarkerFolder
        global MFdialogMode
        global MarkerGameState
        global PhasedKIPlayMarkerGame
    
        #Our game should be loaded, so let's make sure just to be safe!
        if self.markerGameDisplay == None:
            PtDebugPrint("ERROR: xKI.IFinishDisplayCurrentMarkerGame():\tGame was not loaded, aborting displaying the game's details!")
            return

        #Get the marker vault node
        element = BKCurrentContent.getChild()
        if type(element) == type(None):
            PtDebugPrint("ERROR: xKI.IFinishDisplayCurrentMarkerGame():\tCould not finish displaying the marker game, as we the vault node is empty!")
            return
        datatype = element.getType()
        if datatype != PtVaultNodeTypes.kMarkerGameNode:
            PtDebugPrint("ERROR: xKI.IFinishDisplayCurrentMarkerGame():\tCould not finish displaying the marker game, as we have an incorrect vault node type!")
            return
        element = element.upcastToMarkerGameNode()

        questGameFinished = 0

        #Let's save ourselves some typing!  ;)
        mGame = self.markerGameDisplay
        mgData = mGame.gameData.data
        isQuestGame = mgData['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameQuest

        #Determine which mode we're in...
        if self.markerGameManager.gameLoaded():
            #We've got a game in progress, restrict access
            if self.markerGameManager.gameData.data['svrGameTemplateID'] == element.getGameGuid():
                MFdialogMode = kMFPlaying
            else:
                MFdialogMode = kMFOverview
        else:
            #No game in progress, checking status of marker game display
            if mGame.showMarkers:
                if mGame.selectedMarker > -1:
                    MFdialogMode = kMFEditingMarker
                else:
                    MFdialogMode = kMFEditing
            else:
                MFdialogMode = kMFOverview
        #Refresh Mini KI
        self.IRefreshMiniKIMarkerDisplay()
        

         
######=====> This only matters if it is a capture game
#                        # also make sure they are in the right age
#                        if PtGetAgeInfo().getAgeInstanceName() == element.getCreateAgeName():
######=====> This only matters if it is a capture game
        self.IBigKISetToButtons()
        mbtnInvitePlayer = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderInvitePlayer))
        mbtnEditStart = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderEditStartGame))
        mbtnPlayEnd = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderPlayEndGame))
        mrkfldOwner = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderOwner))
        mtbInvitePlayer = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderInvitePlayerTB))
        mtbEditStart = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderEditStartGameTB))
        mtbPlayEnd = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderPlayEndGameTB))
        mrkfldStatus = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderStatus))
        mrkfldTitle = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleText))
        mrkfldTitleBtn = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleBtn))
        mbtnDelete = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderDeleteBtn))
        mbtnGameTimePullD = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTimePullDownBtn))
        mbtnGameTypePullD = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTypePullDownBtn))
        mbtnGameTypePullD.hide()
        mbtnGameTimeArrow = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTimeArrow))
        mbtnGameTypeArrow = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTypeArrow))
        mbtnGameTypeArrow.hide()
        mtbGameTime = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTimeTB))
        mtbGameTimeTitle = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTimeTitleTB))
        mtbGameTimeTitle.setStringW(PtGetLocalizedString("KI.MarkerGame.Time"))
        mtbGameType = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderGameTypeTB))
        mlbMarkerList = ptGUIControlListBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkListbox))
        mlbMarkerTextTB = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextTB))
        mbtnMarkerText = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextBtn))

        mbtnToran = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderToranIcon))
        mbtnToran.disable()
        mbtnHSpan = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderHSpanIcon))
        mbtnHSpan.disable()
        mbtnVSpan = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderVSpanIcon))
        mbtnVSpan.disable()
        mtbToran = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderToranTB))
        mtbHSPan = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderHSpanTB))
        mtbVSpan = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderVSpanTB))

        mbtnEditStart.show()
        mbtnPlayEnd.show()

        if MFdialogMode == kMFOverview:
            # if we are just viewing the MF then don't allow them to change anything
            #  only buttons are: Edit and Play (and delete? and title change?, how about game type?)
            mrkfldTitleBtn.disable()
            mbtnDelete.show()
            mbtnGameTimePullD.hide()
            #mbtnGameTypePullD.hide()
            mbtnGameTimeArrow.hide()
            #mbtnGameTypeArrow.hide()
            mbtnInvitePlayer.hide()
            mtbInvitePlayer.setForeColor(Clear)
            mtbInvitePlayer.setString(" ")
            # if this is a quest game then check to make sure that this IS the owner
            # ..and if it is not a quest game then we must be in the same age as the markerfolder
            if PhasedKIPlayMarkerGame and (not isQuestGame or  element.getCreatorNodeID() == PtGetLocalPlayer().getPlayerID())\
                and (isQuestGame or PtGetAgeInfo().getAgeInstanceName() == element.getCreateAgeName())\
                and not self.markerGameManager.gameLoaded():
                mbtnEditStart.show()
                mtbEditStart.setForeColor(DniColorShowBtn)
            else:
                mbtnEditStart.hide()
                mtbEditStart.setForeColor(DniColorGhostBtn)
            mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.EditButton"))
            mtbEditStart.show()
            # you can only play Capture and Hold games in the age they were intended
            if PhasedKIPlayMarkerGame and (isQuestGame or PtGetAgeInfo().getAgeInstanceName() == element.getCreateAgeName()):
                mbtnPlayEnd.show()
                mtbPlayEnd.setForeColor(DniColorShowBtn)
            else:
                mbtnPlayEnd.hide()
                mtbPlayEnd.setForeColor(DniColorGhostBtn)
            mtbPlayEnd.setString(PtGetLocalizedString("KI.MarkerGame.PlayButton"))
            mtbPlayEnd.show()
            mlbMarkerList.hide()
            mlbMarkerTextTB.hide()
            mbtnToran.hide()
            mbtnHSpan.hide()
            mbtnVSpan.hide()
            mtbToran.hide()
            mtbHSPan.hide()
            mtbVSpan.hide()
            mbtnMarkerText.disable()
        elif MFdialogMode == kMFEditing or MFdialogMode == kMFEditingMarker:
            # if we are editing this
            #  only buttons are: Edit Cancel and Markers
            mrkfldTitleBtn.enable()
            mbtnDelete.hide()

            #mbtnGameTypePullD.show()
            #mbtnGameTypeArrow.show()

            # the quest game does not have a time limit
            if not isQuestGame:
                mbtnGameTimePullD.show()
                mbtnGameTimeArrow.show()
            else:
                mbtnGameTimePullD.hide()
                mbtnGameTimeArrow.hide()
            
            mbtnInvitePlayer.hide()
            mtbInvitePlayer.setForeColor(Clear)
            mtbInvitePlayer.setString(" ")
            mbtnEditStart.show()
            mtbEditStart.setForeColor(DniColorShowBtn)
            mbtnPlayEnd.show()
            mtbPlayEnd.setForeColor(DniColorShowBtn)
            # are we editing the entire game?
            if MFdialogMode == kMFEditing:
                mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.DoneEditButton"))
                mtbEditStart.show()
                mtbPlayEnd.setStringW(PtGetLocalizedString("KI.MarkerGame.AddMarkerButton"))
                mtbPlayEnd.show()
                mlbMarkerList.clearAllElements()
                mlbMarkerList.show()
               
                #Add Markers to the list
                markerList = mgData['markers']
                for marker in markerList:
                    marker = marker.data
                    
                    #Get Dn'i Coords
                    #Here's the real Dn'i Coords
                    coord = ptDniCoordinates()
                    markerPoint = ptPoint3(marker['x'], marker['y'], marker['z'])
                    coord.fromPoint(markerPoint)
                    torans = coord.getTorans()
                    hSpans = coord.getHSpans()
                    vSpans = coord.getVSpans()
                    
                    if isQuestGame:
                        mlbMarkerList.addString("[%s:%d,%d,%d] %s" % (self.IConvertAgeName(marker['age']), torans, hSpans, vSpans, marker['name']))
                    else:
                        mlbMarkerList.addString("[%d,%d,%d] %s" % (torans, hSpans, vSpans, marker['name']))
                
                mlbMarkerTextTB.hide()
                mbtnToran.hide()
                mbtnHSpan.hide()
                mbtnVSpan.hide()
                mtbToran.hide()
                mtbHSPan.hide()
                mtbVSpan.hide()
                mbtnMarkerText.disable()
            # or just editing one of the markers
            else:
                selectedMarker = self.markerGameDisplay.getSelectedMarker()
                if type(selectedMarker) != type(None):
                    # must be editing a marker
                    mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.MarkerListButton"))
                    mtbEditStart.show()
                    mtbPlayEnd.setStringW(PtGetLocalizedString("KI.MarkerGame.RemoveMarkerButton"))
                    mtbPlayEnd.show()
                    mlbMarkerList.hide()
                    mlbMarkerTextTB.show()
                    mlbMarkerTextTB.setString(selectedMarker.data['name'])
                    mbtnToran.show()
                    mbtnHSpan.show()
                    mbtnVSpan.show()
                    mtbToran.show()
                    mtbHSPan.show()
                    mtbVSpan.show()
                    
                    #Get selected marker's coordinates
                    coord = ptDniCoordinates()
                    markerPoint = ptPoint3(selectedMarker.data['x'], selectedMarker.data['y'], selectedMarker.data['z'])
                    coord.fromPoint(markerPoint)

                    mtbToran.setString("%d" % (coord.getTorans()))
                    mtbHSPan.setString("%d" % (coord.getHSpans()))
                    mtbVSpan.setString("%d" % (coord.getVSpans()))
                    mbtnMarkerText.show()
                    mbtnMarkerText.enable()
                else:
                    # error!
                    PtDebugPrint("xKI:Marker: could not find selected marker",level=kErrorLevel)
                    mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.GoBackButton"))
                    mtbEditStart.show()
                    mtbPlayEnd.setString(" ")
                    mtbPlayEnd.show()
                    mlbMarkerList.hide()
                    mlbMarkerTextTB.show()
                    mlbMarkerTextTB.setString("?Unknown mark?")
                    mbtnToran.hide()
                    mbtnHSpan.hide()
                    mbtnVSpan.hide()
                    mtbToran.hide()
                    mtbHSPan.hide()
                    mtbVSpan.hide()
        elif MFdialogMode == kMFPlaying:
            # if we are playing the marker folder
            #  if capture then: Invite, Start and End game
            #  else if quest: then something else
            mrkfldTitleBtn.disable()
            mbtnDelete.hide()
            mbtnGameTimePullD.hide()
            #mbtnGameTypePullD.hide()
            mbtnGameTimeArrow.hide()
            #mbtnGameTypeArrow.hide()
            mbtnToran.hide()
            mbtnHSpan.hide()
            mbtnVSpan.hide()
            mtbToran.hide()
            mtbHSPan.hide()
            mtbVSpan.hide()
            mbtnMarkerText.disable()
            if not isQuestGame:
                if MarkerGameState == kMGGameOn:
                    mtbInvitePlayer.setForeColor(DniColorGhostBtn)
                    mbtnInvitePlayer.hide()
                    mtbEditStart.setForeColor(DniColorGhostBtn)
                    mbtnEditStart.hide()
                else:
                    mbtnInvitePlayer.show()
                    mtbInvitePlayer.setForeColor(DniColorShowBtn)
                    mtbEditStart.setForeColor(DniColorShowBtn)
                    mbtnEditStart.show()
                mtbInvitePlayer.setStringW(PtGetLocalizedString("KI.MarkerGame.InvitePlayerButton"))
                mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.StartGameButton"))
                mtbEditStart.show()
                mbtnPlayEnd.show()
                mtbPlayEnd.setForeColor(DniColorShowBtn)
                mtbPlayEnd.setStringW(PtGetLocalizedString("KI.MarkerGame.EndGameButton"))
                mtbPlayEnd.show()                
                mlbMarkerList.hide()
                mlbMarkerTextTB.hide()
######====> different for Quest game
            else:
                mbtnInvitePlayer.hide()
                mbtnEditStart.show()
                mtbEditStart.setForeColor(DniColorShowBtn)
                mtbEditStart.setString(PtGetLocalizedString("KI.MarkerGame.StopPlayingButton"))
                mtbEditStart.show()
                mbtnPlayEnd.show()
                mtbPlayEnd.setForeColor(DniColorShowBtn)
                mtbPlayEnd.setString(PtGetLocalizedString("KI.MarkerGame.ResetGameButton"))
                mtbPlayEnd.show()
                mlbMarkerList.clearAllElements()
                mlbMarkerList.show()
                # assume that the game is finished unless an unseen marker is still left
                questGameFinished = 1
                # add the markers into the list
                for marker in self.markerGameManager.gameData.data['markers']:
                    marker = marker.data
                    if marker['captured']:
                        #Get marker's coordinates
                        coord = ptDniCoordinates()
                        markerPoint = ptPoint3(marker['x'], marker['y'], marker['z'])
                        coord.fromPoint(markerPoint)
                        mlbMarkerList.addString("[%s:%d,%d,%d] %s" % (self.IConvertAgeName(marker['age']),coord.getTorans(), coord.getHSpans(), coord.getVSpans(), marker['name']))
                    else:
                        questGameFinished = 0
                mlbMarkerTextTB.hide()
        # refresh the text of the buttons (color changed)
        mtbInvitePlayer.refresh()
        mtbEditStart.refresh()
        mtbPlayEnd.refresh()
        # display the content on the screen
        mrkfldTitle.setString(xCensor.xCensor(mgData['svrGameName'],theCensorLevel))
        mrkfldTitle.show()
        #Enable the editable Title
        mrkfldTitleBtn.show()
        mrkfldTitleBtn.enable()
        
        count = self.markerGameDisplay.getNumMarkers()
        if not isQuestGame or MFdialogMode == kMFEditing or MFdialogMode == kMFEditingMarker:
            if count == 0:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusNoMarkers")
            elif count == 1:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusOneMarker")
            else:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusNMarkers", [str(count)])
            if not isQuestGame:
                statusLine += PtGetLocalizedString("KI.MarkerGame.StatusIn", [self.IConvertAgeName(element.getCreateAgeName())])
        else:
            if questGameFinished:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusAllFound")
            else:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusNotAllFound")
        mrkfldStatus.setStringW(statusLine)
        mrkfldStatus.show()

        creatorID = element.getCreatorNodeID()
        
        #We now can get this info from the server using a direct search without the need for cache! (Thanks EAP)
        tempNode = ptVaultPlayerInfoNode()
        tempNode.playerSetID(creatorID)

        try:
            vault = ptVault()
            creatorName = vault.findNode(tempNode).upcastToPlayerInfoNode().playerGetName()
        except:
            creatorName = ""

        mrkfldOwner.setStringW(PtGetLocalizedString("KI.MarkerGame.OwnerTitle") + U" %s [ID:%08d]" % (creatorName,creatorID))
        mrkfldOwner.show()
        minutes = int(mgData['timeLimit']/60)
        mtbGameTime.setString("%d min" % (minutes))
        gameType = "?Unknown? game"
        if mgData['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCapture:
            gameType = PtGetLocalizedString("KI.MarkerGame.CaptureGame")
            mtbGameTime.show()
            mtbGameTimeTitle.show()
        elif mgData['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCaptureAndHold:
            gameType = PtGetLocalizedString("KI.MarkerGame.HoldGame")
            mtbGameTime.show()
            mtbGameTimeTitle.show()
        elif isQuestGame:
            gameType = PtGetLocalizedString("KI.MarkerGame.QuestGame")
            mtbGameTime.hide()
            mtbGameTimeTitle.hide()
        mtbGameType.setStringW(gameType)
        mtbGameType.show()


    def IBigKIDisplayCurrentQuestionNote(self):
        "Display a question note"
        qnTitle = ptGUIControlTextBox(KIQuestionNote.dialog.getControlFromTag(kQNTitle))
        qnNote = ptGUIControlMultiLineEdit(KIQuestionNote.dialog.getControlFromTag(kQNMessage))
        qnAcceptTB = ptGUIControlTextBox(KIQuestionNote.dialog.getControlFromTag(kQNAcceptText))
        qnDeclineTB = ptGUIControlTextBox(KIQuestionNote.dialog.getControlFromTag(kQNDeclineText))
        if type(BKCurrentContent) != type(None):
            if isinstance(BKCurrentContent,QuestionNote):
                # display the content on the screen
                qnTitle.setString(BKCurrentContent.title)
                qnNote.setString(BKCurrentContent.message)
                qnAcceptTB.setString(BKCurrentContent.YesBtnText)
                qnDeclineTB.setString(BKCurrentContent.NoBtnText)
            else:
                PtDebugPrint("xBigKI:QuestionNote: Unknown data type",level=kErrorLevel)
        else:
            PtDebugPrint("xBigKI:QuestionNote: Display current content - BKCurrentContent is None",level=kErrorLevel)

    def IBigKICheckSavePlayer(self):
        "save after player edited"
        global BKGettingPlayerID
        if BKGettingPlayerID:
            # this should create and save a player element into buddies
            BKGettingPlayerID = 0
            plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYPlayerIDEditBox))
            if not plyIDedit.wasEscaped():
                id,msg = self.IGetPIDMsg(plyIDedit.getString())
                if id:
                    localplayer = PtGetLocalPlayer()
                    if id != localplayer.getPlayerID():
                        vault = ptVault()
                        buddies = vault.getBuddyListFolder()
                        if type(buddies) != type(None):
                            if buddies.playerlistHasPlayer(id):
                                plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYDetail))
                                plyDetail.setStringW(PtGetLocalizedString("KI.Player.AlreadyAdded"))
                                plyDetail.show()
                                BKGettingPlayerID = 1   # back in business... of asking them for a number
                            else:
                                buddies.playerlistAddPlayer(id)
                                self.IDoStatusChatMessage(PtGetLocalizedString("KI.Player.Added"),netPropagate=0)
                        if not BKGettingPlayerID:
                            self.IBigKIChangeMode(kBKListMode)
                    else:
                        plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYDetail))
                        plyDetail.setStringW(PtGetLocalizedString("KI.Player.NotYourself"))
                        plyDetail.show()
                        BKGettingPlayerID = 1   # back in business... of asking them for a number
                else:
                    plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kBKIPLYDetail))
                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.NumberOnly"))
                    plyDetail.show()
                    BKGettingPlayerID = 1   # back in business... of asking them for a number
            else:
                # nothing here... just go back to list mode
                self.IBigKIChangeMode(kBKListMode)

    def IBigKICreateJournalNote(self):
        "create a note and add to the Journal"
        global BKCurrentContent
        global BKJournalFolderDict
        global BKJournalListOrder
        global BKJournalFolderSelected
        global BKJournalFolderTopLine
        global BKFolderSelected
        global BKFolderTopLine
        PtDebugPrint("xBigKI: create text note message",level=kDebugDumpLevel)
        # if there is no folder list, then make one
        if len(BKFolderListOrder) == 0:
            self.IBigKIRefreshFolders()
        try:
            journal = BKJournalFolderDict[self.IGetAgeInstanceName()]
            if type(journal) != type(None):
                # make sure that the age folder is selected
                BKFolderTopLine = BKJournalFolderTopLine = 0      # scroll back to the top
                BKFolderSelected = BKJournalFolderSelected = BKJournalListOrder.index(self.IGetAgeInstanceName())
                # create the note
                note = ptVaultTextNoteNode(0)
                
                note.setTextW(PtGetLocalizedString("KI.Journal.InitialMessage"))
                note.setTitleW(PtGetLocalizedString("KI.Journal.InitialTitle"))
                
                BKCurrentContent = journal.addNode(note)
                return BKCurrentContent
            else:
                PtDebugPrint("xBigKI: create journal note, journal not ready",level=kErrorLevel)
                return None
        except KeyError:
            PtDebugPrint("xKI:BigKI - could not find journal for this age -%s" % (self.IGetAgeInstanceName()),level=kErrorLevel)

    def IBigKICreateJournalImage(self,image,useScreenShot = false):
        "create a journal entry that is a picture"
        global BKCurrentContent
        global BKJournalFolderDict
        global BKJournalListOrder
        global BKJournalFolderSelected
        global BKJournalFolderTopLine
        global BKFolderSelected
        global BKFolderTopLine
        PtDebugPrint("xBigKI: create a picture element from ",image,level=kDebugDumpLevel)
        # if there is no folder list, then make one
        if len(BKFolderListOrder) == 0:
            self.IBigKIRefreshFolders()
        try:
            journal = BKJournalFolderDict[self.IGetAgeInstanceName()]
            if type(journal) != type(None):
                # make sure that the age folder is selected
                BKFolderTopLine = BKJournalFolderTopLine = 0      # scroll back to the top
                BKFolderSelected = BKJournalFolderSelected = BKJournalListOrder.index(self.IGetAgeInstanceName())
                # create the image entry
                img_elem = ptVaultImageNode(0)
                if useScreenShot:
                    img_elem.setImageFromScrShot()
                else:
                    img_elem.imageSetImage(image)
                img_elem.setTitleW(PtGetLocalizedString("KI.Image.InitialTitle"))
                BKCurrentContent = journal.addNode(img_elem)
                return BKCurrentContent
            else:
                PtDebugPrint("xBigKI: create journal image, journal not ready",level=kErrorLevel)
                return None
        except KeyError:
            PtDebugPrint("xKI:BigKI - could not find journal for this age -%s" % (self.IGetAgeInstanceName()),level=kErrorLevel)

    def IBigKIEnterEditMode(self,whichfield):
        "enter into edit mode for a particular field"
        global BKCurrentContent
        global BKInEditMode
        global BKEditContent
        global BKEditField
        global BKEditFieldIDs
        # can't be in chatting mode
        self.IEnterChatMode(0)
        # see if we were already in edit mode, save that before re-entrying into edit mode
        if BKInEditMode:
            self.IBigKISaveEdit()
        #~ if whichfield == kBKEditFieldJRNTitle or whichfield == kBKEditFieldJRNNote:
        if whichfield == kBKEditFieldJRNTitle:
            textbox = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[whichfield][kBKEditIDtextbox]))
            button = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[whichfield][kBKEditIDbutton]))
            editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[whichfield][kBKEditIDeditbox]))
        elif whichfield == kBKEditFieldPICTitle:
            textbox = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[whichfield][kBKEditIDtextbox]))
            button = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[whichfield][kBKEditIDbutton]))
            editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[whichfield][kBKEditIDeditbox]))
            editbox.setStringSize(56)
        else:
            textbox = None
            button = None
            editbox = None
        # make sure that we have a valid thing to edit
        if type(textbox) != type(None):
            if type(BKCurrentContent) != type(None):
                ed_element = BKCurrentContent.getChild()
            else:
                ed_element = None
            if type(ed_element) != type(None):
                BKInEditMode = 1
                BKEditContent = BKCurrentContent
                BKEditField = whichfield
                # hide the textbox and the button
                textbox.hide()
                button.hide()
                # set the edit box and display it
                if BKEditField == kBKEditFieldJRNTitle:
                    ed_element = ed_element.upcastToTextNoteNode()
                    editbox.setString(xCensor.xCensor(ed_element.noteGetTitle(),theCensorLevel))
                    KIJournalExpanded.dialog.setFocus(editbox.getKey())
                elif BKEditField == kBKEditFieldPICTitle:
                    ed_element = ed_element.upcastToImageNode()
                    editbox.setString(xCensor.xCensor(ed_element.imageGetTitle(),theCensorLevel))
                    KIPictureExpanded.dialog.setFocus(editbox.getKey())
                else:
                    editbox.setString("")
                editbox.end()
                editbox.show()
                editbox.focus()
                if whichfield == kBKEditFieldJRNTitle or whichfield == kBKEditFieldJRNNote:
                    KIJournalExpanded.dialog.refreshAllControls()
                elif whichfield == kBKEditFieldPICTitle:
                    KIPictureExpanded.dialog.refreshAllControls()
            else:
                PtDebugPrint("xKI:BigKI:EnterEdit content has no element to edit?")
        else:
            # might be for the journal edit?
            if whichfield == kBKEditFieldJRNNote:
                # if so, then its kinda automatically in edit mode
                BKInEditMode = 1
                BKEditContent = BKCurrentContent
                BKEditField = whichfield

    def IBigKISaveEdit(self, noExitEditMode = 0):
        "save whatever they were editing to the right place"
        global BKCurrentContent
        global BKInEditMode
        global BKEditContent
        global BKEditField
        global BKEditFieldIDs

        if BKInEditMode:
            if BKEditField == kBKEditFieldJRNTitle:
                textbox = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDtextbox]))
                button = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDbutton]))
                editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDeditbox]))
            elif BKEditField == kBKEditFieldJRNNote:
                textbox = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDtextbox]))
                button = None
                editbox = None
            elif BKEditField == kBKEditFieldPICTitle:
                textbox = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDtextbox]))
                button = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDbutton]))
                editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDeditbox]))
            else:
                textbox = None
                button = None
                editbox = None

            # make sure that we have a valid thing to edit
            if type(textbox) != type(None):
                if type(BKEditContent) != type(None):
                    ed_element = BKEditContent.getChild()
                    if type(ed_element) != type(None):
                        if type(editbox) != type(None):
                            if not editbox.wasEscaped():
                                textbox.setString(editbox.getString())
                                if BKEditField == kBKEditFieldJRNTitle:
                                    ed_element = ed_element.upcastToTextNoteNode()
                                    jtitle = editbox.getStringW()

                                    if jtitle[:len(PtGetLocalizedString("KI.Journal.InitialTitle"))] == PtGetLocalizedString("KI.Journal.InitialTitle"):
                                        # make sure that they actually added something (so as not to get a blank title)
                                        if jtitle != PtGetLocalizedString("KI.Journal.InitialTitle"):
                                            jtitle = jtitle[len(PtGetLocalizedString("KI.Journal.InitialTitle")):]
                                    ed_element.setTitleW(jtitle)
                                elif BKEditField == kBKEditFieldPICTitle:
                                    ed_element = ed_element.upcastToImageNode()
                                    ptitle = editbox.getStringW()

                                    if ptitle[:len(PtGetLocalizedString("KI.Image.InitialTitle"))] == PtGetLocalizedString("KI.Image.InitialTitle"):
                                        # make sure that they actually added something (so as not to get a blank title)
                                        if ptitle != PtGetLocalizedString("KI.Image.InitialTitle"):
                                            ptitle = ptitle[len(PtGetLocalizedString("KI.Image.InitialTitle")):]
                                    ed_element.setTitleW(ptitle)
                                # save 'em
                                ed_element.save()
                        else:
                            if BKEditField == kBKEditFieldJRNNote:
                                buf = textbox.getEncodedBufferW()
                                
                                if buf[:len(PtGetLocalizedString("KI.Journal.InitialMessage"))] == PtGetLocalizedString("KI.Journal.InitialMessage"):
                                    buf = buf[len(PtGetLocalizedString("KI.Journal.InitialMessage")):]
                                ed_element = ed_element.upcastToTextNoteNode()
                                ed_element.setTextW(buf)
                                # save 'em
                                ed_element.save()
                if BKEditField != kBKEditFieldJRNNote:
                    # put back the fields in to no-edit mode
                    textbox.show()
                    button.show()
                    editbox.hide()
            if not noExitEditMode:
                # take us out of editing
                BKInEditMode = 0
                BKEditContent = None
                BKEditField = -1

    def IBigKICheckFocusChange(self):
        "The focus has changed, see if we need to close the editing"
        global BKInEditMode
        global BKEditField
        global BKEditFieldIDs
        if BKInEditMode:
            if BKEditField == kBKEditFieldJRNTitle:
                editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDeditbox]))
            elif BKEditField == kBKEditFieldPICTitle:
                editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(BKEditFieldIDs[BKEditField][kBKEditIDeditbox]))
            else:
                editbox = None
            if type(editbox) != type(None):
                if editbox.isFocused():
                    return
            self.IBigKISaveEdit()
 
    def IBigKISetSeen(self,content):
        # they could only have seen the element if the bigKI is displaying
        if BigKI.dialog.isEnabled():
            content.setSeen()

    def ISetPlayerNotFound(self,message):
        "Set the PlayerSend to field to 'Player not found'"
        global BKPlayerSelected
        # if they are no longer around, remove the evidence
        BKPlayerSelected = None
        sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
        sendToField.setStringW(U"<"+unicode(message)+U">")
        sendToButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(kBKIToPlayerButton))
        sendToButton.hide()

    def ICheckContentForSender(self,content):
        "Check incoming content to see if there is a sender (and from Incoming) and set SendTo field"
        global BKPlayerSelected
        folder = content.getParent()
        if folder: folder = folder.upcastToFolderNode()
        if type(folder) != type(None) and folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
            sender = content.getSaver()
            if type(sender) != type(None) and sender.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPlayerLine))
                curSendTo = sendToField.getString().strip()
                if curSendTo == "" or len(curSendTo) <= 0:
                    BKPlayerSelected = sender
                    sendToField.setString(sender.playerGetName())

    def IInviteToVisit(self,playerID,ageInfo):
        "Invite someone to visit our age"
        whereToLink = ptAgeLinkStruct()
        whereToLink.setAgeInfo(ageInfo.asAgeInfoStruct())
        ptVault().invitePlayerToAge(whereToLink,playerID)
        self.ISendInviteRevoke(playerID,ageInfo.getDisplayName(),"KI.Invitation.VisitTitle","KI.Invitation.VisitBody")

    def IRevokeToVisit(self,playerID,ageInfo):
        "Revoke someone from visiting our age"
        ptVault().unInvitePlayerToAge(ageInfo.getAgeInstanceGuid(),playerID)

    def ISendInviteRevoke(self,playerID,ageName,title,message):
        "Send an email invitation or revokatation"
        localPlayer = PtGetLocalPlayer()
        invite = ptVaultTextNoteNode(0)
        invite.noteSetText(str(PtGetLocalizedString(message, [ageName,localPlayer.getPlayerName()])))
        invite.noteSetTitle(str(PtGetLocalizedString(title, [ageName])))
        invite.sendTo(playerID)

    def IShowSelectedConfig(self):
        "display the config dialog that is selected"
        global BKRightSideMode
        if BKConfigListOrder[BKFolderSelected] == PtGetLocalizedString("KI.Config.Settings"):
            self.IBigKIChangeMode(kBKKIExpanded)
        elif BKConfigListOrder[BKFolderSelected] == PtGetLocalizedString("KI.Config.Volume"):
            self.IBigKIChangeMode(kBKVolumeExpanded)
        else:
            # is the dialog already showing
            if BKRightSideMode != kBKAgeOwnerExpanded:
                # nope then show it
                self.IBigKIChangeMode(kBKAgeOwnerExpanded)
            else:
                # else just refresh what's in the dialog
                self.IRefreshAgeOwnerSettings()
                self.IBigKIOnlySelectedToButtons()

    def ISaveUserNameFromEdit(self,control):
        newtitle = ""
        try:
            # get the selected age config setting
            myAge = BKConfigFolderDict[BKConfigListOrder[BKFolderSelected]]
            if not control.wasEscaped():
                # set the new title
                myAge.setAgeUserDefinedName(control.getStringW())
                myAge.save()
                PtDebugPrint("KIAgeOwner: updating title to: %s"%(control.getStringW()),level=kDebugDumpLevel )
            else:
                PtDebugPrint("KIAgeOwner: escape hit!",level=kDebugDumpLevel )
            newtitle = myAge.getDisplayName()
        except LookupError:
            PtDebugPrint("KIAgeOwner: where's the stinking age!",level=kDebugDumpLevel )
            myAge = None
        control.hide()
        # reanble the button and text
        title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleTB))
        title.setStringW(newtitle)
        title.show()
        titlebtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kBKAgeOwnerTitleBtn))
        titlebtn.enable()

    def ISaveMarkerFolderNameFromEdit(self,control):
        global theCensorLevel
        title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleText))
        if type(BKCurrentContent) != type(None):
            element = BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kMarkerGameNode:
                    element = element.upcastToMarkerGameNode()
                    if type(element) != type(None):
                        if not control.wasEscaped() and control.getString() != "":
                            # set the new title
                            #This is a quick fix, really the action needs to happen after the server has registered the game name.
                            #But we're going to do it as we need to set the item we're editing immediately!
                            #If the game server gets out of sync, then the user will just need to set it again!
                            newText = xCensor.xCensor(control.getString(),theCensorLevel)
                            #newText = newText.strip("\"")
                            element.setGameName(control.getString())
                            title.setString(control.getString())
                            element.save()
                            PtDebugPrint("KIAgeOwner: updating title to: %s"%(newText),level=kDebugDumpLevel )
                            self.IRefreshPlayerList()
                            self.IRefreshPlayerListDisplay()
                            self.markerGameDisplay.setGameName(newText)
                        else:
                            PtDebugPrint("KIAgeOwner: escape hit!",level=kDebugDumpLevel )
        control.hide()
        # reanble the button and text
        titlebtn = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleBtn))
        titlebtn.enable()
        title.show()

    def ISaveMarkerTextFromEdit(self,control):
        global theCensorLevel
        title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextTB))
        if type(BKCurrentContent) != type(None):
            element = BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kMarkerGameNode:
                    element = element.upcastToMarkerGameNode()
                    if type(element) != type(None):
                        if not control.wasEscaped() and control.getString() != "":
                            newText = xCensor.xCensor(control.getString(),theCensorLevel)
                            # find the selected marker
                            self.markerGameDisplay.setNameOfSelectedMarker(newText)
                        else:
                            PtDebugPrint("KImarkerText: escape hit!",level=kDebugDumpLevel )
        control.hide()
        # reanble the button and text
        titlebtn = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextBtn))
        titlebtn.enable()
        title.show()


##############################################################
##
##  New Item Alert (sorta) private methods
##
##############################################################
    def ICheckInboxForUnseen(self):
        "Get the Inbox and see if there are any new messages"
        vault = ptVault()
        infolder = vault.getInbox()
        if type(infolder) != type(None):
            inreflist = infolder.getChildNodeRefList()
            # is there any that haven't been seen yet
            for inref in inreflist:
                if not inref.beenSeen():
                    self.IAlertKIStart()

    def IAlertKIStart(self):
        "start the alert, unless its already going"
        global AlertTimerActive
        global AlertTimeToUse
        if not PtIsSinglePlayerMode():
            if theKILevel >= kNormalKI:
                if not AlertTimerActive:
                    PtDebugPrint("xKI: show KI alert",level=kDebugDumpLevel)
                    NewItemAlert.dialog.show()
                kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                AlertTimeToUse = kAlertTimeDefault
                kialert.show()

    def IAlertBookStart(self,time=kAlertTimeDefault):
        "start the alert, unless its already going"
        global AlertTimerActive
        global AlertTimeToUse
        if not AlertTimerActive:
            PtDebugPrint("xKI: show Book alert",level=kDebugDumpLevel)
            NewItemAlert.dialog.show()
        bookalert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
        AlertTimeToUse = time
        bookalert.show()

    def IAlertStop(self):
        "stop the alert, ie. hide the alert dialog"
        global AlertTimerActive
        global AlertTimeToUse
        AlertTimerActive = 0
        NewItemAlert.dialog.hide()
        kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
        kialert.hide()
        bookalert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
        bookalert.hide()

    def IAlertStartTimer(self):
        "start the alert timer, unless already going"
        global AlertTimerActive
        if not AlertTimerActive:
            AlertTimerActive = 1
            PtAtTimeCallback(self.key,AlertTimeToUse,kAlertHideTimer)


    def ICheckCalibrationProgress(self):
        for mg in grtzMarkerGames.mgs:
            gameName = mg[1]
            startTime = 1.0
            bestTime = 0.0
            vault = ptVault()
            entry = vault.findChronicleEntry(gameName)
            if (type(entry) != type(None)):
                progressString = entry.chronicleGetValue()
                progList = progressString.split(',')
                if (len(progList) == 2):
                    try:
                        startTime = string.atof(progList[0])
                    except ValueError:
                        pass
                    try:
                        bestTime = string.atof(progList[1])
                    except ValueError:
                        pass
            else:
                PtDebugPrint('game missing -> no GPS', level=kDebugDumpLevel)
                return 
            if (bestTime == 0):
                PtDebugPrint('incomplete game found -> no GPS', level=kDebugDumpLevel)
                return 

        PtDebugPrint('all checks passed -> enable GPS', level=kDebugDumpLevel)
        self.IEnableGPS()


    def IEnableGPS(self):
        vault = ptVault()
        psnlSDL = vault.getPsnlAgeSDL()
        if psnlSDL:
            GPSVar = psnlSDL.findVar('GPSEnabled')
            if (GPSVar.getBool() == 0):
                GPSVar.setBool(1)
                vault.updatePsnlAgeSDL(psnlSDL)


    def GetPelletKIScore(self):
        scoreList = ptScoreMgr().getPlayerScores("PelletDrop")
        if scoreList:
            score = scoreList[0].getValue()
            print "xKI.GetPelletKIScore(): pellet score = ",score
            return score
        else:
            print "xKI.GetPelletKIScore(): no pellets dropped yet, will show default score of '000'"
            return -1


    def IUpdatePelletScore(self):
        pelletTextBox = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kBKIPelletDrop))
        pelletScore = self.GetPelletKIScore()
        if pelletScore >= 0:
            pelletScore = str(pelletScore)
            pelletTextBox.setString(pelletScore)
            #pelletText.show()
        else:
            pelletScore = "000"
            pelletTextBox.setString(pelletScore)
            #pelletText.show()


    def IUploadPelletScore(self):
        pelletscore = self.GetPelletKIScore()
        if pelletscore > 0:
            self.ITransferPelletScore(pelletscore)

            hoodinfoupdate = PtFindActivator("PythHoodInfoImagerUpdater")
            print "hoodinfoupdate: ", hoodinfoupdate
            if hoodinfoupdate:
                notify = ptNotify(self.key)
                notify.clearReceivers()
                notify.addReceiver(hoodinfoupdate)
                notify.netPropagate(1)
                notify.netForce(1)
                notify.setActivate(1.0)
                sname = "Score=%s" % (PtGetLocalPlayer().getPlayerName())
                notify.addVarNumber(sname, pelletscore)
                notify.send()
                print "sending score notify: ", sname, " ", pelletscore

            
    def ITransferPelletScore(self, pelletscore):
        ##Get Hood Score
        hoodScoreList = ptScoreMgr().getCurrentAgeScores("PelletDrop")
        if hoodScoreList:
            hoodScoreID = hoodScoreList[0].getScoreID()
            print "Got the Hood Score ID from existing score"
        else:
            newHoodScore = ptScoreMgr().createCurrentAgeScore("PelletDrop", PtGameScoreTypes.kAccumulative, 0)
            hoodScoreID = newHoodScore.getScoreID()
            print "Got the Hood Score ID from new score"

        ##Check for total score
        totalScoreList = ptScoreMgr().getPlayerScores("PelletTotal")
        if totalScoreList:
            oldTotal = totalScoreList[0].getValue()
            print "Old Total Pellet Score:", oldTotal
            newpoints = totalScoreList[0].addPoints(pelletscore)
            currentTotal = totalScoreList[0].getValue()
            print "Current Total Pellet Score:", currentTotal
            
        else:
            pelletTotalScoreNew = ptScoreMgr().createPlayerScore("PelletTotal", PtGameScoreTypes.kAccumulative, pelletscore)
            if pelletTotalScoreNew == None:
                print "hmm, initial total score says it's none, we've got a problem"            
                
        ##Reset Current Pellet Score
        scoreList = ptScoreMgr().getPlayerScores("PelletDrop")
        if scoreList:
            resetPelletScore = scoreList[0].transferPoints(hoodScoreID,scoreList[0].getValue())
            if resetPelletScore:
                print "Score Successfully reset."
            else:
                print "Score had a problem resetting."
    

################################################################
##
##  class helper - DeviceFolder and Device
##
################################################################
class kiFolder:
    def __init__(self,foldertype):
        self.type = foldertype
        self.name = xLocTools.FolderIDToFolderName(self.type)

class Device:
    def __init__(self,name):
        try:
            idx = name.index("/type=")
            self.type = name[idx+len("/type="):]
            name = name[:idx]
        except (LookupError,ValueError):
            # assume that the default device is an imager
            self.type = "imager"
        self.name = name

    def __eq__(self,other):
        if self.name == other.name:
            return 1
        else:
            return 0

    def __ne__(self,other):
        if self.name == other.name:
            return 0
        else:
            return 1

# a folder type for devices like the imager
class DeviceFolder:
    def __init__(self,name):
        self.name = name
        self.dflist = []

    def __getitem__(self,key):
        return self.dflist[key]

    def __setitem__(self,key,value):
        self.dflist[key] = value

    def append(self,value):
        self.dflist.append(value)

    def remove(self,value):
        self.dflist.remove(value)

    def removeAll(self):
        self.dflist = []

    def index(self,value):
        return self.dflist.index(value)

    def __getslice__(self,i,j):
        return self.self.dflist[i:j]

    def __len__(self):
        return len(self.dflist)

# a folder type that is just for a separator
class SeparatorFolder:
    def __init__(self,name):
        self.name = name


class ChatFlags:
    def __init__(self,flags):
        self.__dict__['flags'] = flags
        # assume that its a normal broadcast message
        self.__dict__['broadcast'] = 1
        # and not sending to one's self
        self.__dict__['toSelf'] = 0
        if flags & kRTChatPrivate:
            self.__dict__['private'] = 1
            self.__dict__['broadcast'] = 0
        else:
            self.__dict__['private'] = 0
        if flags & kRTChatAdmin:
            self.__dict__['admin'] = 1
        else:
            self.__dict__['admin'] = 0
        if flags & kRTChatInterAge:
            self.__dict__['interAge'] = 1
        else:
            self.__dict__['interAge'] = 0
        if flags & kRTChatStatusMsg:
            self.__dict__['status'] = 1
        else:
            self.__dict__['status'] = 0
        if flags & kRTChatNeighborsMsg:
            self.__dict__['neighbors'] = 1
        else:
            self.__dict__['neighbors'] = 0
        self.__dict__['channel'] = (kRTChatChannelMask & flags)/256
    def __setattr__(self,name,value):
        # update the flags attribute
        if name == 'broadcast':
            # only means something if true (false means don't touch)
            if value:
                # reset the private flag
                self.__dict__['flags'] &= kRTChatFlagMask ^ kRTChatPrivate
        elif name == 'private':
            self.__dict__['flags'] &= kRTChatFlagMask ^ kRTChatPrivate
            if value:
                self.__dict__['flags'] |= kRTChatPrivate
                # resets the broadcast flag
                self.__dict__['broadcast'] = 0
            else:
                self.__dict__['broadcast'] = 1
        elif name == 'admin':
            self.__dict__['flags'] &= kRTChatFlagMask ^ kRTChatAdmin
            if value:
                self.__dict__['flags'] |= kRTChatAdmin
        elif name == 'interAge':
            self.__dict__['flags'] &= kRTChatFlagMask ^ kRTChatInterAge
            if value:
                self.__dict__['flags'] |= kRTChatInterAge
        elif name == 'status':
            self.__dict__['flags'] &= kRTChatFlagMask ^ kRTChatStatusMsg
            if value:
                self.__dict__['flags'] |= kRTChatStatusMsg
        elif name == 'neighbors':
            self.__dict__['flags'] &= kRTChatFlagMask ^ kRTChatNeighborsMsg
            if value:
                self.__dict__['flags'] |= kRTChatNeighborsMsg
        elif name == 'channel':
            flagsNoChannel = self.__dict__['flags'] & kRTChatNoChannel
            self.__dict__['flags'] = flagsNoChannel + (value * 256)
        self.__dict__[name] = value
    def __repr__(self):
        str = "ChatFlag: "
        if self.toSelf:
            str += "toSelf "
        if self.broadcast:
            str += "broadcast "
        if self.private:
            str += "private "
        if self.admin:
            str += "admin "
        if self.interAge:
            str += "interAge "
        if self.status:
            str += "status "
        if self.neighbors:
            str += "neighbors "
        str += "channel=%d " % (self.channel)
        str += "flags=%x" % (self.flags)
        return str

class MarkerPlayer:
    def __init__(self,playerID,joined=0):
        self.ID = playerID
        # find player in AgeMember DPL
        dpl = PtGetPlayerListDistanceSorted()
        self.player = None
        self.isUs = 0
        self.team = PtMarkerMsgTeam.kNoTeam
        for plyr in dpl:
            if plyr.getPlayerID() == playerID:
                # found 'em
                self.player = ptPlayer(plyr.getPlayerName(),plyr.getPlayerID())
                break
        if type(self.player) == type(None):
            # maybe its us?
            us = PtGetLocalPlayer()
            if us.getPlayerID() == playerID:
                self.player = us
                self.isUs = 1
        # if its not even us... then just make up one
        if type(self.player) == type(None):
            self.player = ptPlayer("?UNKNOWN?ID:[%d]"%(self.playerID),self.playerID)
        self.isJoined = joined
        self.score = 0
        self.scoreText = ""
    def updateScore(self):
        if type(self.player) != type(None):
            if MarkerGameState == kMGGameOn:
                self.scoreText = "(%d)"%(self.score)
            else:
                self.scoreText = ""

class MarkerGame:
    def __init__(self,masterID,name=None):
        self.masterID = masterID
        self.gameName = name
        dpl = PtGetPlayerListDistanceSorted()
        self.master = None
        self.time = 0
        self.gameTimerOn = 0
        self.timeLeftDPL = None
        self.gameType = PtMarkerMsgGameType.kGameTypeCapture
        self.numberMarkers = 0
        self.markersRemaining = 0
        self.markersRemainingDPL = None
        self.invitedPlayers = []
        self.greenTeamPlayers = []
        self.greenTeamScore = 0
        self.greenTeamDPL = None
        self.redTeamPlayers = []
        self.redTeamScore = 0
        self.redTeamDPL = None
        self.master = MarkerPlayer(masterID)
        # did we find anyone that matches?
        if type(self.master.player) == type(None):
            # then make one up
            self.master.player = ptPlayer("?UNKNOWN?ID:[%d]"%(self.masterID),self.masterID)
        if type(self.gameName) == type(None):
            self.gameName = "%s's MarkerGame" % (self.master.player.getPlayerName())
    def setGame(self,time,type,markers):
        self.setTime(time)
        self.gameType = type
        self.numberMarkers = markers
        self.markersRemaining = markers
    def updateScores(self):
        self.greenTeamScore = 0
        for gplyr in self.greenTeamPlayers:
            self.greenTeamScore += gplyr.score
        self.redTeamScore = 0
        for rplyr in self.redTeamPlayers:
            self.redTeamScore += rplyr.score
        self.markersRemaining = self.numberMarkers - (self.greenTeamScore + self.redTeamScore)
    def resetScores(self):
        self.greenTeamScore = 0
        for gplyr in self.greenTeamPlayers:
            gplyr.score = 0
        self.redTeamScore = 0
        for rplyr in self.redTeamPlayers:
            rplyr.score = 0
        self.markersRemaining = self.numberMarkers
    def setTime(self,time):
        self.time = time
        self.gameTimerOn = 0
    def startTimer(self):
        self.gameTimerOn = 1
        self.endTime = PtGetTime() + self.time
    def timeLeft(self):
        if self.gameTimerOn:
            if PtGetTime() > self.endTime:
                return 0
            return int(self.endTime - PtGetTime())
        else:
            return self.time
    def addPlayerToTeam(self,playerID,teamType):
        if teamType == PtMarkerMsgTeam.kGreenTeam:
            teamplayers = self.greenTeamPlayers
        else:
            teamplayers = self.redTeamPlayers
        found = 0
        for mplayer in teamplayers:
            if mplayer.player.getPlayerID() == playerID:
                mplayer.isJoined = 1
                mplayer.score = 0
                mplayer.team = teamType
                found = 1
                break
        if not found:
            mgplayer = MarkerPlayer(playerID,joined=1)
            mgplayer.team = teamType
            teamplayers.append(mgplayer)
    def addToDPLWorking(self,DPList):
        pass
    def addToDPLPlaying(self,DPList):
        global MarkerGameState
        # create MarkerGame for non-Masters
        DPList.append(CurrentPlayingMarkerGame)
        # add time stuff and remainings
        if MarkerGameState == kMGGameOn:
            gametime = self.timeLeft()
            self.timeLeftDPL = DPLStatusLine( str(PtGetLocalizedString("KI.MarkerGame.TimeRemaining", [str(int(gametime/60)),str(gametime%60)])),AgenBlueDk)
        else:
            self.timeLeftDPL = DPLStatusLine( str(PtGetLocalizedString("KI.MarkerGame.WaitingForStart")),AgenBlueDk)
        if self.gameType == PtMarkerMsgGameType.kGameTypeCapture:
            self.markersRemainingDPL = DPLStatusLine(str(PtGetLocalizedString("KI.MarkerGame.MarkersRemaining", [str(self.markersRemaining)])),AgenBlueDk)
        else:
            self.markersRemainingDPL = DPLStatusLine(str(PtGetLocalizedString("KI.MarkerGame.MarkersUnclaimed", [str(self.markersRemaining)])),AgenBlueDk)
        self.greenTeamDPL = DPLBranchStatusLine(str(PtGetLocalizedString("KI.MarkerGame.GreenTeamScore", [str(self.greenTeamScore)])))
        self.redTeamDPL = DPLBranchStatusLine(str(PtGetLocalizedString("KI.MarkerGame.RedTeamScore", [str(self.redTeamScore)])),closePrev=1)
        DPList.append(self.timeLeftDPL)
        DPList.append(self.markersRemainingDPL)
        DPList.append(self.greenTeamDPL)
        DPList += CurrentPlayingMarkerGame.greenTeamPlayers
        DPList.append(self.redTeamDPL)
        DPList += CurrentPlayingMarkerGame.redTeamPlayers
    def updateGameTime(self):
        global MarkerGameState
        if type(self.timeLeftDPL) != type(None):
            if MarkerGameState == kMGGameOn:
                gametime = self.timeLeft()
                self.timeLeftDPL.updateText(str(PtGetLocalizedString("KI.MarkerGame.TimeRemaining", [str(int(gametime/60)),str(gametime%60)])))
            else:
                self.timeLeftDPL.updateText(str(PtGetLocalizedString("KI.MarkerGame.WaitingForStart")))

class DPLStatusLine:
    def __init__(self,text,color=None):
        self.text = text
        self.color = color
        self.position = -1
    def updateText(self,newText):
        self.text = newText
        self.update()
    def update(self):
        if self.position != -1:
            playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kPlayerList))
            playerlist.setElement(self.position,self.text)
            playerlist.refresh()

class DPLBranchStatusLine(DPLStatusLine):
    def __init__(self,text,closePrev=0):
        DPLStatusLine.__init__(self,text)
        self.closePrev = closePrev

class QuestionNote:
    kNotDefined=0
    kMarkerGameJoin=1
    def __init__(self,type=kNotDefined,title="Question:",msg="",yesBtn=PtGetLocalizedString("KI.YesNoDialog.AcceptButton"),noBtn=PtGetLocalizedString("KI.YesNoDialog.DeclineButton")):
        self.type = type
        self.title = title
        self.message = msg
        self.YesBtnText = yesBtn
        self.NoBtnText = noBtn
    def YesAction(self):
        return
    def NoAction(self):
        return

class MarkerGameJoinQuestion(QuestionNote):
    def __init__(self,gameMasterID,roundLength,gameType,numberMarkers):
        self.game = MarkerGame(gameMasterID)
        self.game.setGame(roundLength,gameType,numberMarkers)
        QuestionNote.__init__(self,type=QuestionNote.kMarkerGameJoin)
        masterName = self.game.master.player.getPlayerName()
        if gameType == PtMarkerMsgGameType.kGameTypeCapture:
            gameName = PtGetLocalizedString("KI.MarkerGame.NameCapture")
            ses = U"s"
            if int(self.game.time/60) == 1:
                ses = U""
            if self.game.numberMarkers == 0:
                instruct = PtGetLocalizedString("KI.MarkerGame.InstructCapNoMarker")
            elif self.game.numberMarkers == 1:
                instruct = PtGetLocalizedString("KI.MarkerGame.InstructCapOneMarker", [str(int(self.game.time/60)),ses])
            else:
                instruct = PtGetLocalizedString("KI.MarkerGame.InstructCapNMarkers", [str(self.game.numberMarkers),str(int(self.game.time/60)),ses])
        elif gameType == PtMarkerMsgGameType.kGameTypeHold:
            gameName = PtGetLocalizedString("KI.MarkerGame.NameHold")
            ses = U"s"
            if int(self.game.time/60) == 1:
                ses = U""
            if self.game.numberMarkers == 0:
                instruct = PtGetLocalizedString("KI.MarkerGame.InstructHoldNoMarker")
            else:
                instruct = PtGetLocalizedString("KI.MarkerGame.InstructHoldNMarkers", [str(self.game.numberMarkers),str(int(self.game.time/60)),ses])
        elif gameType == PtMarkerMsgGameType.kGameTypeQuest:
            gameName = PtGetLocalizedString("KI.MarkerGame.NameQuest")
            instruct = PtGetLocalizedString("KI.MarkerGame.InstructQuest")
        else:
            gameName = PtGetLocalizedString("KI.MarkerGame.NameUnknown")
            instruct = U""
        self.title = PtGetLocalizedString("KI.MarkerGame.QTitle", [masterName])
        self.message = PtGetLocalizedString("KI.MarkerGame.QMessage", [masterName,gameName,instruct])
    def YesAction(self):
        global MarkerJoinRequests
        global MarkerGameState
        global CurrentPlayingMarkerGame
        ptMarkerMgr().joinGame(self.game.masterID,PtMarkerMsgTeam.kNoTeam)
        MarkerGameState = kMGGameCreation
        CurrentPlayingMarkerGame = self.game
        # remove us from the list
        MarkerJoinRequests.remove(self)
    def NoAction(self):
        global MarkerJoinRequests
        MarkerJoinRequests.remove(self)
