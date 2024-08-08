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

from __future__ import annotations

MaxVersionNumber = 8
MinorVersionNumber = 4

import functools
import os
from typing import *

from Plasma import *
from PlasmaConstants import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

from xGUILinkHandler import xGUILinkHandler
import xJournalBookDefs
import xIniAudio
import xIniDisplay
import xIniNumSounds

# define the attributes that will be entered in max
OptionsMenuDlg          = ptAttribGUIDialog(1,"The Options Menu dialog")
CCRDlg                  = ptAttribGUIDialog(2, "The Petition (CCR) dialog")
CCRPopupMenu            = ptAttribGUIPopUpMenu(3, "The Help/Petition Popup Menu")
NavigationDlg           = ptAttribGUIDialog(4, "The Navigation dialog")
KeyMapDlg               = ptAttribGUIDialog(5, "The Key Map dialog")
GraphicsSettingsDlg     = ptAttribGUIDialog(6, "Graphics Settings dialog")
CalibrateDlg            = ptAttribGUIDialog(7, "The Calibration dialog")
AudioSettingsDlg        = ptAttribGUIDialog(8, "The Audio Settings dialog")
TrailerDlg              = ptAttribGUIDialog(9, "The Trailer dialog")
AdvGameSettingDlg       = ptAttribGUIDialog(10, "The Adv Game Settings dialog")
ResetWarnDlg            = ptAttribGUIDialog(11, "The Reset Warning dialog")
ReleaseNotesDlg         = ptAttribGUIDialog(12, "Release Notes dialog")
respDisableItems        = ptAttribResponder(13, "resp: Disable Items", ["enableRes", "disableRes", "enableWindow", "disableWindow", "enableEAX", "disableEAX", "enableGamma", "disableGamma", "enableDynRefl", "disableDynRefl"])
SupportDlg              = ptAttribGUIDialog(14, "Support dialog")


# globals
#--------

gLiveMovie = None
gCurrentMovieName = None
gLiveMovieList = []
kLiveMovieDir = "avi"
kDemoMovieName = "avi/UruPreview.webm"
gPreviewStarted = 0
prevAudioDeviceName = None
gAudioDevices = ()

# =====================================
# Aspect Ratios
#---------
_ratios = (
    ("5:4", 1.25, 1.27),
    ("4:3", 1.28, 1.35),
    ("3:2", 1.47, 1.54),
    ("14:9", 1.55, 1.57),
    ("16:10", 1.58, 1.61),
    ("5:3", 1.65, 1.67),
    ("16:9", 1.70, 1.79),
    ("21:9", 2.34, 2.41),
)

# =====================================
# Options Menu dialog globals
#---------
kOptionsKeyMapBtn = 299
kOptionsKeyMapText = 399
kOptionsSettingsBtn = 300
kOptionsSettingText = 400
kOptionsTrailerBtn = 301
kOptionsTrailerText = 401
kOptionsCreditsBtn = 303
kOptionsCreditsText = 403
kOptionsQuitBtn = 304
kOptionsQuitText = 404
kOptionsCalibrationBtn = 305
kOptionsCalibrationText = 405
kOptionsOkBtn = 310
kOptionsOkText = 800
kOptionsDefaultsText = 810
kOptionsGoBackText = 820

kOptionsNavigationBtn = 311
kOptionsNavigationText = 312
kOptionsGraphicSettingsBtn = 313
kOptionsGraphicsSettingsText = 314
kOptionsAudioSettingsBtn = 315
kOptionsAudioSettingsText = 316
kOptionsResetNow = 317
kOptionsResetLater = 318
kOptionsReleaseNotes = 319

kReleaseTextArea = 892
kReleaseUpArrow = 893
kReleaseDownArrow = 894
kRNOkBtn = 990
kRNGoBackBtn = 991
kRNOkText = 890
kRNGoBackText = 891

kSupportCancel = 780
kSupportLiveHelp = 781
kSupportTicket = 782

kClickOnMeBtn = 99

kTrailerPreviousButton = 98
kTrailerNextButton = 97

gJournalBook = None

# ==============================
# In Game Help Dialog text string IDs
kHelpTitle = 600
kWalkText = 610
kRunText = 611
kTurnLeftText = 612
kBackwardsText = 613
kTurnRightText = 614
kToggleViewText = 615
kJumpText = 616
kMouseWalkText = 620
kMouseRunText = 621
kSelectText = 622
kMousePanCam = 623
kMouseBackwards = 624
kMousePresetsTitle = 630
kMouseNormalText = 631
kMouseNoviceText = 632

kNormNoviceRGID = 750

# =====================================
# Key Map dialog globals
#---------

kKMEditLine1Row1 = 300
kKMEditLine2Row1 = 301
kKMEditLine3Row1 = 302
kKMEditLine4Row1 = 303
kKMEditLine5Row1 = 304
kKMEditLine6Row1 = 305
kKMEditLine7Row1 = 306
kKMEditLine8Row1 = 307
kKMEditLine9Row1 = 308
kKMEditLine10Row1 = 309
kKMEditLine11Row1 = 310
kKMEditLine12Row1 = 311
kKMEditLine13Row1 = 312
kKMEditLine14Row1 = 313
kKMEditLine15Row1 = 314
kKMEditLine16Row1 = 315
kKMEditLine17Row1 = 316
kKMEditLine18Row1 = 317
# these MUST be 100 more than the above constants
kKMEditLine1Row2 = 400
kKMEditLine2Row2 = 401
kKMEditLine3Row2 = 402
kKMEditLine4Row2 = 403
kKMEditLine5Row2 = 404
kKMEditLine6Row2 = 405
kKMEditLine7Row2 = 406
kKMEditLine8Row2 = 407
kKMEditLine9Row2 = 408
kKMEditLine10Row2 = 409
kKMEditLine11Row2 = 410
kKMEditLine12Row2 = 411
kKMEditLine13Row2 = 412
kKMEditLine14Row2 = 413
kKMEditLine15Row2 = 414
kKMEditLine16Row2 = 415
kKMEditLine17Row2 = 416
kKMEditLine18Row2 = 417

kKMOkBtn = 500
kKMDefaultsBtn = 510
kKMGoBackBtn = 520
kKMNextPreviousBtn = 530

# these must be 100 less than the editLineNRowM
kKMTextLine1 = 200
kKMTextLine2 = 201
kKMTextLine3 = 202
kKMTextLine4 = 203
kKMTextLine5 = 204
kKMTextLine6 = 205
kKMTextLine7 = 206
kKMTextLine8 = 207
kKMTextLine9 = 208
kKMTextLine10 = 209
kKMTextLine11 = 210
kKMTextLine12 = 211
kKMTextLine13 = 212
kKMTextLine14 = 213
kKMTextLine15 = 214
kKMTextLine16 = 215
kKMTextLine17 = 216
kKMTextLine18 = 217

kKMNextPreviousText = 830

#
# dictionary of:
#    edit tagID to ( keyMap, singlePlayerFlag, multiPlayerFlag )
#
#  where keyMap is a number if event binding
#                  a string if console command
#                  None if not mapped (mostly on second key on console)
#

class _KeyLine(NamedTuple):
    controlCode: Union[str, int, None]
    singlePlayer: bool
    multiPlayer: bool


gKM1ControlCodesRow1 = {
    kKMEditLine1Row1: _KeyLine(PlasmaControlKeys.kKeyMoveForward, True, True),
    kKMEditLine2Row1: _KeyLine(PlasmaControlKeys.kKeyMoveBackward, True, True),
    kKMEditLine3Row1: _KeyLine(PlasmaControlKeys.kKeyRotateLeft, True, True),
    kKMEditLine4Row1: _KeyLine(PlasmaControlKeys.kKeyRotateRight, True, True),
    kKMEditLine5Row1: _KeyLine(PlasmaControlKeys.kKeyJump, True, True),
    kKMEditLine6Row1: _KeyLine(PlasmaControlKeys.kKeyStrafeLeft, True, True),
    kKMEditLine7Row1: _KeyLine(PlasmaControlKeys.kKeyStrafeRight, True, True),
    kKMEditLine8Row1: _KeyLine(PlasmaControlKeys.kKeyExitMode, True, True),
    kKMEditLine9Row1: _KeyLine(PlasmaControlKeys.kKeySetFirstPersonMode, True, True),
    kKMEditLine10Row1: _KeyLine("Game.KIOpenYeeshaBook", True, True),
    kKMEditLine11Row1: _KeyLine("Game.KIHelp", True, True),
    kKMEditLine12Row1: _KeyLine("Game.KIOpenKI", False, True),
    kKMEditLine13Row1: _KeyLine("Game.KITakePicture", False, True),
    kKMEditLine14Row1: _KeyLine("Game.KICreateJournal", False, True),
    kKMEditLine15Row1: _KeyLine(PlasmaControlKeys.kKeyPushToTalk, False, True),
    kKMEditLine16Row1: _KeyLine("Game.EnterChatMode", False, True),
    kKMEditLine17Row1: _KeyLine("Game.KICreateMarkerFolder", False, True),
    kKMEditLine18Row1: _KeyLine("Game.KICreateMarker", False, True),
}

