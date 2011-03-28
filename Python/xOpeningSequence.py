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
"""Module: xOpeningSequnce
Age: global
Author: Mark DeForest
Date: Aug. 28, 2003
This is the python handler for the Opening Sequence in the Cleft
"""
MaxVersionNumber = 1
MinorVersionNumber = 5

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaConstants import *

import PlasmaControlKeys
import os

import xIniDisplay
import xOptionsMenu


# define the attributes that will be entered in max
IntroMovieDlg = ptAttribGUIDialog(1,"The Intro Movie dialog")
FirstHelpDlg = ptAttribGUIDialog(2, "The First Help dialog")
OrientationDlg = ptAttribGUIDialog(3, "The Orientation dialog")


# globals
#--------

gIntroMovie = None
kAtrusIntroMovie = "avi/URULiveIntro.bik"

gIntroStarted = 0

gOriginalAmbientVolume = 1.0
gOriginalSFXVolume = 1.0
gTotalTickTime = 20.0
gCurrentTick = 0.0

# if we have to start the intro movie by the timer
gIntroByTimer = 0

# constants
#----------
kIntroPauseID = 1
kIntroPauseSeconds = 3.0

kIntroFadeOutID = 2
kIntroFadeOutSeconds = 2.0
kHelpFadeInSeconds = 3.0

kOrientationExitID = 3
kOrientationExitSeconds = 0

kStartGameFadeOutID = 4
kStartGameFadeOutSeconds = 0.5
kStartGameFadeInSeconds = 0.5

kFakeMovieTimeID = 50
kFakeMovieSeconds = 10.0

kSoundFadeInID = 99
kSoundTickTime = 0.10

kIntroPlayedChronicle = "IntroPlayed"

# tag ids for dialog components
#kOrientationOkBtn = 210
kFirstHelpOkBtn = 310
kNormNoviceRGID = 700

#---------------
# These are the old tags!
# tags for the text in the Startup Help screen
#kWelcomeText = 200
#kSomeHelpText = 210
#kForMoreText = 220
#kOkBtnText = 300

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
kOkButton = 650


class xOpeningSequence(ptModifier):
    "The Opening Sequence handler"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 194
        self.version = MaxVersionNumber
        PtDebugPrint("__xOpeningSequence: Max version %d - minor version %d" % (MaxVersionNumber,MinorVersionNumber))

    def OnFirstUpdate(self):
        "First update, load our dialogs"
        global gCurrentTick
        global gIntroByTimer
        global gOriginalAmbientVolume
        global gOriginalSFXVolume
        PtLoadDialog("IntroMovieGUI",self.key)
        PtLoadDialog("OrientationGUI",self.key)
        PtLoadDialog("StartupHelpGUI",self.key)
        PtLoadDialog("IntroBahroBgGUI",self.key)
        gCurrentTick = 0
        try:
            avatar = PtGetLocalAvatar()
            avatar.avatar.registerForBehaviorNotify(self.key)
            gIntroByTimer = 0
        except:
            PtDebugPrint("xOpeningSequence failed to get local avatar")
            gIntroByTimer = 1
            return
        # stop rendering the scene
        PtDisableRenderScene()
        # turn off the cursor
        PtGUICursorOff()
        # turn down the ambient sound
        audio = ptAudioControl()
        gOriginalAmbientVolume = audio.getAmbienceVolume()
        audio.setAmbienceVolume(0.0)
        gOriginalSFXVolume = audio.getSoundFXVolume()
        audio.setSoundFXVolume(0.0)

    def __del__(self):
        "the destructor - dialogs unload somewhere else... in IStartGame()"
        PtDebugPrint("xOpeningSequence::destructor... we're gone!",level=kDebugDumpLevel)

    def AvatarPage(self,sobj,unload,lastout):
        pass

    def OnGUINotify(self,id,control,event):
        "Events from the intro movie and the first help..."
        global gOriginalAmbientVolume
        global gOriginalSFXVolume
        global gIntroMovie
        PtDebugPrint("xOpeningSequence::OnGUINotify id=%d, event=%d control=" % (id,event),control,level=kDebugDumpLevel)
###############################################
##
##  IntroMovie dialog processing
##
###############################################
        if id == IntroMovieDlg.id:
            if event == kDialogLoaded:
                pass
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    pass
            elif event == kAction or event == kValueChanged:
                # test to see which control had the event
                imID = control.getTagID()
                pass
            elif event == kExitMode:
                self.IStartOrientation()
