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
"""
Module: clftEndingCredits.py
Age: Cleft(Tomahna)
Date: September 2003
Do credits at endgame
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *
import time
import copy
import PlasmaControlKeys
import xJournalBookDefs


respStartCreditsMusic = ptAttribResponder(1,"Resp: Start Credits Music")
respStopCreditsMusic = ptAttribResponder(2,"Resp: Stop Credits Music")
respLeaveCredits = ptAttribResponder(3,"Resp: Exit Credits Cam")
RgnSnsrSndLogTracks = ptAttribActivator(4, "Rgn snsr: SndLogTracks scope")
ClkSndLogTracks = ptAttribActivator(5, "Clickable: SndLogTracks scope")
RespSndLogTracks = ptAttribResponder(6,"Resp: SndLogTracks scope")


#globals
gJournalBook = None
gOriginalAmbientVolume = 1.0
gOriginalSFXVolume = 1.0
AlreadyClosed = false

#timer IDs
kFadeOutToCreditsID = 1
kShowCreditsID = 2
kFadeInToCreditsID = 3
kFadeOutToGameID = 4
kHideBookID = 5
kFadeInToGameID = 6

#Timer delay values
kDelayFadeSeconds = 1.5
kFadeOutToCreditsSeconds = 2.0
kFadeInToCreditsSeconds = 1.0
kFadeOutToGameSeconds = 1.0
kFadeInToGameSeconds = 2.0

class clftEndingCredits(ptResponder):

    
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 8802
        version = 5
        self.version = version
        print "__init__ clftEndingCredits v. ",version,".1"
        

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        SDLVarSceneBahro = "clftSceneBahroUnseen"
        ageSDL.setNotify(self.key,SDLVarSceneBahro,0.0)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global gJournalBook

        ageSDL = PtGetAgeSDL()
        
        SDLVarSceneBahro = "clftSceneBahroUnseen"
        if VARname == SDLVarSceneBahro:
            print "OnSDL launched the credits."
            boolSceneBahro = ageSDL[SDLVarSceneBahro][0]
            if boolSceneBahro == 0:
                print "clftEndingCredits.OnSDLNotify(): we're no longer showing the credits here"
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                PtEnableMovementKeys()
                PtSendKIMessage(kEnableKIandBB,0)
#                print "\tclftEndingCredits.OnSDLNotify: loading journal"
#                params = xJournalBookDefs.xJournalBooks["UruCredits"]
#                if len(params) == 4:
#                    width,height,locPath,gui = params
#                else:
#                    width,height,locPath = params
#                    gui = "BkBook"
#                gJournalBook = ptBook(PtGetLocalizedString(locPath),self.key)
#                gJournalBook.setSize(width,height)
#                gJournalBook.setGUI(gui)
#                
#                #PtFadeOut(kDelayFadeSeconds,1)
#                PtDisableMovementKeys()
#                PtAtTimeCallback(self.key,kDelayFadeSeconds,kFadeOutToCreditsID)
            else:
                print "No credits."
        
        pass


    def OnNotify(self,state,id,events):
        global AlreadyClosed        

        if (id == RgnSnsrSndLogTracks.id and state):
            import xSndLogTracks
            if xSndLogTracks.IsLogMode():
                print "Start of Egg!!! Enable Riven scope clickable"
                ClkSndLogTracks.enable()
            else:
                print "not this time"
                ClkSndLogTracks.disable()
                
        if (id == ClkSndLogTracks.id and state):
            #PtPageInNode("clftSndLogTracks")
            RespSndLogTracks.run(self.key,avatar=PtGetLocalAvatar())


        if AlreadyClosed:
            print "failed AlreadyClosed check"
            return
    
        for event in events:
            if event[0] == PtEventType.kBook:
                if event[1] == PtBookEventTypes.kNotifyHide:
                    AlreadyClosed = true
                    PtFadeOut(kFadeOutToGameSeconds,1)
                    print "clftEndingCredits.OnNotify(): Book hidden. FadeOut over", kFadeOutToGameSeconds," seconds"  
                    
                    #The book is already hidden, so just go ahead and fade back in on the game
                    PtAtTimeCallback(self.key,kFadeOutToGameSeconds,kFadeOutToGameID)

                elif event[1] == PtBookEventTypes.kNotifyClose:
                    AlreadyClosed = true
                    PtFadeOut(kFadeOutToGameSeconds,1)
                    print "clftEndingCredits.OnNotify(): Book closed. FadeOut over", kFadeOutToGameSeconds," seconds"                    
                    
                    #We have to hide the book first before we fade back in on the game
                    PtAtTimeCallback(self.key,kFadeOutToGameSeconds+1,kHideBookID)


    def OnTimer(self,id):
        
        global gJournalBook        
        print "clftEndingCredits.OnTimer(): Callback from id:",id
        if id == kFadeOutToCreditsID: # 1
            self.IFadeOutToCredits()
        elif id == kShowCreditsID: # 2 
            self.IShowCredits()            
        elif id == kFadeInToCreditsID: # 3
            self.IFadeInToCredits()
        elif id == kFadeOutToGameID: # 4
            self.IFadeOutToGame()
        elif id == kHideBookID: # 5
            print "clftEndingCredits.OnTimer(): The credits book is now hidden"
            gJournalBook.hide()            
            PtAtTimeCallback(self.key,kFadeOutToGameSeconds,kFadeOutToGameID)

        elif id == kFadeInToGameID: # 6
            PtFadeIn(kFadeInToGameSeconds,1)
            print "clftEndingCredits.OnTimer(): FadeIn over", kFadeInToGameSeconds," seconds"
            cam = ptCamera()
            cam.enableFirstPersonOverride()
            PtEnableMovementKeys()
            PtSendKIMessage(kEnableKIandBB,0)


    def IFadeOutToCredits(self):
        PtFadeOut(kFadeOutToCreditsSeconds,1)
        print "clftEndingCredits.IFadeOutToCredits(): FadeOut over", kFadeOutToCreditsSeconds," seconds."
        PtAtTimeCallback(self.key,4,kShowCreditsID)


    def IShowCredits(self):
        global gJournalBook
        global AlreadyClosed
        
        print "clftEndingCredits.IShowCredits(): Showing Journal now."
        gJournalBook.show(0)
        AlreadyClosed = false
        PtAtTimeCallback(self.key,1,kFadeInToCreditsID)        


    def IFadeInToCredits(self):
        global gJournalBook
        global gOriginalAmbientVolume
        global gOriginalSFXVolume        
        
        # turn down the ambient sound
        audio = ptAudioControl()
        gOriginalAmbientVolume = audio.getAmbienceVolume()
        audio.setAmbienceVolume(0.0)
        gOriginalSFXVolume = audio.getSoundFXVolume()
        audio.setSoundFXVolume(0.0)
        
        respStartCreditsMusic.run(self.key)
        
        PtFadeIn(kFadeInToCreditsSeconds,1)
        print "clftEndingCredits.IFadeInToCredits(): FadeIn over", kFadeInToCreditsSeconds," seconds"


    def IFadeOutToGame(self):
        global gOriginalAmbientVolume
        global gOriginalSFXVolume        

        # restore the ambient sounds
        audio = ptAudioControl()
        audio.setAmbienceVolume(gOriginalAmbientVolume)
        audio.setSoundFXVolume(gOriginalSFXVolume)
        
        respStopCreditsMusic.run(self.key)
        
        respLeaveCredits.run(self.key,avatar=PtGetLocalAvatar())
        PtAtTimeCallback(self.key,kFadeInToGameSeconds,kFadeInToGameID)
