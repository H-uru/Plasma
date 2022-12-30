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
import PlasmaControlKeys
import string
Activate = ptAttribActivator(1, ' clickable ', netForce=1)
Vignettes = ptAttribString(4, 'Toggle multiple dialogs - by Name')
SingleUser = ptAttribBoolean(5, 'One user only?', 0)
KeyMap = {}
KeyMap[PlasmaControlKeys.kKeyMoveForward] = PlasmaControlKeys.kKeyCamPanUp
KeyMap[PlasmaControlKeys.kKeyMoveBackward] = PlasmaControlKeys.kKeyCamPanDown
KeyMap[PlasmaControlKeys.kKeyRotateLeft] = PlasmaControlKeys.kKeyCamPanLeft
KeyMap[PlasmaControlKeys.kKeyRotateRight] = PlasmaControlKeys.kKeyCamPanRight
kExit = 99
Vignette = None
class xMultiDialogToggle(ptModifier,):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5104999
        self.version = 1
        minor = 0
        self.me = self.__class__.__name__
        self.VignetteList = []
        print ('__init__%s v. %d.%d' % (self.me, self.version, minor))



    def IGetAgeFilename(self):
        ageInfo = PtGetAgeInfo()
        if (type(ageInfo) != type(None)):
            return ageInfo.getAgeFilename()
        else:
            return 'GUI'



    def OnFirstUpdate(self):
        try:
            self.VignetteList += Vignettes.value.split(',')
            for i in range(len(self.VignetteList)):
                self.VignetteList[i] = self.VignetteList[i].strip()

            PtDebugPrint(('%s: Dialog list = %s' % (self.me,
             self.VignetteList)))
        except:
            PtDebugPrint(("ERROR: %s.OnFirstUpdate(): Couldn't process dialog list" % self.me))
            return 
        for i in range(len(self.VignetteList)):
            PtLoadDialog(self.VignetteList[i], self.key, self.IGetAgeFilename())




    def __del__(self):
        for i in range(len(self.VignetteList)):
            PtUnloadDialog(self.VignetteList[i])




    def OnNotify(self, state, id, events):
        global Vignette
        if (state and ((id == Activate.id) and PtWasLocallyNotified(self.key))):
            Vignette = self.VignetteList[0]
            self.IStartDialog(1)



    def OnGUINotify(self, id, control, event):
        global Vignette
        if (event == kAction):
            if (control.getTagID() == kExit):
                for i in range(len(self.VignetteList)):
                    if (self.VignetteList[i] == Vignette):
                        try:
                            tmpVignette = self.VignetteList[(i + 1)]
                        except IndexError:
                            self.IQuitDialog(1)
                            return 
                        self.IQuitDialog(0)
                        Vignette = tmpVignette
                        self.IStartDialog(0)
                        break

        elif (event == kExitMode):
            self.IQuitDialog(1)



    def OnControlKeyEvent(self, controlKey, activeFlag):
        PtDebugPrint(('Got controlKey event %d and its activeFlag is %d' % (controlKey,
         activeFlag)), level=kDebugDumpLevel)
        if (controlKey == PlasmaControlKeys.kKeyExitMode):
            self.IQuitDialog(1)



    def IStartDialog(self, init = 1):
        PtLoadDialog(Vignette, self.key, self.IGetAgeFilename())
        if PtIsDialogLoaded(Vignette):
            PtShowDialog(Vignette)
            PtDebugPrint(('%s: Dialog %s goes up' % (self.me,
             Vignette)))
        if init:
            PtGetControlEvents(true, self.key)
            if SingleUser.value:
                Activate.disable()



    def IQuitDialog(self, exit = 1):
        if ((type(Vignette) != type(None)) and (Vignette != '')):
            PtHideDialog(Vignette)
            PtDebugPrint(('%s: Dialog %s goes down' % (self.me,
             Vignette)))
        if exit:
            PtGetControlEvents(false, self.key)
            Activate.enable()
