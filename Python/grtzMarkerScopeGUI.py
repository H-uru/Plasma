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
"""Module: grtzMarkerScopeGUI
Age: Great Zero
Author: Mark DeForest
Date: Dec. 8, 2003

Re-design: 
---->Author: Tye Hooley
---->Date: Dec 2006

This is the python handler for the MarkerGame Machine (CGZs) GUI interface handler
"""

MaxVersionNumber = 2
MinorVersionNumber = 1

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *

import string
import time

import grtzMarkerGames
from PlasmaGameConstants import *
import PlasmaControlKeys




# define the attributes that will be entered in max
MarkerGameDlg = ptAttribGUIDialog(1,"The MarkerGame GUI")
MGAnim = ptAttribAnimation(2,"Turn on/off entire animation")
MGMachineOnResp = ptAttribResponder(3,"Turn On responder")
MGMachineOffResp = ptAttribResponder(4,"Turn Off responder")
aScope1Act = ptAttribNamedActivator(5,"CGZ Scope1 Python", netForce=1)
aScope2Act = ptAttribNamedActivator(6,"CGZ Scope2 Python", netForce=1)
aScope3Act = ptAttribNamedActivator(7,"CGZ Scope3 Python", netForce=1)
aScope4Act = ptAttribNamedActivator(8,"CGZ Scope4 Python", netForce=1)

# constants
kBogusGameName = "bogusGame"

# TagIDs for the GUI
kRGMarkerGameSelect = 500
kMarkerGameFieldStart = 200
kMarkerGameFieldEnd = 340
kMarkerGameNumFieldOffset = 0
kMarkerGameNameFieldOffset = 1

#Timer Callback Types
kDisplayTimer = 0

#Score Refresh types
kNoRefresh          = 0
kFullRefresh        = 1
kPartialRefresh     = 2
kOneTimeFullRefresh = 3


