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

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

act = ptAttribActivator(1, "Act: Buttons")
resp = ptAttribResponder(2, "Resp: Button Push")
ageFileName = ptAttribString(3, "age filename")
ageInstanceName = ptAttribString(4, "age instance name")

class xRegisterAge(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5008
        version = 1
        self.version = version
        PtDebugPrint("__init__xRegisterAge v.", version)
        
    def OnNotify(self,state,id,events):
        if state:
            if id==act.id:
                if not self.IFindAgeLinkInFolder(ageFileName.value):
                    if resp.value:
                        resp.run(self.key,events=events)
                    PtSendKIMessage(kKILocalChatStatusMsg, PtGetLocalizedString("Global.Messages.AgeBookGet", [ageInstanceName.value]))
                    self.CheckAndRegisterAge(ageFileName.value, ageInstanceName.value)

    def IFindAgeLinkInFolder(self, ageName):
        vault = ptVault()
        folder = vault.getAgesIOwnFolder()
        ageName = ageName.lower()
        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild().upcastToAgeLinkNode()
            if link is not None:
                info = link.getAgeInfo()
            else:
                link = content.getChild()
                info = link.upcastToAgeInfoNode()

            if info.getAgeFilename().lower() == ageName:
                return link
        return None

    def CheckAndRegisterAge(self, ageFileName, ageInstanceName):
        PtDebugPrint("xRegisterAge.CheckAndRegisterAge(): ",ageFileName,"; ",ageInstanceName)
        vault = ptVault()
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename(ageFileName)
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if not ageLinkNode:
            info = ptAgeInfoStruct()
            info.setAgeFilename(ageFileName)
            info.setAgeInstanceName(ageInstanceName)

            playerName = PtGetClientName()
            ageGuid = PtGuidGenerate()
            userDefName = ""
            desc = ""

            if playerName[-1] == "s" or playerName[-1] == "S":
                userDefName = "%s'" % playerName
                desc = "%s' %s" % (playerName, info.getAgeInstanceName())
            else:
                userDefName = "%s's" % playerName
                desc = "%s's %s" % (playerName, info.getAgeInstanceName())

            info.setAgeInstanceGuid(ageGuid)
            info.setAgeUserDefinedName(userDefName)
            info.setAgeDescription(desc)

            link = ptAgeLinkStruct()
            link.setAgeInfo(info)

            ptVault().registerOwnedAge(link)
            PtDebugPrint("xRegisterAge.CheckAndRegisterAge(): Registered age - ", ageFileName)