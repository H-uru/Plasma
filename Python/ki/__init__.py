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
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaConstants import *
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

# Personal age SDL helper.
from xPsnlVaultSDL import *

# Jalak constants.
from jlakConstants import *

# xKI sub-modules.
import xKIExtChatCommands
import xKIChat
from xKIConstants import *
from xKIHelpers import *

# Marker Game thingies
import xMarkerMgr

# Define the attributes that will be entered in Max.
KIBlackbar = ptAttribGUIDialog(1, "The Blackbar dialog")
xKIChat.KIBlackbar = KIBlackbar
KIMini = ptAttribGUIDialog(2, "The KIMini dialog")
xKIChat.KIMini = KIMini
KIYesNo = ptAttribGUIDialog(3, "The KIYesNo dialog")
BigKI = ptAttribGUIDialog(5, "The BigKI (Mr. BigStuff)")
xKIChat.BigKI = BigKI
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
xKIChat.KIMicro = KIMicro
KIVolumeExpanded = ptAttribGUIDialog(21, "The volume control dialog")
KIAgeOwnerExpanded = ptAttribGUIDialog(22, "The Age Owner settings dialog")
# Disabled: KIRateIt = ptAttribGUIDialog(23, "The Rate It dialog")
KISettings = ptAttribGUIDialog(24, "The KI settings dialog")
KIMarkerFolderExpanded = ptAttribGUIDialog(27, "The Marker Folder dialog")
KIMarkerFolderPopupMenu = ptAttribGUIPopUpMenu(28, "The MarkerFolder Time Popup Menu")
# Disabled: KIQuestionNote = ptAttribGUIDialog(29, "The Question Note dialog")
# Disabled: KIMarkerTypePopupMenu = ptAttribGUIPopUpMenu(30, "The MarkerFolder Type Popup Menu")
KICreateMarkerGameGUI = ptAttribGUIDialog(31, "The Marker Game Creation GUI")
KIMarkerGameGUIOpen = ptAttribResponder(32, "Marker Game GUI Open Responder")
KIMarkerGameGUIClose = ptAttribResponder(33, "Marker Game GUI Close Responder")
KIJalakMiniIconOn = ptAttribResponder(34, "Jalak KIMini icon 'on/off' resp", ["on", "off"])
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
        PtDebugPrint(u"xKI: Max version {} - minor version {}.".format(MaxVersionNumber, MinorVersionNumber))

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

        # Transparency settings.
        self.originalForeAlpha = 1.0
        self.originalSelectAlpha = 1.0
        self.originalminiKICenter = None
        self.lastminiKICenter = None

        #~~~~~~~#
        # BigKI #
        #~~~~~~~#
        self.previousTime = "20:20"
        self.timeBlinker = True

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
        self.scoreOpCur = kPellets.ScoreNoOp
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
        self.MFScrollPos = 0

        # New Marker Game dialog globals.
        self.markerGameDefaultColor = ""
        self.markerGameSelectedColor = ""
        self.selectedMGType = 0

        # GZ missions state.
        self.gGZPlaying = 0
        self.gGZMarkerInRange = 0
        self.gGZMarkerInRangeRepy = None
        self.gMarkerToGetColor = "off"
        self.gMarkerGottenColor = "off"
        self.gMarkerToGetNumber = 0
        self.gMarkerGottenNumber = 0

        # Jalak GUI globals.
        self.jalakGUIButtons = []
        self.jalakGUIState = False
        self.jalakScript = None

        # Miscellaneous.
        self.imagerMap = {}
        self.pelletImager = ""

        ## Auto-completion manager.
        self.autocompleteState = AutocompleteState()

        ## The chatting manager.
        self.chatMgr = xKIChat.xKIChat(self.StartFadeTimer, self.ResetFadeState, self.FadeCompletely, self.GetCensorLevel)

    ## Unloads any loaded dialogs upon exit.
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
            PtDebugPrint(u"xKI: Unloading Jalak GUI dialog.", level=kWarningLevel)
            KIJalakMiniIconOn.run(self.key, state="off", netPropagate=0, fastforward=1)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).hide()
            PtUnloadDialog("jalakControlPanel")

    #~~~~~~~~~~~~~~~~~~#
    # Plasma Functions #
    #~~~~~~~~~~~~~~~~~~#

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

        self.markerGameManager = xMarkerMgr.MarkerGameManager()
        self.chatMgr.markerGameManager = self.markerGameManager

        # Pass the newly-initialized key to the modules.
        self.chatMgr.key = self.key

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
            PtDebugPrint(u"xKI.OnAccountUpdate(): Active player set. Clear to re-init KI GUI.", level=kDebugDumpLevel)
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
        # Force any open KIs to close.
        self.ToggleMiniKI()

        self.CheckKILight()

        ageName = PtGetAgeName()
        PtDebugPrint(u"xKI.OnServerInitComplete(): Age = ", ageName, level=kDebugDumpLevel)

        if ageName.lower() != "startup":
            self.markerGameManager.OnServerInitComplete()

        # Set up Jalak GUI.
        if ageName == "Jalak":
            PtDebugPrint(u"xKI.OnServerInitComplete(): Loading Jalak GUI dialog.", level=kWarningLevel)
            PtLoadDialog("jalakControlPanel", self.key)
            KIJalakMiniIconOn.run(self.key, state="on", netPropagate=0)
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).show()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
            self.AlertKIStart()
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
            PtDebugPrint(u"xKI.BeginAgeUnLoad(): Failed to get local avatar.")
            return
        if local == avObj:
            PtDebugPrint(u"xKI.BeginAgeUnLoad(): Avatar page out.", level=kDebugDumpLevel)
            curTime = PtGetDniTime()
            timeRemaining = (self.lightStop - curTime)
            if timeRemaining > 0:
                self.DoKILight(0, 1, timeRemaining)
                LocalAvatar = PtGetLocalAvatar()
                avatarKey = LocalAvatar.getKey()
                PtSetLightAnimStart(avatarKey, kKILightObjectName, False)

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
    def OnNotify(self, state, ID, events):

        PtDebugPrint(u"xKI.OnNotify(): Notify state = {}, ID = {}.".format(state, ID), level=kDebugDumpLevel)
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
                    if self.offeredBookMode == kGUI.Offeree:
                        # Then take the offered PlayerBook.
                        self.yeeshaBook.hide()
                        PtToggleAvatarClickability(True)
                        plybkCB.setChecked(0)
                    # Else, they were too late.
                    self.offeredBookMode = kGUI.NotOffering
                    self.bookOfferer = None
                    PtDebugPrint(u"xKI.OnNotify(): Offerer is rescinding the book offer.", level=kDebugDumpLevel)
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
                        self.offeredBookMode = kGUI.Offeree
                        PtDebugPrint(u"xKI.OnNotify(): Offered book by ", self.bookOfferer.getName(), level=kDebugDumpLevel)
                        self.ShowYeeshaBook()
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
                                self.yeeshaBook.hide()
                                PtToggleAvatarClickability(True)
                                plybkCB.setChecked(0)
                                # Prepare to choose who to offer the link to.
                                PtSetOfferBookMode(self.key, "Personal", "Relto")
                    elif event[2] == xLinkingBookDefs.kYeeshaBookLinkID:
                        if self.isYeeshaBookEnabled:
                            self.yeeshaBook.hide()
                            plybkCB.setChecked(0)
                            if self.offeredBookMode == kGUI.Offeree:
                                self.offeredBookMode = kGUI.NotOffering
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
                        self.ToggleYeeshaPageSDL(sdlvar, 1)
                elif event[1] == PtBookEventTypes.kNotifyShow:
                    pass
                elif event[1] == PtBookEventTypes.kNotifyHide:
                    PtToggleAvatarClickability(True)
                    plybkCB.setChecked(0)
                    if self.offeredBookMode == kGUI.Offeree:
                        self.offeredBookMode = kGUI.NotOffering
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
                        self.ToggleYeeshaPageSDL(sdlvar, 0)
                return
        if state:
            # Is it one of the responders that are displaying the BigKI?
            if ID == KIOnResp.id:
                self.ShowBigKIMode()
                self.waitingForAnimation = False
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                toggleCB.enable()
            elif ID == KIOffResp.id:
                BigKI.dialog.hide()
                self.waitingForAnimation = False
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                toggleCB.enable()
        if ID == KIMarkerGameGUIClose.id:
            PtHideDialog("KIMiniMarkers")
        if ID == KIJalakGUIClose.id:
            PtHideDialog("jalakControlPanel")
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
        elif ID == KIJalakGUIOpen.id:
            KIJalakGUIDialog.dialog.enable()
            ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).enable()
        elif ID == KIJalakBtnLights.id:
            btnID = int(KIJalakBtnLights.getState())
            SendNote(self.key, self.jalakScript, "{}".format(btnID))
            PtAtTimeCallback(self.key, kJalakBtnDelaySeconds, kTimers.JalakBtnDelay)

    ## Called by Plasma when a page has been loaded.
    # This is used to delay the display of objects until something is paged in.
    def OnPageLoad(self, what, room):

        if not self.KIGUIInitialized:
            self.KIGUIInitialized = True
            self.SetupKI()

        # When we unload the page, then the device we have must be gone.
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
                self.RefreshMiniKIMarkerDisplay()
                NewItemAlert.dialog.hide()
                kialert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                kialert.hide()
            if self.pelletImager != "":
                self.pelletImager = ""
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).hide()

    ## Called by Plasma when a GUI event occurs.
    # Delegates the appropriate response to the correct handler.
    def OnGUINotify(self, ID, control, event):

        PtDebugPrint(u"xKI.OnGUINotify(): ID = {}, event = {}, control = {}.".format(ID, event, control), level=kDebugDumpLevel)
        if ID == KIBlackbar.id:
            self.ProcessNotifyBlackbar(control, event)
        elif ID == KIMicroBlackbar.id:
            self.ProcessNotifyMicroBlackbar(control, event)
        elif ID == KIMicro.id:
            self.ProcessNotifyMicro(control, event)
        elif ID == KIMini.id:
            self.ProcessNotifyMini(control, event)
        elif ID == BigKI.id:
            self.ProcessNotifyBigKI(control, event)
        elif ID == KIListModeDialog.id:
            self.ProcessNotifyListMode(control, event)
        elif ID == KIPictureExpanded.id:
            self.ProcessNotifyPictureExpanded(control, event)
        elif ID == KIJournalExpanded.id:
            self.ProcessNotifyJournalExpanded(control, event)
        elif ID == KIPlayerExpanded.id:
            self.ProcessNotifyPlayerExpanded(control, event)
        elif ID == KISettings.id:
            self.ProcessNotifySettingsExpanded(control, event)
        elif ID == KIVolumeExpanded.id:
            self.ProcessNotifyVolumeExpanded(control, event)
        elif ID == KIAgeOwnerExpanded.id:
            self.ProcessNotifyAgeOwnerExpanded(control, event)
        elif ID == KIYesNo.id:
            self.ProcessNotifyYesNo(control, event)
        elif ID == NewItemAlert.id:
            self.ProcessNotifyNewItemAlert(control, event)
        elif ID == KICreateMarkerGameGUI.id:
            self.ProcessNotifyCreateMarkerGameGUI(control, event)
        elif ID == KIMarkerFolderExpanded.id:
            self.ProcessNotifyMarkerFolderExpanded(control, event)
        elif ID == KIMarkerFolderPopupMenu.id:
            self.ProcessNotifyMarkerFolderPopupMenu(control, event)
        elif ID == KIJalakGUIDialog.id:
            self.ProcessNotifyJalakGUI(control, event)

    ## Called by Plasma on receipt of a KI message.
    # These are messages which instruct xKI to perform a certain command like
    # enabling or disabling the KI, showing a Yes/No dialog, etc.. These
    # messages can also be sent from the Python to Plasma and back.
    def OnKIMsg(self, command, value):

        PtDebugPrint(u"xKI.OnKIMsg(): command = {} value = {}.".format(command, value), level=kDebugDumpLevel)
        if self.markerGameManager.OnKIMsg(command, value):
            return

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
            self.LocalizeDialog(1)
            KIYesNo.dialog.show()
        elif command == kAddPlayerDevice:
            if "<p>" in value:
                self.pelletImager = value.rstrip("<p>")
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).show()
                PtDebugPrint(u"Pellet Imager:", self.pelletImager, level=kDebugDumpLevel)
                return
            try:
                self.folderOfDevices.index(Device(value))
            except ValueError:
                self.folderOfDevices.append(Device(value))
                self.RefreshPlayerList()
        elif command == kRemovePlayerDevice:
            if "<p>" in value:
                self.pelletImager = ""
                ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).hide()
                return
            try:
                self.folderOfDevices.remove(Device(value))
            except ValueError:
                pass
            self.RefreshPlayerList()
        elif command == kUpgradeKILevel:
            if value >= kLowestKILevel and value <= kHighestKILevel:
                if value > self.KILevel:
                    PtDebugPrint(u"xKI.OnKIMsg(): Upgrading from KI level {} to new KI level of {}.".format(self.KILevel, value), level=kWarningLevel)
                    self.RemoveKILevel(self.KILevel)
                    self.KILevel = value
                    self.chatMgr.KILevel = self.KILevel
                    self.UpdateKILevelChronicle()
                    self.WearKILevel(self.KILevel)
                else:
                    PtDebugPrint(u"xKI.OnKIMsg(): Ignoring, trying to upgrade from KI level {} to new KI level of {}.".format(self.KILevel, value), level=kWarningLevel)
                    self.MakeSureWeWereKILevel()
            else:
                PtDebugPrint(u"xKI.OnKIMsg(): Invalid KI level {}.".format(value), level=kErrorLevel)
        elif command == kDowngradeKILevel:
            if value == self.KILevel:
                PtDebugPrint(u"xKI.OnKIMsg(): Remove KI level of {}.".format(value), level=kWarningLevel)
                if value == kNormalKI:
                    self.RemoveKILevel(kNormalKI)
                    self.KILevel = kMicroKI
                    self.chatMgr.KILevel = self.KILevel
                    self.UpdateKILevelChronicle()
                    self.WearKILevel(self.KILevel)
                else:
                    PtDebugPrint(u"xKI.OnKIMsg(): Ignoring, can't remove to any lower than {}.".format(value), level=kWarningLevel)
            else:
                PtDebugPrint(u"xKI.OnKIMsg(): Ignoring, trying to remove KI Level {}, but currently at {}.".format(value, self.KILevel), level=kWarningLevel)
        elif command == kSetPrivateChatChannel:
            self.chatMgr.privateChatChannel = value
        elif command == kUnsetPrivateChatChannel:
            self.chatMgr.privateChatChannel = 0
        elif command == kStartBookAlert:
            self.AlertBookStart()
        elif command == kStartKIAlert:
            self.AlertKIStart()
        elif command == kUpdatePelletScore:
            self.UpdatePelletScore()
            self.AlertKIStart()
        elif command == kMiniBigKIToggle:
            self.ToggleKISize()
        elif command == kKIPutAway:
            self.ToggleMiniKI()
        elif command == kChatAreaPageUp:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kPageUp)
        elif command == kChatAreaPageDown:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kPageDown)
        elif command == kChatAreaGoToBegin:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kBufferStart)
        elif command == kChatAreaGoToEnd:
            self.chatMgr.ScrollChatArea(PtGUIMultiLineDirection.kBufferEnd)
        elif command == kKITakePicture:
            self.TakePicture()
        elif command == kKICreateJournalNote:
            self.MiniKICreateJournalNote()
        elif command == kKIToggleFade:
            if self.chatMgr.IsFaded():
                self.ResetFadeState()
            else:
                self.FadeCompletely()
        elif command == kKIToggleFadeEnable:
            self.KillFadeTimer()
            if self.chatMgr.fadeEnableFlag:
                self.chatMgr.fadeEnableFlag = False
            else:
                self.chatMgr.fadeEnableFlag = True
            self.StartFadeTimer()
        elif command == kKIChatStatusMsg:
            self.chatMgr.DisplayStatusMessage(value, 1)
        elif command == kKILocalChatStatusMsg:
            self.chatMgr.DisplayStatusMessage(value, 0)
        elif command == kKILocalChatErrorMsg:
            self.chatMgr.AddChatLine(None, value, kChat.SystemMessage)
        elif command == kKIUpSizeFont:
            self.ChangeFontSize(1)
        elif command == kKIDownSizeFont:
            self.ChangeFontSize(-1)
        elif command == kKIOpenYeehsaBook:
            nm = ptNetLinkingMgr()
            if self.KILevel >= kMicroKI and not self.KIDisabled and not self.waitingForAnimation and nm.isEnabled():
                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                    self.ShowYeeshaBook()
                    if self.KILevel == kMicroKI:
                        plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                    else:
                        plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                    plybkCB.setChecked(1)
        elif command == kKIOpenKI:
            if not self.waitingForAnimation:
                if not KIMini.dialog.isEnabled():
                    self.ToggleMiniKI(1)
                elif not BigKI.dialog.isEnabled():
                    if self.chatMgr.fadeEnableFlag and self.chatMgr.IsFaded():
                        self.ResetFadeState()
                    else:
                        self.ToggleKISize()
                else:
                    self.ToggleMiniKI()
        elif command == kKIShowOptionsMenu:
            if self.yeeshaBook:
                self.yeeshaBook.hide()
            PtToggleAvatarClickability(True)
            if self.KILevel == kMicroKI:
                plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                plybkCB.setChecked(0)
            elif self.KILevel > kMicroKI:
                plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
                plybkCB.setChecked(0)
            if not self.waitingForAnimation and not self.KIDisabled:
                PtShowDialog("OptionsMenuGUI")
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
            if command == kKIOKDialogNoQuit:
                self.YNWhatReason = kGUI.YNNoReason
            KIYesNo.dialog.show()
        elif command == kDisableYeeshaBook:
            self.isYeeshaBookEnabled = False
        elif command == kEnableYeeshaBook:
            self.isYeeshaBookEnabled = True
        elif command == kQuitDialog:
            yesText = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesNoTextID))
            yesText.setStringW(PtGetLocalizedString("KI.Messages.LeaveGame"))
            self.LocalizeDialog()
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
            self.UpgradeKIMarkerLevel(value)
            self.RefreshMiniKIMarkerDisplay()
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
            self.DetermineKILevel()
            self.DetermineGZ()
            self.RefreshMiniKIMarkerDisplay()
        elif command == kGZFlashUpdate:
            try:
                args = value.split()
                GZGame = int(args[0])
            except:
                PtDebugPrint(u"xKI.OnKIMsg(): Cannot Update Marker Display, invalid Parameters: {}.".format(value))
                return
            if GZGame == -1:
                self.GZFlashUpdate(value)
            else:
                self.DetermineKILevel()
                if self.gKIMarkerLevel > kKIMarkerNotUpgraded and self.gKIMarkerLevel < kKIMarkerNormalLevel:
                    self.GZFlashUpdate(value)
            self.RefreshMiniKIMarkerDisplay()
        elif command == kGZInRange:
            # Only say markers are in range if there are more markers to get.
            if self.gMarkerToGetNumber > self.gMarkerGottenNumber:
                self.gGZMarkerInRange = value[0]
                self.gGZMarkerInRangeRepy = value[1]
                self.RefreshMiniKIMarkerDisplay()
                # Show alert
                NewItemAlert.dialog.show()
                KIAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
                KIAlert.show()
        elif command == kGZOutRange:
            self.gGZMarkerInRange = 0
            self.gGZMarkerInRangeRepy = None
            self.RefreshMiniKIMarkerDisplay()
            NewItemAlert.dialog.hide()
            KIAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
            KIAlert.hide()

        #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
        # User-created Marker Games messages #
        #~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
        elif command == kKICreateMarker:
            self.CreateAMarker()
        elif command == kKICreateMarkerFolder:
            self.CreateMarkerGame()

    ## Called by Plasma on receipt of a backdoor message.
    # These backdoor messages can trigger various actions.
    def OnBackdoorMsg(self, target, param):

        if target == "ki" and param == "refresh":
            self.BKFolderLineDict = self.BKJournalFolderDict
            self.BKFolderListOrder = self.BKJournalListOrder
            self.BKFolderSelected = self.BKJournalFolderSelected
            self.BKFolderTopLine = self.BKJournalFolderTopLine

            self.BigKIRefreshFolderList()
            self.BigKIRefreshFolderDisplay()
            self.BigKIRefreshContentList()
            self.BigKIRefreshContentListDisplay()

            self.ChangeBigKIMode(kGUI.BKListMode)

        if target.lower() == "cgz":
            self.markerGameManager.OnBackdoorMsg(target, param)

    ## Called by Plasma on receipt of a chat message.
    # This does not get called if the user sends a chat message through the KI,
    # only if he's getting a new message from another player.
    def OnRTChat(self, player, message, flags):

        if message is not None:
            cFlags = xKIChat.ChatFlags(flags)
            # Is it a private channel message that can't be listened to?
            if cFlags.broadcast and cFlags.channel != self.chatMgr.privateChatChannel:
                return

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
    def OnTimer(self, ID):

        # Chat fading.
        if ID == kTimers.Fade:
            # If it is fading, fade a tick.
            if self.chatMgr.fadeMode == kChat.FadeFullDisp:
                self.chatMgr.currentFadeTick -= 1
                # Setup call for next second.
                if self.chatMgr.currentFadeTick > 0:
                    PtAtTimeCallback(self.key, kChat.FullTickTime, kTimers.Fade)
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
        elif ID == kTimers.BKITODCheck and BigKI.dialog.isEnabled():
                self.BigKISetChanging()

        # Time of the currently played Marker Game.
        elif ID == kTimers.MarkerGame and self.currentPlayingMarkerGame is not None:
                self.currentPlayingMarkerGame.updateGameTime()
                PtAtTimeCallback(self.key, 1, kTimers.MarkerGame)

        # Stop an alert.
        elif ID == kTimers.AlertHide:
            self.AlertStop()

        # Take a snapshot after a waiting period.
        elif ID == kTimers.TakeSnapShot:
            PtDebugPrint(u"xKI.OnTimer(): Taking snapshot.")
            PtStartScreenCapture(self.key)

        # Dump the open logs.
        elif ID == kTimers.DumpLogs:
            if (PtDumpLogs(self.chatMgr.logDumpDest)):
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Messages.LogDumpSuccess", [self.chatMgr.logDumpDest]))
            else:
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Messages.LogDumpFailed", [self.chatMgr.logDumpDest]))

        # Turn off the KI light.
        elif ID == kTimers.LightStop:
            self.DoKILight(0, 0)

        # Turn on the Jalak GUI buttons.
        elif ID == kTimers.JalakBtnDelay:
            self.SetJalakGUIButtons(1)

        # Flash the scroll down arrow.
        elif ID == kTimers.IncomingChatFlash:
            if self.KILevel < kNormalKI:
                mKIdialog = KIMicro.dialog
            else:
                mKIdialog = KIMini.dialog
            if self.chatMgr.incomingChatFlashState > 0:
                btn = ptGUIControlButton(mKIdialog.getControlFromTag(kGUI.miniChatScrollDown))
                if self.chatMgr.incomingChatFlashState & 1:
                    btn.hide()
                else:
                    btn.show()
                self.chatMgr.incomingChatFlashState -= 1
                PtAtTimeCallback(self.key, 0.15, kTimers.IncomingChatFlash)
            else:
                mKIdialog.refreshAllControls()

    ## Called by Plasma when a screen capture is done.
    # This gets called once the screen capture is performed and ready to be
    # processed by the KI.
    def OnScreenCaptureDone(self, image):

        PtDebugPrint(u"xKI.OnScreenCaptureDone(): Snapshot is ready to be processed.")
        self.BigKICreateJournalImage(image)
        # Only show the KI if there isn't a dialog in the way.
        if not PtIsGUIModal():
            # Make sure that we are in journal mode.
            if self.BKFolderLineDict is not self.BKJournalFolderDict:
                modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
                modeselector.setValue(0)
            # Hide any previously opened picture.
            if self.BKRightSideMode != kGUI.BKPictureExpanded:
                self.HideBigKIMode()
            self.BKRightSideMode = kGUI.BKPictureExpanded
            # Reset the top line and selection.
            self.BigKIRefreshFolderDisplay()
            # Prepare to edit the caption of the picture.
            self.BigKIEnterEditMode(kGUI.BKEditFieldPICTitle)
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
            self.AlertKIStart()

        self.takingAPicture = False

        # Save the image to the filesystem.
        basePath = os.path.join(PtGetUserPath(), kImages.Directory)
        if not PtCreateDir(basePath):
            PtDebugPrint(u"xKI.OnScreenCaptureDone(): Unable to create \"{}\" directory. Image not saved to disk.".formatZ(basePath))
            return

        imageList = glob.iglob(os.path.join(basePath, "{}[0-9][0-9][0-9][0-9].png".format(kImages.FileNameTemplate)))
        imageNumbers = [int(os.path.basename(img)[7:-4]) for img in imageList] + [0]
        missingNumbers = set(range(1, max(imageNumbers))).difference(set(imageNumbers))
        if len(missingNumbers) > 0:
            firstMissing = min(missingNumbers)
        else:
            firstMissing = max(imageNumbers) + 1
        tryName = os.path.join(basePath, U"{}{:04d}.png".format(kImages.FileNameTemplate, firstMissing))

        PtDebugPrint(u"xKI.OnScreenCaptureDone(): Saving image to \"{}\".".format(tryName), level=kWarningLevel)
        gpsLocation = "{} {} {}".format(self.dniCoords.getTorans(), self.dniCoords.getHSpans(), self.dniCoords.getVSpans())
        metadata = {
            "Explorer" : PtGetLocalPlayer().getPlayerName(),
            "Location" : gpsLocation,
            "Age Name" : xKIHelpers.GetAgeName()
            }
        image.saveAsPNG(tryName, metadata)

    ## Called by Plasma when the player list has been updated.
    # This makes sure that everything is updated and refreshed.
    def OnMemberUpdate(self):

        PtDebugPrint(u"xKI.OnMemberUpdate(): Refresh player list.", level=kDebugDumpLevel)
        if PtIsDialogLoaded("KIMini"):
            self.RefreshPlayerList()

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
            self.SetBigKIToButtons()
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
            self.ResetFadeState()

    ## Called by Plasma on receipt of a high-level player vault event.
    def OnVaultNotify(self, event, tupData):

        PtDebugPrint(u"xKI.OnVaultNotify(): Event = {} and data = {}.".format(event, tupData), level=kDebugDumpLevel)
        if not tupData:
            PtDebugPrint(u"xKI.OnVaultNotify(): Bailing, no Age data.")
            return

        if PtIsDialogLoaded("KIMain"):
            if event == PtVaultNotifyTypes.kRegisteredOwnedAge or event == PtVaultNotifyTypes.kUnRegisteredOwnedAge or event == PtVaultNotifyTypes.kRegisteredVisitAge or event == PtVaultNotifyTypes.kUnRegisteredVisitAge:
                if self.KILevel > kMicroKI:
                    # A new owned Age was added, refresh its folders.
                    if isinstance(tupData[0], ptVaultAgeLinkNode):
                        # Is it the neighborhood?
                        ownedAge = tupData[0].getAgeInfo()
                        if ownedAge is not None:
                            if self.IsAgeMyNeighborhood(ownedAge):
                                self.BigKIRefreshHoodStatics(ownedAge)
                                self.RefreshPlayerList()
                            # Rebuild the player folder list because it might have changed.
                            self.BigKIRefreshFolderList()
                            self.BigKIRefreshFolderDisplay()
                            self.BigKIRefreshContentList()
                            self.BigKIRefreshContentListDisplay()
                            self.RefreshAgeOwnerSettings()
                        else:
                            PtDebugPrint(u"xKI.OnVaultNotify(): No ageInfo. ", level=kErrorLevel)
                    else:
                        PtDebugPrint(u"xKI.OnVaultNotify(): Unknown tuple data type. ", level=kErrorLevel)
            else:
                PtDebugPrint(u"xKI.OnVaultNotify(): Unknown event {}.".format(event), level=kWarningLevel)
        else:
            PtDebugPrint(u"xKI.OnVaultNotify(): BigKI dialog was not loaded, waiting.", level=kDebugDumpLevel)

    ## Called by Plasma on receipt of a low-level player vault event.
    def OnVaultEvent(self, event, tupData):

        PtDebugPrint(u"xKI.VaultEvent(): Event = {} and data = {}.".format(event, tupData), level=kDebugDumpLevel)
        self.HandleVaultTypeEvents(event, tupData)

    ## Called by Plasma on receipt of a low-level Age vault event.
    def OnAgeVaultEvent(self, event, tupData):

        PtDebugPrint(u"xKI.OnAgeVaultEvent(): Event = {} and data = {}.".format(event, tupData), level=kDebugDumpLevel)
        self.HandleVaultTypeEvents(event, tupData)

    ## Called by Plasma when a marker has been captured by the player.
    def OnMarkerMsg(self, msgType, tupData):

        self.markerGameManager.OnMarkerMsg(msgType, tupData)

    ## Called by Plasma on receipt of a game score message.
    # This is used for handling pellet scoring.
    def OnGameScoreMsg(self, msg):

        if isinstance(msg, ptGameScoreListMsg):
            pelletTextBox = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPelletDrop))
            try:
                score = msg.getScores()[0]
                points = score.getPoints()

                if self.scoreOpCur == kPellets.ScoreFetchForDisplay:
                    if points < 0:
                        points = 0
                    pelletTextBox.setString(str(points))
                    PtDebugPrint(u"xKI.OnGameScoreMsg(): PelletDrop score: {}.".format(points), level=kWarningLevel)
                elif self.scoreOpCur == kPellets.ScoreFetchMineForUpload:
                    self.scoreSource = score
                    self.DoScoreOp(kPellets.ScoreFetchUploadDestination)
                elif self.scoreOpCur == kPellets.ScoreFetchUploadDestination:
                    self.scoreDestination = score
                    self.scoreUploaded = self.scoreSource.getPoints()
                    self.DoScoreOp(kPellets.ScoreTransfer)
            except:
                if self.scoreOpCur == kPellets.ScoreFetchForDisplay:
                    pelletTextBox.setString("000")
                elif self.scoreOpCur == kPellets.ScoreFetchUploadDestination:
                    self.DoScoreOp(kPellets.ScoreCreateUploadDestination)

        elif isinstance(msg, ptGameScoreTransferMsg):
            pelletTextBox = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPelletDrop))
            pelletTextBox.setString("000")
            self.UploadPelletScore(self.scoreUploaded)
            del self.scoreDestination
            del self.scoreSource
            self.scoreUploaded = 0

        elif isinstance(msg, ptGameScoreUpdateMsg):
            if self.scoreOpCur == kPellets.ScoreCreateUploadDestination:
                self.scoreDestination = msg.getScore()
                self.scoreUploaded = self.scoreSource.getPoints()
                self.DoScoreOp(kPellets.ScoreTransfer)

        # Process any remaining queued ops.
        self.ProcessScoreOps()

    #~~~~~~~~~~#
    # KI Setup #
    #~~~~~~~~~~#

    ## Sets up the KI for a given player.
    # Goes through all the steps required to ensure the player's KI is
    # appropriately up-to-date when a user starts playing.
    def SetupKI(self):

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

        self.DetermineCensorLevel()
        self.DetermineKILevel()
        self.DetermineKIFlags()
        self.DetermineGZ()

        # Hide all dialogs first.
        KIMicroBlackbar.dialog.hide()
        KIMicro.dialog.hide()
        KIMini.dialog.hide()
        KIBlackbar.dialog.hide()
        BigKI.dialog.hide()
        self.chatMgr.ToggleChatMode(0)

        # Remove unneeded kFontShadowed flags (as long as we can't do that directly in the PRPs)
        for dialogAttr in (BigKI, KIListModeDialog, KIJournalExpanded, KIPictureExpanded, KIPlayerExpanded, KIAgeOwnerExpanded, KISettings, KIMarkerFolderExpanded, KICreateMarkerGameGUI):
            for i in xrange(dialogAttr.dialog.getNumControls()):
                f = ptGUIControl(dialogAttr.dialog.getControlFromIndex(i))
                # call this on all controls, even those that use the color scheme of the
                # dialog and would already report the flag cleared after the first one,
                # as they still need the setFontFlags call to refresh themselves
                f.setFontFlags(f.getFontFlags() & ~int(PtFontFlags.kFontShadowed))

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
            self.CheckInboxForUnseen()

        self.ToggleMiniKI()

        modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
        modeselector.setValue(0)

        self.BigKIRefreshFolderList()
        self.BigKIRefreshFolderDisplay()
        self.BigKIRefreshContentList()
        self.BigKIRefreshContentListDisplay()

        self.ChangeBigKIMode(kGUI.BKListMode)

        # Load the ding dang marker game
        if self.gKIMarkerLevel == kKIMarkerNormalLevel:
            self.markerGameManager.LoadFromVault()

    #~~~~~~~~~~#
    # KI Flags #
    #~~~~~~~~~~#

    ## Sets the KI Flags from the Chronicle.
    # KI Flags are settings for the player's KI (pertaining to Buddies).
    def DetermineKIFlags(self):

        vault = ptVault()

        # Only get PMs and KI Mails from Buddies.
        entry = vault.findChronicleEntry(kChron.OnlyPMs)
        if entry is None:
            # Not found, set to 0 by default.
            vault.addChronicleEntry(kChron.OnlyPMs, kChron.OnlyPMsType, str(int(self.onlyGetPMsFromBuddies)))
        else:
            self.onlyGetPMsFromBuddies = int(entry.chronicleGetValue())

        # Only allow the player to be buddied on request.
        entry = vault.findChronicleEntry(kChron.BuddiesOnRequest)
        if entry is None:
            # Not found, set to 0 by default.
            vault.addChronicleEntry(kChron.BuddiesOnRequest, kChron.BuddiesOnRequestType, str(int(self.onlyAllowBuddiesOnRequest)))
        else:
            self.onlyAllowBuddiesOnRequest = int(entry.chronicleGetValue())

    ## Save the KI Flags to the Chronicle.
    def SaveKIFlags(self):

        vault = ptVault()
        # Only get PMs and KI Mails from Buddies.
        entry = vault.findChronicleEntry(kChron.OnlyPMs)
        if entry is not None:
            entry.chronicleSetValue(str(int(self.onlyGetPMsFromBuddies)))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.OnlyPMs, kChron.OnlyPMsType, str(int(self.onlyGetPMsFromBuddies)))

        # Only allow the player to be buddied on request.
        entry = vault.findChronicleEntry(kChron.BuddiesOnRequest)
        if entry is not None:
            entry.chronicleSetValue(str(int(self.onlyAllowBuddiesOnRequest)))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.BuddiesOnRequest, kChron.BuddiesOnRequestType, str(int(self.onlyAllowBuddiesOnRequest)))

    #~~~~~~~~~~#
    # KI Light #
    #~~~~~~~~~~#

    ## Finds out what the current KI light state is.
    def CheckKILight(self):

        timeRemaining = self.GetKILightChron()
        if not timeRemaining:
            PtDebugPrint(u"xKI.CheckKILight(): Had KI light, but it's currently off.", level=kDebugDumpLevel)
            self.DoKILight(0, 1)
        elif timeRemaining > 0:
            PtDebugPrint(u"xKI.CheckKILight(): Have KI light, time remaining = ", timeRemaining, level=kDebugDumpLevel)
            self.DoKILight(1, 1, timeRemaining)
            self.SetKILightChron(0)
        else:
            PtDebugPrint(u"No KI light.", level=kDebugDumpLevel)

    ## Get the KI light remaining time from the chronicle.
    def GetKILightChron(self):

        vault = ptVault()
        entry = vault.findChronicleEntry("KILightStop")
        if entry is not None:
            entryValue = entry.chronicleGetValue()
            remaining = int(entryValue)
            return remaining
        else:
            PtDebugPrint(u"xKI.GetKILightChron(): No KI light.", level=kDebugDumpLevel)
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
            PtDebugPrint(u"xKI.SetKILightChron(): Set KI light chron to: ", remaining, level=kDebugDumpLevel)
            entry.chronicleSetValue(str(int(remaining)))
            entry.save()

    ## Manages the KI light.
    def DoKILight(self, state, ff, remaining=0):

        thisResp = kListLightResps[state]
        LocalAvatar = PtGetLocalAvatar()
        avatarKey = LocalAvatar.getKey()
        avatarObj = avatarKey.getSceneObject()
        respList = avatarObj.getResponders()
        if len(respList) > 0:
            PtDebugPrint(u"xKI.DoKILight(): Responder list:", level=kDebugDumpLevel)
            for resp in respList:
                PtDebugPrint(u"                 {}".format(resp.getName()))
                if resp.getName() == thisResp:
                    PtDebugPrint(u"xKI.DoKILight(): Found KI light resp: {}.".format(thisResp), level=kDebugDumpLevel)
                    atResp = ptAttribResponder(42)
                    atResp.__setvalue__(resp)
                    atResp.run(self.key, avatar=LocalAvatar, fastforward=ff)
                    if state:
                        PtAtTimeCallback(self.key, remaining, kTimers.LightStop)
                        PtDebugPrint(u"xKI.DoKILight(): Light was on in previous age, turning on for remaining ", remaining, " seconds.", level=kWarningLevel)
                        curTime = PtGetDniTime()
                        self.lightStop = (remaining + curTime)
                        self.lightOn = True
                    else:
                        PtDebugPrint(u"xKI.DoKILight(): Light is shut off, updating chron.", level=kWarningLevel)
                        self.SetKILightChron(remaining)
                        self.lightOn = False
                        PtSetLightAnimStart(avatarKey, kKILightObjectName, False)
                    break
        else:
            PtDebugPrint(u"xKI.DoKILight(): Couldn't find any responders.", level=kErrorLevel)

    #~~~~~~~~~~~~~~#
    # Localization #
    #~~~~~~~~~~~~~~#

    ## Gets the appropriate localized values for a Yes/No dialog.
    def LocalizeDialog(self, dialog_type=0):

        confirm = "KI.YesNoDialog.QuitButton"
        if dialog_type == 1:
            confirm = "KI.YesNoDialog.YESButton"
        yesButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.YesButtonTextID))
        noButton = ptGUIControlTextBox(KIYesNo.dialog.getControlFromTag(kGUI.NoButtonTextID))
        yesButton.setStringW(PtGetLocalizedString(confirm))
        noButton.setStringW(PtGetLocalizedString("KI.YesNoDialog.NoButton"))

    #~~~~~~~~~#
    # Pellets #
    #~~~~~~~~~#

    ## Perform an operation on the pellet score.
    def DoScoreOp(self, op):

        self.scoreOps.append(op)
        if self.scoreOpCur == kPellets.ScoreNoOp:
            self.ProcessScoreOps()

    ## Process the stored score operations.
    def ProcessScoreOps(self):

        if not len(self.scoreOps):
            self.scoreOpCur = kPellets.ScoreNoOp
            return

        self.scoreOpCur = self.scoreOps.pop(0)
        if self.scoreOpCur == kPellets.ScoreFetchForDisplay:
            ptGameScore.findPlayerScores("PelletDrop", self.key)
        elif self.scoreOpCur == kPellets.ScoreFetchMineForUpload:
            ptGameScore.findPlayerScores("PelletDrop", self.key)
        elif self.scoreOpCur == kPellets.ScoreFetchUploadDestination:
            ptGameScore.findAgeScores("PelletDrop", self.key)
        elif self.scoreOpCur == kPellets.ScoreCreateUploadDestination:
            ptGameScore.createAgeScore("PelletDrop", PtGameScoreTypes.kAccumulative, 0, self.key)
        elif self.scoreOpCur == kPellets.ScoreTransfer:
            self.scoreSource.transferPoints(self.scoreDestination, key=self.key)

    ## Update the pellet score to the specified value.
    # If no value is specified, fetch the current score for display.
    def UpdatePelletScore(self, points=0):

        pelletTextBox = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPelletDrop))
        if points:
            pelletTextBox.setString(str(points))
        else:
            pelletTextBox.setString("...")  # Fetching from server.
            self.DoScoreOp(kPellets.ScoreFetchForDisplay)

    ## Upload the new pellet score to the server.
    def UploadPelletScore(self, score=None):

        if score:
            hoodInfoUpdate = PtFindActivator("PythHoodInfoImagerUpdater")
            PtDebugPrint(u"xKI.UploadPelletScore(): HoodInfoUpdate: {}.".format(hoodInfoUpdate), level=kDebugDumpLevel)
            if hoodInfoUpdate:
                notify = ptNotify(self.key)
                notify.clearReceivers()
                notify.addReceiver(hoodInfoUpdate)
                notify.netPropagate(1)
                notify.netForce(1)
                notify.setActivate(1.0)
                sName = "Score={}".format(PtGetLocalPlayer().getPlayerName())
                notify.addVarNumber(sName, score)
                notify.send()
                PtDebugPrint(u"xKI.UploadPelletScore(): Sending score notify: {} {}.".format(sName, score), level=kDebugDumpLevel)
        else:
            self.DoScoreOp(kPellets.ScoreFetchMineForUpload)

    #~~~~~~~~~~~~~~~#
    # Auto-complete #
    #~~~~~~~~~~~~~~~#

    ## Auto-completes the text for a given editing control.
    def Autocomplete(self, control):

        text = control.getStringW()

        proposition = self.autocompleteState.pickNext(text)
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

        proposition = self.autocompleteState.pickFirst(text, players)

        if proposition is not None:
            control.setStringW(proposition)
            control.end()
            control.refresh()

    #~~~~~~~~~~~~~~~~~#
    # Message History #
    #~~~~~~~~~~~~~~~~~#

    ## Set a control's text to a log entry in the message history
    def MessageHistory(self, control, set):

        if (set == "up"):
            if (self.chatMgr.MessageHistoryIs < len(self.chatMgr.MessageHistoryList)-1):
                self.chatMgr.MessageHistoryIs = self.chatMgr.MessageHistoryIs +1
                control.setStringW(self.chatMgr.MessageHistoryList[self.chatMgr.MessageHistoryIs])
                control.end()
                control.refresh()
        elif (set == "down"):
            if (self.chatMgr.MessageHistoryIs > 0):
                self.chatMgr.MessageHistoryIs = self.chatMgr.MessageHistoryIs -1
                control.setStringW(self.chatMgr.MessageHistoryList[self.chatMgr.MessageHistoryIs])
                control.end()
                control.refresh()

    #~~~~~~~~~~#
    # GZ Games #
    #~~~~~~~~~~#

    ## Sets the GZ globals from the Chronicle.
    def DetermineGZ(self):

        if self.gKIMarkerLevel > kKIMarkerNotUpgraded:
            if self.gKIMarkerLevel < kKIMarkerNormalLevel:
                vault = ptVault()
                entry = vault.findChronicleEntry(kChronicleGZGames)
                error = 0
                if entry is not None:
                    gameString = entry.chronicleGetValue()
                    PtDebugPrint(u"xKI.DetermineGZ(): Game string is: \"{}\".".format(gameString), level=kWarningLevel)
                    args = gameString.split()
                    if len(args) == 3:
                        try:
                            self.gGZPlaying = int(args[0])
                            colors = args[1].split(":")
                            outof = args[2].split(":")

                            # Check for corrupted entry.
                            if len(colors) != 2 or len(outof) != 2:
                                PtDebugPrint(u"xKI.DetermineGZ(): Invalid color field or marker field.")
                                raise ValueError

                            # Check for invalid entry.
                            if (colors[0] == "red" or colors[0] == "green") and int(outof[1]) > 15:
                                PtDebugPrint(u"xKI.DetermineGZ(): Invalid marker number entry (i.e. 1515 bug).")
                                raise ValueError

                            self.gMarkerGottenColor = colors[0]
                            self.gMarkerToGetColor = colors[1]
                            self.gMarkerGottenNumber = int(outof[0])
                            self.gMarkerToGetNumber = int(outof[1])

                            return
                        except:
                            PtDebugPrint(u"xKI.DetermineGZ(): Could not read GZ Games Chronicle.", level=kErrorLevel)
                            error = 1
                    else:
                        PtDebugPrint(u"xKI.DetermineGZ(): Invalid GZ Games string formation.", level=kErrorLevel)
                        error = 1

                # If there was a problem, reset everything to "off".
                self.gGZPlaying = 0
                self.gMarkerToGetColor = "off"
                self.gMarkerGottenColor = "off"
                self.gMarkerToGetNumber = 0
                self.gMarkerGottenNumber = 0

                # Reset Marker Games if a corrupted vault occurred.
                if error:
                    PtDebugPrint(u"xKI.DetermineGZ(): Vault corrupted, resetting all Marker Game data.", level=kErrorLevel)
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
            self.gGZPlaying = 0
            self.gMarkerToGetColor = "off"
            self.gMarkerGottenColor = "off"
            self.gMarkerToGetNumber = 0
            self.gMarkerGottenNumber = 0

    ## Update the GZ globals using provided values, not the Chronicle.
    def GZFlashUpdate(self, gameString):

        PtDebugPrint(u"xKI.GZFlashUpdate(): Game string is: \"{}\".".format(gameString), level=kWarningLevel)
        args = gameString.split()
        if len(args) == 3:
            try:
                GZPlaying = int(args[0])
                colors = args[1].split(":")
                outof = args[2].split(":")

                # Check for corrupted entry.
                if len(colors) != 2 or len(outof) != 2:
                    PtDebugPrint(u"xKI.GZFlashUpdate(): Invalid color field or marker field.")
                    raise ValueError

                MarkerGottenColor = colors[0]
                MarkerToGetColor = colors[1]
                MarkerGottenNumber = int(outof[0])
                MarkerToGetNumber = int(outof[1])

                # Check for invalid entry.
                if (colors[0] == "red" or colors[0] == "green") and MarkerToGetNumber > 15:
                    PtDebugPrint(u"xKI.GZFlashUpdate(): Invalid marker number entry (i.e. 1515 bug).")
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
                PtDebugPrint(u"xKI.GZFlashUpdate(): Could not read GZ Games Chronicle. Checking Chronicle for corruption.", level=kErrorLevel)
        else:
            PtDebugPrint(u"xKI.GZFlashUpdate(): Invalid GZ Games string formation. Checking Chronicle for corruption.", level=kErrorLevel)

        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleGZGames)
        if entry is not None:
            if gameString == entry.chronicleGetValue():
                PtDebugPrint(u"xKI.GZFlashUpdate(): Vault corrupted: trying to gracefully reset to a default state.", level=kErrorLevel)
                import grtzKIMarkerMachine
                grtzKIMarkerMachine.ResetMarkerGame()
                return

    ## Update the Chronicle's GZ Games values.
    # This takes the form of a series of values, separated by ":".
    def UpdateGZGamesChronicle(self):

        if self.gGZPlaying:
            vault = ptVault()
            entry = vault.findChronicleEntry(kChronicleGZGames)
            upString = "{} {}:{} {}:{}".format(self.gGZPlaying, self.gMarkerGottenColor, self.gMarkerToGetColor, self.gMarkerGottenNumber, self.gMarkerToGetNumber)
            if entry is not None:
                entry.chronicleSetValue(upString)
                entry.save()
            else:
                vault.addChronicleEntry(kChronicleGZGames, kChronicleGZGamesType, upString)

    ## Register a captured GZ marker.
    def CaptureGZMarker(self):

        if self.gGZPlaying and self.gMarkerToGetNumber > self.gMarkerGottenNumber:
            # Set the marker status to "captured" in the Chronicle.
            vault = ptVault()
            entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
            if entry is not None:
                markers = entry.chronicleGetValue()
                markerIdx = self.gGZMarkerInRange - 1
                if markerIdx >= 0 and markerIdx < len(markers):
                    if len(markers) - (markerIdx + 1) != 0:
                        markers = markers[:markerIdx] + kGZMarkerCaptured + markers[-(len(markers) - (markerIdx + 1)):]
                    else:
                        markers = markers[:markerIdx] + kGZMarkerCaptured
                    entry.chronicleSetValue(markers)
                    entry.save()
                    # Update the "marker gotten" count.
                    totalGotten = markers.count(kGZMarkerCaptured)
                    if self.gKIMarkerLevel > kKIMarkerFirstLevel:
                        # Is this the second wave of markers (or beyond)?
                        totalGotten -= 15
                        if totalGotten < 0:
                            totalGotten = 0
                    if totalGotten > self.gMarkerToGetNumber:
                        totalGotten = self.gMarkerToGetNumber
                    self.gMarkerGottenNumber = totalGotten
                    # Save update to Chronicle.
                    self.UpdateGZGamesChronicle()
                else:
                    PtDebugPrint(u"xKI.CaptureGZMarker(): Invalid marker serial number of {}.".format(self.gGZMarkerInRange))
                    return
            else:
                PtDebugPrint(u"xKI.CaptureGZMarker(): No Chronicle entry found.")
                return
            # Start building the notify message to go back to the orignator.
            if self.gGZMarkerInRangeRepy is not None:
                note = ptNotify(self.key)
                note.clearReceivers()
                note.addReceiver(self.gGZMarkerInRangeRepy)
                note.netPropagate(0)
                note.netForce(0)
                note.setActivate(1)
                note.addVarNumber("Captured", 1)
                note.send()
            self.gGZMarkerInRangeRepy = None
            self.gGZMarkerInRange = 0

    #~~~~~~~~~~~~~~#
    # Marker Games #
    #~~~~~~~~~~~~~~#

    ## Selects a new Marker Game type.
    def SelectMarkerType(self, tagID):

        dlgObj = KICreateMarkerGameGUI
        if tagID and self.selectedMGType != tagID and self.selectedMGType != 0:
            PtDebugPrint(u"xKI.SelectMarkerType(): Old Marker Game type ID: ", self.selectedMGType, level=kDebugDumpLevel)
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(self.selectedMGType)).enable()
            self.ChangeMarkerTypeColor(self.selectedMGType)

        self.selectedMGType = tagID
        PtDebugPrint(u"xKI.SelectMarkerType(): Selecting new Marker Game type: ", self.selectedMGType, level=kDebugDumpLevel)
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
    def CreateMarkerGame(self):

        # Make sure the player's KI Level is appropriately high.
        if self.KILevel <= kMicroKI or self.KIDisabled:
            PtDebugPrint(u"xKI.CreateMarkerGame(): Aborting Marker Game creation request, player does not have the KI.", level=kDebugDumpLevel)
            return

        # Make sure the player's KI Marker Level is appropriately high.
        if self.gKIMarkerLevel < kKIMarkerNormalLevel:
            PtDebugPrint(u"xKI.CreateMarkerGame(): Aborting Marker Game creation request, player does not have sufficient privileges.", level=kDebugDumpLevel)
            return

        # The player cannot be doing another task.
        if self.takingAPicture or self.waitingForAnimation:
            PtDebugPrint(u"xKI.CreateMarkerGame(): Aborting Marker Game creation request, player is busy.", level=kDebugDumpLevel)
            return

        # The player cannot create a game if one is already in progress.
        if self.markerGameManager.playing:
            PtDebugPrint(u"xKI.CreateMarkerGame(): Aborting Marker Game creation request, a game is already in progress.", level=kDebugDumpLevel)
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.MarkerGame.createErrorExistingGame"), kChat.SystemMessage)
            return

        # Make sure the player has enough room.
        if not self.CanMakeMarkerGame():
            PtDebugPrint(u"xKI.CreateMarkerGame(): Aborting Marker Game creation request, player has reached the limit of Marker Games.", level=kDebugDumpLevel)
            self.ShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullMarkerGames"))
            return

        # The player can now launch the Marker Game creation GUI.
        self.HideBigKI()
        PtShowDialog("KIMiniMarkers")
        KIMarkerGameGUIOpen.run(self.key, netPropagate=0)

    ## Finishes creating the Marker Game after the asynchronous mini-game
    # server registers the parameters.
    def FinishCreateMarkerGame(self, gameName):

        # Get the current Age's Journal folder.
        load = 0
        while load < 2:
            try:
                journal = self.BKJournalFolderDict[self.GetAgeInstanceName()]
                if journal is None:
                    raise
                load = 2
            except:
                if load == 1:
                    # Failed twice in a row, it's hopeless.
                    ## @todo Create the folder in case this happens.
                    PtDebugPrint(u"xKI.FinishCreateMarkerGame(): Could not load Age's Journal Folder, Marker Game creation failed.", level=kErrorLevel)
                    return
                load += 1
                self.BigKIRefreshFolderList()

        # Hide the blackbar, just in case.
        KIBlackbar.dialog.hide()

        # Put the toggle button back to the BigKI setting.
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
        toggleCB.setChecked(1)

        # Set the current mode to the Age's Journal folder.
        modeSelector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
        modeSelector.setValue(0)
        self.BKFolderTopLine = self.BKJournalFolderTopLine = 0
        self.BKFolderSelected = self.BKJournalFolderSelected = self.BKJournalListOrder.index(self.GetAgeInstanceName())
        self.BigKIRefreshFolderDisplay()

        # Create the Marker Game node.
        PtDebugPrint(u"xKI.FinishCreateMarkerGame(): Creating Vault node with name = \"{}\".".format(gameName), level=kDebugDumpLevel)
        markerGameNode = ptVaultMarkerGameNode()
        markerGameNode.setCreatorNodeID(PtGetLocalClientID())
        markerGameNode.setGameName(gameName)
        self.BKCurrentContent = journal.addNode(markerGameNode)

        # Change to display current content.
        self.ChangeBigKIMode(kGUI.BKMarkerListExpanded)
        if BigKI.dialog.isEnabled():
            self.ShowBigKIMode()
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
    def CreateAMarker(self):

        if not self.takingAPicture and not self.waitingForAnimation:
            if self.KILevel > kMicroKI and not self.KIDisabled:
                self.UpdateKIUsage()
                if self.CanMakeMarker():
                    markerName = u"{} marker".format(self.markerGameManager.game_name)
                    avaCoord = PtGetLocalAvatar().position()
                    self.markerGameManager.AddMarker(PtGetAgeName(), avaCoord, markerName)
                    PtDebugPrint(u"xKI.CreateAMarker(): Creating marker at: ({}, {}, {}).".format(avaCoord.getX(), avaCoord.getY(), avaCoord.getZ()))
                else:
                    self.ShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullMarkers"))

    ## Perform the necessary operations to switch to a Marker Game.
    def SetWorkingToCurrentMarkerGame(self):

        if self.BKCurrentContent is None:
            PtDebugPrint(u"xKI.SetWorkingToCurrentMarkerGame(): Cannot set working game, as there is no Vault folder.")
            return

        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint(u"xKI.SetWorkingToCurrentMarkerGame(): Cannot set working game, as there is no Vault node.")
            return

        element = element.upcastToMarkerGameNode()
        if element is None:
            PtDebugPrint(u"xKI.SetWorkingToCurrentMarkerGame(): Cannot set working game, as the Vault node is of the wrong type.")
            return

        # Refresh the content.
        self.RefreshPlayerList()
        self.BigKICheckContentRefresh(self.BKCurrentContent)

    ## Reset from the working Marker Game to None.
    def ResetWorkingMarkerGame(self):

        MGmgr = ptMarkerMgr()

        # Don't delete any markers necessary for an existing game.
        if not self.markerGameManager.is_game_loaded:
            MGmgr.hideMarkersLocal()

        # Refresh the content.
        self.MFScrollPos = 0
        self.RefreshPlayerList()
        self.BigKICheckContentRefresh(self.BKCurrentContent)

    #~~~~~~~#
    # Jalak #
    #~~~~~~~#

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

        self.jalakGUIButtons = [jlakRandom, jlakExtreme, jlakWall, jlakAllLow, jlakAllMed, jlakAllHigh,
                          jlakRamp, jlakSphere, jlakBigBox, jlakLilBox, jlakRect, jlakDestroy]

        obj = PtFindSceneobject("JalakDONOTTOUCH", "Jalak")
        pythonScripts = obj.getPythonMods()
        for script in pythonScripts:
            if script.getName() == kJalakPythonComponent:
                self.jalakScript = script
                PtDebugPrint(u"xKI.JalakGUIInit(): Found Jalak's python component.", level=kDebugDumpLevel)
                return
        PtDebugPrint(u"xKI.JalakGUIInit(): Did not find Jalak's python component.", level=kErrorLevel)

    ## Toggle on/off the Jalak KI GUI.
    def JalakGUIToggle(self, ff=0):

        PtDebugPrint(u"xKI.JalakGUIToggle(): toggling GUI.", level=kDebugDumpLevel)
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
                PtDebugPrint(u"xKI.JalakGUIToggle(): Aborting request for Jalak GUI: user is busy.", level=kDebugDumpLevel)
                return
            # Only those that have Gahreesen KI can create a game.
            if self.KILevel <= kMicroKI or self.KIDisabled:
                PtDebugPrint(u"xKI.JalakGUIToggle(): Aborting request for Jalak GUI: user does not have the KI.", level=kDebugDumpLevel)
                return
            self.jalakGUIState = True
            PtShowDialog("jalakControlPanel")
            KIJalakGUIOpen.run(self.key, netPropagate=0)

    ## Activate or deactivate all the buttons in the Jalak GUI.
    def SetJalakGUIButtons(self, state):

        for btn in self.jalakGUIButtons:
            if state:
                btn.enable()
            else:
                btn.disable()

    #~~~~~~~~~~~#
    # KI Levels #
    #~~~~~~~~~~~#

    ## Make sure all the parts of the KI specific to this level are undone.
    def RemoveKILevel(self, level):

        # Is it removing the micro while upgrading?
        if level == kMicroKI:
            # Change the display to be the normal KI.
            KIMicroBlackbar.dialog.hide()
            KIMicro.dialog.hide()
        # Is it going from normal back to micro?
        elif level == kNormalKI:
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
            chatArea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatArea.lock()         # Make the chat display immutable.
            chatArea.unclickable()  # Make the chat display non-clickable.
            chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            chatArea.disableScrollControl()
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
    def WearKILevel(self, level):

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
            self.AlertKIStart()
            # Check the player's inbox.
            self.CheckInboxForUnseen()
            # Refresh the folders, which will create the age journal for this Age.
            self.BigKIRefreshFolderList()

    ## Forcefully make sure the avatar is wearing the current KI level.
    # This ensures the player is wearing either the Yeesha Book, or the Yeesha
    # Book and the Gahreesen KI.
    def MakeSureWeWereKILevel(self):

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
                avatar.avatar.saveClothing()
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
                avatar.avatar.saveClothing()
            except NameError:
                pass

    ## Sets the current KI level from the Chronicle.
    # Also sets the current KI Marker Level.
    def DetermineKILevel(self):

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
            elif oldLevel < kLowestKILevel:
                # Make sure that the user has at least a microKI.
                self.UpdateKILevelChronicle()
        self.chatMgr.KILevel = self.KILevel
        PtDebugPrint(u"xKI.DetermineKILevel(): The KI Level is {}.".format(self.KILevel), level=kWarningLevel)

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
                PtDebugPrint(u"xKI.DetermineKILevel(): Chronicle entry error with the KI's Marker Level, resetting to the default value.", level=kErrorLevel)
                entry.chronicleSetValue(str(self.gKIMarkerLevel))
                entry.save()
        PtDebugPrint(u"xKI.DetermineKILevel(): The KI Marker Level is {}.".format(self.gKIMarkerLevel), level=kWarningLevel)

        entry = vault.findChronicleEntry("feather")
        if entry is None:
            self.chatMgr.gFeather = 0
        else:
            try:
                self.chatMgr.gFeather = int(entry.chronicleGetValue())
            except ValueError:
                self.chatMgr.gFeather = 0

    ## Upgrade the KI Marker Level to a new setting.
    def UpgradeKIMarkerLevel(self, newLevel):

        PtDebugPrint(u"xKI.UpgradeKIMarkerLevel(): KI Marker Level going from {} to {}.".format(self.gKIMarkerLevel, newLevel), level=kWarningLevel)
        if self.KILevel > kMicroKI and newLevel > self.gKIMarkerLevel:
            self.gKIMarkerLevel = newLevel
            vault = ptVault()
            entry = vault.findChronicleEntry(kChronicleKIMarkerLevel)
            if entry is None:
                PtDebugPrint(u"xKI.UpgradeKIMarkerLevel(): Chronicle entry not found, set to {}.".format(self.gKIMarkerLevel), level=kWarningLevel)
                vault.addChronicleEntry(kChronicleKIMarkerLevel, kChronicleKIMarkerLevelType, str(self.gKIMarkerLevel))
            else:
                PtDebugPrint(u"xKI.UpgradeKIMarkerLevel(): Upgrading existing KI Marker Level to {}.".format(self.gKIMarkerLevel), level=kWarningLevel)
                entry.chronicleSetValue(str(self.gKIMarkerLevel))
                entry.save()

    ## Updates the KI level's Chronicle value.
    def UpdateKILevelChronicle(self):

        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleKILevel)
        if entry is not None:
            entry.chronicleSetValue(str(self.KILevel))
            entry.save()
        else:
            vault.addChronicleEntry(kChronicleKILevel, kChronicleKILevelType, str(self.KILevel))

    #~~~~~~~~~~~~~#
    # Yeesha Book #
    #~~~~~~~~~~~~~#

    ## Show the Yeesha Book to the player, in accordance with its status.
    def ShowYeeshaBook(self):

        if self.KILevel >= kMicroKI and not self.KIDisabled and not self.waitingForAnimation:
            if BigKI.dialog.isEnabled() or KIMini.dialog.isEnabled():
                self.ToggleMiniKI()
            startOpen = False
            if self.isYeeshaBookEnabled:
                if self.offeredBookMode == kGUI.NotOffering:
                    YeeshaBDef = xLinkingBookDefs.xYeeshaBookBase + self.GetYeeshaPageDefs()
                else:
                    YeeshaBDef = xLinkingBookDefs.xYeeshaBookNoShare
                    startOpen = True
            else:
                YeeshaBDef = xLinkingBookDefs.xYeeshaBookBroke + self.GetYeeshaPageDefs()
            self.yeeshaBook = ptBook(YeeshaBDef, self.key)
            self.yeeshaBook.setSize(xLinkingBookDefs.YeeshaBookSizeWidth, xLinkingBookDefs.YeeshaBookSizeHeight)
            self.yeeshaBook.show(startOpen)
            PtToggleAvatarClickability(False)

    ## Returns the definitions for the Yeesha pages.
    # Gets called whenever the Relto's Age GUI is drawn.
    def GetYeeshaPageDefs(self):

        pageDefs = ""
        vault = ptVault()
        psnlSDL = vault.getPsnlAgeSDL()
        if psnlSDL:
            for SDLVar, page in xLinkingBookDefs.xYeeshaPages:
                FoundValue = psnlSDL.findVar(SDLVar)
                if FoundValue is not None:
                    PtDebugPrint(u"xKI.GetYeeshaPageDefs(): The previous value of the SDL variable \"{}\" is {}.".format(SDLVar, FoundValue.getInt()), level=kDebugDumpLevel)
                    state = FoundValue.getInt() % 10
                    if state != 0:
                        active = 1
                        if state == 2 or state == 3:
                            active = 0
                        try:
                            pageDefs += page % (active)
                        except LookupError:
                            pageDefs += "<pb><pb>Bogus page {}".format(SDLVar)
        else:
            PtDebugPrint(u"xKI.GetYeeshaPageDefs(): Trying to access the Chronicle psnlSDL failed: psnlSDL = \"{}\".".format(psnlSDL), level=kErrorLevel)
        return pageDefs

    ## Turns on and off the Yeesha pages' SDL values.
    def ToggleYeeshaPageSDL(self, varName, on):
        vault = ptVault()
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
                    PtDebugPrint(u"xKI.ToggleYeeshaPageSDL(): Setting {} to {}.".format(varName, value), level=kDebugDumpLevel)
                    ypageSDL.setInt((size * 10) + value)
                    vault.updatePsnlAgeSDL(psnlSDL)

    #~~~~~~~~~~~#
    # Censoring #
    #~~~~~~~~~~~#

    ## Sets the censor level.
    # By default, it's set at PG, but it fetches the real value from the
    # chronicle. If it is not found in the chronicle, it will set it to PG.
    def DetermineCensorLevel(self):

        self.censorLevel = xCensor.xRatedPG
        vault = ptVault()
        entry = vault.findChronicleEntry(kChronicleCensorLevel)
        if entry is None:
            vault.addChronicleEntry(kChronicleCensorLevel, kChronicleCensorLevelType, str(self.censorLevel))
        else:
            self.censorLevel = int(entry.chronicleGetValue())
        PtDebugPrint(u"xKI.DetermineCensorLevel(): The censor level is {}.".format(self.censorLevel), level=kWarningLevel)

    def GetCensorLevel(self):
        return self.censorLevel

    #~~~~~~~#
    # Fonts #
    #~~~~~~~#

    ## Sets the current font size from the Chronicle.
    def DetermineFontSize(self):

        fontSize = self.GetFontSize()
        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FontSize)
        if entry is None:
            # Not found, add the current size to the Chronicle.
            vault.addChronicleEntry(kChron.FontSize, kChron.FontSizeType, str(fontSize))
        else:
            fontSize = int(entry.chronicleGetValue())
            self.SetFontSize(fontSize)
        PtDebugPrint(u"xKI.DetermineFontSize(): The saved font size is {}.".format(fontSize), level=kWarningLevel)

    ## Saves the current font size to the Chronicle.
    def SaveFontSize(self):

        fontSize = self.GetFontSize()
        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FontSize)
        if entry is not None:
            entry.chronicleSetValue(str(fontSize))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.FontSize, kChron.FontSizeType, str(fontSize))
        PtDebugPrint(u"xKI.SaveFontSize(): Saving font size of {}.".format(fontSize), level=kWarningLevel)

    ## Returns the font size currently applied to the KI.
    def GetFontSize(self):

        if self.KILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        miniChatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        return miniChatArea.getFontSize()

    ## Applies the specified font size.
    def SetFontSize(self, fontSize):

        PtDebugPrint(u"xKI.SetFontSize(): Setting font size to {}.".format(fontSize), level=kWarningLevel)
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
    def ChangeFontSize(self, new):

        size = self.GetFontSize()
        if new == 1:
            fontRange = range(len(kChat.FontSizeList) - 1)
        elif new == -1:
            fontRange = range(len(kChat.FontSizeList) - 1, 0, -1)
        for i in fontRange:
            if size <= kChat.FontSizeList[i] and new == 1:
                size = kChat.FontSizeList[i + 1]
                break
                size = kChat.FontSizeList[i - 1]
                break
        self.SetFontSize(size)
        self.SaveFontSize()
        self.RefreshKISettings()

    #~~~~~~~~#
    # Fading #
    #~~~~~~~~#

    ## Gets the current fading time from the Chronicle.
    def DetermineFadeTime(self):

        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FadeTime)
        if entry is None:
            # Not found, add the current fade time to the Chronicle.
            vault.addChronicleEntry(kChron.FadeTime, kChron.FadeTimeType, str(self.chatMgr.ticksOnFull))
        else:
            self.chatMgr.ticksOnFull = int(entry.chronicleGetValue())
            if self.chatMgr.ticksOnFull == kChat.FadeTimeMax:
                # Disable the fade altogether.
                self.chatMgr.fadeEnableFlag = False
                self.KillFadeTimer()
                PtDebugPrint(u"xKI.DetermineFadeTime(): Fade time disabled.", level=kWarningLevel)
            else:
                self.chatMgr.fadeEnableFlag = True
        PtDebugPrint(u"xKI.DetermineFadeTime(): The saved fade time is {}.".format(self.chatMgr.ticksOnFull), level=kWarningLevel)

    ## Saves the current fading time to the Chronicle.
    def SaveFadeTime(self):

        vault = ptVault()
        entry = vault.findChronicleEntry(kChron.FadeTime)
        if entry is not None:
            entry.chronicleSetValue(str(self.chatMgr.ticksOnFull))
            entry.save()
        else:
            vault.addChronicleEntry(kChron.FadeTime, kChron.FadeTimeType, str(self.chatMgr.ticksOnFull))
        PtDebugPrint(u"xKI.SaveFadeTime(): Saving Fade Time of {}.".format(self.chatMgr.ticksOnFull), level=kWarningLevel)

    ## Start the fade timer.
    # This gets called each time the user does something in relation to the
    # chat to keep it alive.
    def StartFadeTimer(self):

        if not self.chatMgr.fadeEnableFlag:
            return
        if not BigKI.dialog.isEnabled():
            if self.chatMgr.fadeMode in (kChat.FadeNotActive, kChat.FadeDone):
                PtAtTimeCallback(self.key, kChat.FullTickTime, kTimers.Fade)
            self.chatMgr.fadeMode = kChat.FadeFullDisp
            self.currentFadeTick = self.chatMgr.ticksOnFull

    ## End the currently-active timer.
    def KillFadeTimer(self):

        if self.KILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog

        # Optimization: only do this if we are fading or have faded
        if self.chatMgr.fadeMode in (kChat.FadeDoingFade, kChat.FadeDone, kChat.FadeNotActive):
            mKIdialog.setForeColor(-1, -1, -1, self.originalForeAlpha)
            mKIdialog.setSelectColor(-1, -1, -1, self.originalSelectAlpha)
            if self.KILevel == kNormalKI:
                playerlist = ptGUIControlListBox(mKIdialog.getControlFromTag(kGUI.PlayerList))
                playerlist.show()
            chatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatArea.enableScrollControl()
            mKIdialog.refreshAllControls()

        # Toggle state
        if self.chatMgr.fadeMode not in (kChat.FadeNotActive, kChat.FadeDone):
            self.chatMgr.fadeMode = kChat.FadeStopping
        self.currentFadeTick = self.chatMgr.ticksOnFull

    def ResetFadeState(self, force=False):
        """This turns the chat fade OFF and resets it if the user is not chatting.
           Use this instead of calling `KillFadeTimer()` and `StartFadeTimer()` to toggle the state.
           Use `force` to disable checking of the chatting status (why would you do that?)
        """

        # I'm cheating.
        self.KillFadeTimer()
        if not self.chatMgr.isChatting or force:
            self.StartFadeTimer()

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
            self.chatMgr.fadeMode = kChat.FadeDone

    #~~~~~~#
    # Ages #
    #~~~~~~#

    ## Determines whether or not the player can invite visitors to an Age.
    def CanAgeInviteVistors(self, ageInfo, link):

        # Make sure it's not a special Age.
        try:
            for Age in kAges.NoInvite:
                if Age == ageInfo.getAgeFilename():
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

    ## Determines if the Age is the player's Neighborhood.
    def IsAgeMyNeighborhood(self, ageInfo):

        try:
            hoodGUID = ptVault().getLinkToMyNeighborhood().getAgeInfo().getAgeInstanceGuid()
            if not isinstance(hoodGUID, str) or not hoodGUID:
                PtDebugPrint(u"xKI.IsAgeMyNeighborhood(): Neighborhood GUID not valid.", level=kWarningLevel)
                # Can't trust this test, try a different one.
                if ageInfo.getAgeFilename() == "Neighborhood":
                    return True
            else:
                if ageInfo.getAgeInstanceGuid() == hoodGUID:
                    return True
        except AttributeError:
            pass
        return False

    #~~~~~~~~~~~#
    # Age Names #
    #~~~~~~~~~~~#

    ## Returns the formatted and filtered name of an Age instance.
    def GetAgeInstanceName(self, ageInfo=None):

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
            return FilterAgeName(ageInfo.getAgeInstanceName())
        else:
            return "?UNKNOWN?"

    ## Returns the file name of the specified Age.
    def GetAgeFileName(self, ageInfo=None):

        if ageInfo is None:
            ageInfo = PtGetAgeInfo()
        if ageInfo is not None:
            return ageInfo.getAgeFilename()
        else:
            return "?UNKNOWN?"

    #~~~~~~~~#
    # Limits #
    #~~~~~~~~#

    ## Update the used-up space on the KI.
    def UpdateKIUsage(self):

        usage = ptVault().getKIUsage()
        self.numberOfPictures = usage[0]
        self.numberOfNotes = usage[1]
        self.numberOfMarkerFolders = usage[2]
        try:
            self.numberOfMarkers = self.markerGameManager.marker_total
        except:
            self.numberOfMarkers = -1

    ## Check if the player has reached his limit of picture space.
    def CanTakePicture(self):

        self.UpdateKIUsage()
        if kLimits.MaxPictures == -1 or self.numberOfPictures < kLimits.MaxPictures:
            return True
        return False

    ## Check if the player has reached his limit of journal notes space.
    def CanMakeNote(self):

        self.UpdateKIUsage()
        if kLimits.MaxNotes == -1 or self.numberOfNotes < kLimits.MaxNotes:
            return True
        return False

    ## Check if the player has reached his limit of Marker Games.
    def CanMakeMarkerGame(self):

        self.UpdateKIUsage()
        if kLimits.MaxMarkerFolders == -1 or self.numberOfMarkerFolders < kLimits.MaxMarkerFolders:
            return True
        return False

    ## Check if the player has reached his limit of markers for a Marker Game.
    def CanMakeMarker(self):

        self.UpdateKIUsage()
        if kLimits.MaxMarkers == -1 or self.numberOfMarkers < kLimits.MaxMarkers:
            return True
        return False

    #~~~~~~~~#
    # Errors #
    #~~~~~~~~#

    ## Displays a OK dialog-based error message to the player.
    def ShowKIFullErrorMsg(self, msg):

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


    ## Display an error message in the SendTo field.
    def SetSendToErrorMessage(self, message):

        self.BKPlayerSelected = None
        sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
        sendToField.setStringW(U"<" + unicode(message) + U">")
        sendToButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        sendToButton.hide()

    #~~~~~~~~~~~~~#
    # Invitations #
    #~~~~~~~~~~~~~#

    ## Invite another player to visit the player's Age.
    def InviteToVisit(self, playerID, ageInfo):

        whereToLink = ptAgeLinkStruct()
        whereToLink.setAgeInfo(ageInfo.asAgeInfoStruct())
        ptVault().invitePlayerToAge(whereToLink, playerID)
        self.SendInviteRevoke(playerID, ageInfo.getDisplayName(), "KI.Invitation.VisitTitle", "KI.Invitation.VisitBody")

    ## Send an invitation or a revocation to another player.
    def SendInviteRevoke(self, playerID, ageName, title, message):

        localPlayer = PtGetLocalPlayer()
        invite = ptVaultTextNoteNode(0)
        invite.noteSetText(PtGetLocalizedString(message, [ageName, localPlayer.getPlayerName()]))
        invite.noteSetTitle(PtGetLocalizedString(title, [ageName]))
        invite.sendTo(playerID)

    #~~~~~~~~~~~~~~~~~#
    # New Item Alerts #
    #~~~~~~~~~~~~~~~~~#

    ## Check the Inbox for unseen messages.
    def CheckInboxForUnseen(self):

        inFolder = ptVault().getInbox()
        if inFolder is not None:
            inRefList = inFolder.getChildNodeRefList()
            for inRef in inRefList:
                if not inRef.beenSeen():
                    self.AlertKIStart()

    ## Start the KI Alert if it's not already active.
    def AlertKIStart(self):

        if self.KILevel >= kNormalKI:
            PtFlashWindow()
            if not self.alertTimerActive:
                PtDebugPrint(u"xKI.AlertKIStart(): Show KI alert.", level=kDebugDumpLevel)
                NewItemAlert.dialog.show()
            KIAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
            self.alertTimeToUse = kAlertTimeDefault
            KIAlert.show()

    ## Start the Book Alert if it's not already active.
    def AlertBookStart(self, time=kAlertTimeDefault):

        if not self.alertTimerActive:
            PtDebugPrint(u"xKI.AlertBookStart(): Show Book Alert.", level=kDebugDumpLevel)
            NewItemAlert.dialog.show()
        bookAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
        self.alertTimeToUse = time
        bookAlert.show()

    ## Stop all alerts, by hiding their dialogs.
    def AlertStop(self):

        self.alertTimerActive = False
        NewItemAlert.dialog.hide()
        KIAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertKIAlert))
        KIAlert.hide()
        bookAlert = ptGUIControlButton(NewItemAlert.dialog.getControlFromTag(kAlertBookAlert))
        bookAlert.hide()

    ## Starts the alert timer if it's not already active.
    def AlertStartTimer(self):

        if not self.alertTimerActive:
            self.alertTimerActive = True
            PtAtTimeCallback(self.key, self.alertTimeToUse, kTimers.AlertHide)

    #~~~~~~~~~~~~~#
    # Player List #
    #~~~~~~~~~~~~~#

    ## Tries to scroll up the list of players.
    def ScrollPlayerList(self, direction):

        if self.KILevel == kMicroKI:
            return
        elif self.KILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        control = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        currPos = control.getScrollPos()
        if direction == 1:
            if currPos < control.getScrollRange():
                PtDebugPrint(u"xKI.ScrollPlayerList(): Scrolling player list up from {} to {}.".format(currPos, currPos + 1), level=kDebugDumpLevel)
                control.setScrollPos(currPos + 1)
            else:
                PtDebugPrint(u"xKI.ScrollPlayerList(): Not scrolling player list up from {}.".format(currPos), level=kDebugDumpLevel)
        else:
            if currPos > 0:
                PtDebugPrint(u"xKI.ScrollPlayerList(): Scrolling player list down from {} to {}.".format(currPos, currPos - 1), level=kDebugDumpLevel)
                control.setScrollPos(currPos - 1)
            else:
                PtDebugPrint(u"xKI.ScrollPlayerList(): Not scrolling player list down from {}.".format(currPos), level=kDebugDumpLevel)
        self.CheckScrollButtons()
        mKIdialog.refreshAllControls()
        self.ResetFadeState()

    ## Checks to see if the player list scroll buttons should be visible.
    def CheckScrollButtons(self):

        if self.KILevel == kMicroKI:
            return
        elif self.KILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        control = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        currentPos = control.getScrollPos()
        PtDebugPrint(u"xKI.CheckScrollButtons(): Current position = {} and range = {}.".format(currentPos, control.getScrollRange()), level=kDebugDumpLevel)
        try:
            dbtn = ptGUIControlButton(mKIdialog.getControlFromTag(kGUI.miniPlayerListDown))
            if currentPos == 0:
                dbtn.hide()
            else:
                dbtn.show()
            ubtn = ptGUIControlButton(mKIdialog.getControlFromTag(kGUI.miniPlayerListUp))
            if currentPos >= control.getScrollRange():
                ubtn.hide()
            else:
                ubtn.show()
        except KeyError:
            pass

    ## Reloads the player list with the latest values and displays them.
    def RefreshPlayerList(self, forceSmall=False):

        PtDebugPrint(u"xKI.RefreshPlayerList(): Refreshing.", level=kDebugDumpLevel)
        playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        select = playerlist.getSelection()
        if select >= 0 and select < len(self.BKPlayerList):
            self.previouslySelectedPlayer = self.BKPlayerList[select]
            # Vault node refs change frequently, so get the unique ID instead.
            if isinstance(self.previouslySelectedPlayer, ptVaultNodeRef):
                PtDebugPrint(u"xKI.RefreshPlayerList(): Getting the vault node ID of the selected player.", level=kDebugDumpLevel)
                self.previouslySelectedPlayer = self.previouslySelectedPlayer.getChild().getID()
        else:
            self.previouslySelectedPlayer = None
        self.BKPlayerList = []
        vault = ptVault()

        # Age Players
        ageMembers = KIFolder(PtVaultStandardNodes.kAgeMembersFolder)
        if ageMembers is not None:
            self.BKPlayerList.append(ageMembers)
            self.BKPlayerList += PtGetPlayerListDistanceSorted()
        else:
            self.BKPlayerList.append("?NOAgeMembers?")

        # Buddies List
        buddies = vault.getBuddyListFolder()
        if buddies is not None:
            self.BKPlayerList.append(buddies)
            self.BKPlayerList += self.RemoveOfflinePlayers(buddies.getChildNodeRefList())
        else:
            self.BKPlayerList.append("?NOBuddies?")

        # Neighbors List
        neighbors = GetNeighbors()
        if neighbors is not None:
            self.BKPlayerList.append(neighbors)
            onlinePlayers = self.RemoveOfflinePlayers(neighbors.getChildNodeRefList())
            FilterPlayerInfoList(onlinePlayers)
            self.BKPlayerList += onlinePlayers
        else:
            self.BKPlayerList.append("NEIGHBORS")

        # All Players (INTERNAL CLIENT ONLY)
        if PtIsInternalRelease():
            allPlayers = vault.getAllPlayersFolder()
            if allPlayers:
                self.BKPlayerList.append(allPlayers)
                onlinePlayers = self.RemoveOfflinePlayers(allPlayers.getChildNodeRefList())
                FilterPlayerInfoList(onlinePlayers)
                self.BKPlayerList += onlinePlayers
            # don't append a dummy -- we don't care if our vault doesn't have a copy of AllPlayers

        # Age Devices
        if self.folderOfDevices and BigKI.dialog.isEnabled() and not forceSmall:
            self.BKPlayerList.append(self.folderOfDevices)
            for device in self.folderOfDevices:
                self.BKPlayerList.append(device)

        # Pass the new value to the chat manager.
        self.chatMgr.BKPlayerList = self.BKPlayerList

        # Refresh the display.
        self.RefreshPlayerListDisplay()

    ## Removes the offline players in a list of players.
    def RemoveOfflinePlayers(self, playerlist):

        onlineList = []
        ignores = ptVault().getIgnoreListFolder()
        for plyr in playerlist:
            if isinstance(plyr, ptVaultNodeRef):
                PLR = plyr.getChild()
                PLR = PLR.upcastToPlayerInfoNode()
                if PLR is not None and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                    if PLR.playerIsOnline():
                        if not ignores.playerlistHasPlayer(PLR.playerGetID()):
                            onlineList.append(plyr)
        return onlineList

    ## Refresh the display of the player list.
    def RefreshPlayerListDisplay(self):

        playerlist = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        scrollPos = playerlist.getScrollPos()
        playerlist.lock()
        playerlist.clearAllElements()
        newSelection = -1  # Assume no selection.

        idx = 0
        for plyr in self.BKPlayerList:
            if isinstance(plyr, DeviceFolder):
                playerlist.closeBranch()
                playerlist.addBranchW(plyr.name.upper(), 1)
            elif isinstance(plyr, Device):
                playerlist.addStringWithColor(plyr.name, kColors.DniSelectable, kSelectUseGUIColor)
            elif isinstance(plyr, ptVaultNodeRef):
                PLR = plyr.getChild()
                PLR = PLR.upcastToPlayerInfoNode()
                if PLR is not None and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                    if PLR.playerIsOnline():
                        playerlist.addStringWithColor(PLR.playerGetName(), kColors.DniSelectable, kSelectUseGUIColor)
                    else:
                        playerlist.addStringWithColor(PLR.playerGetName(), kColors.AgenBlueDk,kSelectDetermined)
                else:
                    PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): Unknown player element type {}.".format(PLR.getType()), level=kErrorLevel)
            elif isinstance(plyr, ptPlayer):
                preText = " "
                postText = " "
                if plyr.getPlayerID() != 0:
                    if plyr.getDistanceSq() < PtMaxListenDistSq():
                        preText = ">"
                        postText = "<"
                if plyr.getPlayerName() != "":
                    playerlist.addStringWithColor(preText + plyr.getPlayerName() + postText, kColors.DniSelectable, kSelectUseGUIColor)
                else:
                    if plyr.getPlayerID() != 0:
                        playerlist.addStringWithColor(preText + "[ID:{:08d}]".format(plyr.getPlayerID()) + postText, kColors.DniSelectable, kSelectDetermined)
                    else:
                        playerlist.addStringWithColor(preText + "?unknown user?" + postText, kColors.DniSelectable, kSelectDetermined)
            elif isinstance(plyr, KIFolder):
                playerlist.closeBranch()
                playerlist.addBranchW(plyr.name.upper(), 1)
            elif isinstance(plyr, ptVaultPlayerInfoListNode):
                # It's a player list, display its name.
                fldrType = plyr.folderGetType()
                if fldrType == PtVaultStandardNodes.kAgeOwnersFolder:
                    fldrType = PtVaultStandardNodes.kHoodMembersFolder
                playerlist.closeBranch()
                playerlist.addBranchW(xLocTools.FolderIDToFolderName(fldrType).upper(), 1)
            elif isinstance(plyr, ptVaultMarkerGameNode):
                # its a marker list, display its name
                playerlist.closeBranch()
                playerlist.addBranchW(plyr.folderGetName(), 1)
            elif isinstance(plyr, str):
                playerlist.closeBranch()
                playerlist.addBranchW(plyr, 1)
            else:
                PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): Unknown list type ", plyr, level=kErrorLevel)
            # Is it the selected player?
            if self.previouslySelectedPlayer is not None:
                PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): A previously selected player.", self.previouslySelectedPlayer, level=kDebugDumpLevel)
                # Fix for vaultNodeRef comparisons (which no longer work).
                if isinstance(self.previouslySelectedPlayer, long) and isinstance(plyr, ptVaultNodeRef):
                    plyr = plyr.getChild().getID()  # Set to the ID; let the testing begin.
                # Was it the same class?
                if self.previouslySelectedPlayer.__class__ == plyr.__class__:
                    PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): Previous player matches class.", level=kDebugDumpLevel)
                    # And finally, was it the same object?
                    if self.previouslySelectedPlayer == plyr:
                        PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): Previous player matches object, setting to {}.".format(idx), level=kDebugDumpLevel)
                        newSelection = idx
                        # Found him, stop looking.
                        self.previouslySelectedPlayer = None
                    else:
                        PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): Previous player does not match object.", level=kDebugDumpLevel)
                else:
                    PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): Previous player does not match class.", level=kDebugDumpLevel)
            idx += 1
        # Is there no selection?
        if newSelection == -1:
            # Select the first item in the list.
            newSelection = 0
            # Put the caret back to the regular prompt.
            caret = ptGUIControlTextBox(KIMini.dialog.getControlFromTag(kGUI.ChatCaretID))
            caret.setString(">")
        PtDebugPrint(u"xKI.RefreshPlayerListDisplay(): Setting new selection to {}.".format(newSelection), level=kDebugDumpLevel)
        playerlist.setSelection(newSelection)
        self.previouslySelectedPlayer = None

        # Re-establish the selection the player had before.
        playerlist.setScrollPos(scrollPos)
        playerlist.unlock()

        self.CheckScrollButtons()

        # Set the SendTo button.
        sendToButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        if self.BKPlayerSelected is None:
            sendToButton.hide()
        else:
            # Make sure that the person is still here (this shouldn't happen).
            if isinstance(self.BKPlayerSelected, DeviceFolder):
                self.BKPlayerSelected = None
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                sendToField.setString("  ")
                sendToButton.hide()
            # Otherwise see if the device is still in range.
            elif isinstance(self.BKPlayerSelected, Device):
                try:
                    self.folderOfDevices.index(self.BKPlayerSelected)
                except ValueError:
                    # No longer in the list of devices; remove it.
                    self.BKPlayerSelected = None
                    sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                    sendToField.setString("  ")
                    sendToButton.hide()

    #~~~~~~~~~~#
    # Settings #
    #~~~~~~~~~~#

    ## Refresh the KI configuration settings to match the current values.
    def RefreshKISettings(self):

        fontSizeSlider = ptGUIControlKnob(KISettings.dialog.getControlFromTag(kGUI.BKIKIFontSize))
        fontSize = self.GetFontSize()
        # Find font size in font table.
        whichFont = 0
        for fs in kChat.FontSizeList:
            if fontSize <= fs:
                break
            whichFont += 1
        if whichFont >= len(kChat.FontSizeList):
            whichFont = len(kChat.FontSizeList) - 1
        slidePerFont = float(fontSizeSlider.getMax() - fontSizeSlider.getMin() + 1.0) / float(len(kChat.FontSizeList))
        FSslider = int(slidePerFont * whichFont + 0.25)
        fontSizeSlider.setValue(FSslider)

        fadeTimeSlider = ptGUIControlKnob(KISettings.dialog.getControlFromTag(kGUI.BKIKIFadeTime))
        slidePerTime = float(fadeTimeSlider.getMax() - fadeTimeSlider.getMin()) / float(kChat.FadeTimeMax)
        if not self.chatMgr.fadeEnableFlag:
            self.chatMgr.ticksOnFull = kChat.FadeTimeMax
        FTslider = slidePerTime * self.chatMgr.ticksOnFull
        fadeTimeSlider.setValue(FTslider)

        onlyPMCheckbox = ptGUIControlCheckBox(KISettings.dialog.getControlFromTag(kGUI.BKIKIOnlyPM))
        onlyPMCheckbox.setChecked(self.onlyGetPMsFromBuddies)

    ## Refresh the volume settings to match the current values.
    def RefreshVolumeSettings(self):

        audio = ptAudioControl()
        soundFX = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKISoundFXVolSlider))
        setting = audio.getSoundFXVolume()
        soundFX.setValue(setting * 10)

        music = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIMusicVolSlider))
        setting = audio.getMusicVolume()
        music.setValue(setting * 10)

        voice = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIVoiceVolSlider))
        setting = audio.getVoiceVolume()
        voice.setValue(setting * 10)

        ambience = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIAmbienceVolSlider))
        setting = audio.getAmbienceVolume()
        ambience.setValue(setting * 10)

        miclevel = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIMicLevelSlider))
        setting = audio.getMicLevel()
        miclevel.setValue(setting * 10)

        guivolume = ptGUIControlValue(KIVolumeExpanded.dialog.getControlFromTag(kGUI.BKIGUIVolSlider))
        setting = audio.getGUIVolume()
        guivolume.setValue(setting * 10)

    ## Refresh the Age Owner settings to match the current values.
    def RefreshAgeOwnerSettings(self):

        # Is it actually going to display, or is it just an update?
        if BigKI.dialog.isEnabled() and self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
            try:
                # Get the selected Age config setting.
                myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
            except LookupError:
                myAge = None
            if myAge is not None:
                title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleTB))
                title.setString(GetAgeName(myAge))
                titlebtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleBtn))
                titlebtn.enable()
                titleEdit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleEditbox))
                titleEdit.hide()
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

                # For now, Ages can be made public/private only through the Nexus.
                makepublicTB = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerMakePublicTB))
                makepublicBtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerMakePublicBtn))
                makepublicBtn.disable()
                makepublicTB.hide()
                makepublicTB.setString(" ")
                status.setStringW(PtGetLocalizedString("KI.Neighborhood.AgeOwnedStatus", [str(numowners), str(osess), str(numvisitors), str(vsess)]))
                descript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescription))
                encoded = buffer(myAge.getAgeDescription())
                descript.setEncodedBuffer(encoded)

    #~~~~~~~~#
    # miniKI #
    #~~~~~~~~#

    ## Refresh the display of the miniKI indicator bars.
    def RefreshMiniKIMarkerDisplay(self):

        PtDebugPrint(u"xKI.RefreshMiniKIMarkerDisplay(): Refreshing {}:{}.".format(self.gMarkerGottenNumber, self.gMarkerToGetNumber), level=kDebugDumpLevel)
        if self.KILevel > kMicroKI:
            if self.gMarkerGottenNumber == self.gMarkerToGetNumber and (self.gMarkerToGetNumber % 25) == 0:
                xMyMaxMarkers = self.gMarkerToGetNumber
                xMyGotMarkers = self.gMarkerGottenNumber
            else:
                xMyGotMarkers = self.gMarkerGottenNumber % 25
                if self.gMarkerGottenNumber >= math.floor((self.gMarkerToGetNumber / 25)) * 25:
                    xMyMaxMarkers = self.gMarkerToGetNumber % 25
                else:
                    xMyMaxMarkers = 25
            for mcbID in range(kGUI.miniMarkerIndicator01, kGUI.miniMarkerIndicatorLast + 1):
                mcb = ptGUIControlProgress(KIMini.dialog.getControlFromTag(mcbID))
                markerNumber = mcbID - kGUI.miniMarkerIndicator01 + 1
                try:
                    if not self.gKIMarkerLevel or markerNumber > xMyMaxMarkers:
                        mcb.setValue(kGUI.miniMarkerColors["off"])
                    elif markerNumber <= xMyMaxMarkers and markerNumber > xMyGotMarkers:
                        mcb.setValue(kGUI.miniMarkerColors[self.gMarkerToGetColor])
                    else:
                        mcb.setValue(kGUI.miniMarkerColors[self.gMarkerGottenColor])
                except LookupError:
                    PtDebugPrint(u"xKI.RefreshMiniKIMarkerDisplay(): Couldn't find color, defaulting to off.", level=kWarningLevel)
                    mcb.setValue(kGUI.miniMarkerColors["off"])
            btnmtDrip = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZDrip))
            btnmtActive = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZActive))
            btnmtPlaying = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZMarkerGameActive))
            btnmtInRange = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniGZMarkerInRange))
            btnmgNewMarker = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGNewMarker))
            btnmgNewGame = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGNewGame))
            btnmgInactive = ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.miniMGInactive))

            if self.gKIMarkerLevel:
                btnmtDrip.hide()
                if self.gMarkerToGetNumber > self.gMarkerGottenNumber:
                    if self.gGZMarkerInRange:
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

            # Should the Marker Game GUI be displayed?
            if self.gKIMarkerLevel >= kKIMarkerNormalLevel and not self.markerGameManager.is_cgz:
                btnmtDrip.hide()
                btnmtActive.hide()
                btnmtPlaying.hide()
                btnmtInRange.hide()
                try:
                    showMarkers = self.markerGameManager.markers_visible
                except:
                    showMarkers = False
                try:
                    selectedMarker = self.markerGameManager.selected_marker_id
                except :
                    selectedMarker = -1
                if self.markerGameManager.playing:
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

    ## Toggle between the miniKI and the BigKI.
    def ToggleKISize(self):

        if self.KILevel > kMicroKI and (not self.KIDisabled or BigKI.dialog.isEnabled()):
            if self.KIDisabled and BigKI.dialog.isEnabled():
                self.ToggleMiniKI()
                return
            if not self.waitingForAnimation:
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                if BigKI.dialog.isEnabled():
                    self.HideBigKI()
                    # Can't be chatting.
                    self.chatMgr.ToggleChatMode(0)
                    KIBlackbar.dialog.show()
                    if self.lastminiKICenter is not None:
                        dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                        dragbar.setObjectCenter(self.lastminiKICenter)
                        dragbar.unanchor()
                        self.lastminiKICenter = None
                    # Refresh the player list, because it will be the shorter version.
                    self.RefreshPlayerList(True)
                    toggleCB.setChecked(0)
                else:
                    # If there is nothing showing, just bring up the miniKI.
                    if not KIMini.dialog.isEnabled():
                        self.chatMgr.ClearBBMini(0)
                    # Bring up the BigKI, then the miniKI.
                    else:
                        self.waitingForAnimation = True
                        KIBlackbar.dialog.hide()
                        KIMini.dialog.hide()
                        # Can't be chatting.
                        self.chatMgr.ToggleChatMode(0)
                        # Show the BigKI.
                        BigKI.dialog.show()
                        # Save current location and snap back to original.
                        if self.originalminiKICenter is not None:
                            dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                            self.lastminiKICenter = dragbar.getObjectCenter()
                            PtDebugPrint(u"xKI.ToggleKISize(): Distance to original = {}.".format(self.lastminiKICenter.distance(self.originalminiKICenter)), level=kDebugDumpLevel)
                            # If they are close, then snap it to original.
                            if self.lastminiKICenter.distance(self.originalminiKICenter) < 0.027:
                                self.lastminiKICenter = self.originalminiKICenter
                            dragbar.setObjectCenter(self.originalminiKICenter)
                            dragbar.anchor()
                        KIMini.dialog.show()
                        toggleCB.setChecked(1)

    ## Put away the miniKI (and the BigKI, if up).
    def ToggleMiniKI(self, forceOpen = 0):

        if self.KILevel > kMicroKI and (not self.KIDisabled or KIMini.dialog.isEnabled()):
            if KIMini.dialog.isEnabled():
                KIMini.dialog.hide()
                # Put the miniKI back where it used to be.
                if self.lastminiKICenter is not None:
                    dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                    dragbar.setObjectCenter(self.lastminiKICenter)
                    dragbar.unanchor()
                    self.lastminiKICenter = None
                if BigKI.dialog.isEnabled():
                    self.HideBigKI()
                KIBlackbar.dialog.show()
                self.chatMgr.ClearBBMini(-1)
                # Put the toggle button back to the miniKI setting.
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                toggleCB.setChecked(0)
                self.sawTheKIAtleastOnce = True
            else:
                # If the miniKI is hidden, show it.
                if forceOpen:
                    self.chatMgr.ClearBBMini(0)

    ## Take a screenshot through the miniKI.
    def TakePicture(self):

        if not self.takingAPicture and not self.waitingForAnimation:
            # Ignoring the KIDisabled flag here, because screenshots can be
            # taken even with certain GUIs showing.
            if self.KILevel > kMicroKI:
                if self.CanTakePicture():
                    self.takingAPicture = True
                    if not PtIsGUIModal():
                        # Hide everything to take a picture.
                        KIBlackbar.dialog.hide()
                        KIMini.dialog.hide()
                        self.HideBigKIMode()
                        BigKI.dialog.hide()
                        # Put the toggle button back to BigKI.
                        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                        toggleCB.setChecked(1)
                    # Wait a moment, then take the picture.
                    PtAtTimeCallback(self.key, 0.25, kTimers.TakeSnapShot)
                else:
                    # Put up an error message.
                    self.ShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullImages"))

    ## Create a new Journal entry through the miniKI.
    def MiniKICreateJournalNote(self):

        if self.takingAPicture or self.waitingForAnimation:
            return
        if self.KILevel > kMicroKI and not self.KIDisabled:
            if self.CanMakeNote():
                KIBlackbar.dialog.hide()
                # Put the toggle button back to BigKI.
                toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
                toggleCB.setChecked(1)
                # Create the actual journal entry.
                self.BigKICreateJournalNote()
                # Make sure that the player is in Journal mode.
                modeselector = ptGUIControlRadioGroup(BigKI.dialog.getControlFromTag(kGUI.BKRadioModeID))
                modeselector.setValue(0)
                # Set things up so that when the BigKI shows, it goes into edit mode.
                if self.BKRightSideMode != kGUI.BKJournalExpanded:
                    self.HideBigKIMode()
                self.BKRightSideMode = kGUI.BKJournalExpanded
                # Reset the top line and selection.
                self.BigKIRefreshFolderDisplay()
                self.BigKIDisplayJournalEntry()
                # Setup to edit the caption of the note.
                self.BigKIEnterEditMode(kGUI.BKEditFieldJRNTitle)
                if BigKI.dialog.isEnabled():
                    self.ShowBigKIMode()
                else:
                    # Put the miniKI on top.
                    KIMini.dialog.hide()
                    BigKI.dialog.show()
                    KIMini.dialog.show()
                # Was just the miniKI showing?
                if self.lastminiKICenter is None:
                    if self.originalminiKICenter is not None:
                        dragbar = ptGUIControlDragBar(KIMini.dialog.getControlFromTag(kGUI.miniDragBar))
                        self.lastminiKICenter = dragbar.getObjectCenter()
                        dragbar.setObjectCenter(self.originalminiKICenter)
                        dragbar.anchor()
            else:
                # Put up an error message.
                self.ShowKIFullErrorMsg(PtGetLocalizedString("KI.Messages.FullNotes"))

    #~~~~~~~#
    # BigKI #
    #~~~~~~~#

    ## Open up and show the BigKI.
    def ShowBigKI(self):

        self.waitingForAnimation = True
        curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
        toggleCB.disable()
        if curBrainMode == PtBrainModes.kNonGeneric:
            PtDebugPrint(u"xKI.ShowBigKI(): Entering LookingAtKI mode.", level=kDebugDumpLevel)
            PtAvatarEnterLookingAtKI()
            self.isPlayingLookingAtKIMode = True
        PtDisableMovementKeys()
        KIOnResp.run(self.key, netPropagate=0)

    ## Close and hide the BigKI.
    def HideBigKI(self):

        self.waitingForAnimation = True
        toggleCB = ptGUIControlCheckBox(KIMini.dialog.getControlFromTag(kGUI.miniToggleBtnID))
        toggleCB.disable()
        self.HideBigKIMode()
        # Make sure the player was actually looking at the KI.
        if self.isPlayingLookingAtKIMode:
            PtDebugPrint(u"xKI.HideBigKI(): Leaving LookingAtKI mode.", level=kDebugDumpLevel)
            PtAvatarExitLookingAtKI()
        self.isPlayingLookingAtKIMode = False
        PtEnableMovementKeys()
        KIOffResp.run(self.key, netPropagate=0)

    ## Show a new mode inside the BigKI.
    # This can be an expanded picture, a player entry, a list...
    def ShowBigKIMode(self):

        if BigKI.dialog.isEnabled():
            # Hide up/down scroll buttons.
            upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMUpButton))
            upbtn.hide()
            dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMDownButton))
            dwnbtn.hide()
            if self.BKRightSideMode == kGUI.BKListMode:
                KIListModeDialog.dialog.show()
                self.BigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKJournalExpanded:
                KIJournalExpanded.dialog.show()
                if self.IsContentMutable(self.BKCurrentContent):
                    self.BigKIInvertToFolderButtons()
                else:
                    self.BigKIOnlySelectedToButtons()
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKPictureExpanded:
                KIPictureExpanded.dialog.show()
                if self.IsContentMutable(self.BKCurrentContent):
                    self.BigKIInvertToFolderButtons()
                else:
                    self.BigKIOnlySelectedToButtons()
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
                KIPlayerExpanded.dialog.show()
                # If the expanded player is ourselves, then no move buttons.
                localPlayer = PtGetLocalPlayer()
                if self.BKCurrentContent is not None:
                    if isinstance(self.BKCurrentContent, ptPlayer):
                        if self.BKCurrentContent.getPlayerID() == localPlayer.getPlayerID():
                            self.BigKIOnlySelectedToButtons()
                            return
                    # Otherwise assume that it's a plVaultNodeRef.
                    else:
                        elem = self.BKCurrentContent.getChild()
                        if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                            elem = elem.upcastToPlayerInfoNode()
                            if elem.playerGetID() == localPlayer.getPlayerID():
                                self.BigKIOnlySelectedToButtons()
                                return
                self.BigKIInvertToFolderButtons()
            elif self.BKRightSideMode == kGUI.BKVolumeExpanded:
                KIVolumeExpanded.dialog.show()
                self.BigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKKIExpanded:
                KISettings.dialog.show()
                self.BigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
                KIAgeOwnerExpanded.dialog.show()
                self.BigKIOnlySelectedToButtons()
                self.BKCurrentContent = None
                self.BKGettingPlayerID = False
            elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                KIMarkerFolderExpanded.dialog.show()
                if self.IsContentMutable(self.BKCurrentContent):
                    self.BigKIInvertToFolderButtons()
                else:
                    self.BigKIOnlySelectedToButtons()
                self.BKGettingPlayerID = False

    ## Hide an open mode in the BigKI.
    def HideBigKIMode(self):

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

    ## Switch to a new mode in the BigKI.
    # This hides the old mode and displays the new one, or just refreshes
    # the content list if it's a selection change.
    def ChangeBigKIMode(self, newMode):

        # Is the player switching to a new mode?
        if newMode != self.BKRightSideMode:
            self.HideBigKIMode()
            self.BKRightSideMode = newMode
            self.ShowBigKIMode()
        # Or is he changing the selection?
        elif newMode == kGUI.BKListMode:
            self.BigKIOnlySelectedToButtons()

    ## Set the SendTo buttons appropriately.
    # This will set all the little glowing arrows next to items in accordance
    # with the currently displayed mode.
    def SetBigKIToButtons(self):

        if self.BKRightSideMode == kGUI.BKListMode:
            self.BigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKJournalExpanded:
            if self.IsContentMutable(self.BKCurrentContent):
                self.BigKIInvertToFolderButtons()
            else:
                self.BigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKPictureExpanded:
            if self.IsContentMutable(self.BKCurrentContent):
                self.BigKIInvertToFolderButtons()
            else:
                self.BigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
            localPlayer = PtGetLocalPlayer()
            if self.BKCurrentContent is not None:
                if isinstance(self.BKCurrentContent, ptPlayer):
                    if self.BKCurrentContent.getPlayerID() == localPlayer.getPlayerID():
                        self.BigKIOnlySelectedToButtons()
                        return
                # Otherwise assume that it's a plVaultNodeRef.
                else:
                    elem = self.BKCurrentContent.getChild()
                    if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                        elem = elem.upcastToPlayerInfoNode()
                        if elem.playerGetID() == localPlayer.getPlayerID():
                            self.BigKIOnlySelectedToButtons()
                            return
            self.BigKIInvertToFolderButtons()
        elif self.BKRightSideMode == kGUI.BKVolumeExpanded:
            self.BigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKKIExpanded:
            self.BigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
            self.BigKIOnlySelectedToButtons()
        elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
            if self.MFdialogMode not in (kGames.MFEditing, kGames.MFEditingMarker) and self.IsContentMutable(self.BKCurrentContent):
                self.BigKIInvertToFolderButtons()
            else:
                self.BigKIOnlySelectedToButtons()

    ## Show only the selected SendTo buttons.
    def BigKIOnlySelectedToButtons(self):

        toPlayerBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        toPlayerBtn.hide()
        self.BigKIRefreshFolderDisplay()
        # Hide all the buttons.
        for ID in range(kGUI.BKIToIncomingButton, kGUI.BKIToFolderButtonLast + 1):
            toFolder = ptGUIControlButton(BigKI.dialog.getControlFromTag(ID))
            toFolder.hide()
        self.BigKINewContentList()

    ## Determines if the selected content can be sent to someone.
    def BigKICanShowSendToPlayer(self):

        # Make sure that there is a selected player.
        if self.BKPlayerSelected is None:
            return False

        # Make sure that it's something that can be sent to a player.
        if self.BKRightSideMode == kGUI.BKPlayerExpanded or self.BKRightSideMode == kGUI.BKVolumeExpanded or self.BKRightSideMode == kGUI.BKAgeOwnerExpanded:
            return False

        # Make sure that it's not the player.
        if isinstance(self.BKPlayerSelected, ptVaultNodeRef):
            plyrElement = self.BKPlayerSelected.getChild()
            if plyrElement is not None and plyrElement.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                plyrElement = plyrElement.upcastToPlayerInfoNode()
                if plyrElement.playerGetID() == PtGetLocalClientID():
                    return False

        return True

    ## Hides or shows the ToPlayer buttons.
    def BigKIInvertToFolderButtons(self):

        # Setup ToPlayer button.
        toPlayerBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
        if self.BigKICanShowSendToPlayer():
            toPlayerBtn.show()
        else:
            toPlayerBtn.hide()

        # Add the ToPlayer button to the elements.
        selectedButton = self.BKFolderSelected - self.BKFolderTopLine + kGUI.BKIToIncomingButton
        for ID in range(kGUI.BKIToIncomingButton, kGUI.BKIToFolderButtonLast + 1):
            toFolder = ptGUIControlButton(BigKI.dialog.getControlFromTag(ID))
            if ID == selectedButton:
                toFolder.hide()
            else:
                # Don't show on elements that are not there or immutable.
                if ID - kGUI.BKIToIncomingButton <= len(self.BKFolderListOrder) - 1 - self.BKFolderTopLine:
                    try:
                        if self.IsFolderContentMutable(self.BKFolderLineDict[self.BKFolderListOrder[ID - kGUI.BKIToIncomingButton + self.BKFolderTopLine]]):
                            toFolder.show()
                        else:
                            toFolder.hide()
                    except LookupError:
                        toFolder.hide()
                else:
                    toFolder.hide()

    ## Check incoming content for the sender, and set the SendTo field.
    def CheckContentForSender(self, content):

        folder = content.getParent()
        if folder:
            folder = folder.upcastToFolderNode()
        if folder is not None and folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
            sender = content.getSaver()
            if sender is not None and sender.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
                curSendTo = sendToField.getString().strip()
                if not curSendTo:
                    self.BKPlayerSelected = sender
                    sendToField.setString(sender.playerGetName())

    #~~~~~~~~~~~~~~~#
    # BigKI Content #
    #~~~~~~~~~~~~~~~#

    ## Determines whether the specified folder can be modified.
    def IsFolderContentMutable(self, folder):

        # Make sure there is a real folder there to play with.
        if folder is None or not isinstance(folder, ptVaultNode):
            return False

        # If it's not really a folder but an AgeInfoNode, then it's for the canVisit player list.
        if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
            return True

        if folder.getType() != PtVaultNodeTypes.kPlayerInfoListNode and folder.getType() != PtVaultNodeTypes.kFolderNode:
            return False

        # Check for the incoming folder.
        if folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
            return False

        # Check against the AgeMembers folder.
        if folder.folderGetType() == PtVaultStandardNodes.kAgeMembersFolder:
            return False

        # Check for the neighborhood members folder.
        if folder.folderGetType() == PtVaultStandardNodes.kHoodMembersFolder:
            return False

        # Oh hayll no, you can't change AllPlayers
        if folder.folderGetType() == PtVaultStandardNodes.kAllPlayersFolder:
            return False

        # Check for neighborhood CanVisit folder (actually half-mutable, they can delete).
        if folder.folderGetType() == PtVaultStandardNodes.kAgeOwnersFolder:
            return False

        # It's not a special folder, so it's mutable.
        return True

    ## Determines whether this is a hidden folder.
    def IsFolderHidden(self, ageFolder):

        if ageFolder.folderGetName() == "Hidden":
            return True
        return False

    ## Determines whether the content Node Reference is mutable.
    def IsContentMutable(self, nodeRef):

        # Get its parent folder.
        if isinstance(nodeRef, ptVaultNodeRef):
            folder = self.BKCurrentContent.getParent()
            if folder:
                folder = folder.upcastToFolderNode()
                if folder:
                    if folder.folderGetType() == PtVaultStandardNodes.kGlobalInboxFolder:
                        return False
        return True

    #~~~~~~~~~~~~~~#
    # BigKI Values #
    #~~~~~~~~~~~~~~#

    ## Sets some global values for the KI should never change.
    def BigKISetStatics(self):

        ageText = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKICurAgeNameID))
        ageName = GetAgeName().replace("(null)", "").strip()
        PtDebugPrint(u"xKI.BigKISetStatics(): Displaying age name of {}.".format(ageName), level=kDebugDumpLevel)
        ageText.setStringW(ageName)
        playerText = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKPlayerName))
        IDText = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKPlayerID))
        localPlayer = PtGetLocalPlayer()
        playerText.setString(localPlayer.getPlayerName())
        IDText.setString("[ID:{:08d}]".format(localPlayer.getPlayerID()))
        self.UpdatePelletScore()
        self.BigKIRefreshHoodStatics()

    ## Sets some Neighborhood-specific values for the KI that won't change.
    def BigKIRefreshHoodStatics(self, neighborhood=None):

        neighborText = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKNeighborhoodAndID))
        # If a neighborhood was not specified, get the one from the player's Vault.
        if not neighborhood:
            neighborhood = GetNeighborhood()
        if neighborhood is not None:
            neighborName = xLocTools.LocalizeAgeName(neighborhood.getDisplayName())
            if neighborName == U"":
                neighborName = PtGetLocalizedString("KI.Neighborhood.NoName")
            neighborText.setStringW(PtGetLocalizedString("KI.Neighborhood.BottomLine", [xLocTools.MemberStatusString(), neighborName]))
        else:
            neighborText.setStringW(PtGetLocalizedString("KI.Neighborhood.None"))

    ## Sets some global changing values for the KI.
    def BigKISetChanging(self):

        # Use the D'ni time for this Age.
        dniTime = PtGetDniTime()
        if dniTime:
            tupTime = time.gmtime(dniTime)
            if self.timeBlinker:
                curTime = unicode(time.strftime(PtGetLocalizedString("Global.Formats.DateTime"), tupTime))
                self.timeBlinker = False
            else:
                curTime = unicode(time.strftime(PtGetLocalizedString("Global.Formats.DateTime"), tupTime))
                self.timeBlinker = True
        else:
            curTime = PtGetLocalizedString("KI.Errors.TimeBroke")
        if curTime != self.previousTime:
            timeText = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKICurTimeID))
            timeText.setStringW(curTime)
            self.previousTime = curTime
        # Set the D'ni GPS coordinates.
        gps1 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIGPS1TextID))
        gps2 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIGPS2TextID))
        gps3 = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIGPS3TextID))
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
        PtAtTimeCallback(self.key, 5, kTimers.BKITODCheck)

    #~~~~~~~~~~~~~~~~~~#
    # BigKI Refreshing #
    #~~~~~~~~~~~~~~~~~~#

    ## Check to see if a folder needs to be refreshed.
    def BigKICheckFolderRefresh(self, folder=None):

        if folder is not None:
            if folder.getType() == PtVaultNodeTypes.kPlayerInfoListNode:
                self.RefreshPlayerList()
        # Otherwise, check everything just in case.
        else:
            self.RefreshPlayerList()
        # Check content refresh only if using the BigKI.
        if self.KILevel > kMicroKI:
            self.BigKIRefreshContentList()
            self.BigKIRefreshContentListDisplay()

    ## Check to see if the current content has changed since.
    def BigKICheckContentRefresh(self, content):

        if self.BKCurrentContent is not None and content == self.BKCurrentContent:
            if self.BKRightSideMode == kGUI.BKListMode:
                self.BigKIRefreshContentListDisplay()
            elif self.BKRightSideMode == kGUI.BKJournalExpanded:
                self.BigKIDisplayJournalEntry()
            elif self.BKRightSideMode == kGUI.BKPictureExpanded:
                self.BigKIDisplayPicture()
            elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
                self.BigKIDisplayPlayerEntry()
            elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                self.BigKIDisplayMarkerGame()

    ## Check to see if the current content element has changed since.
    def BigKICheckElementRefresh(self, element):

        if self.BKCurrentContent is not None:
            if isinstance(self.BKCurrentContent,ptVaultNodeRef) and element == self.BKCurrentContent.getChild():
                if self.BKRightSideMode == kGUI.BKListMode:
                    self.BigKIRefreshContentListDisplay()
                elif self.BKRightSideMode == kGUI.BKJournalExpanded:
                    self.BigKIDisplayJournalEntry()
                elif self.BKRightSideMode == kGUI.BKPictureExpanded:
                    self.BigKIDisplayPicture()
                elif self.BKRightSideMode == kGUI.BKPlayerExpanded:
                    self.BigKIDisplayPlayerEntry()
                elif self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                    self.BigKIDisplayMarkerGame()

    ## Refresh the list of folders for the Inbox and Age Journal folders.
    def BigKIRefreshFolderList(self):

        # Remember selected and what position in the list the player is at.
        vault = ptVault()

        # Get Journal folder information.
        if xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder) not in self.BKJournalFolderDict:
            inFolder = vault.getInbox()
            if inFolder is not None:
                self.BKJournalListOrder.insert(0, xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder))
                self.BKJournalFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder)] = inFolder

        # Get the Age Journal folders and add any new ones.
        masterAgeFolder = vault.getAgeJournalsFolder()
        if masterAgeFolder is not None:
            ageFolderRefs = masterAgeFolder.getChildNodeRefList()
            for ageFolderRef in ageFolderRefs:
                ageFolder = ageFolderRef.getChild()
                ageFolder = ageFolder.upcastToFolderNode()
                if ageFolder is not None:
                    if not self.IsFolderHidden(ageFolder):
                        ageFolderName = ageFolder.folderGetName()
                        if ageFolderName == "":
                            ageFolderName = "[invalid]"
                        ageFolderName = FilterAgeName(ageFolderName)
                        if ageFolderName in kAges.Hide:
                            continue
                        if ageFolderName not in self.BKJournalFolderDict:
                            # New Age folder, add it.
                            self.BKJournalListOrder.append(ageFolderName)
                        self.BKJournalFolderDict[ageFolderName] = ageFolder
            # Make sure the current Age is at the top of the list.
            try:
                line = self.BKJournalListOrder.index(self.GetAgeInstanceName())
                if line != 1:
                    # It's not at the top of the list, so put it at the top.
                    self.BKJournalListOrder.remove(self.GetAgeInstanceName())
                    self.BKJournalListOrder.insert(1, self.GetAgeInstanceName())
                    # If the player is looking at a Journal entry then switch to list mode.
                    if self.BKRightSideMode == kGUI.BKJournalExpanded or self.BKRightSideMode == kGUI.BKPictureExpanded or self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                        self.ChangeBigKIMode(kGUI.BKListMode)
            except ValueError:
                # Create a folder for most Ages.
                ageName = self.GetAgeFileName().lower()
                if ageName != "startup" and ageName != "avatarcustomization" and ageName != "unknown age" and self.GetAgeInstanceName() != "?unknown?":
                    entry = vault.findChronicleEntry("CleftSolved")
                    cleftSolved = False
                    if entry is not None:
                        if entry.chronicleGetValue() == "yes":
                            cleftSolved = True
                    if self.GetAgeInstanceName() != "D'ni-Riltagamin" or cleftSolved:
                        instAgeName = self.GetAgeInstanceName()
                        createAgeFolder = True

                        ageFolderRefs = masterAgeFolder.getChildNodeRefList()
                        for ageFolderRef in ageFolderRefs:
                            ageFolder = ageFolderRef.getChild()
                            ageFolder = ageFolder.upcastToFolderNode()
                            if ageFolder is not None and ageFolder.getFolderNameW() == instAgeName:
                                createAgeFolder = False
                                break

                        if instAgeName and createAgeFolder:
                            nFolder = ptVaultFolderNode(0)
                            if nFolder is not None:
                                nFolder.setFolderNameW(self.GetAgeInstanceName())
                                nFolder.folderSetType(PtVaultStandardNodes.kAgeTypeJournalFolder)
                                # Add to the master Age folder folder.
                                masterAgeFolder.addNode(nFolder)
                            else:
                                PtDebugPrint(u"xKI.BigKIRefreshFolderList(): Could not create folder for {}.".format(self.GetAgeInstanceName()), level=kErrorLevel)
        else:
            PtDebugPrint(u"xKI.BigKIRefreshFolderList(): Could not find the Master Age jounal folder.", level=kErrorLevel)

        # Get the player lists.
        self.BKPlayerFolderDict.clear()
        self.BKPlayerListOrder = []

        ageMembers = KIFolder(PtVaultStandardNodes.kAgeMembersFolder)
        if ageMembers is not None:
            if xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder) not in self.BKPlayerFolderDict:
                # Add the new player folder.
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder))
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder)] = ageMembers
            PtDebugPrint(u"xKI.BigKIRefreshFolderList(): Updating ageMembers.", level=kDebugDumpLevel)
        else:
            PtDebugPrint(u"xKI.BigKIRefreshFolderList(): AgeMembers folder is missing.", level=kWarningLevel)

        buddies = vault.getBuddyListFolder()
        if buddies is not None:
            if xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder) not in self.BKPlayerFolderDict:
                # Add the new player folder.
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder))
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)] = buddies
        else:
            PtDebugPrint(u"xKI.BigKIRefreshFolderList(): Buddies folder is missing.", level=kWarningLevel)

        # Update the neighborhood folder.
        self.BigKIRefreshNeighborFolder()

        # Update the Recent people folder.
        PIKA = vault.getPeopleIKnowAboutFolder()
        if PIKA is not None:
            if xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder) not in self.BKPlayerFolderDict:
                # Add the new player folder.
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder))
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kPeopleIKnowAboutFolder)] = PIKA
        else:
            PtDebugPrint(u"xKI.BigKIRefreshFolderList(): PeopleIKnowAbout folder is missing.", level=kWarningLevel)
        ignores = vault.getIgnoreListFolder()
        if ignores is not None:
            if xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder) not in self.BKPlayerFolderDict:
                # Add the new player folder.
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder))
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kIgnoreListFolder)] = ignores
        else:
            PtDebugPrint(u"xKI.BigKIRefreshFolderList(): IgnoreList folder is missing.", level=kWarningLevel)

        # All Players
        if PtIsInternalRelease():
            ap = vault.getAllPlayersFolder()
            if ap:
                name = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAllPlayersFolder)
                if name not in self.BKPlayerFolderDict:
                    # Add the new player folder.
                    self.BKPlayerListOrder.append(name)
                self.BKPlayerFolderDict[name] = ap

        # Age Visitors.
        visSep = SeparatorFolder(PtGetLocalizedString("KI.Folders.VisLists"))
        self.BKPlayerListOrder.append(visSep.name)
        self.BKPlayerFolderDict[visSep.name] = visSep
        self.BigKIRefreshAgeVisitorFolders()

        # Age Owners.
        self.BigKIRefreshAgesOwnedFolder()

    ## Refresh the Neighbors folder.
    def BigKIRefreshNeighborFolder(self):

        neighborhood = GetNeighborhood()
        try:
            neighbors = neighborhood.getAgeOwnersFolder()
            if xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder) not in self.BKPlayerFolderDict:
                # Add the new Neighbors folder.
                self.BKPlayerListOrder.append(xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder))
            PtDebugPrint(u"xKI.BigKIRefreshNeighborFolder(): Got the neighbors player folder.", level=kDebugDumpLevel)
            self.BKPlayerFolderDict[xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)] = neighbors
        except AttributeError:
            PtDebugPrint(u"xKI.BigKIRefreshNeighborFolder(): Neighbors folder is missing.", level=kWarningLevel)

    ## Refresh the Age Visitors folders for Ages the player owns.
    def BigKIRefreshAgeVisitorFolders(self):

        vault = ptVault()
        try:
            myAgesFolder = vault.getAgesIOwnFolder()
            listOfMyAgeLinks = myAgesFolder.getChildNodeRefList()
            for myAgeLinkRef in listOfMyAgeLinks:
                myAgeLink = myAgeLinkRef.getChild()
                myAgeLink = myAgeLink.upcastToAgeLinkNode()
                myAge = myAgeLink.getAgeInfo()
                if myAge is not None:
                    if self.CanAgeInviteVistors(myAge, myAgeLink) and myAge.getAgeFilename() not in kAges.Hide:
                        PtDebugPrint(u"xKI.BigKIRefreshAgeVisitorFolders(): Refreshing visitor list for {}.".format(GetAgeName(myAge)), level=kDebugDumpLevel)
                        folderName = xCensor.xCensor(PtGetLocalizedString("KI.Config.OwnerVisitors", [GetAgeName(myAge)]), self.censorLevel)
                        if folderName not in self.BKPlayerFolderDict:
                            # Add the new Age Visitors folder.
                            PtDebugPrint(u"xKI.BigKIRefreshAgeVisitorFolders(): Adding visitor list for {}.".format(GetAgeName(myAge)), level=kDebugDumpLevel)
                            self.BKPlayerListOrder.append(folderName)
                        self.BKPlayerFolderDict[folderName] = myAge
                else:
                    PtDebugPrint(u"xKI.BigKIRefreshAgeVisitorFolders(): Age info for {} is not ready yet.".format(myAgeLink.getUserDefinedName()), level=kErrorLevel)
        except AttributeError:
            PtDebugPrint(u"xKI.BigKIRefreshAgeVisitorFolders(): Error finding Age Visitors folder.", level=kErrorLevel)

    ## Refresh the configuration folder listing for owned Ages.
    # This is currently only used for Neighborhoods.
    def BigKIRefreshAgesOwnedFolder(self):

        # First, get rid of all the Age config entries, in case one of them got deleted.
        self.BKConfigFolderDict.clear()
        self.BKConfigListOrder = []
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
                if myAge is not None:
                    if myAge.getAgeFilename() == "Neighborhood":
                        PtDebugPrint(u"xKI.BigKIRefreshAgesOwnedFolder(): Refreshing owner configuration for Age {}.".format(GetAgeName(myAge)), level=kDebugDumpLevel)
                        configName = xCensor.xCensor(PtGetLocalizedString("KI.Config.OwnerConfig", [GetAgeName(myAge)]), self.censorLevel)
                        if configName not in self.BKConfigFolderDict:
                            # Add the new Age configuration.
                            PtDebugPrint(u"xKI: adding owner config for Age {}.".format(GetAgeName(myAge)), level=kDebugDumpLevel)
                            self.BKConfigListOrder.append(configName)
                        self.BKConfigFolderDict[configName] = myAge
                else:
                    PtDebugPrint(u"xKI.BigKIRefreshAgesOwnedFolder(): Age info for {} is not ready yet.".format(myAgeLink.getUserDefinedName()), level=kErrorLevel)
        except AttributeError:
            PtDebugPrint(u"xKI.BigKIRefreshAgesOwnedFolder(): Error finding Age folder.", level=kErrorLevel)

    ## Reget the contents of the selected content list.
    def BigKINewContentList(self):

        try:
            folderName = self.BKFolderListOrder[self.BKFolderSelected]
            folder = self.BKFolderLineDict[folderName]
            if folder is not None:
                if isinstance(folder, ptVaultNode):
                    if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                        try:
                            self.BKContentList = folder.getCanVisitFolder().getChildNodeRefList()
                        except AttributeError:
                            self.BKContentList = []
                    else:
                        self.BKContentList = folder.getChildNodeRefList()
                    self.BigKIProcessContentList(True)
                    if self.BKFolderSelectChanged:
                        self.BKContentListTopLine = 0
                elif isinstance(folder, KIFolder):
                    self.BKContentList = PtGetPlayerListDistanceSorted()
                    self.BigKIProcessContentList(True)
                    if self.BKFolderSelectChanged:
                        self.BKContentListTopLine = 0
                else:
                    # Shouldn't happen because the player can't click on these.
                    self.BKContentList = []
        except (IndexError, KeyError):
            self.BKContentList = []
        self.BigKIRefreshContentListDisplay()

    ## Refreshes the contents of the selected content list.
    def BigKIRefreshContentList(self):

        try:
            folderName = self.BKFolderListOrder[self.BKFolderSelected]
            folder = self.BKFolderLineDict[folderName]
            if folder is not None:
                if isinstance(folder, ptVaultNode):
                    if folder.getType() == PtVaultNodeTypes.kAgeInfoNode:
                        try:
                            self.BKContentList = folder.getCanVisitFolder().getChildNodeRefList()
                        except AttributeError:
                            self.BKContentList = []
                    else:
                        self.BKContentList = folder.getChildNodeRefList()
                    self.BigKIProcessContentList()
                elif isinstance(folder, KIFolder):
                    self.BKContentList = PtGetPlayerListDistanceSorted()
                    self.BigKIProcessContentList()
            else:
                self.BKContentList = []
        except LookupError:
            pass

    #~~~~~~~~~~~~~~~~~~~~~~~~~~#
    # BigKI Display Refreshing #
    #~~~~~~~~~~~~~~~~~~~~~~~~~~#

    ## Refresh the display of the folders and the selection.
    def BigKIRefreshFolderDisplay(self):

        # Refresh the display of the folders.
        ID = kGUI.BKIIncomingLine
        if self.BKFolderListOrder:
            # Make sure that it is a valid index.
            if self.BKFolderTopLine >= len(self.BKFolderListOrder):
                self.BKFolderTopLine = len(self.BKFolderListOrder) - 1
            # If the selected is off the screen, go to the top then (only in list mode).
            ## @todo Note when the self.BKFolderSelected has changed, refresh the content display.
            if self.BKRightSideMode == kGUI.BKListMode:
                if self.BKFolderSelected < self.BKFolderTopLine:
                    self.BKFolderSelected = self.BKFolderTopLine
                if self.BKFolderSelected > self.BKFolderTopLine + (kGUI.BKIFolderLineLast - kGUI.BKIIncomingLine):
                    self.BKFolderSelected = self.BKFolderTopLine + (kGUI.BKIFolderLineLast - kGUI.BKIIncomingLine)
                if self.BKFolderSelected > self.BKFolderTopLine + len(self.BKFolderListOrder[self.BKFolderTopLine:]) - 1:
                    self.BKFolderSelected = self.BKFolderTopLine + len(self.BKFolderListOrder[self.BKFolderTopLine]) - 1
            selectedFolder = self.BKFolderSelected - self.BKFolderTopLine + kGUI.BKIIncomingLine
            for folderName in self.BKFolderListOrder[self.BKFolderTopLine:]:
                folderField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(ID))
                longFolderField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(ID + 500))
                buttonID = ID - kGUI.BKIFolderLine01+kGUI.BKIFolderLineBtn01
                folderButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(buttonID))
                # Make sure it's not a separator folder.
                if folderName in self.BKFolderLineDict and isinstance(self.BKFolderLineDict[folderName], SeparatorFolder):
                    # This button can't be clicked.
                    folderButton.hide()
                    folderField.setStringJustify(kLeftJustify)
                    folderField.setForeColor(kColors.DniStatic)
                else:
                    folderButton.show()
                    folderField.setStringJustify(kRightJustify)
                    if ID == selectedFolder:
                        folderField.setForeColor(kColors.DniSelected)
                        longFolderField.setForeColor(kColors.DniSelected)
                    else:
                        folderField.setForeColor(kColors.DniSelectable)
                        longFolderField.setForeColor(kColors.DniSelectable)
                folderField.setStringW(folderName)
                longFolderField.setStringW(folderName)
                ID += 1
                if ID > kGUI.BKIFolderLineLast:
                    break
        # Set the up and down buttons, if needed.
        upbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKFolderUpLine))
        if self.BKFolderTopLine > 0:
            upbtn.show()
        else:
            upbtn.hide()
        dwnbtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKFolderDownLine))
        # Has the listbox been filled up?
        if ID > kGUI.BKIFolderLineLast:
            dwnbtn.show()
        else:
            dwnbtn.hide()

        # If there are more folder lines, fill them out to be blank and disable
        # their button fields.
        for tagID in range(ID, kGUI.BKIFolderLineLast + 1):
            folderField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(tagID))
            folderField.setForeColor(kColors.DniSelectable)
            folderField.setString(" ")
            buttonID = tagID - kGUI.BKIFolderLine01 + kGUI.BKIFolderLineBtn01
            folderButton = ptGUIControlButton(BigKI.dialog.getControlFromTag(buttonID))
            folderButton.hide()

    ## Do some extra processing on the content list.
    def BigKIProcessContentList(self, removeInboxStuff=False):

        # Start with nothing in the removeList (remove from current content list).
        removeList = []
        # If it's a player list.
        if self.BKFolderLineDict is self.BKPlayerFolderDict:
            ignores = ptVault().getIgnoreListFolder()
            # Make sure there are some players to process.
            if len(self.BKContentList) > 0:
                # If this is a ptPlayer.
                if isinstance(self.BKContentList[0], ptPlayer):
                    # Sort the list of Age players.
                    try:
                        self.BKContentList.sort(lambda a, b: cmp(a.getPlayerName().lower(), b.getPlayerName().lower()))
                    except:
                        PtDebugPrint(u"xKI.BigKIProcessContentList(): Unable to sort Age players, but don't break the list.", level=kErrorLevel)

                    for idx in range(len(self.BKContentList)):
                        player = self.BKContentList[idx]
                        if isinstance(player, ptPlayer):
                            if ignores.playerlistHasPlayer(player.getPlayerID()):
                                # Remove ignored player.
                                removeList.insert(0, idx)
                        else:
                            # Not a player, remove from the list.
                            removeList.insert(0, idx)
                else:
                    # Sort the list of players, online first.
                    self.BKContentList.sort(CMPplayerOnline)
                    # Remove all the unnamed players and ignored people.
                    for idx in range(len(self.BKContentList)):
                        ref = self.BKContentList[idx]
                        elem = ref.getChild()
                        if elem is not None:
                            if elem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                elem = elem.upcastToPlayerInfoNode()
                                if elem.playerGetName() == "":
                                    # Put them in reverse order in the removeList.
                                    removeList.insert(0, idx)
                                # Check if they are in the ignore list.
                                elif ignores.playerlistHasPlayer(elem.playerGetID()):
                                    # Get parent; in some folders the player has to be still visible.
                                    parent = ref.getParent()
                                    if parent:
                                        parent = parent.upcastToFolderNode()
                                    if parent is None:
                                        # Make sure this is not the IgnoreList.
                                        if parent.folderGetType() != PtVaultStandardNodes.kIgnoreListFolder:
                                            # Put in them in reverse order in the removeList.
                                            removeList.insert(0, idx)
                            else:
                                removeList.insert(0, idx)
                        else:
                            removeList.insert(0, idx)
        elif self.BKFolderListOrder[self.BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
            # Look for KI-Mail from non-Buddies if the player only wants KI-Mail from Buddies.
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
                        PtDebugPrint(u"xKI.BigKIProcessContentList(): Remove from inbox because it's from {}.".format(ref.getSaver().playerGetName()), level=kWarningLevel)
                        # Remove from the list.
                        removeList.insert(0, idx)
                        # Only remove from inbox if specified.
                        if removeInboxStuff:
                            PtDebugPrint(u"xKI.BigKIProcessContentList(): Really removed from inbox because it's from {}, this time.".format(ref.getSaver().playerGetName()), level=kWarningLevel)
                            # Remove from inbox (how will this work?).
                            element = ref.getChild()
                            inbox.removeNode(element)
        if removeList:
            PtDebugPrint(u"xKI.BigKIProcessContentList(): Removing {} contents from being displayed.".format(len(removeList)), level=kWarningLevel)
        for removeidx in removeList:
            del self.BKContentList[removeidx]

        if self.BKFolderListOrder[self.BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kInboxFolder):
            self.BKContentList = self.markerJoinRequests + self.BKContentList
            # Also add in the GlobalInbox stuff here.
            vault = ptVault()
            gInbox = vault.getGlobalInbox()
            if gInbox is not None:
                self.BKContentList = gInbox.getChildNodeRefList() + self.BKContentList
                self.BKContentList.sort(CMPNodeDate)

        removeList = []
        for contentidx in range(len(self.BKContentList)):
            content = self.BKContentList[contentidx]
            if isinstance(content, ptVaultNodeRef):
                element = content.getChild()
                if element is not None:
                    if element.getType() == PtVaultNodeTypes.kFolderNode or element.getType() == PtVaultNodeTypes.kChronicleNode:
                        removeList.insert(0, contentidx)
        for removeidx in removeList:
            del self.BKContentList[removeidx]

    ## Refresh the display of the selected content list.
    def BigKIRefreshContentListDisplay(self):

        if self.BKRightSideMode == kGUI.BKListMode:
            createField = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(kGUI.BKILMTitleCreateLine))
            createBtn = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(kGUI.BKIListModeCreateBtn))
            try:
                if self.BKFolderLineDict is self.BKPlayerFolderDict:
                    if self.BKFolderListOrder[self.BKFolderSelected] == xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder):
                        createField.setStringW(PtGetLocalizedString("KI.Player.CreateBuddyTitle"))
                        createBtn.show()
                    else:
                        createField.setString(" ")
                        createBtn.hide()
                else:
                    createField.setString(" ")
                    createBtn.hide()
            except IndexError:
                createField.setString(" ")
                createBtn.hide()
                if len(self.BKFolderListOrder) != 0:
                    PtDebugPrint(u"xKI.BigKIRefreshContentListDisplay(): Index error: self.BKFolderSelected = {} and list = {}.".format(self.BKFolderSelected, self.BKFolderListOrder), level=kWarningLevel)
                return
            ID = kGUI.BKILMOffsetLine01
            if len(self.BKContentList) != 0:
                if self.BKContentListTopLine >= len(self.BKContentList):
                    self.BKContentListTopLine = len(self.BKContentList) - 1
                for content in self.BKContentList[self.BKContentListTopLine:]:
                    if content is not None:
                        # Add the new line.
                        contentIconJ = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(ID + kGUI.BKILMIconJournalOffset))
                        contentIconAva = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(ID + kGUI.BKILMIconPersonOffset))
                        contentIconP = ptGUIControlDynamicText(KIListModeDialog.dialog.getControlFromTag(ID + kGUI.BKILMIconPictureOffset))
                        contentTitle = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(ID + kGUI.BKILMTitleOffset))
                        contentDate = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(ID + kGUI.BKILMDateOffset))
                        contentFrom = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(ID + kGUI.BKILMFromOffset))
                        if isinstance(content, ptPlayer):
                            contentIconAva.show()
                            contentIconJ.hide()
                            contentIconP.hide()
                            contentTitle.setForeColor(kColors.DniSelectable)
                            contentTitle.setString(xCensor.xCensor(content.getPlayerName(), self.censorLevel))
                            contentTitle.show()
                            contentDate.hide()
                            contentFrom.setForeColor(kColors.DniSelectable)
                            contentFrom.setFontSize(10)
                            contentFrom.setString(GetAgeName())
                            contentFrom.show()
                            # Find the button to enable it.
                            lmButton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((ID - 100) / 10) + kGUI.BKIListModeCreateBtn))
                            lmButton.show()
                            ID += 10
                            if ID > kGUI.BKILMOffsetLineLast:
                                break
                        else:
                            element = content.getChild()
                            if element is not None:
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
                                        dynMap = contentIconP.getMap(0)
                                        image = element.imageGetImage()
                                        dynMap.clearToColor(ptColor(.1, .1, .1, .1))
                                        if image is not None:
                                            dynMap.drawImage(kGUI.BKIImageStartX, kGUI.BKIImageStartY, image, 0)
                                        dynMap.flush()
                                    contentIconP.show()
                                elif element.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                    element = element.upcastToPlayerInfoNode()
                                    contentIconAva.show()
                                    contentIconJ.hide()
                                    contentIconP.hide()
                                elif element.getType() == PtVaultNodeTypes.kMarkerGameNode:
                                    element = element.upcastToMarkerGameNode()
                                    # No icon for Marker Game.
                                    contentIconAva.hide()
                                    contentIconJ.hide()
                                    contentIconP.hide()
                                elif element.getType() == PtVaultNodeTypes.kFolderNode:
                                    continue
                                else:
                                    contentIconAva.hide()
                                    contentIconJ.hide()
                                    contentIconP.hide()
                                if isinstance(element, ptVaultPlayerInfoNode):
                                    # If it's a player, use the title for the player name.
                                    contentTitle.setForeColor(kColors.DniSelectable)
                                    contentTitle.setString(xCensor.xCensor(element.playerGetName(), self.censorLevel))
                                    contentTitle.show()
                                    contentDate.hide()
                                    contentFrom.setForeColor(kColors.DniSelectable)
                                    contentFrom.setFontSize(10)
                                    if element.playerIsOnline():
                                        contentFrom.setString(FilterAgeName(element.playerGetAgeInstanceName()))
                                    else:
                                        contentFrom.setString("  ")
                                    contentFrom.show()
                                else:
                                    # Otherwise it's an image or a text note.
                                    if content.getSaverID() == 0:
                                        # Must be from the DRC.
                                        contentTitle.setForeColor(kColors.DniStatic)
                                        contentDate.setForeColor(kColors.DniStatic)
                                    else:
                                        contentTitle.setForeColor(kColors.DniSelectable)
                                        contentDate.setForeColor(kColors.DniSelectable)

                                    if isinstance(element, ptVaultImageNode):
                                        contentTitle.setString(xCensor.xCensor(element.imageGetTitle(), self.censorLevel))
                                    elif isinstance(element, ptVaultTextNoteNode):
                                        contentTitle.setString(xCensor.xCensor(element.noteGetTitle(), self.censorLevel))
                                    elif isinstance(element, ptVaultMarkerGameNode):
                                        contentTitle.setString(xCensor.xCensor(element.getGameName(), self.censorLevel))
                                    else:
                                        # Probably still downloading because of lag.
                                        contentTitle.setString("--[Downloading]--")
                                        contentTitle.setForeColor(kColors.DniYellow)
                                        PtDebugPrint(u"xKI.BigKIRefreshContentListDisplay(): Unknown data type in content list: type = {}.".format(element.getType()), level=kErrorLevel)
                                    contentTitle.show()
                                    try:
                                        tupTime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
                                        curTime = time.strftime(PtGetLocalizedString("Global.Formats.Date"), tupTime)
                                    except:
                                        curTime = ""
                                    contentDate.setString(curTime)
                                    contentDate.show()
                                    sender = content.getSaver()
                                    # See if the saver was the player.
                                    localPlayer = PtGetLocalPlayer()
                                    if sender is not None and localPlayer.getPlayerID() != sender.playerGetID():
                                        if content.getSaverID() == 0:
                                            # Must be from the DRC.
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
                                            # Must be from the DRC.
                                            contentFrom.setString("DRC")
                                            contentFrom.show()
                                        else:
                                            contentFrom.setString("  ")
                                            contentFrom.hide()
                                # Find the button to enable it.
                                lmButton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((ID - 100) / 10) + kGUI.BKIListModeCreateBtn))
                                lmButton.show()
                                ID += 10
                                if ID > kGUI.BKILMOffsetLineLast:
                                    break
                            else:
                                PtDebugPrint(u"xKI.BigKIRefreshContentListDisplay: No element inside the content.", level=kErrorLevel)
                    else:
                        PtDebugPrint(u"xKI.BigKIRefreshContentListDisplay: No content, even though the folder said there was.", level=kErrorLevel)
            # Set the up and down buttons if needed.
            upBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMUpButton))
            if self.BKContentListTopLine > 0:
                upBtn.show()
            else:
                upBtn.hide()
            dwnBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKLMDownButton))
            # Has the ListBox been filled up?
            if ID > kGUI.BKILMOffsetLineLast:
                dwnBtn.show()
            else:
                dwnBtn.hide()
            # If there are more content lines, fill them out to be blank
            # and disable the button fields.
            for tagID in range(ID,kGUI.BKILMOffsetLineLast + 10, 10):
                iconPic = ptGUIControlDynamicText(KIListModeDialog.dialog.getControlFromTag(tagID + kGUI.BKILMIconPictureOffset))
                iconPic.hide()
                iconJrn = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(tagID + kGUI.BKILMIconJournalOffset))
                iconJrn.hide()
                iconAva = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(tagID + kGUI.BKILMIconPersonOffset))
                iconAva.hide()
                titleField = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagID + kGUI.BKILMTitleOffset))
                titleField.hide()
                dateField = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagID + kGUI.BKILMDateOffset))
                dateField.hide()
                fromField = ptGUIControlTextBox(KIListModeDialog.dialog.getControlFromTag(tagID + kGUI.BKILMFromOffset))
                fromField.hide()
                # Find the button to disable it.
                lmButton = ptGUIControlButton(KIListModeDialog.dialog.getControlFromTag(((tagID - 100) / 10) + kGUI.BKIListModeCreateBtn))
                lmButton.hide()

    #~~~~~~~~~~~~~~~~~~~~~~~#
    # BigKI Content Display #
    #~~~~~~~~~~~~~~~~~~~~~~~#

    ## Display a text Journal entry in the KI.
    def BigKIDisplayJournalEntry(self):

        jrnAgeName = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNAgeName))
        jrnAgeName.hide()
        jrnDate = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNDate))
        jrnDate.hide()
        jrnTitle = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNTitle))
        jrnTitle.hide()
        jrnNote = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNNote))
        jrnNote.hide()
        jrnNote.setBufferLimit(kLimits.JournalTextSize)
        jrnDeleteBtn = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNDeleteButton))
        jrnDeleteBtn.hide()
        jrnTitleBtn = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKIJRNTitleButton))
        if self.BKCurrentContent is None:
            PtDebugPrint(u"xKI.BigKIDisplayJournalEntry(): self.BKCurrentContent is None.", level=kErrorLevel)
            return
        if self.IsContentMutable(self.BKCurrentContent):
            jrnDeleteBtn.show()
            jrnNote.unlock()
            if not self.BKInEditMode or self.BKEditField != kGUI.BKEditFieldJRNTitle:
                jrnTitleBtn.show()
        else:
            jrnNote.lock()
            jrnTitleBtn.hide()
        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint(u"xKI.BigKIDisplayJournalEntry(): Element is None.", level=kErrorLevel)
            return
        dataType = element.getType()
        if dataType != PtVaultNodeTypes.kTextNoteNode:
            PtDebugPrint(u"xKI.BigKIDisplayJournalEntry(): Wrong element type {}.".format(dataType), level=kErrorLevel)
            return
        element = element.upcastToTextNoteNode()
        # Display the content on the screen.
        jrnAgeName.setString(FilterAgeName(xCensor.xCensor(element.getCreateAgeName(), self.censorLevel)))
        jrnAgeName.show()
        tupTime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
        curTime = time.strftime(PtGetLocalizedString("Global.Formats.Date"), tupTime)
        jrnDate.setString(curTime)
        jrnDate.show()
        if not self.BKInEditMode or self.BKEditField != kGUI.BKEditFieldJRNTitle:
            jrnTitle.setString(xCensor.xCensor(element.noteGetTitle(), self.censorLevel))
            jrnTitle.show()
        if not self.BKInEditMode or self.BKEditField != kGUI.BKEditFieldJRNNote:
            encoded = buffer(xCensor.xCensor(element.noteGetText(), self.censorLevel))
            jrnNote.setEncodedBuffer(encoded)
            jrnNote.show()
        self.BigKISetSeen(self.BKCurrentContent)
        # If it came from someone else, add them to the SendTo field.
        self.CheckContentForSender(self.BKCurrentContent)

    ## Create and display a new note in the Journal.
    def BigKICreateJournalNote(self):

        PtDebugPrint(u"xKI.BigKICreateJournalNote(): Create text note message.", level=kDebugDumpLevel)
        # If there is no folder list, then make one.
        if not self.BKFolderListOrder:
            self.BigKIRefreshFolderList()
        try:
            journal = self.BKJournalFolderDict[self.GetAgeInstanceName()]
            if journal is not None:
                # Make sure that the age folder is selected.
                self.BKFolderTopLine = self.BKJournalFolderTopLine = 0  # Scroll back to the top.
                self.BKFolderSelected = self.BKJournalFolderSelected = self.BKJournalListOrder.index(self.GetAgeInstanceName())

                # Create the note.
                note = ptVaultTextNoteNode(0)
                note.setTextW(PtGetLocalizedString("KI.Journal.InitialMessage"))
                note.setTitleW(PtGetLocalizedString("KI.Journal.InitialTitle"))

                self.BKCurrentContent = journal.addNode(note)
                return self.BKCurrentContent
            else:
                PtDebugPrint(u"xKI.BigKICreateJournalNote(): Journal not ready.", level=kErrorLevel)
                return None
        except KeyError:
            PtDebugPrint(u"xKI.BigKICreateJournalNote(): Could not find journal for this Age: {}.".format(self.GetAgeInstanceName()), level=kErrorLevel)

    ## Display a KI Picture in the KI.
    def BigKIDisplayPicture(self):

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
        if self.BKCurrentContent is None:
            PtDebugPrint(u"xKI.BigKIDisplayPicture(): self.BKCurrentContent is None.", level=kErrorLevel)
            return
        if self.IsContentMutable(self.BKCurrentContent):
            picDeleteBtn.show()
            if not self.BKInEditMode or self.BKEditField != kGUI.BKEditFieldPICTitle:
                picTitleBtn.show()
        else:
            picTitleBtn.hide()
        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint(u"xKI.BigKIDisplayPicture(): Element is None.", level=kErrorLevel)
            return
        dataType = element.getType()
        if dataType != PtVaultNodeTypes.kImageNode:
            PtDebugPrint(u"xKI.BigKIDisplayPicture(): Wrong element type {}.".format(dataType), level=kErrorLevel)
            return
        element = element.upcastToImageNode()
        # Display the content on the screen.
        picAgeName.setString(FilterAgeName(xCensor.xCensor(element.getCreateAgeName(), self.censorLevel)))
        picAgeName.show()
        tupTime = time.gmtime(PtGMTtoDniTime(element.getModifyTime()))
        curTime = time.strftime(PtGetLocalizedString("Global.Formats.Date"), tupTime)
        picDate.setString(curTime)
        picDate.show()
        if not self.BKInEditMode or self.BKEditField != kGUI.BKEditFieldPICTitle:
            picTitle.setString(xCensor.xCensor(element.imageGetTitle(), self.censorLevel))
            picTitle.show()
        if picImage.getNumMaps() > 0:
            dynMap = picImage.getMap(0)
            image = element.imageGetImage()
            dynMap.clearToColor(ptColor(.1, .1, .1, .3))
            if image is not None:
                dynMap.drawImage(kGUI.BKIImageStartX, kGUI.BKIImageStartY, image, 0)
            else:
                dynMap.fillRect(kGUI.BKIImageStartX, kGUI.BKIImageStartY, kGUI.BKIImageStartX + 800, kGUI.BKIImageStartY + 600, ptColor(.2, .2, .2, .1))
            dynMap.flush()
        picImage.show()
        self.BigKISetSeen(self.BKCurrentContent)
        # If it came from someone else, add them to the SendTo field.
        self.CheckContentForSender(self.BKCurrentContent)

    ## Create and display a new KI Picture in the Journal.
    def BigKICreateJournalImage(self, image, useScreenShot=False):

        PtDebugPrint(u"xKI.BigKICreateJournalImage(): Create a KI Picture from {}.".format(image), level=kDebugDumpLevel)
        # If there is no folder list, then make one.
        if not self.BKFolderListOrder:
            self.BigKIRefreshFolderList()
        try:
            journal = self.BKJournalFolderDict[self.GetAgeInstanceName()]
            if journal is not None:
                # Make sure that the age folder is selected.
                self.BKFolderTopLine = self.BKJournalFolderTopLine = 0  # Scroll back to the top.
                self.BKFolderSelected = self.BKJournalFolderSelected = self.BKJournalListOrder.index(self.GetAgeInstanceName())
                # Create the image entry.
                imgElem = ptVaultImageNode(0)
                if useScreenShot:
                    imgElem.setImageFromScrShot()
                else:
                    imgElem.imageSetImage(image)
                imgElem.setTitleW(PtGetLocalizedString("KI.Image.InitialTitle"))
                self.BKCurrentContent = journal.addNode(imgElem)
                return self.BKCurrentContent
            else:
                PtDebugPrint(u"xKI.BigKICreateJournalImage(): Journal not ready.", level=kErrorLevel)
                return None
        except KeyError:
            PtDebugPrint(u"xKI.BigKICreateJournalImage(): Could not find journal for this Age: {}.".format(self.GetAgeInstanceName()), level=kErrorLevel)

    ## Display a player entry.
    def BigKIDisplayPlayerEntry(self):

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
        # Is the player asking for a player ID number?
        if self.BKGettingPlayerID:
            plyName.setStringW(PtGetLocalizedString("KI.Player.EnterID"))
            plyName.show()
            plyIDedit.setString("")
            plyIDedit.show()
            plyIDedit.focus()
            KIPlayerExpanded.dialog.setFocus(plyIDedit.getKey())
            return
        if self.BKCurrentContent is None:
            PtDebugPrint(u"xKI.BigKIDisplayPlayerEntry(): self.BKCurrentContent is None.", level=kErrorLevel)
            return
        if isinstance(self.BKCurrentContent, ptPlayer):
            # Display the content on the screen.
            plyName.setString(xCensor.xCensor(self.BKCurrentContent.getPlayerName(), self.censorLevel))
            plyName.show()
            IDText = "{:08d}".format(self.BKCurrentContent.getPlayerID())
            plyID.setString(IDText)
            plyID.show()
            plyDetail.setStringW(PtGetLocalizedString("KI.Player.InAge", [GetAgeName()]))
            plyDetail.show()
            sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
            self.BKPlayerSelected = self.BKCurrentContent
            sendToField.setString(self.BKCurrentContent.getPlayerName())
            return
        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint(u"xKI.BigKIDisplayPlayerEntry(): Element is None.", level=kErrorLevel)
            return
        dataType = element.getType()
        if dataType != PtVaultNodeTypes.kPlayerInfoNode:
            PtDebugPrint(u"xKI.BigKIDisplayPlayerEntry(): Wrong element type {}.".format(dataType), level=kErrorLevel)
            return
        element = element.upcastToPlayerInfoNode()
        # Display the content on the screen.
        plyName.setString(xCensor.xCensor(element.playerGetName(), self.censorLevel))
        plyName.show()
        IDText = "{:08d}".format(element.playerGetID())
        plyID.setString(IDText)
        plyID.show()
        if element.playerIsOnline():
            if element.playerGetAgeInstanceName() == "Cleft":
                plyDetail.setStringW(PtGetLocalizedString("KI.Player.InCleft"))
            elif element.playerGetAgeInstanceName() == "AvatarCustomization":
                plyDetail.setStringW(PtGetLocalizedString("KI.Player.InCloset"))
            else:
                plyDetail.setStringW(PtGetLocalizedString("KI.Player.InAge", [FilterAgeName(element.playerGetAgeInstanceName())]))
        else:
            plyDetail.setStringW(PtGetLocalizedString("KI.Player.Offline"))
        plyDetail.show()
        # Determine if this player can be removed from this folder.
        folder = self.BKCurrentContent.getParent()
        if folder:
            folder = folder.upcastToFolderNode()
        if folder and self.IsFolderContentMutable(folder):
            plyDeleteBtn.show()
        sendToField = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(kGUI.BKIPlayerLine))
        self.BKPlayerSelected = self.BKCurrentContent
        sendToField.setString(element.playerGetName())

    ## Save after a player was edited.
    def BigKICheckSavePlayer(self):

        if self.BKGettingPlayerID:
            # Create and save a player element into Buddies.
            self.BKGettingPlayerID = False
            plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYPlayerIDEditBox))
            if not plyIDedit.wasEscaped():
                ID = self.chatMgr.commandsProcessor.GetPID(plyIDedit.getString())
                if ID:
                    localPlayer = PtGetLocalPlayer()
                    if ID != localPlayer.getPlayerID():
                        vault = ptVault()
                        buddies = vault.getBuddyListFolder()
                        if buddies is not None:
                            if buddies.playerlistHasPlayer(ID):
                                plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDetail))
                                plyDetail.setStringW(PtGetLocalizedString("KI.Player.AlreadyAdded"))
                                plyDetail.show()
                                self.BKGettingPlayerID = True
                            else:
                                buddies.playerlistAddPlayer(ID)
                                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Added"))
                        if not self.BKGettingPlayerID:
                            self.ChangeBigKIMode(kGUI.BKListMode)
                    else:
                        plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDetail))
                        plyDetail.setStringW(PtGetLocalizedString("KI.Player.NotYourself"))
                        plyDetail.show()
                        self.BKGettingPlayerID = True
                else:
                    plyDetail = ptGUIControlTextBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYDetail))
                    plyDetail.setStringW(PtGetLocalizedString("KI.Player.NumberOnly"))
                    plyDetail.show()
                    self.BKGettingPlayerID = True
            else:
                # Nothing here, just go back to list mode.
                self.ChangeBigKIMode(kGUI.BKListMode)

    ## Prepares the display of a marker game, as it may be loading.
    def BigKIDisplayMarkerGame(self):

        # Make sure that the player can view this game.
        if self.gKIMarkerLevel < kKIMarkerNormalLevel:
            self.BigKIDisplayMarkerGameMessage(PtGetLocalizedString("KI.MarkerGame.pendingActionUpgradeKI"))
            return

        # Save some typing.
        mgr = self.markerGameManager
        getControl = KIMarkerFolderExpanded.dialog.getControlFromTag

        # Initialize the markerGameDisplay to the currently selected game.
        # But first, ensure that the player meets all the necessary criteria.
        if self.BKCurrentContent is None:
            PtDebugPrint(u"xKI.BigKIDisplayMarkerGame(): Could not find the current selected content selected.", level=kErrorLevel)
            return
        element = self.BKCurrentContent.getChild()
        if element is None:
            PtDebugPrint(u"xKI.BigKIDisplayMarkerGame(): Could not find the current content's child node.", level=kErrorLevel)
            return
        dataType = element.getType()
        if dataType != PtVaultNodeTypes.kMarkerGameNode:
            PtDebugPrint(u"xKI.BigKIDisplayMarkerGame(): Cannot process this node, wrong data type: {}.".format(element.getType()), level=kErrorLevel)
            return
        element = element.upcastToMarkerGameNode()
        PtDebugPrint(u"xKI.BigKIDisplayMarkerGame(): Starting Marker Game KI Display Manager, loading game: {}.".format(element.getGameName()), level=kDebugDumpLevel)

        ## This was previously BigKIFinishDisplayMarkerGame()
        questGameFinished = False

        # A game is in progress, restrict access.
        if mgr.AmIPlaying(element):
            self.MFdialogMode = kGames.MFPlaying

        # Are we editing this game? If so, how?
        elif mgr.IsActive(element) and mgr.edit_mode:
            if mgr.selected_marker_id != -1:
                self.MFdialogMode = kGames.MFEditingMarker
            else:
                self.MFdialogMode = kGames.MFEditing

        # Whatever.
        else:
            self.MFdialogMode = kGames.MFOverview

        # Refresh miniKI.
        self.RefreshMiniKIMarkerDisplay()

        self.SetBigKIToButtons()
        
        # Hide the invite buttons controls.
        ptGUIControlButton(getControl(kGUI.MarkerFolderInvitePlayer)).hide()
        mtbInvitePlayer = ptGUIControlTextBox(getControl(kGUI.MarkerFolderInvitePlayerTB))
        mtbInvitePlayer.setForeColor(kColors.Clear)
        mtbInvitePlayer.setString(" ")

        mbtnEditStart = ptGUIControlButton(getControl(kGUI.MarkerFolderEditStartGame))
        mbtnPlayEnd = ptGUIControlButton(getControl(kGUI.MarkerFolderPlayEndGame))
        mrkfldOwner = ptGUIControlTextBox(getControl(kGUI.MarkerFolderOwner))
        mtbEditStart = ptGUIControlTextBox(getControl(kGUI.MarkerFolderEditStartGameTB))
        mtbPlayEnd = ptGUIControlTextBox(getControl(kGUI.MarkerFolderPlayEndGameTB))
        mrkfldStatus = ptGUIControlTextBox(getControl(kGUI.MarkerFolderStatus))
        mrkfldTitle = ptGUIControlTextBox(getControl(kGUI.MarkerFolderTitleText))
        mrkfldTitleBtn = ptGUIControlButton(getControl(kGUI.MarkerFolderTitleBtn))
        mbtnDelete = ptGUIControlButton(getControl(kGUI.MarkerFolderDeleteBtn))
        mbtnGameTimePullD = ptGUIControlButton(getControl(kGUI.MarkerFolderTimePullDownBtn))
        mtbGameType = ptGUIControlTextBox(getControl(kGUI.MarkerFolderGameTypeTB))
        mbtnGameTypePullD = ptGUIControlButton(getControl(kGUI.MarkerFolderTypePullDownBtn))
        mbtnGameTypePullD.hide()
        mbtnGameTimeArrow = ptGUIControlButton(getControl(kGUI.MarkerFolderTimeArrow))
        mbtnGameTypeArrow = ptGUIControlButton(getControl(kGUI.MarkerFolderTypeArrow))
        mbtnGameTypeArrow.hide()
        mtbGameTime = ptGUIControlTextBox(getControl(kGUI.MarkerFolderGameTimeTB))
        mtbGameTimeTitle = ptGUIControlTextBox(getControl(kGUI.MarkerFolderGameTimeTitleTB))
        mtbGameTimeTitle.setStringW(PtGetLocalizedString("KI.MarkerGame.Time"))
        mlbMarkerList = ptGUIControlListBox(getControl(kGUI.MarkerFolderMarkListbox))
        mlbMarkerTextTB = ptGUIControlTextBox(getControl(kGUI.MarkerFolderMarkerTextTB))
        mbtnMarkerText = ptGUIControlTextBox(getControl(kGUI.MarkerFolderMarkerTextBtn))

        mbtnToran = ptGUIControlButton(getControl(kGUI.MarkerFolderToranIcon))
        mbtnToran.disable()
        mbtnHSpan = ptGUIControlButton(getControl(kGUI.MarkerFolderHSpanIcon))
        mbtnHSpan.disable()
        mbtnVSpan = ptGUIControlButton(getControl(kGUI.MarkerFolderVSpanIcon))
        mbtnVSpan.disable()
        mtbToran = ptGUIControlTextBox(getControl(kGUI.MarkerFolderToranTB))
        mtbHSPan = ptGUIControlTextBox(getControl(kGUI.MarkerFolderHSpanTB))
        mtbVSpan = ptGUIControlTextBox(getControl(kGUI.MarkerFolderVSpanTB))

        ## FIXME: non quest game types
        mtbGameType.setStringW(PtGetLocalizedString("KI.MarkerGame.NameQuest").title())

        mbtnEditStart.show()
        mbtnPlayEnd.show()

        # Is the player merely looking at a Marker Game?
        if self.MFdialogMode == kGames.MFOverview:
            mrkfldTitleBtn.disable()
            if self.IsContentMutable(self.BKCurrentContent):
                mbtnDelete.show()
            else:
                mbtnDelete.hide()
            mbtnGameTimePullD.hide()
            mbtnGameTimeArrow.hide()
            if element.getCreatorNodeID() == PtGetLocalClientID():
                mbtnEditStart.show()
                mtbEditStart.setForeColor(kColors.DniShowBtn)
            else:
                mbtnEditStart.hide()
                mtbEditStart.setForeColor(kColors.DniGhostBtn)
            mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.EditButton"))
            mtbEditStart.show()
            mbtnPlayEnd.show()
            mtbPlayEnd.setForeColor(kColors.DniShowBtn)
            mtbPlayEnd.setString(PtGetLocalizedString("KI.MarkerGame.PlayButton"))
            mtbPlayEnd.show()
            mlbMarkerList.hide()
            self.BigKIMarkerListScrollVis(False)
            mlbMarkerTextTB.hide()
            mbtnToran.hide()
            mbtnHSpan.hide()
            mbtnVSpan.hide()
            mtbToran.hide()
            mtbHSPan.hide()
            mtbVSpan.hide()
            mbtnMarkerText.disable()
        # Is the player editing a Marker Game?
        elif self.MFdialogMode == kGames.MFEditing or self.MFdialogMode == kGames.MFEditingMarker:
            mrkfldTitleBtn.enable()
            mbtnDelete.hide()
            mbtnGameTimePullD.hide()
            mbtnGameTimeArrow.hide()
            mbtnEditStart.show()
            mtbEditStart.setForeColor(kColors.DniShowBtn)
            mbtnPlayEnd.show()
            mtbPlayEnd.setForeColor(kColors.DniShowBtn)
            # Is the player editing the entire game?
            if self.MFdialogMode == kGames.MFEditing:
                mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.DoneEditButton"))
                mtbEditStart.show()
                mtbPlayEnd.setStringW(PtGetLocalizedString("KI.MarkerGame.AddMarkerButton"))
                mtbPlayEnd.show()
                mlbMarkerList.clearAllElements()
                mlbMarkerList.show()

                # Add the Markers to the list.
                mlbMarkerList.lock()
                for idx, age, pos, desc in mgr.markers:
                    coord = ptDniCoordinates()
                    coord.fromPoint(pos)
                    torans = coord.getTorans()
                    hSpans = coord.getHSpans()
                    vSpans = coord.getVSpans()
                    mlbMarkerList.addStringW(u"[{}:{},{},{}] {}".format(FilterAgeName(age), torans, hSpans, vSpans, xCensor.xCensor(desc, self.censorLevel)))
                mlbMarkerList.unlock()

                # Refresh the scroll position
                self.BigKIMarkerListScrollVis(True)
                mlbMarkerList.setScrollPos(self.MFScrollPos)

                mlbMarkerTextTB.hide()
                mbtnToran.hide()
                mbtnHSpan.hide()
                mbtnVSpan.hide()
                mtbToran.hide()
                mtbHSPan.hide()
                mtbVSpan.hide()
                mbtnMarkerText.disable()
            # Or just editing one of the Markers?
            else:
                selectedMarker = mgr.selected_marker
                if selectedMarker is not None:
                    idx, age, pos, desc = selectedMarker

                    # Must be editing a Marker.
                    mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.MarkerListButton"))
                    mtbEditStart.show()
                    mtbPlayEnd.setStringW(PtGetLocalizedString("KI.MarkerGame.RemoveMarkerButton"))
                    mtbPlayEnd.show()
                    mlbMarkerList.hide()
                    self.BigKIMarkerListScrollVis(False)
                    mlbMarkerTextTB.show()
                    # don't censor here... we don't want censored stuff saved to the vault
                    mlbMarkerTextTB.setStringW(desc)
                    mbtnToran.show()
                    mbtnHSpan.show()
                    mbtnVSpan.show()
                    mtbToran.show()
                    mtbHSPan.show()
                    mtbVSpan.show()

                    # Get the selected Marker's coordinates.
                    coord = ptDniCoordinates()
                    coord.fromPoint(pos)

                    mtbToran.setString(str(coord.getTorans()))
                    mtbHSPan.setString(str(coord.getHSpans()))
                    mtbVSpan.setString(str(coord.getVSpans()))
                    mbtnMarkerText.show()
                    mbtnMarkerText.enable()
                else:
                    # Error...
                    PtDebugPrint(u"xKI.BigKIFinishDisplayMarkerGame(): Could not find selected marker.", level=kErrorLevel)
                    mtbEditStart.setStringW(PtGetLocalizedString("KI.MarkerGame.GoBackButton"))
                    mtbEditStart.show()
                    mtbPlayEnd.setString(" ")
                    mtbPlayEnd.show()
                    mlbMarkerList.hide()
                    mlbMarkerTextTB.show()
                    mlbMarkerTextTB.setString("?Unknown Marker?")
                    mbtnToran.hide()
                    mbtnHSpan.hide()
                    mbtnVSpan.hide()
                    mtbToran.hide()
                    mtbHSPan.hide()
                    mtbVSpan.hide()
        # Is the player currently playing a Marker Game?
        elif self.MFdialogMode == kGames.MFPlaying:
            mrkfldTitleBtn.disable()
            mbtnDelete.hide()
            mbtnGameTimePullD.hide()
            mbtnGameTimeArrow.hide()
            mbtnToran.hide()
            mbtnHSpan.hide()
            mbtnVSpan.hide()
            mtbToran.hide()
            mtbHSPan.hide()
            mtbVSpan.hide()
            mbtnMarkerText.disable()
            mbtnEditStart.show()
            mtbEditStart.setForeColor(kColors.DniShowBtn)
            mtbEditStart.setString(PtGetLocalizedString("KI.MarkerGame.StopPlayingButton"))
            mtbEditStart.show()
            mbtnPlayEnd.show()
            mtbPlayEnd.setForeColor(kColors.DniShowBtn)
            mtbPlayEnd.setString(PtGetLocalizedString("KI.MarkerGame.ResetGameButton"))
            mtbPlayEnd.show()
            mlbMarkerList.clearAllElements()
            mlbMarkerList.show()
            self.BigKIMarkerListScrollVis(True)

            # Assume that the game is finished, unless an unseen Marker is still left.
            questGameFinished = True

            # Add the Markers into the list.
            for idx, age, pos, desc in mgr.markers:
                if mgr.IsMarkerCaptured(idx):
                    coord = ptDniCoordinates()
                    coord.fromPoint(pos)
                    torans = coord.getTorans()
                    hSpans = coord.getHSpans()
                    vSpans = coord.getVSpans()
                    mlbMarkerList.addStringW(u"[{}:{},{},{}] {}".format(FilterAgeName(age), torans, hSpans, vSpans, xCensor.xCensor(desc, self.censorLevel)))
                else:
                    questGameFinished = False
            mlbMarkerTextTB.hide()

        # Refresh the text of the buttons (color changed).
        mtbEditStart.refresh()
        mtbPlayEnd.refresh()
        # Display the content on the screen.
        mrkfldTitle.setStringW(xCensor.xCensor(element.getGameName(), self.censorLevel))
        mrkfldTitle.show()
        # Enable the editable Title.
        mrkfldTitleBtn.show()
        mrkfldTitleBtn.enable()

        count = mgr.marker_total
        if self.MFdialogMode == kGames.MFEditing or self.MFdialogMode == kGames.MFEditingMarker:
            if count == 0:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusNoMarkers")
            elif count == 1:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusOneMarker")
            else:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusNMarkers", [str(count)])
        else:
            if questGameFinished:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusAllFound")
            else:
                statusLine = PtGetLocalizedString("KI.MarkerGame.StatusNotAllFound")
        mrkfldStatus.setStringW(statusLine)
        mrkfldStatus.show()

        creatorID = element.getCreatorNodeID()
        tempNode = ptVaultPlayerInfoNode()
        tempNode.playerSetID(creatorID)

        try:
            vault = ptVault()
            creatorName = vault.findNode(tempNode).upcastToPlayerInfoNode().playerGetName()
        except:
            creatorName = ""

        mrkfldOwner.setStringW(PtGetLocalizedString("KI.MarkerGame.OwnerTitle") + U" {} [ID:{:08d}]".format(creatorName, creatorID))
        mrkfldOwner.show()
        # TODO: time limit
        mtbGameTime.hide()
        mtbGameTimeTitle.hide()

    def BigKIDisplayMarkerGameMessage(self, msg):
        """Displays some message in the Marker Folder subdialog"""

        # Save some typing.
        getControl = KIMarkerFolderExpanded.dialog.getControlFromTag

        # Disable all controls until we need them.
        mrkfldTitle = ptGUIControlTextBox(getControl(kGUI.MarkerFolderTitleText))
        mrkfldTitle.hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderStatus)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderOwner)).hide()
        # Hide the scroll buttons for the Marker list; the scroll control will turn them back on.
        self.BigKIMarkerListScrollVis(False)

        ptGUIControlButton(getControl(kGUI.MarkerFolderInvitePlayer)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderEditStartGame)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderPlayEndGame)).hide()

        ptGUIControlTextBox(getControl(kGUI.MarkerFolderInvitePlayerTB)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderEditStartGameTB)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderPlayEndGameTB)).hide()

        ptGUIControlButton(getControl(kGUI.MarkerFolderTitleBtn)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderDeleteBtn)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderTimePullDownBtn)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderTypePullDownBtn)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderTypePullDownBtn)).disable()
        ptGUIControlButton(getControl(kGUI.MarkerFolderTimeArrow)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderTypeArrow)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderGameTimeTB)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderGameTimeTitleTB)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderGameTypeTB)).hide()
        ptGUIControlListBox(getControl(kGUI.MarkerFolderMarkListbox)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderMarkerTextTB)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderMarkerTextBtn)).hide()
        ptGUIControlButton(getControl(kGUI.MarkerFolderToranIcon)).disable()
        ptGUIControlButton(getControl(kGUI.MarkerFolderHSpanIcon)).disable()
        ptGUIControlButton(getControl(kGUI.MarkerFolderVSpanIcon)).disable()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderToranTB)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderHSpanTB)).hide()
        ptGUIControlTextBox(getControl(kGUI.MarkerFolderVSpanTB)).hide()

        # Show the status.
        mrkfldTitle.setStringW(msg)
        mrkfldTitle.show()
        mrkfldTitle.refresh()

    def BigKIMarkerListScrollVis(self, visible=True):
        if visible:
            ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerListUpBtn)).show()
            ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerListDownBtn)).show()
        else:
            ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerListUpBtn)).hide()
            ptGUIControlButton(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerListDownBtn)).hide()

    ## Show the selected configuration screen.
    def ShowSelectedConfig(self):

        if self.BKConfigListOrder[self.BKFolderSelected] == PtGetLocalizedString("KI.Config.Settings"):
            self.ChangeBigKIMode(kGUI.BKKIExpanded)
        elif self.BKConfigListOrder[self.BKFolderSelected] == PtGetLocalizedString("KI.Config.Volume"):
            self.ChangeBigKIMode(kGUI.BKVolumeExpanded)
        else:
            # Is the dialog hidden?
            if self.BKRightSideMode != kGUI.BKAgeOwnerExpanded:
                self.ChangeBigKIMode(kGUI.BKAgeOwnerExpanded)
            # Otherwise, refresh it.
            else:
                self.RefreshAgeOwnerSettings()
                self.BigKIOnlySelectedToButtons()

    ## Enter edit mode for a particular field.
    def BigKIEnterEditMode(self, whichField):

        # Can't be in chatting mode.
        self.chatMgr.ToggleChatMode(0)
        # If the player was already in edit mode, save the values before re-entering.
        if self.BKInEditMode:
            self.BigKISaveEdit()
        if whichField == kGUI.BKEditFieldJRNTitle:
            textBox = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichField][kGUI.BKEditIDtextbox]))
            button = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichField][kGUI.BKEditIDbutton]))
            editBox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichField][kGUI.BKEditIDeditbox]))
        elif whichField == kGUI.BKEditFieldPICTitle:
            textBox = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichField][kGUI.BKEditIDtextbox]))
            button = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichField][kGUI.BKEditIDbutton]))
            editBox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[whichField][kGUI.BKEditIDeditbox]))
            editBox.setStringSize(56)
        else:
            textBox = None
            button = None
            editBox = None
        # Make sure it is a valid field to edit.
        if textBox is not None:
            if self.BKCurrentContent is not None:
                edElement = self.BKCurrentContent.getChild()
            else:
                edElement = None
            if edElement is not None:
                self.BKInEditMode = True
                self.BKEditContent = self.BKCurrentContent
                self.BKEditField = whichField
                # Hide the TextBox and the button.
                textBox.hide()
                button.hide()
                # Set the edit box and display it.
                if self.BKEditField == kGUI.BKEditFieldJRNTitle:
                    edElement = edElement.upcastToTextNoteNode()
                    editBox.setString(xCensor.xCensor(edElement.noteGetTitle(), self.censorLevel))
                    KIJournalExpanded.dialog.setFocus(editBox.getKey())
                elif self.BKEditField == kGUI.BKEditFieldPICTitle:
                    edElement = edElement.upcastToImageNode()
                    editBox.setString(xCensor.xCensor(edElement.imageGetTitle(), self.censorLevel))
                    KIPictureExpanded.dialog.setFocus(editBox.getKey())
                else:
                    editBox.setString("")
                editBox.end()
                editBox.show()
                editBox.focus()
                if whichField == kGUI.BKEditFieldJRNTitle or whichField == kGUI.BKEditFieldJRNNote:
                    KIJournalExpanded.dialog.refreshAllControls()
                elif whichField == kGUI.BKEditFieldPICTitle:
                    KIPictureExpanded.dialog.refreshAllControls()
            else:
                PtDebugPrint(u"xKI.BigKIEnterEditMode(): Content has no element to edit.", level=kErrorLevel)
        else:
            # Is it for the journal edit?
            if whichField == kGUI.BKEditFieldJRNNote:
                # If so, then it's sort of automatically in edit mode.
                self.BKInEditMode = True
                self.BKEditContent = self.BKCurrentContent
                self.BKEditField = whichField

    ## Save what the player was editing to the right place.
    def BigKISaveEdit(self, noExitEditMode=False):

        if self.BKInEditMode:
            if self.BKEditField == kGUI.BKEditFieldJRNTitle:
                textBox = ptGUIControlTextBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDtextbox]))
                button = ptGUIControlButton(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDbutton]))
                editBox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDeditbox]))
            elif self.BKEditField == kGUI.BKEditFieldJRNNote:
                textBox = ptGUIControlMultiLineEdit(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDtextbox]))
                button = None
                editBox = None
            elif self.BKEditField == kGUI.BKEditFieldPICTitle:
                textBox = ptGUIControlTextBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDtextbox]))
                button = ptGUIControlButton(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDbutton]))
                editBox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDeditbox]))
            else:
                textBox = None
                button = None
                editBox = None

            # Make sure that it can be edited.
            if textBox is not None:
                if self.BKEditContent is not None:
                    edElement = self.BKEditContent.getChild()
                    if edElement is not None:
                        if editBox is not None:
                            if not editBox.wasEscaped():
                                textBox.setString(editBox.getString())
                                if self.BKEditField == kGUI.BKEditFieldJRNTitle:
                                    edElement = edElement.upcastToTextNoteNode()
                                    jTitle = editBox.getStringW()
                                    if jTitle[:len(PtGetLocalizedString("KI.Journal.InitialTitle"))] == PtGetLocalizedString("KI.Journal.InitialTitle"):
                                        # Make sure that the player actually added something (so as not to get a blank title).
                                        if jTitle != PtGetLocalizedString("KI.Journal.InitialTitle"):
                                            jTitle = jTitle[len(PtGetLocalizedString("KI.Journal.InitialTitle")):]
                                    edElement.setTitleW(jTitle)
                                elif self.BKEditField == kGUI.BKEditFieldPICTitle:
                                    edElement = edElement.upcastToImageNode()
                                    pTitle = editBox.getStringW()
                                    if pTitle[:len(PtGetLocalizedString("KI.Image.InitialTitle"))] == PtGetLocalizedString("KI.Image.InitialTitle"):
                                        # Make sure that the player actually added something (so as not to get a blank title).
                                        if pTitle != PtGetLocalizedString("KI.Image.InitialTitle"):
                                            pTitle = pTitle[len(PtGetLocalizedString("KI.Image.InitialTitle")):]
                                    edElement.setTitleW(pTitle)
                                edElement.save()
                        else:
                            if self.BKEditField == kGUI.BKEditFieldJRNNote:
                                buf = textBox.getEncodedBufferW()
                                if buf[:len(PtGetLocalizedString("KI.Journal.InitialMessage"))] == PtGetLocalizedString("KI.Journal.InitialMessage"):
                                    buf = buf[len(PtGetLocalizedString("KI.Journal.InitialMessage")):]
                                edElement = edElement.upcastToTextNoteNode()
                                edElement.setTextW(buf)
                                edElement.save()
                if self.BKEditField != kGUI.BKEditFieldJRNNote:
                    # Put the fields back into no-edit mode.
                    textBox.show()
                    button.show()
                    editBox.hide()
            if not noExitEditMode:
                # Stop editing.
                self.BKInEditMode = False
                self.BKEditContent = None
                self.BKEditField = -1

    ## If the focus has changed, check to see if editing should be stopped.
    def BigKICheckFocusChange(self):

        if self.BKInEditMode:
            if self.BKEditField == kGUI.BKEditFieldJRNTitle:
                editBox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDeditbox]))
            elif self.BKEditField == kGUI.BKEditFieldPICTitle:
                editBox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[self.BKEditField][kGUI.BKEditIDeditbox]))
            else:
                editBox = None
            if editBox is not None:
                if editBox.isFocused():
                    return
            self.BigKISaveEdit()

    ## Mark a content as seen (unimplemented) in the BigKI.
    def BigKISetSeen(self, content):

        if BigKI.dialog.isEnabled():
            content.setSeen()

    #~~~~~~~~~~~~~~#
    # BigKI Saving #
    #~~~~~~~~~~~~~~#

    ## Save the player-editted name of an Age.
    def SaveAgeNameFromEdit(self, control):

        newTitle = ""
        try:
            # Get the selected Age config setting.
            myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
            if not control.wasEscaped():
                # Set the new title.
                myAge.setAgeUserDefinedName(control.getStringW())
                myAge.save()
                PtDebugPrint(u"xKI.SaveAgeNameFromEdit(): Updating title to \"{}\".".format(control.getStringW()), level=kDebugDumpLevel )
            else:
                PtDebugPrint(u"xKI.SaveAgeNameFromEdit(): Escape hit.", level=kDebugDumpLevel )
            newTitle = myAge.getDisplayName()
        except LookupError:
            PtDebugPrint(u"xKI.SaveAgeNameFromEdit(): The current Age could not be found.", level=kDebugDumpLevel )
            myAge = None
        control.hide()
        # Re-enable the button and text.
        title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleTB))
        title.setStringW(newTitle)
        title.show()
        titlebtn = ptGUIControlButton(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleBtn))
        titlebtn.enable()

    ## Save the new name of the Marker Game.
    # This saves it both in the Vault and on the Game Server.
    def SaveMarkerGameNameFromEdit(self, control):

        title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleText))
        if self.BKCurrentContent is not None:
            element = self.BKCurrentContent.getChild()
            if element is not None:
                dataType = element.getType()
                if dataType == PtVaultNodeTypes.kMarkerGameNode:
                    element = element.upcastToMarkerGameNode()
                    if element is not None:
                        if not control.wasEscaped() and control.getString() != "":
                            # Set the new title.
                            newText = xCensor.xCensor(control.getStringW(), self.censorLevel)
                            element.setGameName(control.getStringW())
                            title.setString(control.getStringW())
                            element.save()
                            PtDebugPrint(u"xKI.SaveMarkerGameNameFromEdit(): Updating title to \"{}\".".format(newText), level=kDebugDumpLevel)
                            self.RefreshPlayerList()
                        else:
                            PtDebugPrint(u"xKI.SaveMarkerGameNameFromEdit(): Escape hit.", level=kDebugDumpLevel)
        control.hide()
        # Re-enable the button and text.
        titlebtn = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleBtn))
        titlebtn.enable()
        title.show()

    ## Save the text of a Marker on the Game Server.
    def SaveMarkerTextFromEdit(self, control):

        title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextTB))
        if self.BKCurrentContent is not None:
            element = self.BKCurrentContent.getChild()
            if element is not None:
                dataType = element.getType()
                if dataType == PtVaultNodeTypes.kMarkerGameNode:
                    element = element.upcastToMarkerGameNode()
                    if element is not None:
                        name = control.getStringW()
                        if not control.wasEscaped() and name:
                            self.markerGameManager.selected_marker_name = name
                        else:
                            PtDebugPrint(u"xKI.SaveMarkerTextFromEdit(): escape hit!", level=kDebugDumpLevel )
        control.hide()
        # Re-enable the button and text.
        titlebtn = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextBtn))
        titlebtn.enable()
        title.show()

    #~~~~~~~~~~~~~~~~~~~#
    # GUI Notifications #
    #~~~~~~~~~~~~~~~~~~~#

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
                self.LocalizeDialog(0)
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
                            self.ShowYeeshaBook()
                        else:
                            control.setChecked(0)
                    else:
                        control.setChecked(0)
            elif bbID == kGUI.OptionsMenuButtonID:
                PtShowDialog("OptionsMenuGUI")
            else:
                PtDebugPrint(u"xKI.ProcessNotifyBlackbar(): Don't know this control bbID = {}.".format(bbID), level=kDebugDumpLevel)
        elif event == kInterestingEvent:
            plybkCB = ptGUIControlCheckBox(KIBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
            try:
                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                    PtDebugPrint(u"xKI.ProcessNotifyBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint(u"xKI.ProcessNotifyBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.hide()
            except NameError:
                if self.isEntireYeeshaBookEnabled:
                    PtDebugPrint(u"xKI.ProcessNotifyBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint(u"xKI.ProcessNotifyBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
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
                self.LocalizeDialog(0)
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
                            self.ShowYeeshaBook()
                        else:
                            control.setChecked(0)
                    else:
                        control.setChecked(0)
            elif bbID == kGUI.OptionsMenuButtonID:
                PtShowDialog("OptionsMenuGUI")
            else:
                PtDebugPrint(u"xKI.ProcessNotifyMicroBlackbar(): Don't know this control bbID = {}.".format(bbID), level=kDebugDumpLevel)
        elif event == kInterestingEvent:
            plybkCB = ptGUIControlCheckBox(KIMicroBlackbar.dialog.getControlFromTag(kGUI.PlayerBookCBID))
            try:
                curBrainMode = PtGetLocalAvatar().avatar.getCurrentMode()
                if self.isEntireYeeshaBookEnabled and (curBrainMode == PtBrainModes.kNonGeneric or curBrainMode == PtBrainModes.kAFK or curBrainMode == PtBrainModes.kSit):
                    PtDebugPrint(u"xKI.ProcessNotifyMicroBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint(u"xKI.ProcessNotifyMicroBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.hide()
            except NameError:
                if self.isEntireYeeshaBookEnabled:
                    PtDebugPrint(u"xKI.ProcessNotifyMicroBlackbar(): Show PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.show()
                else:
                    PtDebugPrint(u"xKI.ProcessNotifyMicroBlackbar(): On ladder, hide PlayerBook.", level=kDebugDumpLevel)
                    plybkCB.hide()

    ## Process notifications originating from the microKI.
    # These notifications get called when using the basic chat mode available
    # only until the user obtains a real KI from Gahreesen (thus the name,
    # microKI).
    def ProcessNotifyMicro(self, control, event):

        if event == kDialogLoaded:
            # Fill in the listbox so that the test is near the enter box.
            chatArea = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatArea.lock()         # Make the chat display immutable.
            chatArea.unclickable()  # Make the chat display non-clickable.
            chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            chatArea.disableScrollControl()
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
                if not control.wasEscaped() and control.getStringW() != "":
                    self.chatMgr.SendMessage(control.getStringW())
                self.chatMgr.ToggleChatMode(0)
            elif ctrlID == kGUI.ChatDisplayArea:
                self.ResetFadeState()
        elif event == kFocusChange:
            # If they are chatting, get the focus back.
            if self.chatMgr.isChatting:
                KIMicro.dialog.setFocus(KIMicro.dialog.getControlFromTag(kGUI.ChatEditboxID))
        elif event == kSpecialAction:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                self.Autocomplete(control)

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
            chatArea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatArea.lock()         # Make the chat display immutable.
            chatArea.unclickable()  # Make the chat display non-clickable.
            chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            # Hide the chat scroll buttons (should be nothing in chat area yet anyhow).
            chatArea.disableScrollControl()
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
            for mcbID in range(kGUI.miniMarkerIndicator01, kGUI.miniMarkerIndicatorLast + 1):
                mcb = ptGUIControlProgress(KIMini.dialog.getControlFromTag(mcbID))
                mcb.setValue(kGUI.miniMarkerColors["off"])
        elif event == kShowHide:
            if control.isEnabled():
                if self.pelletImager != "":
                    ptGUIControlButton(KIMini.dialog.getControlFromTag(kGUI.PelletScoreButton)).show()
                if self.miniKIFirstTimeShow:
                    # Set the font size and fade time.
                    self.DetermineFontSize()
                    self.DetermineFadeTime()
                    # If we are chatting then just let it happen.
                    if not self.chatMgr.isChatting:
                        self.chatMgr.ToggleChatMode(0)
                        self.FadeCompletely()
                    self.miniKIFirstTimeShow = False
                self.RefreshPlayerList()
                self.RefreshMiniKIMarkerDisplay()
            else:
                self.chatMgr.ToggleChatMode(0)
                self.chatMgr.ClearBBMini()
        elif event == kAction or event == kValueChanged:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                if not control.wasEscaped() and control.getStringW() != u"":
                    self.chatMgr.SendMessage(control.getStringW())
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
                self.SetBigKIToButtons()
                # No need to keep the focus.
                if self.chatMgr.isChatting:
                    chatedit = ptGUIControlEditBox(KIMini.dialog.getControlFromTag(kGUI.ChatEditboxID))
                    KIMini.dialog.setFocus(chatedit.getKey())
                # They're playing with the player list, so reset the fade.
                self.ResetFadeState()
            elif ctrlID == kGUI.miniPutAwayID:
                self.ToggleMiniKI()
            elif ctrlID == kGUI.miniToggleBtnID:
                self.ToggleKISize()
            elif ctrlID == kGUI.miniTakePicture:
                self.TakePicture()
            elif ctrlID == kGUI.miniCreateJournal:
                self.MiniKICreateJournalNote()
            elif ctrlID == kGUI.miniMuteAll:
                # Hit the mute button, and set mute depending on control.
                audio = ptAudioControl()
                if control.isChecked():
                    audio.muteAll()
                else:
                    audio.unmuteAll()
            elif ctrlID == kGUI.miniPlayerListUp:
                # Scroll the player list up one line.
                self.ScrollPlayerList(1)
            elif ctrlID == kGUI.miniPlayerListDown:
                # Scroll the player list down one line.
                self.ScrollPlayerList(-1)
            elif ctrlID == kGUI.miniGZMarkerInRange:
                self.CaptureGZMarker()
                self.RefreshMiniKIMarkerDisplay()
            elif ctrlID == kGUI.ChatDisplayArea:
                self.ResetFadeState()
            elif ctrlID == kGUI.miniMGNewMarker:
                self.CreateAMarker()
            elif ctrlID == kGUI.miniMGNewGame:
                self.CreateMarkerGame()
            elif ctrlID == kJalakMiniIconBtn:
                if PtGetAgeName() == "Jalak":
                    self.JalakGUIToggle()
                else:
                    ptGUIControlButton(KIMini.dialog.getControlFromTag(kJalakMiniIconBtn)).disable()
            elif ctrlID == kGUI.PelletScoreButton:
                self.UploadPelletScore()
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
                self.Autocomplete(control)
        # Up or Down key to scroll in the chat history
        elif event == kMessageHistoryUp:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                self.MessageHistory(control, "up")
        elif event == kMessageHistoryDown:
            ctrlID = control.getTagID()
            if ctrlID == kGUI.ChatEditboxID:
                self.MessageHistory(control, "down")

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
            for ID in range(kGUI.BKIIncomingBtn, kGUI.BKIFolderLineBtnLast):
                overBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(ID))
                overBtn.setNotifyOnInteresting(1)
        elif event == kShowHide:
            if control.isEnabled():
                # Hide the long folder names.
                for ID in range(kGUI.LONGBKIIncomingLine,kGUI.LONGBKIFolderLineLast+1):
                    longTB = ptGUIControlTextBox(BigKI.dialog.getControlFromTag(ID))
                    longTB.hide()

                self.BigKISetStatics()
                self.BigKISetChanging()
                self.RefreshPlayerList()
                self.KillFadeTimer()
                self.BigKIRefreshFolderList()
                self.BigKIRefreshFolderDisplay()
                self.ShowBigKI()
            else:
                self.StartFadeTimer()
        elif event == kAction or event == kValueChanged:
            bkID = control.getTagID()
            # Is it one of the folder buttons?
            if bkID >= kGUI.BKIIncomingBtn and bkID <= kGUI.BKIFolderLineBtnLast:
                if self.BKFolderLineDict is self.BKConfigFolderDict:
                    self.BKFolderSelected = bkID - kGUI.BKIIncomingBtn + self.BKFolderTopLine
                    self.ShowSelectedConfig()
                else:
                    oldselect = self.BKFolderSelected
                    self.BKFolderSelected = bkID - kGUI.BKIIncomingBtn + self.BKFolderTopLine
                    if oldselect != self.BKFolderSelected:
                        self.BKFolderSelectChanged = True
                    else:
                        self.BKFolderSelectChanged = False
                    self.ChangeBigKIMode(kGUI.BKListMode)
            # Is it the scroll folder up button?
            elif bkID == kGUI.BKFolderUpLine:
                if self.BKFolderTopLine > 0:
                    self.BKFolderTopLine -= 1
                    self.BigKIRefreshFolderDisplay()
                    self.SetBigKIToButtons()
            # Is it the scroll folder down button?
            elif bkID == kGUI.BKFolderDownLine:
                self.BKFolderTopLine += 1
                self.BigKIRefreshFolderDisplay()
                self.SetBigKIToButtons()
            elif bkID == kGUI.BKLMUpButton:
                if self.BKRightSideMode == kGUI.BKListMode:
                    if self.BKContentListTopLine > 0:
                        self.BKContentListTopLine -= kContentListScrollSize
                        if self.BKContentListTopLine < 0:
                            self.BKContentListTopLine = 0
                        self.BigKIRefreshContentListDisplay()
            elif bkID == kGUI.BKLMDownButton:
                if self.BKRightSideMode == kGUI.BKListMode:
                    self.BKContentListTopLine += kContentListScrollSize
                    self.BigKIRefreshContentListDisplay()
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
                                    self.InviteToVisit(playerID, newFolder)
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
                                            self.InviteToVisit(theElement.playerGetID(), newFolder)
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
                    self.ChangeBigKIMode(kGUI.BKListMode)
                    # They could have copied a player, so refresh list.
                    self.RefreshPlayerList()
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
                self.BigKIRefreshFolderDisplay()
                if modeselect == 0 and (self.BKRightSideMode == kGUI.BKPictureExpanded or self.BKRightSideMode == kGUI.BKJournalExpanded or self.BKRightSideMode == kGUI.BKMarkerListExpanded):
                    # The player is taking a picture.
                    self.BigKIInvertToFolderButtons()
                else:
                    # Is the player switching to configuration mode?
                    if modeselect == 2:
                        self.ShowSelectedConfig()
                    # Otherwise, make sure the player is in list mode.
                    else:
                        self.ChangeBigKIMode(kGUI.BKListMode)
            elif bkID == kGUI.BKIToPlayerButton:
                if self.BKCurrentContent is not None and self.BKPlayerSelected is not None:
                    sendElement = self.BKCurrentContent.getChild()
                    toPlayerBtn = ptGUIControlButton(BigKI.dialog.getControlFromTag(kGUI.BKIToPlayerButton))
                    if sendElement is not None:
                        if isinstance(self.BKPlayerSelected, DeviceFolder):
                            pass
                        elif isinstance(self.BKPlayerSelected, Device):
                            if self.BKPlayerSelected.name in self.imagerMap:
                                sName = "Upload={}".format(self.BKPlayerSelected.name)
                                SendNote(self.key, self.imagerMap[self.BKPlayerSelected.name], sName, sendElement.getID())
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
                                self.SetSendToErrorMessage(PtGetLocalizedString("KI.Errors.CantSend"))
                            toPlayerBtn.hide()
                        elif isinstance(self.BKPlayerSelected, ptVaultNodeRef):
                            plyrElement = self.BKPlayerSelected.getChild()
                            if plyrElement is not None and plyrElement.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                plyrElement = plyrElement.upcastToPlayerInfoNode()
                                sendElement.sendTo(plyrElement.playerGetID())
                            else:
                                self.SetSendToErrorMessage(PtGetLocalizedString("KI.Errors.PlayerNotFound"))
                            toPlayerBtn.hide()
                        elif isinstance(self.BKPlayerSelected, ptPlayer):
                            sendElement.sendTo(self.BKPlayerSelected.getPlayerID())
                            toPlayerBtn.hide()
                        else:
                            self.SetSendToErrorMessage(PtGetLocalizedString("KI.Errors.UnknownPlayerType"))
                    else:
                        self.SetSendToErrorMessage(PtGetLocalizedString("KI.Errors.BadJournalElement"))
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
                        if isinstance(self.BKCurrentContent, ptPlayer):
                            nextMode = kGUI.BKPlayerExpanded
                            self.ChangeBigKIMode(nextMode)
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
                                self.ChangeBigKIMode(nextMode)
                            else:
                                PtDebugPrint(u"xKI.ProcessNotifyListMode(): List Mode: content is None for element!", level=kErrorLevel)
            elif lmID == kGUI.BKIListModeCreateBtn:
                if self.BKFolderLineDict is self.BKPlayerFolderDict:
                    self.BKGettingPlayerID = True
                    self.ChangeBigKIMode(kGUI.BKPlayerExpanded)
                else:
                    self.BigKICreateJournalNote()
                    self.ChangeBigKIMode(kGUI.BKJournalExpanded)
                    self.BigKIDisplayJournalEntry()
                    self.BigKIEnterEditMode(kGUI.BKEditFieldJRNTitle)

    ## Process notifications originating from an expanded picture mode in the BigKI.
    # This essentially deals with the taking of new pictures and the editing of
    # existing ones, as well as their deletion.
    def ProcessNotifyPictureExpanded(self, control, event):

        if event == kDialogLoaded:
            editBox = ptGUIControlEditBox(KIPictureExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[kGUI.BKEditFieldPICTitle][kGUI.BKEditIDeditbox]))
            editBox.hide()
        elif event == kShowHide:
            if control.isEnabled():
                self.BigKIDisplayPicture()
        elif event == kAction or event == kValueChanged:
            peID = control.getTagID()
            if peID == kGUI.BKIPICTitleButton:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.BigKIEnterEditMode(kGUI.BKEditFieldPICTitle)
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
                self.LocalizeDialog(1)
                KIYesNo.dialog.show()
            elif peID == kGUI.BKIPICTitleEdit:
                self.BigKISaveEdit(1)
        elif event == kFocusChange:
            if self.IsContentMutable(self.BKCurrentContent):
                self.BigKICheckFocusChange()

    ## Process notifications originating from an expanded journal mode in the BigKI.
    # Handles note creation, editing and deletion.
    def ProcessNotifyJournalExpanded(self, control, event):

        if event == kDialogLoaded:
            editBox = ptGUIControlEditBox(KIJournalExpanded.dialog.getControlFromTag(kGUI.BKEditFieldIDs[kGUI.BKEditFieldJRNTitle][kGUI.BKEditIDeditbox]))
            editBox.hide()
        elif event == kShowHide:
            if control.isEnabled():
                self.BigKIDisplayJournalEntry()
        elif event == kAction or event == kValueChanged:
            jeID = control.getTagID()
            # Is it one of the buttons?
            if jeID == kGUI.BKIJRNTitleButton:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.BigKIEnterEditMode(kGUI.BKEditFieldJRNTitle)
            elif jeID == kGUI.BKIJRNNoteButton:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.BigKIEnterEditMode(kGUI.BKEditFieldJRNNote)
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
                self.LocalizeDialog(1)
                KIYesNo.dialog.show()
            # Is it one of the editing boxes?
            elif jeID == kGUI.BKIJRNTitleEdit or jeID == kGUI.BKIJRNNoteEdit:
                if self.IsContentMutable(self.BKCurrentContent):
                    self.BigKISaveEdit(1)
        elif event == kFocusChange:
            if self.IsContentMutable(self.BKCurrentContent):
                if control is not None:
                    # If the focus is changing to the multiline, the plaer is entering edit mode.
                    jeID = control.getTagID()
                    if jeID == kGUI.BKIJRNNote:
                        self.BigKIEnterEditMode(kGUI.BKEditFieldJRNNote)
                        return
                self.BigKICheckFocusChange()

    ## Process notifications originating from an expanded player mode in the BigKI.
    # Handles deletion of a player's entry.
    def ProcessNotifyPlayerExpanded(self, control, event):

        if event == kShowHide:
            if control.isEnabled():
                self.BigKIDisplayPlayerEntry()
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
                self.LocalizeDialog(1)
                KIYesNo.dialog.show()
            elif plID == kGUI.BKIPLYPlayerIDEditBox:
                self.BigKICheckSavePlayer()
        elif event == kFocusChange:
            if self.BKGettingPlayerID:
                if KIPlayerExpanded.dialog.isEnabled():
                    plyIDedit = ptGUIControlEditBox(KIPlayerExpanded.dialog.getControlFromTag(kGUI.BKIPLYPlayerIDEditBox))
                    plyIDedit.focus()
                    KIPlayerExpanded.dialog.setFocus(plyIDedit.getKey())
                else:
                    self.BKGettingPlayerID = False
                    self.ChangeBigKIMode(kGUI.BKListMode)

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
                self.RefreshKISettings()
            else:
                self.SaveFontSize()
                self.SaveFadeTime()
                self.SaveKIFlags()
        elif event == kAction or event == kValueChanged:
            kiID = control.getTagID()
            if kiID == kGUI.BKIKIFontSize:
                slidePerFont = float(control.getMax() - control.getMin() + 1.0) / float(len(kChat.FontSizeList))
                fontIndex = int(control.getValue() / slidePerFont + 0.25)
                if fontIndex >= len(kChat.FontSizeList):
                    fontIndex = len(kChat.FontSizeList) - 1
                self.SetFontSize(kChat.FontSizeList[fontIndex])
            elif kiID == kGUI.BKIKIFadeTime:
                slidePerTime = float(control.getMax() - control.getMin()) / float(kChat.FadeTimeMax)
                self.chatMgr.ticksOnFull = int(control.getValue() / slidePerTime + 0.25)
                PtDebugPrint(u"xKI.ProcessNotifySettingsExpanded(): FadeTime set to {}.".format(self.chatMgr.ticksOnFull), level=kDebugDumpLevel)
                if self.chatMgr.ticksOnFull == kChat.FadeTimeMax:
                    self.chatMgr.fadeEnableFlag = False
                    PtDebugPrint(u"KISettings: FadeTime disabled.", level=kDebugDumpLevel)
                else:
                    self.chatMgr.fadeEnableFlag = True
                    PtDebugPrint(u"KISettings: FadeTime enabled.", level=kDebugDumpLevel)
            elif kiID == kGUI.BKIKIOnlyPM:
                self.onlyGetPMsFromBuddies = control.isChecked()
            elif kiID == kGUI.BKIKIBuddyCheck:
                self.onlyAllowBuddiesOnRequest = control.isChecked()

    ## Process notifications originating from an expanded settings mode in the BigKI.
    # Handles the sound controls from the options menu being modified.
    def ProcessNotifyVolumeExpanded(self, control, event):

        if event == kShowHide:
            if control.isEnabled():
                self.RefreshVolumeSettings()
        elif event == kAction or event == kValueChanged:
            plID = control.getTagID()
            audio = ptAudioControl()
            if plID == kGUI.BKISoundFXVolSlider:
                setting = control.getValue()
                PtDebugPrint(u"xKI.ProcessNotifyVolumeExpanded(): SoundFX being changed to {:g} (into {:g}).".format(setting, setting / 10), level=kDebugDumpLevel)
                audio.setSoundFXVolume(setting / 10)
            elif plID == kGUI.BKIMusicVolSlider:
                setting = control.getValue()
                PtDebugPrint(u"xKI.ProcessNotifyVolumeExpanded(): Music being changed to {:g} (into {:g}).".format(setting, setting / 10), level=kDebugDumpLevel)
                audio.setMusicVolume(setting / 10)
            elif plID == kGUI.BKIVoiceVolSlider:
                setting = control.getValue()
                PtDebugPrint(u"xKI.ProcessNotifyVolumeExpanded(): Voice being changed to {:g} (into {:g}).".format(setting, setting / 10), level=kDebugDumpLevel)
                audio.setVoiceVolume(setting / 10)
            elif plID == kGUI.BKIAmbienceVolSlider:
                setting = control.getValue()
                PtDebugPrint(u"xKI.ProcessNotifyVolumeExpanded(): Ambience being changed to {:g} (into {:g}).".format(setting, setting / 10), level=kDebugDumpLevel)
                audio.setAmbienceVolume(setting / 10)
            elif plID == kGUI.BKIMicLevelSlider:
                setting = control.getValue()
                PtDebugPrint(u"xKI.ProcessNotifyVolumeExpanded(): MicLevel being changed to {:g} (into {:g}).".format(setting, setting / 10), level=kDebugDumpLevel)
                audio.setMicLevel(setting / 10)
            elif plID == kGUI.BKIGUIVolSlider:
                setting = control.getValue()
                PtDebugPrint(u"xKI.ProcessNotifyVolumeExpanded(): MicLevel being changed to {:g} (into {:g}).".format(setting, setting / 10), level=kDebugDumpLevel)
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
                self.RefreshAgeOwnerSettings()
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
                        PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Making {} private.".format(myAge.getDisplayName()), level=kDebugDumpLevel)
                    else:
                        PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Making {} public.".format(myAge.getDisplayName()), level=kDebugDumpLevel)
                    vault.setAgePublic(myAgeStruct, makePublic)
                    # Let the refresh re-enable the public button.
                    control.disable()
                except AttributeError:
                    PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Couldn't toggle public/private.", level=kErrorLevel)
            elif plID == kGUI.BKAgeOwnerTitleBtn:
                PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Change title button hit.", level=kDebugDumpLevel)
                control.disable()
                title = ptGUIControlTextBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleTB))
                title.hide()
                titleEdit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleEditbox))
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
                PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): edit field set.", level=kDebugDumpLevel)
                self.SaveAgeNameFromEdit(control)
        elif event == kFocusChange:
            PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Focus change.", level=kDebugDumpLevel)
            titleEdit = ptGUIControlEditBox(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerTitleEditbox))
            if titleEdit.isVisible():
                if control is None or (control.getTagID() != kGUI.BKAgeOwnerTitleEditbox and control.getTagID() != kGUI.BKAgeOwnerTitleBtn):
                    self.SaveAgeNameFromEdit(titleEdit)
            if control is not None:
                # Check if the decription was updated.
                plID = control.getTagID()
                if plID == kGUI.BKAgeOwnerDescription:
                    self.BKAgeOwnerEditDescription = True
                    PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Start editing description.", level=kDebugDumpLevel)
                else:
                    if self.BKAgeOwnerEditDescription:
                        descript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescription))
                        myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
                        if myAge is not None:
                            PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Age description updated for {}.".format(myAge.getDisplayName()), level=kDebugDumpLevel)
                            myAge.setAgeDescription(descript.getString())
                            myAge.save()
                        else:
                            PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Neighborhood is None while trying to update description.", level=kDebugDumpLevel)
                    self.BKAgeOwnerEditDescription = False
            else:
                if self.BKAgeOwnerEditDescription:
                    descript = ptGUIControlMultiLineEdit(KIAgeOwnerExpanded.dialog.getControlFromTag(kGUI.BKAgeOwnerDescription))
                    myAge = self.BKConfigFolderDict[self.BKConfigListOrder[self.BKFolderSelected]]
                    if myAge is not None:
                        PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Age description updated for {}.".format(myAge.getDisplayName()), level=kDebugDumpLevel)
                        buff = descript.getEncodedBuffer()
                        myAge.setAgeDescription(str(buff))
                        myAge.save()
                    else:
                        PtDebugPrint(u"xKI.ProcessNotifyAgeOwnerExpanded(): Neighborhood is None while trying to update description.", level=kDebugDumpLevel)
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
                    chatArea = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kGUI.ChatDisplayArea))
                    chatArea.setString("")
                    chatArea.moveCursor(PtGUIMultiLineDirection.kBufferStart)
                    KIMicro.dialog.refreshAllControls()

                    # Clear out all chat on miniKI.
                    chatArea = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
                    chatArea.setString("")
                    chatArea.moveCursor(PtGUIMultiLineDirection.kBufferStart)
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

            elif self.YNWhatReason == kGUI.YNDelete:
                if ynID == kGUI.YesButtonID:
                    # Remove the current element
                    if self.BKCurrentContent is not None:
                        delFolder = self.BKCurrentContent.getParent()
                        delElem = self.BKCurrentContent.getChild()
                        if delFolder is not None and delElem is not None:
                            # Are we removing a visitor from an Age we own?
                            tFolder = delFolder.upcastToFolderNode()
                            if tFolder is not None and tFolder.folderGetType() == PtVaultStandardNodes.kCanVisitFolder:
                                PtDebugPrint(u"xKI.ProcessNotifyYesNo(): Revoking visitor.", level=kDebugDumpLevel)
                                delElem = delElem.upcastToPlayerInfoNode()
                                # Need to refind the folder that has the ageInfo in it.
                                ageFolderName = self.BKFolderListOrder[self.BKFolderSelected]
                                ageFolder = self.BKFolderLineDict[ageFolderName]
                                # Revoke invite.
                                ptVault().unInvitePlayerToAge(ageFolder.getAgeInstanceGuid(), delElem.playerGetID())
                            # Are we removing a player from a player list?
                            elif delFolder.getType() == PtVaultNodeTypes.kPlayerInfoListNode and delElem.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                                PtDebugPrint(u"xKI.ProcessNotifyYesNo(): Removing player from folder.", level=kDebugDumpLevel)
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
                                    if self.markerGameManager.IsActive(delElem):
                                        self.markerGameManager.StopGame()

                                self.BKCurrentContent = None
                                delFolder.removeNode(delElem)
                                PtDebugPrint(u"xKI.ProcessNotifyYesNo(): Deleting element from folder.", level=kDebugDumpLevel)
                        else:
                            PtDebugPrint(u"xKI.ProcessNotifyYesNo(): Tried to delete bad Vault node or delete from bad folder.", level=kErrorLevel)
                        self.ChangeBigKIMode(kGUI.BKListMode)
                        self.RefreshPlayerList()
                self.YNWhatReason = kGUI.YNQuit
                KIYesNo.dialog.hide()
            elif self.YNWhatReason == kGUI.YNOfferLink:
                self.YNWhatReason = kGUI.YNQuit
                KIYesNo.dialog.hide()
                if ynID == kGUI.YesButtonID:
                    if self.offerLinkFromWho is not None:
                        PtDebugPrint(u"xKI.ProcessNotifyYesNo(): Linking to offered age {}.".format(self.offerLinkFromWho.getDisplayName()), level=kDebugDumpLevel)
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
                    elif ynID == kGUI.NoButtonID:
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
                self.YNWhatReason = kGUI.YNQuit
            else:
                self.YNWhatReason = kGUI.YNQuit
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
                self.AlertStartTimer()

    ## Process notifications originating from the Marker Game creation GUI.
    # This gets the values submitted by the player and passes them to the
    # Marker Game manager.
    def ProcessNotifyCreateMarkerGameGUI(self, control, event):

        if control:
            tagID = control.getTagID()
        if event == kDialogLoaded:
            self.markerGameDefaultColor = ptGUIControlTextBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kGUI.MarkerGameLabel1)).getForeColor()
            self.markerGameSelectedColor = ptGUIControlTextBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kGUI.MarkerGameLabel1)).getSelectColor()
        elif event == kShowHide:
            self.InitMarkerGameGUI()
            PtDebugPrint(u"xKI.ProcessNotifyCreateMarkerGameGUI(): Marker Game dialog is showing or hiding.", level=kDebugDumpLevel)
        elif event == kAction or event == kValueChanged:
            if tagID == kGUI.MarkerGameType1 or tagID == kGUI.MarkerGameType2 or tagID == kGUI.MarkerGameType3:
                self.SelectMarkerType(tagID)
            elif tagID == kGUI.CreateMarkerGameCancelBT:
                KIMarkerGameGUIClose.run(self.key, netPropagate=0)
            elif kGUI.CreateMarkerGameSubmitBT:
                markerGameNameText = ptGUIControlEditBox(KICreateMarkerGameGUI.dialog.getControlFromTag(kGUI.CreateMarkerGameNameEB)).getStringW()
                try:
                    markerGameType = kGUI.MarkerGameStates[self.selectedMGType]
                except:
                    markerGameType = 0
                    PtDebugPrint(u"xKI.ProcessNotifyCreateMarkerGameGUI(): Couldn't find marker game type, so setting it to Quest Mode.", level=kWarningLevel)
                self.FinishCreateMarkerGame(markerGameNameText)
                KIMarkerGameGUIClose.run(self.key, netPropagate=0)

    ## Processes notifications originating from an expanded Marker Game mode in the BigKI.
    # This handles the edit buttons, marker saving buttons, deletion buttons,
    # etc..
    def ProcessNotifyMarkerFolderExpanded(self, control, event):

        mgr = self.markerGameManager

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
                self.BigKIDisplayMarkerGame()
        elif event == kAction or event == kValueChanged:
            mFldrID = control.getTagID()
            if mFldrID == kGUI.MarkerFolderEditStartGame:
                mgr.LoadGame(self.BKCurrentContent)

                # Is it the "Edit" button?
                if self.MFdialogMode == kGames.MFOverview:
                    mgr.BeginEditingMarkers()
                    self.SetWorkingToCurrentMarkerGame()
                # Is it the "Done Editing" button?
                elif self.MFdialogMode == kGames.MFEditing:
                    mgr.FinishEditingMarkers()
                    self.ResetWorkingMarkerGame()
                # Is it the "Stop Game" button?
                elif self.MFdialogMode == kGames.MFPlaying:
                    mgr.StopGame(reset=False)
                # Is it the "Save Marker" button?
                elif self.MFdialogMode == kGames.MFEditingMarker:
                    # Should already be saved, just clear selection for now.
                    mgr.selected_marker_id = -1
                self.BigKICheckContentRefresh(self.BKCurrentContent)
            elif mFldrID == kGUI.MarkerFolderPlayEndGame:
                mgr.LoadGame(self.BKCurrentContent)

                # Is it the "Play Game" button?
                if self.MFdialogMode == kGames.MFOverview:
                    mgr.Play()
                # Is it the "Add Marker" button?
                elif self.MFdialogMode == kGames.MFEditing:
                    self.CreateAMarker()
                # Is it the "Reset Game" button?
                elif self.MFdialogMode == kGames.MFPlaying:
                    mgr.StopGame(reset=True)
                # Is it the "Remove Marker" button?
                elif self.MFdialogMode == kGames.MFEditingMarker:
                    mgr.DeleteMarker(mgr.selected_marker_id)
                self.BigKICheckContentRefresh(self.BKCurrentContent)

            elif mFldrID == kGUI.MarkerFolderMarkListbox:
                mgr.LoadGame(self.BKCurrentContent)

                # Save the current scroll position before transitioning to some other content
                mfmlb = ptGUIControlListBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkListbox))
                self.MFScrollPos = mfmlb.getScrollPos()

                if not mgr.playing:
                    # NOTE: We must use selected_marker_index because marker IDs don't necessarily
                    #       match up with the indices used in the GUI
                    mgr.selected_marker_index = control.getSelection()
                    self.BigKICheckContentRefresh(self.BKCurrentContent)

            elif mFldrID == kGUI.MarkerFolderTitleBtn:
                control.disable()
                title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleText))
                titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleEB))
                titleEdit.setStringW(title.getStringW())
                title.hide()
                titleEdit.show()
                titleEdit.end()
                KIMarkerFolderExpanded.dialog.setFocus(titleEdit.getKey())
            elif mFldrID == kGUI.MarkerFolderTitleEB:
                self.SaveMarkerGameNameFromEdit(control)
                self.BigKICheckContentRefresh(self.BKCurrentContent)
            elif mFldrID == kGUI.MarkerFolderMarkerTextBtn:
                control.disable()
                title = ptGUIControlTextBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextTB))
                titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextEB))
                titleEdit.setStringW(title.getStringW())
                title.hide()
                titleEdit.show()
                titleEdit.end()
                KIMarkerFolderExpanded.dialog.setFocus(titleEdit.getKey())
            elif mFldrID == kGUI.MarkerFolderMarkerTextEB:
                self.SaveMarkerTextFromEdit(control)
                self.BigKICheckContentRefresh(self.BKCurrentContent)
            elif mFldrID == kGUI.MarkerFolderTimePullDownBtn or mFldrID == kGUI.MarkerFolderTimeArrow:
                KIMarkerFolderPopupMenu.menu.show()
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
                self.LocalizeDialog(1)
                KIYesNo.dialog.show()
        elif event == kFocusChange:
            titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderTitleEB))
            # Is the editbox enabled and something other than the button is getting the focus?
            if titleEdit.isVisible():
                if control is None or (control.getTagID() != kGUI.MarkerFolderTitleEB and control.getTagID() != kGUI.MarkerFolderTitleBtn):
                    self.SaveMarkerGameNameFromEdit(titleEdit)
            if self.MFdialogMode == kGames.MFEditingMarker:
                titleEdit = ptGUIControlEditBox(KIMarkerFolderExpanded.dialog.getControlFromTag(kGUI.MarkerFolderMarkerTextEB))
                # Is the editbox enabled and something other than the button is getting the focus?
                if titleEdit.isVisible():
                    if control is None or (control.getTagID() != kGUI.MarkerFolderMarkerTextEB and control.getTagID() != kGUI.MarkerFolderMarkerTextBtn):
                        self.SaveMarkerTextFromEdit(titleEdit)
                        self.BigKICheckContentRefresh(self.BKCurrentContent)

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
            # Save the current Marker Game to this type of game.
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

    #~~~~~~~~~~~~~~~~~~~#
    # Vault Type Events #
    #~~~~~~~~~~~~~~~~~~~#

    ## Handles the passed vault type event.
    # This is used to react to saved nodes, new nodes, etc.
    def HandleVaultTypeEvents(self, event, tupData):

        # Make sure that the BigKI dialog is loaded before trying to update it.
        if not PtIsDialogLoaded("KIMain"):
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): BigKI dialog was not loaded, waiting.", level=kDebugDumpLevel)
            return
        if event == PtVaultCallbackTypes.kVaultConnected:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): Connected to the Vault.", level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultDisconnected:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): Disconnected from the Vault.", level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultNodeSaved:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): A node is being saved (ID = {}, type = {}).".format(tupData[0].getID(), tupData[0].getType()), level=kDebugDumpLevel)
            if tupData[0].getType() == PtVaultNodeTypes.kPlayerInfoNode:
                self.RefreshPlayerList()
            elif tupData[0].getType() == PtVaultNodeTypes.kAgeInfoNode:
                self.BigKISetStatics()
                self.BigKIRefreshFolderList()
                self.BigKIOnlySelectedToButtons()
                self.RefreshAgeOwnerSettings()
            self.BigKIRefreshContentList()
            self.BigKIRefreshContentListDisplay()
        elif event == PtVaultCallbackTypes.kVaultNodeInitialized:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): A node has been initalized (ID = {}, type = {}).".format(tupData[0].getID(), tupData[0].getType()), level=kDebugDumpLevel)
            if self.KILevel > kMicroKI:
                self.BigKICheckElementRefresh(tupData[0])
        elif event == PtVaultCallbackTypes.kVaultNodeAdded:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): A node has been added.", level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultNodeRefAdded:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): A node reference has been added (childID = {}, parentID = {}).".format(tupData[0].getChildID(), tupData[0].getParentID()), level=kDebugDumpLevel)
            if self.KILevel > kMicroKI:
                folder = tupData[0].getParent()
                folder = folder.upcastToFolderNode()
                # If the parent of this ref is the Inbox, then it's incoming mail.
                if folder is not None and folder.folderGetType() == PtVaultStandardNodes.kInboxFolder:
                    self.AlertKIStart()
                    # Note: beenSeen() is not yet implemented.
                    if not tupData[0].beenSeen():
                        if self.onlyGetPMsFromBuddies:
                            vault = ptVault()
                            buddies = vault.getBuddyListFolder()
                            if buddies.playerlistHasPlayer(tupData[0].getSaverID()):
                                # then show alert
                                self.AlertKIStart()
                        else:
                            self.AlertKIStart()

                child = tupData[0].getChild()
                child = child.upcastToFolderNode()
                if child is not None:
                    PtDebugPrint(u"xKI.HandleVaultTypeEvents(): Adding a folder, refresh folder list.", level=kDebugDumpLevel)
                    self.BigKIRefreshFolderList()
                self.BigKICheckFolderRefresh(folder)
        elif event == PtVaultCallbackTypes.kVaultRemovingNodeRef:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): A node reference is being removed (childID = {}, parentID = {}).".format(tupData[0].getChildID(), tupData[0].getParentID()), level=kDebugDumpLevel)
        elif event == PtVaultCallbackTypes.kVaultNodeRefRemoved:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): A node reference has been removed (childID, parentID): ", tupData, level=kDebugDumpLevel)
            if self.KILevel > kMicroKI:
                if self.BKRightSideMode == kGUI.BKMarkerListExpanded:
                    self.BigKIDisplayMarkerGame()
                self.BigKICheckFolderRefresh()
        elif event == PtVaultCallbackTypes.kVaultOperationFailed:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): A Vault operation failed (operation, resultCode): ", tupData, level=kDebugDumpLevel)
        else:
            PtDebugPrint(u"xKI.HandleVaultTypeEvents(): Unknown Vault event: {}.".format(event), level=kWarningLevel)
