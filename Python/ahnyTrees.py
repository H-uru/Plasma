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
Module: ahnyTrees
Age: Ahnonay
Date: April, 2007
Author: Derek Odell
Ahnonay Quab control
"""

from Plasma import *
from PlasmaTypes import *


# define the attributes that will be entered in max
rgnTrees                    = ptAttribActivator(1, "act: Tree Detector")
respTreeAnims               = ptAttribResponderList(2, "resp: Tree Anims", byObject=1)
objTrees                    = ptAttribSceneobjectList(3, "obj: Tree Meshs")
SDLTrees                    = ptAttribString(4, "str: SDL Trees (optional)")

# globals
respTreeAnimsList           = []
objTreeList                 = []

#====================================
class ahnyTrees(ptModifier):
    ###########################
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5948
        version = 1
        self.version = version
        PtDebugPrint("__init__ahnyTrees v%d " % (version))

    ###########################
    def OnFirstUpdate(self):
        global respTreeAnimsList
        global objTreeList

        try:
            ageSDL = PtGetAgeSDL()
            ageSDL[SDLTrees.value][0]
        except:
            PtDebugPrint("ahnyTrees.OnServerInitComplete(): ERROR --- Cannot find the Ahnonay Age SDL")
            ageSDL[SDLTrees.value] = (1,1,1,1,1,1,1,1,1,1,1,1,1,1,1)

        ageSDL.setFlags(SDLTrees.value,1,1)
        ageSDL.sendToClients(SDLTrees.value)
        ageSDL.setNotify(self.key,SDLTrees.value,0.0)

        for responder in respTreeAnims.value:
            thisResp = responder.getName()
            respTreeAnimsList.append(thisResp)

        for object in objTrees.value:
            thisObj = object.getName()
            objTreeList.append(thisObj)

        ageSDL = PtGetAgeSDL()
        idx = 0
        for visible in ageSDL[SDLTrees.value]:
            if not visible:
                respTreeAnims.run(self.key, objectName=respTreeAnimsList[idx], fastforward=1)
            idx += 1

    ###########################
    def OnNotify(self,state,id,events):
        global respTreeAnimsList
        global objTreeList
        PtDebugPrint("ahnyTrees.OnNotify: state=%s id=%d events=" % (state, id), events)

        if id == rgnTrees.id:
            for event in events:
                if event[0] == kCollisionEvent and self.sceneobject.isLocallyOwned() :
                    region = event[3]
                    regName = region.getName()
                    for object in objTreeList:
                        if object == regName:
                            ageSDL = PtGetAgeSDL()
                            treeSDL = list(ageSDL[SDLTrees.value])
                            index = objTreeList.index(object)
                            if treeSDL[index]:
                                respTreeAnims.run(self.key, objectName=respTreeAnimsList[index], netForce = 1)
                                treeSDL[index] = 0
                                ageSDL[SDLTrees.value] = tuple(treeSDL)
                                PtDebugPrint("ahnyTrees.OnNotify: Tree knocked down")

