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
"""Module: xMusicBoxAddSong
Age: Global
Author: Adam Van Ornum
Date: Feb 2007
Add songs to the music box
"""

from Plasma import *
from PlasmaTypes import *
from xPsnlVaultSDL import *
import PlasmaKITypes

#=============================================================
# define the attributes that will be entered in max
#=============================================================
actTrigger = ptAttribActivator(1, "Triggerer")
respOneShot = ptAttribResponder(2, "Oneshot resp")
strSoundFile = ptAttribString(3, "Sound file name")
strAgeToAddTo = ptAttribString(4, "Music player age name (not this player)")

#====================================
IClicked = 0

class xMusicBoxAddSong(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5346
        self.version = 1
        PtDebugPrint("xMusicBoxAddSong: init  version = %d" % self.version)

    def OnNotify(self,state,id,events):
        global IClicked
        
        if not state:
            return

        if id == actTrigger.id:
            if PtFindAvatar(events) == PtGetLocalAvatar():
                IClicked = 1
            else:
                IClicked = 0
            respOneShot.run(self.key, events = events)

        elif id == respOneShot.id:
            if IClicked and self.HasMusicBoxYeeshaPage():
                self.AddSong()

    def AddSong(self):
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename(strAgeToAddTo.value)

        vault = ptVault()
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            ageInfoChildren = ageInfoNode.getChildNodeRefList()
            for ageInfoChildRef in ageInfoChildren:
                ageInfoChild = ageInfoChildRef.getChild()
                folder = ageInfoChild.upcastToFolderNode()
                if folder and folder.getFolderName() == "AgeData":
                    ageDataChildren = folder.getChildNodeRefList()
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "MusicBoxSongs":
                            songs = chron.getValue()
                            if songs.find(strSoundFile.value) == -1:
                                PtDebugPrint("xMusicBoxAddSong.AddSong: Adding file %s" % strSoundFile.value)
                                chron.setValue(strSoundFile.value + ";" + songs)
                                PtSendKIMessageInt(PlasmaKITypes.kStartBookAlert,0)
                                return #break
                    break
        PtDebugPrint("ERROR: xMusicBoxAddSong.AddSong():\tCould not add song: %s" % strSoundFile.value)

    def HasMusicBoxYeeshaPage(self):
        sdl = xPsnlVaultSDL()
        curVal = sdl["YeeshaPage9"][0]
        return (curVal > 0)
