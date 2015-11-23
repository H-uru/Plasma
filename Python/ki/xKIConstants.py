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

# Plasma Engine.
from Plasma import *
from PlasmaTypes import *

# Uru modules.
import xEnum

## Number of items to scroll through in content list.
kContentListScrollSize = 5

## Waiting message for marker games.
kMessageWait = xEnum.Enum("createMarker, changeMarkerName")

## Python component for the Jalak Age.
kJalakPythonComponent = "cPythField"

## Keys to ignore (Escape, Enter, Space).
kDefaultKeyIgnoreList = [ 10, 27, 32 ]

# "New Item" alerts (flashing KI and Relto Book).
kAlertTimeDefault = 10.0
kMaxBookAlertTime = 20.0
kAlertKIAlert = 60
kAlertBookAlert = 61

## How many of our chat messages shall we log
kMessageHistoryListMax = 50

## KI light responders.
kListLightResps = ["respKILightOff","respKILightOn" ]

## The name of the KI light scene object.
kKILightObjectName = "RTOmniKILight"

## The easter eggs chat commands.
kEasterEggs = {
               "city" : {"see" : "You see the remnants of a great civilization, ready to be rebuilt. Where are the flying monkeys?",
                         "exits" : "NorthWest and South."},
               "Personal" : {"see" : "You see a small hut... looks deserted.",
                             "exits" : None},
               "Teledahn" : {"see" : "You see 'shrooms everywhere! Big ones, small ones. Are they edible?",
                             "exits" : "East."},
               "Nexus" : {"see" : "You see a jukebox-like machine.",
                          "exits" : None},
               "Garden" : {"see" : "You see bugs. BUGS! I hate bugs.",
                           "exits" : "North and South."},
               "EderTsogal" : {"see" : "You see grass, water and things floating in the air (not feathers).",
                               "exits" : "North. But you'll have to climb or fly to get there."},
               "Dereno" : {"see" : "Ah, Dah-Ree-Toe. You see... Well, if someone would clean those stupid windows you could see a *lot*. Have I been here before? Maybe all pods just look the same.",
                           "exits" : "SouthWest and East. But they are both blocked."},
               "BahroCave" : {"see" : "You see a darkly lit cavern. Strange images on the wall next to you, flickering in the subdued light.Be afraid. Be very afraid!",
                              "exits" : "North, West and East. But they are blocked by a large hole in the floor."},
               "Minkata" : {"see" : "You see sand and dust in all directions. Above you there is a filtered sun or two... or more. Somewhere there is a horse with no name.",
                            "exits" : "East. Nine days away."},
               "Cleft" : {"see" : "You see sand for as far as the eye can see. Gonna need a vehicle of some sort.",
                          "exits" : "... well, I don't know. Maybe you can ask the old man (if he ever stops listening to that music!).",
                          "people" : "an old man. Ok, maybe he's not standing. BTW, wasn't he on M*A*S*H?"},
}

## Constants for Age display names.
class kAges:
    Display = {"Ae'gura" : "D'ni-Ae'gura",
               "AhnonayCathedral" : "Ahnonay Cathedral",
               "AvatarCustomization" : "Avatar Customization",
               "BaronCityOffice" : "D'ni-Ae'gura",
               "city" : "D'ni-Ae'gura",
               "Cleft" : "D'ni-Riltagamin",
               "Descent" : "D'ni-Tiwah",
               "EderDelin" : "Eder Delin",
               "EderTsogal" : "Eder Tsogal",
               "Er'canaCitySilo" : "D'ni-Ashem'en",
               "ErcanaCitySilo" : "D'ni-Ashem'en",
               "GreatTreePub" : "The Watcher's Pub",
               "Great Zero" : "D'ni-Rezeero",
               "GreatZero" : "D'ni-Rezeero",
               "GuildPub-Cartographers" : "The Cartographers' Pub",
               "GuildPub-Greeters" : "The Greeters' Pub",
               "GuildPub-Maintainers" : "The Maintainers' Pub",
               "GuildPub-Messengers" : "The Messengers' Pub",
               "GuildPub-Writers" : "The Writers' Pub",
               "Kirel" : "D'ni-Kirel",
               "K'veer" : "D'ni-K'veer",
               "Kveer" : "D'ni-K'veer",
               "Neighborhood02" : "D'ni-Kirel",
               "Old Spy Room" : "D'ni-Ae'gura",
               "philRelto" : "Phil's Relto",
               "Shaft" : "D'ni-Tiwah",
               "Spy Room" : "D'ni-Ae'gura",
               "spyroom" : "D'ni-Ae'gura",
               "Trebivdil" : "Tre'bivdil",
               "trebivdil" : "Tre'bivdil",
               "vothol" : "Vothol Gallery"}
    Hide = {"BahroCave", "PelletBahroCave", "Pellet Cave", "LiveBahroCave", "LiveBahroCaves"}
    NoInvite = {"Personal", "Nexus", "Cleft", "AvatarCustomization", "city",
                "BahroCave", "LiveBahroCave", "LiveBahroCaves", 
                "BaronCityOffice", "ErcanaCitySilo", "GreatZero", "Shaft",
                "Descent", "Spyroom", "Myst"}
    Replace = {"Ercana" : "Er'cana",
               "Garden" : "Eder Kemo",
               "Garrison" : "Gahreesen",
               "Gira" : "Eder Gira",
               "Personal" : "Relto",
              }
    
