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
Module: nb01RegisterNexusLink
Age: Neighborhood
Date: October, 2002
Author: Bill Slease
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
#~ from PlasmaVaultConstants import *
#~ from PlasmaNetConstants import *


# define the attributes that will be entered in max
actClick = ptAttribActivator(1,"Actvtr: click me")
respOneshot = ptAttribResponder(2,"Rspndr: one shot")
respKIGlow =  ptAttribResponder(3,"Rspndr: ki glow",netForce=1)

# globals
boolClickerIsMe = false
myID = 0
hoodID = 0
hoodMgr = None
kInviteTitle = ""
kInviteMsg = ""

class nb01RegisterNexusLink(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5020
        version = 1
        self.version = version
        print "__init__nb01RegisterNexusLink v.", version
        
    def OnNotify(self,state,id,events):
        global boolClickerIsMe
        global myID
        global hoodID
        global hoodMgr
        
        #######################
        ##
        ##  stick your hand in the slot 
        ##
        #######################
        
        if id==actClick.id:
            if not state:
                return
            respOneshot.run(self.key,events=events)
            if PtWasLocallyNotified(self.key):
                boolClickerIsMe = true
                objAvatar = PtFindAvatar(events)
                myID = PtGetClientIDFromAvatarKey(objAvatar.getKey())
            return
            
        ##################################
        ##
        ##  figure out what to do with the hand in the slot
        ##
        ##################################
        # Udpated to do nothing. -eap.
        
        if id == respOneshot.id and boolClickerIsMe:
            if not state:
                return
            boolClickerIsMe = false # done with this var, reset it
            
            kiLevel = PtDetermineKILevel()
            print "nb01RegisterNexusLink.OnNotify:\tplayer ki level is",kiLevel

            # case 1:  player has no KI
            # ki slot doesn't respond, just return
            
            if kiLevel < kNormalKI:
                return
#            else:
#                respKIGlow.run(self.key)
#                
#            # case 2: player is in their own hood
#            # take no further action
#            
#            linkMgr = ptNetLinkingMgr()
#            if ( linkMgr.inMyNeighborhoodAge() ):
#                PtDebugPrint("nb01RegisterNexusLink:\tYou are already a member of this Neighborhood.")
#                return
#                
#            # case 3: player not a member, has a KI, but has not been invited to join
#            # player should become registered visitor of this hood
#            
#            hoodMgr = ptNeighborhoodMgr()
#            if type(hoodMgr) == type(None):
#                PtDebugPrint("nb01RegisterNexusLink:\thood manager type is None")
#                return
#            ageVault = ptAgeVault()
#            if type(ageVault) == type(None):
#                PtDebugPrint("nb01RegisterNexusLink:\tage vault type is None")
#                return
#            hoodNode = ageVault.getNeighborhood()
#            if type(hoodNode) == type(None):
#                PtDebugPrint("nb01RegisterNexusLink:\thood node type is None")
#                return
#            hoodID = hoodNode.getID()
#            if not hoodMgr.canJoin(hoodID):
#                hoodMgr.invitePlayer(myID,kInviteTypeHoodVisitInvitation,kInviteTitle,kInviteMsg)
#                PtDebugPrint("nb01RegisterNexusLink:\tYou are now a registered visitor of this Neighborhood.")
#                return
#
#            # case 3: player has KI and has been invited to become a member of this hood
#            # player can choose to join or if declines, becomes visitor as well as prosepective member
#            joinString = "You have been invited to join our neighborhood.  Would you like to become a member?"
#            ageVault = ptAgeVault()
#            if type(ageVault) != type(None):
#                hoodNode = ageVault.getNeighborhood()
#                hoodName = hoodNode.hoodGetTitle()
#                joinString = "You have been invited to join our neighborhood.  Would you like to become a member of %s?" % hoodName
#            PtYesNoDialog(self.key,joinString)
#            return
#
#        # callback from "do you want to join?" yes/no dialog (hopefully)
#        if id==(-1):
#            if state: # user answered yes, they join
#                hoodMgr.joinNeighborhood(hoodID)
#                PtDebugPrint("nb01RegisterNexusLink:\tYou are now a member of this Neighborhood!")
#            else: # user answered no, the get added to visitor list
#                hoodMgr.invitePlayer(myID,kInviteTypeHoodVisitInvitation,kInviteTitle,kInviteMsg)
#                PtDebugPrint("nb01RegisterNexusLink:\tYou are now a registered visitor of this Neighborhood.")
#                return
#
#
