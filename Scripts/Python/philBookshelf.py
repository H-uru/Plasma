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
Module: philBookshelf
Age: PhilRelto
Date: January 2004
Author: Adam Van Ornum
This handles the bookshelf in phil's personal age...only the neighborhood book does anything
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import PlasmaControlKeys

actBookshelf = ptAttribActivator(1, "Actvtr:Bookshelf")

actBook = ptAttribActivator(2,"Actvtr:Book")
respPresentBook = ptAttribResponder(3,"Rspndr:PresentBook")
respShelveBook = ptAttribResponder(4,"Rspndr:ShelveBook")

SeekBehavior = ptAttribBehavior(5, "Smart seek before GUI") # used to make user walk in front of shelf before using it
ShelfCamera = ptAttribSceneobject(6,"Bookshelf camera") # the camera used when engaging the shelf
HutCamera = ptAttribSceneobject(7,"Hut circle camera") # the camera which was used before engaging the shelf
actBookshelfExit = ptAttribActivator(8, "Actvr: Exit bookshelf")
respLinkOut = ptAttribResponder(9, "Resp: link out")
respMoveShelf = ptAttribResponder(10, "Resp: move shelf", ["raise", "lower"])

theBook = None
LocalAvatar = None


class philBookshelf(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5327
        self.version = 1
        minor = 1
        PtDebugPrint(('__init__philBookshelf v. %d.%d' % (self.version, minor)))



    def OnServerInitComplete(self):
        global LocalAvatar

        respMoveShelf.run(self.key, state = "lower", fastforward = 1)
        actBookshelfExit.disable()

        LocalAvatar = PtGetLocalAvatar()


    def OnNotify(self,state,id,events):
        boolLinkerIsMe = PtWasLocallyNotified(self.key)
        PtDebugPrint(('philBookshelf.OnNotify(): state = %d, id = %d, me = %s' % (state, id, boolLinkerIsMe)))
        
        if id == actBookshelfExit.id:
            self.IDisengageShelf(boolLinkerIsMe)
            return

        if id == SeekBehavior.id:
            for event in events:
                avatar = PtFindAvatar(events)
                if event[0] == kMultiStageEvent and event[1] == 0 and LocalAvatar == avatar: # Smart seek completed. Exit multistage, and show GUI.
                    SeekBehavior.gotoStage(avatar, -1) 
                    PtDebugPrint("philBookshelf.OnNotify():\tengaging bookshelf")
                    avatar.draw.disable()
                    # set camera to Shelf Camera
                    virtCam = ptCamera()
                    virtCam.save(ShelfCamera.sceneobject.getKey())

                    PtAtTimeCallback(self.key, .1, 1)

        if id == actBookshelf.id and state:
            respMoveShelf.run(self.key, state = "raise", fastforward = 1)
            avatar = PtFindAvatar(events)
            if LocalAvatar == avatar:
                # disable blackbar so people can't bring up other
                PtSendKIMessage(kDisableKIandBB,0)
                # Disable First Person Camera
                cam = ptCamera()
                cam.undoFirstPerson()
                cam.disableFirstPersonOverride()
                PtRecenterCamera()
                SeekBehavior.run(avatar)
                PtDisableMovementKeys()
                    
        elif id == actBook.id and state:
            actBook.disable()
            respPresentBook.run(self.key)
        
        # no linking book in MOUL but fix it anyway
        elif ((id == respPresentBook.id) and boolLinkerIsMe):
            global theBook
            
            # book is finished presenting - now link
            bookcode = '<font size=10><img src="xDRCBookRubberStamp2*1#0.hsm" pos=125,120 blend=alpha><pb><img src="xLinkPanelKirelDefault*1#0.hsm" align=center link=0 blend=alpha>'
            
            theBook = ptBook(bookcode, self.key)
            theBook.setGUI("BkBook")
            theBook.setSize(1.0, 1.0)
            theBook.show(1)
            
        elif id == respShelveBook.id:
            actBook.enable()

        else:
            for event in events:
                if event[0] == PtEventType.kBook:
                    PtDebugPrint("philBookshelf: BookNotify  event=%d, id=%d" % (event[1],event[2]))
                    if event[1] == PtBookEventTypes.kNotifyImageLink:
                        if event[2] >= 0:
                            PtDebugPrint("philBookshelf:Book: hit linking panel %s" % (event[2]))
                            theBook.hide()
                            #respShelveBook.run(self.key)
                            self.IDisengageShelf(boolLinkerIsMe)
                            respLinkOut.run(self.key)
                            
                    elif event[1] == PtBookEventTypes.kNotifyHide:
                        PtDebugPrint("philBookshelf:Book: NotifyHide")
                        
                        respShelveBook.run(self.key)
    

    def IDisengageShelf(self, boolLinkerIsMe = False):
        PtDebugPrint(('philBookshelf.IDisengageShelf(): me = %s' % boolLinkerIsMe))
        actBookshelfExit.disable()
        # fastforward removed because it disables netPropagate
        respMoveShelf.run(self.key, state = "lower")
        if boolLinkerIsMe:
            LocalAvatar.draw.enable()
            #reeneable first person
            cam = ptCamera()
            cam.enableFirstPersonOverride()
            # go back to the Hut Circle Cam
            virtCam = ptCamera()
            virtCam.save(HutCamera.sceneobject.getKey())
            PtEnableMovementKeys()
            PtGetControlEvents(False,self.key)
            PtSendKIMessage(kEnableKIandBB,0)



    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode or controlKey == PlasmaControlKeys.kKeyMoveBackward:
            self.IDisengageShelf(True)


    def OnTimer(self, id):
        if id == 1:
            PtGetControlEvents(True,self.key)
            actBookshelfExit.enable()


    def OnBackdoorMsg(self, target, param):
        if target == 'book':
            if param == 'enable':
                actBook.enable()
            elif param == 'disable':
                actBook.disable()

        elif target == 'shelf':
            if param == 'enable':
                actBookshelf.enable()
                actBookshelf.value[0].getSceneObject().physics.suppress(0)
            elif param == 'disable':
                actBookshelf.disable()
                actBookshelf.value[0].getSceneObject().physics.suppress(1)