## Constants used for the chat.
class kChat:
    MaxChatSize = 2048
    MaxNumChatItems = 50
    StartNumChatItems = 9
    StartOffScreenLine = 0
    ChatBlankLine = "  \n"
    
    # Chat message types.
    SelfMsg = 1
    BroadcastMsg = 2
    PrivateMsg = 3
    AdminBroadcastMsg = 4
    AdminPrivateMsg = 5
    PrivateMsgSelf = 6
    OfferLink = 7
    SystemMessage = 8
    InterAge = 9
    InterAgeSelf = 10
    
    # Chat fading.
    FadeNotActive = 0
    FadeFullDisp = 1
    FadeDoingFade = 2
    FadeStopping = 3
    FadeDone = 4
    FadeTimeMax = 120
    FullTickTime = 1.0
    FadeTickTime = 0.05
    TicksOnFade = 16
    
    # Chat fonts.
    FontSizeList = [ 7, 8, 10, 12, 14 ]
    
## Constants used for the chronicle.
class kChron:
    FontSize = "PlayerKIFontSize"
    FontSizeType = 2
    FadeTime = "PlayerKIFadeTime"
    FadeTimeType = 2
    OnlyPMs = "PlayerKIOnlyPMsBuddies"
    OnlyPMsType = 2
    BuddiesOnRequest = "PlayerKIBuddiesOnRequest"
    BuddiesOnRequestType = 2
    CGZPlaying = "CGZPlaying"
    Party = "PartyAge"

## Color definitions.
class kColors:
    # Basic colors (RGB).
    Clear       = ptColor(0, 0, 0, 0)
    AgenBlueDk  = ptColor(0.65, 0.6353, 0.745, 1.0)
    AgenGreenLt = ptColor(0.8745, 1.0, 0.85, 1.0)
    AgenGreenDk = ptColor(0.65, 0.745, 0.6353, 1.0)
                
    DniYellow   = ptColor(0.851, 0.812, 0.576, 1.0)
    DniYellowLt = ptColor(1.0, 1.0, 0.6, 1.0)
    DniCyan     = ptColor(0.576, 0.867, 0.851, 1.0)
    DniBlue     = ptColor(0.780, 0.706, 0.870, 1.0)
    DniRed      = ptColor(1.0, 0.216, 0.380, 1.0)
    DniGreen    = ptColor(0.698, 0.878, 0.761, 1.0)
    DniGreenDk  = ptColor(0.0, 0.596, 0.211, 1.0)
    DniPurple   = ptColor(0.878, 0.698, 0.819, 1.0)
    DniWhite    = ptColor().white()

    DniShowRed  = ptColor(1.0, 0.851, 0.874, 1.0)
    DniHideBlue = ptColor(0.780, 0.706, 0.870, 0.3)
    
    DniShowBtn     = DniShowRed
    DniGhostBtn    = DniHideBlue
    
    # Chat colors (messages and headers).
    ChatMessage             = DniWhite
    ChatMessageMention      = DniYellowLt
    ChatHeaderBroadcast     = DniBlue
    ChatHeaderPrivate       = DniYellow
    ChatHeaderAdmin         = DniCyan
    ChatHeaderStatus        = DniBlue
    ChatHeaderError         = DniRed
    ChatHeaderNeighbors     = DniPurple
    ChatHeaderBuddies       = DniGreen
    
    DniSelectable           = DniGreen
    DniSelected             = DniWhite
    DniStatic               = DniBlue

