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
"""Module: xMusicBox
Age: Global
Author: Adam Van Ornum
Date: January 2004
A music box type thing
"""

from Plasma import *
from PlasmaTypes import *

#=============================================================
# define the attributes that will be entered in max
#=============================================================
actTrigger = ptAttribActivator(1, "Triggerer")
respOneShot = ptAttribResponder(2, "Oneshot resp")
respStart = ptAttribResponder(3, "Start responder")
respStop = ptAttribResponder(4, "Stop responder")
soSoundObj = ptAttribSceneobject(5, "Sound sceneobject")
strPath = ptAttribString(6, "File path")
strSoundObj = ptAttribString(7, "Sound component name")
strInitialSong = ptAttribString(8, "Initial song name")
sdlCurrentSongVar = ptAttribString(9, "Current song sdl var")

# globals
CurrentFile = None
SoundObjIndex = 0
NumFiles = 0
IsPlaying = 0
TriggeringAvatar = None

#====================================

class xMusicBox(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5329
        self.version = 3
        PtDebugPrint("xMusicBox: init  version = %d" % self.version)

    def OnServerInitComplete(self):
        global SoundObjIndex
        global CurrentFile

        musicBoxChronFound = 0
        ageDataFolder = None

        ageVault = ptAgeVault()
        ageInfoNode = ageVault.getAgeInfo()

        ageInfoChildren = ageInfoNode.getChildNodeRefList()
        for ageInfoChildRef in ageInfoChildren:
            ageInfoChild = ageInfoChildRef.getChild()
            folder = ageInfoChild.upcastToFolderNode()
            if folder and folder.folderGetName() == "AgeData":
                ageDataFolder = folder
                ageDataChildren = folder.getChildNodeRefList()
                for ageDataChildRef in ageDataChildren:
                    ageDataChild = ageDataChildRef.getChild()
                    chron = ageDataChild.upcastToChronicleNode()
                    if chron and chron.getName() == "MusicBoxSongs":
                        musicBoxChronFound = 1

        if not ageDataFolder:
            newFolder = ptVaultFolderNode(0)
            newFolder.folderSetName("AgeData")
            ageInfoNode.addNode(newFolder)
            
            newNode = ptVaultChronicleNode(0)
            newNode.chronicleSetName("MusicBoxSongs")
            newNode.chronicleSetValue(strInitialSong.value)
            newFolder.addNode(newNode)

        elif not musicBoxChronFound:
            newNode = ptVaultChronicleNode(0)
            newNode.chronicleSetName("MusicBoxSongs")
            newNode.chronicleSetValue(strInitialSong.value)
            ageDataFolder.addNode(newNode)

        SoundObjIndex = soSoundObj.value.getSoundIndex(strSoundObj.value)
        PtDebugPrint("xMusicBox.OnServerInitComplete: using sound object index:" + str(SoundObjIndex))

        if type(sdlCurrentSongVar.value) == type("") and len(sdlCurrentSongVar.value) > 0:
            ageSDL = PtGetAgeSDL()
            filename = ageSDL[sdlCurrentSongVar.value][0]

            if len(filename) > 4:
                if filename[-4:].lower() == ".ogg":
                    iscompressed = 1
                else:
                    iscompressed = 0

                CurrentFile = (filename, iscompressed)

                soSoundObj.value.setSoundFilename(SoundObjIndex, CurrentFile[0], CurrentFile[1])
                respStart.run(self.key)

    def OnNotify(self,state,id,events):
        global IsPlaying
        global CurrentFile
        global SoundObjIndex
        global TriggeringAvatar
        
        if not state:
            return

        if id == actTrigger.id:
            TriggeringAvatar = PtFindAvatar(events)
            respOneShot.run(self.key, events = events)

        elif id == respOneShot.id:
            self.NextSong()

            respStop.run(self.key)
            if CurrentFile:
                soSoundObj.value.setSoundFilename(SoundObjIndex, CurrentFile[0], CurrentFile[1])
                respStart.run(self.key)

                currentSong = (CurrentFile[0],)
            else:
                currentSong = ("",)

            playerid = PtGetClientIDFromAvatarKey(TriggeringAvatar.getKey())
            localClient = PtGetLocalClientID()

            islocalavatar = (playerid == localClient)
            sdlvarisvalid = (type(sdlCurrentSongVar.value) == type("") and len(sdlCurrentSongVar.value) > 0)

            if islocalavatar and sdlvarisvalid:
                print "Setting cur song var to: ", currentSong
                ageSDL = PtGetAgeSDL()
                ageSDL[sdlCurrentSongVar.value] = currentSong
                
    def NextSong(self):
        global CurrentFile

        songList = self.GetMusicBoxSongList()

        if len(songList) == 0:
            index = -1
        else:
            index = 0
            if CurrentFile:
                index = songList.index(CurrentFile[0])
                if index != (len(songList) - 1):
                    index += 1
                else:
                    index = -1

        if index == -1:
            CurrentFile = None
        else:
            filename = songList[index]
            if filename[-4:].lower() == ".ogg":
                iscompressed = 1
            else:
                iscompressed = 0

            CurrentFile = (filename, iscompressed)

        PtDebugPrint("xMusicBox.NextSong: Going to try to play: %s" % str(CurrentFile), level=kDebugDumpLevel)

    def GetMusicBoxSongList(self):
        songList = []
        
        ageVault = ptAgeVault()
        ageInfoNode = ageVault.getAgeInfo()

        ageInfoChildren = ageInfoNode.getChildNodeRefList()
        for ageInfoChildRef in ageInfoChildren:
            ageInfoChild = ageInfoChildRef.getChild()
            folder = ageInfoChild.upcastToFolderNode()
            if folder and folder.folderGetName() == "AgeData":
                ageDataChildren = folder.getChildNodeRefList()
                for ageDataChildRef in ageDataChildren:
                    ageDataChild = ageDataChildRef.getChild()
                    chron = ageDataChild.upcastToChronicleNode()
                    if chron and chron.getName() == "MusicBoxSongs":
                        songList = chron.getValue().split(";")
        return songList
