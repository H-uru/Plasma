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
"""Module: xOptionsMenu
Age: global
Author: Mark DeForest
Date: July 22, 2003
This is the python handler for the Options Menu
---- Phased for Prologue I thru Episode I


"""


MaxVersionNumber = 8
MinorVersionNumber = 4

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import xLocalization
import xJournalBookDefs
import PlasmaControlKeys
import os
import copy
import webbrowser
import sys


#import xIniInput
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
respDisableItems        = ptAttribResponder(13, "resp: Disable Items", ["enableRes", "disableRes", "enableWindow", "disableWindow", "enableEAX", "disableEAX", "enableGamma", "disableGamma"])
SupportDlg              = ptAttribGUIDialog(14, "Support dialog")


# globals
#--------

WebLaunchCmd = None
gLiveMovie = None
kLiveMovieName = "avi/URULiveIntro.bik"
kDemoMovieName = "avi/UruPreview.bik"
gPreviewStarted = 0
prevAudioDeviceName = None

# =====================================
# Options Menu dialog globals
#---------
kOptionsKeyMapBtn = 299
kOptionsKeyMapText = 399
kOptionsSettingsBtn = 300
kOptionsSettingText = 400
kOptionsTrailerBtn = 301
kOptionsTrailerText = 401
kOptionsHelpBtn = 302
kOptionsHelpText = 402
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


kSupportCancel = 780
kSupportLiveHelp = 781
kSupportTicket = 782

kClickOnMeBtn = 99

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
kIGHHeaderText = 700
kIGMHelpMessage = 710
kIGMCCRBtnText = 531

kNormNoviceRGID = 750


# =====================================
# Help (CCR) dialog globals
#---------
kHelpDropDownButton = 100
kHelpCCROutToLunch = 110
kHelpType = 200
kHelpTitle = 201
kHelpMLE = 202
CCRHelpDialogType = 0
kCCRPetitionTypeText = 280
kCCRSubjectText = 281
kCCRCommentText = 282
kCCRSubmitBtnText = 290
xCCRCancelBtnText = 291

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
gKM1ControlCodesRow1 = {
                        kKMEditLine1Row1 : (PlasmaControlKeys.kKeyMoveForward,1,1) ,\
                        kKMEditLine2Row1 : (PlasmaControlKeys.kKeyMoveBackward,1,1) ,\
                        kKMEditLine3Row1 : (PlasmaControlKeys.kKeyRotateLeft,1,1) ,\
                        kKMEditLine4Row1 : (PlasmaControlKeys.kKeyRotateRight,1,1) ,\
                        kKMEditLine5Row1 : (PlasmaControlKeys.kKeyJump,1,1) ,\
                        kKMEditLine6Row1 : (PlasmaControlKeys.kKeyStrafeLeft,1,1) ,\
                        kKMEditLine7Row1 : (PlasmaControlKeys.kKeyStrafeRight,1,1) ,\
                        kKMEditLine8Row1 : (PlasmaControlKeys.kKeyExitMode,1,1) ,\
                        kKMEditLine9Row1 : (PlasmaControlKeys.kKeySetFirstPersonMode,1,1) ,\
                        kKMEditLine10Row1 : ("Game.KIOpenYeeshaBook",1,1),\
                        kKMEditLine11Row1 : ("Game.KIHelp",1,1) ,\
                        kKMEditLine12Row1 : ("Game.KIOpenKI",0,1) ,\
                        kKMEditLine13Row1 : ("Game.KITakePicture",0,1),\
                        kKMEditLine14Row1 : ("Game.KICreateJournal",0,1),\
                        kKMEditLine15Row1 : (PlasmaControlKeys.kKeyPushToTalk,0,1) ,\
                        kKMEditLine16Row1 : ("Game.EnterChatMode",0,1) ,\
                        kKMEditLine17Row1 : ("Game.KICreateMarkerFolder",0,1) ,\
                        kKMEditLine18Row1 : ("Game.KICreateMarker",0,1) ,\
                    }
gKM1ControlCodesRow2 = {
                        kKMEditLine1Row2 : (PlasmaControlKeys.kKeyMoveForward,1,1) ,\
                        kKMEditLine2Row2 : (PlasmaControlKeys.kKeyMoveBackward,1,1) ,\
                        kKMEditLine3Row2 : (PlasmaControlKeys.kKeyRotateLeft,1,1) ,\
                        kKMEditLine4Row2 : (PlasmaControlKeys.kKeyRotateRight,1,1) ,\
                        kKMEditLine5Row2 : (PlasmaControlKeys.kKeyJump,1,1) ,\
                        kKMEditLine6Row2 : (PlasmaControlKeys.kKeyStrafeLeft,1,1) ,\
                        kKMEditLine7Row2 : (PlasmaControlKeys.kKeyStrafeRight,1,1) ,\
                        kKMEditLine8Row2 : (PlasmaControlKeys.kKeyExitMode,1,1) ,\
                        kKMEditLine9Row2 : (PlasmaControlKeys.kKeySetFirstPersonMode,1,1) ,\
                        kKMEditLine10Row2 : (None,0,0),\
                        kKMEditLine11Row2 : (None,0,0) ,\
                        kKMEditLine12Row2 : (None,0,0),\
                        kKMEditLine13Row2 : (None,0,0) ,\
                        kKMEditLine14Row2 : (None,0,0) ,\
                        kKMEditLine15Row2 : (PlasmaControlKeys.kKeyPushToTalk,0,1) ,\
                        kKMEditLine16Row2 : (None,0,0) ,\
                        kKMEditLine17Row2 : (None,0,0) ,\
                        kKMEditLine18Row2 : (None,0,0) ,\
                    }

defaultControlCodeBinds = { PlasmaControlKeys.kKeyMoveForward : ( "UpArrow","(unmapped)" ) ,\
                            PlasmaControlKeys.kKeyMoveBackward : ( "DownArrow","(unmapped)" ),\
                            PlasmaControlKeys.kKeyRotateLeft : ( "LeftArrow","(unmapped)" ) ,\
                            PlasmaControlKeys.kKeyRotateRight : ( "RightArrow","(unmapped)" ) ,\
                            PlasmaControlKeys.kKeyJump : ( "SpaceBar","(unmapped)" ),\
                            PlasmaControlKeys.kKeyStrafeLeft : ( "Comma","(unmapped)" ) ,\
                            PlasmaControlKeys.kKeyStrafeRight : ( "Period","(unmapped)" ) ,\
                            PlasmaControlKeys.kKeyExitMode : ( "Backspace","Esc" ) ,\
                            PlasmaControlKeys.kKeySetFirstPersonMode : ( "F1","F_C" ) ,\
                            "Game.KIOpenYeeshaBook" : ("F3","(unmapped)"),\
                            "Game.KIHelp" : ("F4","(unmapped)"),\
                            "Game.KIOpenKI" : ("F2","(unmapped)"),\
                            "Game.KITakePicture" : ("F5","(unmapped)"),\
                            "Game.KICreateJournal" : ("F6","(unmapped)"),\
                            PlasmaControlKeys.kKeyPushToTalk : ( "Tab","(unmapped)" ) ,\
                            "Game.EnterChatMode" : ("(unmapped)","(unmapped)"),\
                            "Game.KICreateMarkerFolder" : ("F8","(unmapped)"),\
                            "Game.KICreateMarker" : ("F7","(unmapped)"),\
                        }

