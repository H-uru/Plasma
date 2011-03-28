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
This module contains the routines to read and write the Graphics.ini file
"""

import string
import xIniHelper
from Plasma import *

gIniFile = None
gFilename = U"graphics.ini"
gFilenameAndPath = U""

kGraphicsWidth = "Graphics.Width"
kGraphicsHeight = "Graphics.Height"
kGraphicsColorDepth = "Graphics.ColorDepth"
kGraphicsWindowed = "Graphics.Windowed"
kGraphicsTextureQuality = "Graphics.TextureQuality"
kGraphicsAntiAliasLevel = "Graphics.AntiAliasAmount"
kGraphicsAnisotropicLevel = "Graphics.AnisotropicLevel"
kGraphicsQualityLevel = "Quality.Level"
kGraphicsShadows = "Graphics.Shadow.Enable"
kGraphicsVerticalSync = "Graphics.EnableVSync"
kGraphicsShadowQuality = "Graphics.Shadow.VisibleDistance"

CmdList = [kGraphicsWidth, kGraphicsHeight, kGraphicsColorDepth, kGraphicsWindowed, kGraphicsTextureQuality, kGraphicsAntiAliasLevel, kGraphicsAnisotropicLevel, kGraphicsQualityLevel, kGraphicsShadows, kGraphicsVerticalSync, kGraphicsShadowQuality]
DefaultsList = ["800", "600", "32", "false", "2", "0", "0", "2", "true", "false", "0"]

def ConstructFilenameAndPath():
    global gFilenameAndPath
    if gFilenameAndPath == U"":
        if PtIsInternalRelease():
            # check for local file
            localNameAndPath = U"init/" + gFilename
            if PtFileExists(localNameAndPath):
                gFilenameAndPath = localNameAndPath
                print U"xIniDisplay::ConstructFilenameAndPath(): Using internal \"" + gFilenameAndPath + U"\" file"
                return
        # otherwise, use the standard init path
        gFilenameAndPath = PtGetInitPath() + U"/" + gFilename
        print U"xIniDisplay::ConstructFilenameAndPath(): Using user-level \"" + gFilenameAndPath + U"\" file"

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
        gIniFile.addEntry(kGraphicsWidth + " 800")
        gIniFile.addEntry(kGraphicsHeight + " 600")
        gIniFile.addEntry(kGraphicsColorDepth + " 32")
        gIniFile.addEntry(kGraphicsWindowed + " false")
        gIniFile.addEntry(kGraphicsTextureQuality + " 2")
        gIniFile.addEntry(kGraphicsAntiAliasLevel + " 0")
        gIniFile.addEntry(kGraphicsAnisotropicLevel + " 0")
        gIniFile.addEntry(kGraphicsQualityLevel + " 2")
        gIniFile.addEntry(kGraphicsShadows + " true")
        gIniFile.addEntry(kGraphicsVerticalSync + " false")
        gIniFile.addEntry(kGraphicsShadowQuality + " 0")
        gIniFile.writeFile(gFilenameAndPath)

    else:
        iniChanged = 0
        for idx in range(len(CmdList)):
            cmd = CmdList[idx]
            entry,idx = gIniFile.findByCommand(cmd)
            if not entry:
                gIniFile.addEntry(cmd + " " + DefaultsList[idx])
                iniChanged = 1

        if iniChanged:
            ConstructFilenameAndPath()
            gIniFile.writeFile(gFilenameAndPath)

def SetGraphicsOptions(width, heigth, colordepth, windowed, texquality, aaLevel, anisoLevel, qualityLevel, useShadows, vsync, shadowqual):
    if gIniFile:
        paramList = [width, heigth, colordepth, windowed, texquality, aaLevel, anisoLevel, qualityLevel, useShadows, vsync, shadowqual]
        for idx in range(len(CmdList)):
            entry,junk = gIniFile.findByCommand(CmdList[idx])
            val = str(paramList[idx])
            if entry:
                entry.setValue(0, val)
            else:
                gIniFile.addEntry("%s %s" % (CmdList[idx], val))
                
def GetGraphicsOptions():
    optsList = {}
    if gIniFile:
        for cmd in CmdList:
            entry,idx = gIniFile.findByCommand(cmd)
            if entry:
                value = entry.getValue(0)
                if value:
                    try:
                        optsList[cmd] = int(value)
                    except ValueError:
                        optsList[cmd] = value
    return optsList