## Constants for KI Chat commands.
class kCommands:
    Localized = {PtGetLocalizedString("KI.Commands.ChatClearAll") : "ClearChat",
                 PtGetLocalizedString("KI.Commands.ChatStartLog") : "StartLog",
                 PtGetLocalizedString("KI.Commands.ChatStopLog") : "StopLog",
                 PtGetLocalizedString("KI.Commands.AddBuddy") : "AddBuddy",
                 PtGetLocalizedString("KI.Commands.RemoveBuddy") : "RemoveBuddy",
                 PtGetLocalizedString("KI.Commands.Ignore") : "IgnorePlayer",
                 PtGetLocalizedString("KI.Commands.Unignore") : "UnignorePlayer",
                 PtGetLocalizedString("KI.Commands.DumpLogs") : "DumpLogs",
                 PtGetLocalizedString("KI.Commands.DumpLog") : "DumpLogs",
                 PtGetLocalizedString("KI.Commands.ChangePassword") : "ChangePassword"}
    Jalak = {"/savecolumns" : "SaveColumns",
             "/loadcolumns" : "LoadColumns"}
    Internal = {"/revisitcleft" : "RevisitCleft",
                "/restart" : "RestartGame"}
    EasterEggs = {"/look" : "LookAround",
                  "/get feather" : "GetFeather",
                  "/look in pocket" : "LookForFeathers"}
    Text = {"/go" : "Put one foot in front of the other and eventually you will get there.",
            "/fly" : "You close your eyes, you feel light headed and the ground slips away from your feet... Then you open your eyes and WAKE UP! (Ha, you can only dream about flying.)"}
    Other = {"/party" : "PartyTime",
             "/saveclothing" : "SaveClothing",
             "/loadclothing" : "LoadClothing",
             "/threaten" : "CoopExample"}

## Numeric limits for the KI.
class kLimits:
    JournalTextSize = 2048
    MaxPictures = 15
    MaxNotes = 30
    MaxMarkerFolders = 10
    MaxMarkers = 1000
    MaxRecentPlayerListSize = 50

