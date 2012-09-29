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

MaxVersionNumber = 58
MinorVersionNumber = 52

# Plasma engine.
from Plasma import *
from PlasmaGame import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *

import time
import string
import xCensor
import xLinkingBookDefs
import xBookGUIs
import re    # Used for CoD Fix.
import os    # Used for saving pictures locally.
import glob  # Used for saving pictures locally.
import math

import xLocTools
import xEnum
from xMarkerGameManager import *    # Logic for Marker Games.
from xMarkerGameKIDisplay import *  # Support to display user-created marker
                                    # game details within the KI.

# Personal age SDL helper.
from xPsnlVaultSDL import *

# Jalak constants.
from jlakConstants import *

# xKI sub-modules.
import xKIExtChatCommands
from xKIChat import *
from xKIConstants import *
from xKIHelpers import *

# Define the attributes that will be entered in max.
KIBlackbar = ptAttribGUIDialog(1, "The Blackbar dialog")
KIMini = ptAttribGUIDialog(2, "The KIMini dialog")
KIYesNo = ptAttribGUIDialog(3, "The KIYesNo dialog")
BigKI = ptAttribGUIDialog(5, "The BigKI (Mr. BigStuff)")
NewItemAlert = ptAttribGUIDialog(7, "The new item alert dialog")
KIListModeDialog = ptAttribGUIDialog(9, "The list mode dialog")
KIPictureExpanded = ptAttribGUIDialog(10, "The Picture expanded dialog")
KIJournalExpanded = ptAttribGUIDialog(11, "The journal entry expanded dialog")
KIOnAnim = ptAttribAnimation(12, "Turn on/off the KI on animation")
KIOnResp = ptAttribResponder(13, "Turn On responder")
KIOffResp = ptAttribResponder(14, "Turn Off responder")
KIPlayerExpanded = ptAttribGUIDialog(17, "The player expanded dialog")
KIMicroBlackbar = ptAttribGUIDialog(18, "The micro Blackbar dialog")
KIMicro = ptAttribGUIDialog(19, "The micro KI dialog")
KIVolumeExpanded = ptAttribGUIDialog(21, "The volume control dialog")
KIAgeOwnerExpanded = ptAttribGUIDialog(22, "The Age Owner settings dialog")
# Disabled: KIRateIt = ptAttribGUIDialog(23, "The Rate It dialog")
KISettings = ptAttribGUIDialog(24, "The KI settings dialog")
KIMarkerFolderExpanded = ptAttribGUIDialog(27, "The Marker Folder dialog")
KIMarkerFolderPopupMenu = ptAttribGUIPopUpMenu(28, "The MarkerFolder Time Popup Menu")
KIQuestionNote = ptAttribGUIDialog(29, "The Question Note dialog")
# Disabled: KIMarkerTypePopupMenu = ptAttribGUIPopUpMenu(30, "The MarkerFolder Type Popup Menu")
KICreateMarkerGameGUI = ptAttribGUIDialog(31, "The Marker Game Creation GUI")
KIMarkerGameGUIOpen = ptAttribResponder(32, "Marker Game GUI Open Responder")
KIMarkerGameGUIClose = ptAttribResponder(33, "Marker Game GUI Close Responder")
KIJalakMiniIconOn = ptAttribResponder(34, "Jalak KIMini icon 'on/off' resp", ['on', 'off'])
KIJalakGUIDialog = ptAttribGUIDialog(35, "The Jalak GUI dialog")
KIJalakGUIOpen = ptAttribResponder(36, "Jalak GUI 'open' resp")
KIJalakGUIClose = ptAttribResponder(37, "Jalak GUI 'close' resp")
KIJalakBtnLights = ptAttribResponder(38, "Jalak GUI btn lights resp", statelist=JalakBtnStates, netForce=0)


