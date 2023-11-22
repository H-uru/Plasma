# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/

#==============================================================================#
#                                                                              #
#    Offline KI                                                                #
#                                                                              #
#    Copyright (C) 2004-2011  The Offline KI contributors                      #
#    See the file AUTHORS for more info about the contributors (including      #
#    contact information)                                                      #
#                                                                              #
#    This program is free software: you can redistribute it and/or modify      #
#    it under the terms of the GNU General Public License as published by      #
#    the Free Software Foundation, either version 3 of the License, or         #
#    (at your option) any later version, with or (at your option) without      #
#    the Uru exception (see below).                                            #
#                                                                              #
#    This program is distributed in the hope that it will be useful,           #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of            #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
#    GNU General Public License for more details.                              #
#                                                                              #
#    Please see the file COPYING for the full GPLv3 license, or see            #
#    <http://www.gnu.org/licenses/>                                            #
#                                                                              #
#    Uru exception: In addition, this file may be used in combination with     #
#    (non-GPL) code within the context of Uru.                                 #
#                                                                              #
#==============================================================================#

from Plasma import *
from PlasmaTypes import *
import string
actButton1 = ptAttribActivator(1, 'Act: Button 1')
actButton2 = ptAttribActivator(2, 'Act: Button 2')
actButton3 = ptAttribActivator(3, 'Act: Button 3')
actButton4 = ptAttribActivator(4, 'Act: Button 4')
actButton5 = ptAttribActivator(5, 'Act: Button 5')
actButton6 = ptAttribActivator(6, 'Act: Button 6')
actButton7 = ptAttribActivator(7, 'Act: Button 7')
actButton8 = ptAttribActivator(8, 'Act: Button 8')
actButton9 = ptAttribActivator(9, 'Act: Button 9')
respButton1 = ptAttribResponder(10, 'Resp: Button 1 Down')
respButton2 = ptAttribResponder(11, 'Resp: Button 2 Down')
respButton3 = ptAttribResponder(12, 'Resp: Button 3 Down')
respButton4 = ptAttribResponder(13, 'Resp: Button 4 Down')
respButton5 = ptAttribResponder(14, 'Resp: Button 5 Down')
respButton6 = ptAttribResponder(15, 'Resp: Button 6 Down')
respButton7 = ptAttribResponder(16, 'Resp: Button 7 Down')
respButton8 = ptAttribResponder(17, 'Resp: Button 8 Down')
respButton9 = ptAttribResponder(18, 'Resp: Button 9 Down')
respButtonsUp = ptAttribResponder(19, 'Resp: All Buttons Up')
Solution = ptAttribString(20, 'Solution sequence')
ButtonSDL = ptAttribString(21, 'Button SDL name')
SolvedSDL = ptAttribString(22, 'Solved SDL name')
boolReplay = ptAttribBoolean(23, 'Replay if solved?', 1)
floatTimeOut = ptAttribFloat(24, 'Click time-out', 1.2)
boolFirstUpdate = ptAttribBoolean(25, 'Init SDL on first update?', 0)
ButtonDict = {1: (actButton1, respButton1),
 2: (actButton2, respButton2),
 3: (actButton3, respButton3),
 4: (actButton4, respButton4),
 5: (actButton5, respButton5),
 6: (actButton6, respButton6),
 7: (actButton7, respButton7),
 8: (actButton8, respButton8),
 9: (actButton9, respButton9)}