defaultControlCodeBindsOrdered = [  PlasmaControlKeys.kKeyMoveForward, PlasmaControlKeys.kKeyMoveBackward, PlasmaControlKeys.kKeyRotateLeft, PlasmaControlKeys.kKeyRotateRight, PlasmaControlKeys.kKeyJump, PlasmaControlKeys.kKeyStrafeLeft, PlasmaControlKeys.kKeyStrafeRight, PlasmaControlKeys.kKeyExitMode, PlasmaControlKeys.kKeySetFirstPersonMode, "Game.KIOpenYeeshaBook", "Game.KIHelp", "Game.KIOpenKI", "Game.KITakePicture", "Game.KICreateJournal", PlasmaControlKeys.kKeyPushToTalk, "Game.EnterChatMode", "Game.KICreateMarkerFolder", "Game.KICreateMarker"]

kVideoResolutions = ["800x600", "1024x768", "1280x960", "1280x1024", "1600x1200"]
kVideoExtraResolutions = ["1152x864", "1280x720", "1280x768", "1280x800", "1280x854", "1440x900", "1600x900", "1680x1050", "1920x1080", "1920x1200", "2560x1600"]
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

kGSAudioMuteCheckbox = 456
kGSMouseTurnSensSlider = 460
kGSSoundPrioritySlider = 462
kGSPukeCamCheckbox = 464
kGSMouseInvertCheckbox = 466
kGSWalkAndPan = 468
kGSStayInFirstPerson = 470
kGSClickToTurn = 472

kGSFakeButton5 = 605
kGSFakeButton6 = 606
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

# - string IDs for the Advances Settings localization
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

gWasMuted = 0
gFirstReltoVisit = true

gMouseSensitivity = "150"
gSmoothCam = "0"
gMouseInvert = "0"
gWalkAndPan = "0"
gStayInFirstPerson = "0"
gClickToTurn = "0"
gAudioHack = 0
gCurrentReleaseNotes = ""

class xOptionsMenu(ptModifier):
    "The Options dialog modifier, includes Help/Petition and others"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 195
        self.version = MaxVersionNumber
        PtDebugPrint("__xOptionsMenu: Max version %d - minor version %d" % (MaxVersionNumber,MinorVersionNumber))
        
        self.restartWarn = false
        self.goingToCalibration = 0

    def OnFirstUpdate(self):
        global WebLaunchCmd
        "First update, load our dialogs"
        PtLoadDialog("OptionsMenuGUI",self.key)
        PtLoadDialog("KIHelp",self.key)
        PtLoadDialog("KIHelpMenu",self.key)
        PtLoadDialog("KeyMapDialog",self.key)
        PtLoadDialog("GameSettingsDialog",self.key)
        PtLoadDialog("CalibrationGUI",self.key)
        PtLoadDialog("TrailerPreviewGUI",self.key)
        PtLoadDialog("KeyMap2Dialog",self.key)
        PtLoadDialog("AdvancedGameSettingsDialog",self.key)
        PtLoadDialog("OptionsHelpGUI",self.key)
        PtLoadDialog("NavigationSettingsDialog",self.key)
        PtLoadDialog("GraphicsSettingsDialog",self.key)
        PtLoadDialog("AudioSettingsDialog",self.key)
        PtLoadDialog("OptionsMenuRestart",self.key)
        PtLoadDialog("ReleaseNotesDialog",self.key)
        PtLoadDialog("ContactSupportGUI",self.key)

        WebLaunchCmd = webbrowser.open_new

    def __del__(self):
        "the destructor - unload any dialogs we loaded"
        PtUnloadDialog("KIHelp")
        PtUnloadDialog("KIHelpMenu")
        PtUnloadDialog("KeyMapDialog")
        PtUnloadDialog("GameSettingsDialog")
        PtUnloadDialog("CalibrationGUI")
        PtUnloadDialog("TrailerPreviewGUI")
        PtUnloadDialog("KeyMap2Dialog")
        PtUnloadDialog("AdvancedGameSettingsDialog")
        PtUnloadDialog("OptionsHelpGUI")
        PtUnloadDialog("OptionsMenuRestart")
        PtUnloadDialog("ReleaseNotesDialog")
        PtUnloadDialog("ContactSupportGUI")

    def BeginAgeUnLoad(self,avatar):
        "When the current age is being unloaded, ie. we've linked out - hide all modal dialogs!"
        global gJournalBook
        global gPreviewStarted
        if OptionsMenuDlg.dialog.isEnabled():
            OptionsMenuDlg.dialog.hide()
        if CCRDlg.dialog.isEnabled():
            CCRDlg.dialog.hide()
        if KeyMapDlg.dialog.isEnabled():
            KeyMapDlg.dialog.hide()
        #~ if SettingsDlg.dialog.isEnabled():
            #~ SettingsDlg.dialog.hide()
        if CalibrateDlg.dialog.isEnabled():
            CalibrateDlg.dialog.hide()
        if SupportDlg.dialog.isEnabled():
            SupportDlg.dialog.hide()
        if gPreviewStarted:
            PtFadeOut(kOptionFadeOutSeconds,1)
            PtAtTimeCallback(self.key, kOptionFadeOutSeconds, kTrailerFadeOutID)
        #~ if KeyMap2Dlg.dialog.isEnabled():
            #~ KeyMap2Dlg.dialog.hide()
        if AdvGameSettingDlg.dialog.isEnabled():
            AdvGameSettingDlg.dialog.hide()
        if NavigationDlg.dialog.isEnabled():
            NavigationDlg.dialog.hide()
        if ReleaseNotesDlg.dialog.isEnabled():
            ReleaseNotesDlg.dialog.hide()
        if gJournalBook:
            gJournalBook.hide()

    def OnPageLoad(self,what,room):
        global gFirstReltoVisit

        if (room == "Personal_psnlMYSTII" or room == "Personal_District_psnlMYSTII") and gFirstReltoVisit:
            gFirstReltoVisit = false

            vault = ptVault()
            if type(vault) != type(None):
                entry = vault.findChronicleEntry("KeyMap")
                if type(entry) == type(None):
                    # not found... create defaults
                    self.ISetDefaultKeyMappings()

            self.LoadAdvSettings()
            self.LoadKeyMap()
            GammaVal = self.getChronicleVar("gamma")
            if GammaVal == None:
                PtSetGamma2(0)
            else:
                PtAtTimeCallback (self.key, 5, 999)

    def OnNotify(self,state,id,events):
        global gFirstReltoVisit
        "Notify - should only be needed for credits book"
        PtDebugPrint("xOptionsMenu: Notify  state=%f, id=%d" % (state,id),level=kDebugDumpLevel)

        if id == -1:
            print "Options Menu got notify, resetting First Visit status"
            gFirstReltoVisit = true
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
            if event == kAction or event == kValueChanged:
                # test to see which control had the event
                omID = control.getTagID()
                if omID == kOptionsNavigationBtn:
                    OptionsMenuDlg.dialog.hide()
                    NavigationDlg.dialog.show()
                    print "The Navigation dialog should show now..."
                elif omID == kOptionsGraphicSettingsBtn:
                    OptionsMenuDlg.dialog.hide()
                    GraphicsSettingsDlg.dialog.show()
                    print "The graphics dialog should show now..."
                elif omID == kOptionsAudioSettingsBtn:
                    self.InitAudioControlsGUI()
                    OptionsMenuDlg.dialog.hide()
                    AudioSettingsDlg.dialog.show()
                    print "The audio dialog should show now..."
                elif omID == kOptionsKeyMapBtn:
                    OptionsMenuDlg.dialog.hide()
                    KeyMapDlg.dialog.show()
                elif omID == kOptionsTrailerBtn:
                    gPreviewStarted = 1
                    OptionsMenuDlg.dialog.hide()
                    PtFadeOut(kOptionFadeOutSeconds,1)
                    PtAtTimeCallback(self.key, kOptionFadeOutSeconds, kOptionFadeOutID)
                elif omID == kOptionsHelpBtn:
                    OptionsMenuDlg.dialog.hide()
                    SupportDlg.dialog.show()

                    #OptionsMenuDlg.dialog.hide()
                    #print "CCRDlg.dialog.isEnabled = ",CCRDlg.dialog.isEnabled()                    
                    #CCRDlg.dialog.show()                    
                    #print "CCRDlg.dialog.isEnabled = ",CCRDlg.dialog.isEnabled()                    
                    ##~ HelpDlg.dialog.show()
                    #print "the CCR dialog should show now..."
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
##  Contact Support dialog processing
##
###############################################
        elif id == SupportDlg.id:
            if event == kDialogLoaded:
                pass

            elif event == kShowHide:
                if control.isEnabled():
                    pass

            elif event == kAction or event == kValueChanged:
                spID = control.getTagID()
                if spID == kSupportTicket:
                    WebLaunchCmd("http://www.mystonline.com/contact.html")
                elif spID == kSupportLiveHelp:
                    actName = PtGetAccountName()
                    kiNum = unicode(PtGetLocalPlayer().getPlayerID())
                    acctInfo = urlencode({'fullname':actName})
                    fullURL = U"http://support.mystonline.com/live/visitor/index.php?_m=livesupport&_a=startclientchat&proactive=0&departmentid=0&randno=4&" + acctInfo + U":" + kiNum
                    #print "DEBUG: URL = " + fullURL
                    WebLaunchCmd(fullURL)
                elif spID == kSupportCancel:
                    SupportDlg.dialog.hide()
                    OptionsMenuDlg.dialog.show()
                    
            elif event == kExitMode:
                SupportDlg.dialog.hide()


