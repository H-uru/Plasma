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
Module: psnlMyCloset
Age: Personal Age
Date: March 2003
Author: Bill Slease

Updated in July 03 to link to ACA instead of showing GUI
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaNetConstants import *
import PlasmaControlKeys
import string

# ---------
# max wiring
# ---------

actClosetOpen = ptAttribActivator(1, "Open Activator")
respClosetOpen = ptAttribResponder(2, "Open Responder", netForce=1)
actClosetClose = ptAttribActivator(3, "Close Activator")
respClosetClose = ptAttribResponder(4, "Close Responder", netForce=1)
objOpenClosetBlockers = ptAttribSceneobjectList(9,"Door Blockers")

# ---------
# globals
# ---------

kSDLClosetClosed = "psnlClosetClosed"
boolAmOwner = false

AgeStartedIn = None

kCloseClosetTimer = 99
kVisitorDisableTimer = 100

class psnlMyCloset(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5016
        self.version = 6
        
    def __del__(self):
        pass

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

    def OnServerInitComplete(self):
        global boolAmOwner
        
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags(kSDLClosetClosed,1,1)
            ageSDL.sendToClients(kSDLClosetClosed)

            # register for notification of ClosetClosed SDL var changes
            ageSDL.setNotify(self.key,kSDLClosetClosed,0.0)
            
            ##################
            # Initialize the scene...  #
            ##################
    
            # if the owner is linking in: animate the closet closed if it's open, if we come from ACA, it will look right
            # if we come from logging in, we link in at the dock and won't see the animation anyway
            vault = ptVault()
            if vault.amOwnerOfCurrentAge():
                boolAmOwner = true
                PtDebugPrint("psnlCloset.OnServerInitComplete():\tWelcome Home!")
                try:
                    closetClosed = ageSDL[kSDLClosetClosed][0]
                except:
                    PtDebugPrint("psnlCloset.OnServerInitComplete():\tERROR reading SDL from vault, defaulting to closed (fastforward)")
                    self.ICloseCloset(1)
                if not closetClosed:
                    # just close the door for now!
                    #self.ICloseCloset(1)
                    PtDebugPrint("psnlCloset.OnServerInitComplete():\tCloset is open, so setting a timer to close it")
                    self.IOpenCloset(1) # fast forward it open (in case it starts closed for some reason)
                    PtAtTimeCallback(self.key, 2, kCloseClosetTimer) # we will close it in two seconds
                else:
                    PtDebugPrint("psnlCloset.OnServerInitComplete():\tCloset is closed, making sure the geometry matches")
                    self.ICloseCloset(1) # make sure the object is actually closed
            else: # not the owner: disable clickables, set correct closet door state
                PtDebugPrint("psnlCloset.OnServerInitComplete():\tWelcome Visitor")
                PtAtTimeCallback(self.key, 1, kVisitorDisableTimer) # we will disable the clickables in a second, since disabling them now doesn't work
                try:
                    closetClosed = ageSDL[kSDLClosetClosed][0]
                except:
                    PtDebugPrint("psnlCloset.OnServerInitComplete():\tERROR reading SDL from vault, defaulting closed")
                    closetClosed = true
                if not closetClosed: # assume closet is in use by owner
                    self.IOpenCloset(1) # fastforward the closet open
                else: # either closet is closed, or I'm alone...if I'm alone we want to force it closed anyway so...
                    self.ICloseCloset(1) # fastforward the closet closed
        return
    
    def IOpenCloset(self,ff=0,events=None):
        vault = ptVault()
        respClosetOpen.run(self.key,fastforward=ff,events=events)
        if vault.amOwnerOfCurrentAge():
            ageSDL = PtGetAgeSDL()
            ageSDL.setTagString(kSDLClosetClosed,"ignore")
            ageSDL[kSDLClosetClosed] = (0,)
        actClosetOpen.disable()
    
    def ICloseCloset(self,ff=0,events=None):
        vault = ptVault()
        respClosetClose.run(self.key,fastforward=ff,events=events)
        if vault.amOwnerOfCurrentAge():
            ageSDL = PtGetAgeSDL()
            ageSDL.setTagString(kSDLClosetClosed,"ignore")
            ageSDL[kSDLClosetClosed] = (1,)
        actClosetOpen.enable()
        for obj in objOpenClosetBlockers.value:
            obj.physics.suppress(true)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        # the closet doesn't really need to respond to SDL events...but in case I change my mind...
        if VARname == kSDLClosetClosed:
            if AgeStartedIn == PtGetAgeName():
                ageSDL = PtGetAgeSDL()
                PtDebugPrint("psnlClosetDoor.OnSDLNotify():\t VARname:%s, SDLname:%s, tag:%s, value:%d, playerID:%d" % (VARname,SDLname,tag,ageSDL[VARname][0],playerID))
            if tag == "ignore":
                return
            else:
                PtDebugPrint("psnlClosetDoor.OnSDLNotify():\ttag not ignore, ignoring anyway :P")
                return
    
    def OnNotify(self,state,id,events):
        # open the closet
        if id==actClosetOpen.id and state:
            if AgeStartedIn == PtGetAgeName():
                self.IOpenCloset(events=events)
        # closet has finished opening, handle the setup, and link out (if we were locally notified)
        elif id==respClosetOpen.id and state:
            for obj in objOpenClosetBlockers.value:
                obj.physics.suppress(false)
            if PtWasLocallyNotified(self.key):
                self.ILinkToACA() # link to the ACA
    
    def OnTimer(self, id):
        # are we trying to close the closet on link in?
        if id == kCloseClosetTimer:
            self.ICloseCloset()
        elif id == kVisitorDisableTimer:
            actClosetOpen.disable()
            actClosetClose.disable()
    
    def OnControlKeyEvent(self,controlKey,activeFlag):
        "exit closet"
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IExitCloset(false)
            return

    def ILinkToACA(self):
        PtDisableControlKeyEvents(self.key)
        PtSendKIMessage(kDisableKIandBB,0)
        PtDisableMovementKeys()
        ageLink = ptAgeLinkStruct()
        ageInfo = ageLink.getAgeInfo()
        temp = ptAgeInfoStruct()
        temp.copyFrom(ageInfo)
        ageInfo = temp
        ageInfo.setAgeFilename("AvatarCustomization")
        ageLink.setAgeInfo(ageInfo)
        ageLink.setLinkingRules(PtLinkingRules.kOriginalBook)
        linkmgr = ptNetLinkingMgr()
        linkmgr.linkToAge(ageLink)