class xSimplePanel(ptResponder,):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 1049699
        self.version = 1
        minor = 0
        self.me = self.__class__.__name__
        print ('__init__%s v. %d.%d' % (self.me,
         self.version,
         minor))



    def OnFirstUpdate(self):
        EmptyButtons = []
        for i in ButtonDict:
            (act, resp,) = ButtonDict[i]
            if ((not len(act.value)) or (not len(resp.value))):
                EmptyButtons.append(i)

        for i in EmptyButtons:
            del ButtonDict[i]

        print ('%s.OnFirstUpdate: Valid buttons: %d' % (self.me,
         len(ButtonDict)))
        if (len(ButtonDict) != len(Solution.value)):
            print ('%s.OnFirstUpdate: WARNING! Solution length mismatch: %d' % (self.me,
             len(Solution.value)))
        if boolFirstUpdate.value:
            self.OnServerInitComplete()



    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.sendToClients(ButtonSDL.value)
        ageSDL.sendToClients(SolvedSDL.value)
        ageSDL.setFlags(ButtonSDL.value, 1, 1)
        ageSDL.setFlags(SolvedSDL.value, 1, 1)
        ageSDL.setNotify(self.key, ButtonSDL.value, 0.0)
        ageSDL.setNotify(self.key, SolvedSDL.value, 0.0)
        ButtonsPushed = ageSDL[ButtonSDL.value][0]
        ButtonsPushed = str(ButtonsPushed)
        print ('%s.OnServerInitComplete: When I got here, ButtonsPushed = %s' % (self.me,
         ButtonsPushed))
        if (len(ButtonsPushed) >= len(Solution.value)):
            PuzzleSolved = ageSDL[SolvedSDL.value][0]
            if ((not PuzzleSolved) or boolReplay.value):
                respButtonsUp.run(self.key, fastforward=1)
                ageSDL[ButtonSDL.value] = (0,)
                return 
        for i in ButtonDict:
            if (str(i) in ButtonsPushed):
                (act, resp,) = ButtonDict[i]
                resp.run(self.key, fastforward=1)
                act.disable()

        if ('0' in ButtonsPushed):
            ageSDL[ButtonSDL.value] = (0,)



    def OnNotify(self, state, id, events):
        if ((id in ButtonDict) and (state and PtWasLocallyNotified(self.key))):
            ageSDL = PtGetAgeSDL()
            PtSetGlobalClickability(0)
            PtAtTimeCallback(self.key, floatTimeOut.value, 1)
            print ('%s.OnNotify: Button #%d pushed' % (self.me,
             id))
            PuzzleSolved = ageSDL[SolvedSDL.value][0]
            if (PuzzleSolved and boolReplay.value):
                print ('%s.OnNotify: Oops... you have reset the puzzle!' % self.me)
                ageSDL[SolvedSDL.value] = (0,)
            ButtonsPushed = ageSDL[ButtonSDL.value][0]
            ButtonsPushed = str(ButtonsPushed)
            print ('%s.OnNotify: Before, ButtonsPushed was %s' % (self.me,
             ButtonsPushed))
            ButtonsPushed = int((ButtonsPushed + str(id)))
            print ('%s.OnNotify: Now, ButtonsPushed is %s' % (self.me,
             ButtonsPushed))
            ageSDL[ButtonSDL.value] = (ButtonsPushed,)
            if (len(str(ButtonsPushed)) >= len(Solution.value)):
                PtAtTimeCallback(self.key, 1, 2)



    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        if (VARname == ButtonSDL.value):
            ageSDL = PtGetAgeSDL()
            ButtonsPushed = ageSDL[ButtonSDL.value][0]
            print ('%s.OnSDLNotify: New ButtonsPushed = %s' % (self.me,
             ButtonsPushed))
            if (ButtonsPushed == 0):
                for i in ButtonDict:
                    (act, resp,) = ButtonDict[i]
                    act.enable()

                return 
            ButtonsPushed = str(ButtonsPushed)
            LastButtonPushed = int(ButtonsPushed[-1:])
            (act, resp,) = ButtonDict[LastButtonPushed]
            resp.run(self.key, netPropagate=0)
            act.disable()



    def OnTimer(self, id):
        if (id == 1):
            PtSetGlobalClickability(1)
        elif (id == 2):
            ageSDL = PtGetAgeSDL()
            ButtonsPushed = ageSDL[ButtonSDL.value][0]
            print ('%s.OnTimer: Check solution. ButtonsPushed = %s' % (self.me,
             ButtonsPushed))
            if (ButtonsPushed == int(Solution.value)):
                print ('%s.OnTimer: Puzzle solved. Unlocking...' % self.me)
                ageSDL[SolvedSDL.value] = (1,)
                if (not boolReplay.value):
                    return 
            respButtonsUp.run(self.key)
            ageSDL[ButtonSDL.value] = (0,)