## The master class handling all of Uru's GUI.
# This includes the KI itself, the blackbar, the Jalak GUI...
class xKI(ptModifier):

    ## Initialize the KI.
    # Called only once during Uru execution when the game is started. At this
    # stage, Plasma is not yet initialized.
    def __init__(self):

        # Set up the KI.
        ptModifier.__init__(self)
        self.id = 199
        self.version = MaxVersionNumber
        self.folderOfDevices = DeviceFolder(PtGetLocalizedString("KI.Folders.Devices"))
        PtDebugPrint("__xKI: Max version %d - minor version %d.a" % (MaxVersionNumber, MinorVersionNumber))

        # Prepare the GUI's default state.
        self.KIGUIInitialized = False
        self.KIDisabled = False
        self.KIHardDisabled = False
        self.isPlayingLookingAtKIMode = False
        self.miniKIWasUp = False
        self.sawTheKIAtLeastOnce = False
        self.miniKIFirstTimeShow = True

        # Set the default KI levels.
        self.censorLevel = 0
        self.gKIMarkerLevel = 0
        self.KILevel = kMicroKI  # Assume the KI is at the lowest level.
        self.MGKILevel = 0

        # Player state.
        self.onlyGetPMsFromBuddies = False
        self.onlyAllowBuddiesOnRequest = False
        self.takingAPicture = False
        self.waitingForAnimation = False

        # Messages for user created Marker Game waiting screen.
        self.pendingMGaction = None
        self.pendingMGmessage = None

        # Transparency settings.
        self.originalForeAlpha = 1.0
        self.originalSelectAlpha = 1.0
        self.originalminiKICenter = None
        self.lastminiKICenter = None

        #########
        # BigKI #
        #########
        self.previousTime = "20:20"
        self.timeBlinker = 1
        self.gFeather = 0

        self.BKPlayerList = []
        self.BKPlayerSelected = None
        self.previouslySelectedPlayer = None

        self.BKJournalFolderDict = {}
        self.BKJournalListOrder = []
        self.BKJournalFolderSelected = 0
        self.BKJournalFolderTopLine = 0
        self.BKPlayerFolderDict = {}
        self.BKPlayerListOrder = []
        self.BKPlayerFolderSelected = 0
        self.BKPlayerFolderTopLine = 0
        self.BKConfigFolderDict = {}
        self.BKConfigListOrder = [PtGetLocalizedString("KI.Config.Settings")]
        self.BKConfigDefaultListOrder = [PtGetLocalizedString("KI.Config.Settings")]
        self.BKConfigFolderSelected = 0
        self.BKConfigFolderTopLine = 0
        self.BKFolderLineDict = self.BKJournalFolderDict
        self.BKFolderListOrder = self.BKJournalListOrder
        self.BKFolderSelected = self.BKJournalFolderSelected
        self.BKFolderTopLine = self.BKJournalFolderTopLine
        self.BKFolderSelectChanged = False
        self.BKIncomingFolder = None
        self.BKNewItemsInInbox = 0

        self.BKCurrentContent = None
        self.BKContentList = []
        self.BKContentListTopLine = 0

        self.BKInEditMode = False
        self.BKEditContent = None
        self.BKEditField = -1
        self.BKGettingPlayerID = False

        self.BKRightSideMode = kGUI.BKListMode

        self.BKAgeOwnerEditDescription = False

        # New item alert.
        self.alertTimerActive = False
        self.alertTimeToUse = kAlertTimeDefault

        # Yes/No dialog globals.
        self.YNWhatReason = kGUI.YNQuit
        self.YNOutsideSender = None

        # Player book globals.
        self.bookOfferer = None
        self.offerLinkFromWho = None
        self.offeredBookMode = 0
        self.psnlOfferedAgeInfo = None

        # The ptBook Yeesha book object.
        self.isEntireYeeshaBookEnabled = True
        self.isYeeshaBookEnabled = True
        self.yeeshaBook = None

        # KI light state.
        self.lightOn = False
        self.lightStop = 0

        # Pellet score operations.
        self._scoreOpCur = kPellets.ScoreNoOp
        self.scoreOps = []

        # KI usage values.
        self.numberOfPictures = 0
        self.numberOfNotes = 0
        self.numberOfMarkerFolders = 0
        self.numberOfMarkers = 0

        # Marker Games state.
        self.currentPlayingMarkerGame = None
        self.markerGameState = kGames.MGNotActive
        self.markerGameTimeID = 0
        self.markerJoinRequests = []
        self.MFdialogMode = kGames.MFOverview

        # New Marker Game dialog globals.
        self.markerGameDefaultColor = ""
        self.markerGameSelectedColor = ""
        self.selectedMGType = 0

        # GZ missions state.
        self.gGZPlaying = 0
        self.gGZMarkerInRange = 0
        self.gGZMarkerInRangeRepy = None
        self.gMarkerToGetColor = 'off'
        self.gMarkerGottenColor = 'off'
        self.gMarkerToGetNumber = 0
        self.gMarkerGottenNumber = 0

        # Jalak GUI globals.
        self.jalakGUIButtons = []
        self.jalakGUIState = False
        self.jalakScript = None

        # Phased KI.
        self.phasedKICreateNotes = True
        self.phasedKICreateImages = True
        self.phasedKIShareYeeshaBook = True
        self.phasedKIInterAgeChat = True
        self.phasedKINeighborsInDPL = True
        self.phasedKIBuddies = True
        self.phasedKIPlayMarkerGame = True    # Disabled until Marker Games are completed.
        self.phasedKICreateMarkerGame = True  # Disabled until Marker Games are completed.
        self.phasedKISendNotes = True
        self.phasedKISendImages = True
        self.phasedKISendMarkerGame = True    # Disabled until Marker Games are completed.
        self.phasedKIShowMarkerGame = True    # Disabled until Marker Games are completed.

        # Miscellaneous.
        self.imagerMap = {}
        self.pelletImager = ""

        ## Auto-completion manager.
        self.autocompleteState = AutocompleteState()
        
        ## The chatting manager.
        self.chatMgr = xKIChat(BigKI, KIBlackbar, KIMicro, KIMini, self.GetAgeName, (self.StartFadeTimer, self.KillFadeTimer, self.FadeCompletely))

    # Unloads any loaded dialogs upon exit.
    def __del__(self):

        PtUnloadDialog("KIMicroBlackBar")
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

        if PtGetAgeName() == "Jalak":
            PtDebugPrint("xKI: Unloading Jalak GUI dialog.", level=kWarningLevel)
            KIJalakMiniIconOn.run(self.key, state="off", netPropagate=0, fastforward=1)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).hide()
            PtUnloadDialog("jalakControlPanel")

    ####################
    # Plasma Functions #
    ####################

    ## Called by Plasma on receipt of the first PythonFileMod message.
    # Preloads all the GUI dialogs.
    def OnInit(self):

        # Preload all the dialogs.
        PtLoadDialog("KIBlackBar", self.key)
        PtLoadDialog("KIMicroBlackBar", self.key)
        PtLoadDialog("KIMicro", self.key)
        PtLoadDialog("KIMini", self.key)
        PtLoadDialog("KIMain", self.key)
        PtLoadDialog("KIListMode", self.key)
        PtLoadDialog("KIJournalExpanded", self.key)
        PtLoadDialog("KIPictureExpanded", self.key)
        PtLoadDialog("KIPlayerExpanded", self.key)
        PtLoadDialog("KIAgeOwnerSettings", self.key)
        PtLoadDialog("KISettings", self.key)
        PtLoadDialog("KIMarkerFolder", self.key)
        PtLoadDialog("KIMarkerTimeMenu", self.key)
        PtLoadDialog("KIQuestionNote", self.key)
        PtLoadDialog("KIMarkerTypeMenu", self.key)
        PtLoadDialog("KIYesNo", self.key)
        PtLoadDialog("KINewItemAlert", self.key)
        PtLoadDialog("OptionsMenuGUI")
        PtLoadDialog("IntroBahroBgGUI")
        PtLoadDialog("KIHelp")
        PtLoadDialog("KIHelpMenu")
        PtLoadDialog("KeyMapDialog")
        PtLoadDialog("GameSettingsDialog")
        PtLoadDialog("CalibrationGUI")
        PtLoadDialog("TrailerPreviewGUI")
        PtLoadDialog("KeyMap2Dialog")
        PtLoadDialog("AdvancedGameSettingsDialog")
        PtLoadDialog("OptionsHelpGUI")
        PtLoadDialog("bkNotebook")
        PtLoadDialog("bkBahroRockBook")
        PtLoadDialog("YeeshaPageGUI")
        PtLoadDialog("KIMiniMarkers", self.key)

        self.markerGameManager = None
        self.markerGameDisplay = None

    ## Called by Plasma on receipt of the first plEvalMsg.
    # Loads all the dialogs for the first update.
    def OnFirstUpdate(self):

        # Create the dnicoordinate keeper.
        self.dniCoords = ptDniCoordinates()

        # To start with, use randized numbers instead of the real thing.
        self.chatMgr.logFile = ptStatusLog()

        xBookGUIs.LoadAllBookGUIs()

        logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutTextID))
        logoutText.hide()
        logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutButtonID))
        logoutButton.hide()

    ## Called by Plasma when the player updates his account.
    # This includes switching avatars and changing passwords. Because the KI
    # gets started at initial load time, the KI needs to be re-initialized once
    # the playerID has been updated, triggering this function.
    def OnAccountUpdate(self, updateType, result, playerID):

        if updateType == PtAccountUpdateType.kActivePlayer and result == 0 and playerID != 0:
            PtDebugPrint("xKI.OnAccountUpdate(): Active player set. Clear to re-init KI GUI.", level=kDebugDumpLevel)
            self.KIGUIInitialized = False
        elif updateType == PtAccountUpdateType.kChangePassword:
            if result == 0:
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Messages.PasswordChangeSuccess"))
            else:
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Messages.PasswordChangeFailure"))

    ## Called by Plasma when the Age state has been fully received.
    # This function re-initializes Marker Games, puts away the KI and performs
    # various other preparations.
    def OnServerInitComplete(self):

        if self.markerGameDisplay is not None:
            self.markerGameDisplay = None

        # Update the marker game manager.
        if self.markerGameManager is None:
            # Game is initialized on account update (so ignore if player is not selected).
            PtDebugPrint("xKI.OnServerInitComplete(): ERROR: Could not find marker manger, re-creating a new one.")
            self.markerGameManager = MarkerGameManager(self)
        else:
            # Loading new age, re-load marker game manager.
            ageName = PtGetAgeInfo().getAgeFilename()
            if ageName.lower() != "startup":
                PtDebugPrint("xKI.OnServerInitComplete(): DEBUG: Reloading Marker Game Manager.")
                self.markerGameManager = MarkerGameManager(self)

        # Force any open KIs to close.
        self.IminiPutAwayKI()

        self.CheckKILight()

        ageName = PtGetAgeName()
        PtDebugPrint("xKI.OnServerInitComplete(): Age = ", ageName, level=kDebugDumpLevel)

        # Set up Jalak GUI.
        if ageName == "Jalak":
            PtDebugPrint("xKI.OnServerInitComplete(): Loading Jalak GUI dialog.", level=kWarningLevel)
            PtLoadDialog("jalakControlPanel", self.key)
            KIJalakMiniIconOn.run(self.key, state="on", netPropagate=0)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).show()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
            self.IAlertKIStart()
        else:
            KIJalakMiniIconOn.run(self.key, state="off", netPropagate=0, fastforward=1)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).hide()

    ## Called by Plasma when the avatar is linked out of an Age.
    # Depending on the Age the avatar was linking out, the Jalak GUI will be
    # disabled and the KI light turned off.
    def BeginAgeUnLoad(self, avObj):

        ageName = PtGetAgeName()
        if ageName == "Descent":
            return
        elif ageName == "Jalak":
            if self.jalakGUIState:
                self.JalakGUIToggle(1)

        if not self.lightOn:
            return

        local = 0
        try:
            local = PtGetLocalAvatar()
        except:
            PtDebugPrint("xKI.BeginAgeUnLoad(): Failed to get local avatar.")
            return
        if local == avObj:
            PtDebugPrint("xKI.BeginAgeUnLoad(): Avatar page out.", level=kDebugDumpLevel)
            curTime = PtGetDniTime()
            timeRemaining = (self.lightStop - curTime)
            if timeRemaining > 0:
                self.DoKILight(0, 1, timeRemaining)
                LocalAvatar = PtGetLocalAvatar()
                avatarKey = LocalAvatar.getKey()
                PtSetLightAnimStart(avatarKey, KILightObjectName, False)

    ## Get any leftover, unwanted keystrokes.
    def OnDefaultKeyCaught(self, ch, isDown, isRepeat, isShift, isCtrl, keycode):

        if ord(ch) != 0:
            if not ord(ch) in kDefaultKeyIgnoreList:
                if isDown and not isCtrl and not isRepeat:
                    if not self.KIDisabled and not PtIsEnterChatModeKeyBound():
                        self.chatMgr.ToggleChatMode(1, firstChar=ch)

    ## Called by Plasma on receipt of a plNotifyMsg.
    # This big function deals with the various responses sent upon a triggered
    # event, such as a book being offered.
    def OnNotify(self, state, id, events):

        PtDebugPrint("xKI.OnNotify(): Notify state = %f, id = %d" % (state, id), level=kDebugDumpLevel)
        # Is it a notification from the scene input interface or PlayerBook?
        for event in events:
            if event[0] == kOfferLinkingBook:
                if self.KILevel == kMicroKI:
                    plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                else:
                    plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                # Make sure we get the events back if someone else used the PlayerBook.
                if event[2] == -999:
                    # If waiting for offeree to accept.
                    if self.offeredBookMode == kOfferee:
                        # Then take the offered PlayerBook.
                        YeeshaBook.hide()
                        PtToggleAvatarClickability(True)
                        plybkCB.setChecked(0)
                    # Else, they were too late.
                    self.offeredBookMode = kGUI.NotOffering
                    self.bookOfferer = None
                    PtDebugPrint("xKI.OnNotify(): Offerer is rescinding the book offer.", level=kDebugDumpLevel)
                    PtToggleAvatarClickability(True)
                    return
                # The book is being offered by someone else.
                elif event[2] == 999:
                    self.bookOfferer = event[1]
                    avID = PtGetClientIDFromAvatarKey(self.bookOfferer.getKey())
                    if ptVault().getIgnoreListFolder().playerlistHasPlayer(avID):
                        self.offeredBookMode = kGUI.NotOffering
                        PtNotifyOffererLinkRejected(avID)
                        self.bookOfferer = None
                        PtToggleAvatarClickability(True)
                        return
                    else:
                        OfferedBookMode = kGUI.Offeree
                        PtDebugPrint("xKI.OnNotify(): Offered book by ", self.bookOfferer.getName(), level=kDebugDumpLevel)
                        self.IShowYeeshaBook()
                        PtToggleAvatarClickability(False)
                        return

            # Is it from the YeeshaBook?
            elif event[0] == PtEventType.kBook:
                if self.KILevel == kMicroKI:
                    plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                else:
                    plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                if event[1] == PtBookEventTypes.kNotifyImageLink:
                    if event[2] == xLinkingBookDefs.kYeeshaBookShareID:
                        if self.isYeeshaBookEnabled:
                            # Rescind any previous offers.
                            PtClearOfferBookMode()
                            if self.offeredBookMode == kGUI.NotOffering:
                                # Take the PlayerBook.
                                YeeshaBook.hide()
                                PtToggleAvatarClickability(True)
                                plybkCB.setChecked(0)
                                # Prepare to choose who to offer the link to.
                                PtSetOfferBookMode(self.key, "Personal", "Relto")
                    elif event[2] == xLinkingBookDefs.kYeeshaBookLinkID:
                        if self.isYeeshaBookEnabled:
                            YeeshaBook.hide()
                            plybkCB.setChecked(0)
                            if self.offeredBookMode == kGUI.Offeree:
                                selfofferedBookMode = kGUI.NotOffering
                                avID = PtGetClientIDFromAvatarKey(self.bookOfferer.getKey())
                                PtNotifyOffererLinkAccepted(avID)
                                PtNotifyOffererLinkCompleted(avID)
                                self.bookOfferer = None
                            else:
                                # Is the player on a ladder or something?
                                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                                if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                                    linkmgr = ptNetLinkingMgr()
                                    linkmgr.linkToMyPersonalAgeWithYeeshaBook()
                                    PtToggleAvatarClickability(True)
                    elif event[2] >= xLinkingBookDefs.kYeeshaPageStartID:
                        whatpage = event[2] - xLinkingBookDefs.kYeeshaPageStartID
                        sdlvar = xLinkingBookDefs.xYeeshaPages[whatpage][0]
                        self.IToggleYeeshaPageSDL(sdlvar, 1)
                elif event[1] == PtBookEventTypes.kNotifyShow:
                    pass
                elif event[1] == PtBookEventTypes.kNotifyHide:
                    PtToggleAvatarClickability(True)
                    plybkCB.setChecked(0)
                    if OfferedBookMode == kGUI.Offeree:
                        OfferedBookMode = kGUI.NotOffering
                        avID = PtGetClientIDFromAvatarKey(self.bookOfferer.getKey())
                        PtNotifyOffererLinkRejected(avID)
                        self.bookOfferer = None
                elif event[1] == PtBookEventTypes.kNotifyNextPage:
                    pass
                elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                    pass
                elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                    if event[2] >= xLinkingBookDefs.kYeeshaPageStartID:
                        whatpage = event[2] - xLinkingBookDefs.kYeeshaPageStartID
                        sdlvar = xLinkingBookDefs.xYeeshaPages[whatpage][0]
                        self.IToggleYeeshaPageSDL(sdlvar, 0)
                return
        if state:
            # Is it one of the responders that are displaying the BigKI?
            if id == KIOnResp.id:
                self.IBigKIShowMode()
                self.waitingForAnimation = False
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                toggleCB.enable()
            elif id == KIOffResp.id:
                BigKI.dialog.hide()
                self.waitingForAnimation = False
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                toggleCB.enable()
        if id == KIMarkerGameGUIClose.id:
            PtHideDialog("KIMiniMarkers")
        if id == KIJalakGUIClose.id:
            PtHideDialog("jalakControlPanel")
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
        elif id == KIJalakGUIOpen.id:
            KIJalakGUIDialog.dialog.enable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
        elif id == KIJalakBtnLights.id:
            btnID = int(KIJalakBtnLights.getState())
            self.SendNote('%d' % (btnID))
            PtAtTimeCallback(self.key, kJalakBtnDelaySeconds, kTimers.JalakBtnDelay)

    ## Called by Plasma on ???.
    # This is used to delay the display of objects until something is paged in.
    def OnPageLoad(self, what, room):

        if not self.KIGUIInitialized:
            self.KIGUIInitialized = False
            self.ISetupKI()

        # When we upload the page, then the device we have must be gone.
        if what == kUnloaded:
            self.folderOfDevices.removeAll()
            # Remove any selected player.
            if self.KILevel == kNormalKI:
                self.BKPlayerSelected = None
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                sendToField.setString(" ")
            if self.gGZMarkerInRange:
                self.gGZMarkerInRange = 0
                self.gGZMarkerInRangeRepy = None
                self.IRefreshMiniKIMarkerDisplay()
                NewItemAlert.dialog.hide()
                kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                kialert.hide()
            if self.pelletImager != "":
                self.pelletImager = ""
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).hide()

    ## Called by Plasma when a GUI event occurs.
    # Delegates the appropriate response to the correct handler.
    def OnGUINotify(self, id, control, event):

        PtDebugPrint("xKI.OnGUINotify(): id = %d, event = %d control = " % (id, event), control, level=kDebugDumpLevel)
        if id == KIBlackbar.id:
            self.ProcessNotifyBlackbar(control, event)
        elif id == KIMicroBlackbar.id:
            self.ProcessNotifyMicroBlackbar(control, event)
        elif id == KIMicro.id:
            self.ProcessNotifyMicro(control, event)
        elif id == KIMini.id:
            self.ProcessNotifyMini(control, event)
        elif id == BigKI.id:
            self.ProcessNotifyBigKI(control, event)
        elif id == KIListModeDialog.id:
            self.ProcessNotifyListMode(control, event)
        elif id == KIPictureExpanded.id:
            self.ProcessNotifyPictureExpanded(control, event)
        elif id == KIJournalExpanded.id:
            self.ProcessNotifyJournalExpanded(control, event)
        elif id == KIPlayerExpanded.id:
            self.ProcessNotifyPlayerExpanded(control, event)
        elif id == KISettings.id:
            self.ProcessNotifySettingsExpanded(control, event)
        elif id == KIVolumeExpanded.id:
            self.ProcessNotifyVolumeExpanded(control, event)
        elif id == KIAgeOwnerExpanded.id:
            self.ProcessNotifyAgeOwnerExpanded(control, event)
        elif id == KIYesNo.id:
            self.ProcessNotifyYesNo(control, event)
        elif id == NewItemAlert.id:
            self.ProcessNotifyNewItemAlert(control, event)
        elif id == KICreateMarkerGameGUI.id:
            self.ProcessNotifyCreateMarkerGameGUI(control, event)
        elif id == KIMarkerFolderExpanded.id:
            self.ProcessNotifyMarkerFolderExpanded(control, event)
        elif id == KIMarkerFolderPopupMenu.id:
            self.ProcessNotifyMarkerFolderPopupMenu(control, event)
        elif id == KIQuestionNote.id:
            self.ProcessNotifyQuestionNote(control, event)
        elif id == KIJalakGUIDialog.id:
            self.ProcessNotifyJalakGUI(control, event)

    ## Called by Plasma on receipt of a KI message.
    # These are messages which instruct xKI to perform a certain command like
    # enabling or disabling the KI, showing a Yes/No dialog, etc.. These
    # messages can also be sent from the Python to Plasma and back.
    def OnKIMsg(self, command, value):

        PtDebugPrint("xKI.OnKIMsg(): command = %d value = " % (command), value, level=kDebugDumpLevel)
        if command == kEnterChatMode and not self.KIDisabled:
            self.chatMgr.ToggleChatMode(1)
        elif command == kSetChatFadeDelay:
            self.chatMgr.ticksOnFull = value
        elif command == kSetTextChatAdminMode:
            self.chatMgr.isAdmin = value
        elif command == kDisableKIandBB:
            self.KIDisabled = True
            self.chatMgr.KIDisabled = True
            self.KIHardDisabled = True
            if self.KILevel == kMicroKI:
                KIMicroBlackbar.dialog.hide()
                if KIMicro.dialog.isEnabled():
                    self.miniKIWasUp = True
                    KIMicro.dialog.hide()
                else:
                    self.miniKIWasUp = False
            else:
                KIBlackbar.dialog.hide()
                if KIMini.dialog.isEnabled():
                    self.miniKIWasUp = True
                    KIMini.dialog.hide()
                else:
                    self.miniKIWasUp = False
                if not self.waitingForAnimation:
                    # Hide all dialogs on the right side of the BigKI and hide it.
                    KIListModeDialog.dialog.hide()
                    KIPictureExpanded.dialog.hide()
                    KIJournalExpanded.dialog.hide()
                    KIPlayerExpanded.dialog.hide()
                    BigKI.dialog.hide()
                    KIOnAnim.animation.skipToTime(1.5)
            # If an outsider has a Yes/No up, tell them No.
            if self.YNWhatReason == kGUI.YNOutside:
                if self.YNOutsideSender is not None:
                    note = ptNotify(self.key)
                    note.clearReceivers()
                    note.addReceiver(self.YNOutsideSender)
                    note.netPropagate(0)
                    note.netForce(0)
                    note.setActivate(0)
                    note.addVarNumber("YesNo", 0)
                    note.send()
                self.YNOutsideSender = None
            # Hide the Yeesha Book.
            if self.yeeshaBook:
                self.yeeshaBook.hide()
            PtToggleAvatarClickability(True)
            plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
            plybkCB.setChecked(0)
            self.YNWhatReason = kGUI.YNQuit
            KIYesNo.dialog.hide()
        elif command == kEnableKIandBB:
            self.KIDisabled = False
            self.chatMgr.KIDisabled = False
            self.KIHardDisabled = False
            if self.KILevel == kMicroKI:
                KIMicroBlackbar.dialog.show()
                if self.miniKIWasUp:
                    KIMicro.dialog.show()
            else:
                KIBlackbar.dialog.show()
                if self.miniKIWasUp:
                    self.chatMgr.ClearBBMini(0)
                    KIMini.dialog.show()
        elif command == kTempDisableKIandBB:
            self.KIDisabled = True
            self.chatMgr.KIDisabled = True
            if self.KILevel == kMicroKI:
                KIMicroBlackbar.dialog.hide()
            else:
                KIBlackbar.dialog.hide()
            if self.yeeshaBook:
                self.yeeshaBook.hide()
            PtToggleAvatarClickability(True)
            plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
            plybkCB.setChecked(0)
        elif command == kTempEnableKIandBB and not self.KIHardDisabled:
            self.KIDisabled = False
            self.chatMgr.KIDisabled = False
            if self.KILevel == kMicroKI:
                KIMicroBlackbar.dialog.showNoReset()
            else:
                KIBlackbar.dialog.showNoReset()
        elif command == kYesNoDialog:
            self.YNWhatReason = kGUI.YNOutside
            self.YNOutsideSender = value[1]
            yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
            yesText.setStringW(value[0])
            self.ILocalizeDialog(1)
            KIYesNo.dialog.show()
        elif command == kAddPlayerDevice:
            if "<p>" in value:
                self.pelletImager = value.rstrip("<p>")
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).show()
                PtDebugPrint("Pellet Imager:", self.pelletImager, level=kDebugDumpLevel)
                return
            try:
                self.folderOfDevices.index(Device(value))
            except ValueError:
                self.folderOfDevices.append(Device(value))
                self.IRefreshPlayerList()
                self.IRefreshPlayerListDisplay()
        elif command == kRemovePlayerDevice:
            if "<p>" in value:
                self.pelletImager = ""
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).hide()
                return
            try:
                FolderOfDevices.remove(Device(value))
            except ValueError:
                pass
            self.IRefreshPlayerList()
            self.IRefreshPlayerListDisplay()
        elif command == kUpgradeKILevel:
            if value >= kLowestKILevel and value <= kHighestKILevel:
                if value > self.KILevel:
                    PtDebugPrint("xKI.OnKIMsg(): Upgrading from KI level %d to new KI level of %d." % (self.KILevel, value), level=kWarningLevel)
                    self.IRemoveKILevel(self.KILevel, upgrading=1)
                    self.KILevel = value
                    self.chatMgr.KILevel = self.KILevel
                    self.IUpdateKILevelChronicle()
                    self.IWearKILevel(self.KILevel)
                else:
                    PtDebugPrint("xKI.OnKIMsg(): Ignoring, trying to upgrade from KI level %d to new KI level of %d." % (self.KILevel, value), level=kWarningLevel)
                    self.IMakeSureWeWereKILevel()
            else:
                PtDebugPrint("xKI.OnKIMsg(): Invalid KI level %d." % (value), level=kErrorLevel)
        elif command == kDowngradeKILevel:
            if value == self.KILevel:
                PtDebugPrint("xKI.OnKIMsg(): Remove KI level of %d." % (value), level=kWarningLevel)
                if value == kNormalKI:
                    self.IRemoveKILevel(kNormalKI)
                    self.KILevel = kMicroKI
                    self.chatMgr.KILevel = self.KILevel
                    self.IUpdateKILevelChronicle()
                    self.IWearKILevel(self.KILevel)
                else:
                    PtDebugPrint("xKI.OnKIMsg(): Ignoring, can't remove to any lower than %d." % (value), level=kWarningLevel)
            else:
                PtDebugPrint("xKI.OnKIMsg(): Ignoring, trying to remove KI Level %d, but currently at %d." % (value, self.KILevel), level=kWarningLevel)
        elif command == kSetPrivateChatChannel:
            self.chatMgr.privateChatChannel = value
        elif command == kUnsetPrivateChatChannel:
            self.chatMgr.privateChatChannel = 0
        elif command == kStartBookAlert:
            self.IAlertBookStart()
        elif command == kStartKIAlert:
            self.IAlertKIStart()
        elif command == kUpdatePelletScore:
            self.IUpdatePelletScore()
            self.IAlertKIStart()
        elif command == kMiniBigKIToggle:
            self.IminiToggleKISize()
        elif command == kKIPutAway:
            self.IminiPutAwayKI()
        elif command == kChatAreaPageUp:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kPageUp)
        elif command == kChatAreaPageDown:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kPageDown)
        elif command == kChatAreaGoToBegin:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kBufferStart)
        elif command == kChatAreaGoToEnd:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kBufferEnd)
        elif command == kKITakePicture:
            self.IminiTakePicture()
        elif command == kKICreateJournalNote:
            self.IminiCreateJournal()
        elif command == kKIToggleFade:
            if self.chatMgr.IsFaded():
                self.chatMgr.KillFadeTimer()
                self.chatMgr.StartFadeTimer()
            else:
                self.chatMgr.FadeCompletely()
        elif command == kKIToggleFadeEnable:
            self.KillFadeTimer()
            if self.chatMgr.fadeEnableFlag:
                self.chatMgr.fadeEnableFlag = 0
            else:
                self.chatMgr.fadeEnableFlag = 1
            self.StartFadeTimer()
        elif command == kKIChatStatusMsg:
            self.chatMgr.DisplayStatusMessage(value, 1)
        elif command == kKILocalChatStatusMsg:
            self.chatMgr.DisplayStatusMessage(value, 0)
        elif command == kKILocalChatErrorMsg:
            self.chatMgr.AddChatLine(None, value, kChat.SystemMessage)
        elif command == kKIUpSizeFont:
            self.IChangeFontSize(1)
        elif command == kKIDownSizeFont:
            self.IChangeFontSize(-1)
        elif command == kKIOpenYeehsaBook:
            nm = ptNetLinkingMgr()
            if self.KILevel >= kMicroKI and not self.KIDisabled and not self.waitingForAnimation and nm.isEnabled():
                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                    self.IShowYeeshaBook()
                    if self.KILevel == kMicroKI:
                        plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                    else:
                        plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                    plybkCB.setChecked(1)
        elif command == kKIOpenKI:
            if not self.waitingForAnimation:
                if not KIMini.dialog.isEnabled():
                    self.IminiPutAwayKI(1)
                elif not BigKI.dialog.isEnabled():
                    if not self.chatMgr.fadeEnableFlag and self.chatMgr.IsFaded():
                        self.IminiToggleKISize()
                    elif self.chatMgr.fadeEnableFlag and self.chatMgr.IsFaded():
                        self.KillFadeTimer()
                        self.StartFadeTimer()
                    else:
                        self.IminiToggleKISize()
                else:
                    self.IminiPutAwayKI()
        elif command == kKIPhasedAllOn:
            self.phasedKICreateNotes = True
            self.phasedKICreateImages = True
            self.phasedKIShareYeeshaBook = True
            self.phasedKIInterAgeChat = True
            self.phasedKINeighborsInDPL = True
            self.phasedKIBuddies = True
            self.phasedKIPlayMarkerGame = True
            self.phasedKICreateMarkerGame = True
            self.phasedKISendNotes = True
            self.phasedKISendImages = True
            self.phasedKISendMarkerGame = True
            self.phasedKIShowMarkerGame = True
        elif command == kKIPhasedAllOff:
            self.phasedKICreateNotes = False
            self.phasedKICreateImages = False
            self.phasedKIShareYeeshaBook = False
            self.phasedKIInterAgeChat = False
            self.phasedKINeighborsInDPL = False
            selfp.hasedKIBuddies = False
            self.phasedKIPlayMarkerGame = False
            self.phasedKICreateMarkerGame = False
            self.phasedKISendNotes = False
            self.phasedKISendImages = False
            self.phasedKISendMarkerGame = False
            self.phasedKIShowMarkerGame = False
        elif command == kKIOKDialog or command == kKIOKDialogNoQuit:
            reasonField = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
            try:
                localized = kLoc.OKDialogDict[value]
            except KeyError:
                localized = U"UNTRANSLATED: " + unicode(value)
            reasonField.setStringW(localized)
            noButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.NoButtonID))
            noButton.hide()
            noBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.NoButtonTextID))
            noBtnText.hide()
            yesBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesButtonTextID))
            yesBtnText.setStringW(PtGetLocalizedString("KI.YesNoDialog.OKButton"))
            self.YNWhatReason = kGUI.YNQuit
            if command == kGUI.KIOKDialogNoQuit:
                self.YNWhatReason = kGUI.YNNoReason
            KIYesNo.dialog.show()
        elif command == kDisableYeeshaBook:
            self.isYeeshaBookEnabled = False
        elif command == kEnableYeeshaBook:
            self.isYeeshaBookEnabled = True
        elif command == kQuitDialog:
            yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
            yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
            self.ILocalizeQuitNoDialog()
            logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutTextID))
            logoutText.show()
            logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutButtonID))
            logoutButton.show()
            KIYesNo.dialog.show()
        elif command == kDisableEntireYeeshaBook:
            self.isEntireYeeshaBookEnabled = False
        elif command == kEnableEntireYeeshaBook:
            self.isEntireYeeshaBookEnabled = True
        elif command == kUpgradeKIMarkerLevel:
            self.IUpgradeKIMarkerLevel(value)
            self.IRefreshMiniKIMarkerDisplay()
        elif command == kKIShowMiniKI:
            if self.KILevel >= kNormalKI:
                self.chatMgr.ClearBBMini(0)
        elif command == kFriendInviteSent:
            if value == 0:
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Messages.InviteSuccess"))
            else:
                if value == 30:
                    msg = PtGetLocalizedString("KI.Errors.InvitesDisabled")
                else:
                    msg = "Error occured trying to send invite: " + str(value)
                self.chatMgr.AddChatLine(None, msg, kChat.SystemMessage)

        elif command == kRegisterImager:
            self.imagerMap[value[0]] = value[1]

        #~~~~~~~~~~~~~~~~~~~~~#
        # GZ Markers messages #
        #~~~~~~~~~~~~~~~~~~~~~#
        elif command == kGZUpdated:
            if value != 0:
                # Setting the max in the GZ marker chronicle.
                vault = ptVault()
                entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
                if entry is not None:
                    markers = entry.chronicleGetValue()
                    if len(markers) < value:
                        # Need to increase the capacity of the markers; start as active.
                        markers += kGZMarkerAvailable * (value - len(markers))
                        entry.chronicleSetValue(markers)
                        entry.save()
                else:
                    # If there are none, then just add another entry; start as active.
                    markers = kGZMarkerAvailable * value
                    vault.addChronicleEntry(kChronicleGZMarkersAquired, kChronicleGZMarkersAquiredType, markers)
            self.IDetermineKILevel()
            self.IDetermineGZ()
            self.IRefreshMiniKIMarkerDisplay()
        elif command == kGZFlashUpdate:
            try:
                args = value.split()
                GZGame = int(args[0])
            except:
                PtDebugPrint("xKI.OnKIMsg(): Cannot Update Marker Display, invalid Parameters: %s." % value)
                return
            if GZGame == -1:
                self.IGZFlashUpdate(value)
            else:
                self.IDetermineKILevel()
                if self.gKIMarkerLevel > kKIMarkerNotUpgraded and self.gKIMarkerLevel < kKIMarkerNormalLevel:
                    self.IGZFlashUpdate(value)
            self.IRefreshMiniKIMarkerDisplay()
        elif command == kGZInRange:
            # Only say markers are in range if there are more markers to get.
            if self.gMarkerToGetNumber > self.gMarkerGottenNumber:
                self.gGZMarkerInRange = value[0]
                self.gGZMarkerInRangeRepy = value[1]
                self.IRefreshMiniKIMarkerDisplay()
                if not KIMini.dialog.isEnabled():
                    NewItemAlert.dialog.show()
                    KIAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                    KIAlert.show()
        elif command == kGZOutRange:
            self.gGZMarkerInRange = 0
            self.gGZMarkerInRangeRepy = None
            self.IRefreshMiniKIMarkerDisplay()
            NewItemAlert.dialog.hide()
            KIAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
            KIAlert.hide()
        elif command == kGames.MGStartCGZGame:
            PtDebugPrint("xKI.OnKIMsg(): Creating a CGZ Marker Game with (game number = %s)." % (value))
            if value is not None:
                self.markerGameManager.createCGZMarkerGame(value)
            else:
                PtDebugPrint("xKI.OnKIMsg(): ERROR: Invalid game parameter, aborting game creation.")
        elif command == kGames.MGStopCGZGame:
            if self.markerGameManager is not None:
                self.markerGameManager.stopCGZGame()

        #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
        # User-created Marker Games messages #
        #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
        elif command == kKICreateMarker:
            self.ICreateAMarker()
        elif command == kKICreateMarkerFolder:
            self.ICreateMarkerGame()

    ## Called by Plasma on receipt of a message from the game client.
    # The game client handles Marker Games.
    def OnGameCliMsg(self, msg):

        msgType = msg.getType()

        if msgType == PtGameCliMsgTypes.kGameCliPlayerJoinedMsg:
            joinMsg = msg.upcastToFinalGameCliMsg()
            if joinMsg.playerID() != PtGetLocalClientID():
                return
            if self.markerGameDisplay is not None:
                self.markerGameDisplay.registerPlayerJoin(joinMsg)
            else:
                self.markerGameManager.registerPlayerJoin(joinMsg)
            return

        if msgType != PtGameCliMsgTypes.kGameCliMarkerMsg:
            return

        msg = msg.upcastToGameMsg()
        msgType = msg.getMarkerMsgType()
        finalMsg = msg.upcastToFinalMarkerMsg()

        # Is a template being created?
        if msgType == PtMarkerMsgTypes.kMarkerTemplateCreated:
            if self.markerGameDisplay is not None:
                self.markerGameDisplay.registerTemplateCreated(finalMsg)
            else:
                self.markerGameManager.registerTemplateCreated(finalMsg)

        # Is a game being started?
        elif msgType == PtMarkerMsgTypes.kMarkerGameStarted:
            if self.markerGameDisplay is not None and self.markerGameDisplay.isMyMsg(finalMsg):
                self.IBKCheckContentRefresh(self.BKCurrentContent)

        # Is a game being paused?
        elif msgType == PtMarkerMsgTypes.kMarkerGamePaused:
            if self.markerGameManager.isMyMsg(finalMsg):
                self.markerGameManager.registerPauseGame(finalMsg)
            self.IBKCheckContentRefresh(self.BKCurrentContent)

        # Is a game being reset?
        elif msgType == PtMarkerMsgTypes.kMarkerGameReset:
            self.markerGameManager.registerResetGame(finalMsg)
            self.markerGameDisplay.registerResetGame(finalMsg)
            self.IBKCheckContentRefresh(self.BKCurrentContent)

        # Is a game over?
        elif msgType == PtMarkerMsgTypes.kMarkerGameOver:
            self.markerGameManager.registerMarkerGameOver(finalMsg)

        # Is a game's name being changed?
        elif msgType == PtMarkerMsgTypes.kMarkerGameNameChanged:
            if self.markerGameDisplay is not None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay.registerGameName(finalMsg)
                return
            self.markerGameManager.registerGameName(finalMsg)

        # Is a game being deleted?
        elif msgType == PtMarkerMsgTypes.kMarkerGameDeleted:
            if self.markerGameDisplay is not None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay = None
            self.markerGameManager.registerDeleteGame(finalMsg)

        # Is a marker being added?
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerAdded:
            if self.markerGameDisplay is not None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay.registerMarker(finalMsg)
                    if self.pendingMGmessage is not None and self.pendingMGmessage == kMessageWait.createMarker and BigKI.dialog.isEnabled():
                        self.pendingMGmessage = None
                        self.ISetWorkingToCurrentMarkerFolder()
                    return
            self.pendingMGmessage = None
            self.markerGameManager.registerMarker(finalMsg)

        # Is a marker being deleted?
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerDeleted:
            if self.markerGameDisplay is not None and self.markerGameDisplay.isMyMsg(finalMsg):
                self.markerGameDisplay.registerDeleteMarker(finalMsg)
                self.IBKCheckContentRefresh(self.BKCurrentContent)

        # Is a marker's name being changed?
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerNameChanged:
            if self.markerGameDisplay is not None and self.markerGameDisplay.isMyMsg(finalMsg):
                self.markerGameDisplay.registerMarkerNameChanged(finalMsg)
                if self.pendingMGmessage is not None and self.pendingMGmessage == kMessageWait.changeMarkerName:
                    if MFdialogMode == kMFEditingMarker:
                        self.IBKCheckContentRefresh(self.BKCurrentContent)
            self.pendingMGmessage = None

        # Is a marker being captured?
        elif msgType == PtMarkerMsgTypes.kMarkerMarkerCaptured:
            if self.markerGameDisplay is not None and self.markerGameDisplay.isMyMsg(finalMsg):
                if self.markerGameManager.gameData.data['svrGameTemplateID'] != self.markerGameDisplay.gameData.data['svrGameTemplateID']:
                    self.markerGameDisplay.registerMarkerCaptured(finalMsg)
            if self.markerGameManager.gameLoaded() and self.markerGameManager.isMyMsg(finalMsg):
                self.markerGameManager.registerMarkerCaptured(finalMsg)
        # Is a marker game's type being set?
        elif msgType == PtMarkerMsgTypes.kMarkerGameType:
            if self.markerGameDisplay is not None:
                if self.markerGameDisplay.isMyMsg(finalMsg):
                    self.markerGameDisplay.registerGameType(finalMsg)
                return
            self.markerGameManager.registerGameType(finalMsg)

    ## Called by Plasma on receipt of a backdoor message.
    # These backdoor messages can trigger various actions.
    def OnBackdoorMsg(self, target, param):

        if target == "ki" and param == "refresh":
            self.BKFolderLineDict = self.BKJournalFolderDict
            self.BKFolderListOrder = self.BKJournalListOrder
            self.BKFolderSelected = self.BKJournalFolderSelected
            self.BKFolderTopLine = self.BKJournalFolderTopLine

            self.IBigKIRefreshFolders()
            self.IBigKIRefreshFolderDisplay()
            self.IBigKIRefreshContentList()
            self.IBigKIRefreshContentListDisplay()

            self.IBigKIChangeMode(kGUI.BKListMode)

        if target.lower() == "cgz":
            self.markerGameManager.OnBackdoorMsg(target, param)

    ## Called by Plasma on receipt of a chat message.
    # This does not get called if the user sends a chat message through the KI,
    # only if he's getting a new message from another player.
    def OnRTChat(self, player, message, flags):

        if message is not None:
            message = unicode(message, kCharSet)
            cFlags = ChatFlags(flags)
            # Is it a private channel message that can't be listened to?
            if cFlags.broadcast and cFlags.channel != self.chatMgr.privateChatChannel:
                return

            # Censor the message to the player's taste.
            message = xCensor.xCensor(message, self.censorLevel)

            # Is the message from an ignored plaer?
            vault = ptVault()
            ignores = vault.getIgnoreListFolder()
            if ignores is not None and ignores.playerlistHasPlayer(player.getPlayerID()):
                return

            # Display the message if it passed all the above checks.
            self.chatMgr.AddChatLine(player, message, cFlags, forceKI=not self.sawTheKIAtLeastOnce)

            # If they are AFK and the message was directly to them, send back their state to sender.
            try:
                if self.KIDisabled or PtGetLocalAvatar().avatar.getCurrentMode() == PtBrainModes.kAFK and cFlags.private and not cFlags.toSelf:
                    myself = PtGetLocalPlayer()
                    AFKSelf = ptPlayer(myself.getPlayerName() + PtGetLocalizedString("KI.Chat.AFK"), myself.getPlayerID())
                    PtSendRTChat(AFKSelf, [player], " ", cFlags.flags)
            except NameError:
                pass

    ## Called by Plasma when a timer is running.
    # Used to handle fading and the current time in the BigKI.
    def OnTimer(self, id):

        # Chat fading.
        if id == kTimers.Fade:
            # If it is fading, fade a tick.
            if self.chatMgr.fadeMode == kChat.FadeFullDisp:
                self.chatMgr.currentFadeTick -= 1
                # Setup call for next second.
                if self.chatMgr.currentFadeTick > 0:
                    PtAtTimeCallback(self.key, kGUI.FullTickTime, kTimers.Fade)
                else:
                    self.chatMgr.fadeMode = kChat.FadeDoingFade
                    self.chatMgr.currentFadeTick = kChat.TicksOnFade
                    PtAtTimeCallback(self.key, kChat.FadeTickTime, kTimers.Fade)
            elif self.chatMgr.fadeMode == kChat.FadeDoingFade:
                self.chatMgr.currentFadeTick -= 1
                if self.chatMgr.currentFadeTick > 0:
                    # Fade a little.
                    if self.KILevel < kNormalKI:
                        mKIdialog = KIMicro.dialog
                    else:
                        mKIdialog = KIMini.dialog
                    mKIdialog.setForeColor(-1, -1, -1, self.originalForeAlpha * self.chatMgr.currentFadeTick / kChat.TicksOnFade)
                    mKIdialog.setSelectColor(-1, -1, -1, self.originalSelectAlpha * self.chatMgr.currentFadeTick / kChat.TicksOnFade)
                    mKIdialog.refreshAllControls()
                    # Setup call for next second.
                    PtAtTimeCallback(self.key, kChat.FadeTickTime, kTimers.Fade)
                else:
                    # Completely fade out.
                    self.chatMgr.FadeCompletely()
            elif self.chatMgr.fadeMode == kChat.FadeStopping:
                self.chatMgr.fadeMode = kChat.FadeNotActive

        # Time of day.
        elif id == kTimers.BKITODCheck and BigKI.dialog.isEnabled():
                self.IBigKISetChanging()

        # Time of the currently played Marker Game.
        elif id == kTimers.MarkerGame and self.currentPlayingMarkerGame is not None:
                self.currentPlayingMarkerGame.updateGameTime()
                PtAtTimeCallback(self.key, 1, kTimers.MarkerGame)

        # Stop an alert.
        elif id == kTimers.AlertHide:
            self.IAlertStop()

        # Take a snapshot after a waiting period.
        elif id == kTimers.TakeSnapShot:
            PtDebugPrint("xKI.OnTimer(): Taking snapshot.")
            PtStartScreenCapture(self.key)

        # Dump the open logs.
        elif id == kTimers.DumpLogs:
            if (PtDumpLogs(self.logDumpDest)):
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Messages.LogDumpSuccess", [self.logDumpDest]))
            else:
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Messages.LogDumpFailed", [self.logDumpDest]))

        # Turn off the KI light.
        elif id == kTimers.LightStop:
            self.DoKILight(0, 0)

        # Turn on the Jalak GUI buttons.
        elif id == kTimers.JalakBtnDelay:
            self.SetJalakGUIButtons(1)

    ## Called by Plasma when a screen capture is done.
    # This gets called once the screen capture is performed and ready to be
    # processed by the KI.
    def OnScreenCaptureDone(self, image):

        PtDebugPrint("xKI.OnScreenCaptureDone(): Snapshot is ready to be processed.")
        self.IBigKICreateJournalImage(image)
        # Only show the KI if there isn't a dialog in the way.
        if not PtIsGUIModal():
            # Make sure that we are in journal mode.
            if self.BKFolderLineDict is not self.BKJournalFolderDict:
                modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
                modeselector.setValue(0)
            # Hide any previously opened picture.
            if self.BKRightSideMode != kGUI.BKPictureExpanded:
                self.IBigKIHideMode()
            self.BKRightSideMode = kGUI.BKPictureExpanded
            # Reset the top line and selection.
            self.IBigKIRefreshFolderDisplay()
            # Prepare to edit the caption of the picture.
            self.IBigKIEnterEditMode(kGUI.BKEditFieldPICTitle)
            BigKI.dialog.show()
            # Was just the miniKI showing?
            if self.lastminiKICenter is None and self.originalminiKICenter is not None:
                dragBar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                self.lastminiKICenter = dragBar.getObjectCenter()
                dragBar.setObjectCenter(self.originalminiKICenter)
                dragBar.anchor()
            KIMini.dialog.show()
        else:
            # If the KI isn't supposed to be displayed, at least flash it so they know something happened.
            self.IAlertKIStart()

        self.takingAPicture = False

        # Save the image to the filesystem.
        if "saveAsPNG" in dir(image):
            preferredExtension = "png"
        else:
            preferredExtension = "jpg"

        basePath = os.path.join(PtGetUserPath(), kImages.Directory)
        if not PtCreateDir(basePath):
            PtDebugPrint("xKI.OnScreenCaptureDone(): Unable to create '{0}' directory. Image not saved to disk.".formatZ(basePath))
            return

        imageList = glob.iglob(os.path.join(basePath, "{0}[0-9][0-9][0-9][0-9].{1}".format(kImages.FileNameTemplate, preferredExtension)))
        imageNumbers = [int(os.path.basename(img)[7:-4]) for img in imageList] + [0]
        missingNumbers = set(range(1, max(imageNumbers))).difference(set(imageNumbers))
        if len(missingNumbers) > 0:
            firstMissing = min(missingNumbers)
        else:
            firstMissing = max(imageNumbers) + 1
        tryName = os.path.join(basePath, U'{0}{1:04d}.{2}'.format(kImages.FileNameTemplate, firstMissing, preferredExtension))

        PtDebugPrint("xKI.OnScreenCaptureDone(): Saving image to '{0}'.".format(tryName), level=kWarningLevel)
        if "saveAsPNG" in dir(image):
            image.saveAsPNG(tryName)
        else:
            image.saveAsJPEG(tryName, 90)

    ## Called by Plasma when the player list has been updated.
    # This makes sure that everything is updated and refreshed.
    def OnMemberUpdate(self):

        PtDebugPrint("xKI.OnMemberUpdate(): Refresh player list.", level=kDebugDumpLevel)
        if PtIsDialogLoaded("KIMini"):
            self.IRefreshPlayerList()
            self.IRefreshPlayerListDisplay()

    ## Called by Plasma when a new player is selected in the player list.
    def OnRemoteAvatarInfo(self, player):

        if self.KILevel < kNormalKI:
            return
        avatarSet = 0
        if isinstance(player, ptPlayer):
            avatarSet = 1
            self.BKPlayerSelected = player
            sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
            sendToField.setString(player.getPlayerName())
            self.IBigKISetToButtons()
            # Find the player in the list and select them.
            for pidx in range(len(self.BKPlayerList)):
                if isinstance(self.BKPlayerList[pidx], ptPlayer) and self.BKPlayerList[pidx] == player:
                    playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
                    playerlist.setSelection(pidx)
                    # Set the caret for the chat.
                    caret = ptGUIControlTextBox(KIMini.dialog.getControlFromTag(kGUI.ChatCaretID))
                    caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt") + unicode(player.getPlayerName()) + U" >")
                    privateChbox = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniPrivateToggle))
                    privateChbox.setChecked(1)
                    break
        if avatarSet:
            if not KIMini.dialog.isEnabled():
                KIMini.dialog.show()
            self.KillFadeTimer()
            self.StartFadeTimer()

    ## Called by Plasma on receipt of a high-level player vault event.
    def OnVaultNotify(self, event, tupData):

        PtDebugPrint("xKI.OnVaultNotify(): Event = %d and data = " % (event), tupData, level=kDebugDumpLevel)
        if not tupData:
            PtDebugPrint("xKI.OnVaultNotify(): Bailing, no Age data.")
            return

        if PtIsDialogLoaded("KIMain"):
            if event == PtVaultNotifyTypes.kRegisteredOwnedAge or event == PtVaultNotifyTypes.kUnRegisteredOwnedAge or event == PtVaultNotifyTypes.kRegisteredVisitAge or event == PtVaultNotifyTypes.kUnRegisteredVisitAge:
                if self.KILevel > kMicroKI:
                    # A new owned Age was added, refresh its folders.
                    if isinstance(tupData[0], ptVaultAgeLinkNode):
                        # Is it the neighborhood?
                        ownedAge = tupData[0].getAgeInfo()
                        if ownedAge is not None:
                            if self.IIsAgeMyNeighborhood(ownedAge):
                                self.IBigKIRefreshHoodStatics(ownedAge)
                                self.IRefreshPlayerList()
                                self.IRefreshPlayerListDisplay()
                            # Rebuild the player folder list because it might have changed.
                            self.IBigKIRefreshFolders()
                            self.IBigKIRefreshFolderDisplay()
                            self.IBigKIRefreshContentList()
                            self.IBigKIRefreshContentListDisplay()
                            self.IRefreshAgeOwnerSettings()
                        else:
                            PtDebugPrint("xKI.OnVaultNotify(): ERROR: No ageInfo. ", level=kErrorLevel)
                    else:
                        PtDebugPrint("xKI.OnVaultNotify(): ERROR: Unknown tuple data type. ", level=kErrorLevel)
            else:
                PtDebugPrint("xKI.OnVaultNotify(): ERROR: unknown event %d." % (event), level=kWarningLevel)
        else:
            PtDebugPrint("xKI.OnVaultNotify(): BigKI dialog was not loaded, waiting.", level=kDebugDumpLevel)

    ## Called by Plasma on receipt of a low-level player vault event.
    def OnVaultEvent(self, event, tupData):

        PtDebugPrint("xKI.VaultEvent(): Event = %d and data = " % (event), tupData, level=kDebugDumpLevel)
        self.HandleVaultTypeEvents(event, tupData)

    ## Called by Plasma on receipt of a low-level Age vault event.
    def OnAgeVaultEvent(self, event, tupData):

        PtDebugPrint("xKI.OnAgeVaultEvent(): Event = %d and data = " % (event), tupData, level=kDebugDumpLevel)
        self.HandleVaultTypeEvents(event, tupData)

    ## Called by Plasma when a marker has been captured by the player.
    def OnMarkerMsg(self, msgType, tupData):

        if msgType == PtMarkerMsgType.kMarkerCaptured:
            PtDebugPrint("xKI.OnMarkerMsg(): Marker captured; ID = \"%s\"." % tupData[0], level=kDebugDumpLevel)
            if self.markerGameDisplay is not None:
                return
            self.markerGameManager.captureMarker(tupData[0])

    ############
    # KI Setup #
    ############

    ## Sets up the KI for a given player.
    # Goes through all the steps required to ensure the player's KI is
    # appropriately up-to-date when a user starts playing.
    def ISetupKI(self):
        "All logic necessary to setup the KI for a given player"

        self.BKPlayerList = []
        self.BKPlayerSelected = None
        self.previouslySelectedPlayer = None

        self.BKJournalFolderDict = {}
        self.BKJournalListOrder = []
        self.BKJournalFolderSelected = 0
        self.BKJournalFolderTopLine = 0
        self.BKPlayerFolderDict = {}
        self.BKPlayerListOrder = []
        self.BKPlayerFolderSelected = 0
        self.BKPlayerFolderTopLine = 0
        self.BKConfigFolderDict = {}
        self.BKConfigFolderSelected = 0
        self.BKConfigFolderTopLine = 0
        self.BKFolderLineDict = self.BKJournalFolderDict
        self.BKFolderListOrder = self.BKJournalListOrder
        self.BKFolderSelected = self.BKJournalFolderSelected
        self.BKFolderTopLine = self.BKJournalFolderTopLine
        self.BKFolderSelectChanged = False
        self.BKIncomingFolder = None

        self.BKNewItemsInInbox = 0
        self.BKCurrentContent = None
        self.BKContentList = []
        self.BKContentListTopLine = 0

        self.isYeeshaBookEnabled = True
        self.isEntireYeeshaBookEnabled = True

        self.IDetermineCensorLevel()
        self.IDetermineKILevel()
        self.IDetermineKIFlags()
        self.IDetermineGZ()

        # Hide all dialogs first.
        KIMicroBlackbar.dialog.hide()
        KIMicro.dialog.hide()
        KIMini.dialog.hide()
        KIBlackbar.dialog.hide()
        BigKI.dialog.hide()
        self.chatMgr.ToggleChatMode(0)

        if self.KILevel == kMicroKI:
            # Show the microBlackbar.
            KIMicroBlackbar.dialog.show()
            # Show the microKI.
            KIMicro.dialog.show()
        elif self.KILevel == kNormalKI:
            # Show the normal Blackbar.
            KIBlackbar.dialog.show()
            self.chatMgr.ClearBBMini()
            # Check for unseen messages.
            self.ICheckInboxForUnseen()

        self.IminiPutAwayKI()

        modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
        modeselector.setValue(0)

        self.IBigKIRefreshFolders()
        self.IBigKIRefreshFolderDisplay()
        self.IBigKIRefreshContentList()
        self.IBigKIRefreshContentListDisplay()

        self.IBigKIChangeMode(kGUI.BKListMode)

        # Clear out any existing marker game.
        self.markerGameManager = MarkerGameManager(self)

    ############
    # KI Flags #
    ############

    ## Sets the KI Flags from the Chronicle.
    # KI Flags are settings for the player's KI (pertaining to Buddies).
    def IDetermineKIFlags(self):

        vault = ptVault()
        
        # Only get PMs and KI Mails from Buddies.
        entry = vault.findChronicleEntry(kChron.OnlyPMs)
        if entry is None:
            # Not found, set to 0 by default.
            vault.addChronicleEntry(kChron.OnlyPMs, kChron.OnlyPMsType, str(self.onlyGetPMsFromBuddies))
        else:
            self.onlyGetPMsFromBuddies = int(entry.chronicleGetValue())

        # Only allow the player to be buddied on request.
        entry = vault.findChronicleEntry(kChron.BuddiesOnRequest)
        if entry is None:
            # Not found, set to 0 by default.
            vault.addChronicleEntry(kChron.BuddiesOnRequest, kChron.BuddiesOnRequestType, str(self.onlyAllowBuddiesOnRequest))
        else:
            self.onlyAllowBuddiesOnRequest = int(entry.chronicleGetValue())

    ## Save the KI Flags to the Chronicle.
    def ISaveKIFlags(self):

        vault = ptVault()
        # Only get PMs and KI Mails from Buddies.
        entry = vault.findChronicleEntry(kChron.OnlyPMs)
        if entry is not None:
            entry.chronicleSetValue(str(self.onlyGetPMsFromBuddies))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.OnlyPMs, kChron.OnlyPMsType, str(self.onlyGetPMsFromBuddies))

        # Only allow the player to be buddied on request.
        entry = vault.findChronicleEntry(kChron.BuddiesOnRequest)
        if entry is not None:
            entry.chronicleSetValue(str(self.onlyAllowBuddiesOnRequest))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.BuddiesOnRequest, kChron.BBuddiesOnRequestType, str(self.onlyAllowBuddiesOnRequest))

    ############
    # KI Light #
    ############

    ## Finds out what the current KI light state is.
    def CheckKILight(self):

        timeRemaining = self.GetKILightChron()
        if not timeRemaining:
            PtDebugPrint("xKI.CheckKILight(): Had KI light, but it's currently off.", level=kDebugDumpLevel)
            self.DoKILight(0, 1)
        elif timeRemaining > 0:
            PtDebugPrint("xKI.CheckKILight(): Have KI light, time remaining = ", timeRemaining, level=kDebugDumpLevel)
            self.DoKILight(1, 1, timeRemaining)
            self.SetKILightChron(0)
        else:
            PtDebugPrint("No KI light.", level=kDebugDumpLevel)

    ## Get the KI light remaining time from the chronicle.
    def GetKILightChron(self):

        vault = ptVault()
        entry = vault.findChronicleEntry("KILightStop")
        if entry is not None:
            entryValue = entry.chronicleGetValue()
            remaining = int(entryValue)
            return remaining
        else:
            PtDebugPrint("xKI.GetKILightChron(): No KI light.", level=kDebugDumpLevel)
            return -1

    ## Set the KI light remaining time in the chronicle.
    def SetKILightChron(self, remaining):

        vault = ptVault()
        entry = vault.findChronicleEntry("KILightStop")
        if entry is not None:
            entryValue = entry.chronicleGetValue()
            oldVal = int(entryValue)
            if remaining == oldVal:
                return
            PtDebugPrint("xKI.SetKILightChron(): Set KI light chron to: ", remaining, level=kDebugDumpLevel)
            entry.chronicleSetValue(str(remaining))
            entry.save()

    ## Manages the KI light.
    def DoKILight(self, state, ff, remaining=0):

        thisResp = listLightResps[state]
        LocalAvatar = PtGetLocalAvatar()
        avatarKey = LocalAvatar.getKey()
        avatarObj = avatarKey.getSceneObject()
        respList = avatarObj.getResponders()
        if len(respList) > 0:
            PtDebugPrint("xKI.DoKILight(): Responder list:", level=kDebugDumpLevel)
            for resp in respList:
                PtDebugPrint("                 %s" % (resp.getName()))
                if resp.getName() == thisResp:
                    PtDebugPrint("xKI.DoKILight(): Found KI light resp: %s" % (thisResp), level=kDebugDumpLevel)
                    atResp = ptAttribResponder(42)
                    atResp.__setvalue__(resp)
                    atResp.run(self.key, avatar=LocalAvatar, fastforward=ff)
                    if state:
                        PtAtTimeCallback(self.key, remaining, kTimers.LightStop)
                        PtDebugPrint("xKI.DoKILight(): Light was on in previous age, turning on for remaining ", remaining, " seconds.", level=kWarningLevel)
                        curTime = PtGetDniTime()
                        self.lightStop = (remaining + curTime)
                        self.lightOn = True
                    else:
                        PtDebugPrint("xKI.DoKILight(): Light is shut off, updating chron.", level=kWarningLevel)
                        self.SetKILightChron(remaining)
                        self.lightOn = False
                        PtSetLightAnimStart(avatarKey, KILightObjectName, False)
                    break
        else:
            PtDebugPrint("xKI.DoKILight(): ERROR: couldn't find any responders.")

    ################
    # Localization #
    ################

    ## Gets the appropriate localized values for a Yes/No dialog.
    def ILocalizeDialog(self, dialog_type=0):

        confirm = "KI.YesNoDialog.QuitButton"
        if dialog_type == 1:
            confirm = "KI.YesNoDialog.YESButton"
        yesButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesButtonTextID))
        noButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.NoButtonTextID))
        yesButton.setStringW(PtGetLocalizedString(confirm))
        noButton.setStringW(PtGetLocalizedString("KI.YesNoDialog.NoButton"))

    ############
    # GZ Games #
    ############

    ## Sets the GZ globals from the Chronicle.
    def IDetermineGZ(self):

        if self.gKIMarkerLevel > kKIMarkerNotUpgraded:
            if self.gKIMarkerLevel < kKIMarkerNormalLevel:
                vault = ptVault()
                entry = vault.findChronicleEntry(kChronicleGZGames)
                error = 0
                if entry is not None:
                    gameString = entry.chronicleGetValue()
                    PtDebugPrint("xKI.IDetermineGZ(): Game string is: \"%s\"." % (gameString), level=kWarningLevel)
                    args = gameString.split()
                    if len(args) == 3:
                        try:
                            self.gGZPlaying = int(args[0])
                            colors = args[1].split(':')
                            outof = args[2].split(':')

                            # Check for corrupted entry.
                            if len(colors) != 2 or len(outof) !=2:
                                PtDebugPrint("xKI.IDetermineGZ(): Invalid color field or marker field.")
                                raise ValueError

                            # Check for invalid entry.
                            if (colors[0] == 'red' or colors[0] == 'green') and int(outof[1]) > 15:
                                PtDebugPrint("xKI.IDetermineGZ(): Invalid marker number entry (i.e. 1515 bug).")
                                raise ValueError

                            self.gMarkerGottenColor = colors[0]
                            self.gMarkerToGetColor = colors[1]
                            self.gMarkerGottenNumber = int(outof[0])
                            self.gMarkerToGetNumber = int(outof[1])

                            return
                        except:
                            PtDebugPrint("xKI.IDetermineGZ(): ERROR: Could not read GZ Games Chronicle.")
                            error = 1
                    else:
                        PtDebugPrint("xKI.IDetermineGZ(): ERROR: Invalid GZ Games string formation.")
                        error = 1
                
                # If there was a problem, reset everything to "off".
                self.gGZPlaying = 0
                self.gMarkerToGetColor = "off"
                self.gMarkerGottenColor = "off"
                self.gMarkerToGetNumber = 0
                self.gMarkerGottenNumber = 0

                # Reset Marker Games if a corrupted vault occurred.
                if error:
                    PtDebugPrint("xKI.IDetermineGZ(): ERROR: Vault corrupted, resetting all Marker Game data.", level=kErrorLevel)
                    import grtzKIMarkerMachine
                    grtzKIMarkerMachine.ResetMarkerGame()
            else:
                # Can't be playing a GZ Game.
                self.gGZPlaying = 0
                # Clear only if there are no currently active games.
                if self.markerGameState == kGames.MGNotActive or self.currentPlayingMarkerGame is None:
                    self.gMarkerToGetColor = "off"
                    self.gMarkerGottenColor = "off"
                    self.gMarkerToGetNumber = 0
                    self.gMarkerGottenNumber = 0
        else:
            # Reset everything to "off".
            gGZPlaying = 0
            gMarkerToGetColor = "off"
            gMarkerGottenColor = "off"
            gMarkerToGetNumber = 0
            gMarkerGottenNumber = 0

    ## Update the GZ globals using provided values, not the Chronicle.
    def IGZFlashUpdate(self, gameString):
        
        PtDebugPrint("xKI.IGZFlashUpdate(): Game string is: \"%s\"." % (gameString), level=kWarningLevel)
        args = gameString.split()
        if len(args) == 3:
            try:
                GZPlaying = int(args[0])
                colors = args[1].split(':')
                outof = args[2].split(':')

                # Check for corrupted entry.
                if len(colors) != 2 or len(outof) !=2:
                    PtDebugPrint("xKI.IGZFlashUpdate(): Invalid color field or marker field.")
                    raise ValueError

                MarkerGottenColor = colors[0]
                MarkerToGetColor = colors[1]
                MarkerGottenNumber = int(outof[0])
                MarkerToGetNumber = int(outof[1])

                # Check for invalid entry.
                if (colors[0] == 'red' or colors[0] == 'green') and MarkerToGetNumber > 15:
                    PtDebugPrint("xKI.IGZFlashUpdate(): Invalid marker number entry (i.e. 1515 bug).")
                    raise ValueError

                # Make sure the player is playing a GZ Game.
                if GZPlaying != -1:
                    self.gGZPlaying = GZPlaying

                self.gMarkerGottenColor = MarkerGottenColor
                self.gMarkerToGetColor = MarkerToGetColor
                self.gMarkerGottenNumber = MarkerGottenNumber
                self.gMarkerToGetNumber = MarkerToGetNumber
                return

            except:
                PtDebugPrint("xKI.IGZFlashUpdate(): ERROR: Could not read GZ Games Chronicle. Checking Chronicle for corruption.")
        else:
            PtDebugPrint("xKI.IGZFlashUpdate(): ERROR: Invalid GZ Games string formation. Checking Chronicle for corruption.")

        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleGZGames)
        if entry is not None:
            if gameString == entry.chronicleGetValue():
                PtDebugPrint("xKI.IGZFlashUpdate(): ERROR: Vault corrupted: trying to gracefully reset to a default state.")
                import grtzKIMarkerMachine
                grtzKIMarkerMachine.ResetMarkerGame()
                return

    ## Update the Chronicle's GZ Games values.
    # This takes the form of a series of values, separated by ":".
    def IUpdateGZGamesChronicle(self):

        if self.gGZPlaying:
            vault = ptVault()
            entry = vault.findChronicleEntry(kChronicleGZGames)
            upString = "%d %s:%s %d:%d" % (self.gGZPlaying, self.gMarkerGottenColor, self.gMarkerToGetColor, self.gMarkerGottenNumber, self.gMarkerToGetNumber)
            if entry is not None:
                entry.chronicleSetValue(upString)
                entry.save()
            else:
                vault.addChronicleEntry(kChronicleGZGames, kChronicleGZGamesType, upString)

    ################
    # Marker Games #
    ################

    ## Selects a new Marker Game type.
    def SelectMarkerType(self, tagID):

        dlgObj = KICreateMarkerGameGUI
        if tagID and self.selectedMGType != tagID and self.selectedMGType != 0:
            PtDebugPrint("xKI.SelectMarkerType(): Old Marker Game type ID: ", self.selectedMGType, level=kDebugDumpLevel)
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(self.selectedMGType)).enable()
            self.ChangeMarkerTypeColor(self.selectedMGType)

        self.selectedMGType = tagID
        PtDebugPrint("xKI.SelectMarkerType(): Selecting new Marker Game type: ", self.selectedMGType, level=kDebugDumpLevel)
        ptGUIControlButton(dlgObj.dialog.getControlFromTag(self.selectedMGType)).disable()
        self.ChangeMarkerTypeColor(tagID)

    ## Change the Marker Game type color.
    def ChangeMarkerTypeColor(self, tagID):

        dlgObj = KICreateMarkerGameGUI
        currentColor = ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID + 5)).getForeColor()
        if currentColor == self.markerGameDefaultColor:
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID + 5)).setForeColor(self.markerGameSelectedColor)
        elif currentColor == self.markerGameSelectedColor:
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID + 5)).setForeColor(self.markerGameDefaultColor)

    ## Initialize the Marker Game creation GUI.
    def InitMarkerGameGUI(self):

        dlgObj = KICreateMarkerGameGUI
        self.selectedMGType = kGUI.MarkerGameType1
        if self.MGKILevel == 2:
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType1)).disable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType2)).enable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType2)).show()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel2)).show()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType3)).enable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType3)).show()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel3)).show()
        elif self.MGKILevel == 1:
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType1)).disable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType2)).enable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType2)).show()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel2)).show()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType3)).hide()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel3)).hide()
        elif self.MGKILevel == 0:
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType1)).disable()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType2)).hide()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel2)).hide()
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameType3)).hide()
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel3)).hide()

        ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel1)).setForeColor(self.markerGameSelectedColor)
        ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel2)).setForeColor(self.markerGameDefaultColor)
        ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(kGUI.MarkerGameLabel3)).setForeColor(self.markerGameDefaultColor)

        playerName = PtGetLocalPlayer().getPlayerName()
        if playerName[-1] == "s":
            addToName = "'"
        else:
            addToName = "'s"
        gameName = playerName + addToName + " Marker Game"

        ptGUIControlEditBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kGUI.CreateMarkerGameNameEB)).setString(gameName)

    ## Begin the creation of a new Marker Game.
    def ICreateMarkerGame(self):

        # Make sure the player's KI Level is appropriately high.
        if self.KILevel <= kMicroKI or self.KIDisabled:
            PtDebugPrint("xKI.ICreateMarkerGame(): DEBUG: Aborting Marker Game creation request, player does not have the KI.")
            return

        # Make sure the player's KI Marker Level is appropriately high.
        if not self.phasedKICreateMarkerGame or self.gKIMarkerLevel < kKIMarkerNormalLevel:
            PtDebugPrint("xKI.ICreateMarkerGame(): DEBUG: Aborting Marker Game creation request, player does not have sufficient privileges.")
            return

        # The player cannot be doing another task.
        if self.takingAPicture or self.waitingForAnimation:
            PtDebugPrint("xKI.ICreateMarkerGame(): DEBUG: Aborting Marker Game creation request, player is busy.")
            return

        # The player cannot create a game if one is already in progress.
        if self.markerGameManager.gameLoaded():
            PtDebugPrint("xKI.ICreateMarkerGame(): DEBUG: Aborting Marker Game creation request, a game is already in progress.")
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.MarkerGame.createErrorExistingGame"), kChat.SystemMessage)
            return

        # Make sure the player has enough room.
        if not self.ICanMakeMarkerFolder():
            PtDebugPrint("xKI.ICreateMarkerGame(): DEBUG: Aborting Marker Game creation request, player has reached the limit of Marker Games.")
            self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullMarkerGames"))
            return

        # The player can now launch the Marker Game creation GUI.
        self.IBigKIHideBigKI()
        PtShowDialog("KIMiniMarkers")
        KIMarkerGameGUIOpen.run(self.key, netPropagate=0)


    ## Finishes creating the Marker Game after the asynchronous mini-game
    # server registers the parameters.
    def IFinishCreateMarkerFolder(self, gameName, gameGUID):

        # Get the current Age's Journal folder.
        load = 0
        while load < 2:
            try:
                journal = self.BKJournalFolderDict[self.IGetAgeInstanceName()]
                if journal is None:
                    raise
                load = 2
            except:
                if load == 1:
                    # Failed twice in a row, it's hopeless.
                    # TODO: Create the folder in case this happens.
                    PtDebugPrint("xKI.IFinishCreateMarkerFolder(): ERROR: Could not load Age's Journal Folder, Marker Game creation failed.")
                    return
                load += 1
                self.IBigKIRefreshFolders()

        # Hide the blackbar, just in case.
        KIBlackbar.dialog.hide()

        # Put the toggle button back to the BigKI setting.
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
        toggleCB.setChecked(1)

        # Set the current mode to the Age's Journal folder.
        modeSelector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
        modeSelector.setValue(0)
        self.BKFolderTopLine = self.BKJournalFolderTopLine = 0
        self.BKFolderSelected = self.BKJournalFolderSelected = self.BKJournalListOrder.index(self.IGetAgeInstanceName())
        self.IBigKIRefreshFolderDisplay()

        # Create the Marker Game node.
        PtDebugPrint("xKI.IFinishCreateMarkerFolder(): DEBUG: Creating Vault node with name = \"{0}\" and GUID = \"{1}\".".format(gameName, gameGUID))
        markerGameNode = ptVaultMarkerGameNode()
        markerGameNode.setGameName(gameName)
        markerGameNode.setGameGuid(gameGUID)
        self.BKCurrentContent = journal.addNode(markerGameNode)

        # Change to display current content.
        self.IBigKIChangeMode(kGUI.BKMarkerListExpanded)
        if BigKI.dialog.isEnabled():
            self.IBigKIShowMode()
        else:
            KIMini.dialog.hide()
            BigKI.dialog.show()
            KIMini.dialog.show()
        if self.lastminiKICenter is None:
            if self.originalminiKICenter is not None:
               dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
               self.lastminiKICenter = dragbar.getObjectCenter()
               dragbar.setObjectCenter(self.originalminiKICenter)
               dragbar.anchor()

    ## Add a new marker to the existing Marker Game.
    def ICreateAMarker(self):

        # Set this in case the player is looking at the BigKI.
        self.pendingMGmessage = kMessageWait.createMarker

        if not self.takingAPicture and not self.waitingForAnimation:
            if self.KILevel > kMicroKI and not self.KIDisabled:
                if self.markerGameDisplay is not None and self.markerGameDisplay.gameData is not None:
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
                            PtDebugPrint("xKI.ICreateAMarker(): Creating marker at: ({0}, {1}, {2}).".format(x, y, z))
                        except:
                            PtDebugPrint("xKI.ICreateAMarker(): ERROR: Marker creation failed.")
                            return
                    else:
                        self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullMarkers"))

    ## Perform the necessary operations to switch to a Marker Game.
    def ISetWorkingToCurrentMarkerFolder(self):

        if self.markerGameDisplay is None:
            PtDebugPrint("xKI.ISetWorkingToCurrentMarkerFolder(): ERROR: Cannot set working game, as the game isn't loaded.")
            return

        game = self.markerGameDisplay.gameData
        if game == None:
            PtDebugPrint("xKI.ISetWorkingToCurrentMarkerFolder(): ERROR: Cannot set working game, as there is no game data.")
            return

        if self.BKCurrentContent is None:
            PtDebugPrint("xKI.ISetWorkingToCurrentMarkerFolder(): ERROR: Cannot set working game, as there is no Vault folder.")
            return

        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint("xKI.ISetWorkingToCurrentMarkerFolder(): ERROR: Cannot set working game, as there is no Vault node.")
            return

        datatype = element.getType()
        if datatype != PtVaultNodeTypes.kMarkerGameNode:
            PtDebugPrint("xKI.ISetWorkingToCurrentMarkerFolder(): ERROR: Cannot set working game, as the Vault node is of the wrong type.")
            return

        element = element.upcastToMarkerGameNode()
        if element is None:
            PtDebugPrint("xKI.ISetWorkingToCurrentMarkerFolder(): ERROR: Cannot set working game, as the Vault node is empty.")
            return

        # Refresh the content.
        self.IRefreshPlayerList()
        self.IRefreshPlayerListDisplay()
        self.IBKCheckContentRefresh(self.BKCurrentContent)

    ## Reset from the working Marker Game to None.
    def IResetWorkingMarkerFolder(self):

        MGmgr = ptMarkerMgr()

        # Don't delete any markers necessary for an existing game.
        if not self.markerGameManager.gameLoaded():
            MGmgr.hideMarkersLocal()

        self.markerGameDisplay = None

        # Refresh the content.
        self.IRefreshPlayerList()
        self.IRefreshPlayerListDisplay()
        self.IBKCheckContentRefresh(self.BKCurrentContent)

    #########
    # Jalak #
    #########

    ## Initialize the Jalak KI GUI.
    def JalakGUIInit(self):

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

        jlakGUIButtons = [jlakRandom, jlakExtreme, jlakWall, jlakAllLow, jlakAllMed, jlakAllHigh,
                          jlakRamp, jlakSphere, jlakBigBox, jlakLilBox, jlakRect, jlakDestroy]

        obj = PtFindSceneobject("JalakDONOTTOUCH", "Jalak")
        pythonScripts = obj.getPythonMods()
        for script in pythonScripts:
            if script.getName() == kJalakPythonComponent:
                self.jalakScript = script
                PtDebugPrint("xKI.JalakGUIInit(): Found Jalak's python component.", level=kDebugDumpLevel)
                return
        PtDebugPrint("xKI.JalakGUIInit(): ERROR: Did not find Jalak's python component.")

    ## Toggle on/off the Jalak KI GUI.
    def JalakGUIToggle(self, ff=0):

        PtDebugPrint("xKI.JalakGUIToggle(): toggling GUI.", level=kDebugDumpLevel)
        ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
        if PtGetAgeName() != "Jalak":
            self.jalakGUIState = False
            return
        if self.jalakGUIState:
            self.jalakGUIState = False
            KIJalakGUIClose.run(self.key, netPropagate=0, fastforward=ff)
            if ff:
                PtHideDialog("jalakControlPanel")
        else:
            # User cannot be busy doing some other task.
            if self.takingAPicture or self.waitingForAnimation:
                PtDebugPrint("xKI.JalakGUIToggle(): DEBUG: Aborting request for Jalak GUI: user is busy.")
                return
            # Only those that have Gahreesen KI can create a game.
            if self.KILevel <= kMicroKI or self.KIDisabled:
                PtDebugPrint("xKI.JalakGUIToggle(): DEBUG: Aborting request for Jalak GUI: user does not have the KI.")
                return
            self.jalakGUIState = True
            PtShowDialog("jalakControlPanel")
            KIJalakGUIOpen.run(self.key, netPropagate=0)

    ## Sends a notification message in Jalak to the Jalak script.
    def SendJalakNote(self, extraInfo):

        note = ptNotify(self.key)
        note.clearReceivers()
        note.addReceiver(self.jalakScript)
        note.netPropagate(0)
        note.netForce(0)
        note.setActivate(1.0)
        note.addVarNumber(str(extraInfo), 1.0)
        note.send()

    ## Activate or deactivate all the buttons in the Jalak GUI.
    def SetJalakGUIButtons(self, state):

        for btn in jlakGUIButtons:
            if state:
                btn.enable()
            else:
                btn.disable()

    #############
    # KI Levels #
    #############

    ## Make sure all the parts of the KI specific to this level are undone.
    def IRemoveKILevel(self, level, upgrading=0):

        # Is it going from normal back to micro?
        if level == kNormalKI:
            avatar = PtGetLocalAvatar()
            gender = avatar.avatar.getAvatarClothingGroup()
            if gender > kFemaleClothingGroup:
                gender = kMaleClothingGroup
            avatar.netForce(1)
            if gender == kFemaleClothingGroup:
                avatar.avatar.removeClothingItem("FAccKI")
            else:
                avatar.avatar.removeClothingItem("MAccKI")
            avatar.avatar.saveClothing()
            # Fill in the listbox so that the test is near the enter box.
            chatarea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatarea.lock()         # Make the chat display immutable.
            chatarea.unclickable()  # Make the chat display non-clickable.
            chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            chatarea.disableScrollControl()
            # Hide everything specific to the normalKI.
            KIBlackbar.dialog.hide()
            KIMini.dialog.hide()
            KIListModeDialog.dialog.hide()
            KIPictureExpanded.dialog.hide()
            KIJournalExpanded.dialog.hide()
            KIPlayerExpanded.dialog.hide()
            BigKI.dialog.hide()
            KIOnAnim.animation.skipToTime(1.5)

    ## Perform all operations associated with the newly-obtained KI level.
    def IWearKILevel(self, level):

        if level == kMicroKI:
            avatar = PtGetLocalAvatar()
            gender = avatar.avatar.getAvatarClothingGroup()
            if gender > kFemaleClothingGroup:
                gender = kMaleClothingGroup
            avatar.netForce(1)
            if gender == kFemaleClothingGroup:
                avatar.avatar.wearClothingItem("FAccPlayerBook")
            else:
                avatar.avatar.wearClothingItem("MAccPlayerBook")
            avatar.avatar.saveClothing()
            # Show the microKI.
            KIMicroBlackbar.dialog.show()
            self.chatMgr.ClearBBMini()
            KIMicro.dialog.show()
            self.chatMgr.ToggleChatMode(0)
        elif level == kNormalKI:
            avatar = PtGetLocalAvatar()
            gender = avatar.avatar.getAvatarClothingGroup()
            if gender > kFemaleClothingGroup:
                gender = kMaleClothingGroup
            avatar.netForce(1)
            if gender == kFemaleClothingGroup:
                avatar.avatar.wearClothingItem("FAccKI")
            else:
                avatar.avatar.wearClothingItem("MAccKI")
            avatar.avatar.saveClothing()
            # Change the display to match the normal KI.
            KIBlackbar.dialog.show()
            self.chatMgr.ClearBBMini()
            KIOnAnim.animation.skipToTime(1.5)
            # Alert the user to the newly-available KI.
            self.IAlertKIStart()
            # Check the player's inbox.
            self.ICheckInboxForUnseen()
            # Refresh the folders, which will create the age journal for this Age.
            self.IBigKIRefreshFolders()
            # Show the microKI too.
            KIMicroBlackbar.dialog.show()
            self.chatMgr.ClearBBMini()

    ## Forcefully make sure the avatar is wearing the current KI level.
    # This ensures the player is wearing either the Yeesha Book, or the Yeesha
    # Book and the Gahreesen KI.
    def IMakeSureWeWereKILevel(self):

        if self.KILevel == kMicroKI:
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
        elif self.KILevel == kNormalKI:
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

    ## Sets the current KI level from the Chronicle.
    # Also sets the current KI Marker Level.
    def IDetermineKILevel(self):

        # Set the global KI Level.
        self.KILevel = kMicroKI
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleKILevel)
        if entry is None:
            # Not found, set to MicroKI by default.
            vault.addChronicleEntry(kChronicleKILevel, kChronicleKILevelType, str(self.KILevel))
        else:
            oldLevel = int(entry.chronicleGetValue())
            if oldLevel >= kLowestKILevel and oldLevel <= kHighestKILevel:
                self.KILevel = oldLevel
        self.chatMgr.KILevel = self.KILevel
        PtDebugPrint("xKI.IDetermineKILevel(): The KI Level is %d." % (self.KILevel), level=kWarningLevel)
        
        # Set the KI Marker Level.
        self.gKIMarkerLevel = 0
        entry = vault.findChronicleEntry(kChronicleKIMarkerLevel)
        if entry is None:
            # Not found, set to 0 by default.
            vault.addChronicleEntry(kChronicleKIMarkerLevel, kChronicleKIMarkerLevelType, str(self.gKIMarkerLevel))
        else:
            try:
                self.gKIMarkerLevel = int(entry.chronicleGetValue())
            except:
                PtDebugPrint("xKI.IDetermineKILevel(): ERROR: Chronicle entry error with the KI's Marker Level, resetting to the default value.")
                entry.chronicleSetValue(str(gKIMarkerLevel))
                entry.save()
        PtDebugPrint("xKI.IDetermineKILevel(): The KI Marker Level is %d." % (self.gKIMarkerLevel), level=kWarningLevel)

        entry = vault.findChronicleEntry("feather")
        if entry is None:
            self.gFeather = 0
        else:
            try:
                self.gFeather = int(entry.chronicleGetValue())
            except ValueError:
                self.gFeather = 0

    ## Upgrade the KI Marker Level to a new setting.
    def IUpgradeKIMarkerLevel(self, newLevel):

        PtDebugPrint("xKI.IUpgradeKIMarkerLevel(): KI Marker Level going from %d to %d." % (self.gKIMarkerLevel, newLevel), level=kWarningLevel)
        if self.KILevel > kMicroKI and newLevel > self.gKIMarkerLevel:
            self.gKIMarkerLevel = newLevel
            vault = ptVault()
            entry = vault.findChronicleEntry(kChronicleKIMarkerLevel)
            if entry is None:
                PtDebugPrint("xKI.IUpgradeKIMarkerLevel(): Chronicle entry not found, set to %d." % (self.gKIMarkerLevel), level=kWarningLevel)
                vault.addChronicleEntry(kChronicleKIMarkerLevel, kChronicleKIMarkerLevelType, str(self.gKIMarkerLevel))
            else:
                PtDebugPrint("xKI.IUpgradeKIMarkerLevel(): Upgrading existing KI Marker Level to %d." % (self.gKIMarkerLevel), level=kWarningLevel)
                entry.chronicleSetValue(str(self.gKIMarkerLevel))
                entry.save()

    ## Updates the KI level's Chronicle value.
    def IUpdateKILevelChronicle(self):

        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleKILevel)
        if entry is not None:
            entry.chronicleSetValue(str(self.KILevel))
            entry.save()
        else:
            vault.addChronicleEntry(kChronicleKILevel, kChronicleKILevelType, str(self.KILevel))

    ###############
    # Yeesha Book #
    ###############

    ## Show the Yeesha Book to the player, in accordance with its status.
    def IShowYeeshaBook(self):

        if self.KILevel >= kMicroKI and not self.KIDisabled and not self.waitingForAnimation:
            if BigKI.dialog.isEnabled() or KIMini.dialog.isEnabled():
                self.IminiPutAwayKI()
            startOpen = False
            if self.isYeeshaBookEnabled:
                if self.offeredBookMode == kGUI.NotOffering:
                    if self.phasedKIShareYeeshaBook:
                        YeeshaBDef = xLinkingBookDefs.xYeeshaBookBase + self.IGetYeeshaPageDefs()
                    else:
                        YeeshaBDef = xLinkingBookDefs.xYeeshaBookNoShare + self.IGetYeeshaPageDefs()
                else:
                    YeeshaBDef = xLinkingBookDefs.xYeeshaBookNoShare
                    startOpen = True
            else:
                YeeshaBDef = xLinkingBookDefs.xYeeshaBookBroke + self.IGetYeeshaPageDefs()
            self.yeeshaBook = ptBook(YeeshaBDef, self.key)
            self.yeeshaBook.setSize(xLinkingBookDefs.YeeshaBookSizeWidth, xLinkingBookDefs.YeeshaBookSizeHeight)
            self.yeeshaBook.show(startOpen)
            PtToggleAvatarClickability(False)

    ## Returns the definitions for the Yeesha pages.
    # Gets called whenever the Relto's Age GUI is drawn.
    def IGetYeeshaPageDefs(self):

        pageDefs = ""
        vault = ptVault()
        if vault is not None:
            psnlSDL = vault.getPsnlAgeSDL()
            if psnlSDL:
                for SDLVar, page in xLinkingBookDefs.xYeeshaPages:
                    FoundValue = psnlSDL.findVar(SDLVar)
                    if FoundValue is not None:
                        PtDebugPrint("xKI.IGetYeeshaPageDefs(): The previous value of the SDL variable \"{0}\" is {1}.".format(SDLVar, FoundValue.getInt()), level=kDebugDumpLevel)
                        state = FoundValue.getInt() % 10
                        if state != 0:
                            active = 1
                            if state == 2 or state == 3:
                                active = 0
                            try:
                                pageDefs += page % (active)
                            except LookupError:
                                pageDefs += "<pb><pb>Bogus page {0}".format(SDLVar)
            else:
                PtDebugPrint("xKI.IGetYeeshaPageDefs(): ERROR: Trying to access the Chronicle psnlSDL failed: psnlSDL = \"{0}\".".format(psnlSDL), level=kErrorLevel)
        else:
            PtDebugPrint("xKI.IGetYeeshaPageDefs(): ERROR: Trying to access the Vault failed, can't access YeeshaPageChanges Chronicle.", level=kErrorLevel)
        return pageDefs

    ## Turns on and off the Yeesha pages' SDL values.
    def IToggleYeeshaPageSDL(self, varName, on):
        vault = ptVault()
        if vault is not None:
            psnlSDL = vault.getPsnlAgeSDL()
            if psnlSDL:
                ypageSDL = psnlSDL.findVar(varName)
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
                    if value is not None:
                        PtDebugPrint("xKI.IToggleYeeshaPageSDL(): Setting {0} to {1}.".format(varName, value), level=kDebugDumpLevel)
                        ypageSDL.setInt((size * 10) + value)
                        vault.updatePsnlAgeSDL(psnlSDL)

    #############
    # Censoring #
    #############
    
    ## Sets the censor level.
    # By default, it's set at PG, but it fetches the real value from the
    # chronicle. If it is not found in the chronicle, it will set it to PG.
    def IDetermineCensorLevel(self):

        self.censorLevel = xCensor.xRatedPG
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleCensorLevel)
        if entry is None:
            vault.addChronicleEntry(kChronicleCensorLevel, kChronicleCensorLevelType, str(self.censorLevel))
        else:
            self.censorLevel = int(entry.chronicleGetValue())
        PtDebugPrint("xKI.IDetermineCensorLevel(): The censor level is %d." % (self.censorLevel), level=kWarningLevel)

    #########
    # Fonts #
    #########
    
    ## Sets the current font size from the Chronicle.
    def IDetermineFontSize(self):

        fontSize = self.IGetFontSize()
        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FontSize)
        if entry is None:
            # Not found, add the current size to the Chronicle.
            vault.addChronicleEntry(kChron.FontSize, kChron.FontSizeType, str(fontSize))
        else:
            fontSize = int(entry.chronicleGetValue())
            self.ISetFontSize(fontSize)
        PtDebugPrint("xKI.IDetermineFontSize(): The saved font size is %d." % (fontSize), level=kWarningLevel)

    ## Saves the current font size to the Chronicle.
    def ISaveFontSize(self):

        fontSize = self.IGetFontSize()
        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FontSize)
        if entry is not None:
            entry.chronicleSetValue(str(fontSize))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.FontSize, kChron.FontSizeType, str(fontSize))
        PtDebugPrint("xKI.ISaveFontSize(): Saving font size of %d." % (fontSize), level=kWarningLevel)

    ## Returns the font size currently applied to the KI.
    def IGetFontSize(self):

        if self.KILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        miniChatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        return miniChatArea.getFontSize()

    ## Applies the specified font size.
    def ISetFontSize(self, fontSize):

        PtDebugPrint("xKI.ISetFontSize(): Setting font size to {0}.".format(fontSize), level=kWarningLevel)
        if self.KILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        miniChatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        miniChatArea.setFontSize(fontSize)
        miniChatArea.refresh()
        microChatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        microChatArea.setFontSize(fontSize)
        microChatArea.refresh()
        noteArea = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNNote))
        noteArea.setFontSize(fontSize)
        noteArea.refresh()
        ownerNotes = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescription))
        ownerNotes.setFontSize(fontSize)
        ownerNotes.refresh()

    ## Changes the font size currently in effect.
    def IChangeFontSize(self, new):

        size = self.IGetFontSize()
        if new is 1:
            fontRange = range(len(kGUI.FontSizeList) - 1)
        elif new is -1:
            fontRange = range(len(FontSizeList) - 1, 0, -1)
        for i in fontRange:
            if size <= kGUI.FontSizeList[i] and new is 1:
                size = kGUI.FontSizeList[i + 1]
                break
            if size >= kGUI.FontSizeList[i] and new is -1:
                size = kGUI.FontSizeList[i - 1]
                break
        self.ISetFontSize(size)
        self.ISaveFontSize()
        self.IRefreshKISettings()

    ########
    # Chat #
    ########

    ## Checks for possible commands pre-pending a chat message.
    # Returns the appropriate message and performs all the necessary operations
    # to apply the command.
    def ICheckChatCommands(self, chatMessage):

        if chatMessage.lower().startswith(PtGetLocalizedString("KI.Commands.Unignore")):
            pid,msg = self.GetPIDMsg(chatMessage[len(PtGetLocalizedString("KI.Commands.Unignore")):])
            if pid:
                vault = ptVault()
                ignores = vault.getIgnoreListFolder()
                if type(ignores) != type(None):
                    if ignores.playerlistHasPlayer(pid):
                        ignores.playerlistRemovePlayer(pid)
                        self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
                    else:
                        self.chatMgr.AddChatLine(None,PtGetLocalizedString("KI.Player.NotFound"),kChat.SystemMessage)
            else:
                # check the ignore list to see if they are there by name
                vault = ptVault()
                ignores = vault.getIgnoreListFolder()
                if type(ignores) != type(None):
                    ignorerefs = ignores.getChildNodeRefList()
                    theName = string.lstrip(chatMessage[len(PtGetLocalizedString("KI.Commands.Unignore")):])
                    for plyr in ignorerefs:
                        if isinstance(plyr,ptVaultNodeRef):
                            PLR = plyr.getChild()
                            PLR = PLR.upcastToPlayerInfoNode()
                            # its an element.. should be a player
                            if type(PLR) != type(None) and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                if theName.startswith(PLR.playerGetName()):
                                    # found them
                                    ignores.playerlistRemovePlayer(PLR.playerGetID())
                                    self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
                                    return None
                self.chatMgr.AddChatLine(None,PtGetLocalizedString("KI.Player.NumberOnly"),kChat.SystemMessage)
            return None
        if chatMessage.lower().startswith(PtGetLocalizedString("KI.Commands.AutoShout")):
            self.autoShout = abs(self.autoShout - 1)
            if self.autoShout:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Messages.AutoShoutEnabled"), kChatBroadcastMsg)
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Messages.AutoShoutDisabled"), kChatBroadcastMsg)
            return None
        if chatMessage.lower().startswith(PtGetLocalizedString("KI.Commands.DumpLogs")):
            destination = chatMessage[len(PtGetLocalizedString("KI.Commands.DumpLogs")):]
            destination = destination.strip() # remove whitespace
            if destination == "":
                self.chatMgr.AddChatLine(None,PtGetLocalizedString("KI.Errors.MalformedLogDumpCmd"),kChat.SystemMessage)
                return None
            # we are using a timer here so that we can print out some last status messages to the log before
            # the log is dumped to it's new home
            PtDebugPrint("-- Logs dumped to: \"" + destination + "\" at " + time.strftime("%d %b %Y %H:%M:%S (GMT)", time.gmtime()))
            self.logDumpDest = destination # so the timer can get at it
            PtAtTimeCallback(self.key,0.25,kDumpLogsTimer)
            return None
        if chatMessage.lower().startswith(PtGetLocalizedString("KI.Commands.DumpLog")):
            # cause people are too damn lazy to type the s character
            destination = chatMessage[len(PtGetLocalizedString("KI.Commands.DumpLog")):]
            destination = destination.strip() # remove whitespace
            if destination == "":
                self.chatMgr.AddChatLine(None,PtGetLocalizedString("KI.Errors.MalformedLogDumpCmd"),kChat.SystemMessage)
                return None
            # we are using a timer here so that we can print out some last status messages to the log before
            # the log is dumped to it's new home
            PtDebugPrint("-- Logs dumped to: \"" + destination + "\" at " + time.strftime("%d %b %Y %H:%M:%S (GMT)", time.gmtime()))
            self.logDumpDest = destination # so the timer can get at it
            PtAtTimeCallback(self.key,0.25,kDumpLogsTimer)
            return None
        if chatMessage.lower().startswith(PtGetLocalizedString("KI.Commands.ChangePassword")):
            newpassword = chatMessage[len(PtGetLocalizedString("KI.Commands.ChangePassword")):].strip()
            if newpassword == "":
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.BadPassword"), kChat.SystemMessage)
                return None
            elif len(newpassword) > 15:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.PasswordTooLong"), kChat.SystemMessage)
                return None
            PtChangePassword(newpassword)
            return None
        if chatMessage.lower().startswith(PtGetLocalizedString("KI.Commands.SendFriendInvite")):
            commands = chatMessage[len(PtGetLocalizedString("KI.Commands.SendFriendInvite")):].strip().split(" ", 1)
            emailaddr = commands[0]
            toName = None

            if len(commands) == 2:
                toName = xCensor.xCensor(commands[1], self.censorLevel)
            if emailaddr == "":
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.MissingEmailAddress"), kChat.SystemMessage)
                return None
            elif len(emailaddr) > 63:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.EmailAddressTooLong"), kChat.SystemMessage)
                return None

            if toName and len(toName) > 0:
                PtSendFriendInvite(emailaddr, toName)
            else:
                PtSendFriendInvite(emailaddr)

            return None
        if chatMessage.lower().startswith(str("/savecolumns")) and PtGetAgeName() == "Jalak":
            fName = chatMessage[13:].strip()
            if fName:
                fName = fName + ".txt"
            else:
                fName = "JalakColumns.txt"
            self.SendNote("SaveColumns;" + fName)
            return None
        if chatMessage.lower().startswith(str("/loadcolumns")) and PtGetAgeName() == "Jalak":
            fName = chatMessage[13:].strip()
            if fName:
                fName = fName + ".txt"
            else:
                fName = "JalakColumns.txt"
            self.SendNote("LoadColumns;" + fName)
            return None
        if PtIsInternalRelease() and chatMessage == "/revisitcleft":
            # find the cleft chronicle and delete it
            vault = ptVault()
            chron = vault.findChronicleEntry("CleftSolved")
            if type(chron) != type(None):
                chronFolder = vault.getChronicleFolder()
                if type(chronFolder) != type(None):
                    chronFolder.removeNode(chron)
            return None
        if PtIsInternalRelease() and chatMessage == "/restart":
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
        if chatMessage == "/look":
            plist = self.chatMgr.GetPlayersInChatDistance(minPlayers=-1)
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
            self.chatMgr.AddChatLine(None,"%s:\n%s  Standing near you is %s.\n  There are exits to the %s."%(self.GetAgeName(),see,people,exits),0)
            return None
        if chatMessage.lower().startswith("/go ") or chatMessage == "/go":
            self.chatMgr.AddChatLine(None,"Put one foot in front of the other and eventually you will get there.",0)
            return None
        if chatMessage.lower().startswith("/get feather"):
            loc = self.IGetAgeFileName()
            if loc == "Gira":
                if self.gFeather < 7:
                    self.chatMgr.AddChatLine(None,"You pick up a plain feather and put it in your pocket. I know you didn't see yourself do that... trust me, you have a feather in your pocket.",0)
                    self.gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if entry is None:
                        vault.addChronicleEntry("feather",1,str(self.gFeather))
                    else:
                        entry.chronicleSetValue(str(self.gFeather))
                        entry.save()
                else:
                    self.chatMgr.AddChatLine(None,"You can only carry seven plain feathers.",0)
            elif loc == 'EderDelin':
                if self.gFeather == 7:
                    self.chatMgr.AddChatLine(None,"You search... and find the 'Red' feather and put it in your pocket.",0)
                    self.gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if entry is None:
                        vault.addChronicleEntry("feather",1,str(self.gFeather))
                    else:
                        entry.chronicleSetValue(str(self.gFeather))
                        entry.save()
                elif self.gFeather > 7:
                    self.chatMgr.AddChatLine(None,"You search... but find no other feathers.",0)
                else:
                    self.chatMgr.AddChatLine(None,"You search... but then suddenly stop when you realize that you are missing seven plain feathers.",0)
            elif loc == 'Dereno':
                if self.gFeather == 8:
                    self.chatMgr.AddChatLine(None,"You search... and find the 'Blue' feather and put it in your pocket.",0)
                    self.gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if entry is None:
                        vault.addChronicleEntry("feather",1,str(self.gFeather))
                    else:
                        entry.chronicleSetValue(str(self.gFeather))
                        entry.save()
                elif self.gFeather > 8:
                    self.chatMgr.AddChatLine(None,"You search... but find no other feathers.",0)
                else:
                    self.chatMgr.AddChatLine(None,"You search... but then suddenly stop when you realize that you are missing the 'Red' feather.",0)
            elif loc == 'Payiferen':
                if self.gFeather == 9:
                    self.chatMgr.AddChatLine(None,"You search... and find the 'Black' feather and put it in your pocket.",0)
                    self.gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if entry is None:
                        vault.addChronicleEntry("feather",1,str(self.gFeather))
                    else:
                        entry.chronicleSetValue(str(self.gFeather))
                        entry.save()
                elif self.gFeather > 9:
                    self.chatMgr.AddChatLine(None,"You search... but find no other feathers.",0)
                else:
                    self.chatMgr.AddChatLine(None,"You search... but then suddenly stop when you realize that you are missing the 'Blue' feather.",0)
            elif loc == 'Ercana':
                if self.gFeather == 10:
                    self.chatMgr.AddChatLine(None,"You search... and find the 'Silver' feather and put it in your pocket.",0)
                    self.gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if entry is None:
                        vault.addChronicleEntry("feather",1,str(self.gFeather))
                    else:
                        entry.chronicleSetValue(str(self.gFeather))
                        entry.save()
                elif self.gFeather > 10:
                    self.chatMgr.AddChatLine(None,"You search... but find no other feathers.",0)
                else:
                    self.chatMgr.AddChatLine(None,"You search... but then suddenly stop when you realize that you are missing the 'Black' feather.",0)
            elif loc == 'Jalak':
                if self.gFeather == 11:
                    self.chatMgr.AddChatLine(None,"You search... and find the 'Duck' feather and put it in your pocket.",0)
                    self.gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if entry is None:
                        vault.addChronicleEntry("feather",1,str(self.gFeather))
                    else:
                        entry.chronicleSetValue(str(self.gFeather))
                        entry.save()
                elif self.gFeather > 11:
                    self.chatMgr.AddChatLine(None,"You search... but find no other feathers.",0)
                else:
                    self.chatMgr.AddChatLine(None,"You search... but then suddenly stop when you realize that you are missing the 'Silver' feather.",0)
            elif loc == 'Ahnonay':
                if self.gFeather == 12:
                    self.chatMgr.AddChatLine(None,"You search... and find a large 'Rukh' feather (how could you have missed it?) and put it in your pocket.",0)
                    self.gFeather += 1
                    vault = ptVault()
                    entry = vault.findChronicleEntry("feather")
                    if entry is None:
                        vault.addChronicleEntry("feather",1,str(self.gFeather))
                    else:
                        entry.chronicleSetValue(str(self.gFeather))
                        entry.save()
                elif self.gFeather > 12:
                    self.chatMgr.AddChatLine(None,"You search... but find no other feathers.",0)
                else:
                    self.chatMgr.AddChatLine(None,"You search... but then suddenly stop when you realize that you are missing the 'Duck' feather.",0)
            else:
                self.chatMgr.AddChatLine(None,"There are no feathers here.",0)
            return None
        if chatMessage == "/look in pocket":
            if self.gFeather:
                if self.gFeather == 1:
                    self.chatMgr.AddChatLine(None,"You see a feather!",0)
                else:
                    pfeathers = self.gFeather
                    if pfeathers > 7:
                        pfeathers = 7
                    pOut = "You see %d plain feathers" % (pfeathers)
                    if self.gFeather>7:
                        pOut += " and a 'Red' feather"
                    if self.gFeather>8:
                        pOut += " and a 'Blue' feather"
                    if self.gFeather>9:
                        pOut += " and a 'Black' feather"
                    if self.gFeather>10:
                        pOut += " and a 'Silver' feather"
                    if self.gFeather>11:
                        pOut += " and a 'Duck' feather"
                    if self.gFeather>12:
                        pOut += " and a large 'Rukh' feather (sticking out of your pocket)"
                    pOut += "."
                    self.chatMgr.AddChatLine(None,pOut,0)
            else:
                self.chatMgr.AddChatLine(None,"There is nothing there but lint.",0)
            return None
        if chatMessage.lower().startswith("/fly"):
            self.chatMgr.AddChatLine(None,"You close your eyes, you feel light headed and the ground slips away from your feet... Then you open your eyes and WAKE UP! (Ha, you can only dream about flying.)",0)
            return None
        if chatMessage.lower().startswith("/get "):
            if chatMessage[-1:] == "s":
                v = "are"
            else:
                v = "is"
            self.chatMgr.AddChatLine(None,"The %s %s too heavy to lift. Maybe you should stick to feathers." % (chatMessage[len("/get "):],v),0)
            return None
        #
        # The check for emote commands should be last
        # search message for emote commmand (could embedd into message)
        if chatMessage.startswith('/'):
            # find the emote at the start of the message
            words = chatMessage.split()
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
                self.chatMgr.DisplayStatusMessage(statusMsg, 1)
                chatMessage = chatMessage[len(words[0]):]
                if chatMessage == "":
                    return None
                return chatMessage[1:]
            except LookupError:
                try:
                    command = xKIExtChatCommands.xChatExtendedChat[unicode(string.lower(words[0][1:]))]

                    if type(command) == type(""):
                        # see if there is text afterwords to be passed as arguments to the console command
                        args = chatMessage[len(words[0]):]
                        PtConsole(command+args)
                    else:
                        # must be a functions
                        try:
                            args = chatMessage[len(words[0])+1:]
                            if args != "":
                                try:
                                    retDisp = command(args)
                                except TypeError:
                                    retDisp = command()
                                    return args
                            else:
                                retDisp = command()
                            if (type(retDisp) == type(U"")) or (type(retDisp) == type("")):
                                self.chatMgr.DisplayStatusMessage(retDisp)
                            elif type(retDisp) == type(()):
                                if retDisp[0]:
                                    self.chatMgr.AddChatLine(None,retDisp[1],kChat.SystemMessage)
                                else:
                                    self.chatMgr.DisplayStatusMessage(retDisp[1])
                        except:
                            PtDebugPrint("xKI: chat command function did not run",command,level=kErrorLevel)
                    return None
                except LookupError:
                    # make sure that its not one of the special handled commands
                    if unicode(string.lower(words[0])) in xKIExtChatCommands.xChatSpecialHandledCommands:
                        # let the later processing handle these commands
                        return chatMessage
                    else:
                        # if they miss typed then error message
                        self.chatMgr.AddChatLine(None,PtGetLocalizedString("KI.Errors.CommandError", [chatMessage]),kChat.SystemMessage)

                    return None
        return chatMessage

    ##########
    # Fading #
    ##########
    
    ## Gets the current fading time from the Chronicle.
    def IDetermineFadeTime(self):

        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FadeTime)
        if entry is None:
            # Not found, add the current fade time to the Chronicle.
            vault.addChronicleEntry(kChron.FadeTime, kChron.FadeTimeType, str(self.chatMgr.ticksOnFull))
        else:
            self.chatMgr.ticksOnFull = int(entry.chronicleGetValue())
            if self.chatMgr.ticksOnFull == kChat.FadeTimeMax:
                # Disable the fade altogether.
                self.chatMgr.fadeEnableFlag = 0
                self.KillFadeTimer()
                PtDebugPrint("xKI.IDetermineFadeTime(): Fade time disabled.", level=kWarningLevel)
            else:
                self.chatMgr.fadeEnableFlag = 1
        PtDebugPrint("xKI.IDetermineFadeTime(): The saved fade time is %d." % (self.chatMgr.ticksOnFull), level=kWarningLevel)

    ## Saves the current fading time to the Chronicle.
    def ISaveFadeTime(self):

        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FadeTime)
        if type(entry) != type(None):
            entry.chronicleSetValue(str(self.chatMgr.ticksOnFull))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.FadeTime,kChron.FadeTimeType, str(self.chatMgr.ticksOnFull))
        PtDebugPrint("xKI: Saving Fade Time of %d" % (self.chatMgr.ticksOnFull),level=kWarningLevel)

    ## Start the fade timer.
    # This gets called each time the user does something in relation to the
    # chat to keep it alive.
    def StartFadeTimer(self):

        if not self.chatMgr.fadeEnableFlag:
            return
        if not BigKI.dialog.isEnabled():
            if self.chatMgr.fadeMode == kChat.FadeNotActive:
                PtAtTimeCallback(self.key, kChat.FullTickTime, kTimers.Fade)
            self.fadeMode = kChat.FadeFullDisp
            self.currentFadeTick = self.chatMgr.ticksOnFull

    ## End the currently-active timer.
    def KillFadeTimer(self):

        if self.KILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        if self.fadeMode != kChat.FadeNotActive:
            self.fadeMode = kChat.FadeStopping
        self.currentFadeTick = self.chatMgr.ticksOnFull
        mKIdialog.setForeColor(-1, -1, -1, self.originalForeAlpha)
        mKIdialog.setSelectColor(-1, -1, -1, self.originalSelectAlpha)
        if self.KILevel == kNormalKI:
            playerlist = ptGUIControlListBox(mKIdialog.getControlFromTag(kGUI.PlayerList))
            playerlist.show()
        chatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatArea.enableScrollControl()
        mKIdialog.refreshAllControls()

    ## Make the miniKI lists completely faded out.
    def FadeCompletely(self):

        if self.KILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        # If the BigKI is enabled, make the chat opaque once more.
        if BigKI.dialog.isEnabled():
            mKIdialog.setForeColor(-1, -1, -1, self.originalForeAlpha)
            mKIdialog.setSelectColor(-1, -1, -1, self.originalSelectAlpha)
            mKIdialog.refreshAllControls()
        # Otherwise, add full transparency and hide everything.
        else:
            mKIdialog.setForeColor(-1, -1, -1, 0)
            mKIdialog.setSelectColor(-1, -1, -1, 0)
            mKIdialog.refreshAllControls()
            if self.KILevel == kNormalKI:
                playerlist = ptGUIControlListBox(mKIdialog.getControlFromTag(kGUI.PlayerList))
                playerlist.hide()
            chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
            self.fadeMode = kChat.FadeNotActive

    ########
    # Ages #
    ########

    ## Determines whether or not the player can invite visitors to an Age.
    def ICanAgeInviteVistors(self, ageInfo, link):

        # Make sure it's not a special Age.
        try:
            for Age in kAges.NoInvite:
                if Age == ageInfo.getAgeFilname():
                    return False
        except AttributeError:
            pass

        # Make sure that the Age has not been deleted.
        if link.getVolatile():
            return False

        # Make sure the player has a default link to this Age.
        # If not, the Age has not yet been finished.
        spawnPoints = link.getSpawnPoints()
        for spawnlink in spawnPoints:
            if spawnlink.getTitle() == "Default":
                return True
        return False

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

    #############
    # Age Names #
    #############

    ## Returns the formatted and filtered name of an Age instance.
    def IGetAgeInstanceName(self, ageInfo=None):

        if ageInfo is None:
            ageInfo = PtGetAgeInfo()
        if ageInfo is not None:
            if ageInfo.getAgeInstanceName() == "D'ni-Rudenna":
                sdl = xPsnlVaultSDL()
                if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                    pass
                else:
                    return "Unknown"
            if ageInfo.getAgeInstanceName() == "Ae'gura":
                return "D'ni-Ae'gura"
            return self.FilterAgeName(ageInfo.getAgeInstanceName())
        else:
            return "?UNKNOWN?"

    ## Returns an Age's name the way a player should see it.
    # This displayed is used in the top-right corner of the BigKI. In contrast
    # to FilterAgeName, this requires an ageInfo to get the name.
    def GetAgeName(self, ageInfo=None):

        isSubAge = False
        ageVault = ptAgeVault()
        if ageInfo is None:
            ageInfo = ageVault.getAgeInfo()

        if ageInfo is None:
            return "?UNKNOWN?"
        
        isChildAge = False
        parent = ageInfo.getParentAgeLink()
        if parent:
            parentInfo = parent.getAgeInfo()
            if parentInfo:
                isChildAge = True

        if ageInfo.getAgeFilename() != "Neighborhood":
            subAges = ageVault.getSubAgesFolder()
            if subAges and subAges.getChildNodeCount() > 0:
                isSubAge = True

        if ageInfo.getAgeFilename() == "BahroCave":
            sdl = xPsnlVaultSDL()
            if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                return "D'ni-Rudenna"

        if ageInfo.getAgeInstanceName() == "Ae'gura" or ageInfo.getAgeFilename() == "city":
            if isChildAge:
                return "D'ni-Ae'gura'"
            return "D'ni-Ae'gura"

        if ageInfo.getAgeFilename() in kAges.Hide:
            return "Unknown"

        if ageInfo.getAgeFilename() in kAges.Display:
            return kAges.Display[ageInfo.getAgeFilename()]

        # Don't include the owner's name on D'ni locations.
        if ageInfo.getAgeInstanceName().startswith("D'ni"):
            return ageInfo.getAgeInstanceName()

        # For some reason it thinks Er'cana and Ahnonay are sub-Ages. Get their
        # display name instead.
        if (isChildAge or isSubAge) and not (ageInfo.getAgeFilename() == "Ercana" or ageInfo.getAgeFilename()[:7] == "Ahnonay"):
            localizeName = ageInfo.getAgeInstanceName()
            if isChildAge:
                localizeName += "'"
        else:
            localizeName = ageInfo.getDisplayName()

        return self.FilterAgeName(xLocTools.LocalizeAgeName(localizeName))

    ## Replace the Age's name as is appropriate.
    # It accepts as a parameter a name, unlike GetAgeName.
    def FilterAgeName(self, ageName):

        # Replace file names with display names - only once, from the right.
        # This fixes a bug in which avatars' names containing words like Garden
        # incorrectly get replaced.
        for Age, replacement in kAges.Replace.iteritems():
            ageNameList = ageName.rsplit(Age, 1)
            ageName = replacement.join(ageNameList)

        # Find the appropriate display name.
        if ageName == "GreatZero'":
            ageName = "D'ni-Rezeero'"
        elif ageName == "???" or ageName == "BahroCave":
            sdl = xPsnlVaultSDL()
            if sdl["TeledahnPoleState"][0] > 5 or sdl["KadishPoleState"][0] > 5 or sdl["GardenPoleState"][0] > 5 or sdl["GarrisonPoleState"][0] > 5:
                ageName = "D'ni-Rudenna"
            else:
                ageName = "Unknown"
        elif ageName in kAges.Hide:
            ageName = "Unknown"
        else:
            for Age in kAges.Display:
                if Age.lower() == ageName.lower():
                    ageName = kAges.Display[Age]
                    break

        ageName = ageName.replace("(null)", "").strip()
        return ageName

    ## Returns the file name of the specified Age.
    def IGetAgeFileName(self, ageInfo=None):

        if ageInfo is None:
            ageInfo = PtGetAgeInfo()
        if ageInfo is not None:
            return ageInfo.getAgeFilename()
        else:
            return "?UNKNOWN?"

    ################
    # Neighborhood #
    ################

    ## Find the player's neighborhood.
    def IGetNeighborhood(self):

        try:
            return ptVault().getLinkToMyNeighborhood().getAgeInfo()
        except AttributeError:
            PtDebugPrint("xKI.IGetNeighborhood(): Neighborhood not found.", level=kDebugDumpLevel)
            return None

    ## Find the player's neighbors.
    def IGetNeighbors(self):

        try:
            return self.IGetNeighborhood().getAgeOwnersFolder()
        except AttributeError:
            PtDebugPrint("xKI.IGetNeighbors(): List of neighbors not found.", level=kDebugDumpLevel)
            return None

    ##########
    # Limits #
    ##########

    ## Update the used-up space on the KI.
    def IUpdateKIUsage(self):

        usage = ptVault().getKIUsage()
        self.numberOfPictures = usage[0]
        self.numberOfNotes = usage[1]
        self.numberOfMarkerFolders = usage[2]
        try:
            self.numberOfMarkers = self.markerGameDisplay.gameData.data['numMarkers']
        except:
            self.numberOfMarkers = -1

    ## Check if the player has reached his limit of picture space.
    def ICanTakePicture(self):

        self.IUpdateKIUsage()
        if kLimits.MaxPictures == -1 or self.numberOfPictures < kLimits.MaxPictures:
            return True
        return False

    ## Check if the player has reached his limit of journal notes space.
    def ICanMakeNote(self):

        self.IUpdateKIUsage()
        if kLimits.MaxNotes == -1 or self.numberOfNotes < kLimits.MaxNotes:
            return True
        return False

    ## Check if the player has reached his limit of Marker Games.
    def ICanMakeMarkerFolder(self):

        self.IUpdateKIUsage()
        if kLimits.MaxMarkerFolders == -1 or self.numberOfMarkerFolders < kLimits.MaxMarkerFolders:
            return True
        return False

    ## Check if the player has reached his limit of markers for a Marker Game.
    def ICanMakeMarker(self):

        self.IUpdateKIUsage()
        if kLimits.MaxMarkers == -1 or self.numberOfMarkers < kLimits.MaxMarkers:
            return True
        return False

    ##########
    # Errors #
    ##########

    ## Displays a OK dialog-based error message to the player.
    def IShowKIFullErrorMsg(self, msg):

        self.YNWhatReason = kGUI.YNKIFull
        reasonField = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
        reasonField.setStringW(msg)
        yesButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesButtonID))
        yesButton.hide()
        yesBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesButtonTextID))
        yesBtnText.hide()
        noBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.NoButtonTextID))
        noBtnText.setStringW(PtGetLocalizedString("KI.YesNoDialog.OKButton"))
        KIYesNo.dialog.show()

    #####################
    # GUI Notifications #
    #####################

    ## Process notifications originating from the Blackbar.
    # This handles objects like the Yeesha book icon, the quitting icon, the
    # options menu icon and the miniKI icon, when the KI has been obtained.
    def ProcessNotifyBlackbar(self, control, event):

        if event == kDialogLoaded:
            pass
        elif event == kAction or event == kValueChanged:
            bbID = control.getTagID()
            if bbID == kGUI.MiniMaximizeRGID:
                if control.getValue() == 0:
                    if PtIsDialogLoaded("KIMini"):
                        KIMini.dialog.show()
                elif control.getValue() == -1:
                    if PtIsDialogLoaded("KIMini"):
                        KIMini.dialog.hide()
            elif bbID == kGUI.ExitButtonID:
                yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
                yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
                self.ILocalizeDialog(0)
                logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutTextID))
                logoutText.show()
                logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutButtonID))
                logoutButton.show()
                KIYesNo.dialog.show()
            elif bbID == kGUI.PlayerBookCBID:
                if control.isChecked():
                    curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                    if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                        if not self.waitingForAnimation:
                            self.IShowYeeshaBook()
                        else:
                            control.setChecked(0)
                    else:
                        control.setChecked(0)
            else:
                PtDebugPrint("xKI.ProcessNotifyBlackbar(): Don't know this control bbID = %d" % (bbID), level=kDebugDumpLevel)
        elif event == kInterestingEvent:
            plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
            try:
                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                    PtDebugPrint("xKI.ProcessNotifyBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint("xKI.ProcessNotifyBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.hide()
            except NameError:
                if self.isEntireYeeshaBookEnabled:
                    PtDebugPrint("xKI.ProcessNotifyBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint("xKI.ProcessNotifyBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.hide()

    ## Process notifications originating from the microKI's Blackbar.
    # This takes care of the same duties at the Blackbar processor, with the
    # sole difference that it is used only while the user does not yet possess
    # a full KI from Gahreesen (thus there is no miniKI icon).
    def ProcessNotifyMicroBlackbar(self, control, event):

        if event == kDialogLoaded:
            rollBtn = ptGUIControlButton(KIMicroBlackbar.dialog.getControlFromTag(kGUI.RolloverLeftID))
            rollBtn.setNotifyOnInteresting(1)
            rollBtn = ptGUIControlButton(KIMicroBlackbar.dialog.getControlFromTag(kGUI.RolloverRightID))
            rollBtn.setNotifyOnInteresting(1)
        elif event == kAction or event == kValueChanged:
            bbID = control.getTagID()
            if bbID == kGUI.ExitButtonID:
                yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
                yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
                self.ILocalizeDialog(0)
                logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutTextID))
                logoutText.show()
                logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutButtonID))
                logoutButton.show()
                KIYesNo.dialog.show()
            elif bbID == kGUI.PlayerBookCBID:
                if control.isChecked():
                    curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                    if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                        if not self.waitingForAnimation:
                            self.IShowYeeshaBook()
                        else:
                            control.setChecked(0)
                    else:
                        control.setChecked(0)
            else:
                PtDebugPrint("xKI.ProcessNotifyMicroBlackbar(): Don't know this control bbID = %d" % (bbID), level=kDebugDumpLevel)
        elif event == kInterestingEvent:
            plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
            try:
                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                    PtDebugPrint("xKI.ProcessNotifyMicroBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint("xKI.ProcessNotifyMicroBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.hide()
            except NameError:
                if self.isEntireYeeshaBookEnabled:
                    PtDebugPrint("xKI.ProcessNotifyMicroBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint("xKI.ProcessNotifyMicroBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.hide()

    ## Process notifications originating from the microKI.
    # These notifications get called when using the basic chat mode available
    # only until the user obtains a real KI from Gahreesen (thus the name,
    # microKI).
    def ProcessNotifyMicro(self, control, event):

        if event == kDialogLoaded:
            # Fill in the listbox so that the test is near the enter box.
            chatarea = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatarea.lock()         # Make the chat display immutable.
            chatarea.unclickable()  # Make the chat display non-clickable.
            chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            chatarea.disableScrollControl()
            btnUp = ptGUIControlButton(KIMicro.dialog.getControlFromTag(kGUI.miniChatScrollUp))
            btnUp.show()
            btnUp.hide()
            # Set the edit box buffer size to something larger.
            chatedit = ptGUIControlEditBox(KIMicro.dialog.getControlFromTag(kGUI.ChatEditboxID))
            chatedit.setStringSize(500)
            chatedit.setChatMode(1)
        elif event == kShowHide:
            if control.isEnabled():
                if not self.chatMgr.isChatting:
                    self.FadeCompletely()
        elif event == kAction or event == kValueChanged:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                if not control.wasEscaped() and control.getString() != "":
                    self.chatMgr.SendMessage(control.getString())
                self.chatMgr.ToggleChatMode(0)
            elif ctrlID == kGUI.ChatDisplayArea:
                self.KillFadeTimer()
                self.StartFadeTimer()
        elif event == kFocusChange:
            # If they are chatting, get the focus back.
            if self.chatMgr.isChatting:
                KIMicro.dialog.setFocus(KIMicro.dialog.getControlFromTag(kGUI.ChatEditboxID))
        elif event == kSpecialAction:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                self.IAutocomplete(control)

    ## Process notifications originating from the miniKI.
    # The miniKI is the display in the top-left corner of the screen (by
    # default); these notifications are triggered through interaction with the
    # various buttons on it. It also takes care of the floating player list.
    def ProcessNotifyMini(self, control, event):

        if event == kDialogLoaded:
            # Get the original position of the miniKI.
            dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
            self.originalminiKICenter = dragbar.getObjectCenter()
            # Retreive the original alpha.
            fore = control.getForeColor()
            self.originalForeAlpha = fore.getAlpha()
            sel = control.getSelectColor()
            self.originalSelectAlpha = sel.getAlpha()
            # Fill in the listbox so that the test is near the enter box.
            chatarea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatarea.lock()         # Make the chat display immutable.
            chatarea.unclickable()  # Make the chat display non-clickable.
            chatarea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            # Hide the chat scroll buttons (should be nothing in chat area yet anyhow).
            chatarea.disableScrollControl()
            btnUp = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniChatScrollUp))
            btnUp.show()
            privateChbox = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniPrivateToggle))
            privateChbox.disable()
            # Set the edit box buffer size to something larger.
            chatedit = ptGUIControlEditBox(KIMini.dialog.getControlFromTag(kGUI.ChatEditboxID))
            chatedit.setStringSize(500)
            chatedit.setChatMode(1)
            # Default the marker tag stuff to be off.
            btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZDrip))
            btnmt.hide()
            btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZActive))
            btnmt.hide()
            btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZMarkerGameActive))
            btnmt.hide()
            btnmt = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZMarkerInRange))
            btnmt.hide()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGNewMarker)).hide()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGNewGame)).hide()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGInactive)).hide()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGInactive)).disable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).hide()
            # Set the color to the off color.
            for mcbid in range(kGUI.miniMarkerIndicator01, kGUI.miniMarkerIndicatorLast + 1):
                mcb = ptGUIControlProgress(KIMini.dialog.getControlFromTag(mcbid))
                mcb.setValue(kGUI.miniMarkerColors['off'])
        elif event == kShowHide:
            if control.isEnabled():
                if self.pelletImager != "":
                    ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).show()
                if self.miniKIFirstTimeShow:
                    # Set the font size and fade time.
                    self.IDetermineFontSize()
                    self.IDetermineFadeTime()
                    # If we are chatting then just let it happen.
                    if not self.chatMgr.isChatting:
                        self.chatMgr.ToggleChatMode(0)
                        self.FadeCompletely()
                    self.miniKIFirstTimeShow = False
                self.IRefreshPlayerList()
                self.IRefreshPlayerListDisplay()
                self.IRefreshMiniKIMarkerDisplay()
            else:
                self.chatMgr.ToggleChatMode(0)
                self.chatMgr.ClearBBMini()
        elif event == kAction or event == kValueChanged:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                if not control.wasEscaped() and control.getString() != "":
                    self.chatMgr.SendMessage(control.getString())
                self.chatMgr.ToggleChatMode(0)
                self.StartFadeTimer()
            elif ctrlID == kGUI.PlayerList:
                # Make sure they don't click outside what's there.
                plyrsel = control.getSelection()
                if plyrsel >= control.getNumElements():
                    control.setSelection(0)
                    plyrsel = 0
                # Place selected player in SendTo textbox.
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                caret = ptGUIControlTextBox(KIMini.dialog.getControlFromTag(kGUI.ChatCaretID))
                caret.setString(">")
                privateChbox = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniPrivateToggle))
                privateChbox.setChecked(0)
                if plyrsel == -1:
                    self.BKPlayerSelected = None
                else:
                    self.BKPlayerSelected = self.BKPlayerList[plyrsel]
                    # Is it a Device Folder or just a string?
                    if isinstance(self.BKPlayerSelected, DeviceFolder) or isinstance(self.BKPlayerSelected, str):
                        # Can't be sent to.
                        pass
                    elif isinstance(self.BKPlayerSelected, Device):
                        sendToField.setString(self.BKPlayerSelected.name)
                    # Is it a specific player info node?
                    elif isinstance(self.BKPlayerSelected, ptVaultNodeRef):
                        plyrInfoNode = self.BKPlayerSelected.getChild()
                        plyrInfo = plyrInfoNode.upcastToPlayerInfoNode()
                        if plyrInfo is not None:
                            sendToField.setString(plyrInfo.playerGetName())
                            # Set private caret.
                            caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt") + unicode(plyrInfo.playerGetName()) + U" >")
                            privateChbox.setChecked(1)
                        else:
                            self.BKPlayerSelected = None
                    # Is it a specific player?
                    elif isinstance(self.BKPlayerSelected, ptPlayer):
                        sendToField.setString(self.BKPlayerSelected.getPlayerName())
                        caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt") + unicode(self.BKPlayerSelected.getPlayerName()) + U" >")
                        privateChbox.setChecked(1)
                    # Is it a list of players?
                    elif isinstance(self.BKPlayerSelected, ptVaultPlayerInfoListNode):
                        fldrType = self.BKPlayerSelected.folderGetType()
                        # Is it not the All Players folder?
                        if fldrType != PtVaultStandardNodes.kAgeMembersFolder:
                            # If it's a list of age owners, it's a list of neighbors.
                            if fldrType == PtVaultStandardNodes.kAgeOwnersFolder:
                                fldrType = PtVaultStandardNodes.kHoodMembersFolder
                            caret.setStringW(PtGetLocalizedString("KI.Chat.TOPrompt") + xLocTools.FolderIDToFolderName(fldrType) + U" >")
                        # It's not private and no player is selected.
                        privateChbox.setChecked(0)
                        self.BKPlayerSelected = None
                if self.BKPlayerSelected is None:
                    sendToField.setString(" ")
                self.IBigKISetToButtons()
                # No need to keep the focus.
                if self.chatMgr.isChatting:
                    chatedit = ptGUIControlEditBox(KIMini.dialog.getControlFromTag(kGUI.ChatEditboxID))
                    KIMini.dialog.setFocus(chatedit.getKey())
                # They're playing with the player list, so kill the fade timer.
                self.KillFadeTimer()
                self.StartFadeTimer()
            elif ctrlID == kGUI.miniPutAwayID:
                self.IminiPutAwayKI()
            elif ctrlID == kGUI.miniToggleBtnID:
                self.IminiToggleKISize()
            elif ctrlID == kGUI.miniTakePicture:
                self.IminiTakePicture()
            elif ctrlID == kGUI.miniCreateJournal:
                self.IminiCreateJournal()
            elif ctrlID == kGUI.miniMuteAll:
                # Hit the mute button, and set mute depending on control.
                audio = ptAudioControl()
                if control.isChecked():
                    audio.muteAll()
                else:
                    audio.unmuteAll()
            elif ctrlID == kGUI.miniPlayerListUp:
                # Scroll the player list up one line.
                playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
                self.IScrollUpListbox(playerlist, kGUI.miniPlayerListUp, kGUI.miniPlayerListDown)
            elif ctrlID == kGUI.miniPlayerListDown:
                # Scroll the player list down one line.
                playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
                self.IScrollDownListbox(playerlist, kGUI.miniPlayerListUp, kGUI.miniPlayerListDown)
            elif ctrlID == kGUI.miniGZMarkerInRange:
                self.ICaptureGZMarker()
                self.IRefreshMiniKIMarkerDisplay()
            elif ctrlID == kGUI.ChatDisplayArea:
                self.KillFadeTimer()
                self.StartFadeTimer()
            elif ctrlID == kGUI.miniMGNewMarker:
                self.ICreateAMarker()
            elif ctrlID == kGUI.miniMGNewGame:
                self.ICreateMarkerGame()
            elif ctrlID == kJalakMiniIconBtn:
                if PtGetAgeName() == "Jalak":
                    self.JalakGUIToggle()
                else:
                    ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            elif ctrlID == kGUI.PelletScoreButton:
                self.IUploadPelletScore()
        elif event == kFocusChange:
            # If they are chatting, get the focus back.
            if self.chatMgr.isChatting:
                # If the bigKI is up then let the focus go where it wants.
                # Otherwise put the focus back to the chat line.
                if not BigKI.dialog.isEnabled():
                    KIMini.dialog.setFocus(KIMini.dialog.getControlFromTag(kGUI.ChatEditboxID))
                else:
                    if not self.BKInEditMode:
                        KIMini.dialog.setFocus(KIMini.dialog.getControlFromTag(kGUI.ChatEditboxID))
        elif event == kSpecialAction:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                self.IAutocomplete(control)

    ## Process notifications originating from the BigKI itself.
    # This does not process notifications specific to an expanded view - each
    # view gets its own function, to avoid bloat. This rather deals with
    # controls such as the scroll button, the mode switcher, etc., anything
    # related to the navigation interface.
    def ProcessNotifyBigKI(self, control, event):

        if event == kDialogLoaded:
            self.BKInEditMode = False
            # Put animation at off position, so there is no pop when the animation plays.
            KIOnAnim.animation.skipToTime(1.5)
            pdisable = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKDisabledPeopleButton))
            pdisable.disable()
            gdisable = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKDisabledGearButton))
            gdisable.disable()
            self.IBigKISetNotifyForLong()
        elif event == kShowHide:
            if control.isEnabled():
                self.IBigKIHideLongFolderNames()
                self.IBigKISetStatics()
                self.IBigKISetChanging()
                self.IRefreshPlayerList()
                self.IRefreshPlayerListDisplay()
                self.KillFadeTimer()
                self.IBigKIRefreshFolders()
                self.IBigKIRefreshFolderDisplay()
                self.IBigKIShowBigKI()
            else:
                self.StartFadeTimer()
        elif event == kAction or event == kValueChanged:
            bkID = control.getTagID()
            # Is it one of the folder buttons?
            if bkID >= kGUI.BKIIncomingBtn and bkID <= kGUI.BKIFolderLineBtnLast:
                if self.BKFolderLineDict is self.BKConfigFolderDict:
                    self.BKFolderSelected = bkID - kGUI.BKIIncomingBtn + self.BKFolderTopLine
                    self.IShowSelectedConfig()
                else:
                    oldselect = self.BKFolderSelected
                    self.BKFolderSelected = bkID - kGUI.BKIIncomingBtn + self.BKFolderTopLine
                    if oldselect != self.BKFolderSelected:
                        self.BKFolderSelectChanged = True
                    else:
                        self.BKFolderSelectChanged = False
                    self.IBigKIChangeMode(kGUI.BKListMode)
            # Is it the scroll folder up button?
            elif bkID == kGUI.BKFolderUpLine:
                if self.BKFolderTopLine > 0:
                    self.BKFolderTopLine -= 1
                    self.IBigKIRefreshFolderDisplay()
                    self.IBigKISetToButtons()
            # Is it the scroll folder down button?
            elif bkID == kGUI.BKFolderDownLine:
                self.BKFolderTopLine += 1
                self.IBigKIRefreshFolderDisplay()
                self.IBigKISetToButtons()
            elif bkID == kGUI.BKLMUpButton:
                if self.BKRightSideMode == kGUI.BKListMode:
                    if self.BKContentListTopLine > 0:
                        self.BKContentListTopLine -= kContentListScrollSize
                        if self.BKContentListTopLine < 0:
                            self.BKContentListTopLine = 0
                        self.IBigKIRefreshContentListDisplay()
            elif bkID == kGUI.BKLMDownButton:
                if self.BKRightSideMode == kGUI.BKListMode:
                    self.BKContentListTopLine += kContentListScrollSize
                    self.IBigKIRefreshContentListDisplay()
            elif bkID >= kGUI.BKIToIncomingButton and bkID <= kGUI.BKIToFolderButtonLast:
                toFolderNum = bkID - kGUI.BKIToFolderButton01 + self.BKFolderTopLine + 1
                # If they are in an expanded mode, then they can move the element to another folder.
                if self.BKRightSideMode != kGUI.BKListMode and self.BKCurrentContent is not None:
                    # Move the current content to the selected folder.
                    if isinstance(self.BKCurrentContent, ptPlayer):
                        # Add to new folder.
                        try:
                            newFolderName = self.BKFolderListOrder[toFolderNum]
                            newFolder = self.BKFolderLineDict[newFolderName]
                            playerID = self.BKCurrentContent.getPlayerID()
                            localPlayerID = PtGetLocalPlayer().getPlayerID()
                            if newFolder is not None and playerID != localPlayerID:
                                if newFolder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                                    self.IInviteToVisit(playerID, newFolder)
                                elif newFolder.getType() == PtVaultNodeTypes.kPlayerInfoListNode:
                                    newFolder.playerlistAddPlayer(playerID)
                        except (IndexError, KeyError):
                            # If there was an error, display whatever was already selected.
                            toFolderNum = self.BKFolderSelected
                    else:
                        oldFolder = self.BKCurrentContent.getParent()
                        theElement = self.BKCurrentContent.getChild()
                        if theElement is not None:
                            # Add to new folder.
                            try:
                                newFolderName = self.BKFolderListOrder[toFolderNum]
                                newFolder = self.BKFolderLineDict[newFolderName]
                                localPlayerID = PtGetLocalPlayer().getPlayerID()
                                if newFolder is not None:
                                    if newFolder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                                        theElement = theElement.upcastToPlayerInfoNode()
                                        if theElement is not None and theElement.playerGetID() != localPlayerID:
                                            self.IInviteToVisit(theElement.playerGetID(), newFolder)
                                    elif newFolder.getType() == PtVaultNodeTypes.kPlayerInfoListNode:
                                        theElement = theElement.upcastToPlayerInfoNode()
                                        if theElement is not None and theElement.playerGetID() != localPlayerID:
                                            theElement = theElement.upcastToPlayerInfoNode()
                                            newFolder.playerlistAddPlayer(theElement.playerGetID())
                                    else:
                                        self.BKCurrentContent = newFolder.addNode(theElement)
                                        if oldFolder is not None:
                                            oldFolder.removeNode(theElement)
                            except (IndexError, KeyError):
                                # If there was an error, display whatever was already selected.
                                toFolderNum = self.BKFolderSelected
                    # Leave it at the folder they are on.
                    self.BKFolderSelectChanged = True
                    self.IBigKIChangeMode(kGUI.BKListMode)
                    # They could have copied a player, so refresh list.
                    self.IRefreshPlayerList()
                    self.IRefreshPlayerListDisplay()
            elif bkID == kGUI.BKRadioModeID:
                # Save the previous selected and top line.
                if self.BKFolderLineDict is self.BKJournalFolderDict:
                    self.BKJournalFolderSelected = self.BKFolderSelected
                    self.BKJournalFolderTopLine = self.BKFolderTopLine
                elif self.BKFolderLineDict is self.BKPlayerFolderDict:
                    self.BKPlayerFolderSelected = self.BKFolderSelected
                    self.BKPlayerFolderTopLine = self.BKFolderTopLine
                elif self.BKFolderLineDict is self.BKConfigFolderDict:
                    self.BKConfigFolderSelected = self.BKFolderSelected
                    self.BKConfigFolderTopLine = self.BKFolderTopLine
                modeselect = control.getValue()
                # Is it journal mode?
                if modeselect == 0:
                    self.BKFolderLineDict = self.BKJournalFolderDict
                    self.BKFolderListOrder = self.BKJournalListOrder
                    self.BKFolderSelected = self.BKJournalFolderSelected
                    self.BKFolderTopLine = self.BKJournalFolderTopLine
                # Is it player list mode?
                elif modeselect == 1:
                    self.BKFolderLineDict = self.BKPlayerFolderDict
                    self.BKFolderListOrder = self.BKPlayerListOrder
                    self.BKFolderSelected = self.BKPlayerFolderSelected
                    self.BKFolderTopLine = self.BKPlayerFolderTopLine
                # It is configuration mode.
                else:
                    self.BKFolderLineDict = self.BKConfigFolderDict
                    self.BKFolderListOrder = self.BKConfigListOrder
                    self.BKFolderSelected = self.BKConfigFolderSelected
                    self.BKFolderTopLine = self.BKConfigFolderTopLine
                # Reset the top line and selection.
                self.IBigKIRefreshFolderDisplay()
                if modeselect == 0 and (self.BKRightSideMode == kGUI.BKPictureExpanded or self.BKRightSideMode == kGUI.BKJournalExpanded or self.BKRightSideMode == kGUI.BKMarkerListExpanded):
                    # The player is taking a picture.
                    self.IBigKIInvertToFolderButtons()
                else:
                    # Is the player switching to configuration mode?
                    if modeselect == 2:
                        self.IShowSelectedConfig()
                    # Otherwise, make sure the player is in list mode.
                    else:
                        self.IBigKIChangeMode(kGUI.BKListMode)
            elif bkID == kGUI.BKIToPlayerButton:
                if self.BKCurrentContent is not None and self.BKPlayerSelected is not None:
                    sendElement = self.BKCurrentContent.getChild()
                    toPlayerBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
                    if sendElement is not None:
                        if isinstance(self.BKPlayerSelected, DeviceFolder):
                            pass
                        elif isinstance(self.BKPlayerSelected, Device):
                            if self.BKPlayerSelected.name in self.imagerMap:
                                notify = ptNotify(self.key)
                                notify.clearReceivers()
                                notify.addReceiver(self.imagerMap[self.BKPlayerSelected.name])
                                notify.netPropagate(1)
                                notify.netForce(1)
                                notify.setActivate(1.0)
                                sName = "Upload=%s" % (self.BKPlayerSelected.name)
                                notify.addVarNumber(sName, sendElement.getID())
                                notify.send()
                            toPlayerBtn.hide()
                        elif isinstance(self.BKPlayerSelected, ptVaultNode):
                            if self.BKPlayerSelected.getType() == PtVaultNodeTypes.kPlayerInfoListNode:
                                plyrRefList = self.BKPlayerSelected.getChildNodeRefList()
                                for plyrRef in plyrRefList:
                                    plyr = plyrRef.getChild()
                                    plyr = plyr.upcastToPlayerInfoNode()
                                    if plyr is not None:
                                        sendElement.sendTo(plyr.playerGetID())
                            elif self.BKPlayerSelected.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                sendElement.sendTo(self.BKPlayerSelected.playerGetID())
                            else:
                                self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.CantSend"))
                            toPlayerBtn.hide()
                        elif isinstance(self.BKPlayerSelected, ptVaultNodeRef):
                            plyrElement = self.BKPlayerSelected.getChild()
                            if plyrElement is not None and plyrElement.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                plyrElement = plyrElement.upcastToPlayerInfoNode()
                                sendElement.sendTo(plyrElement.playerGetID())
                            else:
                                self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.PlayerNotFound"))
                            toPlayerBtn.hide()
                        elif isinstance(self.BKPlayerSelected, ptPlayer):
                            sendElement.sendTo(self.BKPlayerSelected.getPlayerID())
                            toPlayerBtn.hide()
                        else:
                            self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.UnknownPlayerType"))
                    else:
                        self.ISetPlayerNotFound(PtGetLocalizedString("KI.Errors.BadJournalElement"))
        elif event == kInterestingEvent:
            if control is not None:
                shortTB = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(control.getTagID() + 21))
                longTB = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(control.getTagID() + 521))
                if shortTB.getStringJustify() == kRightJustify and control.isInteresting():
                    # Switch to long versions.
                    longTB.setForeColor(shortTB.getForeColor())
                    longTB.setString(shortTB.getString())
                    shortTB.hide()
                    longTB.show()
                else:
                    shortTB.show()
                    longTB.hide()

    ## Process notifications originating from a list mode in the BigKI.
    # Handles navigation from a list mode (like a list of players or notes) to
    # the specified content (a picture, a note, a marker game...). Also takes
    # care of creating a new entry when asked.
    def ProcessNotifyListMode(self, control, event):

        if event == kAction or event == kValueChanged:
            lmID = control.getTagID()
            if lmID >= kGUI.BKIListModeLineBtn01 and lmID <= kGUI.BKIListModeLineBtnLast:
                # Find out which button was clicked and its associated content.
                whichOne = lmID - kGUI.BKIListModeLineBtn01 + self.BKContentListTopLine
                if whichOne < len(self.BKContentList):
                    theContent = self.BKContentList[whichOne]
                    if theContent is not None:
                        self.BKCurrentContent = theContent
                        if isinstance(self.BKCurrentContent, QuestionNote):
                            nextMode = kGUI.BKQuestionNote
                            self.IBigKIChangeMode(nextMode)
                        elif isinstance(self.BKCurrentContent, ptPlayer):
                            nextMode = kGUI.BKPlayerExpanded
                            self.IBigKIChangeMode(nextMode)
                        else:
                            theElement = theContent.getChild()
                            if theElement is not None:
                                dataType = theElement.getType()
                                if dataType == PtVaultNodeTypes.kTextNoteNode:
                                    nextMode = kGUI.BKJournalExpanded
                                elif dataType == PtVaultNodeTypes.kImageNode:
                                    nextMode = kGUI.BKPictureExpanded
                                elif dataType == PtVaultNodeTypes.kPlayerInfoNode:
                                    nextMode = kGUI.BKPlayerExpanded
                                elif dataType == PtVaultNodeTypes.kMarkerGameNode:
                                    nextMode = kGUI.BKMarkerListExpanded
                                else:
                                    self.BKCurrentContent = None
                                    nextMode = kGUI.BKListMode
                                self.IBigKIChangeMode(nextMode)
                            else:
                                PtDebugPrint("xKI.ProcessNotifyListMode(): List Mode: content is None for element!", level=kErrorLevel)
            elif lmID == kGUI.BKIListModeCreateBtn:
                if self.BKFolderLineDict is self.BKPlayerFolderDict:
                    self.BKGettingPlayerID = True
                    self.IBigKIChangeMode(kGUI.BKPlayerExpanded)
                else:
                    self.IBigKICreateJournalNote()
                    self.IBigKIChangeMode(kGUI.BKJournalExpanded)
                    self.IBigKIDisplayCurrentContentJournal()
                    self.IBigKIEnterEditMode(kGUI.BKEditFieldJRNTitle)

    ## Process notifications originating from an expanded picture mode in the BigKI.
    # This essentially deals with the taking of new pictures and the editing of
    # existing ones, as well as their deletion.
    def ProcessNotifyPictureExpanded(self, control, event):

        if event == kDialogLoaded:
            editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[kGUI.BKEditFieldPICTitle][kGUI.BKEditIDeditbox]))
            editbox.hide()
        elif event == kShowHide:
            if control.isEnabled():
                self.IBigKIDisplayCurrentContentImage()
        elif event == kAction or event == kValueChanged:
            peID = control.getTagID()
            if peID == kGUI.BKIPICTitleButton:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.IBigKIEnterEditMode(kGUI.BKEditFieldPICTitle)
            elif peID == kGUI.BKIPICDeleteButton:
                self.YNWhatReason = kGUI.YNDelete
                elem = self.BKCurrentContent.getChild()
                elem = elem.upcastToImageNode()
                if elem is not None:
                    picTitle = elem.imageGetTitle()
                else:
                    picTitle = "<unknown>"
                yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
                yesText.setStringW(PtGetLocalizedString("KI.Messages.DeletePicture", [xCensor.xCensor(picTitle, self.censorLevel)]))
                self.ILocalizeDialog(1)
                KIYesNo.dialog.show()
            elif peID == kGUI.BKIPICTitleEdit:
                self.IBigKISaveEdit(1)
        elif event == kFocusChange:
            if self.IsContentMutable(self.BKCurrentContent):
                self.IBigKICheckFocusChange()

    ## Process notifications originating from an expanded journal mode in the BigKI.
    # Handles note creation, editing and deletion.
    def ProcessNotifyJournalExpanded(self, control, event):

        if event == kDialogLoaded:
            editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[kGUI.BKEditFieldJRNTitle][kGUI.BKEditIDeditbox]))
            editbox.hide()
        elif event == kShowHide:
            if control.isEnabled():
                self.IBigKIDisplayCurrentContentJournal()
        elif event == kAction or event == kValueChanged:
            jeID = control.getTagID()
            # Is it one of the buttons?
            if jeID == kGUI.BKIJRNTitleButton:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.IBigKIEnterEditMode(kGUI.BKEditFieldJRNTitle)
            elif jeID == kGUI.BKIJRNNoteButton:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.IBigKIEnterEditMode(kGUI.BKEditFieldJRNNote)
            elif jeID == kGUI.BKIJRNDeleteButton:
                self.YNWhatReason = kGUI.YNDelete
                elem = self.BKCurrentContent.getChild()
                elem = elem.upcastToTextNoteNode()
                if elem is not None:
                    jrnTitle = elem.noteGetTitle()
                else:
                    jrnTitle = "<unknown>"
                yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
                yesText.setStringW(PtGetLocalizedString("KI.Messages.DeleteJournal", [xCensor.xCensor(jrnTitle, self.censorLevel)]))
                self.ILocalizeDialog(1)
                KIYesNo.dialog.show()
            # Is it one of the editing boxes?
            elif jeID == kGUI.BKIJRNTitleEdit or jeID == kGUI.BKIJRNNoteEdit:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.IBigKISaveEdit(1)
        elif event == kFocusChange:
            if self.IsContentMutable(self.BKCurrentContent):
                if control is not None:
                    # If the focus is changing to the multiline, the plaer is entering edit mode.
                    jeID = control.getTagID()
                    if jeID == kGUI.BKIJRNNote:
                        self.IBigKIEnterEditMode(kGUI.BKEditFieldJRNNote)
                        return
                self.IBigKICheckFocusChange()

    ## Process notifications originating from an expanded player mode in the BigKI.
    # Handles deletion of a player's entry.
    def ProcessNotifyPlayerExpanded(self, control, event):

        if event == kShowHide:
            if control.isEnabled():
                self.IBigKIDisplayCurrentContentPlayer()
        elif event == kAction or event == kValueChanged:
            plID = control.getTagID()
            # Is it one of the buttons?
            if plID == kGUI.BKIPLYDeleteButton:
                self.YNWhatReason = kGUI.YNDelete
                elem = self.BKCurrentContent.getChild()
                elem = elem.upcastToPlayerInfoNode()
                if elem is not None:
                    plyrName = elem.playerGetName()
                else:
                    plyrName = "<unknown>"
                try:
                    pfldName = self.BKFolderListOrder[self.BKFolderSelected]
                except LookupError:
                    pfldName = "<unknown>"
                yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
                yesText.setStringW(PtGetLocalizedString("KI.Messages.DeletePlayer", [xCensor.xCensor(plyrName, self.censorLevel), pfldName]))
                self.ILocalizeDialog(1)
                KIYesNo.dialog.show()
            elif plID == kGUI.BKIPLYPlayerIDEditBox:
                self.IBigKICheckSavePlayer()
        elif event == kFocusChange:
            if self.BKGettingPlayerID:
                if KIPlayerExpanded.dialog.isEnabled():
                    plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYPlayerIDEditBox))
                    plyIDedit.focus()
                    KIPlayerExpanded.dialog.setFocus(plyIDedit.getKey())
                else:
                    self.BKGettingPlayerID = False
                    self.IBigKIChangeMode(kGUI.BKListMode)

    ## Process notifications originating from an expanded settings mode in the BigKI.
    # Handles the processing tied to settings modification.
    def ProcessNotifySettingsExpanded(self, control, event):

        if event == kShowHide:
            if control.isEnabled():
                tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kGUI.BKIKISettingsText))
                tfield.setStringW(PtGetLocalizedString("KI.Config.Settings"))
                tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kGUI.BKIKIFontSizeText))
                tfield.setStringW(PtGetLocalizedString("KI.Config.FontSize"))
                tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kGUI.BKIKIFadeTimeText))
                tfield.setStringW(PtGetLocalizedString("KI.Config.ChatFadeTime"))
                tfield = ptGUIControlTextBox(KISettings.dialog.getControlFromTag(kGUI.BKIKIOnlyPMText))
                tfield.setStringW(PtGetLocalizedString("KI.Config.OnlyBuddies"))
                self.IRefreshKISettings()
            else:
                self.ISaveFontSize()
                self.ISaveFadeTime()
                self.ISaveKIFlags()
        elif event == kAction or event == kValueChanged:
            kiID = control.getTagID()
            if kiID == kGUI.BKIKIFontSize:
                slidePerFont = float(control.getMax() - control.getMin() + 1.0) / float(len(kGUI.FontSizeList))
                fontIndex = int(control.getValue() / slidePerFont + 0.25)
                if fontIndex >= len(FontSizeList):
                    fontIndex = len(FontSizeList) - 1
                self.ISetFontSize(kGUI.FontSizeList[fontIndex])
            elif kiID == kGUI.BKIKIFadeTime:
                slidePerTime = float(control.getMax() - control.getMin()) / float(kChat.FadeTimeMax)
                self.chatMgr.ticksOnFull = int(control.getValue() / slidePerTime + 0.25)
                PtDebugPrint("xKI.ProcessNotifySettingsExpanded(): FadeTime set to %d." % (self.chatMgr.ticksOnFull), level=kDebugDumpLevel)
                if self.chatMgr.ticksOnFull == kChat.FadeTimeMax:
                    self.chatMgr.fadeEnableFlag = 0
                    PtDebugPrint("KISettings: FadeTime disabled.", level=kDebugDumpLevel)
                else:
                    self.chatMgr.fadeEnableFlag = 1
                    PtDebugPrint("KISettings: FadeTime enabled.", level=kDebugDumpLevel)
            elif kiID == kGUI.BKIKIOnlyPM:
                self.onlyGetPMsFromBuddies = control.isChecked()
            elif kiID == kGUI.BKIKIBuddyCheck:
                self.onlyAllowBuddiesOnRequest = control.isChecked()

    ## Process notifications originating from an expanded settings mode in the BigKI.
    # Handles the sound controls from the options menu being modified.
    def ProcessNotifyVolumeExpanded(self, control, event):

        if event == kShowHide:
            if control.isEnabled():
                self.IRefreshVolumeSettings()
        elif event == kAction or event == kValueChanged:
            plID = control.getTagID()
            audio = ptAudioControl()
            if plID == kGUI.BKISoundFXVolSlider:
                setting = control.getValue()
                PtDebugPrint("xKI.ProcessNotifyVolumeExpanded(): SoundFX being changed to %g (into %g)." % (setting, setting / 10), level=kDebugDumpLevel)
                audio.setSoundFXVolume(setting / 10)
            elif plID == kGUI.BKIMusicVolSlider:
                setting = control.getValue()
                PtDebugPrint("xKI.ProcessNotifyVolumeExpanded(): Music being changed to %g (into %g)." % (setting, setting / 10), level=kDebugDumpLevel)
                audio.setMusicVolume(setting / 10)
            elif plID == kGUI.BKIVoiceVolSlider:
                setting = control.getValue()
                PtDebugPrint("xKI.ProcessNotifyVolumeExpanded(): Voice being changed to %g (into %g)." % (setting, setting / 10), level=kDebugDumpLevel)
                audio.setVoiceVolume(setting / 10)
            elif plID == kGUI.BKIAmbienceVolSlider:
                setting = control.getValue()
                PtDebugPrint("xKI.ProcessNotifyVolumeExpanded(): Ambience being changed to %g (into %g)." % (setting, setting / 10), level=kDebugDumpLevel)
                audio.setAmbienceVolume(setting / 10)
            elif plID == kGUI.BKIMicLevelSlider:
                setting = control.getValue()
                PtDebugPrint("xKI.ProcessNotifyVolumeExpanded(): MicLevel being changed to %g (into %g)." % (setting, setting / 10), level=kDebugDumpLevel)
                audio.setMicLevel(setting / 10)
            elif plID == kGUI.BKIGUIVolSlider:
                setting = control.getValue()
                PtDebugPrint("xKI.ProcessNotifyVolumeExpanded(): MicLevel being changed to %g (into %g)." % (setting, setting / 10), level=kDebugDumpLevel)
                audio.setGUIVolume(setting / 10)

    ## Process notifications originating from an expanded Age Owner mode in the BigKI.
    # Processes owned Ages (currently only applies to Neighborhoods). Note that
    # the public/private feature is currently available only through the Nexus.
    # This mostly handles description modifications for now.
    def ProcessNotifyAgeOwnerExpanded(self, control, event):

        if event == kShowHide:
            if control.isEnabled():
                tField = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescriptionTitle))
                tField.setStringW(PtGetLocalizedString("KI.Config.Description"))
                titleEdit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleEditbox))
                titleEdit.hide()
                self.IRefreshAgeOwnerSettings()
        elif event == kAction or event == kValueChanged:
            plID = control.getTagID()
            if plID == kGUI.BKAgeOwnerMakePublicBtn:
                # This feature is not currently available in the BigKI.
                try:
                    vault = ptVault()
                    myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
                    myAgeStruct = myAge.asAgeInfoStruct()
                    makePublic = 1
                    if myAge.isPublic():
                        makePublic = 0
                        PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Making %s private." % (myAge.getDisplayName()), level=kDebugDumpLevel)
                    else:
                        PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Making %s public." % (myAge.getDisplayName()), level=kDebugDumpLevel)
                    vault.setAgePublic(myAgeStruct, makePublic)
                    # Let the refresh re-enable the public button.
                    control.disable()
                except AttributeError:
                    PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): ERROR: Couldn't toggle public/private.", level=kErrorLevel)
            elif plID == kGUI.BKAgeOwnerTitleBtn:
                PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Change title button hit.", level=kDebugDumpLevel)
                control.disable()
                title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleTB))
                title.hide()
                titleedit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleEditbox))
                try:
                    # Get the selected Age config setting.
                    myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
                    titleEdit.setString(myAge.getAgeUserDefinedName())
                except LookupError:
                    titleEdit.setString("")
                titleEdit.show()
                titleEdit.end()
                KIAgeOwnerExpanded.dialog.setFocus(titleEdit.getKey())
            elif plID == kGUI.BKAgeOwnerTitleEditbox:
                PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): edit field set.", level=kDebugDumpLevel)
                self.ISaveUserNameFromEdit(control)
        elif event == kFocusChange:
            PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Focus change.", level=kDebugDumpLevel)
            titleEdit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleEditbox))
            if titleEdit.isVisible():
                if control is None or (control.getTagID() != kGUI.BKAgeOwnerTitleEditbox and control.getTagID() != kGUI.BKAgeOwnerTitleBtn):
                    self.ISaveUserNameFromEdit(titleEdit)
            if control is not None:
                # Check if the decription was updated.
                plID = control.getTagID()
                if plID == kGUI.BKAgeOwnerDescription:
                    self.BKAgeOwnerEditDescription = True
                    PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Start editing description.", level=kDebugDumpLevel)
                else:
                    if self.BKAgeOwnerEditDescription:
                        deScript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescription))
                        myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
                        if myAge is not None:
                            PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Age description updated for %s." % (myAge.getDisplayName()), level=kDebugDumpLevel)
                            myAge.setAgeDescription(deScript.getString())
                            myAge.save()
                        else:
                            PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Neighborhood is None while trying to update description.", level=kDebugDumpLevel)
                    self.BKAgeOwnerEditDescription = False
            else:
                if self.BKAgeOwnerEditDescription:
                    deScript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescription))
                    myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
                    if myAge is not None:
                        PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Age description updated for %s." % (myAge.getDisplayName()), level=kDebugDumpLevel)
                        buff = descript.getEncodedBuffer()
                        myAge.setAgeDescription(str(buff))
                        myAge.save()
                    else:
                        PtDebugPrint("xKI.ProcessNotifyAgeOwnerExpanded(): Neighborhood is None while trying to update description.", level=kDebugDumpLevel)
                self.BKAgeOwnerEditDescription = False

    ## Process notifications originating from a YesNo dialog.
    # Yes/No dialogs are omnipresent throughout Uru. Those processed here are:
    # - Quitting dialog (quit/logout/cancel).
    # - Deleting dialog (yes/no); various such dialogs.
    # - Link offer dialog (yes/no).
    # - Outside sender dialog (?).
    # - KI Full dialog (OK); just a notification.
    def ProcessNotifyYesNo(self, control, event):

        if event == kAction or event == kValueChanged:
            ynID = control.getTagID()
            if self.YNWhatReason == kGUI.YNQuit:
                if ynID == kGUI.YesButtonID:
                    PtConsole("App.Quit")
                elif ynID == kGUI.NoButtonID:
                    KIYesNo.dialog.hide()
                    logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutTextID))
                    logoutText.hide()
                    logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutButtonID))
                    logoutButton.hide()
                elif ynID == kGUI.YesNoLogoutButtonID:
                    KIYesNo.dialog.hide()
                    logoutText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutTextID))
                    logoutText.hide()
                    logoutButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesNoLogoutButtonID))
                    logoutButton.hide()

                    # Clear out all chat on microKI.
                    chatarea = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kGUI.ChatDisplayArea))
                    chatarea.setString("")
                    chatarea.moveCursor(PtGUIMultiLineDirection.kBufferStart)
                    KIMicro.dialog.refreshAllControls()

                    # Clear out all chat on miniKI.
                    chatarea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
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

            elif self.YNWhatReason == kYNDelete:
                if ynID == kGUI.YesButtonID:
                    # Remove the current element
                    if self.BKCurrentContent is not None:
                        delFolder = self.BKCurrentContent.getParent()
                        delElem = self.BKCurrentContent.getChild()
                        if delFolder is not None and delElem is not None:
                            # Are we removing a visitor from an Age we own?
                            tFolder = delFolder.upcastToFolderNode()
                            if tFolder is not None and tFolder.folderGetType() == PtVaultStandardNodes.kCanVisitFolder:
                                PtDebugPrint("xKI.ProcessNotifyYesNo(): Revoking visitor.", level=kDebugDumpLevel)
                                delElem = delElem.upcastToPlayerInfoNode()
                                # Need to refind the folder that has the ageInfo in it.
                                ageFolderName = self.BKFolderListOrder[self.BKFolderSelected]
                                ageFolder = self.BKFolderLineDict[ageFolderName]
                                self.IRevokeToVisit(delElem.playerGetID(), ageFolder)
                            # Are we removing a player from a player list?
                            elif delFolder.getType() == PtVaultNodeTypes.kPlayerInfoListNode and delElem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                PtDebugPrint("xKI.ProcessNotifyYesNo(): Removing player from folder.", level=kDebugDumpLevel)
                                delFolder = delFolder.upcastToPlayerInfoListNode()
                                delElem = delElem.upcastToPlayerInfoNode()
                                delFolder.playerlistRemovePlayer(delElem.playerGetID())
                                self.BKPlayerSelected = None
                                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                                sendToField.setString(" ")
                            # Are we removing a journal entry?
                            else:
                                # See if this is a Marker Game folder that is being deleted.
                                if delElem.getType() == PtVaultNodeTypes.kMarkerGameNode:
                                    # Delete all markers from the Marker Manager display.
                                    mrkrDisplay = ptMarkerMgr()
                                    if not self.markerGameManager.gameLoaded():
                                        mrkrDisplay.removeAllMarkers()
                                    # Delete the game.
                                    if self.markerGameDisplay is None:
                                        PtDebugPrint("xKI.ProcessNotifyYesNo(): ERROR: Cannot delete Marker Game as it is not loaded.")
                                        return
                                    self.markerGameDisplay.deleteGame()
                                    # Reset the game in case it was being played.
                                    if self.markerGameDisplay.gameData.data['svrGameTemplateID'] == self.markerGameManager.gameData.data['svrGameTemplateID']:
                                        self.markerGameManager.deleteGame()

                                self.BKCurrentContent = None
                                delFolder.removeNode(delElem)
                                PtDebugPrint("xKI.ProcessNotifyYesNo(): Deleting element from folder.", level=kDebugDumpLevel)
                        else:
                            PtDebugPrint("xKI.ProcessNotifyYesNo(): ERROR: Tried to delete bad Vault node or delete from bad folder.", level=kErrorLevel)
                        self.IBigKIChangeMode(kGUI.BKListMode)
                        self.IRefreshPlayerList()
                        self.IRefreshPlayerListDisplay()
                self.YNWhatReason = kGUI.YNQuit
                KIYesNo.dialog.hide()
            elif self.YNWhatReason == kGUI.YNOfferLink:
                self.YNWhatReason = kGUI.YNQuit
                KIYesNo.dialog.hide()
                if ynID == kGUI.YesButtonID:
                    if self.offerLinkFromWho is not None:
                        PtDebugPrint("xKI.ProcessNotifyYesNo(): Linking to offered age %s." % (self.offerLinkFromWho.getDisplayName()), level=kDebugDumpLevel)
                        link = ptAgeLinkStruct()
                        link.setLinkingRules(PtLinkingRules.kBasicLink)
                        link.setAgeInfo(self.offerLinkFromWho)
                        ptNetLinkingMgr().linkToAge(link)
                        self.offerLinkFromWho = None
                self.offerLinkFromWho = None
            elif self.YNWhatReason == kGUI.YNOutside:
                self.YNWhatReason = kGUI.YNQuit
                KIYesNo.dialog.hide()
                if self.YNOutsideSender is not None:
                    note = ptNotify(self.key)
                    note.clearReceivers()
                    note.addReceiver(self.YNOutsideSender)
                    note.netPropagate(0)
                    note.netForce(0)
                    # Is it a good return?
                    if ynID == kGUI.YesButtonID:
                        note.setActivate(1)
                        note.addVarNumber("YesNo", 1)
                    # Or a bad return?
                    elif ynID == kNoButtonID:
                        note.setActivate(0)
                        note.addVarNumber("YesNo", 0)
                    note.send()
                self.YNOutsideSender = None
            elif self.YNWhatReason == kGUI.YNKIFull:
                KIYesNo.dialog.hide()
                yesButton = ptGUIControlButton(KIYesNo.dialog.getControlFromTag(kGUI.YesButtonID))
                yesButton.show()
                yesBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesButtonTextID))
                yesBtnText.show()
                noBtnText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.NoButtonTextID))
                noBtnText.setStringW(PtGetLocalizedString("KI.YesNoDialog.NOButton"))
                self.YNWhatReason = kYNQuit
            else:
                self.YNWhatReason = kYNQuit
                KIYesNo.dialog.hide()
                self.YNOutsideSender = None
        elif event == kExitMode:
            self.YNWhatReason = kGUI.YNQuit
            KIYesNo.dialog.hide()
            self.YNOutsideSender = None

    ## Process notifications originating from a new item alert dialog.
    # Such alerts make either the KI's icon or the Yeesha Book icon
    # flash for a while.
    def ProcessNotifyNewItemAlert(self, control, event):
        if event == kDialogLoaded:
            kiAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
            kiAlert.disable()
            kiAlert.hide()
            bookalert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
            bookalert.disable()
            bookalert.hide()
        elif event == kShowHide:
            if control.isEnabled():
                self.IAlertStartTimer()

    ## Process notifications originating from the Marker Game creation GUI.
    # This gets the values submitted by the player and passes them to the
    # Marker Game manager.
    def ProcessNotifyCreateMarkerGameGUI(self, control, event):
        if control:
            tagID = control.getTagID()
        if event == kDialogLoaded:
            self.MarkerGameDefaultColor = ptGUIControlTextBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kGUI.MarkerGameLabel1)).getForeColor()
            self.MarkerGameSelectedColor = ptGUIControlTextBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kGUI.MarkerGameLabel1)).getSelectColor()
        elif event == kShowHide:
            self.InitMarkerGameGUI()
            PtDebugPrint("xKI.ProcessNotifyCreateMarkerGameGUI(): Marker Game dialog is showing or hiding.", level=kDebugDumpLevel)
        elif event == kAction or event == kValueChanged:
            if tagID == kGUI.MarkerGameType1 or tagID == kGUI.MarkerGameType2 or tagID == kGUI.MarkerGameType3:
                self.SelectMarkerType(tagID)
            elif tagID == kGUI.CreateMarkerGameCancelBT:
                KIMarkerGameGUIClose.run(self.key, netPropagate=0)
            elif kGUI.CreateMarkerGameSubmitBT:
                markerGameNameText = ptGUIControlEditBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kGUI.CreateMarkerGameNameEB)).getString()
                markerGameName = xCensor.xCensor(markerGameNameText, xCensor.xRatedPG)
                try:
                    markerGameType = kGUI.MarkerGameStates[self.SelectedMGType]
                except:
                    markerGameType = 0
                    PtDebugPrint("xKI.ProcessNotifyCreateMarkerGameGUI(): Couldn't find marker game type, so setting it to Quest Mode.", level=kWarningLevel)
                # Create the marker game display and wait for a return KI
                # message; upon receipt, IFinishCreateMarkerFolder() will be
                # called.
                self.markerGameDisplay = xMarkerGameKIDisplay(self, "", markerGameType, markerGameName)
                KIMarkerGameGUIClose.run(self.key, netPropagate=0)

    ## Processes notifications originating from an expanded Marker Game mode in the BigKI.
    # This handles the edit buttons, marker saving buttons, deletion buttons,
    # etc..
    def ProcessNotifyMarkerFolderExpanded(self, control, event):

        # Display a loading message just in case.
        self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionLoading")

        if event == kDialogLoaded:
            typeField = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderGameTimeTB))
            typeField.setString(kLoc.MarkerFolderPopupMenu[self.markerGameTimeID][0])
        elif event == kShowHide:
            # Reset the edit text lines.
            if control.isEnabled():
                titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleEB))
                titleEdit.hide()
                markerEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextEB))
                markerEdit.hide()
                self.IBigKIDisplayCurrentContentMarkerFolder()
            else:
                if self.markerGameDisplay is not None and self.markerGameDisplay.gameData is not None:
                    # Possibly still adding markers, check if in edit mode before destroying the game.
                    if not self.markerGameDisplay.showMarkers:
                        self.markerGameDisplay = None
        elif event == kAction or event == kValueChanged:
            mFldrID = control.getTagID()
            if mFldrID == kGUI.MarkerFolderEditStartGame:
                # Is it the "Edit" button?
                if MFdialogMode == kGUI.MFOverview:
                    if self.markerGameDisplay is None:
                        PtDebugPrint("xKI.ProcessNotifyMarkerFolderExpanded(): ERROR: Cannot locate the game, aborting edit game request.")
                        return
                    self.IShowMarkerGameLoading()
                    self.markerGameDisplay.editMarkers()
                    self.ISetWorkingToCurrentMarkerFolder()
                # Is it the "Done Editing" button?
                elif MFdialogMode == kGUI.MFEditing:
                    self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionSaving")
                    self.IShowMarkerGameLoading()
                    self.markerGameDisplay.exitEditMarkers()
                    self.IResetWorkingMarkerFolder()
                # Is it the "Stop Game" button?
                elif MFdialogMode == kGUI.MFPlaying:
                    if self.markerGameManager.gameLoaded():
                        self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionEndingGame")
                        self.IShowMarkerGameLoading()
                        self.markerGameManager.stopGame()
                # Is it the "Save Marker" button?
                elif MFdialogMode == kGUI.MFEditingMarker:
                    # Should already be saved, just clear selection for now.
                    self.markerGameDisplay.setSelectedMarker(-1)
                    self.IBKCheckContentRefresh(self.BKCurrentContent)
            elif mFldrID == kGUI.MarkerFolderPlayEndGame:
                # Is it the "Play Game" button?
                if MFdialogMode == kGUI.MFOverview:
                    self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionPrepareGame")
                    self.IShowMarkerGameLoading()
                    self.markerGameManager = MarkerGameManager(self, self.markerGameDisplay)
                # Is it the "Add Marker" button?
                elif MFdialogMode == kGUI.MFEditing:
                    self.ICreateAMarker()
                # Is it the "End Game" button?
                elif MFdialogMode == kGUI.MFPlaying:
                    self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionResetGame")
                    self.IShowMarkerGameLoading()
                    self.markerGameManager.resetGame()
                # Is it the "Remove Marker" button?
                elif MFdialogMode == kGUI.MFEditingMarker:
                    self.markerGameDisplay.deleteSelectedMarker()

            elif mFldrID == kGUI.MarkerFolderMarkListbox:
                if self.markerGameDisplay is not None:
                    markerlistSelectable = True
                    if self.markerGameDisplay.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameQuest:
                        # If playing, then the marker list is not selectable.
                        if self.markerGameManager.gameData.data['svrGameTemplateID'] == self.markerGameDisplay.gameData.data['svrGameTemplateID']:
                            markerlistSelectable = False
                    if markerlistSelectable:
                        markerSel = control.getSelection()
                        self.markerGameDisplay.setSelectedMarker(markerSel)
                        self.IBKCheckContentRefresh(self.BKCurrentContent)

            elif mFldrID == kGUI.MarkerFolderTitleBtn:
                control.disable()
                title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleText))
                titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleEB))
                titleEdit.setString(title.getString())
                title.hide()
                titleEdit.show()
                titleEdit.end()
                KIMarkerFolderExpanded.dialog.setFocus(titleEdit.getKey())
            elif mFldrID == kGUI.MarkerFolderTitleEB:
                self.ISaveMarkerFolderNameFromEdit(control)
            elif mFldrID == kGUI.MarkerFolderMarkerTextBtn:
                control.disable()
                title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextTB))
                titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextEB))
                titleEdit.setString(title.getString())
                title.hide()
                titleEdit.show()
                titleEdit.end()
                KIMarkerFolderExpanded.dialog.setFocus(titleEdit.getKey())
            elif mFldrID == kGUI.MarkerFolderMarkerTextEB:
                self.pendingMGmessage = kMessageWait.changeMarkerName
                self.ISaveMarkerTextFromEdit(control)
            elif mFldrID == kGUI.MarkerFolderTimePullDownBtn or mFldrID == kGUI.MarkerFolderTimeArrow:
                KIMarkerFolderPopupMenu.menu.show()
            elif mFldrID == kGUI.MarkerFolderTypePullDownBtn or mFldrID == kGUI.MarkerFolderTypeArrow:
                KIMarkerTypePopupMenu.menu.show()
            elif mFldrID == kGUI.MarkerFolderDeleteBtn:
                self.YNWhatReason = kGUI.YNDelete
                elem = self.BKCurrentContent.getChild()
                elem = elem.upcastToMarkerGameNode()
                if elem is not None:
                    mfTitle = elem.getGameName()
                else:
                    mfTitle = "<unknown>"
                yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
                yesText.setStringW(PtGetLocalizedString("KI.Messages.DeletePicture", [xCensor.xCensor(mfTitle, self.censorLevel)]))
                self.ILocalizeDialog(1)
                KIYesNo.dialog.show()
        elif event == kFocusChange:
            titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleEB))
            # Is the editbox enabled and something other than the button is getting the focus?
            if titleEdit.isVisible():
                if control is None or (control.getTagID() != kGUI.MarkerFolderTitleEB and control.getTagID() != kGUI.MarkerFolderTitleBtn):
                    self.ISaveMarkerFolderNameFromEdit(titleEdit)
            if MFdialogMode == kGUI.MFEditingMarker:
                titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextEB))
                # Is the editbox enabled and something other than the button is getting the focus?
                if titleEdit.isVisible():
                    if control is None or (control.getTagID() != kGUI.MarkerFolderMarkerTextEB and control.getTagID() != kGUI.MarkerFolderMarkerTextBtn):
                        self.ISaveMarkerTextFromEdit(titleEdit)

    ## Processes notifications originating from the Marker Game popup menu.
    # (What is this? Is it used?)
    def ProcessNotifyMarkerFolderPopupMenu(self, control, event):
        if event == kDialogLoaded:
            for menuItem in kLoc.MarkerFolderPopupMenu:
                KIMarkerFolderPopupMenu.menu.addNotifyItem(menuItem[0])
        elif event == kAction:
            menuID = control.getTagID()
            self.markerGameTimeID = menuID
            typeField = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderGameTimeTB))
            typeField.setString(kLoc.MarkerFolderPopupMenu[self.markerGameTimeID][0])
            # Save the current marker folder to this type of game.
            if self.BKCurrentContent is not None:
                element = self.BKCurrentContent.getChild()
                if element is not None:
                    datatype = element.getType()
                    if datatype == PtVaultNodeTypes.kMarkerGameNode:
                        element = element.upcastToMarkerGameNode()
                        if element:
                            element.setRoundLength(kLoc.MarkerFolderPopupMenu[self.markerGameTimeID][1])
                            element.save()
        elif event == kExitMode:
            if KIMarkerFolderPopupMenu.menu.isEnabled():
                KIMarkerFolderPopupMenu.menu.hide()
            elif KIMarkerTypePopupMenu.menu.isEnabled():
                KIMarkerTypePopupMenu.menu.hide()

    ## Processes notifications originating from a question note.
    # This is the name Uru uses for interactive text notes (???).
    def ProcessNotifyQuestionNote(self, control, event):
        if event == kShowHide:
            if control.isEnabled():
                self.IBigKIDisplayCurrentQuestionNote()
        elif event == kAction or event == kValueChanged:
            qnID = control.getTagID()
            # Is it one of two buttons?
            if qnID == kQNAcceptBtn:
                try:
                    self.BKCurrentContent.YesAction()
                    self.IBKCheckFolderRefresh(ptVault().getInbox())
                    self.IBigKIChangeMode(kGUI.BKListMode)
                except AttributeError:
                    pass
            elif qnID == kQNDeclineBtn:
                try:
                    self.BKCurrentContent.NoAction()
                    self.IBKCheckFolderRefresh(ptVault().getInbox())
                    self.IBigKIChangeMode(kGUI.BKListMode)
                except AttributeError:
                    pass

    ## Processes notifications originating from the Jalak GUI.
    # These controls are only used within the Age of Jalak, obviously.
    def ProcessNotifyJalakGUI(self, control, event):
        if event == kDialogLoaded:
            self.JalakGUIInit()
        elif event == kAction or event == kValueChanged:
            if control is not None:
                tagID = control.getTagID()
                btn = str(tagID)
                if btn in JalakBtnStates:
                    KIJalakBtnLights.run(self.key, state=btn, netPropagate=0)
                    self.SetJalakGUIButtons(0)

    #####################
    # Vault Type Events #
    #####################
    
    ## Handles the passed vault type event.
    # This is used to react to saved nodes, new nodes, etc.
    def HandleVaultTypeEvents(self, event, tupData):
        # Make sure that the BigKI dialog is loaded before trying to update it.
        if not PtIsDialogLoaded("KIMain"):
            PtDebugPrint("xKI.HandleVaultTypeEvents(): BigKI dialog was not loaded, waiting.", level=kDebugDumpLevel)
            return
        if event == PtVaultCallbackTypes.kVaultConnected:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): Connected to the Vault.", level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultDisconnected:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): Disconnected from the Vault.", level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultNodeSaved:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): A node is being saved (id = %d, type = %d)." % (tupData[0].getID(), tupData[0].getType()), level=kDebugDumpLevel)
            if tupData[0].getType() == PtVaultNodeTypes.kPlayerInfoNode:
                self.IRefreshPlayerList()
                self.IRefreshPlayerListDisplay()
            elif tupData[0].getType() == PtVaultNodeTypes.kAgeInfoNode:
                self.IBigKISetStatics()
                self.IBigKIRefreshFolders()
                self.IBigKIOnlySelectedToButtons()
                self.IRefreshAgeOwnerSettings()
            self.IBigKIRefreshContentList()
            self.IBigKIRefreshContentListDisplay()
        elif event == PtVaultCallbackTypes.kVaultNodeInitialized:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): A node has been initalized (id = %d, type = %d)." % (tupData[0].getID(), tupData[0].getType()), level=kDebugDumpLevel)
            if self.KILevel > kMicroKI:
                node = tupData[0]
                self.IBKCheckElementRefresh(tupData[0])
        elif event == PtVaultCallbackTypes.kVaultNodeAdded:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): A node has been added.", level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultNodeRefAdded:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): A node reference has been added (childID = %d, parentID = %d)." % (tupData[0].getChildID(), tupData[0].getParentID()), level=kDebugDumpLevel)
            if self.KILevel > kMicroKI:
                folder = tupData[0].getParent()
                folder = folder.upcastToFolderNode()
                # If the parent of this ref is the Inbox, then it's incoming mail.
                if folder is not None and folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
                    self.IAlertKIStart()
                    # Note: beenSeen() is not yet implemented.
                    if not tupData[0].beenSeen():
                        if self.onlyGetPMsFromBuddies:
                            vault = ptVault()
                            inbox = vault.getInbox()
                            buddies = vault.getBuddyListFolder()
                            if buddies.playerlistHasPlayer(tupData[0].getSaverID()):
                                # then show alert
                                self.IAlertKIStart()
                        else:
                            self.IAlertKIStart()

                child = tupData[0].getChild()
                child = child.upcastToFolderNode()
                if child is not None:
                    PtDebugPrint("xKI.HandleVaultTypeEvents(): Adding a folder, refresh folder list.", level=kDebugDumpLevel)
                    self.IBigKIRefreshFolders()
                self.IBKCheckFolderRefresh(folder)
        elif event == PtVaultCallbackTypes.kVaultRemovingNodeRef:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): A node reference is being removed (childID = %d, parentID = %d)." % (tupData[0].getChildID(), tupData[0].getParentID()), level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultNodeRefRemoved:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): A node reference has been removed (childID, parentID): ", tupData, level=kDebugDumpLevel)
            if self.KILevel > kMicroKI:
                if self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                    self.IBigKIDisplayCurrentContentMarkerFolder()
                self.IBKCheckFolderRefresh()
        elif event == PtVaultCallbackTypes.kVaultOperationFailed:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): A Vault operation failed (operation, resultCode): ", tupData, level=kDebugDumpLevel)
        else:
            PtDebugPrint("xKI.HandleVaultTypeEvents(): Unknown Vault event: %d." % (event), level=kWarningLevel)


    def IScrollUpListbox(self,control,upbtnID,downbtnID):
        "Try to scroll the listbox up one line"
        if self.KILevel == kMicroKI:
            return
        elif self.KILevel == kMicroKI:
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
        self.KillFadeTimer()
        self.StartFadeTimer()

    def IScrollDownListbox(self,control,upbtnID,downbtnID):
        "Try to scroll the listbox down one line"
        if self.KILevel == kMicroKI:
            return
        elif self.KILevel == kMicroKI:
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
        self.KillFadeTimer()
        self.StartFadeTimer()

    def ICheckScrollButtons(self,control,upbtnID,downbtnID):
        "Check to see if the scroll buttons should be visible or not"
        if self.KILevel == kMicroKI:
            return
        elif self.KILevel == kMicroKI:
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

        PtDebugPrint("xBigKI: refresh playerlist",level=kDebugDumpLevel)
        playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        select = playerlist.getSelection()
        if select >= 0 and select < len(self.BKPlayerList):
            self.previouslySelectedPlayer = self.BKPlayerList[select]
            #Vault node refs change frequently so we cannot do a straight comparison
            #Altering to use the child Node's ID...  it is supposed to be unique
            if type(self.previouslySelectedPlayer) == ptVaultNodeRef:
                PtDebugPrint("xKI.IRefreshPlayerList() Player selected getting the vault node ID", level=kDebugDumpLevel)
                self.previouslySelectedPlayer = self.previouslySelectedPlayer.getChild().getID()

        else:
            self.previouslySelectedPlayer = None
        self.BKPlayerList = []
        vault = ptVault()
        # if the bigKI is showing
        if BigKI.dialog.isEnabled() and not forceSmall:
            # get the AgeMember and Buddy folders and fill in
            agemembers = KIFolder(PtVaultStandardNodes.kAgeMembersFolder)
            if type(agemembers) != type(None):
                self.BKPlayerList.append(agemembers)
                self.BKPlayerList += PtGetPlayerListDistanceSorted()
            else:
                self.BKPlayerList.append("?NOAgeMembers?")
            if self.phasedKIBuddies:
                buddies = vault.getBuddyListFolder()
                if type(buddies) != type(None):
                    self.BKPlayerList.append(buddies)
                    self.BKPlayerList += self.IRemoveOfflinePlayers(buddies.getChildNodeRefList())
                else:
                    self.BKPlayerList.append("?NOBuddies?")
            if self.phasedKINeighborsInDPL:
                neighbors = self.IGetNeighbors()
                if type(neighbors) != type(None):
                    self.BKPlayerList.append(neighbors)
                    onlinePlayers = self.IRemoveOfflinePlayers(neighbors.getChildNodeRefList())

                    localplayer = PtGetLocalPlayer()
                    for idx in range(len(onlinePlayers)):
                        PLR = onlinePlayers[idx].getChild().upcastToPlayerInfoNode()
                        if PLR.playerGetID() == localplayer.getPlayerID():
                            del onlinePlayers[idx]
                            break
                    self.BKPlayerList += onlinePlayers
                else:
                    self.BKPlayerList.append("NEIGHBORS")
            if type(FolderOfDevices) != type(None) and len(FolderOfDevices) > 0:
                self.BKPlayerList.append(FolderOfDevices)
                for device in FolderOfDevices:
                    self.BKPlayerList.append(device)
        # else is the BigKI is not showing
        else:
            # get the AgeMember and Buddy folders and fill in
            agemembers = KIFolder(PtVaultStandardNodes.kAgeMembersFolder)
            if type(agemembers) != type(None):
                self.BKPlayerList.append(agemembers)
                self.BKPlayerList += PtGetPlayerListDistanceSorted()
            else:
                self.BKPlayerList.append("?NOAgeMembers?")
            if self.phasedKIBuddies:
                buddies = vault.getBuddyListFolder()
                if type(buddies) != type(None):
                    self.BKPlayerList.append(buddies)
                    self.BKPlayerList += self.IRemoveOfflinePlayers(buddies.getChildNodeRefList())
                else:
                    self.BKPlayerList.append("?NOBuddies?")
            if self.phasedKINeighborsInDPL:
                neighbors = self.IGetNeighbors()
                if type(neighbors) != type(None):
                    self.BKPlayerList.append(neighbors)
                    onlinePlayers = self.IRemoveOfflinePlayers(neighbors.getChildNodeRefList())

                    localplayer = PtGetLocalPlayer()
                    for idx in range(len(onlinePlayers)):
                        PLR = onlinePlayers[idx].getChild().upcastToPlayerInfoNode()
                        if PLR.playerGetID() == localplayer.getPlayerID():
                            del onlinePlayers[idx]
                            break
                    self.BKPlayerList += onlinePlayers
                else:
                    self.BKPlayerList.append("NEIGHBORS")
        
        # Pass the new value to the chat manager.
        self.chatMgr.BKPlayerList = self.BKPlayerList

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

    def IRefreshPlayerListDisplay(self):
        # now we can display this mess
        PtDebugPrint("xKI: refresh playerlist display",level=kDebugDumpLevel)
        playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        scrollPos = playerlist.getScrollPos()
        playerlist.lock()
        playerlist.clearAllElements()
        newselection = -1    # assume no selection

        idx = 0
        for plyr in self.BKPlayerList:
            if isinstance(plyr,DeviceFolder):
                playerlist.closeBranch()
                playerlist.addBranchW(plyr.name.upper(),1)
            elif isinstance(plyr,Device):
                playerlist.addStringWithColor(plyr.name,kColors.DniSelectable,kSelectUseGUIColor)
            elif isinstance(plyr,ptVaultNodeRef):
                PLR = plyr.getChild()
                PLR = PLR.upcastToPlayerInfoNode()
                # its an element.. should be a player
                if type(PLR) != type(None) and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                    if PLR.playerIsOnline():
                        playerlist.addStringWithColor(PLR.playerGetName(),kColors.DniSelectable,kSelectUseGUIColor)
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
                    playerlist.addStringWithColor(preText+plyr.getPlayerName()+postText,kColors.DniSelectable,kSelectUseGUIColor)
                else:
                    if plyr.getPlayerID() != 0:
                        playerlist.addStringWithColor(preText+"[ID:%08d]"%(plyr.getPlayerID())+postText,kColors.DniSelectable,kSelectDetermined)
                    else:
                        playerlist.addStringWithColor(preText+"?unknown user?"+postText,kColors.DniSelectable,kSelectDetermined)
            elif isinstance(plyr,KIFolder):
                playerlist.closeBranch()
                playerlist.addBranchW(plyr.name.upper(),1)
            elif isinstance(plyr,ptVaultPlayerInfoListNode):
                # its a player list, display its name
                fldrType = plyr.folderGetType()
                # if its a list of age owners... must be list of neighbors
                if fldrType == PtVaultStandardNodes.kAgeOwnersFolder:
                    fldrType = PtVaultStandardNodes.kHoodMembersFolder
                playerlist.closeBranch()
                playerlist.addBranchW(xLocTools.FolderIDToFolderName(fldrType).upper(),1)
            elif isinstance(plyr,ptVaultMarkerGameNode):
                # its a marker list, display its name
                playerlist.closeBranch()
                playerlist.addBranchW(plyr.folderGetName(),1)
            elif type(plyr) == type(""):
                playerlist.closeBranch()
                playerlist.addBranchW(plyr,1)
            else:
                PtDebugPrint("xBigKI: unknown list type ",plyr,level=kErrorLevel)
                pass
            # try to find out if it is the selected one
            # ...was there something selected?
            if type(self.previouslySelectedPlayer) != type(None):
                PtDebugPrint("xKI: a previously selected player",self.previouslySelectedPlayer, level=kDebugDumpLevel)

                #Fix for vaultNodeRef comparisons (they no longer work)!
                if type(self.previouslySelectedPlayer) == long and type(plyr) == ptVaultNodeRef:
                    plyr = plyr.getChild().getID()  #Set to the ID and now we can test!

                # was it the same class?
                if self.previouslySelectedPlayer.__class__ == plyr.__class__:
                    PtDebugPrint("xKI: Match class - previous player matches class", level=kDebugDumpLevel)
                    # and finlly, was it the same object
                    if self.previouslySelectedPlayer == plyr:
                        PtDebugPrint("xKI: Match object - previous player matches object, setting to %d"%(idx), level=kDebugDumpLevel)
                        newselection = idx
                        # found it... stop looking
                        self.previouslySelectedPlayer = None
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
            caret = ptGUIControlTextBox(KIMini.dialog.getControlFromTag(kGUI.ChatCaretID))
            caret.setString(">")
        PtDebugPrint("xKI:mini: setting new selection to %d"%(newselection),level=kDebugDumpLevel)
        playerlist.setSelection(newselection)
        self.previouslySelectedPlayer = None

        # re-establish the selection they had before
        playerlist.setScrollPos(scrollPos)
        playerlist.unlock()

        self.ICheckScrollButtons(playerlist,kGUI.miniPlayerListUp,kGUI.miniPlayerListDown)

        # set the SendTo button
        sendToButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        if self.BKPlayerSelected is None:
            sendToButton.hide()
        else:
            # else make sure that the person is still here
            if isinstance(self.BKPlayerSelected,DeviceFolder):
                # this shouldn't happen
                self.BKPlayerSelected = None
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                sendToField.setString("  ")
                sendToButton.hide()
            elif isinstance(self.BKPlayerSelected,Device):
                # see if the device is still in range
                try:
                    FolderOfDevices.index(self.BKPlayerSelected)
                except ValueError:
                    # no longer in the list of devices... then remove
                    self.BKPlayerSelected = None
                    sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                    sendToField.setString("  ")
                    sendToButton.hide()

    def IRefreshMiniKIMarkerDisplay(self):
        "refresh the display on the miniKI indicator bars"
        PtDebugPrint("xKI:GZ: Refreshing MarkerDisplay  %d:%d" % (gMarkerGottenNumber,gMarkerToGetNumber), level=kDebugDumpLevel)
        if self.KILevel > kMicroKI:
            if ((gMarkerGottenNumber == gMarkerToGetNumber) and ((gMarkerToGetNumber % 25) == 0)):
                xMyMaxMarkers = gMarkerToGetNumber
                xMyGotMarkers = gMarkerGottenNumber
            else:
                xMyGotMarkers = (gMarkerGottenNumber % 25)
                if (gMarkerGottenNumber >= (math.floor((gMarkerToGetNumber / 25)) * 25)):
                    xMyMaxMarkers = (gMarkerToGetNumber % 25)
                else:
                    xMyMaxMarkers = 25
            for mcbid in range(kGUI.miniMarkerIndicator01,kGUI.miniMarkerIndicatorLast+1):
                mcb = ptGUIControlProgress(KIMini.dialog.getControlFromTag(mcbid))
                markerNumber = mcbid - kGUI.miniMarkerIndicator01 + 1
                try:
                    if ((not self.gKIMarkerLevel) or (markerNumber > xMyMaxMarkers)):
                        mcb.setValue(kGUI.miniMarkerColors['off'])
                    elif ((markerNumber <= xMyMaxMarkers) and (markerNumber > xMyGotMarkers)):
                        mcb.setValue(kGUI.miniMarkerColors[gMarkerToGetColor])
                    else:
                        mcb.setValue(kGUI.miniMarkerColors[gMarkerGottenColor])
                except LookupError:
                    PtDebugPrint("xKI:GZ - couldn't find color - defaulting to off", level=kWarningLevel)
                    mcb.setValue(kGUI.miniMarkerColors['off'])
            btnmtDrip = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZDrip))
            btnmtActive = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZActive))
            btnmtPlaying = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZMarkerGameActive))
            btnmtInRange = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZMarkerInRange))

            btnmgNewMarker = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGNewMarker))
            btnmgNewGame = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGNewGame))
            btnmgInactive = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGInactive))


            if self.gKIMarkerLevel:
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
            if self.markerGameManager is not None and self.markerGameManager.gameData.data['svrGameTypeID'] == PtMarkerGameTypes.kMarkerGameCGZ:
                playingCGZ = 1
            else:
                playingCGZ = 0

            if self.gKIMarkerLevel >= kKIMarkerNormalLevel and self.phasedKICreateMarkerGame and not playingCGZ:
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
        fontSizeSlider = ptGUIControlKnob(KISettings.dialog.getControlFromTag(kGUI.BKIKIFontSize))
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

        fadeTimeSlider = ptGUIControlKnob(KISettings.dialog.getControlFromTag(kGUI.BKIKIFadeTime))
        slidePerTime = float(fadeTimeSlider.getMax()-fadeTimeSlider.getMin())/float(kFadeTimeMax)
        if not self.chatMgr.fadeEnableFlag:
            self.chatMgr.ticksOnFull = kFadeTimeMax
        FTslider = slidePerTime * self.chatMgr.ticksOnFull
        fadeTimeSlider.setValue(FTslider)

        onlyPMCheckbox = ptGUIControlCheckBox(KISettings.dialog.getControlFromTag(kGUI.BKIKIOnlyPM))
        onlyPMCheckbox.setChecked(self.onlyGetPMsFromBuddies)

    def IRefreshVolumeSettings(self):
        "refresh the volume settings to the current settings"
        audio = ptAudioControl()
        soundFX = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKISoundFXVolSlider))
        setting = audio.getSoundFXVolume()
        soundFX.setValue(setting*10)

        music = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(xBKIMusicVolSlider))
        setting = audio.getMusicVolume()
        music.setValue(setting*10)

        voice = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(xBKIVoiceVolSlider))
        setting = audio.getVoiceVolume()
        voice.setValue(setting*10)

        ambience = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIAmbienceVolSlider))
        setting = audio.getAmbienceVolume()
        ambience.setValue(setting*10)

        miclevel = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIMicLevelSlider))
        setting = audio.getMicLevel()
        miclevel.setValue(setting*10)

        guivolume = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIGUIVolSlider))
        setting = audio.getGUIVolume()
        guivolume.setValue(setting*10)

    def IRefreshAgeOwnerSettings(self):
        "refresh the Age Owner neighborhood settings"
        # make sure that we are actually going to display (or is it just an update)
        if BigKI.dialog.isEnabled() and self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
            try:
                # get the selected age config setting
                myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
            except LookupError:
                myAge = None
            if type(myAge) != type(None):
                title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleTB))
                title.setString(self.GetAgeName(myAge))
                titlebtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleBtn))
                titlebtn.enable()
                titleedit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleEditbox))
                titleedit.hide()
                status = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerStatusTB))
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
                makepublicTB = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerMakePublicTB))
                makepublicBtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerMakePublicBtn))
                makepublicBtn.disable()
                makepublicTB.hide()
                makepublicTB.setString(" ")
                status.setStringW(PtGetLocalizedString("KI.Neighborhood.AgeOwnedStatus", [str(numowners),str(osess),str(numvisitors),str(vsess)]))
                descript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescription))
                encoded = buffer(myAge.getAgeDescription())
                descript.setEncodedBuffer(encoded)

    def IminiToggleKISize(self):
        "Toggle between mini and BigKI"
        if self.KILevel > kMicroKI and (not self.KIDisabled or BigKI.dialog.isEnabled()):
            if self.KIDisabled and BigKI.dialog.isEnabled():
                self.IminiPutAwayKI()
                return
            if not self.waitingForAnimation:
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                if BigKI.dialog.isEnabled():
                    # hide the BigKI part
                    self.IBigKIHideBigKI()
                    # can't be in chatting mode
                    self.chatMgr.ToggleChatMode(0)
                    KIBlackbar.dialog.show()
                    if LastminiKICenter is not None:
                        dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                        dragbar.setObjectCenter(self.lastminiKICenter)
                        dragbar.unanchor()
                        LastminiKICenter = None
                    # refresh the player list, because it will be the shorten version
                    self.IRefreshPlayerList(forceSmall=1)
                    self.IRefreshPlayerListDisplay()
                    toggleCB.setChecked(0)
                else:
                    # if there is nothing showing, then just bring up the miniKI
                    if not KIMini.dialog.isEnabled():
                        self.chatMgr.ClearBBMini(0)    # set the checkbox, which will show the dialog... chain reaction
                    else:
                        self.waitingForAnimation = True
                        # hide the miniKI and blackbar
                        KIBlackbar.dialog.hide()
                        # hide the miniKI so that it is shown on top of the bigKI
                        KIMini.dialog.hide()
                        # can't be in chatting mode
                        self.chatMgr.ToggleChatMode(0)
                        # show the big stuff
                        BigKI.dialog.show()
                        # save current location and snap back to original
                        if self.originalminiKICenter is not None:
                            dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                            LastminiKICenter = dragbar.getObjectCenter()
                            PtDebugPrint("xKI:distance to original = %f"%(LastminiKICenter.distance(self.originalminiKICenter)),level=kDebugDumpLevel)
                            # if they are close, then snap it to original
                            if LastminiKICenter.distance(self.originalminiKICenter) < 0.027:
                                LastminiKICenter = self.originalminiKICenter
                            dragbar.setObjectCenter(self.originalminiKICenter)
                            dragbar.anchor()
                        KIMini.dialog.show()
                        toggleCB.setChecked(1)

    def IminiPutAwayKI(self, forceOpen = 0):
        if self.KILevel > kMicroKI and (not self.KIDisabled or KIMini.dialog.isEnabled()):
            if KIMini.dialog.isEnabled():
                # put away the KI (both mini and big)
                KIMini.dialog.hide()
                # and put back where it used to be
                if LastminiKICenter is not None:
                    dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                    dragbar.setObjectCenter(LastminiKICenter)
                    dragbar.unanchor()
                    LastminiKICenter = None
                if BigKI.dialog.isEnabled():
                    # hide the BigKI part
                    self.IBigKIHideBigKI()
                KIBlackbar.dialog.show()
                self.chatMgr.ClearBBMini(-1)
                # put the toggle button back to miniKI
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                toggleCB.setChecked(0)
                ISawTheKIAtleastOnce = True
            else:
                # else if already disabled, then bring up the miniKI
                if forceOpen:
                    self.chatMgr.ClearBBMini(0)    # set the checkbox, which will show the dialog... chain reaction

    def IminiTakePicture(self):
        if self.phasedKICreateImages:
            if not self.takingAPicture and not self.waitingForAnimation:
                # NOTE: we are ignoring the self.KIDisabled flag here because we want to be able to take sshots even
                # if the KI is disabled by certain GUIs. Hopefully this won't cause any weird issues, but since
                # it's only a screenshot, not much can go wrong (famous last words...)
                if self.KILevel > kMicroKI:
                    if self.ICanTakePicture():
                        self.takingAPicture = True
                        # if a modal dialog is showing, just snap the shot because the KI is
                        # already out of the way
                        if not PtIsGUIModal():
                            # hide everything to take a picture
                            KIBlackbar.dialog.hide()
                            KIMini.dialog.hide()
                            self.IBigKIHideMode()
                            BigKI.dialog.hide()
                            # put the toggle button back to bigKI
                            toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                            toggleCB.setChecked(1)
                        # wait a second and take the picture!
                        PtAtTimeCallback(self.key,0.25,kTakeSnapShot)
                    else:
                        # put up some kinda error message
                        self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullImages"))


    def IminiCreateJournal(self):
        if self.phasedKICreateNotes:
            if not self.takingAPicture and not self.waitingForAnimation:
                if self.KILevel > kMicroKI and not self.KIDisabled:
                    if self.ICanMakeNote():
                        # hide the blackbar (even if its already hid)
                        KIBlackbar.dialog.hide()
                        # put the toggle button back to bigKI
                        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                        toggleCB.setChecked(1)
                        # create a journal entry
                        self.IBigKICreateJournalNote()
                        # make sure that we are in journal mode
                        modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
                        modeselector.setValue(0)
                        # set things up so that when the bigKI shows it goes into edit mode
                        if self.BKRightSideMode != kGUI.BKJournalExpanded:
                            # make sure if a different is showing, hide it
                            self.IBigKIHideMode()
                        # now it "show the journal expanded" mode
                        self.BKRightSideMode = kGUI.BKJournalExpanded
                        # reset the topline and selection
                        self.IBigKIRefreshFolderDisplay()
                        self.IBigKIDisplayCurrentContentJournal()
                        # setup to edit the title of the journal (err... the caption)
                        self.IBigKIEnterEditMode(kGUI.BKEditFieldJRNTitle)
                        if BigKI.dialog.isEnabled():
                            self.IBigKIShowMode()
                        else:
                            # need to make the miniKI come up over the bigKI
                            KIMini.dialog.hide()
                            # show the big stuff
                            BigKI.dialog.show()
                            # need to make the miniKI come up over the bigKI
                            KIMini.dialog.show()
                        # was just the miniMI showing (not the BigKI?)
                        if LastminiKICenter is None:
                            if self.originalminiKICenter is not None:
                                dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                                LastminiKICenter = dragbar.getObjectCenter()
                                dragbar.setObjectCenter(self.originalminiKICenter)
                                dragbar.anchor()
                    else:
                        # put up some kinda error message
                        self.IShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullNotes"))

    def ICaptureGZMarker(self):
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
                    if self.gKIMarkerLevel > kKIMarkerFirstLevel:
                        # if this is the second wave of markers (or beyond)
                        totalGotten -= 15
                        if totalGotten < 0:
                            totalGotten = 0
                    if totalGotten > gMarkerToGetNumber:
                        totalGotten = gMarkerToGetNumber
                    gMarkerGottenNumber = totalGotten
                    # save update to chronicle
                    self.IUpdateGZGamesChronicle()
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
        self.waitingForAnimation = True
        curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
        toggleCB.disable()
        if curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit:
            PtDebugPrint("xKI:ShowKI enter LookingAtKI mode",level=kDebugDumpLevel)
            PtAvatarEnterLookingAtKI()
            IsPlayingLookingAtKIMode = True
        PtDisableMovementKeys()
        KIOnResp.run(self.key,netPropagate=0)
        if (self.gKIMarkerLevel == kKIMarkerNormalLevel):
            sdl = xPsnlVaultSDL()
            if (not sdl['GPSEnabled'][0]):
                PtDebugPrint('xKI:ShowKI check calibration', level=kDebugDumpLevel)
                try:
                    self.ICheckCalibrationProgress()
                except:
                    PtDebugPrint("ERROR: xKI.IBigKIShowBigKI(): Couldn't execute self.ICheckCalibrationProgress()")


    def IBigKIHideBigKI(self):
        self.waitingForAnimation = True
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
        toggleCB.disable()
        self.IBigKIHideMode()
        # make sure we were in
        if IsPlayingLookingAtKIMode:
            PtDebugPrint("xKI:HideKI exit LookingAtKI mode",level=kDebugDumpLevel)
            PtAvatarExitLookingAtKI()
        IsPlayingLookingAtKIMode = False
        PtEnableMovementKeys()
        KIOffResp.run(self.key,netPropagate=0)

    def IBigKIShowMode(self):

        if BigKI.dialog.isEnabled():
            self.IResetListModeArrows()
            if self.BKRightSideMode == kGUI.BKListMode:
                KIListModeDialog.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKJournalExpanded:
                KIJournalExpanded.dialog.show()
                if self.IsContentMutable(self.BKCurrentContent):
                    self.IBigKIInvertToFolderButtons()
                else:
                    self.IBigKIOnlySelectedToButtons()
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKPictureExpanded:
                KIPictureExpanded.dialog.show()
                if self.IsContentMutable(self.BKCurrentContent):
                    self.IBigKIInvertToFolderButtons()
                else:
                    self.IBigKIOnlySelectedToButtons()
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
                KIPlayerExpanded.dialog.show()
                # if the expanded player is ourselves, then no move buttons!
                localplayer = PtGetLocalPlayer()
                if type(self.BKCurrentContent) != type(None):
                    if isinstance(self.BKCurrentContent, ptPlayer):
                        if self.BKCurrentContent.getPlayerID() == localplayer.getPlayerID():
                            self.IBigKIOnlySelectedToButtons()
                            return
                    # else assume that its a plVaultNodeRef
                    else:
                        elem = self.BKCurrentContent.getChild()
                        if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                            elem = elem.upcastToPlayerInfoNode()
                            if elem.playerGetID() == localplayer.getPlayerID():
                                self.IBigKIOnlySelectedToButtons()
                                return
                self.IBigKIInvertToFolderButtons()
            elif self.BKRightSideMode == kGUI.BKVolumeExpanded:
                KIVolumeExpanded.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKKIExpanded:
                KISettings.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
                KIAgeOwnerExpanded.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                KIMarkerFolderExpanded.dialog.show()
                if self.phasedKISendMarkerGame and self.IsContentMutable(self.BKCurrentContent):
                    self.IBigKIInvertToFolderButtons()
                else:
                    self.IBigKIOnlySelectedToButtons()
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKQuestionNote:
                KIQuestionNote.dialog.show()
                self.IBigKIOnlySelectedToButtons()
                self.BKGettingPlayerID = False

    def IBigKIHideMode(self):
        if self.BKRightSideMode == kGUI.BKListMode:
            KIListModeDialog.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKJournalExpanded:
            KIJournalExpanded.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKPictureExpanded:
            KIPictureExpanded.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
            KIPlayerExpanded.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKVolumeExpanded:
            KIVolumeExpanded.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKKIExpanded:
            KISettings.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
            KIAgeOwnerExpanded.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
            KIMarkerFolderExpanded.dialog.hide()
        elif self.BKRightSideMode == kGUI.BKQuestionNote:
            KIQuestionNote.dialog.hide()

    def IBigKIChangeMode(self,newmode):
        if newmode != self.BKRightSideMode:
            self.IBigKIHideMode()
            self.BKRightSideMode = newmode
            self.IBigKIShowMode()
        elif newmode == kGUI.BKListMode:
            # might just be a selection change
            self.IBigKIOnlySelectedToButtons()

    def IBigKISetToButtons(self):
        if self.BKRightSideMode == kGUI.BKListMode:
            self.IBigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKJournalExpanded:
            if self.IsContentMutable(self.BKCurrentContent):
                self.IBigKIInvertToFolderButtons()
            else:
                self.IBigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKPictureExpanded:
            if self.IsContentMutable(self.BKCurrentContent):
                self.IBigKIInvertToFolderButtons()
            else:
                self.IBigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
            localplayer = PtGetLocalPlayer()
            if type(self.BKCurrentContent) != type(None):
                if isinstance(self.BKCurrentContent, ptPlayer):
                    if self.BKCurrentContent.getPlayerID() == localplayer.getPlayerID():
                        self.IBigKIOnlySelectedToButtons()
                        return
                # else assume that its a plVaultNodeRef
                else:
                    elem = self.BKCurrentContent.getChild()
                    if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                        elem = elem.upcastToPlayerInfoNode()
                        if elem.playerGetID() == localplayer.getPlayerID():
                            self.IBigKIOnlySelectedToButtons()
                            return
            self.IBigKIInvertToFolderButtons()
        elif self.BKRightSideMode == kGUI.BKVolumeExpanded:
            self.IBigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKKIExpanded:
            self.IBigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
            self.IBigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
            if self.phasedKISendMarkerGame and MFdialogMode != kMFPlaying and self.IsContentMutable(self.BKCurrentContent):
                self.IBigKIInvertToFolderButtons()
            else:
                self.IBigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKQuestionNote:
            self.IBigKIOnlySelectedToButtons()

    def IBigKISetNotifyForLong(self):
        for id in range(kGUI.BKIIncomingBtn, kGUI.BKIFolderLineBtnLast):
            overBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(id))
            overBtn.setNotifyOnInteresting(1)

    def IBigKIHideLongFolderNames(self):
        for id in range(kGUI.LONGBKIIncomingLine,kGUI.LONGBKIFolderLineLast+1):
            longTB = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(id))
            longTB.hide()

    def IResetListModeArrows(self):
        # hide up/down scroll buttons (this should be in expanded mode)
        upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMUpButton))
        upbtn.hide()
        dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMDownButton))
        dwnbtn.hide()

    def IBigKIOnlySelectedToButtons(self):
        toPlayerBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        toPlayerBtn.hide()
        self.IBigKIRefreshFolderDisplay()
        # hide all the buttons
        for id in range(kGUI.BKIToIncomingButton,kGUI.BKIToFolderButtonLast+1):
            toFolder = ptGUIControlButton(BigKI.dialog.getControlFromTag(id))
            toFolder.hide()
        self.IBigKINewContentList()

    def IBigKICanShowSendToPlayer(self):
        # make sure there is a Selected player
        if self.BKPlayerSelected is None:
            return false
        # make sure that its something that can be sent to a player
        if self.BKRightSideMode == kGUI.BKPlayerExpanded or self.BKRightSideMode == kGUI.BKVolumeExpanded or self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
            return false
        if self.BKRightSideMode == kGUI.BKJournalExpanded:
            if not self.phasedKISendNotes:
                return false
        if self.BKRightSideMode == kGUI.BKPictureExpanded:
            if not self.phasedKISendImages:
                return false
        # make sure that its not ourselves
        if isinstance(self.BKPlayerSelected,ptVaultNodeRef):
            plyrElement = self.BKPlayerSelected.getChild()
            if type(plyrElement) != type(None) and plyrElement.getType()==PtVaultNodeTypes.kPlayerInfoNode:
                plyrElement = plyrElement.upcastToPlayerInfoNode()
                if plyrElement.playerGetID() == PtGetLocalClientID():
                    return false
        # then finally it must ok
        return true

    def IBigKIInvertToFolderButtons(self):
        # setup 'toplayer' button
        toPlayerBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        if self.IBigKICanShowSendToPlayer():
            toPlayerBtn.show()
        else:
            toPlayerBtn.hide()
        selectedButton = self.BKFolderSelected - self.BKFolderTopLine + kGUI.BKIToIncomingButton
        for id in range(kGUI.BKIToIncomingButton,kGUI.BKIToFolderButtonLast+1):
            toFolder = ptGUIControlButton(BigKI.dialog.getControlFromTag(id))
            if id == selectedButton:
                toFolder.hide()
            else:
                # but don't show on elements that are not there or immutable
                if id - kGUI.BKIToIncomingButton <= len(self.BKFolderListOrder)-1-self.BKFolderTopLine:
                    try:
                        if self.IsFolderContentsMutable(self.BKFolderLineDict[self.BKFolderListOrder[id-kGUI.BKIToIncomingButton+self.BKFolderTopLine]]):
                            # assume that we are going to show the bugger
                            toFolder.show()
                        else:
                            toFolder.hide()
                    except LookupError:
                        toFolder.hide()
                else:
                    toFolder.hide()

    def IsFolderContentsMutable(self,folder):
        "determines whether the folder in question can be changed"
        # make sure there is a real folder there to play with
        if folder is None or not isinstance(folder,ptVaultNode):
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
            folder = self.BKCurrentContent.getParent()
            if folder:
                folder = folder.upcastToFolderNode()
                if folder:
                    if folder.folderGetType() == PtVaultStandardNodes.kGlobalInboxFolder:
                        return 0
        return 1

    def IBigKISetStatics(self):
        "Initialize the static string fields"
        agetext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKICurAgeNameID))
        ageName = self.GetAgeName().replace("(null)", "").strip()
        PtDebugPrint("xKI: displaying age name of %s"%(ageName),level=kDebugDumpLevel)
        agetext.setStringW(ageName)
        playertext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKPlayerName))
        idtext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKPlayerID))
        localplayer = PtGetLocalPlayer()
        playertext.setString(localplayer.getPlayerName())
        idtext.setString("[ID:%08d]"%(localplayer.getPlayerID()))
        self.IUpdatePelletScore()
        self.IBigKIRefreshHoodStatics()

    def IBigKIRefreshHoodStatics(self,neighborhood=None):
        neighbortext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKNeighborhoodAndID))
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

    def IBigKISetChanging(self):
        "Set the changing fields to current settings"
        # use the D'ni time for this age
        dnitime = PtGetDniTime()
        if dnitime:
            tuptime = time.gmtime(dnitime)
            if TimeBlinker:
                curtime = unicode(time.strftime(PtGetLocalizedString("Global.Formats.DateTime"),tuptime))
                TimeBlinker = 0
            else:
                curtime = unicode(time.strftime(PtGetLocalizedString("Global.Formats.DateTime"),tuptime))
                TimeBlinker = 1
        else:
            curtime = PtGetLocalizedString("KI.Errors.TimeBroke")
        if curtime != PreviousTime:
            timetext = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKICurTimeID))
            timetext.setStringW(curtime)
            PreviousTime = curtime
        # get the gps dni coordinates holders on the screen
        gps1 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIGPS1TextID))
        gps2 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIGPS2TextID))
        gps3 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIGPS3TextID))
        # set the GPS numbers
        # later on.... we will use the real stuff
        self.dniCoords.update()
        if self.gKIMarkerLevel == kKIMarkerNormalLevel:
            sdl = xPsnlVaultSDL()
            if sdl["GPSEnabled"][0]:
                gps1.setString(str(self.dniCoords.getTorans()))
                gps2.setString(str(self.dniCoords.getHSpans()))
                gps3.setString(str(self.dniCoords.getVSpans()))
            else:
                gps1.setString("0")
                gps2.setString("0")
                gps3.setString("0")
        else:
            gps1.setString("0")
            gps2.setString("0")
            gps3.setString("0")
        PtAtTimeCallback(self.key,5,kGUI.BKITODCheck)

    def IBKCheckFolderRefresh(self,folder=None,content=None):
        "check to see if this a folder we're concerned about and needs to be refreshed"
        if type(folder) != type(None):
            if folder.getType() == PtVaultNodeTypes.kPlayerInfoListNode:
                self.IRefreshPlayerList()
                self.IRefreshPlayerListDisplay()
        else:
            # if its none, then just check everything
            self.IRefreshPlayerList()
            self.IRefreshPlayerListDisplay()
        # only need to check refresh if we are actaully using the bigKI
        if self.KILevel > kMicroKI:
            self.IBigKIRefreshContentList()
            self.IBigKIRefreshContentListDisplay()

    def IBKCheckContentRefresh(self,content):
        "check to see if the content.element they are looking at changed underneath them"
        if type(self.BKCurrentContent) != type(None) and content == self.BKCurrentContent:
            if self.BKRightSideMode == kGUI.BKListMode:
                self.IBigKIRefreshContentListDisplay()
            elif self.BKRightSideMode == kGUI.BKJournalExpanded:
                self.IBigKIDisplayCurrentContentJournal()
            elif self.BKRightSideMode == kGUI.BKPictureExpanded:
                self.IBigKIDisplayCurrentContentImage()
            elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
                self.IBigKIDisplayCurrentContentPlayer()
            elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                self.IBigKIDisplayCurrentContentMarkerFolder()
        else:
            pass

    def IBKCheckElementRefresh(self,element):
        "check to see if the content.element they are looking at changed underneath them"
        if type(self.BKCurrentContent) != type(None):
            if isinstance(self.BKCurrentContent,ptVaultNodeRef) and element == self.BKCurrentContent.getChild():
                if self.BKRightSideMode == kGUI.BKListMode:
                    self.IBigKIRefreshContentListDisplay()
                elif self.BKRightSideMode == kGUI.BKJournalExpanded:
                    self.IBigKIDisplayCurrentContentJournal()
                elif self.BKRightSideMode == kGUI.BKPictureExpanded:
                    self.IBigKIDisplayCurrentContentImage()
                elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
                    self.IBigKIDisplayCurrentContentPlayer()
                elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                    self.IBigKIDisplayCurrentContentMarkerFolder()

    def IBigKIRefreshFolders(self):
        "Refresh the Folders list for the Inbox and the Age Journal folders"
        # need to remember selected and what position in the list we are at
        vault = ptVault()
        #
        # get the journal folder stuffs
        #
        if not self.BKJournalFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder)):
            infolder = vault.getInbox()
            if type(infolder) != type(None):
                self.BKJournalListOrder.insert(0,xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder))
                self.BKJournalFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder)] = infolder
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
                        agefoldername = self.FilterAgeName(agefoldername)
                        if agefoldername in kAges.Hide:
                            continue
                        if not self.BKJournalFolderDict.has_key(agefoldername):
                            # a new one... add
                            self.BKJournalListOrder.append(agefoldername)
                        self.BKJournalFolderDict[agefoldername] = agefolder
            # make sure the current age is at the top of the list
            try:
                line = self.BKJournalListOrder.index(self.IGetAgeInstanceName())
                if line != 1:
                    # its not at the top of the list... put at the top
                    self.BKJournalListOrder.remove(self.IGetAgeInstanceName())
                    self.BKJournalListOrder.insert(1,self.IGetAgeInstanceName())
                    # if they are looking at a journal entry then put them in list mode
                    if self.BKRightSideMode == kGUI.BKJournalExpanded or self.BKRightSideMode == kGUI.BKPictureExpanded or self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                        # put 'em in list mode
                        self.IBigKIChangeMode(kGUI.BKListMode)
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
        self.BKPlayerFolderDict.clear()
        del self.BKPlayerListOrder[:]
        agemembers = KIFolder(PtVaultStandardNodes.kAgeMembersFolder)
        if type(agemembers) != type(None):
            if not self.BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder)):
                # a new one... add
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder))
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder)] = agemembers
            PtDebugPrint("xBigKI: updating agemembers ",level=kDebugDumpLevel)
        else:
            PtDebugPrint("xBigKI: AgeMembers folder is missing!",level=kWarningLevel)
        if self.phasedKIBuddies:
            buddies = vault.getBuddyListFolder()
            if type(buddies) != type(None):
                if not self.BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)):
                    # a new one... add
                    self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder))
                self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)] = buddies
            else:
                PtDebugPrint("xBigKI: Buddies folder is missing!",level=kWarningLevel)
        if self.phasedKINeighborsInDPL:
            # update the neighborhood folders
            self.IBigKIRefreshNeighborFolders()
        # update the Recent people folder
        PIKA = vault.getPeopleIKnowAboutFolder()
        if type(PIKA) != type(None):
            if not self.BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder)):
                # a new one... add
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder))
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder)] = PIKA
        else:
            PtDebugPrint("xBigKI: PeopleIKnowAbout folder is missing!",level=kWarningLevel)
        ignores = vault.getIgnoreListFolder()
        if type(ignores) != type(None):
            if not self.BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder)):
                # a new one... add
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder))
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder)] = ignores
        else:
            PtDebugPrint("xBigKI: People I'm ignoring folder is missing!")
        # ==== age visitors ====
        # add in folder separator
        visSep = SeparatorFolder(PtGetLocalizedString("KI.Folders.VisLists"))
        self.BKPlayerListOrder.append(visSep.name)
        self.BKPlayerFolderDict[visSep.name] = visSep
        self.IBigKIRefreshAgeVisitorFolders()
        # ====Age owners ===
        self.IBigKIRefreshAgesOwnedFolder()

    def IBigKIRefreshNeighborFolders(self):
        "refresh just the neighborhood folders"
        neighborhood = self.IGetNeighborhood()
        try:
            neighbors = neighborhood.getAgeOwnersFolder()
            if not self.BKPlayerFolderDict.has_key(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)):
                # a new one... add
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder))
            PtDebugPrint("xKI: got the neighbors player folder",level=kDebugDumpLevel)
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)] = neighbors
        except AttributeError:
            PtDebugPrint("xBigKI: Neighborhood is missing!",level=kWarningLevel)

    def IBigKIRefreshAgeVisitorFolders(self):
        "refresh the vistor folders for the ages that I own"
        vault = ptVault()
        try:
            myAgesFolder = vault.getAgesIOwnFolder()
            listOfMyAgeLinks = myAgesFolder.getChildNodeRefList()
            for myAgeLinkRef in listOfMyAgeLinks:
                myAgeLink = myAgeLinkRef.getChild()
                myAgeLink = myAgeLink.upcastToAgeLinkNode()
                myAge = myAgeLink.getAgeInfo()
                if type(myAge) != type(None):
                    if self.ICanAgeInviteVistors(myAge,myAgeLink) and (myAge.getAgeFilename() not in kAges.Hide) and (myAge.getAgeFilename() != "Myst"):
                        PtDebugPrint("xKI: refreshing visitor list for %s"%(self.GetAgeName(myAge)), level=kDebugDumpLevel)
                        foldername = xCensor.xCensor(PtGetLocalizedString("KI.Config.OwnerVisitors", [self.GetAgeName(myAge)]),self.censorLevel)
                        if not self.BKPlayerFolderDict.has_key(foldername):
                            # a new one... add
                            PtDebugPrint("xKI: adding visitor list for %s"%(self.GetAgeName(myAge)), level=kDebugDumpLevel)
                            self.BKPlayerListOrder.append(foldername)
                        self.BKPlayerFolderDict[foldername] = myAge
                else:
                    PtDebugPrint("xKI: age info for %s is not ready yet"%(myAgeLink.getUserDefinedName()),level=kErrorLevel)
        except AttributeError:
            PtDebugPrint("xKI: error finding age visitors folder",level=kErrorLevel)

    def IBigKIRefreshAgesOwnedFolder(self):
        "refresh the config folder listing for all the ages I own"
        # first get rid of all the age config entries, in case one of the got deleted
        self.BKConfigFolderDict.clear()
        del self.BKConfigListOrder[:]
        for config in self.BKConfigDefaultListOrder:
            self.BKConfigListOrder.append(config)
        vault = ptVault()
        try:
            myAgesFolder = vault.getAgesIOwnFolder()
            listOfMyAgeLinks = myAgesFolder.getChildNodeRefList()
            for myAgeLinkRef in listOfMyAgeLinks:
                myAgeLink = myAgeLinkRef.getChild()
                myAgeLink = myAgeLink.upcastToAgeLinkNode()
                myAge = myAgeLink.getAgeInfo()
                if type(myAge) != type(None):
                    if myAge.getAgeFilename() == "Neighborhood" and (myAge.getAgeFilename() not in kAges.Hide) and (myAge.getAgeFilename() != "Myst"):
                        PtDebugPrint("xKI: refreshing owner config for Age %s"%(self.GetAgeName(myAge)),level=kDebugDumpLevel)
                        configname = xCensor.xCensor(PtGetLocalizedString("KI.Config.OwnerConfig", [self.GetAgeName(myAge)]),self.censorLevel)
                        if not self.BKConfigFolderDict.has_key(configname):
                            # a new one... add
                            PtDebugPrint("xKI: adding owner config for Age %s"%(self.GetAgeName(myAge)),level=kDebugDumpLevel)
                            self.BKConfigListOrder.append(configname)
                        self.BKConfigFolderDict[configname] = myAge
                else:
                    PtDebugPrint("xKI:(AgeOwnerRefresh) age info for %s is not ready yet"%(myAgeLink.getUserDefinedName()),level=kErrorLevel)
        except AttributeError:
            PtDebugPrint("xKI:(AgeOwnerRefresh) error finding age folder",level=kErrorLevel)


    def IBigKIRefreshFolderDisplay(self):
        "Refresh the display of the folders and the selection"
        # refresh the display of the folders
        id = kGUI.BKIIncomingLine
        if len(self.BKFolderListOrder) != 0:
            # make sure that it is a valid index
            if self.BKFolderTopLine >= len(self.BKFolderListOrder):
                self.BKFolderTopLine = len(self.BKFolderListOrder)-1
            # if the selected is off the screen to the top then (but only in list mode)
