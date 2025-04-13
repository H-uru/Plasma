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
This module does the sound log tracks stuff
"""
from Plasma import *
from PlasmaTypes import *

kLogTrackVarname = "SndLogTrack"
kLogModeVarname = "VaporTrail"

def LogTrack(currentState,nextState):
    updated = 0
    vault = ptVault()
    entry = vault.findChronicleEntry(kLogTrackVarname)
    if entry is not None:
        state = entry.getValue()
        if state == currentState:
            avatar = PtGetLocalAvatar()
            worn = avatar.avatar.getAvatarClothingList()
            currentgender = avatar.avatar.getAvatarClothingGroup()
            if currentgender == kFemaleClothingGroup:
                clothingName = "02_FTorso11_01"
            else:
                clothingName = "02_MTorso09_01"
            itsThere = 0
            for item in worn:
                if item[0] == clothingName:
                    itsThere = 1
            if itsThere:
                entry.setValue(nextState)
                entry.save()
                updated = 1
                PtDebugPrint("updated to %s" % (nextState))
            else:
                PtDebugPrint("no track")
        if not updated:
            # ha, ha ...start over
            PtDebugPrint("not updated")
#            entry.setValue("15")
#            entry.save()
    return updated

def InitLogTrack(nextState):
    vault = ptVault()
    entry = vault.findChronicleEntry(kLogTrackVarname)
    if entry is not None:
        entry.setValue(nextState)
        entry.save()
    else:
        vault.addChronicleEntry(kLogTrackVarname,1,nextState)

def SetLogMode():
    vault = ptVault()
    entry = vault.findChronicleEntry(kLogModeVarname)
    if entry is not None:
        entry.setValue("white")
        entry.save()
    else:
        vault.addChronicleEntry(kLogModeVarname,1,"white")

def UnsetLogMode():
    vault = ptVault()
    entry = vault.findChronicleEntry(kLogModeVarname)
    if entry is not None:
        entry.setValue("blue")
        entry.save()

def IsLogMode():
    vault = ptVault()
    entry = vault.findChronicleEntry(kLogModeVarname)
    if entry is not None:
        if entry.getValue() == "white":
            return 1
    return 0

def WhatIsLog():
    vault = ptVault()
    entry = vault.findChronicleEntry(kLogTrackVarname)
    if entry is not None:
        PtDebugPrint(entry.getValue())
    else:
        PtDebugPrint("not initialized")

def GetTrack():
    avatar = PtGetLocalAvatar()
    currentgender = avatar.avatar.getAvatarClothingGroup()
    if currentgender == kFemaleClothingGroup:
        avatar.avatar.wearClothingItem("02_FTorso11_01")
    else:
        avatar.avatar.wearClothingItem("02_MTorso09_01")
    avatar.avatar.saveClothing() # save any clothing changes
