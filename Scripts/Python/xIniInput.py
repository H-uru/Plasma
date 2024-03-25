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
This module contains the routines to read and write the input.ini file
"""

import xIniHelper


gFilename = "./init/input.fni"
gIniFile = None

# the different volume commands
kBindCmd = "Keyboard.BindAction"
kConsoleBind = "Keyboard.BindConsoleCmd"

# the bind actions
kWalkForward = '"Walk Forward"'
kWalkBack = '"Walk Backward"'
kTurnLeft = '"Turn Left"'
kTurnRight = '"Turn Right"'
kJump = '"Jump Key"'
kExit = '"Exit Mode"'
kExiGUI = '"Exit GUI Mode"'
kTalk = '"Push to talk"'


def WriteIni():
    global gIniFile
    if gIniFile:
        gIniFile.writeFile(gFilename)

def ReadIni():
    global gIniFile
    gIniFile = xIniHelper.iniFile(gFilename)
    if gIniFile.isEmpty():
        # add defaults
        gIniFile.addEntry("# This is an auto-generated file.")
        gIniFile.addEntry("")

def SetControlKey(controlKey,primary="(unmapped),",secondary="(unmapped),"):
    if gIniFile:
        entry,idx = gIniFile.findByLastValue(controlKey)
        if entry:
            entry.setValue(0,primary)
            entry.setValue(1,secondary)
        else:
            gIniFile.addEntry("%s %s %s %s" % (kBindCmd,primary,secondary,controlKey))

def GetControlKey(controlKey):
    if gIniFile:
        entry,idx = gIniFile.findByLastValue(controlKey)
        if entry and entry.getValue(0) and entry.getValue(1):
            return entry.getValue(0),entry.getValue(1)
        else:
            return "(unmapped),","(unmapped),"

def SetConsoleKey(consoleCommand,primary="(unmapped),"):
    if gIniFile:
        entry,idx = gIniFile.findByLastValue(consoleCommand)
        if entry:
            entry.setValue(0,primary)
        else:
            gIniFile.addEntry("%s %s %s" % (kConsoleBind,primary,consoleCommand))