# Need to note when the self.BKFolderSelected has changed... need to refresh the content display
            if self.BKRightSideMode == kGUI.BKListMode:
                if self.BKFolderSelected < self.BKFolderTopLine:
                    self.BKFolderSelected = self.BKFolderTopLine
                if self.BKFolderSelected > self.BKFolderTopLine+(kGUI.BKIFolderLineLast-kGUI.BKIIncomingLine):
                    self.BKFolderSelected = self.BKFolderTopLine+(kGUI.BKIFolderLineLast-kGUI.BKIIncomingLine)
                if self.BKFolderSelected > self.BKFolderTopLine+len(self.BKFolderListOrder[self.BKFolderTopLine:])-1:
                    self.BKFolderSelected = self.BKFolderTopLine+len(self.BKFolderListOrder[self.BKFolderTopLine])-1
            selectedFolder = self.BKFolderSelected - self.BKFolderTopLine + kGUI.BKIIncomingLine
            for foldername in self.BKFolderListOrder[self.BKFolderTopLine:]:
                folderfield = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(id))
                LONGfolderfield = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(id+500))
                buttonid = id-kGUI.BKIFolderLine01+kGUI.BKIFolderLineBtn01
                folderbutton = ptGUIControlButton(BigKI.dialog.getControlFromTag(buttonid))
                # make sure its not a separator folder
                if foldername in self.BKFolderLineDict and isinstance(self.BKFolderLineDict[foldername],SeparatorFolder):
                    # don't allow them to click on this one
                    folderbutton.hide()
                    # and left justifiy it
                    folderfield.setStringJustify(kLeftJustify)
                    # and color it as a static field (sorta)
                    folderfield.setForeColor(kColors.DniStatic)
                else:
                    folderbutton.show()
                    # make sure its right justify (default)
                    folderfield.setStringJustify(kRightJustify)
                    # set the color of the folder field
                    if id == selectedFolder:
                        folderfield.setForeColor(kColors.DniSelected)
                        LONGfolderfield.setForeColor(kColors.DniSelected)
                    else:
                        folderfield.setForeColor(kColors.DniSelectable)
                        LONGfolderfield.setForeColor(kColors.DniSelectable)
                folderfield.setStringW(foldername)
                LONGfolderfield.setStringW(foldername)
                id += 1
                if id > kGUI.BKIFolderLineLast:
                    break
        # set the up and down buttons if needed
        upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKFolderUpLine))
        if self.BKFolderTopLine > 0:
            upbtn.show()
        else:
            upbtn.hide()
        dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKFolderDownLine))
        if id > kGUI.BKIFolderLineLast:         # have we filled up the listbox?
            # yes, then we need down arrow
            dwnbtn.show()
        else:
            dwnbtn.hide()

        # if there are more folder lines, fill them out to be blank
        #   and disable the button fields while you're at it
        for tagid in range(id,kGUI.BKIFolderLineLast+1):
            folderfield = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(tagid))
            folderfield.setForeColor(kColors.DniSelectable)
            folderfield.setString(" ")
            buttonid = tagid-kGUI.BKIFolderLine01+kGUI.BKIFolderLineBtn01
            folderbutton = ptGUIControlButton(BigKI.dialog.getControlFromTag(buttonid))
            folderbutton.hide()

    def IBigKINewContentList(self):
        "reget the contents of the folder that is selected, if we are in list mode"
        # reget the contents of the selected folder
        try:
            foldername = self.BKFolderListOrder[self.BKFolderSelected]
            folder = self.BKFolderLineDict[foldername]
            if type(folder) != type(None):
                if isinstance(folder,ptVaultNode):
                    if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                        try:
                            self.BKContentList = folder.getCanVisitFolder().getChildNodeRefList()
                        except AttributeError:
                            self.BKContentList = []
                    else:
                        self.BKContentList = folder.getChildNodeRefList()
                    self.IBigKIProcessContentList(removeInboxStuff=1)
                    if self.BKFolderSelectChanged:
                        self.BKContentListTopLine = 0
                elif isinstance(folder,KIFolder):
                    self.BKContentList = PtGetPlayerListDistanceSorted()
                    self.IBigKIProcessContentList(removeInboxStuff=1)
                    if self.BKFolderSelectChanged:
                        self.BKContentListTopLine = 0
                else:
                    # this really shouldn't happen becuase the user should not be able to click on these kind of folders
                    self.BKContentList = []
        except (IndexError,KeyError):
            del self.BKContentList[:]
        self.IBigKIRefreshContentListDisplay()

    def IBigKIRefreshContentList(self):
        "display the contents of the folder that is selected, if we are in list mode"
        # reget the contents of the selected folder
        try:
            foldername = self.BKFolderListOrder[self.BKFolderSelected]
            folder = self.BKFolderLineDict[foldername]
            if type(folder) != type(None):
                if isinstance(folder,ptVaultNode):
                    if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                        try:
                            self.BKContentList = folder.getCanVisitFolder().getChildNodeRefList()
                        except AttributeError:
                            self.BKContentList = []
                    else:
                        self.BKContentList = folder.getChildNodeRefList()
                    self.IBigKIProcessContentList()
                elif isinstance(folder,KIFolder):
                    self.BKContentList = PtGetPlayerListDistanceSorted()
                    self.IBigKIProcessContentList()
            else:
                del self.BKContentList[:]
        except LookupError:
            pass

    def IBigKIProcessContentList(self,removeInboxStuff=0):
        "Do extra processing on content folder list"
        # start with nothing in the removelist (remove from current content list)
        removelist = []
        # if player list
        if self.BKFolderLineDict is self.BKPlayerFolderDict:
            ignores = ptVault().getIgnoreListFolder()
            # make sure there are some players to process
            if len(self.BKContentList) > 0:
                # if this is a ptPlayer
                if isinstance(self.BKContentList[0],ptPlayer):
                    # sort the list of age players - up front
                    try:
                        self.BKContentList.sort(lambda a, b: cmp(a.getPlayerName().lower(), b.getPlayerName().lower()))
                    except:
                        PtDebugPrint("xBigKI: Unable to sort age players but let's not break the list", level=kErrorLevel)

                    for idx in range(len(self.BKContentList)):
                        player = self.BKContentList[idx]
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
                    self.BKContentList.sort(CMPplayerOnline)
                    # remove all the no-named players and ignored people
                    for idx in range(len(self.BKContentList)):
                        ref = self.BKContentList[idx]
                        elem = ref.getChild()
                        if type(elem) != type(None):
                            if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                elem = elem.upcastToPlayerInfoNode()
                                if elem.playerGetName() == "":
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
                            else:
                                removelist.insert(0,idx)
                        else:
                            removelist.insert(0,idx)
        elif self.BKFolderListOrder[self.BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
            # look for KIMail from non-Buddies if they only want KIMail from buddies
            vault = ptVault()
            inbox = vault.getInbox()
            buddies = vault.getBuddyListFolder()
            ignores = vault.getIgnoreListFolder()

            for idx in range(len(self.BKContentList)):
                ref = self.BKContentList[idx]
                if ref is not None:
                    if ref.getSaver() is None or ref.getSaverID() == 0:
                        continue

                    if (self.onlyGetPMsFromBuddies and not buddies.playerlistHasPlayer(ref.getSaverID())) or ignores.playerlistHasPlayer(ref.getSaverID()):
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
            del self.BKContentList[removeidx]

        if self.BKFolderListOrder[self.BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
            self.BKContentList = self.markerJoinRequests + self.BKContentList
            # also add in the GlobalInbox stuff here
            vault = ptVault()
            ginbox = vault.getGlobalInbox()
            if type(ginbox) != type(None):
                self.BKContentList = ginbox.getChildNodeRefList() + self.BKContentList
                self.BKContentList.sort(CMPNodeDate)

        removelist = []
        for contentidx in range(len(self.BKContentList)):
            content = self.BKContentList[contentidx]
            if isinstance(content, ptVaultNodeRef):
                element = content.getChild()
                if type(element) != type(None):
                    if element.getType() == PtVaultNodeTypes.kFolderNode:
                        removelist.insert(0,contentidx)
        for removeidx in removelist:
            del self.BKContentList[removeidx]

    def IBigKIRefreshContentListDisplay(self):
        "display the contents of the folder that is selected, if we are in list mode"
        if self.BKRightSideMode == kGUI.BKListMode:
            createfield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(kGUI.BKILMTitleCreateLine))
            createBtn = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(kGUI.BKIListModeCreateBtn))
            try:
                if self.BKFolderLineDict is self.BKPlayerFolderDict:
                    # there might be something different here in the future...
                    if self.BKFolderListOrder[self.BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder):
                        createfield.setStringW(PtGetLocalizedString("KI.Player.CreateBuddyTitle"))
                        createBtn.show()
                    else:
                        createfield.setString(" ")
                        createBtn.hide()
                elif self.BKFolderListOrder[self.BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
                    createfield.setString(" ")
                    createBtn.hide()
                else:
                    createfield.setString(" ")
                    createBtn.hide()
            except IndexError:
                createfield.setString(" ")
                createBtn.hide()
                if len(self.BKFolderListOrder) != 0:
                    PtDebugPrint("xKI: Index error self.BKFolderSelected=%d and list=" % (self.BKFolderSelected),self.BKFolderListOrder,level=kWarningLevel)
                return
            id = kGUI.BKILMOffsetLine01
            if len(self.BKContentList) != 0:
                # make sure that the top
                if self.BKContentListTopLine >= len(self.BKContentList):
                    self.BKContentListTopLine = len(self.BKContentList)-1
                for content in self.BKContentList[self.BKContentListTopLine:]:
                    if type(content) != type(None):
                        # put on the list line
                        contentIconJ = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(id+kGUI.BKILMIconJournalOffset))
                        contentIconAva = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(id+kGUI.BKILMIconPersonOffset))
                        contentIconP = ptGUIControlDynamicText(KIListModeDialog.dialog.getControlFromTag(id+kGUI.BKILMIconPictureOffset))
                        contentTitle = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(id+kGUI.BKILMTitleOffset))
                        contentDate = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(id+kGUI.BKILMDateOffset))
                        contentFrom = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(id+kGUI.BKILMFromOffset))
                        if isinstance(content,QuestionNote):
                            contentIconJ.show()
                            contentIconP.hide()
                            contentIconAva.hide()
                            contentTitle.setForeColor(kColors.DniSelectable)
                            contentTitle.setString(xCensor.xCensor(content.title,self.censorLevel))
                            contentTitle.show()
                            contentDate.setString(" ")
                            contentDate.hide()
                            contentFrom.setForeColor(kColors.DniSelectable)
                            contentFrom.setFontSize(10)
                            contentFrom.setString(xCensor.xCensor(content.game.master.player.getPlayerName(),self.censorLevel))
                            contentFrom.show()
                            # find the button to enable it
                            lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((id-100)/10)+kGUI.BKIListModeCreateBtn))
                            lmbutton.show()
                            id += 10
                            if id > kGUI.BKILMOffsetLineLast:
                                break
                        elif isinstance(content,ptPlayer):
                            contentIconAva.show()
                            contentIconJ.hide()
                            contentIconP.hide()
                            contentTitle.setForeColor(kColors.DniSelectable)
                            contentTitle.setString(xCensor.xCensor(content.getPlayerName(),self.censorLevel))
                            contentTitle.show()
                            contentDate.hide()
                            contentFrom.setForeColor(kColors.DniSelectable)
                            contentFrom.setFontSize(10)
                            contentFrom.setString(self.GetAgeName())
                            contentFrom.show()
                            # find the button to enable it
                            lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((id-100)/10)+kGUI.BKIListModeCreateBtn))
                            lmbutton.show()
                            id += 10
                            if id > kGUI.BKILMOffsetLineLast:
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
                                            dynmap.drawImage(kGUI.BKIImageStartX,kGUI.BKIImageStartY,image,0)
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
                                    contentTitle.setForeColor(kColors.DniSelectable)
                                    contentTitle.setString(xCensor.xCensor(element.playerGetName(),self.censorLevel))
                                    contentTitle.show()
                                    contentDate.hide()
                                    contentFrom.setForeColor(kColors.DniSelectable)
                                    contentFrom.setFontSize(10)
                                    if element.playerIsOnline():
                                        contentFrom.setString(self.FilterAgeName(element.playerGetAgeInstanceName()))
                                    else:
                                        contentFrom.setString("  ")
                                    contentFrom.show()
                                else:
                                    # else its an image or a text note
                                    if content.getSaverID() == 0:
                                        # must be from the DRC!
                                        contentTitle.setForeColor(kColors.DniStatic)
                                        contentDate.setForeColor(kColors.DniStatic)
                                    else:
                                        contentTitle.setForeColor(kColors.DniSelectable)
                                        contentDate.setForeColor(kColors.DniSelectable)

                                    if isinstance(element,ptVaultImageNode):
                                        preText = ""
                                        #if not content.beenSeen():
                                        #    preText = "*"
                                        contentTitle.setString(preText+xCensor.xCensor(element.imageGetTitle(),self.censorLevel))
                                    elif isinstance(element,ptVaultTextNoteNode):
                                        preText = ""
                                        #if not content.beenSeen():
                                        #    preText = "*"
                                        contentTitle.setString(preText+xCensor.xCensor(element.noteGetTitle(),self.censorLevel))

                                    elif isinstance(element,ptVaultMarkerGameNode):
                                        contentTitle.setString(xCensor.xCensor(element.getGameName(),self.censorLevel))

                                    else:
                                        #We'll assume that it's still downloading due to network lag and hope for the best!
                                        contentTitle.setString("--[Downloading]--")
                                        contentTitle.setForeColor(DniYellow)

                                        PtDebugPrint("xKI: error - unknown data type in content list. type=%d"%(element.getType()),element,level=kErrorLevel)
                                    contentTitle.show()
                                    try:
                                        tuptime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
                                        curtime = time.strftime(PtGetLocalizedString("Global.Formats.Date"),tuptime)
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
                                            contentFrom.setForeColor(kColors.DniStatic)
                                            contentFrom.setFontSize(13)
                                            contentFrom.setString("DRC")
                                        else:
                                            contentFrom.setForeColor(kColors.DniSelectable)
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
                                lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((id-100)/10)+kGUI.BKIListModeCreateBtn))
                                lmbutton.show()
                                id += 10
                                if id > kGUI.BKILMOffsetLineLast:
                                    break
                            else:
                                PtDebugPrint("bigKI: no element inside the content. Doh!",level=kErrorLevel)
                    else:
                        PtDebugPrint("bigKI: no content, even though the folder said it was!",level=kErrorLevel)
            else:
                pass
            # set the up and down buttons if needed
            upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMUpButton))
            if self.BKContentListTopLine > 0:
                upbtn.show()
            else:
                upbtn.hide()
            dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMDownButton))
            if id > kGUI.BKILMOffsetLineLast:         # have we filled up the listbox?
                # yes, then we need down arrow
                dwnbtn.show()
            else:
                dwnbtn.hide()
            # if there are more content lines, fill them out to be blank
            #   and disable the button fields while you're at it
            for tagid in range(id,kGUI.BKILMOffsetLineLast+10,10):
                iconpic = ptGUIControlDynamicText(KIListModeDialog.dialog.getControlFromTag(tagid+kGUI.BKILMIconPictureOffset))
                iconpic.hide()
                iconjrn = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(tagid+kGUI.BKILMIconJournalOffset))
                iconjrn.hide()
                iconava = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(tagid+kGUI.BKILMIconPersonOffset))
                iconava.hide()
                titlefield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagid+kGUI.BKILMTitleOffset))
                titlefield.hide()
                datefield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagid+kGUI.BKILMDateOffset))
                datefield.hide()
                fromfield = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagid+kGUI.BKILMFromOffset))
                fromfield.hide()
                # find the button to disable it
                lmbutton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((tagid-100)/10)+kGUI.BKIListModeCreateBtn))
                lmbutton.hide()

    def IBigKIDisplayCurrentContentJournal(self):
        "Display a text journal note entry"
        jrnAgeName = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNAgeName))
        jrnAgeName.hide()
        jrnDate = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNDate))
        jrnDate.hide()
        jrnTitle = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNTitle))
        jrnTitle.hide()
        jrnNote = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNNote))
        jrnNote.hide()
        jrnNote.setBufferLimit(kJournalTextSize)
        jrnDeleteBtn = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNDeleteButton))
        jrnDeleteBtn.hide()
        jrnTitleBtn = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNTitleButton))
        if type(self.BKCurrentContent) != type(None):
            if self.IsContentMutable(self.BKCurrentContent):
                jrnDeleteBtn.show()
                jrnNote.unlock()
                if BKInEditMode and BKEditField == kGUI.BKEditFieldJRNTitle:
                    pass
                else:
                    jrnTitleBtn.show()
            else:
                jrnNote.lock()
                jrnTitleBtn.hide()
            element = self.BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kTextNoteNode:
                    element = element.upcastToTextNoteNode()
                    # display the content on the screen
                    jrnAgeName.setString(self.FilterAgeName(xCensor.xCensor(element.getCreateAgeName(),self.censorLevel)))
                    jrnAgeName.show()
                    tuptime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
                    curtime = time.strftime(PtGetLocalizedString("Global.Formats.Date"),tuptime)
                    jrnDate.setString(curtime)
                    jrnDate.show()
                    if BKInEditMode and BKEditField == kGUI.BKEditFieldJRNTitle:
                        pass
                    else:
                        jrnTitle.setString(xCensor.xCensor(element.noteGetTitle(),self.censorLevel))
                        jrnTitle.show()
                    if BKInEditMode and BKEditField == kGUI.BKEditFieldJRNNote:
                        pass
                    else:
                        encoded = buffer(xCensor.xCensor(element.noteGetText(),self.censorLevel))
                        jrnNote.setEncodedBuffer(encoded)
                        jrnNote.show()
                    self.IBigKISetSeen(self.BKCurrentContent)
                    # if this came from someone else (and its in the Incoming folder?)...
                    # ... then set the SendTo field so we can reply to them
                    self.ICheckContentForSender(self.BKCurrentContent)
                else:
                    PtDebugPrint("xBigKI: Display current content - wrong element type %d" % (datatype),level=kErrorLevel)
            else:
                PtDebugPrint("xBigKI: Display current content - element is None",level=kErrorLevel)
        else:
            PtDebugPrint("xBigKI: Display current content - self.BKCurrentContent is None",level=kErrorLevel)

    def IBigKIDisplayCurrentContentImage(self):
        "Display an image(picture) entry"
        picAgeName = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKIPICAgeName))
        picAgeName.hide()
        picDate = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKIPICDate))
        picDate.hide()
        picTitle = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKIPICTitle))
        picTitle.hide()
        picImage = ptGUIControlDynamicText(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKIPICImage))
        picImage.hide()
        picDeleteBtn = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKIPICDeleteButton))
        picDeleteBtn.hide()
        picTitleBtn = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKIPICTitleButton))
        if type(self.BKCurrentContent) != type(None):
            if self.IsContentMutable(self.BKCurrentContent):
                picDeleteBtn.show()
                if BKInEditMode and BKEditField == kGUI.BKEditFieldPICTitle:
                    pass
                else:
                    picTitleBtn.show()
            else:
                picTitleBtn.hide()
            element = self.BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kImageNode:
                    element = element.upcastToImageNode()
                    # display the content on the screen
                    picAgeName.setString(self.FilterAgeName(xCensor.xCensor(element.getCreateAgeName(),self.censorLevel)))
                    picAgeName.show()
                    tuptime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
                    curtime = time.strftime(PtGetLocalizedString("Global.Formats.Date"),tuptime)
                    picDate.setString(curtime)
                    picDate.show()
                    if BKInEditMode and BKEditField == kGUI.BKEditFieldPICTitle:
                        pass
                    else:
                        picTitle.setString(xCensor.xCensor(element.imageGetTitle(),self.censorLevel))
                        picTitle.show()
                    if picImage.getNumMaps() > 0:
                        dynmap = picImage.getMap(0)
                        image = element.imageGetImage()
                        dynmap.clearToColor(ptColor(.1,.1,.1,.3))
                        if type(image) != type(None):
                            dynmap.drawImage(kGUI.BKIImageStartX,kGUI.BKIImageStartY,image,0)
                        else:
                            dynmap.fillRect(kGUI.BKIImageStartX,kGUI.BKIImageStartY,kGUI.BKIImageStartX+800,kGUI.BKIImageStartY+600,ptColor(.2,.2,.2,.1))
                        dynmap.flush()
                    picImage.show()
                    self.IBigKISetSeen(self.BKCurrentContent)
                    # if this came from someone else (and its in the Incoming folder?)...
                    # ... then set the SendTo field so we can reply to them
                    self.ICheckContentForSender(self.BKCurrentContent)
                else:
                    PtDebugPrint("xBigKI: Display current content - wrong element type %d" % (datatype),level=kErrorLevel)
            else:
                PtDebugPrint("xBigKI: Display current content - element is None",level=kErrorLevel)
        else:
            PtDebugPrint("xBigKI: Display current content - self.BKCurrentContent is None",level=kErrorLevel)

    def IBigKIDisplayCurrentContentPlayer(self):
        "Display a player element entry"
        plyName = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYName))
        plyName.hide()
        plyID = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYID))
        plyID.hide()
        plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYPlayerIDEditBox))
        plyIDedit.hide()
        plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDetail))
        plyDetail.hide()
        plyDeleteBtn = ptGUIControlButton(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDeleteButton))
        plyDeleteBtn.hide()
        # are we asking for a player id number?
        if self.BKGettingPlayerID:
            plyName.setStringW(PtGetLocalizedString("KI.Player.EnterID"))
            plyName.show()
            plyIDedit.setString("")
            plyIDedit.show()
            plyIDedit.focus()
            KIPlayerExpanded.dialog.setFocus(plyIDedit.getKey())
        else:
            if type(self.BKCurrentContent) != type(None):
                if isinstance(self.BKCurrentContent,ptPlayer):
                    # display the content on the screen
                    plyName.setString(xCensor.xCensor(self.BKCurrentContent.getPlayerName(),self.censorLevel))
                    plyName.show()
                    idtext = "%08d" % (self.BKCurrentContent.getPlayerID())
                    plyID.setString(idtext)
                    plyID.show()
                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.InAge", [self.GetAgeName()]))
                    plyDetail.show()
                    sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                    self.BKPlayerSelected = self.BKCurrentContent
                    sendToField.setString(self.BKCurrentContent.getPlayerName())
                else:
                    element = self.BKCurrentContent.getChild()
                    if type(element) != type(None):
                        datatype = element.getType()
                        if datatype == PtVaultNodeTypes.kPlayerInfoNode:
                            element = element.upcastToPlayerInfoNode()
                            # display the content on the screen
                            plyName.setString(xCensor.xCensor(element.playerGetName(),self.censorLevel))
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
                                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.InAge", [self.FilterAgeName(element.playerGetAgeInstanceName())]))
                            else:
                                plyDetail.setStringW(PtGetLocalizedString("KI.Player.Offline"))
                            plyDetail.show()
                            # determine if this player can be removed from this folder
                            folder = self.BKCurrentContent.getParent()
                            if folder: folder = folder.upcastToFolderNode()
                            if folder and self.IsFolderContentsMutable(folder):
                                plyDeleteBtn.show()
                            sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                            self.BKPlayerSelected = self.BKCurrentContent
                            sendToField.setString(element.playerGetName())
                        else:
                            PtDebugPrint("xBigKI: Display current content - wrong element type %d" % (datatype),level=kErrorLevel)
                    else:
                        PtDebugPrint("xBigKI: Display current content - element is None",level=kErrorLevel)
            else:
                PtDebugPrint("xBigKI: Display current content - self.BKCurrentContent is None",level=kErrorLevel)

    def IBigKIDisplayCurrentContentMarkerFolder(self):
        "Prepares to display a marker game, as we may need to load it"

        #Ensure that we can even view this game....
        if not self.phasedKIPlayMarkerGame or not self.phasedKIShowMarkerGame or self.gKIMarkerLevel < kKIMarkerNormalLevel:
            #Let the user know the KI isn't configured to view this game...
            self.pendingMGaction = PtGetLocalizedString("KI.MarkerGame.pendingActionUpgradeKI")
            self.IShowMarkerGameLoading()
            return

        #Initialize the markerGameDisplay to the currently selected game
        #But first, we need to ensure that we've got everything necessary!
        if self.BKCurrentContent is None:
            PtDebugPrint("ERROR: xKI.IBigKIDisplayCurrentContentMarkerFolder(): Could not find the current content selected!")
            return

        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint("ERROR: xKI.IBigKIDisplayCurrentContentMarkerFolder(): Could not find the current content child node!")
            return

        datatype = element.getType()
        if datatype != PtVaultNodeTypes.kMarkerGameNode:
            PtDebugPrint("ERROR: xKI.IBigKIDisplayCurrentContentMarkerFolder(): Cannot process this node, wrong data type: %s!" %element.getType())
            return

        element = element.upcastToMarkerGameNode()
        PtDebugPrint("DEBUG: xKI.IBigKIDisplayCurrentContentMarkerFolder(): Starting Marker Game KI Display Manager, loading game: %s guid: %s" %(element.getGameName(), element.getGameGuid()))


        #Now that we're here there's two possibilities: 1) we've just created a game and trying to display it, 2) we've clicked on an existing game
        if self.markerGameDisplay is not None:
            if element.getGameGuid() == self.markerGameDisplay.gameData.data['svrGameTemplateID']:
                # just display the details!
                self.IFinishDisplayCurrentMarkerGame()
                return

        #wait, if the currently played game is the game we're editing, we must load it's data
        if self.markerGameManager is not None and self.markerGameManager.gameLoaded():
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
        mbtnEditStart.hide()
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
        if self.pendingMGaction is not None:
            msg = "----" + self.pendingMGaction + "----"
        else:
            msg = "Please Wait"

        mrkfldTitle.setStringW(msg)
        mrkfldTitle.show()
        mrkfldTitle.refresh()


    def IFinishDisplayCurrentMarkerGame(self):
        "Displays the loaded marker game to the KI"

        #Our game should be loaded, so let's make sure just to be safe!
        if self.markerGameDisplay == None:
            PtDebugPrint("ERROR: xKI.IFinishDisplayCurrentMarkerGame(): Game was not loaded, aborting displaying the game's details!")
            return

        #Get the marker vault node
        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint("ERROR: xKI.IFinishDisplayCurrentMarkerGame(): Could not finish displaying the marker game, as we the vault node is empty!")
            return
        datatype = element.getType()
        if datatype != PtVaultNodeTypes.kMarkerGameNode:
            PtDebugPrint("ERROR: xKI.IFinishDisplayCurrentMarkerGame(): Could not finish displaying the marker game, as we have an incorrect vault node type!")
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

        self.IBigKISetToButtons()
        mbtnEditStart = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderEditStartGame))
        mbtnPlayEnd = ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderPlayEndGame))
        mrkfldOwner = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderOwner))
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
            mbtnGameTimeArrow.hide()
            # if this is a quest game then check to make sure that this IS the owner
            # ..and if it is not a quest game then we must be in the same age as the markerfolder
            if self.phasedKIPlayMarkerGame and (not isQuestGame or  element.getCreatorNodeID() == PtGetLocalPlayer().getPlayerID())\
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
            if self.phasedKIPlayMarkerGame and (isQuestGame or PtGetAgeInfo().getAgeInstanceName() == element.getCreateAgeName()):
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
                        mlbMarkerList.addString("[%s:%d,%d,%d] %s" % (self.FilterAgeName(marker['age']), torans, hSpans, vSpans, marker['name']))
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

                    mtbToran.setString(str(coord.getTorans()))
                    mtbHSPan.setString(str(coord.getHSpans()))
                    mtbVSpan.setString(str(coord.getVSpans()))
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
                    mlbMarkerList.addString("[%s:%d,%d,%d] %s" % (self.FilterAgeName(marker['age']),coord.getTorans(), coord.getHSpans(), coord.getVSpans(), marker['name']))
                else:
                    questGameFinished = 0
            mlbMarkerTextTB.hide()
        # refresh the text of the buttons (color changed)
        mtbEditStart.refresh()
        mtbPlayEnd.refresh()
        # display the content on the screen
        mrkfldTitle.setString(xCensor.xCensor(mgData['svrGameName'],self.censorLevel))
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
                statusLine += PtGetLocalizedString("KI.MarkerGame.StatusIn", [self.FilterAgeName(element.getCreateAgeName())])
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
        if isQuestGame:
            gameType = PtGetLocalizedString("KI.MarkerGame.QuestGame")
            mtbGameTime.hide()
            mtbGameTimeTitle.hide()


    def IBigKIDisplayCurrentQuestionNote(self):
        "Display a question note"
        qnTitle = ptGUIControlTextBox(KIQuestionNote.dialog.getControlFromTag(kQNTitle))
        qnNote = ptGUIControlMultiLineEdit(KIQuestionNote.dialog.getControlFromTag(kQNMessage))
        qnAcceptTB = ptGUIControlTextBox(KIQuestionNote.dialog.getControlFromTag(kQNAcceptText))
        qnDeclineTB = ptGUIControlTextBox(KIQuestionNote.dialog.getControlFromTag(kQNDeclineText))
        if type(self.BKCurrentContent) != type(None):
            if isinstance(self.BKCurrentContent,QuestionNote):
                # display the content on the screen
                qnTitle.setString(self.BKCurrentContent.title)
                qnNote.setString(self.BKCurrentContent.message)
                qnAcceptTB.setString(self.BKCurrentContent.YesBtnText)
                qnDeclineTB.setString(self.BKCurrentContent.NoBtnText)
            else:
                PtDebugPrint("xBigKI:QuestionNote: Unknown data type",level=kErrorLevel)
        else:
            PtDebugPrint("xBigKI:QuestionNote: Display current content - self.BKCurrentContent is None",level=kErrorLevel)

    def IBigKICheckSavePlayer(self):
        "save after player edited"
        if self.BKGettingPlayerID:
            # this should create and save a player element into buddies
            self.BKGettingPlayerID = False
            plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYPlayerIDEditBox))
            if not plyIDedit.wasEscaped():
                id,msg = self.chatMgr.GetPIDMsg(plyIDedit.getString())
                if id:
                    localplayer = PtGetLocalPlayer()
                    if id != localplayer.getPlayerID():
                        vault = ptVault()
                        buddies = vault.getBuddyListFolder()
                        if type(buddies) != type(None):
                            if buddies.playerlistHasPlayer(id):
                                plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDetail))
                                plyDetail.setStringW(PtGetLocalizedString("KI.Player.AlreadyAdded"))
                                plyDetail.show()
                                self.BKGettingPlayerID = True   # back in business... of asking them for a number
                            else:
                                buddies.playerlistAddPlayer(id)
                                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Added"))
                        if not self.BKGettingPlayerID:
                            self.IBigKIChangeMode(kGUI.BKListMode)
                    else:
                        plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDetail))
                        plyDetail.setStringW(PtGetLocalizedString("KI.Player.NotYourself"))
                        plyDetail.show()
                        self.BKGettingPlayerID = True   # back in business... of asking them for a number
                else:
                    plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDetail))
                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.NumberOnly"))
                    plyDetail.show()
                    self.BKGettingPlayerID = True   # back in business... of asking them for a number
            else:
                # nothing here... just go back to list mode
                self.IBigKIChangeMode(kGUI.BKListMode)

    def IBigKICreateJournalNote(self):
        "create a note and add to the Journal"
        PtDebugPrint("xBigKI: create text note message",level=kDebugDumpLevel)
        # if there is no folder list, then make one
        if len(self.BKFolderListOrder) == 0:
            self.IBigKIRefreshFolders()
        try:
            journal = self.BKJournalFolderDict[self.IGetAgeInstanceName()]
            if type(journal) != type(None):
                # make sure that the age folder is selected
                self.BKFolderTopLine = self.BKJournalFolderTopLine = 0      # scroll back to the top
                self.BKFolderSelected = self.BKJournalFolderSelected = self.BKJournalListOrder.index(self.IGetAgeInstanceName())
                # create the note
                note = ptVaultTextNoteNode(0)

                note.setTextW(PtGetLocalizedString("KI.Journal.InitialMessage"))
                note.setTitleW(PtGetLocalizedString("KI.Journal.InitialTitle"))

                self.BKCurrentContent = journal.addNode(note)
                return self.BKCurrentContent
            else:
                PtDebugPrint("xBigKI: create journal note, journal not ready",level=kErrorLevel)
                return None
        except KeyError:
            PtDebugPrint("xKI:BigKI - could not find journal for this age -%s" % (self.IGetAgeInstanceName()),level=kErrorLevel)

    def IBigKICreateJournalImage(self,image,useScreenShot = false):
        "create a journal entry that is a picture"
        PtDebugPrint("xBigKI: create a picture element from ",image,level=kDebugDumpLevel)
        # if there is no folder list, then make one
        if len(self.BKFolderListOrder) == 0:
            self.IBigKIRefreshFolders()
        try:
            journal = self.BKJournalFolderDict[self.IGetAgeInstanceName()]
            if type(journal) != type(None):
                # make sure that the age folder is selected
                self.BKFolderTopLine = self.BKJournalFolderTopLine = 0      # scroll back to the top
                self.BKFolderSelected = self.BKJournalFolderSelected = self.BKJournalListOrder.index(self.IGetAgeInstanceName())
                # create the image entry
                img_elem = ptVaultImageNode(0)
                if useScreenShot:
                    img_elem.setImageFromScrShot()
                else:
                    img_elem.imageSetImage(image)
                img_elem.setTitleW(PtGetLocalizedString("KI.Image.InitialTitle"))
                self.BKCurrentContent = journal.addNode(img_elem)
                return self.BKCurrentContent
            else:
                PtDebugPrint("xBigKI: create journal image, journal not ready",level=kErrorLevel)
                return None
        except KeyError:
            PtDebugPrint("xKI:BigKI - could not find journal for this age -%s" % (self.IGetAgeInstanceName()),level=kErrorLevel)

    def IBigKIEnterEditMode(self,whichfield):
        "enter into edit mode for a particular field"
        # can't be in chatting mode
        self.chatMgr.ToggleChatMode(0)
        # see if we were already in edit mode, save that before re-entrying into edit mode
        if BKInEditMode:
            self.IBigKISaveEdit()
        if whichfield == kGUI.BKEditFieldJRNTitle:
            textbox = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichfield][kGUI.BKEditIDtextbox]))
            button = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichfield][kGUI.BKEditIDbutton]))
            editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichfield][kGUI.BKEditIDeditbox]))
        elif whichfield == kGUI.BKEditFieldPICTitle:
            textbox = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichfield][kGUI.BKEditIDtextbox]))
            button = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichfield][kGUI.BKEditIDbutton]))
            editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichfield][kGUI.BKEditIDeditbox]))
            editbox.setStringSize(56)
        else:
            textbox = None
            button = None
            editbox = None
        # make sure that we have a valid thing to edit
        if type(textbox) != type(None):
            if type(self.BKCurrentContent) != type(None):
                ed_element = self.BKCurrentContent.getChild()
            else:
                ed_element = None
            if type(ed_element) != type(None):
                BKInEditMode = True
                BKEditContent = self.BKCurrentContent
                BKEditField = whichfield
                # hide the textbox and the button
                textbox.hide()
                button.hide()
                # set the edit box and display it
                if BKEditField == kGUI.BKEditFieldJRNTitle:
                    ed_element = ed_element.upcastToTextNoteNode()
                    editbox.setString(xCensor.xCensor(ed_element.noteGetTitle(),self.censorLevel))
                    KIJournalExpanded.dialog.setFocus(editbox.getKey())
                elif BKEditField == kGUI.BKEditFieldPICTitle:
                    ed_element = ed_element.upcastToImageNode()
                    editbox.setString(xCensor.xCensor(ed_element.imageGetTitle(),self.censorLevel))
                    KIPictureExpanded.dialog.setFocus(editbox.getKey())
                else:
                    editbox.setString("")
                editbox.end()
                editbox.show()
                editbox.focus()
                if whichfield == kGUI.BKEditFieldJRNTitle or whichfield == kGUI.BKEditFieldJRNNote:
                    KIJournalExpanded.dialog.refreshAllControls()
                elif whichfield == kGUI.BKEditFieldPICTitle:
                    KIPictureExpanded.dialog.refreshAllControls()
            else:
                PtDebugPrint("xKI:BigKI:EnterEdit content has no element to edit?")
        else:
            # might be for the journal edit?
            if whichfield == kGUI.BKEditFieldJRNNote:
                # if so, then its kinda automatically in edit mode
                BKInEditMode = True
                BKEditContent = self.BKCurrentContent
                BKEditField = whichfield

    def IBigKISaveEdit(self, noExitEditMode = 0):
        "save whatever they were editing to the right place"

        if BKInEditMode:
            if BKEditField == kGUI.BKEditFieldJRNTitle:
                textbox = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDtextbox]))
                button = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDbutton]))
                editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDeditbox]))
            elif BKEditField == kGUI.BKEditFieldJRNNote:
                textbox = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDtextbox]))
                button = None
                editbox = None
            elif BKEditField == kGUI.BKEditFieldPICTitle:
                textbox = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDtextbox]))
                button = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDbutton]))
                editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDeditbox]))
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
                                if BKEditField == kGUI.BKEditFieldJRNTitle:
                                    ed_element = ed_element.upcastToTextNoteNode()
                                    jtitle = editbox.getStringW()

                                    if jtitle[:len(PtGetLocalizedString("KI.Journal.InitialTitle"))] == PtGetLocalizedString("KI.Journal.InitialTitle"):
                                        # make sure that they actually added something (so as not to get a blank title)
                                        if jtitle != PtGetLocalizedString("KI.Journal.InitialTitle"):
                                            jtitle = jtitle[len(PtGetLocalizedString("KI.Journal.InitialTitle")):]
                                    ed_element.setTitleW(jtitle)
                                elif BKEditField == kGUI.BKEditFieldPICTitle:
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
                            if BKEditField == kGUI.BKEditFieldJRNNote:
                                buf = textbox.getEncodedBufferW()

                                if buf[:len(PtGetLocalizedString("KI.Journal.InitialMessage"))] == PtGetLocalizedString("KI.Journal.InitialMessage"):
                                    buf = buf[len(PtGetLocalizedString("KI.Journal.InitialMessage")):]
                                ed_element = ed_element.upcastToTextNoteNode()
                                ed_element.setTextW(buf)
                                # save 'em
                                ed_element.save()
                if BKEditField != kGUI.BKEditFieldJRNNote:
                    # put back the fields in to no-edit mode
                    textbox.show()
                    button.show()
                    editbox.hide()
            if not noExitEditMode:
                # take us out of editing
                BKInEditMode = False
                BKEditContent = None
                BKEditField = -1

    def IBigKICheckFocusChange(self):
        "The focus has changed, see if we need to close the editing"
        if BKInEditMode:
            if BKEditField == kGUI.BKEditFieldJRNTitle:
                editbox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDeditbox]))
            elif BKEditField == kGUI.BKEditFieldPICTitle:
                editbox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[BKEditField][kGUI.BKEditIDeditbox]))
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
        # if they are no longer around, remove the evidence
        self.BKPlayerSelected = None
        sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
        sendToField.setStringW(U"<"+unicode(message)+U">")
        sendToButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        sendToButton.hide()

    def ICheckContentForSender(self,content):
        "Check incoming content to see if there is a sender (and from Incoming) and set SendTo field"
        folder = content.getParent()
        if folder: folder = folder.upcastToFolderNode()
        if type(folder) != type(None) and folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
            sender = content.getSaver()
            if type(sender) != type(None) and sender.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                curSendTo = sendToField.getString().strip()
                if curSendTo == "" or len(curSendTo) <= 0:
                    self.BKPlayerSelected = sender
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
        invite.noteSetText(PtGetLocalizedString(message, [ageName,localPlayer.getPlayerName()]))
        invite.noteSetTitle(PtGetLocalizedString(title, [ageName]))
        invite.sendTo(playerID)

    def IShowSelectedConfig(self):
        "display the config dialog that is selected"
        if self.BKConfigListOrder[self.BKFolderSelected] == PtGetLocalizedString("KI.Config.Settings"):
            self.IBigKIChangeMode(kGUI.BKKIExpanded)
        elif self.BKConfigListOrder[self.BKFolderSelected] == PtGetLocalizedString("KI.Config.Volume"):
            self.IBigKIChangeMode(kGUI.BKVolumeExpanded)
        else:
            # is the dialog already showing
            if self.BKRightSideMode != kGUI.BKAgeOwnerExpanded:
                # nope then show it
                self.IBigKIChangeMode(kGUI.BKAgeOwnerExpanded)
            else:
                # else just refresh what's in the dialog
                self.IRefreshAgeOwnerSettings()
                self.IBigKIOnlySelectedToButtons()

    def ISaveUserNameFromEdit(self,control):
        newtitle = ""
        try:
            # get the selected age config setting
            myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
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
        title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleTB))
        title.setStringW(newtitle)
        title.show()
        titlebtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleBtn))
        titlebtn.enable()

    def ISaveMarkerFolderNameFromEdit(self,control):
        title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderTitleText))
        if type(self.BKCurrentContent) != type(None):
            element = self.BKCurrentContent.getChild()
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
                            newText = xCensor.xCensor(control.getString(),self.censorLevel)
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
        title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kMarkerFolderMarkerTextTB))
        if type(self.BKCurrentContent) != type(None):
            element = self.BKCurrentContent.getChild()
            if type(element) != type(None):
                datatype = element.getType()
                if datatype == PtVaultNodeTypes.kMarkerGameNode:
                    element = element.upcastToMarkerGameNode()
                    if type(element) != type(None):
                        if not control.wasEscaped() and control.getString() != "":
                            newText = xCensor.xCensor(control.getString(),self.censorLevel)
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
        if self.KILevel >= kNormalKI:
            PtFlashWindow()
            if not self.alertTimerActive:
                PtDebugPrint("xKI: show KI alert",level=kDebugDumpLevel)
                NewItemAlert.dialog.show()
            kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
            self.alertTimeToUse = kAlertTimeDefault
            kialert.show()

    def IAlertBookStart(self,time=kAlertTimeDefault):
        "start the alert, unless its already going"
        if not self.alertTimerActive:
            PtDebugPrint("xKI: show Book alert",level=kDebugDumpLevel)
            NewItemAlert.dialog.show()
        bookalert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
        self.alertTimeToUse = time
        bookalert.show()

    def IAlertStop(self):
        "stop the alert, ie. hide the alert dialog"
        self.alertTimerActive = False
        NewItemAlert.dialog.hide()
        kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
        kialert.hide()
        bookalert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
        bookalert.hide()

    def IAlertStartTimer(self):
        "start the alert timer, unless already going"
        if not self.alertTimerActive:
            self.alertTimerActive = True
            PtAtTimeCallback(self.key,self.alertTimeToUse,kTimers.AlertHide)


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



    def _IProcScoreOps(self):
        if not len(self.scoreOps):
            self._scoreOpCur = kPellets.ScoreNoOp
            return

        self._scoreOpCur = self.scoreOps.pop(0)
        if self._scoreOpCur == kPellets.ScoreFetchForDisplay:
            ptGameScore.findPlayerScores("PelletDrop", self.key)
        elif self._scoreOpCur == kPellets.ScoreFetchMineForUpload:
            ptGameScore.findPlayerScores("PelletDrop", self.key)
        elif self._scoreOpCur == kPellets.ScoreFetchUploadDestination:
            ptGameScore.findAgeScores("PelletDrop", self.key)
        elif self._scoreOpCur == kPellets.ScoreCreateUploadDestination:
            ptGameScore.createAgeScore("PelletDrop", PtGameScoreTypes.kAccumulative, 0, self.key)
        elif self._scoreOpCur == kPellets.ScoreTransfer:
            self._scoreSource.transferPoints(self._scoreDestination, key=self.key)



    def DoScoreOp(self, op):
        self.scoreOps.append(op)
        if self._scoreOpCur == kPellets.ScoreNoOp:
            self._IProcScoreOps()


    def OnGameScoreMsg(self, msg):
        if isinstance(msg, ptGameScoreListMsg):
            pelletTextBox = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPelletDrop))
            try:
                score = msg.getScores()[0]
                points = score.getPoints()

                if self._scoreOpCur == kPellets.ScoreFetchForDisplay:
                    if points < 0:
                        points = 0 # Hmmm...
                    pelletTextBox.setString(str(points))
                    PtDebugPrint("xKI.OnGameScoreMsg(): PelletDrop score... %i" % points, level=kWarningLevel)
                elif self._scoreOpCur == kPellets.ScoreFetchMineForUpload:
                    self._scoreSource = score
                    self.DoScoreOp(kPellets.ScoreFetchUploadDestination)
                elif self._scoreOpCur == kPellets.ScoreFetchUploadDestination:
                    self._scoreDestination = score
                    self._scoreUploaded = self._scoreSource.getPoints()
                    self.DoScoreOp(kPellets.ScoreTransfer)
            except:
                if self._scoreOpCur == kPellets.ScoreFetchForDisplay:
                    pelletTextBox.setString("000")
                elif self._scoreOpCur == kPellets.ScoreFetchUploadDestination:
                    self.DoScoreOp(kPellets.ScoreCreateUploadDestination)

        elif isinstance(msg, ptGameScoreTransferMsg):
            pelletTextBox = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPelletDrop))
            pelletTextBox.setString("000")
            self.IUploadPelletScore(self._scoreUploaded)
            del self._scoreDestination
            del self._scoreSource
            self._scoreUploaded = 0

        elif isinstance(msg, ptGameScoreUpdateMsg):
            if self._scoreOpCur == kPellets.ScoreCreateUploadDestination:
                self._scoreDestination = msg.getScore()
                self._scoreUploaded = self._scoreSource.getPoints()
                self.DoScoreOp(kPellets.ScoreTransfer)

        # Proc any more queue ops
        self._IProcScoreOps()


    def IUpdatePelletScore(self, points = 0):
        pelletTextBox = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPelletDrop))
        if points:
            pelletTextBox.setString(str(points))
        else:
            pelletTextBox.setString("...") # Fetching from server...
            self.DoScoreOp(kPellets.ScoreFetchForDisplay)


    def IUploadPelletScore(self, score = None):
        if score:
            hoodinfoupdate = PtFindActivator("PythHoodInfoImagerUpdater")
            PtDebugPrint("hoodinfoupdate: ", hoodinfoupdate, level=kDebugDumpLevel)
            if hoodinfoupdate:
                notify = ptNotify(self.key)
                notify.clearReceivers()
                notify.addReceiver(hoodinfoupdate)
                notify.netPropagate(1)
                notify.netForce(1)
                notify.setActivate(1.0)
                sName = "Score=%s" % (PtGetLocalPlayer().getPlayerName())
                notify.addVarNumber(sName, score)
                notify.send()
                PtDebugPrint("sending score notify: ", sName, " ", score, level=kDebugDumpLevel)
        else:
            self.DoScoreOp(kPellets.ScoreFetchMineForUpload)

    def IAutocomplete(self, control):
        text = control.getStringW()

        proposition = autocompleteState.pickNext(text)
        if proposition is not None:
            control.setStringW(proposition)
            control.end()
            control.refresh()
            return

        players = set()

        for item in self.BKPlayerList:
            if isinstance(item, ptPlayer):
                players.add(item.getPlayerName())
            elif isinstance(item, ptVaultNodeRef):
                player = item.getChild()
                playerInfo = player.upcastToPlayerInfoNode()
                if playerInfo is not None:
                    players.add(playerInfo.playerGetName())

        proposition = autocompleteState.pickFirst(text, players)

        if proposition is not None:
            control.setStringW(proposition)
            control.end()
            control.refresh()


################################################################
##
##  class helper - DeviceFolder and Device
##
################################################################
class KIFolder:
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

class QuestionNote:
    kNotDefined=0
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