###############################################
##
##  Release Notes dialog processing
##
###############################################
        elif id == ReleaseNotesDlg.id:
            global gCurrentReleaseNotes

            if event == kDialogLoaded:
                textField = ptGUIControlMultiLineEdit(ReleaseNotesDlg.dialog.getControlFromTag(kReleaseTextArea))
                textField.setString("")
                textField.lock()
                textField.unclickable()
                try:
                    f = open("ReleaseNotes.txt", "r")
                    gCurrentReleaseNotes = f.read()
                    f.close()
                except:
                    print "[TXT processing] Error while reading ReleaseNotes.txt"
            elif event == kShowHide:
                if control.isEnabled():
                    textField = ptGUIControlMultiLineEdit(ReleaseNotesDlg.dialog.getControlFromTag(kReleaseTextArea))
                    textField.clearBuffer()
                    textField.insertString(gCurrentReleaseNotes)
                    print textField.getString()
                    textField.setScrollPosition(1)
                    textField.setScrollPosition(0)
            elif event == kAction or event == kValueChanged:
                rnID = control.getTagID()
                if rnID == kRNOkBtn:
                    ReleaseNotesDlg.dialog.hide()
                elif rnID == kRNGoBackBtn:
                    ReleaseNotesDlg.dialog.hide()
                    OptionsMenuDlg.dialog.show()                    
            elif event == kExitMode:
                ReleaseNotesDlg.dialog.hide()


###############################################
##
##  CCR dialog processing
##
###############################################
        elif id == CCRDlg.id:
            if event == kDialogLoaded:
                # set the deafault for the drop down
                CCRHelpDialogType = xLocalization.xKI.xCCRHelpPopupDefault
                typeField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kHelpType))
                typeField.setString(xLocalization.xKI.xCCRHelpPopupMenu[CCRHelpDialogType][0])
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    # set the text localized strings
                    textField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kCCRPetitionTypeText))
                    textField.setString(xLocalization.xKI.xCCRPetitionTypeText)
                    textField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kCCRSubjectText))
                    textField.setString(xLocalization.xKI.xCCRSubjectText)
                    textField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kCCRCommentText))
                    textField.setString(xLocalization.xKI.xCCRCommentText)
                    textField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kCCRSubmitBtnText))
                    textField.setString(xLocalization.xKI.xCCRSubmitBtnText)
                    textField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(xCCRCancelBtnText))
                    textField.setString(xLocalization.xKI.xCCRCancelBtnText)
                    # put back defaults to the fields in the dialog box
                    CCRHelpDialogType = xLocalization.xKI.xCCRHelpPopupDefault
                    typeField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kHelpType))
                    typeField.setString(xLocalization.xKI.xCCRHelpPopupMenu[CCRHelpDialogType][0])
                    titleField = ptGUIControlEditBox(CCRDlg.dialog.getControlFromTag(kHelpTitle))
                    titleField.clearString()
                    bodyField = ptGUIControlMultiLineEdit(CCRDlg.dialog.getControlFromTag(kHelpMLE))
                    bodyField.clearBuffer()
                    CCROutField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kHelpCCROutToLunch))
                    if PtIsCCRAway():
                        CCROutField.setString(xLocalization.xKI.xCCRAwayText)
                        CCROutField.show()
                    else:
                        CCROutField.hide()
            elif event == kAction or event == kValueChanged:
                helpID = control.getTagID()
                if helpID == kYesButtonID:
                    # get the text from the title and the body
                    titleField = ptGUIControlEditBox(CCRDlg.dialog.getControlFromTag(kHelpTitle))
                    bodyField = ptGUIControlMultiLineEdit(CCRDlg.dialog.getControlFromTag(kHelpMLE))
                    PtSendPetitionToCCR(bodyField.getString(),xLocalization.xKI.xCCRHelpPopupMenu[CCRHelpDialogType][1],titleField.getString())
                    CCRDlg.dialog.hide()
                elif helpID == kNoButtonID:
                    CCRDlg.dialog.hide()
                elif helpID == kHelpDropDownButton:
                    CCRPopupMenu.menu.show()
            elif event == kExitMode:
                if CCRPopupMenu.menu.isEnabled():
                    CCRPopupMenu.menu.hide()
                else:
                    CCRDlg.dialog.hide()

        elif id == CCRPopupMenu.id:
            if event == kDialogLoaded:
                # load the popup memu with items
                for menuItem in xLocalization.xKI.xCCRHelpPopupMenu:
                    CCRPopupMenu.menu.addNotifyItem(menuItem[0])
            elif event == kAction:
                menuID = control.getTagID()
                # set the type field
                CCRHelpDialogType = menuID
                typeField = ptGUIControlTextBox(CCRDlg.dialog.getControlFromTag(kHelpType))
                typeField.setString(xLocalization.xKI.xCCRHelpPopupMenu[CCRHelpDialogType][0])

