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
This module contains the routines to read and write the Volume.ini file
"""

import string
import xIniHelper
from PlasmaConstants import *
from Plasma import *

gIniFile = None
gFilename = U"audio.ini"
gFilenameAndPath = U""

# the different volume commands
kVolCmd = "Audio.SetChannelVolume"
kMicLevel = "Audio.SetMicVolume"
kSoundPri = "Audio.SetPriorityCutoff"
kMute = "Audio.MuteAll"
kVoiceRec = "Audio.EnableVoiceChat"

# the diffent channel tags
kSoundFX = "SoundFX"
kMusic = "BgndMusic"
kVoice = "Voice"
kAmbience = "Ambience"
kGUI = "GUI"
kNPCVoice = "NPCVoice"

# value strings
kBeTrue = "true"
kBeFalse = "false"

def ConstructFilenameAndPath():
    global gFilenameAndPath
    if gFilenameAndPath == U"":
        if PtIsInternalRelease():
            # check for local file
            localNameAndPath = U"init/" + gFilename
            if PtFileExists(localNameAndPath):
                gFilenameAndPath = localNameAndPath
                print U"xIniAudio::ConstructFilenameAndPath(): Using internal \"" + gFilenameAndPath + U"\" file"
                return
        # otherwise, use the standard init path
        gFilenameAndPath = PtGetInitPath() + U"/" + gFilename
        print U"xIniAudio::ConstructFilenameAndPath(): Using user-level \"" + gFilenameAndPath + U"\" file"

def WriteIni():
    global gIniFile
    if gIniFile:
        ConstructFilenameAndPath()
        gIniFile.writeFile(gFilenameAndPath)

def ReadIni():
    global gIniFile
    ConstructFilenameAndPath()
    gIniFile = xIniHelper.iniFile(gFilenameAndPath)
    if gIniFile.isEmpty():
        # add defaults
        gIniFile.addEntry("# This is an auto-generated file.")
        gIniFile.addEntry("\n")

def SetSoundFXVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kSoundFX)
        volstr = "%g" % volume
        if entry:
            entry.setValue(1,volstr)
        else:
            gIniFile.addEntry("%s %s %g" % (kVolCmd,kSoundFX,volume))

def GetSoundFXVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kSoundFX)
        if entry:
            value = entry.getValue(1)
            if value:
                return string.atof(value)
        return 0.0

def SetMusicVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kMusic)
        volstr = "%g" % volume
        if entry:
            entry.setValue(1,volstr)
        else:
            gIniFile.addEntry("%s %s %g" % (kVolCmd,kMusic,volume))

def GetMusicVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kMusic)
        if entry:
            value = entry.getValue(1)
            if value:
                return string.atof(value)
        return 0.0

def SetVoiceVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kVoice)
        volstr = "%g" % volume
        if entry:
            entry.setValue(1,volstr)
        else:
            gIniFile.addEntry("%s %s %g" % (kVolCmd,kVoice,volume))

def GetVoiceVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kVoice)
        if entry:
            value = entry.getValue(1)
            if value:
                return string.atof(value)
        return 0.0

def SetAmbienceVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kAmbience)
        volstr = "%g" % volume
        if entry:
            entry.setValue(1,volstr)
        else:
            gIniFile.addEntry("%s %s %g" % (kVolCmd,kAmbience,volume))

def GetAmbienceVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kAmbience)
        if entry:
            value = entry.getValue(1)
            if value:
                return string.atof(value)
        return 0.0

def SetGUIVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kGUI)
        volstr = "%g" % volume
        if entry:
            entry.setValue(1,volstr)
        else:
            gIniFile.addEntry("%s %s %g" % (kVolCmd,kGUI,volume))

def GetGUIVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kGUI)
        if entry:
            value = entry.getValue(1)
            if value:
                return string.atof(value)
        return 0.0

def SetNPCVoiceVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kNPCVoice)
        volstr = "%g" % volume
        if entry:
            entry.setValue(1,volstr)
        else:
            gIniFile.addEntry("%s %s %g" % (kVolCmd,kNPCVoice,volume))

def GetNPCVoiceVolume(volume):
    if gIniFile:
        entry,idx = gIniFile.findByFirstValue(kNPCVoice)
        if entry:
            value = entry.getValue(1)
            if value:
                return string.atof(value)
        return 0.0

def SetMicLevel(level):
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kMicLevel)
        volstr = "%g" % level
        if entry:
            entry.setValue(0,volstr)
        else:
            gIniFile.addEntry("%s %g" % (kMicLevel,level))

def GetMicLevel(level):
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kMicLevel)
        if entry:
            value = entry.getValue(0)
            if value:
                return string.atof(value)
        return 0.0

def SetVoiceRecording(level):
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kVoiceRec)
        volstr = "%g" % level
        if entry:
            entry.setValue(0,volstr)
        else:
            gIniFile.addEntry("%s %g" % (kVoiceRec,level))

def GetVoiceRecording(level):
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kVoiceRec)
        if entry:
            value = entry.getValue(0)
            if value:
                return string.atof(value)
        return 0.0

def SetSoundPriority(priority):
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kSoundPri)
        pristr = "%d" % priority
        if entry:
            entry.setValue(0,pristr)
        else:
            gIniFile.addEntry("%s %d" % (kSoundPri,priority))

def GetSoundPriority():
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kSoundPri)
        if entry:
            value = entry.getValue(0)
            if value:
                return string.atoi(value)
        return 0

def SetMute(mute):
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kMute)
        mutestr = "%d" % mute
        if entry:
            entry.setValue(0,mutestr)
        else:
            gIniFile.addEntry("%s %d" % (kMute,mute))

def GetMute():
    if gIniFile:
        entry,idx = gIniFile.findByCommand(kMute)
        if entry:
            value = entry.getValue(0)
            if value:
                return string.atoi(value)
        return 0

def GetAudioMode():
    mode = None
    
    if gIniFile:
        entryInit,idxInit = gIniFile.findByCommand("Audio.Initialize")
        entryHard,idxHard = gIniFile.findByCommand("Audio.UseHardware")
        entryEAX,idxEAX = gIniFile.findByCommand("Audio.UseEAX")

        if entryEAX and entryEAX.getValue(0) == kBeTrue:
            mode = 3
        else:
            if entryHard and entryHard.getValue(0) == kBeTrue:
                mode = 2
            else:
                if entryInit and entryInit.getValue(0) == kBeTrue:
                    mode = 1
                else:
                    mode = 0
    else:
        mode = None

    return mode

def SetAudioMode(init, device, eax):
    global gIniFile

    if gIniFile:
        entryInit,idxInit = gIniFile.findByCommand("Audio.Initialize")
        entryDev,idxDev = gIniFile.findByCommand("Audio.SetDeviceName")
        entryEAX,idxEAX = gIniFile.findByCommand("Audio.UseEAX")

        if init:
            val = kBeTrue
        else:
            val = kBeFalse

        if entryInit:
            entryInit.setValue(0, val)
        else:
            gIniFile.addEntry("Audio.Initialize " + val)

        print device
        if entryDev:
            entryDev.setValue(0, "\"" + device + "\"")
        else:
            gIniFile.addEntry("Audio.SetDeviceName \"" + device + "\"")

        if eax:
            val = kBeTrue
        else:
            val = kBeFalse

        if entryEAX:
            entryEAX.setValue(0, val)
        else:
            gIniFile.addEntry("Audio.UseEAX " + val)