gKM1ControlCodesRow2 = {
    kKMEditLine1Row2: _KeyLine(PlasmaControlKeys.kKeyMoveForward, True, True),
    kKMEditLine2Row2: _KeyLine(PlasmaControlKeys.kKeyMoveBackward, True, True),
    kKMEditLine3Row2: _KeyLine(PlasmaControlKeys.kKeyRotateLeft, True, True),
    kKMEditLine4Row2: _KeyLine(PlasmaControlKeys.kKeyRotateRight, True, True),
    kKMEditLine5Row2: _KeyLine(PlasmaControlKeys.kKeyJump, True, True),
    kKMEditLine6Row2: _KeyLine(PlasmaControlKeys.kKeyStrafeLeft, True, True),
    kKMEditLine7Row2: _KeyLine(PlasmaControlKeys.kKeyStrafeRight, True, True),
    kKMEditLine8Row2: _KeyLine(PlasmaControlKeys.kKeyExitMode, True, True),
    kKMEditLine9Row2: _KeyLine(PlasmaControlKeys.kKeySetFirstPersonMode, True, True),
    kKMEditLine10Row2: _KeyLine(None, False, False),
    kKMEditLine11Row2: _KeyLine(None, False, False),
    kKMEditLine12Row2: _KeyLine(None, False, False),
    kKMEditLine13Row2: _KeyLine(None, False, False),
    kKMEditLine14Row2: _KeyLine(None, False, False),
    kKMEditLine15Row2: _KeyLine(PlasmaControlKeys.kKeyPushToTalk, False, True),
    kKMEditLine16Row2: _KeyLine(None, False, False),
    kKMEditLine17Row2: _KeyLine(None, False, False),
    kKMEditLine18Row2: _KeyLine(None, False, False),
}

kDefaultControlCodeBinds = {
    PlasmaControlKeys.kKeyMoveForward: ("UpArrow", "(unmapped)"),
    PlasmaControlKeys.kKeyMoveBackward: ("DownArrow", "(unmapped)"),
    PlasmaControlKeys.kKeyRotateLeft: ("LeftArrow", "(unmapped)"),
    PlasmaControlKeys.kKeyRotateRight: ("RightArrow", "(unmapped)"),
    PlasmaControlKeys.kKeyJump: ("SpaceBar", "(unmapped)"),
    PlasmaControlKeys.kKeyStrafeLeft: ("Comma", "(unmapped)"),
    PlasmaControlKeys.kKeyStrafeRight: ("Period", "(unmapped)"),
    PlasmaControlKeys.kKeyExitMode: ("Backspace", "Esc"),
    PlasmaControlKeys.kKeySetFirstPersonMode: ("F1", "F_C"),
    "Game.KIOpenYeeshaBook": ("F3", "(unmapped)"),
    "Game.KIHelp": ("F4", "(unmapped)"),
    "Game.KIOpenKI": ("F2", "(unmapped)"),
    "Game.KITakePicture": ("F5", "(unmapped)"),
    "Game.KICreateJournal": ("F6", "(unmapped)"),
    PlasmaControlKeys.kKeyPushToTalk: ( "Tab", "(unmapped)" ),
    "Game.EnterChatMode": ("(unmapped)", "(unmapped)"),
    "Game.KICreateMarkerFolder": ("F8", "(unmapped)"),
    "Game.KICreateMarker": ("F7", "(unmapped)"),
}

kControlCodes = tuple(kDefaultControlCodeBinds.keys())

kVideoQuality = ["Low", "Medium", "High", "Ultra"]
kVideoTextureQuality = ["Low", "Medium", "High"]
kVideoAntiAliasing = {"0": 0, "2": 1, "4": 2, "6": 3}
kVideoAnisoFiltering = {"0": 0, "2": 1, "4": 2, "8": 3, "16": 4, "1": 0}

#--Volume expanded controls
kGSOkBtn = 500
kGSDefaultsBtn = 510
kGSGoBackBtn = 520
kGSAdvPreviousBtn = 530
kGSAdvPreviousBtnText = 531

kAudioMusicVolumeSliderTag = 300
kAudioAmbienceVolumeSliderTag = 301
kAudioVoiceVolumeSliderTag = 303
kAudioNPCVoiceSlider = 302
kAudioSoundEffectsVolumeSliderTag = 304
kAudioNumberOfSoundsSliderTag = 305
kAudioMuteCheckBoxTag = 306

kAudioModeID = 751
kAudioModeCBID01 = 752
kAudioModeCBID02 = 753
kAudioModeCBID03 = 754
kAudioModeTextID = 755
kAudioModeEAXTextID = 757

kGSEnableVoiceChat=340

kGSEnableSubtitles=341

####==> GUI dialog sounds will just match the SoundFX levels
##kGSGUIVolSlider=305
####==> GUI dialog sounds will just match the SoundFX levels
kGSMicLevelSlider=400

kGSDisplayGammaSlider = 450
kGSDisplayShadowDistSlider = 452
kVideoShadowsCheckTag = 454
kVideoAntiAliasingSliderTag = 451
kVideoWindowedCheckTag = 474
kVideoFilteringSliderTag = 455
kVideoQualitySliderTag = 457
kVideoWindowedTextTag = 475
kVideoResTextHeaderTag = 476
kVideoTextureQualitySliderTag = 458
kVideoShadowQualitySliderTag = 459
kVideoResSliderTag = 461
kVideoResTextTag = 473
kVideoVerticalSyncCheckTag = 453
kVideoDynamicReflectionsCheckTag = 900
kVideoDynamicReflectionsTextTag = 901

kGSAudioMuteCheckbox = 456
kGSMouseTurnSensSlider = 460
kGSSoundPrioritySlider = 462
kGSPukeCamCheckbox = 464
kGSMouseInvertCheckbox = 466
kGSWalkAndPan = 468
kGSStayInFirstPerson = 470
kGSClickToTurn = 472

kGSVoiceHeader = 615
kGSMyVoiceHeader = 616

# - string IDs that need to be localized
kGSVolumeHeader = 700
kGSVolSoundFXText = 701
kGSVolMuteText = 702
kGSVolMusicText = 703
kGSVolAmbientText = 704
kGSDisplayHeader = 720
kGSDispGamaText = 721
kGSDispShadowsText = 722
kGSAdvancedBtnText = 830

# - string IDs for the Advanced Settings localization
kAGSAdvanceHeader = 700
kAGSShadowQualityText = 701
kAGSQuickerCameraText = 702
kAGSMouseTurn = 703
kAGSSoundPriority = 704
kAGSMouseInvert = 705
kAGSWalkAndPan = 706
kAGSStayInFirstPerson = 707
kAGSClickToTurn = 708

# ====================================
# Yes/No dialog globals
#----Controls
kYesNoTextID=12
kYesButtonID=10
kNoButtonID=11
kYesButtonTextID = 60
kNoButtonTextID = 61

# ========================
# Calibration dialog IDs
kCalMessageText = 700

# ========================
# Reset Warning Dialog
kResetWarningYes = 550
kResetWarningNo = 551


#----------------- trailer
kOptionFadeOutID = 2
kOptionFadeOutSeconds = 1.0
kTrailerInSeconds = 1.0
kTrailerFadeInID = 5

kTrailerFadeOutID = 3
kTrailerFadeOutSeconds = 1.0
kOptionFadeInSeconds = 0.5

gOriginalAmbientVolume = 1.0
gOriginalSFXVolume = 1.0
gOriginalMusicVolume = 1.0

gMouseSensitivity = "150"
gSmoothCam = "0"
gMouseInvert = "0"
gWalkAndPan = "0"
gStayInFirstPerson = "0"
gClickToTurn = "0"
gAudioHack = 0
gCurrentReleaseNotes = "Welcome to Myst Online: Uru Live!"

