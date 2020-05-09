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
        
        if not state:
            return


        #PtDebugPrint("clftIntroMusic: We've got notification from ID #:%s" %id)

        #-----Activators-----
        startMusicActIDs = (actStartMusic01.id, actStartMusic02.id, actStartMusic03.id)
        if id in startMusicActIDs:
            if musicState == kOff:
                PtDebugPrint("clftIntroMusic: ---Starting Music---")
                musicState = kInitialPlay
                respStartMusic.run(self.key)
            return
        
        elif id == actStopMusic.id:
            if musicState == kInitialPlay:
                PtDebugPrint("clftIntroMusic: ###Stopping Music###")
                respStopInitialMusic.run(self.key)
            elif musicState == kRandomPlay:
                PtDebugPrint("clftIntroMusic: ###Stopping Music###")
                respStopRandomMusic.run(self.key)
            musicState = kOff
            return

        #-----Responders-----
        elif id == respStartMusic.id:
            if musicState == kInitialPlay:
                PtDebugPrint("clftIntroMusic: ___Randomly Starting Music___")
                musicState = kRandomPlay
                respStartRandomMusic.run(self.key)
            return


