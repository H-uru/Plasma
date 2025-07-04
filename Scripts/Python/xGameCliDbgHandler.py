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
# JPEG Library README, Windows Mefdia SDK EULA, or QuickTime SDK EULA, the
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

from __future__ import annotations

from Plasma import *
from PlasmaConstants import *
from PlasmaGameConstants import *
from PlasmaTypes import *

from typing import *

level = kWarningLevel

def _EnumGetNameFromValue(enum, value) -> str:
    if reverseLookup := getattr(enum, "reverseLookup", None):
        return reverseLookup.get(value, f"<UNKNOWN: {value}>")
    return str(value)

# Shortcut so you don't have to type so much in the console
def new() -> xGameCliDbgHandler:
    return xGameCliDbgHandler()

class xGameCliDbgHandler:
    def OnGameCliInstance(self, error):
        error = _EnumGetNameFromValue(PtGameJoinError, error)
        PtDebugPrint(f"xGameCliDbgHandler.OnGameCliInstance(): {error=}", level=level)

    def OnGameCliDelete(self):
        PtDebugPrint("xGameCliDbgHandler.OnGameCliDelete()", level=level)

    def OnPlayerJoined(self, playerID):
        PtDebugPrint(f"xGameCliDbgHandler.OnPlayerJoined(): {playerID=}", level=level)

    def OnPlayerLeft(self, playerID):
        PtDebugPrint(f"xGameCliDbgHandler.OnPlayerLeft(): {playerID=}", level=level)

    def OnOwnerChanged(self, newOwnerID):
        PtDebugPrint(f"xGameCliDbgHandler.OnOwnerChanged(): {newOwnerID=}", level=level)

    # #####################################
    #               VarSync               #
    #######################################

    def OnVarChanged(self, varID: int, varValue: Union[float, str]):
        PtDebugPrint(f"xGameCliDbgHandler.OnVarChanged(): {varID=} {varValue=}", level=level)

    def OnAllVarsSent(self):
        PtDebugPrint("xGameCliDbgHandler.OnAllVarsSent()", level=level)

    def OnVarCreated(self, varName: str, varID: int, varValue: Union[float, str]):
        PtDebugPrint(f"xGameCliDbgHandler.OnVarCreated(): {varName=} {varID=} {varValue=}", level=level)