###############################################
##
##  Key Map dialog processing
##
###############################################
        elif id == KeyMapDlg.id:
            if event == kDialogLoaded:
                pass
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    # localize the strings
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine1))
                    textField.setString(xLocalization.xOptions.xKMMoveForward)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine2))
                    textField.setString(xLocalization.xOptions.xKMMoveBack)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine3))
                    textField.setString(xLocalization.xOptions.xKMRotLeft)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine4))
                    textField.setString(xLocalization.xOptions.xKMRotRight)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine5))
                    textField.setString(xLocalization.xOptions.xKMJump)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine6))
                    textField.setString(xLocalization.xOptions.xKMStrafeLeft)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine7))
                    textField.setString(xLocalization.xOptions.xKMStrafeRight)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine8))
                    textField.setString(xLocalization.xOptions.xKMExitMode)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMTextLine9))
                    textField.setString(xLocalization.xOptions.xKMFirstPerson)
                    # buttons localized
                    #~ textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kKMNextPreviousText))
                    #~ textField.setString(xLocalization.xOptions.xKMNextText)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kOptionsOkText))
                    textField.setString(xLocalization.xOptions.xOptionsResumeText)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kOptionsDefaultsText))
                    textField.setString(xLocalization.xOptions.xOptionsDefaultsText)
                    textField = ptGUIControlTextBox(KeyMapDlg.dialog.getControlFromTag(kOptionsGoBackText))
                    textField.setString(xLocalization.xOptions.xOptionsGoBackText)
                    self.IShowMappedKeys(control,gKM1ControlCodesRow1,gKM1ControlCodesRow2)
                    # read the ini file in
                    # xIniInput.ReadIni()
                else:
                    # xIniInput.WriteIni()
                    pass
            elif event == kAction or event == kValueChanged:
                kmID = control.getTagID()
                if kmID == kKMOkBtn:
                    KeyMapDlg.dialog.hide()
                elif kmID in gKM1ControlCodesRow1.keys():
                    NewKeyMapString = ""
                    # get the new keys and bind
                    km = ptKeyMap()
                    cCode,spFlag,mpFlag = gKM1ControlCodesRow1[kmID]
                    if type(cCode) == type(""):
                        key1 = km.convertVKeyToChar(control.getLastKeyCaptured(),control.getLastModifiersCaptured())
                        km.bindKeyToConsoleCommand(key1,cCode)
                        KeyMapString = self.getChronicleVar("KeyMap")
                        KeyMapArray = KeyMapString.split()
                        KeyMapArray[(kmID-300)] = key1
                        for key in KeyMapArray:
                            NewKeyMapString += key + " "
                        self.setNewChronicleVar("KeyMap", NewKeyMapString.rstrip())
                    elif type(cCode) != type(None):
                        otherID = kmID + 100
                        otherField = ptGUIControlEditBox(KeyMapDlg.dialog.getControlFromTag(otherID))
                        key1 = km.convertVKeyToChar(control.getLastKeyCaptured(),control.getLastModifiersCaptured())
                        key2 = km.convertVKeyToChar(otherField.getLastKeyCaptured(),otherField.getLastModifiersCaptured())
                        controlStr = km.convertControlCodeToString(cCode)
                        km.bindKey(key1,key2,controlStr)
                        KeyMapString = self.getChronicleVar("KeyMap")
                        KeyMapArray = KeyMapString.split()
                        KeyMapArray[(kmID-300)] = key1 + "$" + key2 + " "
                        for key in KeyMapArray:
                            NewKeyMapString += key + " "
                        self.setNewChronicleVar("KeyMap", NewKeyMapString.rstrip())
                    # lose the focus when done
                    KeyMapDlg.dialog.noFocus()
                    # force writing the keymap
                    km.writeKeyMap()
                    # re-show the keymap because they may have been stupid and map the same key to multiple actions
                    self.IShowMappedKeys(KeyMapDlg.dialog,gKM1ControlCodesRow1,gKM1ControlCodesRow2)
                    # need to re-set the ini file, in case something got unmapped
                    #self.IMatchIniToGame()
                elif kmID in gKM1ControlCodesRow2.keys():
                    NewKeyMapString = ""
                    # get the new keys and bind
                    km = ptKeyMap()
                    cCode,spFlag,mpFlag = gKM1ControlCodesRow2[kmID]
                    if type(cCode) == type(""):
                        # console command  - this shouldn't really happen!
                        key1 = km.convertVKeyToChar(control.getLastKeyCaptured(),control.getLastModifiersCaptured())
                        km.bindKeyToConsoleCommand(key1,cCode)
                        # console keys not in input.ini... yet
                        KeyMapString = self.getChronicleVar("KeyMap")
                        KeyMapArray = KeyMapString.split()
                        KeyMapArray[(kmID-300)] = key1
                        for key in KeyMapArray:
                            NewKeyMapString += key + " "
                        self.setNewChronicleVar("KeyMap", NewKeyMapString.rstrip())
                        #xIniInput.SetConsoleKey('"'+cCode+'"',key1+',')
                    elif type(cCode) != type(None):
                        otherID = kmID - 100
                        otherField = ptGUIControlEditBox(KeyMapDlg.dialog.getControlFromTag(otherID))
                        key2 = km.convertVKeyToChar(control.getLastKeyCaptured(),control.getLastModifiersCaptured())
                        key1 = km.convertVKeyToChar(otherField.getLastKeyCaptured(),otherField.getLastModifiersCaptured())
                        controlStr = km.convertControlCodeToString(cCode)
                        km.bindKey(key1,key2,controlStr)
                        KeyMapString = self.getChronicleVar("KeyMap")
                        KeyMapArray = KeyMapString.split()
                        KeyMapArray[(otherID-300)] = key1 + "$" + key2 + " "
                        for key in KeyMapArray:
                            NewKeyMapString += key + " "
                        self.setNewChronicleVar("KeyMap", NewKeyMapString.rstrip())
                        #xIniInput.SetControlKey('"'+controlStr+'"',key1+',',key2+',')
                    # lose the focus when done
                    KeyMapDlg.dialog.noFocus()
                    # force writing the keymap
                    km.writeKeyMap()
                    # re-show the keymap because they may have been stupid and map the same key to multiple actions
                    self.IShowMappedKeys(KeyMapDlg.dialog,gKM1ControlCodesRow1,gKM1ControlCodesRow2)
                    # need to re-set the ini file, in case something got unmapped
                    #self.IMatchIniToGame()
                elif kmID == kKMDefaultsBtn:
                    self.ISetDefaultKeyMappings()
                    self.IShowMappedKeys(KeyMapDlg.dialog,gKM1ControlCodesRow1,gKM1ControlCodesRow2)
                elif kmID == kKMGoBackBtn:
                    KeyMapDlg.dialog.hide()
                    AdvGameSettingDlg.dialog.show()                    
                    #~ OptionsMenuDlg.dialog.show()
                    
                #~ elif kmID == kKMNextPreviousBtn:
                    #~ KeyMapDlg.dialog.hide()
                    #~ KeyMap2Dlg.dialog.show()
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
                print "Yes, the Advanced Settings dialog loaded."
            elif event == kShowHide:
                if control.isEnabled():
                    self.IRefreshAdvSettings()
            elif event == kAction or event == kValueChanged:
                gsID = control.getTagID()
                print "gsID = " + str(gsID)

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
            if event == kDialogLoaded:
                textField = ptGUIControlTextBox(CalibrateDlg.dialog.getControlFromTag(kCalMessageText))
                textField.setString(xLocalization.xOptions.xCalMessageText)
            elif event == kShowHide:
                if control.isEnabled():
                    textField = ptGUIControlTextBox(CalibrateDlg.dialog.getControlFromTag(kCalMessageText))
                    textField.setString(xLocalization.xOptions.xCalMessageText)
            elif event == kAction or event == kValueChanged:
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
            if event == kDialogLoaded:
                pass
            elif event == kShowHide:
                pass
            elif event == kAction or event == kValueChanged:
                tpID = control.getTagID()
                if tpID == kClickOnMeBtn:
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
                print "Yes, the ResetWarn Dialog loaded."
                pass
            elif event == kShowHide:
                print "event = kShowHide = ", kShowHide
                pass                
            elif event == kAction or event == kValueChanged:
                # test to see which control had the event
                warnID = control.getTagID()
                print "warnID = ",warnID
                if warnID == kResetWarningYes:
                    print "I need to shut down Plasma now."
                    self.WriteVideoControls()
                    PtConsole("App.Quit")
                elif warnID == kResetWarningNo:
                    print "close the dialog now."
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
            #~ print "navigation event = ", event
            #~ print "kShowHide = ",kShowHide, " kAction = ",kAction, " kValueChanged = ", kValueChanged
            if event == kDialogLoaded:
                print "Yes, the Navigation dialog loaded."
                pass
                
                
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    self.IRefreshHelpSettings()
                    # set the text localized strings
                    print "I should now localize the Navigation dialog text fields"
                    #~ textField = ptGUIControlTextBox(NavigationDlg.dialog.getControlFromTag(kOptionsGoBackText))
                    #~ textField.setString(xLocalization.xOptions.xOptionsGoBackText)
                    #~ textField = ptGUIControlTextBox(NavigationDlg.dialog.getControlFromTag(kOptionsOkText))
                    #~ textField.setString(xLocalization.xOptions.xOptionsResumeText)
                    #~ textField = ptGUIControlTextBox(NavigationDlg.dialog.getControlFromTag(kGSAdvancedBtnText))
                    #~ textField.setString(xLocalization.xOptions.xGSAdvancedBtnText)                    
                    
            elif event == kAction or event == kValueChanged:

                NavigationID = control.getTagID()
                print "NavigationID = ", NavigationID                
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
                        print "CTT on"
                        PtSetClickToTurn(1)
                        gClickToTurn = "1"
                        AdvSettingsString = self.getChronicleVar("AdvSettings")

                        if AdvSettingsString == None:
                            # create settings
                            self.setNewChronicleVar("AdvSettings", "150 0 0 0 0 1")
                        else:
                            self.setNewChronicleVar("AdvSettings", (AdvSettingsString[0:-1] + "1"))
                    else:
                        print "CTT off"
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
                    print "self.restartWarn =", self.restartWarn
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
                    #print "Read: " + str(videoSlider.getValue())
                    #print "Based: " + str(videoSlider.getValue() * (len(vidResList) - 1))
                    #print "Round: " + str(round(resSlider))
                    #print "Index: " + str(curSelection)
                    #print "Set: " + str(float(curSelection) / (len(vidResList) - 1))
                    if len(vidResList) == 1:
                        videoSlider.setValue(0)
                        videoSlider.disable()
                        respDisableItems.run(self.key, state="disableRes")
                        videoText.setString("800x600")
                        videoText.setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                        ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextHeaderTag)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                    else:
                        control.setValue(float(curSelection) / (len(vidResList) - 1))
                        if curText != vidResList[curSelection]:
                            if vidResList[curSelection] in ["800x600", "1024x768", "1152x864", "1280x960", "1600x1200"]:
                                videoText.setString(vidResList[curSelection] + " [4:3]")
                            elif vidResList[curSelection] in ["1280x720", "1600x900"]:
                                videoText.setString(vidResList[curSelection] + " [16:9]")
                            elif vidResList[curSelection] in ["1280x800", "1440x900", "1680x1050", "1920x1200", "2560x1600"]:
                                videoText.setString(vidResList[curSelection] + " [16:10]")
                            elif vidResList[curSelection] in ["1280x854"]:
                                videoText.setString(vidResList[curSelection] + " [3:2]")
                            elif vidResList[curSelection] in ["1280x768"]:
                                videoText.setString(vidResList[curSelection] + " [5:3]")
                            elif vidResList[curSelection] in ["1280x1024"]:
                                videoText.setString(vidResList[curSelection] + " [5:4]")

                elif tagID == kVideoQualitySliderTag or tagID == kVideoTextureQualitySliderTag:
                    self.restartWarn = 1
                    curVal = round(control.getValue())
                    control.setValue(int(curVal))

                elif tagID == kVideoWindowedCheckTag:
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
                    if not vidRes.getString() in vidResList:
                        vidRes.setString("800x600 [4:3]")
                    videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResSliderTag))
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
                            if vidRes.getString() == vidResList[res]:
                                videoField.setValue( float(res) / (numRes - 1))
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
                print "Yes, the Audio dialog loaded."
                
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    print "I need to localize the Audio dialog text"
                    self.restartAudio = 0

                else:
                    if self.restartAudio:
                        audio = ptAudioControl()
                        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeID))
                        audModeNum = audio.getNumAudioDevices() - 1
                        curSelection = round(audioField.getValue() * audModeNum) 
                        intCurSelection = int(curSelection)

                        #Enable EAX Support
                        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))
                        audio.useEAXAcceleration(EAXcheckbox.isChecked())

                        audio.setDeviceName(audio.getAudioDeviceName(intCurSelection), 1)
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
                    #~ print "Number of Audio Devices: %d" % (audio.getNumAudioDevices())
                    audModeNum = audio.getNumAudioDevices() - 1
                    curSelection = round(control.getValue() * audModeNum)
                    intCurSelection = int(curSelection)
                    control.setValue(curSelection/audModeNum)

                    audioDeviceName = audio.getAudioDeviceName(intCurSelection)

                    audioModeCtrlTextBox = ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeTextID))
                    curText = audioModeCtrlTextBox.getString()
                    if curText != audio.getAudioDeviceName(intCurSelection):
                        audioModeCtrlTextBox.setString(audioDeviceName)

                    if audioDeviceName != prevAudioDeviceName:  #Only update the EAX checkbox when the mouse has been let up...
                        print "Audio Device Name changed!"
                        prevAudioDeviceName = audioDeviceName
                        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))
                        if not audio.supportsEAX(audioDeviceName):
                            print "Disabling EAX checkbox"
                            #Disable EAX checkbox
                            EAXcheckbox.disable()
                            respDisableItems.run(self.key, state="disableEAX")
                            EAXcheckbox.setChecked(false)
                            ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeEAXTextID)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                        else:
                            print "Enabling EAX checkbox"
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
        global gWasMuted
        global gPreviewStarted
        if id == kOptionFadeOutID:
            # is the movie there?
            try:
                if PtIsDemoMode():
                    os.stat(kDemoMovieName)
                else:
                    os.stat(kLiveMovieName)
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
            # temp mute sound
            audio = ptAudioControl()
            if audio.isMuted():
                gWasMuted = 1
            else:
                gWasMuted = 0
                audio.muteAll()
            PtFadeIn(kTrailerInSeconds,0)
            PtAtTimeCallback(self.key, kTrailerInSeconds, kTrailerFadeInID)
            if PtIsDemoMode():
                gLiveMovie = ptMoviePlayer(kDemoMovieName,self.key)
            else:
                gLiveMovie = ptMoviePlayer(kLiveMovieName,self.key)
            gLiveMovie.playPaused()
        elif id == kTrailerFadeInID:
            PtDebugPrint("xLiveTrailer - roll the movie",level=kDebugDumpLevel)
            if type(gLiveMovie) != type(None):
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
            if not gWasMuted:
                audio = ptAudioControl()
                audio.unmuteAll()
            OptionsMenuDlg.dialog.show()
            PtFadeIn(kOptionFadeInSeconds,0)
        elif id == 999:
            GammaVal = self.getChronicleVar("gamma")
            PtSetGamma2(float(GammaVal))

    def OnMovieEvent(self,movieName,reason):
        PtDebugPrint("xLiveTrailer: got movie done event on %s, reason=%d" % (movieName,reason),level=kDebugDumpLevel)
        if gLiveMovie:
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
        videoResField = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))
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

        if vidRes in ["800x600", "1024x768", "1152x864", "1280x960", "1600x1200"]:
            videoResField.setString(vidRes + " [4:3]")
        elif vidRes in ["1280x720", "1600x900"]:
            videoResField.setString(vidRes + " [16:9]")
        elif vidRes in ["1280x800", "1440x900", "1680x1050", "1920x1200", "2560x1600"]:
            videoResField.setString(vidRes + " [16:10]")
        elif vidRes in ["1280x854"]:
            videoResField.setString(vidRes + " [3:2]")
        elif vidRes in ["1280x768"]:
            videoResField.setString(vidRes + " [5:3]")
        elif vidRes in ["1280x1024"]:
            videoResField.setString(vidRes + " [5:4]")

    def InitVideoControlsGUI(self):
        xIniDisplay.ReadIni()
        opts = xIniDisplay.GetGraphicsOptions()

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoQualitySliderTag))
        videoField.setValue(int(opts[xIniDisplay.kGraphicsQualityLevel]))

        videoField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoTextureQualitySliderTag))
        videoField.setValue(int(opts[xIniDisplay.kGraphicsTextureQuality]))

        shadows = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoShadowsCheckTag))
        if opts[xIniDisplay.kGraphicsShadows] == 1:
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

        #if PtGetDesktopColorDepth() == 16 or PtGetDesktopWidth() < 800:
        #    ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedTextTag)).setForeColor(ptColor(.3, .3, .3, .5))
        #    windowed.disable()
        #    dowindow = 0

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

        if vidRes in ["800x600", "1024x768", "1152x864", "1280x960", "1600x1200"]:
            videoResField.setString(vidRes + " [4:3]")
        elif vidRes in ["1280x720", "1600x900"]:
            videoResField.setString(vidRes + " [16:9]")
        elif vidRes in ["1280x800", "1440x900", "1680x1050", "1920x1200", "2560x1600"]:
            videoResField.setString(vidRes + " [16:10]")
        elif vidRes in ["1280x854"]:
            videoResField.setString(vidRes + " [3:2]")
        elif vidRes in ["1280x768"]:
            videoResField.setString(vidRes + " [5:3]")
        elif vidRes in ["1280x1024"]:
            videoResField.setString(vidRes + " [5:4]")

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

    def WriteVideoControls(self, setMode = 0):
        videoField = ptGUIControlTextBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoResTextTag))
        width, height = videoField.getString().split("x")
        height, trash = height.split(" ")
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

        vsync = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoVerticalSyncCheckTag)).isChecked()
        if vsync:
            vsyncstr = "true"
        else:
            vsyncstr = "false"

        gammaField = ptGUIControlKnob(GraphicsSettingsDlg.dialog.getControlFromTag(kGSDisplayGammaSlider))
        gamma = gammaField.getValue()

        '''
        print "width: " + str(width)
        print "height: " + str(height)
        print "colordepth: " + str(colordepth)
        print "windowed: " + str(windowed)
        print "tex_quality: " + str(tex_quality)
        print "antialias: " + str(antialias)
        print "aniso: " + str(aniso)
        print "quality: " + str(quality)
        print "shadows: " + str(shadows)
        '''

        xIniDisplay.SetGraphicsOptions(width, height, colordepth, windowed, tex_quality, antialias, aniso, quality, shadows, vsyncstr, shadow_quality)
        xIniDisplay.WriteIni()
        self.setNewChronicleVar("gamma", gamma)

        if setMode:
            PtSetGraphicsOptions(width, height, colordepth, windowed == "true", antialias, aniso, vsync)
            print "SETTING GAMMA"
            PtSetGamma2(gamma)
            PtSetShadowVisDistance(shadow_quality)

            if shadows:
                PtEnableShadows()
            else:
                PtDisableShadows()

    def GetVideoResList(self):
        windowed = ptGUIControlCheckBox(GraphicsSettingsDlg.dialog.getControlFromTag(kVideoWindowedCheckTag)).isChecked()

        vidResList = []

        possibleVidResList = copy.copy(kVideoResolutions)
        possibleVidResList.extend(kVideoExtraResolutions)

        if windowed:
            # get current display mode
            desktopWidth = PtGetDesktopWidth()
            desktopHeight = PtGetDesktopHeight()

            for res in possibleVidResList:
                if int(res[:res.find("x")]) < desktopWidth and int(res[(res.find("x") + 1):]) < desktopHeight:
                    vidResList.append(res)

        else:
            # get supported display resolutions
            supportedResList = PtGetSupportedDisplayModes()

            for res in supportedResList:
                resStr = str(res[0]) + "x" + str(res[1])
                if resStr in possibleVidResList:
                    vidResList.append(resStr)

        vidResList.sort(res_comp)

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

        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))

        xIniAudio.SetAudioMode( true, audio.getDeviceName(), EAXcheckbox.isChecked() )
        #xIniAudio.SetAudioMode( audio.isEnabled(), audio.getDeviceName(), EAXcheckbox.isChecked() )
        #xIniAudio.SetAudioMode( audio.isEnabled(), audio.getDeviceName(), audio.isUsingEAXAcceleration() )
        #xIniAudio.SetMicLevel( audio.getMicLevel() )
        xIniAudio.SetVoiceRecording( audio.isVoiceRecordingEnabled() )
        
        xIniAudio.WriteIni()

    def InitAudioControlsGUI(self):
        global prevAudioDeviceName
        
        xIniAudio.ReadIni()
        audio = ptAudioControl()

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

        audioField = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioMuteCheckBoxTag))
        audioField.setChecked( audio.isMuted() )

        EAXcheckbox = ptGUIControlCheckBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeCBID03))
        EAXcheckbox.setChecked( audio.isUsingEAXAcceleration() )
        EAXcheckbox.enable()
        respDisableItems.run(self.key, state="enableEAX")

        audioField = ptGUIControlKnob(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeID))
        numAudioDevices = audio.getNumAudioDevices() - 1.0

        if numAudioDevices > 0:
            for num in range(audio.getNumAudioDevices()):
                if audio.getAudioDeviceName(num) == audio.getDeviceName():
                    if not audio.supportsEAX(audio.getDeviceName()):
                        EAXcheckbox.disable()
                        respDisableItems.run(self.key, state="disableEAX")
                        EAXcheckbox.setChecked(false)
                        ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeEAXTextID)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))

                    audioField.setValue(num/numAudioDevices)
                    audioModeCtrlTextBox = ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeTextID))
                    audioDeviceName = prevAudioDeviceName = audio.getAudioDeviceName(num)
                    audioModeCtrlTextBox.setString(audioDeviceName)
        else:
            EAXcheckbox.disable()
            respDisableItems.run(self.key, state="disableEAX")
            EAXcheckbox.setChecked(false)
            ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeEAXTextID)).setForeColor(ptColor(0.839, 0.785, 0.695, 1))
            audioField.disable()
            audioModeCtrlTextBox = ptGUIControlTextBox(AudioSettingsDlg.dialog.getControlFromTag(kAudioModeTextID))
            audioModeCtrlTextBox.setString("None")
            audioModeCtrlTextBox.setForeColor(ptColor(0.839, 0.785, 0.695, 1))
                
    def setNewChronicleVar(self, chronicleVar, value):
        kModuleName = "Personal"
        kChronicleVarType = 0
        vault = ptVault()
        if type(vault) != type(None):
            entry = vault.findChronicleEntry(chronicleVar)
            if type(entry) == type(None):
                # not found... add current level chronicle
                vault.addChronicleEntry(chronicleVar,kChronicleVarType,str(value))
                print "%s:\tentered new chronicle counter %s" % (kModuleName,chronicleVar)
            else:
                entry.chronicleSetValue(str(value))
                entry.save()
                print "%s:\tyour current value for %s is %s" % (kModuleName,chronicleVar,entry.chronicleGetValue())
        else:
            PtDebugPrint("%s:\tERROR trying to access vault -- can't update %s variable in chronicle." % (kModuleName,chronicleVar))

    def getChronicleVar(self, chronicleVar):
        kModuleName = "Personal"
        kChronicleVarType = 0
        vault = ptVault()
        if type(vault) != type(None):
            entry = vault.findChronicleEntry(chronicleVar)
            print "getChronicleVar.chronicleVar: " + chronicleVar
            #print "getChronicleVar.Entry: " , entry
            if type(entry) == type(None):
                # not found... add current level chronicle
                #vault.addChronicleEntry(chronicleVar,kChronicleVarType,"%d" %(0))
                #PtDebugPrint("%s:\tentered new chronicle counter %s" % (kModuleName,chronicleVar))
                return None
            else:
                value = entry.chronicleGetValue()
                print "getChronicleVar(): " + chronicleVar + " = " + value
                return value
        else:
            PtDebugPrint("%s:\tERROR trying to access vault -- can't retrieve %s variable in chronicle." % (kModuleName,kChronicleVarName))
        
    def IRefreshHelpSettings(self):
        clickToTurn = ptGUIControlRadioGroup(NavigationDlg.dialog.getControlFromTag(kNormNoviceRGID))
        clickToTurn.setValue(PtIsClickToTurn())


    def IRefreshAdvSettings(self):
        "refresh the volume settings to the current settings"
        # shadows
        
        # We'll have to do this later, since it's no longer part of the AdvDisplaySettings Dialog
        #~ shadowDistKnob = ptGUIControlValue(AdvGameSettingDlg.dialog.getControlFromTag(kGSDisplayShadowDistSlider))
        #~ setting = PtGetShadowVisDistance()
        #~ shadowDistKnob.setValue(setting*10.0)
        
        #Will have to wire this into Audio panel later
        #~ soundPriKnob = ptGUIControlValue(AdvGameSettingDlg.dialog.getControlFromTag(kGSSoundPrioritySlider))
        #~ audio = ptAudioControl()
        #~ soundPriKnob.setValue(audio.getPriorityCutoff()*(11.0/10.0))
        
        mouseSensKnob = ptGUIControlValue(AdvGameSettingDlg.dialog.getControlFromTag(kGSMouseTurnSensSlider))
        print "IRefreshAdvSettings: PtGetMouseTurnSensitivity() = %d" % (PtGetMouseTurnSensitivity())
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
                            
        pukeCamCheckbox = ptGUIControlCheckBox(AdvGameSettingDlg.dialog.getControlFromTag(kGSPukeCamCheckbox))
        cam = ptCamera()
        pukeCamCheckbox.setChecked(cam.isSmootherCam())
        mouseInvertCheckbox = ptGUIControlCheckBox(AdvGameSettingDlg.dialog.getControlFromTag(kGSMouseInvertCheckbox))
        mouseInvertCheckbox.setChecked(PtIsMouseInverted())
        walkAndPanCheckbox = ptGUIControlCheckBox(AdvGameSettingDlg.dialog.getControlFromTag(kGSWalkAndPan))
        walkAndPanCheckbox.setChecked(cam.isWalkAndVerticalPan())
        stayInFirstCheckbox = ptGUIControlCheckBox(AdvGameSettingDlg.dialog.getControlFromTag(kGSStayInFirstPerson))
        stayInFirstCheckbox.setChecked(cam.isStayInFirstPerson())
        clickToTurnCheckbox = ptGUIControlCheckBox(AdvGameSettingDlg.dialog.getControlFromTag(kGSClickToTurn))
        clickToTurnCheckbox.setChecked(PtIsClickToTurn())


    def LoadAdvSettings(self):
        global gMouseSensitivity
        global gSmoothCam
        global gMouseInvert
        global gWalkAndPan
        global gStayInFirstPerson
        global gClickToTurn
        import string

        AdvSettingsString = self.getChronicleVar("AdvSettings")
        AdvSettingsArray = []

        if AdvSettingsString == None:
            # create settings
            self.setNewChronicleVar("AdvSettings", "150 0 0 0 0 0")
            AdvSettingsArray = ["150","0","0","0","0","0"]
        else:
            AdvSettingsArray = AdvSettingsString.split()

        value = string.atoi(AdvSettingsArray[0])
        PtSetMouseTurnSensitivity(float(value))
        gMouseSensitivity = AdvSettingsArray[0]

        value = string.atoi(AdvSettingsArray[1])
        cam = ptCamera()
        cam.setSmootherCam(value)
        gSmoothCam = AdvSettingsArray[1]

        value = string.atoi(AdvSettingsArray[2])
        gMouseInvert = AdvSettingsArray[2]
        if value:
            PtSetMouseInverted()
        else:
            PtSetMouseUninverted()

        value = string.atoi(AdvSettingsArray[3])
        cam = ptCamera()
        cam.setWalkAndVerticalPan(value)
        gWalkAndPan = AdvSettingsArray[3]

        value = string.atoi(AdvSettingsArray[4])
        cam = ptCamera()
        cam.setStayInFirstPerson(value)
        gStayInFirstPerson = AdvSettingsArray[4]

        value = string.atoi(AdvSettingsArray[5])
        PtSetClickToTurn(value)
        gClickToTurn = AdvSettingsArray[5]

    def LoadKeyMap(self):
        km = ptKeyMap()
        KeyMapString = self.getChronicleVar("KeyMap")
        KeyMapArray = KeyMapString.split()
        counter = 0
        # set the key binds back to the saved
        for control_code in defaultControlCodeBindsOrdered:
            if type(control_code) == type(""):
                key1 = KeyMapArray[counter]
                print "Binding " + key1 + " to " + control_code
                km.bindKeyToConsoleCommand(key1,control_code)
            else:
                controlStr = km.convertControlCodeToString(control_code)
                SubArray = KeyMapArray[counter].split("$")
                key1 = SubArray[0]
                key2 = SubArray[1]
                print "Binding " + key1 + " & " + key2 + " to " + controlStr
                km.bindKey(key1,key2,controlStr)
            counter += 1

    def IsThereACover(self,bookHtml):
        # search the bookhtml string looking for a cover
        idx = bookHtml.find('<cover')
        if idx >= 0:
            return 1
        return 0

    def IShowMappedKeys(self,dlg,mapRow1,mapRow2):
        km = ptKeyMap()
        for cID in mapRow1.keys():
            field = ptGUIControlEditBox(dlg.getControlFromTag(cID))
            field.setSpecialCaptureKeyMode(1)
            # set the mapping
            controlCode,spFlag,mpFlag = mapRow1[cID]
            if type(controlCode) != type(None) and ( ( spFlag and PtIsSinglePlayerMode() ) or ( mpFlag and not PtIsSinglePlayerMode() ) ):
                # is the control code a console command?
                if type(controlCode) == type(""):
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
            if type(controlCode) != type(None) and ( ( spFlag and PtIsSinglePlayerMode() ) or ( mpFlag and not PtIsSinglePlayerMode() ) ):
                # is the control code a console command?
                if type(controlCode) == type(""):
                    # this shouldn't really happen!
                    field.setLastKeyCapture(km.getBindingKeyConsole(controlCode),km.getBindingFlagsConsole(controlCode))
                # else must be a event binding
                else:
                    field.setLastKeyCapture(km.getBindingKey2(controlCode),km.getBindingFlags2(controlCode))
            else:
                # disable this field
                field.hide()

    # SHOULD BE OK TO REMOVE - IF YOU NEED IT, TALK TO DEREK
    #def IMatchIniToGame(self):
    #    km = ptKeyMap()
    #    # get the keycodes for each of the control codes
    #    for control_code in defaultControlCodeBinds.keys():
    #        if type(control_code) == type(""):
    #            key1 = km.convertVKeyToChar(km.getBindingKeyConsole(control_code),km.getBindingFlagsConsole(control_code))
    #            self.setNewChronicleVar(control_code, key1)
    #            #xIniInput.SetConsoleKey('"'+control_code+'"',key1+',')
    #        else:
    #            controlStr = km.convertControlCodeToString(control_code)
    #            key1 = km.convertVKeyToChar(km.getBindingKey1(control_code),km.getBindingFlags1(control_code))
    #            key2 = km.convertVKeyToChar(km.getBindingKey2(control_code),km.getBindingFlags2(control_code))
    #            self.setNewChronicleVar(controlStr, key1)
    #            self.setNewChronicleVar(controlStr+"2", key2)
    #            #xIniInput.SetControlKey('"'+controlStr+'"',key1+',',key2+',')

    def ISetDefaultKeyMappings(self):
        km = ptKeyMap()
        KeyMapString = ""
        # set the key binds back to the defaults
        for control_code in defaultControlCodeBindsOrdered:
            if type(control_code) == type(""):
                key1 = defaultControlCodeBinds[control_code][0]
                km.bindKeyToConsoleCommand(key1,control_code)
                KeyMapString += key1 + " "
            else:
                controlStr = km.convertControlCodeToString(control_code)
                key1, key2 = defaultControlCodeBinds[control_code]
                km.bindKey(key1,key2,controlStr)
                KeyMapString += key1 + "$" + key2 + " "

        self.setNewChronicleVar("KeyMap", KeyMapString.rstrip())