class grtzMarkerScopeGUI(ptModifier):
    "The MarkerScope get a game GUI handler"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 213
        self.version = MaxVersionNumber
        self.selectorChanged = 0
        self.init = 0
        self.lastGame = -1
        self.lastGameScore = -1
        self.updateLevel = kNoRefresh
        self.scopeID = -1
        self.debug = 0
        PtDebugPrint("__grtzMarkerScopeGUI: Max version %d - minor version %d" % (MaxVersionNumber,MinorVersionNumber))

    def __del__(self):
        "the destructor - unload any dialogs we loaded"
        PtUnloadDialog("MarkerGameGUI")

    def OnFirstUpdate(self):
        "First update, load our dialogs"
        PtLoadDialog("MarkerGameGUI",self.key)

    def OnNotify(self,state,id,events):
        "Activated... start telescope"
        #~PtDebugPrint("grtzMarkerScopeGUI:OnNotify  state=%f id=%d events=" % (state,id),events,level=kDebugDumpLevel)
        if id == (-1):
            print "Tye: YEA!! IT WORKED!"
            return
        
        if id == aScope1Act.id or id == aScope2Act.id or id == aScope3Act.id or id == aScope4Act.id:
            self.scopeID = id
            if PtDetermineKIMarkerLevel() < kKIMarkerNormalLevel:
                MarkerGameDlg.dialog.hide()
            else:
                MGMachineOffResp.run(self.key,netPropagate=0)
        elif id == MGMachineOffResp.id:
            MarkerGameDlg.dialog.hide()
            actList = (aScope1Act, aScope2Act, aScope3Act, aScope4Act)
            for act in actList:
                if act.id == self.scopeID:
                    self.scopeID = -1
                    act.enable()
                    PtDisableControlKeyEvents(self.key)
                    return
        elif id == MGMachineOnResp.id:
            PtEnableControlKeyEvents(self.key)

    def OnGUINotify(self,id,control,event):
        "Events from all the dialogs in the MarkerGameGUI Age..."
        if id != MarkerGameDlg.id:
            return

        #~PtDebugPrint("grtzMarkerScopeGUI::OnGUINotify id=%d, event=%d control=" % (id,event),control)#,level=kDebugDumpLevel )
        if event == kDialogLoaded:
            PtDebugPrint("DEBUG: grtzMarkerScopeGUI():\tMarker Scope GUU--->Dialog Loaded!")
            # hide stuff until needed
            MGAnim.animation.skipToTime(1.5)

        elif event == kShowHide:
            if control.isEnabled():
                PtDebugPrint("DEBUG: grtzMarkerScopeGUI():\tMarker Scope-GUI--->Show Dialog")
                # make sure the player has enough level to see the GUI
                gameType = grtzMarkerGames.GetCurrentGameType()
                if PtDetermineKIMarkerLevel() < kKIMarkerNormalLevel:
                    PtDebugPrint("DEBUG: grtzMarkerScopeGUI.OnGUINotify():\tKI Level not high enough, disabling GUI controls")
                    gameSelector = ptGUIControlRadioGroup(MarkerGameDlg.dialog.getControlFromTag(kRGMarkerGameSelect))
                    gameSelector.setValue(-1)
                    gameSelector.disable()
                    self.updateLevel = kNoRefresh
                elif gameType > -1 and gameType != PtMarkerGameTypes.kMarkerGameCGZ:
                    PtDebugPrint("DEBUG: grtzMarkerScopeGUI.OnGUINotify():\tUser Created Marker Game in Progress, disabling GUI controls")
                    gameSelector = ptGUIControlRadioGroup(MarkerGameDlg.dialog.getControlFromTag(kRGMarkerGameSelect))
                    gameSelector.setValue(-1)
                    gameSelector.disable()
                    self.updateLevel = kNoRefresh
                    msg = "Please quit existing User-Created Marker Game, to play a CGZ Game."
                    PtSendKIMessage(kKILocalChatStatusMsg, msg)
                    self.IRefreshGamesDisplay()
                    MGMachineOnResp.run(self.key,netPropagate=0)
                else: #We're loading the GUI
                    # Get gameData (if Any)
                    gameNum,numCaptured,numMarkers = grtzMarkerGames.GetGameProgress()
                    PtDebugPrint("DEBUG: grtzMarkerScopeGUI.OnGUINotify():\tInitializing GUI, current gameNum = %s" % gameNum)
                    
                    #Set refresh level to not update, later we'll change it if necessary...
                    self.updateLevel = kNoRefresh
                    self.lastGame = -1
                    self.lastGameScore = -1

                    if gameNum > -1 and numCaptured > -1 and numMarkers > -1:
                        if numCaptured == numMarkers:
                            #We've got a completed game!!!
                            #Stop the game....
                            PtSendKIMessage(kMGStopCGZGame, 0)

                            #Save the game information
                            self.lastGame = gameNum
                            self.lastGameScore = grtzMarkerGames.GetGameScore(gameNum)

                            #Now set the selector to unselected...
                            gameNum = -1

                            #Setup the timer to refresh the display
                            self.updateLevel = kPartialRefresh


                    # Set selector to the game being played (if Any)
                    gameSelector = ptGUIControlRadioGroup(MarkerGameDlg.dialog.getControlFromTag(kRGMarkerGameSelect))
                    gameSelector.enable()
                    oldGame = gameSelector.getValue()
                    if gameNum != oldGame:
                        gameSelector.setValue(gameNum)
                        self.init = 0
                    else:
                        #We've already initialized properly, no need to do it again!
                        self.init = 1

                    if gameNum > -1:
                        # start/continue the counting                     
                        self.updateLevel = kPartialRefresh

                    #Refresh Display
                    self.IRefreshGamesDisplay()
                    MGMachineOnResp.run(self.key,netPropagate=0)

                    if self.updateLevel > kNoRefresh:
                        PtAtTimeCallback(self.key,1,kDisplayTimer)
            else:
                PtDebugPrint("DEBUG: grtzMarkerScopeGUI():\tMarker Scope-GUI--->Hide dialog")
                #Hide scope...    
                MGAnim.animation.skipToTime(1.5)
                self.lastGame = -1
                self.lastGameScore = -1

                #Stop the timer callbacks...
                self.updateLevel = kNoRefresh

                #Get the game selected
                gameSelector = ptGUIControlRadioGroup(MarkerGameDlg.dialog.getControlFromTag(kRGMarkerGameSelect))
                reqGameNum = gameSelector.getValue()
                curGameNum = grtzMarkerGames.GetCurrentCGZGame()

                #Start the game if necessary
                if self.selectorChanged:
                    #Exit if somehow we really don't have a valid game!
                    if reqGameNum < 0 or reqGameNum > len(grtzMarkerGames.mgs):
                        PtDebugPrint("ERROR: grtzMarkerScopeGUI.OnGUINotify:\tCannot start invalid game number: %d" %reqGameNum)
                        return
                    
                    #Finally, start the new game!
                    PtDebugPrint("DEBUG: grtzMarkerScopeGUI.OnGUINotify():\tReceived GUI notification to start game: %s" %reqGameNum)
                    PtSendKIMessageInt(kMGStartCGZGame, reqGameNum)

                    #Reset selector sensor
                    self.selectorChanged = 0

        elif event == kValueChanged:
            rgid = control.getTagID()
            if rgid == kRGMarkerGameSelect:
                #The first time we update the selector, we don't want to register a new game selected!!!
                #All played games are initialized upon age load, not upon marker scope GUI load....
                if self.init == 0:
                    PtDebugPrint("DEBUG: grtzMarkerScopeGUI.OnGUINotify():\tFinished initialization.... (ignoring selector change)")
                    self.init = 1  #Make sure we don't try to re-init!
                    return

                self.selectorChanged = 1  #let us know if the selector changed for (so we can start/stop a game if necessary)

                gameNum = control.getValue()
                PtDebugPrint("DEBUG: grtzMarkerScopeGUI.OnGUINotify():\tReceived CGZ Game Selector change: %s" %gameNum)


                #If we were playing an existing game, we need to quit it!!!
                curGame = grtzMarkerGames.GetCurrentCGZGame()
                if curGame != None and curGame > -1:
                    PtDebugPrint("DEBUG: grtzMarkerScopeGUI.OnGUINotify():\tSelector value changed, stopping current game!")
                    PtSendKIMessage(kMGStopCGZGame,0)
                    self.updateLevel = kOneTimeFullRefresh
                
                #Reset the score if we've seleced a game
                prevRefresh = self.updateLevel
                if gameNum > -1 and gameNum < len(grtzMarkerGames.mgs):
                    grtzMarkerGames.UpdateScore(gameNum,PtGetDniTime(),0.0)
                    self.updateLevel = kPartialRefresh
                else:
                    #just need to refresh the entire display to ensure we've stopped any timer displays
                    self.updateLevel = kOneTimeFullRefresh                    
                    pass

                #Force one update...
                self.IRefreshGamesDisplay()

                #Update display... only if we didn't have plans to update...
                if prevRefresh == kNoRefresh:
                    PtAtTimeCallback(self.key,0 ,kDisplayTimer)

        
    def OnTimer(self,id):
        "timer event, update things"
        if id == kDisplayTimer:
            if self.updateLevel == kFullRefresh:
                self.IRefreshGamesDisplay()
            elif self.updateLevel == kPartialRefresh:
                self.IPartialRefreshDisplay()
            elif self.updateLevel == kOneTimeFullRefresh:
                self.IRefreshGamesDisplay()

                #Stop the timer if we don't have a new game selected....
                gameSelector = ptGUIControlRadioGroup(MarkerGameDlg.dialog.getControlFromTag(kRGMarkerGameSelect))
                curGame = gameSelector.getValue()
                if curGame < 0:
                    self.updateLevel = kNoRefresh
                else:
                    self.updateLevel = kPartialRefresh

            #Call our next refresh (if necessary)
            if self.updateLevel > kNoRefresh:
                PtAtTimeCallback(self.key,1,kDisplayTimer)


    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IQuitScope()
        elif controlKey == PlasmaControlKeys.kKeyMoveBackward or controlKey == PlasmaControlKeys.kKeyRotateLeft or controlKey == PlasmaControlKeys.kKeyRotateRight:
            self.IQuitScope()
 

    def IQuitScope(self):
        scopeNames = ("GZMachineScope01","GZMachineScope02","GZMachineScope03","GZMachineScope04")
        notify = ptNotify(self.key)
        notify.clearReceivers()
       
        for name in scopeNames:
            try: 
                obj = PtFindSceneobject(name, "GreatZero")
                pythonScripts = obj.getPythonMods()
                for script in pythonScripts:
                    notify.addReceiver(script)
            except:
                PtDebugPrint("ERROR: IQuitTelescope():\tCould not send quit message to the scope named: %s" % name)

        notify.netPropagate(0)
        notify.setActivate(1.0)
        notify.send()



    def IRefreshGamesDisplay(self):
        "refresh the number and best time fields for the games"
        if self.debug:
            print "---->FULL UPDATE!"
        gameSelector = ptGUIControlRadioGroup(MarkerGameDlg.dialog.getControlFromTag(kRGMarkerGameSelect))
        curGame = gameSelector.getValue()

        curGameName = grtzMarkerGames.GetCGZGameName(curGame)
        for fieldID in range(kMarkerGameFieldStart,kMarkerGameFieldEnd,10):
            numTB = ptGUIControlTextBox(MarkerGameDlg.dialog.getControlFromTag(fieldID+kMarkerGameNumFieldOffset))
            timeTB = ptGUIControlTextBox(MarkerGameDlg.dialog.getControlFromTag(fieldID+kMarkerGameNameFieldOffset))
            gameidx = (fieldID-kMarkerGameFieldStart)/10
            numTB.setString("%d" % (grtzMarkerGames.GetNumMarkers(gameidx)))
            gameName = grtzMarkerGames.GetCGZGameName(gameidx)

            startTime,bestTime = grtzMarkerGames.GetGameTime(gameName)
            showZeros = 0
            if curGameName == gameName:
                # use the currentTime
                bestTime = PtGetDniTime() - startTime
                showZeros = 1

            if showZeros == 1 and bestTime == 0:
                timeTB.setString("00:00:00")
            elif bestTime >= 0.1 :
                tuptime = time.gmtime(bestTime)
                preText = ""
                postText = ""
                
                #Check for best game....
                if self.lastGame == gameidx:
                    if self.lastGameScore < bestTime or (self.lastGameScore > 0 and bestTime <= 0):
                        preText = "*"
                        postText = "*"
                if tuptime[2] == 1:
                    ostring = preText+"%02d:%02d:%02d"+postText
                    timeTB.setString(ostring % (tuptime[3],tuptime[4],tuptime[5]))
                else:
                    ostring = preText+"%02d:%02d:%02d:%02d"+postText
                    timeTB.setString(ostring % (tuptime[2]-1,tuptime[3],tuptime[4],tuptime[5]))
            else:
                timeTB.setString("--:--:--")  



    def IPartialRefreshDisplay(self):
        "Refreshes only the volitile time entries (i.e. a game being played or a completed game)"
        
        gameSelector = ptGUIControlRadioGroup(MarkerGameDlg.dialog.getControlFromTag(kRGMarkerGameSelect))
        curGame = gameSelector.getValue()


        #check a completed game
        if self.lastGame > -1:
            if self.debug:
                print "PARTIAL--LASTGAME!"
            fieldID = self.lastGame * 10 + kMarkerGameFieldStart
            timeTB = ptGUIControlTextBox(MarkerGameDlg.dialog.getControlFromTag(fieldID+kMarkerGameNameFieldOffset))
            gameName = grtzMarkerGames.GetCGZGameName(self.lastGame)

            startTime,bestTime = grtzMarkerGames.GetGameTime(gameName)

            if bestTime >= 0.1:
                tuptime = time.gmtime(bestTime)
                modText = ""

                if self.lastGameScore < bestTime or (self.lastGameScore > 0 and bestTime <= 0):
                    modText = "*"
                    #Kill the timer if we were only awaiting a best game update
                    if curGame == -1:
                        self.updateLevel = kNoRefresh
                if tuptime[2] == 1:
                    ostring = modText+"%02d:%02d:%02d"+modText
                    timeTB.setString(ostring % (tuptime[3],tuptime[4],tuptime[5]))
                else:
                    ostring = modText+"%02d:%02d:%02d:%02d"+modText
                    timeTB.setString(ostring % (tuptime[2]-1,tuptime[3],tuptime[4],tuptime[5]))
            else:
                timeTB.setString("--:--:--")

        #Check the current game
        if curGame > -1:
            if self.debug:
                print "PARTIAL--CURRENT!"
            fieldID = curGame * 10 + kMarkerGameFieldStart
            timeTB = ptGUIControlTextBox(MarkerGameDlg.dialog.getControlFromTag(fieldID+kMarkerGameNameFieldOffset))
            gameName = grtzMarkerGames.GetCGZGameName(curGame)

            startTime,bestTime = grtzMarkerGames.GetGameTime(gameName)
            bestTime = PtGetDniTime() - startTime

            if bestTime == 0:
                timeTB.setString("00:00:00")
            elif bestTime >= 0.1:
                tuptime = time.gmtime(bestTime)
                if tuptime[2] == 1:
                    ostring = "%02d:%02d:%02d"
                    timeTB.setString(ostring % (tuptime[3],tuptime[4],tuptime[5]))
                else:
                    ostring = "%02d:%02d:%02d:%02d"
                    timeTB.setString(ostring % (tuptime[2]-1,tuptime[3],tuptime[4],tuptime[5]))
            else:
                timeTB.setString("--:--:--")


    def OnBackdoorMsg(self, target, param):
        if target.lower() == "updatelevel":
            #Setup the levels for nice and easy string output
            levels = ("kNoRefresh","kFullRefresh","kPartialRefresh","kOneTimeFullRefresh")
            
            if param.lower() == "print":                
                print "***Current Update Level = %s" %levels[self.updateLevel]
            else:
                reqLevel = -1
                try:
                    reqLevel = int(param)
                    if reqLevel < 0 or reqLevel > len(levels):
                        raise
                except:
                    print "@@@ ERROR: invalid argument @@@"
                    return

                print "***Setting Update Level = %s" % reqLevel
                oldLevel = self.updateLevel
                self.updateLevel = reqLevel
                if reqLevel > kNoRefresh and oldLevel > kNoRefresh:
                    PtAtTimeCallback(self.key,1,kDisplayTimer)

        elif target.lower() == "lastgame":
            if param.lower() == "print":
                print "***Last Game = %s, with last score = %s" % (self.lastGame, self.lastGameScore)
            else:
                try:
                    reqGame = int(param)
                    if reqGame < 0 or reqGame > 13:
                        raise
                except:
                    print "@@@ ERROR: invalid argument @@@"
                    return

                print "***Setting Last Game = %s" %reqGame
                self.lastGame = reqGame

        elif target.lower() == "lastgamescore":
            if param.lower() == "print":
                print "***Last Game = %s, with last score = %s" % (self.lastGame, self.lastGameScore)
            else:
                try:
                    reqScore = int(param)
                    if reqScore < 0:
                        raise
                except:
                    print "@@@ ERROR: invalid argument @@@"
                    return
                print "***Setting Last Game Score = %s" %reqScore
                self.lastGameScore = reqScore

        elif target.lower() == "debug":            
            self.debug = abs(self.debug - 1)
            mode = "OFF"
            if self.debug:
                mode = "ON"
                
            print "***Turning on CGZ Marker Scope GUI Debug Mode: %s" % mode


            

               