###############################################
##
##  First Help dialog processing
##
###############################################
        elif id == OrientationDlg.id:
            if event == kDialogLoaded:
                # see if the there actually is a movie to play
                skipMovie = 1
                try:
                    os.stat(kAtrusIntroMovie)
                    # its there! show the background, which will start the movie
                    PtShowDialog("IntroBahroBgGUI")
                    skipMovie = 0
                except:
                    skipMovie = 1
                if skipMovie:
                    # no movie... just show the help screen
                    # start rendering the scene again
                    PtEnableRenderScene()
                    PtGUICursorOn()
                    Orientation.dialog.show()
                    PtDebugPrint("xOpeningSequence - no intro movie!!!",level=kDebugDumpLevel)
            elif event == kAction or event == kValueChanged:
                orientationID = control.getTagID()
                if orientationID == kFirstHelpOkBtn:
                    self.IStartHelp()
            elif event == kExitMode:
                self.IStartHelp()
        elif id == FirstHelpDlg.id:
            if event == kDialogLoaded:
                PtDebugPrint("xOpeningSequence - quiet sounds and show background",level=kDebugDumpLevel)
                # this SHOULD be in the max file, but since someone has the KI max file tied up, it will have to go here
                # set the text localized strings
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kHelpTitle))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Title"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kWalkText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Walk"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kRunText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Run"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kTurnLeftText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.TurnLeft"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kBackwardsText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.WalkBackwards"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kTurnRightText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.TurnRight"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kToggleViewText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.ToggleView"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kJumpText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.KeyCommands.Jump"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kMouseWalkText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Walk"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kMouseRunText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Run"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kSelectText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Select"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kMousePanCam))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.MousePanCam"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kMouseBackwards))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.WalkBackwards"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kMousePresetsTitle))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.MousePresets"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kMouseNormalText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Normal"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kMouseNoviceText))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.StartupHelp.Novice"))
                textField = ptGUIControlTextBox(FirstHelpDlg.dialog.getControlFromTag(kOkButton))
                textField.setStringW(PtGetLocalizedString("OptionsMenu.Main.Ok"))
                # hide the ok button until they agree to the terms... or pick normal or novice
                ##textField.setString(" ")
                ##okBtn = ptGUIControlButton(FirstHelpDlg.dialog.getControlFromTag(kFirstHelpOkBtn))
                ##okBtn.hide()
            elif event == kShowHide:
                # reset the edit text lines
                if control.isEnabled():
                    nnRG = ptGUIControlRadioGroup(FirstHelpDlg.dialog.getControlFromTag(kNormNoviceRGID))
                    if PtIsClickToTurn():
                        nnRG.setValue(1)
                    else:
                        nnRG.setValue(0)
                    xIniDisplay.ReadIni()
                else:
                    xIniDisplay.WriteIni()
            elif event == kAction or event == kValueChanged:
                helpID = control.getTagID()
                if helpID == kFirstHelpOkBtn:
                    # get the setting of the novice/normal radio group
                    nnRG = ptGUIControlRadioGroup(FirstHelpDlg.dialog.getControlFromTag(kNormNoviceRGID))
                    vault = ptVault()
                    entry = vault.findChronicleEntry(kIntroPlayedChronicle)
                    if nnRG.getValue() == 1:
                        PtSetClickToTurn(1)
                        #xIniDisplay.SetClickToTurn()
                        vault.addChronicleEntry("ClickToTurn",2,"yes")
                    else:
                        PtSetClickToTurn(0)
                        #xIniDisplay.RemoveClickToTurn()
                        vault.addChronicleEntry("ClickToTurn",2,"no")
                    self.IStartGame()
            elif event == kExitMode:
                self.IStartGame()
        elif id == -1:
            if event == kShowHide:
                if control.isEnabled():
                    gIntroMovie = ptMoviePlayer(kAtrusIntroMovie,self.key)
                    gIntroMovie.playPaused()
                    if gIntroByTimer:
                        PtAtTimeCallback(self.key, kIntroPauseSeconds, kIntroPauseID)

    def OnBehaviorNotify(self,type,id,state):
        global playScene
        PtDebugPrint("xOpeningSequence.OnBehaviorNotify(): %d" % (type))
        if type == PtBehaviorTypes.kBehaviorTypeLinkIn and not state:
            self.IStartMovie()
            avatar = PtGetLocalAvatar()
            avatar.avatar.unRegisterForBehaviorNotify(self.key)

    def OnTimer(self,id):
        if id == kIntroPauseID:
            self.IStartMovie()
        elif id == kIntroFadeOutID:
            self.IFinishStartOrientation()
        elif id == kOrientationExitID:
            self.IFinishStartHelp()
        elif id == kStartGameFadeOutID:
            self.IFinishStartGame()
        elif id == kSoundFadeInID:
            self.IUpdateSounds()

    def OnMovieEvent(self,movieName,reason):
        PtDebugPrint("xOpeningSequence: movie done ",level=kDebugDumpLevel)
        if gIntroMovie:
            self.IStartOrientation()


    def IStartMovie(self):
        global gIntroMovie
        global gIntroStarted
        if gIntroMovie:
            if not gIntroStarted:
                # show the dialog
                IntroMovieDlg.dialog.show()
                gIntroMovie.resume()
                PtDebugPrint("xOpeningSequence - playing movie",level=kDebugDumpLevel)
            else:
                PtDebugPrint("xOpeningSequence - movie already playing",level=kDebugDumpLevel)
            gIntroStarted = 1


    def IStartOrientation(self):
        "Inserting this new orientation GUI between movie and help GUI"
        PtFadeOut(kIntroFadeOutSeconds,1)
        PtAtTimeCallback(self.key, kIntroFadeOutSeconds, kIntroFadeOutID)


    def IFinishStartOrientation(self):
        "we've faded out, now show orientation and start fade in"
        global gIntroMovie
        PtHideDialog("IntroBahroBgGUI")
        if gIntroMovie:
            gIntroMovie.stop()
            gIntroMovie = None
        IntroMovieDlg.dialog.hide()
        # start rendering the scene again
        PtEnableRenderScene()
        PtGUICursorOn()
        OrientationDlg.dialog.show()
        PtFadeIn(kHelpFadeInSeconds,0)


    def IStartHelp(self):
        "One more screen before they start the game... when will it ever stop!"
        #PtFadeOut(kIntroFadeOutSeconds,1)
        OrientationDlg.dialog.hide()
        PtAtTimeCallback(self.key, kOrientationExitSeconds, kOrientationExitID)


    def IFinishStartHelp(self):
        "we've faded out, now show help and start fade in"
        FirstHelpDlg.dialog.show()
        #PtFadeIn(kHelpFadeInSeconds,0)


    def IStartGame(self):
        "Start the game... let them play!"
        # 1) set chronicle variable to say they've complete intro
        vault = ptVault()
        entry = vault.findChronicleEntry(kIntroPlayedChronicle)
        if type(entry) != type(None):
            entry.chronicleSetValue("yes")
            entry.save()
        else:
            vault.addChronicleEntry(kIntroPlayedChronicle,2,"yes")
        # 2) fade screen up.. or something....? maybe
        #### don't fade for now...
        #PtFadeOut(kStartGameFadeOutSeconds,1)
        #PtAtTimeCallback(self.key, kStartGameFadeOutSeconds, kStartGameFadeOutID)
        self.IFinishStartGame()

    def IFinishStartGame(self):
        # 3) hide the help dialog
        FirstHelpDlg.dialog.hide()
        # 3.5) ambient sound back to original
        PtAtTimeCallback(self.key, kSoundTickTime, kSoundFadeInID)
        # 4) re-enable KI and blackbar
        # enable twice because if we came from the ACA (closet->ACA->personal) it was disabled twice
        PtSendKIMessage(kEnableKIandBB,0)
        PtSendKIMessage(kEnableKIandBB,0)
        # enable yeesha book in case we came from the bahro cave
        PtSendKIMessage(kEnableYeeshaBook,0)
        # 5) bring up the lights and let them play
        ### don't fade in (unless you faded out first)
        #PtFadeIn(kStartGameFadeInSeconds,0)
###        # finally - unload ouselves...
###        PtUnloadDialog("StartupHelpGUI")
###        PtUnloadDialog("IntroMovieGUI")   # ... we may have to have some else unload us(?)

    def IUpdateSounds(self):
        global gTotalTickTime
        global gCurrentTick
        audio = ptAudioControl()
        gCurrentTick += 1
        if gCurrentTick >= gTotalTickTime:
            # we're done, be sure the volumes are at the fullest
            audio.setAmbienceVolume(gOriginalAmbientVolume)
            audio.setSoundFXVolume(gOriginalSFXVolume)
            # and finally - unload ouselves...
            PtUnloadDialog("StartupHelpGUI")
            PtUnloadDialog("OrientationGUI")
            PtUnloadDialog("IntroMovieGUI")   # ... we may have to have some else unload us(?)
        else:
            audio.setAmbienceVolume(gOriginalAmbientVolume*(gCurrentTick/gTotalTickTime))
            audio.setSoundFXVolume(gOriginalSFXVolume*(gCurrentTick/gTotalTickTime))
            PtAtTimeCallback(self.key, kSoundTickTime, kSoundFadeInID)