def res_comp(elem1, elem2):
    elem1w = int(elem1[:elem1.find("x")])
    elem1h = int(elem1[elem1.find("x") + 1:])
    elem2w = int(elem2[:elem2.find("x")])
    elem2h = int(elem2[elem2.find("x") + 1:])

    result = cmp(elem1w, elem2w)
    if result == 0:
        return cmp(elem1h, elem2h)
    else:
        return result


def urlencode(query,doseq=0):
    """Encode a sequence of two-element tuples or dictionary into a URL query string.

    If any values in the query arg are sequences and doseq is true, each
    sequence element is converted to a separate parameter.

    If the query arg is a sequence of two-element tuples, the order of the
    parameters in the output will match the order of parameters in the
    input.
    """

    if hasattr(query,"items"):
        # mapping objects
        query = query.items()
    else:
        # it's a bother at times that strings and string-like objects are
        # sequences...
        try:
            # non-sequence items should not work with len()
            x = len(query)
            # non-empty strings will fail this
            if len(query) and type(query[0]) != types.TupleType:
                raise TypeError
            # zero-length sequences of all types will get here and succeed,
            # but that's a minor nit - since the original implementation
            # allowed empty dicts that type of behavior probably should be
            # preserved for consistency
        except TypeError:
            ty,va,tb = sys.exc_info()
            raise TypeError, "not a valid non-string sequence or mapping object", tb

    l = []
    if not doseq:
        # preserve old behavior
        for k, v in query:
            k = quote_plus(str(k))
            v = quote_plus(str(v))
            l.append(k + '=' + v)
    else:
        for k, v in query:
            k = quote_plus(str(k))
            if type(v) == types.StringType:
                v = quote_plus(v)
                l.append(k + '=' + v)
            elif type(v) == types.UnicodeType:
                # is there a reasonable way to convert to ASCII?
                # encode generates a string, but "replace" or "ignore"
                # lose information and "strict" can raise UnicodeError
                v = quote_plus(v.encode("ASCII","replace"))
                l.append(k + '=' + v)
            else:
                try:
                    # is this a sufficient test for sequence-ness?
                    x = len(v)
                except TypeError:
                    # not a sequence
                    v = quote_plus(str(v))
                    l.append(k + '=' + v)
                else:
                    # loop over the sequence
                    for elt in v:
                        l.append(k + '=' + quote_plus(str(elt)))
    return '&'.join(l)