class xOptionsMenu(ptModifier):
    "The Options dialog modifier"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 195
        self.version = MaxVersionNumber
        PtDebugPrint("__xOptionsMenu: Max version %d - minor version %d" % (MaxVersionNumber,MinorVersionNumber))
        
        self.restartWarn = False
        self.goingToCalibration = 0
        self.refreshBindings = True

    def OnFirstUpdate(self):
        global WebLaunchCmd
        "First update, load our dialogs"
        PtLoadDialog("OptionsMenuGUI",self.key)
        PtLoadDialog("KeyMapDialog",self.key)
        PtLoadDialog("GameSettingsDialog",self.key)
        PtLoadDialog("CalibrationGUI",self.key)
        PtLoadDialog("TrailerPreviewGUI",self.key)
        PtLoadDialog("AdvancedGameSettingsDialog",self.key)
        PtLoadDialog("NavigationSettingsDialog",self.key)
        PtLoadDialog("GraphicsSettingsDialog",self.key)
        PtLoadDialog("AudioSettingsDialog",self.key)
        PtLoadDialog("OptionsMenuRestart",self.key)
        PtLoadDialog("ReleaseNotesDialog",self.key)

    def __del__(self):
        "the destructor - unload any dialogs we loaded"
        PtUnloadDialog("KeyMapDialog")
        PtUnloadDialog("GameSettingsDialog")
        PtUnloadDialog("CalibrationGUI")
        PtUnloadDialog("TrailerPreviewGUI")
        PtUnloadDialog("AdvancedGameSettingsDialog")
        PtUnloadDialog("NavigationSettingsDialog")
        PtUnloadDialog("GraphicsSettingsDialog")
        PtUnloadDialog("AudioSettingsDialog")
        PtUnloadDialog("OptionsMenuRestart")
        PtUnloadDialog("ReleaseNotesDialog")

    def BeginAgeUnLoad(self,avatar):
        "When the current age is being unloaded, ie. we've linked out - hide all modal dialogs!"
        global gJournalBook
        global gPreviewStarted
        if OptionsMenuDlg.dialog.isEnabled():
            OptionsMenuDlg.dialog.hide()
        if KeyMapDlg.dialog.isEnabled():
            KeyMapDlg.dialog.hide()
        if CalibrateDlg.dialog.isEnabled():
            CalibrateDlg.dialog.hide()
        if gPreviewStarted:
            PtFadeOut(kOptionFadeOutSeconds,1)
            PtAtTimeCallback(self.key, kOptionFadeOutSeconds, kTrailerFadeOutID)
        if AdvGameSettingDlg.dialog.isEnabled():
            AdvGameSettingDlg.dialog.hide()
        if NavigationDlg.dialog.isEnabled():
            NavigationDlg.dialog.hide()
        if ReleaseNotesDlg.dialog.isEnabled():
            ReleaseNotesDlg.dialog.hide()
        if gJournalBook:
            gJournalBook.hide()

    def OnServerInitComplete(self):
        if self.refreshBindings:
            self.refreshBindings = False

            self.LoadAdvSettings()
            self.LoadKeyMap()
            GammaVal = self.getChronicleVar("gamma")
            if GammaVal == None:
                PtSetGamma2(0)
            else:
                PtAtTimeCallback (self.key, 5, 999)

    def OnAccountUpdate(self, notify, result, playerID):
        # If a new player is set, we'll want to reload the key bindings from their vault.
        if notify == PtAccountUpdateType.kActivePlayer and playerID:
            self.refreshBindings = True

    def OnNotify(self,state,id,events):
        "Notify - should only be needed for credits book"
        PtDebugPrint("xOptionsMenu: Notify  state=%f, id=%d" % (state,id),level=kDebugDumpLevel)

        if id == -1:
            PtDebugPrint("Options Menu got notify, resetting First Visit status")
            self.refreshBindings = True
            return

        # is it a notification from the scene input interface or PlayerBook?
        for event in events:
            # is it from the Credits book?
            if event[0] == PtEventType.kBook:
                PtDebugPrint("xOptionsMenu: BookNotify  event=%d, id=%d" % (event[1],event[2]),level=kDebugDumpLevel)
                if event[1] == PtBookEventTypes.kNotifyImageLink:
                    PtDebugPrint("xOptionsMenu:Book: hit linking panel",level=kDebugDumpLevel)
                    pass
                elif event[1] == PtBookEventTypes.kNotifyShow:
                    PtDebugPrint("xOptionsMenu:Book: Notify Show",level=kDebugDumpLevel)
                    pass
                elif event[1] == PtBookEventTypes.kNotifyHide:
                    PtDebugPrint("xOptionsMenu:Book: NotifyHide",level=kDebugDumpLevel)
                    pass
                elif event[1] == PtBookEventTypes.kNotifyNextPage:
                    PtDebugPrint("xOptionsMenu:Book: NotifyNextPage",level=kDebugDumpLevel)
                    pass
                elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                    PtDebugPrint("xOptionsMenu:Book: NotifyPreviousPage",level=kDebugDumpLevel)
                    pass
                elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                    PtDebugPrint("xOptionsMenu:Book: NotifyCheckUncheck",level=kDebugDumpLevel)
                    pass
                # nothing more to do, when there is a book event
                return


    def OnGUINotify(self,id,control,event):
        "Events from all the dialogs in the Option Age..."
        global WebLaunchCmd
        global CCRHelpDialogType
        global gJournalBook
        global gPreviewStarted
        global prevAudioDeviceName
        PtDebugPrint("xOptionsMenu::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel)
###############################################
##
##  OptionsMenu dialog processing
##
###############################################
        if id == OptionsMenuDlg.id:
            if event == kShowHide:
                pass

            elif event == kAction or event == kValueChanged:
                # test to see which control had the event
                omID = control.getTagID()
                if omID == kOptionsNavigationBtn:
                    OptionsMenuDlg.dialog.hide()
                    NavigationDlg.dialog.show()
                    PtDebugPrint("The Navigation dialog should show now...")
                elif omID == kOptionsGraphicSettingsBtn:
                    OptionsMenuDlg.dialog.hide()
                    GraphicsSettingsDlg.dialog.show()
                    PtDebugPrint("The graphics dialog should show now...")
                elif omID == kOptionsAudioSettingsBtn:
                    self.InitAudioControlsGUI()
                    OptionsMenuDlg.dialog.hide()
                    AudioSettingsDlg.dialog.show()
                    PtDebugPrint("The audio dialog should show now...")
                elif omID == kOptionsKeyMapBtn:
                    OptionsMenuDlg.dialog.hide()
                    KeyMapDlg.dialog.show()
                elif omID == kOptionsTrailerBtn:
                    gPreviewStarted = 1
                    OptionsMenuDlg.dialog.hide()
                    PtFadeOut(kOptionFadeOutSeconds,1)
                    PtAtTimeCallback(self.key, kOptionFadeOutSeconds, kOptionFadeOutID)
                elif omID == kOptionsReleaseNotes:
                    OptionsMenuDlg.dialog.hide()
                    ReleaseNotesDlg.dialog.show()
                elif omID == kOptionsCreditsBtn:
                    OptionsMenuDlg.dialog.hide()
                    startOpened = 0
                    params = xJournalBookDefs.xJournalBooks["UruCredits"]
                    if len(params) == 4:
                        width,height,locPath,gui = params
                    else:
                        width,height,locPath = params
                        gui = "BkBook"
                    journalContents = PtGetLocalizedString(locPath)
                    gJournalBook = ptBook(journalContents,self.key)
                    gJournalBook.setSize(width,height)
                    # make sure there is a cover to show
                    if not startOpened:
                        if not self.IsThereACover(journalContents):
                            startOpened = 1
                    gJournalBook.setGUI(gui)
                    gJournalBook.show(startOpened)
                elif omID == kOptionsCalibrationBtn:
                    # make sure that we get the events (AvaCusta may have stole them)
                    PtLoadDialog("CalibrationGUI",self.key)
                    OptionsMenuDlg.dialog.hide()
                    CalibrateDlg.dialog.show()
                elif omID == kOptionsQuitBtn:
                    OptionsMenuDlg.dialog.hide()
                    PtSendKIMessage(kQuitDialog,0)
                elif omID == kOptionsOkBtn:
                    OptionsMenuDlg.dialog.hide()
            elif event == kExitMode:
                OptionsMenuDlg.dialog.hide()


###############################################
##
##  Release Notes dialog processing
##
###############################################
        elif id == ReleaseNotesDlg.id:
            global gCurrentReleaseNotes

            if event == kDialogLoaded:
                try:
                    with open("ReleaseNotes.txt", "r") as f:
                        gCurrentReleaseNotes = f.read()
                except Exception as e:
                    PtDebugPrint("[TXT processing] Error while reading ReleaseNotes.txt:", e)

                self._releaseNotesCtrl = xGUILinkHandler(ptGUIControlMultiLineEdit(ReleaseNotesDlg.dialog.getControlFromTag(kReleaseTextArea)))
                self._releaseNotesCtrl.lock()
            elif event == kShowHide:
                if control.isEnabled():
                    # BOOM if you do this on dialog load. Probably due to how early it happens
                    # in the init process. Do it now.
                    if not self._releaseNotesCtrl.getBufferSize():
                        self._releaseNotesCtrl.setString(gCurrentReleaseNotes)

                    self._releaseNotesCtrl.setScrollPosition(1)
                    self._releaseNotesCtrl.setScrollPosition(0)
            elif event == kAction or event == kValueChanged:
                rnID = control.getTagID()
                if rnID == kRNOkBtn:
                    ReleaseNotesDlg.dialog.hide()
                elif rnID == kRNGoBackBtn:
                    ReleaseNotesDlg.dialog.hide()
                    OptionsMenuDlg.dialog.show()
                elif rnID == kReleaseTextArea and event == kAction:
                    self._releaseNotesCtrl.openLink()
            elif event == kExitMode:
                ReleaseNotesDlg.dialog.hide()


###############################################
##
##  Key Map dialog processing
##
###############################################
        elif id == KeyMapDlg.id:
            if event == kDialogLoaded:
                pass
            elif event == kShowHide:
                if control.isEnabled():
                    self.IShowMappedKeys(control, gKM1ControlCodesRow1, gKM1ControlCodesRow2)
            elif event == kAction or event == kValueChanged:
                kmID = control.getTagID()
                if kmID == kKMOkBtn:
                    KeyMapDlg.dialog.hide()
                elif kmID in gKM1ControlCodesRow1 or kmID in gKM1ControlCodesRow2:
                    self.ISetKeyMapping(
                        kmID - 300,
                        control.getLastKeyCaptured(),
                        control.getLastModifiersCaptured(),
                        kmID in gKM1ControlCodesRow1
                    )
                    KeyMapDlg.dialog.noFocus()
                    self.IShowMappedKeys(KeyMapDlg.dialog, gKM1ControlCodesRow1, gKM1ControlCodesRow2)
                elif kmID == kKMDefaultsBtn:
                    self.ISetDefaultKeyMappings()
                    self.IShowMappedKeys(KeyMapDlg.dialog,gKM1ControlCodesRow1,gKM1ControlCodesRow2)
                elif kmID == kKMGoBackBtn:
                    KeyMapDlg.dialog.hide()
                    AdvGameSettingDlg.dialog.show()                    
            elif event == kExitMode:
                KeyMapDlg.dialog.hide()

###############################################
##
##  Advanced Game settings dialog processing
##
###############################################

        elif id == AdvGameSettingDlg.id:
            global gMouseSensitivity
            global gSmoothCam
            global gMouseInvert
            global gWalkAndPan
            global gStayInFirstPerson
            global gClickToTurn

            if event == kDialogLoaded:
                PtDebugPrint("Yes, the Advanced Settings dialog loaded.")
            elif event == kShowHide:
                if control.isEnabled():
                    self.IRefreshAdvSettings()
            elif event == kAction or event == kValueChanged:
                gsID = control.getTagID()
                PtDebugPrint("gsID = " + str(gsID))

                if gsID == kOptionsKeyMapBtn:
                    self.setNewChronicleVar("AdvSettings", (gMouseSensitivity + " " + gSmoothCam + " " + gMouseInvert + " " + gWalkAndPan + " " + gStayInFirstPerson + " " + gClickToTurn))
                    AdvGameSettingDlg.dialog.hide()
                    KeyMapDlg.dialog.show()
                elif gsID == kGSOkBtn:
                    self.setNewChronicleVar("AdvSettings", (gMouseSensitivity + " " + gSmoothCam + " " + gMouseInvert + " " + gWalkAndPan + " " + gStayInFirstPerson + " " + gClickToTurn))
                    AdvGameSettingDlg.dialog.hide()
                elif gsID == kGSDefaultsBtn:
                    self.setNewChronicleVar("AdvSettings", ("150 0 0 0 0 0"))
                    gMouseSensitivity = str(int(150))
                    gSmoothCam = "0"
                    gMouseInvert = "0"
                    gWalkAndPan = "0"
                    gStayInFirstPerson = "0"
                    gClickToTurn = "0"
                    for control in [kGSPukeCamCheckbox, kGSMouseInvertCheckbox, kGSWalkAndPan, kGSStayInFirstPerson, kGSClickToTurn]:
                        checkBox = ptGUIControlCheckBox(AdvGameSettingDlg.dialog.getControlFromTag(control))
                        checkBox.setChecked(0)
                    slider = ptGUIControlKnob(AdvGameSettingDlg.dialog.getControlFromTag(kGSMouseTurnSensSlider))
                    slider.setValue(5.0)
                    PtSetMouseTurnSensitivity(150)
                    PtSetMouseUninverted()
                    cam = ptCamera()
                    cam.setSmootherCam(0)
                    cam.setWalkAndVerticalPan(0)
                    cam.setStayInFirstPerson(0)
                    PtSetClickToTurn(0)
                elif gsID == kGSGoBackBtn:
                    self.setNewChronicleVar("AdvSettings", (gMouseSensitivity + " " + gSmoothCam + " " + gMouseInvert + " " + gWalkAndPan + " " + gStayInFirstPerson + " " + gClickToTurn))
                    AdvGameSettingDlg.dialog.hide()
                    NavigationDlg.dialog.show()
                elif gsID == kGSMouseTurnSensSlider:
                    sensitivity = control.getValue()
                    nsensitive = 150.0
                    if sensitivity > 5.0:
                        # 150 - 350
                        nsensitive = ((sensitivity-5.0) * 40.0) + 150.0
                    elif sensitivity == 5.0:
                        nsensitive = 150.0
                    else:
                        # 50 - 150
                        nsensitive = (sensitivity * 20.0)+50.0
                    PtSetMouseTurnSensitivity(nsensitive)
                    gMouseSensitivity = str(int(nsensitive))
                elif gsID == kGSPukeCamCheckbox:
                    cam = ptCamera()
                    if control.isChecked():
                        cam.setSmootherCam(1)
                        gSmoothCam = "1"
                    else:
                        cam.setSmootherCam(0)
                        gSmoothCam = "0"
                elif gsID == kGSMouseInvertCheckbox:
                    if control.isChecked():
                        PtSetMouseInverted()
                        gMouseInvert = "1"
                    else:
                        PtSetMouseUninverted()
                        gMouseInvert = "0"
                elif gsID == kGSWalkAndPan:
                    cam = ptCamera()
                    if control.isChecked():
                        cam.setWalkAndVerticalPan(1)
                        gWalkAndPan = "1"
                    else:
                        cam.setWalkAndVerticalPan(0)
                        gWalkAndPan = "0"
                elif gsID == kGSStayInFirstPerson:
                    cam = ptCamera()
                    if control.isChecked():
                        cam.setStayInFirstPerson(1)
                        gStayInFirstPerson = "1"
                    else:
                        cam.setStayInFirstPerson(0)
                        gStayInFirstPerson = "0"
                elif gsID == kGSClickToTurn:
                    if control.isChecked():
                        PtSetClickToTurn(1)
                        gClickToTurn = "1"
                    else:
                        PtSetClickToTurn(0)
                        gClickToTurn = "0"

            elif event == kExitMode:
                AdvGameSettingDlg.dialog.hide()

###############################################
##
##  Calibration dialog processing
##
###############################################
        elif id == CalibrateDlg.id:
            if event == kAction or event == kValueChanged:
                cbID = control.getTagID()
                if cbID == kClickOnMeBtn:
                    CalibrateDlg.dialog.hide()
                    GraphicsSettingsDlg.dialog.show()
            elif event == kExitMode:
                CalibrateDlg.dialog.hide()

###############################################
##
##  Trailer preview dialog processing
##
###############################################
        elif id == TrailerDlg.id:
            global gLiveMovie
            global gLiveMovieList
            global gCurrentMovieName
            if event == kDialogLoaded:
                pass
            elif event == kShowHide:
                pass
            elif event == kAction or event == kValueChanged:
                tpID = control.getTagID()
                if tpID == kTrailerPreviousButton:
                    if gLiveMovie:
                        gLiveMovie.pause()
                        tempIndex = gLiveMovieList.index(gCurrentMovieName)-1
                        if tempIndex < 0:
                            gCurrentMovieName = gLiveMovieList[len(gLiveMovieList)-1]
                        else:
                            gCurrentMovieName = gLiveMovieList[tempIndex]
                        gLiveMovie = ptMoviePlayer(os.path.join(kLiveMovieDir, gCurrentMovieName),self.key)
                        gLiveMovie.play()
                elif tpID == kTrailerNextButton:
                    if gLiveMovie:
                        gLiveMovie.pause()
                        tempIndex = gLiveMovieList.index(gCurrentMovieName)+1
                        if tempIndex >= len(gLiveMovieList):
                            gCurrentMovieName = gLiveMovieList[0]
                        else:
                            gCurrentMovieName = gLiveMovieList[tempIndex]
                        gLiveMovie = ptMoviePlayer(os.path.join(kLiveMovieDir, gCurrentMovieName),self.key)
                        gLiveMovie.play()
                elif tpID == kClickOnMeBtn:
                    if gLiveMovie:
                        gLiveMovie.pause()
                    gPreviewStarted = 0
                    PtFadeOut(kTrailerFadeOutSeconds,1)
                    PtAtTimeCallback(self.key, kTrailerFadeOutSeconds, kTrailerFadeOutID)
            elif event == kExitMode:
                gPreviewStarted = 0
                PtFadeOut(kTrailerFadeOutSeconds,1)
                PtAtTimeCallback(self.key, kTrailerFadeOutSeconds, kTrailerFadeOutID)

###############################################
##
##  Reset Warning dialog processing
##
###############################################
        elif id == ResetWarnDlg.id:
            if event == kDialogLoaded:
                PtDebugPrint("Yes, the ResetWarn Dialog loaded.")
                pass
            elif event == kShowHide:
                PtDebugPrint("event = kShowHide = ", kShowHide)
                pass                
            elif event == kAction or event == kValueChanged:
                # test to see which control had the event
                warnID = control.getTagID()
                PtDebugPrint("warnID = ",warnID)
                if warnID == kResetWarningYes:
                    PtDebugPrint("I need to shut down Plasma now.")
                    self.WriteVideoControls()
                    PtConsole("App.Quit")
                elif warnID == kResetWarningNo:
                    PtDebugPrint("close the dialog now.")
                    ResetWarnDlg.dialog.hide()
                    if self.goingToCalibration:
                        PtLoadDialog("CalibrationGUI",self.key)
                        CalibrateDlg.dialog.show()
                        self.goingToCalibration = 0
                    else:
                        GraphicsSettingsDlg.dialog.show()                

###############################################
##
##  Navigation dialog processing
##
###############################################
        elif id == NavigationDlg.id:
            #~ PtDebugPrint("navigation event = ", event)
            #~ PtDebugPrint("kShowHide = ",kShowHide, " kAction = ",kAction, " kValueChanged = ", kValueChanged)
            if event == kDialogLoaded:
                PtDebugPrint("Yes, the Navigation dialog loaded.")
                pass

            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    self.IRefreshHelpSettings()

            elif event == kAction or event == kValueChanged:
                NavigationID = control.getTagID()
                PtDebugPrint("NavigationID = ", NavigationID)                
                if NavigationID == kKMOkBtn:
                    NavigationDlg.dialog.hide()
                elif NavigationID == kKMGoBackBtn:
                    NavigationDlg.dialog.hide()
                    OptionsMenuDlg.dialog.show()
                elif NavigationID == kKMNextPreviousBtn:
                    NavigationDlg.dialog.hide()  
                    AdvGameSettingDlg.dialog.show()
                elif NavigationID == kNormNoviceRGID:
                    if control.getValue():
                        PtDebugPrint("CTT on")
                        PtSetClickToTurn(1)
                        gClickToTurn = "1"
                        AdvSettingsString = self.getChronicleVar("AdvSettings")

                        if AdvSettingsString == None:
                            # create settings
                            self.setNewChronicleVar("AdvSettings", "150 0 0 0 0 1")
                        else:
                            self.setNewChronicleVar("AdvSettings", (AdvSettingsString[0:-1] + "1"))
                    else:
                        PtDebugPrint("CTT off")
                        PtSetClickToTurn(0)
                        gClickToTurn = "0"
                        AdvSettingsString = self.getChronicleVar("AdvSettings")

                        if AdvSettingsString == None:
                            # create settings
                            self.setNewChronicleVar("AdvSettings", "150 0 0 0 0 0")
                        else:
                            self.setNewChronicleVar("AdvSettings", (AdvSettingsString[0:-1] + "0"))

###############################################
##
##  GraphicsSettings dialog processing
##
###############################################
        elif id == GraphicsSettingsDlg.id:
            if event == kShowHide:
                if control.isEnabled():
                    self.InitVideoControlsGUI()
                    self.restartWarn = 0
                    
            elif (event == kAction or event == kValueChanged):
                if control:
                    tagID = control.getTagID()
                else:
                    tagID = 0
                    
                if tagID in [kKMOkBtn, kKMGoBackBtn, kOptionsCalibrationBtn]:
                    PtDebugPrint("self.restartWarn =", self.restartWarn)
                    if self.restartWarn:
                        GraphicsSettingsDlg.dialog.hide()
                        ResetWarnDlg.dialog.show()
                        if tagID in [kKMOkBtn, kKMGoBackBtn]:
                            self.restartWarn = 0
                    else:
                        self.WriteVideoControls(1)
                
                if tagID == kKMOkBtn:
                    GraphicsSettingsDlg.dialog.hide()

                elif tagID == kKMGoBackBtn:
                    GraphicsSettingsDlg.dialog.hide()
                    OptionsMenuDlg.dialog.show()

                elif tagID == kOptionsCalibrationBtn:
                    if self.restartWarn:
                        self.goingToCalibration = 1
                        self.restartWarn = 0
                    else:
                        # make sure that we get the events (ACA may have stole them)
                        PtLoadDialog("CalibrationGUI",self.key)
                        GraphicsSettingsDlg.dialog.hide()
                        CalibrateDlg.dialog.show()

                elif tagID == kGSDefaultsBtn:
                    self.restartWarn = 1
                    self.ResetVideoToDefault()

                elif tagID == kVideoResSliderTag:
                    videoText = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))
                    videoSlider = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResSliderTag))
                    curText = videoText.getString()
                    vidResList = self.GetVideoResList()
                    resSlider = videoSlider.getValue() * (len(vidResList) - 1)
                    curSelection = int(round(resSlider))
                    if len(vidResList) == 1:
                        videoSlider.setValue(0)
                        videoSlider.disable()
                        respDisableItems.run(self.key, state="disableRes")
                        videoText.setString("800x600")
                        videoText.setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                        ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextHeaderTag)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                    else:
                        control.setValue(float(curSelection) / (len(vidResList) - 1))
                        newValue = vidResList[curSelection]
                        self.SetVidResField(newValue)

                elif tagID == kVideoQualitySliderTag or tagID == kVideoTextureQualitySliderTag:
                    self.restartWarn = 1
                    curVal = round(control.getValue())
                    control.setValue(int(curVal))

                elif tagID == kVideoWindowedCheckTag:
                    videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResSliderTag))
                    if not control.isChecked():
                        ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResSliderTag)).enable()
                        respDisableItems.run(self.key, state="enableRes")
                        ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag)).setForeColor(ptColor(1, 1, 1, 1))
                        ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextHeaderTag)).setForeColor(ptColor(1, 1, 1, 1))
                        respDisableItems.run(self.key, state="enableGamma")
                        ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDisplayGammaSlider)).enable()
                        ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDispGamaText)).setForeColor(ptColor(1, 1, 1, 1))
                    else:
                        respDisableItems.run(self.key, state="disableGamma")
                        ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDisplayGammaSlider)).disable()
                        ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDispGamaText)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                    
                    vidResList = self.GetVideoResList()
                    vidRes = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))
                    if not self.GetVidResField() in vidResList:
                        vidRes.setString("800x600 [4:3]")
                        videoField.setValue(0.0)
                    
                    numRes = len(vidResList)
                    if numRes == 1:
                        videoField.setValue(0)
                        videoField.disable()
                        respDisableItems.run(self.key, state="disableRes")
                        vidRes.setString("800x600 [4:3]")
                        vidRes.setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                        ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextHeaderTag)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                    else:
                        for res in range(numRes):
                            if self.GetVidResField() == vidResList[res]:
                                videoField.setValue(float(res) / float(numRes - 1))
                                break

                elif tagID == kVideoAntiAliasingSliderTag or tagID == kVideoFilteringSliderTag:
                    curVal = round(control.getValue())
                    control.setValue(int(curVal))

                elif tagID == kGSDisplayGammaSlider:
                    gamma = control.getValue()
                    PtSetGamma2(gamma)

