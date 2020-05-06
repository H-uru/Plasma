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
AlreadyClosed = False

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
        PtDebugPrint("__init__ clftEndingCredits v. ",version,".1")
        

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        SDLVarSceneBahro = "clftSceneBahroUnseen"
        ageSDL.setNotify(self.key,SDLVarSceneBahro,0.0)


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global gJournalBook

        ageSDL = PtGetAgeSDL()
        
        SDLVarSceneBahro = "clftSceneBahroUnseen"
        if VARname == SDLVarSceneBahro:
            PtDebugPrint("OnSDL launched the credits.")
            boolSceneBahro = ageSDL[SDLVarSceneBahro][0]
            if boolSceneBahro == 0:
                PtDebugPrint("clftEndingCredits.OnSDLNotify(): we're no longer showing the credits here")
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                PtEnableMovementKeys()
                PtSendKIMessage(kEnableKIandBB,0)
#                PtDebugPrint("\tclftEndingCredits.OnSDLNotify: loading journal")
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
                PtDebugPrint("No credits.")
        
        pass


    def OnNotify(self,state,id,events):
        global AlreadyClosed        

        if (id == RgnSnsrSndLogTracks.id and state):
            import xSndLogTracks
            if xSndLogTracks.IsLogMode():
                PtDebugPrint("Start of Egg!!! Enable Riven scope clickable")
                ClkSndLogTracks.enable()
            else:
                PtDebugPrint("not this time")
                ClkSndLogTracks.disable()
                
        if (id == ClkSndLogTracks.id and state):
            #PtPageInNode("clftSndLogTracks")
            RespSndLogTracks.run(self.key,avatar=PtGetLocalAvatar())


        if AlreadyClosed:
            PtDebugPrint("failed AlreadyClosed check")
            return
    
        for event in events:
            if event[0] == PtEventType.kBook:
                if event[1] == PtBookEventTypes.kNotifyHide:
                    AlreadyClosed = True
                    PtFadeOut(kFadeOutToGameSeconds,1)
                    PtDebugPrint("clftEndingCredits.OnNotify(): Book hidden. FadeOut over", kFadeOutToGameSeconds," seconds")  
                    
                    #The book is already hidden, so just go ahead and fade back in on the game
                    PtAtTimeCallback(self.key,kFadeOutToGameSeconds,kFadeOutToGameID)

                elif event[1] == PtBookEventTypes.kNotifyClose:
                    AlreadyClosed = True
                    PtFadeOut(kFadeOutToGameSeconds,1)
                    PtDebugPrint("clftEndingCredits.OnNotify(): Book closed. FadeOut over", kFadeOutToGameSeconds," seconds")                    
                    
                    #We have to hide the book first before we fade back in on the game
                    PtAtTimeCallback(self.key,kFadeOutToGameSeconds+1,kHideBookID)


    def OnTimer(self,id):
        
        global gJournalBook        
        PtDebugPrint("clftEndingCredits.OnTimer(): Callback from id:",id)
        if id == kFadeOutToCreditsID: # 1
            self.IFadeOutToCredits()
        elif id == kShowCreditsID: # 2 
            self.IShowCredits()            
        elif id == kFadeInToCreditsID: # 3
            self.IFadeInToCredits()
        elif id == kFadeOutToGameID: # 4
            self.IFadeOutToGame()
        elif id == kHideBookID: # 5
            PtDebugPrint("clftEndingCredits.OnTimer(): The credits book is now hidden")
            gJournalBook.hide()            
            PtAtTimeCallback(self.key,kFadeOutToGameSeconds,kFadeOutToGameID)

        elif id == kFadeInToGameID: # 6
            PtFadeIn(kFadeInToGameSeconds,1)
            PtDebugPrint("clftEndingCredits.OnTimer(): FadeIn over", kFadeInToGameSeconds," seconds")
            cam = ptCamera()
            cam.enableFirstPersonOverride()
            PtEnableMovementKeys()
            PtSendKIMessage(kEnableKIandBB,0)


    def IFadeOutToCredits(self):
        PtFadeOut(kFadeOutToCreditsSeconds,1)
        PtDebugPrint("clftEndingCredits.IFadeOutToCredits(): FadeOut over", kFadeOutToCreditsSeconds," seconds.")
        PtAtTimeCallback(self.key,4,kShowCreditsID)


    def IShowCredits(self):
        global gJournalBook
        global AlreadyClosed
        
        PtDebugPrint("clftEndingCredits.IShowCredits(): Showing Journal now.")
        gJournalBook.show(0)
        AlreadyClosed = False
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
        PtDebugPrint("clftEndingCredits.IFadeInToCredits(): FadeIn over", kFadeInToCreditsSeconds," seconds")


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