## Ties to the GUI elements.
class kGUI:
    # Blackbar.
    ExitButtonID = 4
    PlayerBookCBID = 15
    MiniMaximizeRGID = 34
    OptionsMenuButtonID = 200
    microChatButton = 100
    
    RolloverLeftID = 998
    RolloverRightID = 999
    
    # miniKI.
    ChatCaretID = 12
    ChatEditboxID = 5
    ChatDisplayArea = 70
    FolderPlayerList = 30
    PlayerList = 31

    miniToggleBtnID = 1
    miniPutAwayID = 4
    miniTakePicture = 60
    miniMuteAll = 61
    miniPrivateToggle = 62
    miniCreateJournal = 63
    miniDragBar = 50
    miniChatScrollUp = 51
    miniChatScrollDown = 52
    miniPlayerListUp = 53
    miniPlayerListDown = 54

    mini7Indicator1 = 71
    mini7Indicator2 = 72
    mini7Indicator3 = 73
    mini7Indicator4 = 74

    miniMarkerIndicator01 = 601
    miniMarkerIndicatorLast = 625
    miniMarkerColors = {
                        "off"       : 0.0,
                        "redlt"     : 1.5,
                        "red"       : 3.5,
                        "yellowlt"  : 6.0,
                        "yellow"    : 8.5,
                        "purplelt"  : 11.0,
                        "purple"    : 13.5,
                        "greenlt"   : 16.0,
                        "green"     : 18.5,
                        }
    miniGZDrip = 700
    miniGZActive = 701
    miniGZMarkerGameActive = 702
    miniGZMarkerInRange = 703
    miniMGNewMarker = 1010
    miniMGNewGame = 1011
    miniMGInactive = 1012
    
    # Big KI - right panel.
    BKListMode = 1
    BKJournalExpanded = 2
    BKPictureExpanded = 3
    BKPlayerExpanded = 4
    BKVolumeExpanded = 5
    BKAgeOwnerExpanded = 6
    BKKIExpanded = 7
    BKMarkerListExpanded = 8
    BKQuestionNote = 9
    
    # Big KI - controls.
    BKIPutAwayID = 4
    BKToggleMiniID = 14
    BKFolderUpLine = 66
    BKFolderDownLine = 67
    BKLMUpButton = 110
    BKLMDownButton = 111
    BKDisabledPeopleButton = 300
    BKDisabledGearButton = 301
    
    # Big KI - controls (left).
    BKIIncomingBtn = 70
    BKIFolderLineBtn01 = 71
    BKIFolderLineBtnLast = 78
    BKIToPlayerButton = 80
    BKIToIncomingButton = 81
    BKIToFolderButton01 = 82
    BKIToFolderButtonLast = 88
    
    # Big KI - controls (right).
    BKIListModeCreateBtn = 80
    BKIListModeLineBtn01 = 81
    BKIListModeLineBtnLast = 89

    # Big KI - fields.
    BKNeighborhoodAndID = 59
    BKICurAgeNameID = 60
    BKICurTimeID = 61
    BKIGPS1TextID = 62
    BKIGPS2TextID = 63
    BKIGPS3TextID = 64
    BKPlayerListID = 65
    BKRadioModeID = 68
    BKIPlayerLine = 90
    BKIIncomingLine = 91
    BKIFolderLine01 = 92
    BKIFolderLineLast = 98
    BKPlayerName = 200
    BKPlayerID = 201
    BKIPelletDrop = 400
    BKILMTitleCreateLine = 101
    BKILMOffsetLine01 = 110
    BKILMOffsetLineLast = 190
    BKILMIconPictureOffset = 0
    BKILMTitleOffset = 1
    BKILMDateOffset = 2
    BKILMFromOffset = 3
    BKILMIconJournalOffset = 4
    BKILMIconPersonOffset = 5
    
    # Big KI - fields (elongated on mouseover).
    LONGBKIIncomingLine = 591
    LONGBKIFolderLine01 = 592
    LONGBKIFolderLineLast = 598
    
    # Big KI - expanded journal.
    BKIJRNTitleButton = 80
    BKIJRNNoteButton = 81
    BKIJRNDeleteButton = 82
    BKIJRNTitleEdit = 85
    BKIJRNNoteEdit = 86
    BKIJRNAgeName = 90
    BKIJRNDate = 91
    BKIJRNTitle = 92
    BKIJRNNote = 93
    BKIJRNFrom = 94
    BKIJRNSentDate = 95
    
    # Big KI - expanded picture.
    BKIPICTitleButton = 80
    BKIPICDeleteButton = 82
    BKIPICTitleEdit = 85
    BKIPICAgeName= 90
    BKIPICDate = 91
    BKIPICTitle = 92
    BKIPICImage = 93
    BKIPICFrom = 94
    BKIPICSentDate = 95
    BKIImageStartX = 112
    BKIImageStartY = 212
    
    # Big KI - expanded player.
    BKIPLYPlayerIDEditBox = 80
    BKIPLYDeleteButton = 82
    BKIPLYName = 90
    BKIPLYID = 91
    BKIPLYDetail = 92
    
    # Big KI - expanded KI settings.
    BKIKIFontSize=80
    BKIKIFadeTime = 81
    BKIKIOnlyPM = 90
    BKIKIBuddyCheck = 91
    BKIKISettingsText = 570
    BKIKIFontSizeText = 580
    BKIKIFadeTimeText = 581
    BKIKIOnlyPMText = 590
    
    # Big KI - expanded volume.
    BKISoundFXVolSlider = 80
    BKIMusicVolSlider = 81
    BKIVoiceVolSlider = 82
    BKIAmbienceVolSlider = 83
    BKIMicLevelSlider = 84
    BKIGUIVolSlider = 85
    
    # Big KI - expanded Age owner settings.
    BKAgeOwnerTitleTB = 90
    BKAgeOwnerTitleBtn = 91
    BKAgeOwnerTitleEditbox = 92
    BKAgeOwnerStatusTB = 93
    BKAgeOwnerMakePublicTB = 94
    BKAgeOwnerMakePublicBtn = 95
    BKAgeOwnerGUIDTB = 96
    BKAgeOwnerDescription = 99
    BKAgeOwnerDescriptionTitle = 510
    
    # Big KI - expanded marker folder.
    MarkerFolderTitleText = 80
    MarkerFolderTitleBtn = 81
    MarkerFolderTitleEB = 82
    MarkerFolderOwner = 84
    MarkerFolderStatus = 85
    MarkerFolderInvitePlayer = 90
    MarkerFolderEditStartGame = 93
    MarkerFolderPlayEndGame = 94
    MarkerFolderInvitePlayerTB = 100
    MarkerFolderEditStartGameTB = 103
    MarkerFolderPlayEndGameTB = 104

    MarkerFolderGameTimeTitleTB = 199
    MarkerFolderGameTimeTB = 200
    MarkerFolderTimePullDownBtn = 201
    MarkerFolderTimeArrow = 202
    MarkerFolderGameTypeTB = 210
    MarkerFolderTypePullDownBtn = 211
    MarkerFolderTypeArrow = 212
    MarkerFolderDeleteBtn = 150

    MarkerFolderToranIcon = 220
    MarkerFolderHSpanIcon = 221
    MarkerFolderVSpanIcon = 222
    MarkerFolderToranTB = 224
    MarkerFolderHSpanTB = 225
    MarkerFolderVSpanTB = 226

    MarkerFolderMarkListbox = 300
    MarkerFolderMarkerListUpBtn = 301
    MarkerFolderMarkerListDownBtn = 302
    MarkerFolderMarkerTextTB = 310
    MarkerFolderMarkerTextBtn = 311
    MarkerFolderMarkerTextEB = 312

    # Big KI - editables.
    BKEditIDtextbox = 0
    BKEditIDbutton = 1
    BKEditIDeditbox = 2
    BKEditFieldIDs = [ [BKIJRNTitle, BKIJRNTitleButton, BKIJRNTitleEdit],
                       [BKIJRNNote, BKIJRNNoteButton, BKIJRNNoteEdit],
                       [BKIPICTitle, BKIPICTitleButton, BKIPICTitleEdit] ]
    BKEditFieldJRNTitle = 0
    BKEditFieldJRNNote = 1
    BKEditFieldPICTitle = 2
    
    # Yes/No dialog.
    YesNoTextID=12
    YesButtonID = 10
    NoButtonID = 11
    YesButtonTextID = 60
    NoButtonTextID = 61
    YesNoLogoutButtonID = 62
    YesNoLogoutTextID = 63
    YNQuit = 0
    YNDelete = 1
    YNOfferLink = 2
    YNOutside = 3
    YNKIFull = 4
    YNWanaPlay = 5
    YNNoReason = 6
    
    # Question note dialog.
    QNTitle = 100
    QNMessage = 101
    QNAcceptBtn = 110
    QNAcceptText = 120
    QNDeclineBtn = 111
    QNDeclineText = 121

    # Player Book.
    NotOffering = 0
    Offeree = 1
    Offerer = 2
    
    # Marker game creation dialog.
    CreateMarkerGameNameEB = 1000
    MarkerGameType1 = 1001
    MarkerGameType2 = 1002
    MarkerGameType3 = 1003
    CreateMarkerGameSubmitBT = 1004
    CreateMarkerGameCancelBT = 1005
    MarkerGameLabel1 = 1006
    MarkerGameLabel2 = 1007
    MarkerGameLabel3 = 1008
    MarkerGameStates = { MarkerGameType1 : 0, "UNKNOWN" : 1, MarkerGameType2 : 2, MarkerGameType3 : 3 }
    
    # Pellet score upload button.
    PelletScoreButton = 1020
    
    # Scrolling up and down.
    ScrollDown = 0
    ScrollUp = 1

