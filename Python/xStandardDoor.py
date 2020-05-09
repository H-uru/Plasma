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
Module: xStandardDoor
Age: Global
Date: January 2003
Author: Bill Slease

--- assumes high-level SDL in effect ---

VALID HINT STRINGS:
    fromOutside
    fromInside
    fromAuto  (currently only supported for closing not opening)
    ignore
    fastforward
"""

from Plasma import *
from PlasmaTypes import *

# ---------
# max wiring
# ---------

stringSDLVarClosed = ptAttribString(13,"SDL Bool Closed")
xrgnDoorBlocker = ptAttribExcludeRegion(14,"Exclude Region")

actInterior  = ptAttribActivator(15,"Clickable: interior")
respOpenInt = ptAttribResponder(16,"Rspndr: open from inside",netForce=0)

actExterior  = ptAttribActivator(17,"Clickable: exterior")
respOpenExt = ptAttribResponder(18,"Rspndr: open from outside",netForce=0)

boolCanManualClose = ptAttribBoolean(19,"Player can close me",default=1)
respCloseInt  = ptAttribResponder(20,"Rspndr: close from inside",netForce=0)
respCloseExt  = ptAttribResponder(21,"Rspndr: close from outside",netForce=0)

boolCanAutoClose = ptAttribBoolean(22,"Door can autoclose")
respAutoClose = ptAttribResponder(23,"Rspndr: Auto Close",netForce=0)

# doors that auto-close or auto-open can be left in bogus state if players lose connection etc.
# if I enter an age by myself and a door that should've auto-opened is closed...just open it and correct the state
boolForceOpen = ptAttribBoolean(24,"Force Open if Age Empty")
# if I enter an age by myself and a door that should've auto-closed is open...just close it and correct the state
boolForceClose = ptAttribBoolean(25,"Force Close if Age Empty")

boolOwnedDoor = ptAttribBoolean(26,"Only Owners Can Use",default=False)

stringSDLVarEnabled = ptAttribString(27,"SDL Bool Enabled (Optional)") # for lockable or powerable doors etc.

# ---------
# globals
# ---------
boolEnableOK = True # False if it's owned door and I'm not an owner, true otherwise -- set in OnServerInitComplete
AgeStartedIn = None

class xStandardDoor(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5031
        
        version = 6
        self.version = version
        PtDebugPrint("__init__xStandardDoor v.", version)
        self.DoorStack = []

    def OnFirstUpdate(self):
        PtDebugPrint("xStandardDoor: Located in age %s, Max Object %s" % (str(PtGetAgeName()),str(self.sceneobject.getName())))
        # get the age we started in
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
        if not stringSDLVarClosed.value:
            PtDebugPrint("xStandardDoor.OnFirstUpdate():\tERROR: missing SDL var name in max file")

    def OnServerInitComplete(self):
        global boolEnableOK
        
        # make sure that we are in the age we think we're in
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(stringSDLVarClosed.value,1,1)
            ageSDL.sendToClients(stringSDLVarClosed.value)
            
            # register for notification of doorClosed SDL var changes
            ageSDL.setNotify(self.key,stringSDLVarClosed.value,0.0)
            ageSDL.setNotify(self.key,stringSDLVarEnabled.value,0.0)
    
            # get initial SDL state
            try:
                doorClosed = ageSDL[stringSDLVarClosed.value][0]
            except:
                doorClosed = True
                PtDebugPrint("xStandardDoor.OnServerInitComplete():\tERROR: age sdl read failed, defaulting door closed value")
            PtDebugPrint("xStandardDoor.OnServerInitComplete():\tageSDL[%s] = %d" % (stringSDLVarClosed.value,doorClosed) )
            
            try:
                if stringSDLVarEnabled.value != "":
                    doorEnabled = ageSDL[stringSDLVarEnabled.value][0]
                else:
                    doorEnabled = True
            except:
                doorEnabled = True
                PtDebugPrint("xStandardDoor.OnServerInitComplete():\tERROR: age sdl read failed, defaulting door enabled")
    
            # correct SDL state if necessary
            if len(PtGetPlayerList()) == 0: # I'm the only person here
                if boolForceOpen.value and doorClosed:
                    doorClosed = 0
                    ageSDL.setTagString(stringSDLVarClosed.value,"ignore")
                    ageSDL[stringSDLVarClosed.value] = (0,)
                    PtDebugPrint("xStandardDoor.OnServerInitComplete():\tdoor closed, but I'm the only one here...opening")
                elif boolForceClose.value and not doorClosed:
                    doorClosed = 1
                    ageSDL.setTagString(stringSDLVarClosed.value,"ignore")
                    ageSDL[stringSDLVarClosed.value] = (1,)
                    PtDebugPrint("xStandardDoor.OnServerInitComplete():\tdoor open, but I'm the only one here...closing")
    
            # initialize door whatnots based on SDL state
            if not boolOwnedDoor.value:
                boolEnableOK = True
            else:
                vault = ptVault()
                if vault.amOwnerOfCurrentAge():
                    PtDebugPrint("xStandardDoor.OnServerInitComplete():\tWelcome Home!")
                    boolEnableOK = True
                else:
                    PtDebugPrint("xStandardDoor.OnServerInitComplete():\tWelcome Visitor.")
                    boolEnableOK = False
            if not doorEnabled:
                boolEnableOK = False
    
            if not doorClosed:
                xrgnDoorBlocker.releaseNow(self.key)
                if len(respOpenExt.value):
                    respOpenExt.run(self.key,fastforward=1)
                elif len(respOpenInt.value):
                    respOpenInt.run(self.key,fastforward=1)
                else:
                    PtDebugPrint("xStandardDoor.OnServerInitComplete():\tERROR - no open responder defined, can't init door open")
                if boolCanManualClose.value and boolEnableOK:
                    actExterior.enable()
                    actInterior.enable()
                else:
                    actExterior.disable()
                    actInterior.disable()
            else:
                xrgnDoorBlocker.clearNow(self.key)
                if boolCanAutoClose.value:
                    respAutoClose.run(self.key,fastforward=1)
                elif boolCanManualClose.value:
                    if len(respCloseExt.value):
                        respCloseExt.run(self.key,fastforward=1)
                    elif len(respCloseInt.value):
                        respCloseInt.run(self.key,fastforward=1)
                    else:
                        PtDebugPrint("xStandardDoor.OnServerInitComplete():\tERROR - no close responder defined, can't init door closed")
                else:
                    PtDebugPrint("xStandardDoor.OnServerInitComplete():\tWARNING: door set to neither manual close nor auto close, can't init closed")
                if boolEnableOK:
                    actExterior.enable()
                    actInterior.enable()
                else:
                    actExterior.disable()
                    actInterior.disable()


    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global boolEnableOK
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if VARname == stringSDLVarEnabled.value:
                PtDebugPrint("xStandardDoor.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[stringSDLVarEnabled.value][0],playerID))
                doorEnabled = ageSDL[stringSDLVarEnabled.value][0]
    
                if not doorEnabled:
                    PtDebugPrint("xStandardDoor.OnSDLNotify():\tDoor Disabled")
                    boolEnableOK = False
                    actExterior.disable()
                    actInterior.disable()
                    return
                    
                if not boolOwnedDoor.value:
                    PtDebugPrint("xStandardDoor.OnSDLNotify():\tDoor Enabled")
                    boolEnableOK = True
                    actExterior.enable()
                    actInterior.enable()
                    return
                else:
                    vault = ptVault()
                    if vault.amOwnerOfCurrentAge():
                        PtDebugPrint("xStandardDoor.OnSDLNotify():\tOwners-Only Door Enabled")
                        boolEnableOK = True
                        actExterior.enable()
                        actInterior.enable()
                        return
                    PtDebugPrint("xStandardDoor.OnSDLNotify():\tOwners-Only Door Enabled...but I'm not an owner")
                    return
                
            if VARname == stringSDLVarClosed.value:
                PtDebugPrint("xStandardDoor.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[stringSDLVarClosed.value][0],playerID))
                
                if tag == "ignore": 
                    return
    
                # is door state change from player or vault manager?
                if playerID:
                    objAvatar = ptSceneobject(PtGetAvatarKeyFromClientID(playerID),self.key)
                    fastforward = 0
                else:   # playerID == 0 --> invalid player aka Vault Manager
                    objAvatar = None
                    fastforward = 1 # pop door to it's new state (skip one-shots that require valid player)
                PtDebugPrint("xStandardDoor.OnSDLNotify():\tnotification from playerID: %d" % (playerID))
                
                #if tag == "fastforward":
                #    fastforward = 1
                
                actExterior.disable()
                actInterior.disable()
                if ageSDL[stringSDLVarClosed.value][0]: 
                    # doorClosed changed to true (close the door)
                    try:
                        OkTags = ["fromOutside", "fromInside", "fromAuto", "fastforward"]
                        if tag in OkTags:
                            self.SendNote('%s;%d;%d;true' % (tag, playerID, fastforward), playerID)
                            PtDebugPrint('xStandardDoor.OnSDLNotify():\tClosing ', tag) 
                        else:
                            PtDebugPrint("xStandardDoor.OnSDLNotify():\tWARNING missing or invalid hint string:%s" % (tag)) # ok if from vaultmanager
                            fastforward = 1
                            if boolCanManualClose.value:
                                respCloseExt.run(self.key,fastforward=fastforward)
                            elif boolCanAutoClose.value:
                                respAutoClose.run(self.key,fastforward=fastforward)
                            else:
                                PtDebugPrint("xStandardDoor.OnSDLNotify():\tWARNING: door set to neither manual close nor auto close, can't close")
                    except:
                        PtDebugPrint("xStandardDoor.OnSDLNotify():\tERROR processing sdl var %s hint string: %s" % (VARname,tag))
                    if fastforward: # reenable clickables and xregion state via responder won't happen so do it manually
                        if boolEnableOK:
                            actExterior.enable()
                            actInterior.enable()
                        xrgnDoorBlocker.clearNow(self.key)                    
                else:
                    # doorClosed changed to False (open the door)
                    try:
                        OkTags = ["fromOutside", "fromInside"]
                        if tag in OkTags:
                            self.SendNote('%s;%d;%d;false' % (tag, playerID, fastforward), playerID)
                            PtDebugPrint('xStandardDoor.OnSDLNotify():\tOpening ', tag)
                        else:
                            PtDebugPrint("xStandardDoor.OnSDLNotify():\tWARNING missing or invalid hint string:%s" % (tag))
                            fastforward = 1
                            respOpenExt.run(self.key,fastforward=fastforward)
                    except:
                        PtDebugPrint("xStandardDoor.OnSDLNotify():\tERROR processing sdl var %s hint string: %s" % (VARname,tag))
                        fastforward = 1
                        respOpenExt.run(self.key,fastforward=fastforward)
                    if fastforward: # reenable clickables and xregion state via responder won't happen so do it manually
                        if boolCanManualClose.value and boolEnableOK:
                            actExterior.enable()
                            actInterior.enable()
                        xrgnDoorBlocker.releaseNow(self.key)                    

    def OnNotify(self,state,id,events):
        PtDebugPrint("xStandardDoor: ID notified:", id)
        
        if id == -1:
            self.DoorStack.append(events[0][1])
            PtDebugPrint("xStandardDoor: New list is: %s" % (str(self.DoorStack)))

            if len(self.DoorStack) == 1:
                PtDebugPrint("xStandardDoor: List is only one command long, so I'm playing it")
                code = self.DoorStack[0]
                PtDebugPrint("xStandardDoor: Playing command: %s" % (code))
                self.ExecCode(code)
                return

        # reenable clickables after door open/close anim runs
        elif ( id==respOpenExt.id or id==respOpenInt.id ):
            self.UpdateRespStack()
            if boolCanManualClose.value and boolEnableOK:                
                actExterior.enable()
                actInterior.enable()
        elif ( id==respCloseExt.id or id==respCloseInt.id or id==respAutoClose.id):
            self.UpdateRespStack()
            if boolEnableOK:
                actExterior.enable()
                actInterior.enable()

    def SendNote(self, ExtraInfo, avatar):
        PtDebugPrint("xStandardDoor: Avatar who did this:", avatar)
        if avatar == 0:
            notify = ptNotify(self.key)
            notify.clearReceivers()
            notify.addReceiver(self.key)
            notify.netPropagate(1)
            notify.netForce(1)
            notify.setActivate(1.0)
            notify.addVarNumber(str(ExtraInfo),1.0)
            notify.send()
            
        elif ptSceneobject(PtGetAvatarKeyFromClientID(avatar),self.key) == PtGetLocalAvatar():
            notify = ptNotify(self.key)
            notify.clearReceivers()
            notify.addReceiver(self.key)
            notify.netPropagate(1)
            notify.netForce(1)
            notify.setActivate(1.0)
            notify.addVarNumber(str(ExtraInfo),1.0)
            notify.send()

    def UpdateRespStack (self):
        #Updates the Responder List
        old = self.DoorStack.pop(0)
        PtDebugPrint("xStandardDoor: Getting rid of Resp: %s" % (old))
        if len(self.DoorStack):            
            PtDebugPrint("xStandardDoor: There's at lest one more Resp to play.")
            code = self.DoorStack[0]            
            PtDebugPrint("Playing command: %s" % (code))
            self.ExecCode(code)

    def ExecCode (self, code):
        try:
            chunks = code.split(';')
            tag = chunks[0];
            playerId = int(chunks[1])
            fastForward = int(chunks[2])
            doorClosed = chunks[3]
            if doorClosed == "true": # no easy way to convert strings to bools in python
                if tag == "fromOutside":
                    respCloseExt.run(self.key,avatar=ptSceneobject(PtGetAvatarKeyFromClientID(playerId),self.key),fastforward=fastForward,netPropagate=0)
                elif tag == "fromInside":
                    respCloseInt.run(self.key,avatar=ptSceneobject(PtGetAvatarKeyFromClientID(playerId),self.key),fastforward=fastForward,netPropagate=0)
                elif tag == "fromAuto":
                    respAutoClose.run(self.key,fastforward=fastForward,netPropagate=0)
                elif tag == "fastforward":
                    respCloseExt.run(self.key,avatar=ptSceneobject(PtGetAvatarKeyFromClientID(playerId),self.key),fastforward=1,netPropagate=0)
                else:
                    PtDebugPrint("xStandardDoor.ExecCode(): ERROR! Invalid tag '%s'." % (tag))
                    self.DoorStack.pop(0)
            else:
                if tag == "fromOutside":
                    respOpenExt.run(self.key,avatar=ptSceneobject(PtGetAvatarKeyFromClientID(playerId),self.key),fastforward=fastForward,netPropagate=0)
                elif tag == "fromInside" or playerId == 0:
                    respOpenInt.run(self.key,avatar=ptSceneobject(PtGetAvatarKeyFromClientID(playerId),self.key),fastforward=fastForward,netPropagate=0)
                else:
                    PtDebugPrint("xStandardDoor.ExecCode(): ERROR! Invalid tag '%s'." % (tag))
                    self.DoorStack.pop(0)
        except:
            PtDebugPrint("xStandardDoor.ExecCode(): ERROR! Invalid code '%s'." % (code))
            self.DoorStack.pop(0)