def quote_plus(s, safe = ''):
    """Quote the query fragment of a URL; replacing ' ' with '+'"""
    if ' ' in s:
        l = s.split(' ')
        for i in range(len(l)):
            l[i] = quote(l[i], safe)
        return '+'.join(l)
    else:
        return quote(s, safe)

def quote(s, safe = '/'):
    """quote('abc def') -> 'abc%20def'

    Each part of a URL, e.g. the path info, the query, etc., has a
    different set of reserved characters that must be quoted.

    RFC 2396 Uniform Resource Identifiers (URI): Generic Syntax lists
    the following reserved characters.

    reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
                  "$" | ","

    Each of these characters is reserved in some component of a URL,
    but not necessarily in all of them.

    By default, the quote function is intended for quoting the path
    section of a URL.  Thus, it will not encode '/'.  This character
    is reserved, but in typical usage the quote function is being
    called on a path where the existing slash characters are used as
    reserved characters.
    """
    safe = always_safe + safe
    if _fast_safe_test == safe:
        return _fast_quote(s)
    res = list(s)
    for i in range(len(res)):
        c = res[i]
        if c not in safe:
            res[i] = '%%%02X' % ord(c)
    return ''.join(res)

always_safe = ('ABCDEFGHIJKLMNOPQRSTUVWXYZ'
               'abcdefghijklmnopqrstuvwxyz'
               '0123456789' '_.-')
_fast_safe_test = always_safe + '/'
_fast_safe = None
