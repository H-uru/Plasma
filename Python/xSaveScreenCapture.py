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
Module: xSaveScreenCapture
Age: Global
Date: November 2003
Author: Adam Van Ornum
Saves a screen capture started elsewhere
"""

from Plasma import *
from PlasmaTypes import *

strFileName = ptAttribString(1, "File name")
intQuality = ptAttribInt(2,"JPEG Quality", default=75)

class xSaveScreenCapture(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5319
        self.version = 1
        print "__init__xSaveScreenCapture v.", self.version

    def OnScreenCaptureDone(self,image):
        if type(strFileName.value) == type("") and strFileName.value != "":
            image.saveAsJPEG(strFileName.value, intQuality.value)
    