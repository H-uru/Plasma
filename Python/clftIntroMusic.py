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
Module: clftIntroMusic.py
Age: Cleft
Date: October 2006
Author: Tye Hooley
Controls the Intro Music
"""

from Plasma import *
from PlasmaTypes import *


actStartMusic01 = ptAttribActivator(1,"Start Music Activator 01")
actStartMusic02 = ptAttribActivator(2,"Start Music Activator 02")
actStartMusic03 = ptAttribActivator(3,"Start Music Activator 03")
respStartMusic = ptAttribResponder(4,"Start Music Repsonder")
respStartRandomMusic = ptAttribResponder(5,"Start Random Music Responder")

actStopMusic = ptAttribActivator(6,"Stop Music Activator")
respStopInitialMusic = ptAttribResponder(7,"Stop Initial Music Responder")
respStopRandomMusic = ptAttribResponder(8,"Stop Random Music Responder")



# globals

#Music States
kOff         = 0
kInitialPlay = 1
kRandomPlay  = 2

musicState = kOff



class clftIntroMusic(ptResponder):

    def __init__(self):
        global musicState
        ptResponder.__init__(self)
        self.id = 5249
        self.version = 1


    def OnNotify(self,state,id,events):
        global musicState
        
        if state == false:
            return


        #print "clftIntroMusic: We've got notification from ID #:%s" %id 

        #-----Activators-----
        startMusicActIDs = (actStartMusic01.id, actStartMusic02.id, actStartMusic03.id)
        if id in startMusicActIDs:
            if musicState == kOff:
                print "clftIntroMusic: ---Starting Music---"
                musicState = kInitialPlay
                respStartMusic.run(self.key)
            return
        
        elif id == actStopMusic.id:
            if musicState == kInitialPlay:
                print "clftIntroMusic: ###Stopping Music###"
                respStopInitialMusic.run(self.key)
            elif musicState == kRandomPlay:
                print "clftIntroMusic: ###Stopping Music###"
                respStopRandomMusic.run(self.key)
            musicState = kOff
            return

        #-----Responders-----
        elif id == respStartMusic.id:
            if musicState == kInitialPlay:
                print "clftIntroMusic: ___Randomly Starting Music___"
                musicState = kRandomPlay
                respStartRandomMusic.run(self.key)
            return


