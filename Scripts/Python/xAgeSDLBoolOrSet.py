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

"""
Age SDL Boolean OR Set Module

This module accepts a comma separated list of input SDL variables. When any of them change,
a logical OR is performed on the values, and the result is stored in the output SDL variable.
"""

from __future__ import annotations
from typing import *

from Plasma import *
from PlasmaTypes import *

stringSDLVariableInput = ptAttribString(1, "AgeSDL Input Variables (comma separated)")
stringSDLVariableOutput = ptAttribString(2, "AgeSDL Output Variable")

class xAgeSDLBoolOrSet(ptResponder):
    if TYPE_CHECKING:
        key: ptKey = ...
        sceneobject: ptSceneobject = ...

    def __init__(self):
        super().__init__()
        self.id = 1827796758
        self.version = 1

    def OnServerInitComplete(self):
        inputVariables, outputVariable = self.inputVariables, self.outputVariable
        if not inputVariables:
            PtDebugPrint(f"xAgeSDLBoolOrSet.OnServerInitComplete(): [{self.sceneobject.getName()}] Input variable list empty, bailing!")
            return
        if not all(inputVariables):
            PtDebugPrint(f"xAgeSDLBoolOrSet.OnServerInitComplete(): [{self.sceneobject.getName()}] One of the input variables is empty, bailing!")
            return
        if len(inputVariables) == 1:
            PtDebugPrint(f"xAgeSDLBoolOrSet.OnServerInitComplete(): [{self.sceneobject.getName()}] Hmm... Only one input variable?", level=kWarningLevel)
        if not outputVariable:
            PtDebugPrint(f"xAgeSDLBoolOrSet.OnServerInitComplete(): [{self.sceneobject.getName()}] Output variable empty, bailing!")
            return

        ageSDL = PtGetAgeSDL()
        ageSDL.setFlags(outputVariable, True, True)
        ageSDL.sendToClients(outputVariable)
        for i in inputVariables:
            ageSDL.setNotify(self.key, i, 0.0)

        self.updateSDL(ageSDL)

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        if not VARname in self.inputVariables:
            return

        ageSDL = PtGetAgeSDL()
        PtDebugPrint(f"xAgeSDLBoolOrSet.OnSDLNotify(): {VARname} = {ageSDL[VARname][0]}", level=kDebugDumpLevel)
        self.updateSDL(ageSDL)

    def updateSDL(self, ageSDL: ptSDL) -> None:
        result = any((ageSDL[i][0] for i in self.inputVariables))
        PtDebugPrint(f"xAgeSDLBoolOrSet.updateSDL(): {self.outputVariable} = {result}", level=kWarningLevel)
        ageSDL[self.outputVariable] = (result,)

    @property
    def inputVariables(self):
        return [i.strip() for i in stringSDLVariableInput.value.split(",")]

    @property
    def outputVariable(self):
        return stringSDLVariableOutput.value
