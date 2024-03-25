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
Module: Descent.py
Age: Descent
Date: January 2004
Event Manager hooks for Descent
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *

class Descent(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5325
        self.version = 1

    def OnServerInitComplete(self):
        self.ICityBookLink()

    def OnVaultNotify(self,type,tupdata):
        # was an owned age just added?
        if type == PtVaultNotifyTypes.kRegisteredOwnedAge:
            link = tupdata[0]
            name = link.getAgeInfo().getAgeFilename()
            if name == "Descent":
                cityBookSP = ptSpawnPointInfo("dsntShaftFall","LinkInPointShaftFall")
                link.addSpawnPoint(cityBookSP)
                link.save()
                PtDebugPrint("Descent - setting the spawnpoint for the city book - OnVaultNotify",level=kDebugDumpLevel)
            else:
                PtDebugPrint("Descent - registering age '%s' (why is this not Descent?)"%(name))
        else:
            PtDebugPrint("Descent:OnVaultNotify - not what we want - type=%d"%(type),level=kDebugDumpLevel)

    def ICityBookLink(self):
        vault = ptVault()
        folder = vault.getAgesIOwnFolder()
        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            name = link.getAgeInfo().getAgeFilename()
            if name == "Descent":
                cityBookSP = ptSpawnPointInfo("dsntShaftFall","LinkInPointShaftFall")
                link.addSpawnPoint(cityBookSP)
                link.save()
                PtDebugPrint("Descent:OnServerInitComplete - setting the spawnpoint for the city book",level=kDebugDumpLevel)
                return
        PtDebugPrint("Descent:OnServerInitComplete - error - Descent is not there... hopefully OnVaultNotify catches it")
        return