###############################################
##
##  AudioSettings dialog processing
##
###############################################
                    
        elif id == AudioSettingsDlg.id:
            if event == kDialogLoaded:
                PtDebugPrint("Yes, the Audio dialog loaded.")
                
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    self.restartAudio = 0

                else:
                    if self.restartAudio:
                        audio = ptAudioControl()
                        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeID))
                        audModeNum = len(gAudioDevices) - 1
                        curSelection = round(audioField.getValue() * audModeNum) 
                        intCurSelection = int(curSelection)

                        #Enable EAX Support
                        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))
                        audio.useEAXAcceleration(EAXcheckbox.isChecked())

                        audio.setPlaybackDevice(gAudioDevices[intCurSelection], 1)
                    self.WriteAudioControls()

            elif event == kAction or event == kValueChanged:
                if control:
                    tagID = control.getTagID()
                else:
                    tagID = 0

                if tagID == kKMOkBtn:
                    AudioSettingsDlg.dialog.hide()

                elif tagID == kKMGoBackBtn:
                    AudioSettingsDlg.dialog.hide()
                    OptionsMenuDlg.dialog.show()

                if tagID == kAudioSoundEffectsVolumeSliderTag:
                    audio = ptAudioControl()
                    audio.setSoundFXVolume( control.getValue() )
                    audio.setGUIVolume( control.getValue() )

                elif tagID == kAudioNPCVoiceSlider:
                    audio = ptAudioControl()
                    audio.setNPCVoiceVolume( control.getValue() )

                elif tagID == kGSMicLevelSlider:
                    audio = ptAudioControl()
                    audio.setMicLevel( control.getValue() )

                elif tagID == kGSEnableVoiceChat:
                    audio = ptAudioControl()
                    audio.enableVoiceRecording( control.isChecked() )
                    audio.enableVoiceChat( control.isChecked() )

                elif tagID == kGSEnableSubtitles:
                    audio = ptAudioControl()
                    if control.isChecked():
                        audio.enableSubtitles()
                    else:
                        audio.disableSubtitles()

                elif tagID == kAudioMusicVolumeSliderTag:
                    audio = ptAudioControl()
                    audio.setMusicVolume( control.getValue() )

                elif tagID == kAudioAmbienceVolumeSliderTag:
                    audio = ptAudioControl()
                    audio.setAmbienceVolume( control.getValue() )

                elif tagID == kAudioNumberOfSoundsSliderTag:
                    audio = ptAudioControl()
                    audio.setPriorityCutoff( int(control.getValue()) )
                    control.setValue(int(control.getValue()))

                elif tagID == kAudioMuteCheckBoxTag:
                    audio = ptAudioControl()
                    if control.isChecked():
                        audio.muteAll()
                    else:
                        audio.unmuteAll()

                elif tagID == kAudioModeID:
                    self.restartAudio = 1
                    audio = ptAudioControl()
                    #~ PtDebugPrint("Number of Audio Devices: %d" % (audio.getNumAudioDevices()))
                    audModeNum = len(gAudioDevices)  - 1
                    curSelection = round(control.getValue() * audModeNum)
                    intCurSelection = int(curSelection)
                    control.setValue(curSelection/audModeNum)

                    audioDeviceName = gAudioDevices[intCurSelection]

                    audioModeCtrlTextBox = ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeTextID))
                    audioModeCtrlTextBox.setString(audio.getFriendlyDeviceName(audioDeviceName))

                    if audioDeviceName != prevAudioDeviceName:  #Only update the EAX checkbox when the mouse has been let up...
                        PtDebugPrint("Audio Device Name changed!")
                        prevAudioDeviceName = audioDeviceName
                        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))
                        if not audio.isEAXSupported():
                            PtDebugPrint("Disabling EAX checkbox")
                            #Disable EAX checkbox
                            EAXcheckbox.disable()
                            respDisableItems.run(self.key, state="disableEAX")
                            EAXcheckbox.setChecked(False)
                            ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeEAXTextID)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                        else:
                            PtDebugPrint("Enabling EAX checkbox")
                            #We don't need to automatically check the EAX box, but do enable the control
                            EAXcheckbox.enable()
                            respDisableItems.run(self.key, state="enableEAX")
                            EAXcheckbox.setChecked(audio.isUsingEAXAcceleration())
                            ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeEAXTextID)).setForeColor(ptColor(1, 1, 1, 1))

                elif tagID == kAudioModeCBID03:
                    self.restartAudio = 1
                    pass

    def OnTimer(self,id):
        global gLiveMovie
        global gLiveMovieList
        global gCurrentMovieName
        global gOriginalSFXVolume
        global gOriginalAmbientVolume
        global gOriginalMusicVolume
        global gPreviewStarted
        if id == kOptionFadeOutID:
            # is the movie there?
            try:
                if PtIsDemoMode():
                    os.stat(kDemoMovieName)
                else:
                    gLiveMovieList = []
                    for file in os.listdir(kLiveMovieDir):
                        tempfile = os.path.join(kLiveMovieDir, file)
                        if os.stat(tempfile).st_size > 1000 and tempfile.endswith(".webm"):
                            gLiveMovieList.append(file)
                    else:
                        if gLiveMovieList == []:
                            raise TypeError('No webm found')
                # its there! show the background, which will start the movie
                # just continue processing
            except:
                # no movie... just go back to where they were
                PtDebugPrint("xLiveTrailer - no intro movie!!!",level=kDebugDumpLevel)
                PtFadeOut(kTrailerFadeOutSeconds,1)
                PtFadeIn(kOptionFadeInSeconds,0)
                return
            PtDebugPrint("xLiveTrailer - start showing movie",level=kDebugDumpLevel)
            OptionsMenuDlg.dialog.hide()
            PtShowDialog("IntroBahroBgGUI")
            TrailerDlg.dialog.show()
            # stop rendering the scene while showing the movie
            PtDisableRenderScene()
            # dim the cursor
            PtGUICursorDimmed()
            # temp mute background sound
            audio = ptAudioControl()
            gOriginalAmbientVolume = audio.getAmbienceVolume()
            audio.setAmbienceVolume(0.0)
            gOriginalSFXVolume = audio.getSoundFXVolume()
            audio.setSoundFXVolume(0.0)
            gOriginalMusicVolume = audio.getMusicVolume()
            audio.setMusicVolume(0.0)
            PtFadeIn(kTrailerInSeconds,0)
            PtAtTimeCallback(self.key, kTrailerInSeconds, kTrailerFadeInID)
            if PtIsDemoMode():
                gLiveMovie = ptMoviePlayer(kDemoMovieName,self.key)
            else:
                gCurrentMovieName = gLiveMovieList[0]
                gLiveMovie = ptMoviePlayer(os.path.join(kLiveMovieDir, gCurrentMovieName),self.key)
            gLiveMovie.playPaused()
        elif id == kTrailerFadeInID:
            PtDebugPrint("xLiveTrailer - roll the movie",level=kDebugDumpLevel)
            if gLiveMovie is not None:
                gLiveMovie.resume()
            #gLiveMovie.play()
        elif id == kTrailerFadeOutID:
            PtDebugPrint("xLiveTrailer - done",level=kDebugDumpLevel)
            gPreviewStarted = 0
            TrailerDlg.dialog.hide()
            PtHideDialog("IntroBahroBgGUI")
            if gLiveMovie:
                PtDebugPrint("xLiveTrailer - but stop the movie first",level=kDebugDumpLevel)
                gLiveMovie.stop()
                gLiveMovie = None
            # start rendering the scene again
            PtEnableRenderScene()
            # cursor back on
            PtGUICursorOn()
            # enable background sounds again
            audio = ptAudioControl()
            audio.setAmbienceVolume(gOriginalAmbientVolume)
            audio.setSoundFXVolume(gOriginalSFXVolume)
            audio.setMusicVolume(gOriginalMusicVolume)
            OptionsMenuDlg.dialog.show()
            PtFadeIn(kOptionFadeInSeconds,0)
        elif id == 999:
            GammaVal = self.getChronicleVar("gamma")
            PtSetGamma2(float(GammaVal))

    def OnMovieEvent(self,movieName,reason):
        PtDebugPrint("xLiveTrailer: got movie done event on %s, reason=%d" % (movieName,reason),level=kDebugDumpLevel)
        if gLiveMovie and movieName == os.path.join(kLiveMovieDir, gCurrentMovieName):
            PtFadeOut(kTrailerFadeOutSeconds,1)
            PtAtTimeCallback(self.key, kTrailerFadeOutSeconds, kTrailerFadeOutID)

    def ResetVideoToDefault(self):
        defaults = PtGetDefaultDisplayParams()

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoQualitySliderTag))
        videoField.setValue(defaults[7])

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoTextureQualitySliderTag))
        videoField.setValue(defaults[6])

        shadows = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoShadowsCheckTag))
        shadows.setChecked(defaults[8])

        windowed = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedCheckTag))
        windowed.setChecked(defaults[2])

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoFilteringSliderTag))
        videoField.setValue(kVideoAnisoFiltering[str(defaults[5])])

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoShadowQualitySliderTag))
        videoField.setValue(defaults[3])

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoAntiAliasingSliderTag))
        videoField.setValue(kVideoAntiAliasing[str(defaults[4])])

        videoField = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoVerticalSyncCheckTag))
        videoField.setChecked(defaults[9])

        vidRes =  str(defaults[0]) + "x" + str(defaults[1])
        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResSliderTag))

        vidResList = self.GetVideoResList()
        numRes = len(vidResList)

        gammaField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDisplayGammaSlider))
        gammaField.setValue(0.5)

        if not vidRes in vidResList:
            vidRes = vidResList[numRes-1]
        
        for res in range(numRes):
            if vidRes == vidResList[res]:
                if numRes > 1:
                    videoField.setValue( float(res) / (numRes - 1))
                else:
                    videoField.setValue( 0 )
        self.SetVidResField(vidRes)

    def InitVideoControlsGUI(self):
        xIniDisplay.ReadIni()
        opts = xIniDisplay.GetGraphicsOptions()

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoQualitySliderTag))
        videoField.setValue(int(opts[xIniDisplay.kGraphicsQualityLevel]))

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoTextureQualitySliderTag))
        videoField.setValue(int(opts[xIniDisplay.kGraphicsTextureQuality]))

        shadows = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoShadowsCheckTag))
        shadowsOpt = opts[xIniDisplay.kGraphicsShadows]
        if shadowsOpt == 1 or (isinstance(shadowsOpt, str) and shadowsOpt.casefold() == "true"):
            shadows.setChecked(1)
        else:
            shadows.setChecked(0)

        desktopWidth = PtGetDesktopWidth()
        desktopHeight = PtGetDesktopHeight()
        if desktopWidth == 800 and desktopHeight == 600:
            ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedCheckTag)).disable()
            respDisableItems.run(self.key, state="disableWindow")
            ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedTextTag)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
        else:
            windowed = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedCheckTag))
            dowindow = (opts[xIniDisplay.kGraphicsWindowed] == "true")
            windowed.setChecked(dowindow)

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoFilteringSliderTag))
        videoField.setValue( kVideoAnisoFiltering[str(opts[xIniDisplay.kGraphicsAnisotropicLevel])] )

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoAntiAliasingSliderTag))
        videoField.setValue( kVideoAntiAliasing[str(opts[xIniDisplay.kGraphicsAntiAliasLevel])] )

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoShadowQualitySliderTag))
        videoField.setValue( float(opts[xIniDisplay.kGraphicsShadowQuality]) )

        videoField = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoVerticalSyncCheckTag))
        if opts[xIniDisplay.kGraphicsVerticalSync] == "true":
            videoField.setChecked(1)
        else:
            videoField.setChecked(0)

        dynReflCB = GraphicsSettingsDlg.dialog.getControlModFromTag(kVideoDynamicReflectionsCheckTag)
        dynReflTB = GraphicsSettingsDlg.dialog.getControlModFromTag(kVideoDynamicReflectionsTextTag)
        if PtSupportsPlanarReflections():
            respDisableItems.run(self.key, state="enableDynRefl")
            dynReflCB.setChecked(bool(opts[xIniDisplay.kGraphicsDynamicReflections]))
            dynReflCB.enable()
            dynReflTB.setForeColor(ptColor().white())
        else:
            respDisableItems.run(self.key, state="disableDynRefl")
            dynReflCB.setChecked(False)
            dynReflCB.disable()
            dynReflTB.setForeColor(ptColor(0.839, 0.785, 0.695, 1))

        # video res stuff
        vidRes = str(opts[xIniDisplay.kGraphicsWidth]) + "x" + str(opts[xIniDisplay.kGraphicsHeight])
        videoResField = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResSliderTag))

        vidResList = self.GetVideoResList()
        numRes = len(vidResList)

        if not vidRes in vidResList:
            vidRes = vidResList[numRes-1]

        for res in range(numRes):
            if vidRes == vidResList[res]:
                if numRes > 1:
                    videoField.setValue( float(res) / (numRes - 1))
                else:
                    videoField.setValue( 0 )
                    videoField.disable()
                    respDisableItems.run(self.key, state="disableRes")
                    videoResField.setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                    ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextHeaderTag)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
        self.SetVidResField(vidRes)

        gammaField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDisplayGammaSlider))
        GammaVal = self.getChronicleVar("gamma")
        if ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedCheckTag)).isChecked():
            respDisableItems.run(self.key, state="disableGamma")
            gammaField.disable()
            ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDispGamaText)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
        else:
            respDisableItems.run(self.key, state="enableGamma")
            gammaField.enable()
            ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDispGamaText)).setForeColor(ptColor(1, 1, 1, 1))
            if GammaVal == None:
                gammaField.setValue( 0 )
            else:
                gammaField.setValue( float(GammaVal) )

    def _AspectRatio(self, w, h):
        """Returns the appropriate aspect ratio for the given resolution"""
        # Compares between a range of acceptable aspect ratios due to liberal decisions and marketing
        # done by both GPU vendors and marketing departments. CWalther reported initially that 1280x854
        # is not mathematically 3/2. Ultrawide monitors being marketed as "21:9" despite their being
        # 2.35∶1 in some cases (or some other unknown ratio) makes this even more annoying...
        aspect = float(w) / float(h)
        for ratio, lower, upper in _ratios:
            if upper >= aspect>= lower:
                return " [{}]".format(ratio)
        return ""
    
    def GetVidResField(self):
        videoResField = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))
        value = videoResField.getString().split(' ')
        return value[0]
    
    def SetVidResField(self, value):
        videoResField = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))
        w, h = value.split("x")
        label = value + self._AspectRatio(w, h)
        videoResField.setString(label)

    def WriteVideoControls(self, setMode = 0):
        videoField = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))
        width, height = videoField.getString().split("x")
        try:
            height, trash = height.split(" ")
        except ValueError:
            # there was no trash after height, so eat the exception
            pass
        width = int(width)
        height = int(height)

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoQualitySliderTag))
        quality = int(videoField.getValue())

        colordepth = 32

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoTextureQualitySliderTag))
        tex_quality = int(videoField.getValue())

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoShadowQualitySliderTag))
        shadow_quality = videoField.getValue()

        videoField = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedCheckTag))
        if videoField.isChecked():
            windowed = "true"
        else:
            windowed = "false"

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoAntiAliasingSliderTag))
        aaVal = int(videoField.getValue())
        antialias = 0
        for key in kVideoAntiAliasing.keys():
            if kVideoAntiAliasing[key] == aaVal:
                antialias = int(key)
        
        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoFilteringSliderTag))
        afVal = int(videoField.getValue())
        aniso = 0
        for key in kVideoAnisoFiltering.keys():
            if kVideoAnisoFiltering[key] == afVal:
                aniso = int(key)
                break

        shadows = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoShadowsCheckTag)).isChecked()
        shadowsstr = "true" if shadows else "false"

        vsync = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoVerticalSyncCheckTag)).isChecked()
        if vsync:
            vsyncstr = "true"
        else:
            vsyncstr = "false"

        gammaField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDisplayGammaSlider))
        gamma = gammaField.getValue()

        dynRefl = int(GraphicsSettingsDlg.dialog.getControlModFromTag(kVideoDynamicReflectionsCheckTag).isChecked())

        xIniDisplay.SetGraphicsOptions(width, height, colordepth, windowed, tex_quality, antialias, aniso, quality, shadowsstr, vsyncstr, shadow_quality, dynRefl)
        xIniDisplay.WriteIni()
        self.setNewChronicleVar("gamma", gamma)

        if setMode:
            PtSetGraphicsOptions(width, height, colordepth, windowed == "true", antialias, aniso, vsync)
            PtDebugPrint("SETTING GAMMA")
            PtSetGamma2(gamma)
            PtSetShadowVisDistance(shadow_quality)
            PtEnablePlanarReflections(dynRefl)

            if shadows:
                PtEnableShadows()
            else:
                PtDisableShadows()

    def GetVideoResList(self):
        windowed = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedCheckTag)).isChecked()

        vidResList = []
        for i in PtGetSupportedDisplayModes():
            if i[0] < 800 or i[1] < 600:
                continue
            if windowed and (i[0] >= PtGetDesktopWidth() and i[1] >= PtGetDesktopHeight()):
                continue
            vidResList.append("%ix%i" % (i[0], i[1]))
        vidResList.sort(key=functools.cmp_to_key(res_comp))
        return vidResList

    def WriteAudioControls(self):
        audio = ptAudioControl()

        xIniAudio.SetSoundPriority( audio.getPriorityCutoff() )
        xIniAudio.SetSoundFXVolume( audio.getSoundFXVolume() )
        xIniAudio.SetGUIVolume( audio.getSoundFXVolume() )
        xIniAudio.SetMusicVolume( audio.getMusicVolume() )
        xIniAudio.SetAmbienceVolume( audio.getAmbienceVolume() )
        xIniAudio.SetNPCVoiceVolume( audio.getNPCVoiceVolume() )
        xIniAudio.SetMute( audio.isMuted() )
        xIniAudio.SetSubtitle( audio.areSubtitlesEnabled() )

        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))

        xIniAudio.SetAudioMode( True, audio.getPlaybackDevice(), EAXcheckbox.isChecked() )
        #xIniAudio.SetAudioMode( audio.isEnabled(), audio.getDeviceName(), EAXcheckbox.isChecked() )
        #xIniAudio.SetAudioMode( audio.isEnabled(), audio.getDeviceName(), audio.isUsingEAXAcceleration() )
        #xIniAudio.SetMicLevel( audio.getMicLevel() )
        xIniAudio.SetVoiceRecording( audio.isVoiceRecordingEnabled() )
        
        xIniAudio.WriteIni()

    def InitAudioControlsGUI(self):
        global gAudioDevices
        global prevAudioDeviceName
        
        xIniAudio.ReadIni()
        audio = ptAudioControl()

        gAudioDevices = audio.getPlaybackDevices()

        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioNumberOfSoundsSliderTag))
        audioField.setValue( audio.getPriorityCutoff() )
    
        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioSoundEffectsVolumeSliderTag))
        audioField.setValue( audio.getSoundFXVolume() )

        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioMusicVolumeSliderTag))
        audioField.setValue( audio.getMusicVolume() )

        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioAmbienceVolumeSliderTag))
        audioField.setValue( audio.getAmbienceVolume() )

        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioNPCVoiceSlider))
        audioField.setValue( audio.getNPCVoiceVolume() )

        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kGSMicLevelSlider))
        audioField.setValue( audio.getMicLevel() )

        audioField = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kGSEnableVoiceChat))
        audioField.setChecked( audio.isVoiceRecordingEnabled() )

        audioField = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kGSEnableSubtitles))
        audioField.setChecked( audio.areSubtitlesEnabled() )

        audioField = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioMuteCheckBoxTag))
        audioField.setChecked( audio.isMuted() )

        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))
        EAXcheckbox.setChecked( audio.isUsingEAXAcceleration() )
        EAXcheckbox.enable()
        respDisableItems.run(self.key, state="enableEAX")

        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeID))
        numAudioDevices = len(gAudioDevices) - 1.0

        if numAudioDevices > 0:
            for num, device in enumerate(gAudioDevices):
                if gAudioDevices[num] == audio.getPlaybackDevice():
                    if not audio.isEAXSupported():
                        EAXcheckbox.disable()
                        respDisableItems.run(self.key, state="disableEAX")
                        EAXcheckbox.setChecked(False)
                        ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeEAXTextID)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))

                    audioField.setValue(num/numAudioDevices)
                    audioModeCtrlTextBox = ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeTextID))
                    audioDeviceName = prevAudioDeviceName = audio.getPlaybackDevice()
                    audioModeCtrlTextBox.setString(audio.getFriendlyDeviceName(device))
        else:
            EAXcheckbox.disable()
            respDisableItems.run(self.key, state="disableEAX")
            EAXcheckbox.setChecked(False)
            ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeEAXTextID)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
            audioField.disable()
            audioModeCtrlTextBox = ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeTextID))
            audioModeCtrlTextBox.setString("None")
            audioModeCtrlTextBox.setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                
    def setNewChronicleVar(self, chronicleVar, value):
        kModuleName = "Personal"
        kChronicleVarType = 0
        vault = ptVault()
        entry = vault.findChronicleEntry(chronicleVar)
        if entry is None:
            # not found... add current level chronicle
            vault.addChronicleEntry(chronicleVar,kChronicleVarType,str(value))
            PtDebugPrint("%s:\tentered new chronicle counter %s" % (kModuleName,chronicleVar))
        else:
            entry.setValue(str(value))
            entry.save()
            PtDebugPrint("%s:\tyour current value for %s is %s" % (kModuleName,chronicleVar,entry.getValue()))

    def getChronicleVar(self, chronicleVar):
        kModuleName = "Personal"
        kChronicleVarType = 0
        vault = ptVault()
        entry = vault.findChronicleEntry(chronicleVar)
        PtDebugPrint("getChronicleVar.chronicleVar: " + chronicleVar)
        #PtDebugPrint("getChronicleVar.Entry: " , entry)
        if entry is None:
            # not found... add current level chronicle
            #vault.addChronicleEntry(chronicleVar,kChronicleVarType,"%d" %(0))
            #PtDebugPrint("%s:\tentered new chronicle counter %s" % (kModuleName,chronicleVar))
            return None
        else:
            value = entry.getValue()
            PtDebugPrint("getChronicleVar(): " + chronicleVar + " = " + value)
            return value
        
    def IRefreshHelpSettings(self):
        clickToTurn = ptGUIControlRadioGroup(NavigationDlg.dialog.getControlFromTag(kNormNoviceRGID))
        clickToTurn.setValue(PtIsClickToTurn())


    def IRefreshAdvSettings(self):
        "refresh the volume settings to the current settings"
        getControl = AdvGameSettingDlg.dialog.getControlModFromTag

        # shadows
        
        # We'll have to do this later, since it's no longer part of the AdvDisplaySettings Dialog
        #~ shadowDistKnob = getControl(kGSDisplayShadowDistSlider)
        #~ setting = PtGetShadowVisDistance()
        #~ shadowDistKnob.setValue(setting*10.0)
        
        #Will have to wire this into Audio panel later
        #~ soundPriKnob = getControl(kGSSoundPrioritySlider)
        #~ audio = ptAudioControl()
        #~ soundPriKnob.setValue(audio.getPriorityCutoff()*(11.0/10.0))
        
        mouseSensKnob = getControl(kGSMouseTurnSensSlider)
        PtDebugPrint("IRefreshAdvSettings: PtGetMouseTurnSensitivity() = %d" % (PtGetMouseTurnSensitivity()))
        sensitive = PtGetMouseTurnSensitivity() - 50.0
        if sensitive <= 0.0:
            mouseSensKnob.setValue(0.0)
        else:
            if sensitive >= 300.0:
                mouseSensKnob.setValue(10.0)
            else:
                if sensitive > 100.0:
                    sensitive -= 100.0
                    mouseSensKnob.setValue((sensitive/40.0) + 5.0)
                elif sensitive == 100.0:
                    mouseSensKnob.setValue(5.0)
                else:
                    mouseSensKnob.setValue(sensitive/20.0)

        cam = ptCamera()
        getControl(kGSPukeCamCheckbox).setChecked(cam.isSmootherCam())
        getControl(kGSMouseInvertCheckbox).setChecked(PtIsMouseInverted())
        getControl(kGSWalkAndPan).setChecked(cam.isWalkAndVerticalPan())
        getControl(kGSStayInFirstPerson).setChecked(cam.isStayInFirstPerson())
        getControl(kGSClickToTurn).setChecked(PtIsClickToTurn())


    def LoadAdvSettings(self):
        global gMouseSensitivity
        global gSmoothCam
        global gMouseInvert
        global gWalkAndPan
        global gStayInFirstPerson
        global gClickToTurn

        AdvSettingsString = self.getChronicleVar("AdvSettings")
        AdvSettingsArray = []

        if AdvSettingsString == None:
            # create settings
            self.setNewChronicleVar("AdvSettings", "150 0 0 0 0 0")
            AdvSettingsArray = ["150","0","0","0","0","0"]
        else:
            AdvSettingsArray = AdvSettingsString.split()

        value = int(AdvSettingsArray[0])
        PtSetMouseTurnSensitivity(float(value))
        gMouseSensitivity = AdvSettingsArray[0]

        value = int(AdvSettingsArray[1])
        cam = ptCamera()
        cam.setSmootherCam(value)
        gSmoothCam = AdvSettingsArray[1]

        value = int(AdvSettingsArray[2])
        gMouseInvert = AdvSettingsArray[2]
        if value:
            PtSetMouseInverted()
        else:
            PtSetMouseUninverted()

        value = int(AdvSettingsArray[3])
        cam = ptCamera()
        cam.setWalkAndVerticalPan(value)
        gWalkAndPan = AdvSettingsArray[3]

        value = int(AdvSettingsArray[4])
        cam = ptCamera()
        cam.setStayInFirstPerson(value)
        gStayInFirstPerson = AdvSettingsArray[4]

        value = int(AdvSettingsArray[5])
        PtSetClickToTurn(value)
        gClickToTurn = AdvSettingsArray[5]

    def LoadKeyMap(self):
        km = ptKeyMap()
        KeyMapString = self.getChronicleVar("KeyMap")
        if not KeyMapString:
            PtDebugPrint("xOptionsMenu.LoadKeyMap():\tHmm... Empty chronicle... Setting to default.")
            self.ISetDefaultKeyMappings()
            return

        # set the key binds back to the saved
        for controlCode, mappedKey in zip(kDefaultControlCodeBinds, KeyMapString.split(" ")):
            if isinstance(controlCode, str):
                PtDebugPrint(f"xOptionsMenu.LoadKeyMap(): Binding {mappedKey=} to {controlCode=}", level=kWarningLevel)
                km.bindKeyToConsoleCommand(mappedKey, controlCode)
            else:
                controlStr = km.convertControlCodeToString(controlCode)
                key1, _, key2 = mappedKey.partition("$")
                PtDebugPrint(f"xOptionsMenu.LoadKeyMap(): Binding {key1=} & {key2=} to {controlStr=}", level=kWarningLevel)
                km.bindKey(key1, key2, controlStr)

    def IsThereACover(self,bookHtml):
        # search the bookhtml string looking for a cover
        idx = bookHtml.find('<cover')
        if idx >= 0:
            return 1
        return 0

    def IUpdateKeyMapChron(self) -> None:
        keyMapStr = " ".join(
            self.IGetBoundKey(controlCode) if isinstance(controlCode, str) else f"{self.IGetBoundKey(controlCode, 0)}${self.IGetBoundKey(controlCode, 1)}"
            for controlCode in kDefaultControlCodeBinds
        )
        self.setNewChronicleVar("KeyMap", keyMapStr)

    def IGetBoundKey(self, controlCode: Union[int, str], keyIdx: int = 0) -> str:
        km = ptKeyMap()
        if isinstance(controlCode, str):
            assert keyIdx == 0
            return km.convertVKeyToChar(km.getBindingKeyConsole(controlCode), km.getBindingFlagsConsole(controlCode))
        elif keyIdx == 0:
            return km.convertVKeyToChar(km.getBindingKey1(controlCode), km.getBindingFlags1(controlCode))
        elif keyIdx == 1:
            return km.convertVKeyToChar(km.getBindingKey2(controlCode), km.getBindingFlags2(controlCode))
        else:
            raise ValueError(f"{keyIdx=}")

    def ISetKeyMapping(self, controlCodeId: int, vkey: int, modifiers: int, isPrimary: bool) -> None:
        km = ptKeyMap()
        newKeyStr = km.convertVKeyToChar(vkey, modifiers)

        # If this is the same key as before, unmap the binding.
        controlCode = kControlCodes[controlCodeId]
        if self.IGetBoundKey(controlCode, keyIdx=0 if isPrimary else 1) == newKeyStr:
            newKeyStr = "(unmapped)"

        # This will cause any previous uses of the key to be unbound.
        if isinstance(controlCode, str):
            PtDebugPrint(f"xOptionsMenu.ISetKeyMapping(): Binding {newKeyStr=} to console command {controlCode=}")
            km.bindKeyToConsoleCommand(newKeyStr, controlCode)
        else:
            if isPrimary:
                primaryStr = newKeyStr
                secondaryStr = km.convertVKeyToChar(km.getBindingKey2(controlCode), km.getBindingFlags2(controlCode))
            else:
                primaryStr = km.convertVKeyToChar(km.getBindingKey1(controlCode), km.getBindingFlags1(controlCode))
                secondaryStr = newKeyStr

            controlStr = km.convertControlCodeToString(controlCode)
            PtDebugPrint(f"xOptionsMenu.ISetKeyMapping(): Binding {primaryStr=} {secondaryStr=} to {controlStr=}")
            km.bindKey(primaryStr, secondaryStr, controlStr)

        self.IUpdateKeyMapChron()

    def IShowMappedKeys(self,dlg,mapRow1,mapRow2):
        km = ptKeyMap()
        for cID in mapRow1.keys():
            field = ptGUIControlEditBox(dlg.getControlFromTag(cID))
            field.setSpecialCaptureKeyMode(1)
            # set the mapping
            controlCode,spFlag,mpFlag = mapRow1[cID]
            if controlCode is not None and ( ( spFlag and PtIsSinglePlayerMode() ) or ( mpFlag and not PtIsSinglePlayerMode() ) ):
                # is the control code a console command?
                if isinstance(controlCode, str):
                    field.setLastKeyCapture(km.getBindingKeyConsole(controlCode),km.getBindingFlagsConsole(controlCode))
                # else must be a event binding
                else:
                    field.setLastKeyCapture(km.getBindingKey1(controlCode),km.getBindingFlags1(controlCode))
            else:
                # disable this field
                field.hide()
                # if the first key is disabled, then the entire line should be! which is 100 less then the first field tag ID
                ftext = ptGUIControlTextBox(dlg.getControlFromTag(cID-100))
                ftext.hide()
        for cID in mapRow2.keys():
            field = ptGUIControlEditBox(dlg.getControlFromTag(cID))
            field.setSpecialCaptureKeyMode(1)
            # set the mapping
            controlCode,spFlag,mpFlag = mapRow2[cID]
            if controlCode is not None and ( ( spFlag and PtIsSinglePlayerMode() ) or ( mpFlag and not PtIsSinglePlayerMode() ) ):
                # is the control code a console command?
                if isinstance(controlCode, str):
                    # this shouldn't really happen!
                    field.setLastKeyCapture(km.getBindingKeyConsole(controlCode),km.getBindingFlagsConsole(controlCode))
                # else must be a event binding
                else:
                    field.setLastKeyCapture(km.getBindingKey2(controlCode),km.getBindingFlags2(controlCode))
            else:
                # disable this field
                field.hide()

    def ISetDefaultKeyMappings(self):
        km = ptKeyMap()
        for controlCode, (key1, key2) in kDefaultControlCodeBinds.items():
            if isinstance(controlCode, str):
                km.bindKeyToConsoleCommand(key1, controlCode)
            else:
                controlStr = km.convertControlCodeToString(controlCode)
                km.bindKey(key1, key2, controlStr)
        self.IUpdateKeyMapChron()

def res_comp(elem1, elem2):
    elem1w = int(elem1[:elem1.find("x")])
    elem1h = int(elem1[elem1.find("x") + 1:])
    elem2w = int(elem2[:elem2.find("x")])
    elem2h = int(elem2[elem2.find("x") + 1:])

    # cmp is gone in Python 3
    _cmp = lambda a, b: (a > b) - (a < b)

    result = _cmp(elem1w, elem2w)
    if result == 0:
        return _cmp(elem1h, elem2h)
    else:
        return result