## Constants for marker games.
class kGames:
    MGNotActive = 0
    MGGameCreation = 1
    MGGameOn = 2
    MFOverview = 1
    MFEditing = 2
    MFEditingMarker = 3
    MFPlaying = 4

## Constants for the KI images.
class kImages:
    Directory = U"KIimages"
    FileNameTemplate = U"KIimage"

## Constants for the KI.
class kKI:
    
    # Folders.
    JournalFolder = 0
    PlayersFolder = 1
    ConfigFolder = 2
    
    # State settings.
    ClosedKI = 0
    miniKI = 1
    BigKI = 2
    ToggleKI = 3

## Localization helpers.
class kLoc:
    MarkerFolderPopupMenu = [
        (PtGetLocalizedString("KI.MarkerGame.Popup1Min"), 60),
        (PtGetLocalizedString("KI.MarkerGame.Popup2Min"), 120),
        (PtGetLocalizedString("KI.MarkerGame.Popup5Min"), 300),
        (PtGetLocalizedString("KI.MarkerGame.Popup10Min"), 600),
    ]
    
    OKDialogDict = {
        "" :                                                                        PtGetLocalizedString("KI.Errors.EmptyError"),               #01
        "TERMINATED: Server LogOff. Reason: Logged In Elsewhere" :                  PtGetLocalizedString("KI.Errors.LoggedInElsewhere"),        #02
        "TERMINATED: Server LogOff. Reason: Timed Out" :                            PtGetLocalizedString("KI.Errors.TimedOut"),                 #03
        "TERMINATED: Server LogOff. Reason: Not Authenticated" :                    PtGetLocalizedString("KI.Errors.NotAuthenticated"),         #04
        "TERMINATED: Server LogOff. Reason: Kicked Off" :                           PtGetLocalizedString("KI.Errors.KickedOff"),                #05
        "TERMINATED: Server LogOff. Reason: Unknown Reason" :                       PtGetLocalizedString("KI.Errors.UnknownReason"),            #06
        "TERMINATED: Server LogOff. Reason: UNKNOWN REASON CODE" :                  PtGetLocalizedString("KI.Errors.UnknownReason2"),           #09
        "SERVER SILENCE" :                                                          PtGetLocalizedString("KI.Errors.ServerSilence"),            #10
        "BAD VERSION" :                                                             PtGetLocalizedString("KI.Errors.OldVersion"),               #11
        "Player Disabled" :                                                         PtGetLocalizedString("KI.Errors.PlayerDisabled"),           #12
        "CAN'T FIND AGE" :                                                          PtGetLocalizedString("KI.Errors.CantFindAge"),              #13
        "AUTH RESPONSE FAILED" :                                                    PtGetLocalizedString("KI.Errors.AuthFailed"),               #14
        "AUTH TIMEOUT" :                                                            PtGetLocalizedString("KI.Errors.AuthTimeout"),              #15
        "SDL Desc Problem" :                                                        PtGetLocalizedString("KI.Errors.SDLDescProblem"),           #16
        "Unspecified error" :                                                       PtGetLocalizedString("KI.Errors.UnspecifiedError"),         #17
        "Failed to send msg" :                                                      PtGetLocalizedString("KI.Errors.FailedToSendMsg"),          #18
        "Authentication timed out" :                                                PtGetLocalizedString("KI.Errors.AuthenticationTimedOut"),   #19
        "Peer timed out" :                                                          PtGetLocalizedString("KI.Errors.PeerTimedOut"),             #20
        "Server silence" :                                                          PtGetLocalizedString("KI.Errors.ServerSilence2"),           #21
        "Protocol version mismatch" :                                               PtGetLocalizedString("KI.Errors.ProtocolVersionMismatch"),  #22
        "Auth failed" :                                                             PtGetLocalizedString("KI.Errors.AuthFailed2"),              #23
        "Failed to create player" :                                                 PtGetLocalizedString("KI.Errors.FailedToCreatePlayer"),     #24
        "Invalid error code" :                                                      PtGetLocalizedString("KI.Errors.InvalidErrorCode"),         #25
        "linking banned" :                                                          PtGetLocalizedString("KI.Errors.LinkingBanned"),            #26
        "linking restored" :                                                        PtGetLocalizedString("KI.Errors.LinkingRestored"),          #27
        "silenced" :                                                                PtGetLocalizedString("KI.Errors.Silenced"),                 #28
        "unsilenced" :                                                              PtGetLocalizedString("KI.Errors.Unsilenced"),               #29
        }

## Pellet Score operation types.
class kPellets:
    ScoreNoOp = 0
    ScoreFetchForDisplay = 1
    ScoreFetchMineForUpload = 2
    ScoreFetchUploadDestination = 3
    ScoreCreateUploadDestination = 4
    ScoreTransfer = 5

## A list of timer values.
class kTimers:
    Fade = 1
    BKITODCheck = 2
    AlertHide = 3
    TakeSnapShot = 4
    MarkerGame = 5
    DumpLogs = 6
    LightStop = 7
    JalakBtnDelay = 8
