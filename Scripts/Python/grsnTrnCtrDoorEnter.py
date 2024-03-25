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

arrivePt     = ptAttribSceneobject(1,"arrive point")
triggerRgn1 = ptAttribActivator(2,"door 1 region sensor")
triggerRgn2 = ptAttribNamedActivator(3,"door 2 region sensor")
door1OpenResponder = ptAttribResponder(4,"door 1 open responder",netForce=1)
door1CloseResponder = ptAttribResponder(5,"door 1 close responder",netForce=1)
door2OpenResponder = ptAttribNamedResponder(6,"door 2 open responder",netForce=1)
door2CloseResponder = ptAttribNamedResponder(7,"door 2 close responder",netForce=1)
behaviorWalkIn = ptAttribBehavior(8,"walk in behavior",netForce=1)
behaviorWalkOut = ptAttribBehavior(9,"walk out behavior",netForce=1)
subWorld = ptAttribSceneobject(10,"subworld")
camera = ptAttribSceneobject(11,"exit camera")

avatarEntering = 0

class grsnTrnCtrDoorEnter(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 3276
        self.version = 1
        PtDebugPrint("grsnTrnCtrDoorEnter: Max version %d - minor version %d" % (self.version,2))

    def OnNotify(self,state,id,events):
        global avatarEntering
        
        if (id == triggerRgn1.id):
            if (PtFindAvatar(events) != PtGetLocalAvatar()):
                return
            PtDebugPrint(" must have ki")
            kiLevel = PtGetLocalKILevel()
            if (kiLevel < 2):
                return
            for event in events:
                if (event[0]==1 and event[1]==1):
                    avatarEntering = PtFindAvatar(events)
                    PtDebugPrint("entered the region, disable this one and the other door's triggers")
                    triggerRgn1.disable()
                    triggerRgn2.disable()
                    if (avatarEntering == PtGetLocalAvatar()):
                        PtDebugPrint("stop this avatar")
                        PtDisableMovementKeys()
                        PtDebugPrint("take away first person")
                        cam = ptCamera()
                        cam.disableFirstPersonOverride()
                        cam.undoFirstPerson()
                        PtSendKIMessage(kDisableEntireYeeshaBook,0)
                    PtDebugPrint("open the door")
                    door1OpenResponder.run(self.key,avatar=avatarEntering)
                    return
        
        if (id == door1OpenResponder.id):
            if (avatarEntering != PtGetLocalAvatar()):
                return
            
            PtDebugPrint(" door is open, walk in")
            avatarEntering.avatar.runBehaviorSetNotify(behaviorWalkIn.value,self.key,behaviorWalkIn.netForce)
            return
        
        if (id == behaviorWalkIn.id):
            if (avatarEntering != PtGetLocalAvatar()):
                return
            
            for event in events:
                if event[0] == kMultiStageEvent and event[2] == kAdvanceNextStage: 
                    PtDebugPrint(" Smart seek completed. Exit multistage, close exterior door")
                    behaviorWalkIn.gotoStage(avatarEntering,-1)
                    door1CloseResponder.run(self.key,avatar=avatarEntering)
                    return
        
        if (id == door1CloseResponder.id):
            if (avatarEntering != PtGetLocalAvatar()):
                return
            
            PtDebugPrint("door closed, teleport and open other door")
            if (avatarEntering == PtGetLocalAvatar()):
                camera.value.pushCameraCut(avatarEntering.getKey())
            avatarEntering.avatar.exitSubWorld()
            avatarEntering.physics.warpObj(arrivePt.value.getKey())
            door2OpenResponder.run(self.key,avatar=avatarEntering)
            return
            
        if (id == door2OpenResponder.id):
            if (avatarEntering != PtGetLocalAvatar()):
                return
            
            PtDebugPrint("interior door open, walk out")
            avatarEntering.avatar.runBehaviorSetNotify(behaviorWalkOut.value,self.key,behaviorWalkOut.netForce)
            return
            
        if (id == behaviorWalkOut.id):
            if (avatarEntering != PtGetLocalAvatar()):
                return
            
            for event in events:
                if event[0] == kMultiStageEvent and event[2] == kAdvanceNextStage: 
                    PtDebugPrint(" Smart seek completed. Exit multistage, close interior door")
                    door2CloseResponder.run(self.key,avatar=avatarEntering)
                    behaviorWalkOut.gotoStage(avatarEntering,-1)
                    return
        
        if (id == door2CloseResponder.id):
            if (avatarEntering != PtGetLocalAvatar()):
                return
            
            PtDebugPrint(" process complete, re-enable detectors and free avatar ")
            triggerRgn1.enable()
            triggerRgn2.enable()
            if (avatarEntering == PtGetLocalAvatar()):
                PtEnableMovementKeys()
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                PtSendKIMessage(kEnableEntireYeeshaBook,0)
            avatarEntering = 0

