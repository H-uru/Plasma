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
Module: xDialogStartUp
Age: Global
Date: March 2006
Author: Derek Odell
Start Up dialog logic
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
import PlasmaControlKeys

import re
import webbrowser

# define the attributes that will be entered in max
GUIDiag4a   = ptAttribGUIDialog(1, "GUI Dialog 4a")
GUIDiag4b   = ptAttribGUIDialog(2, "GUI Dialog 4b")
GUIDiag4c   = ptAttribGUIDialog(3, "GUI Dialog 4c")
GUIDiag4d   = ptAttribGUIDialog(4, "GUI Dialog 4d")
GUIDiag5    = ptAttribGUIDialog(5, "GUI Dialog 5")
GUIDiag6    = ptAttribGUIDialog(6, "GUI Dialog 6")

resp4aPlayer01  = ptAttribResponder(7, "resp: 4A Player01", ["in", "out"])
resp4aPlayer02  = ptAttribResponder(8, "resp: 4A Player02", ["in", "out"])
resp4aPlayer03  = ptAttribResponder(9, "resp: 4A Player03", ["in", "out"])
resp4aPlayer04  = ptAttribResponder(10, "resp: 4A Player04", ["in", "out"])
resp4aPlayer05  = ptAttribResponder(11, "resp: 4A Player05", ["in", "out"])
resp4aPlayer06  = ptAttribResponder(12, "resp: 4A Player06", ["in", "out"])

resp4bPlayer01  = ptAttribResponder(13, "resp: 4B Player01", ["in", "out"])
resp4bPlayer02  = ptAttribResponder(14, "resp: 4B Player02", ["in", "out"])
resp4bPlayer03  = ptAttribResponder(15, "resp: 4B Player03", ["in", "out"])
resp4bPlayer04  = ptAttribResponder(16, "resp: 4B Player04", ["in", "out"])
resp4bPlayer05  = ptAttribResponder(17, "resp: 4B Player05", ["in", "out"])
resp4bPlayer06  = ptAttribResponder(18, "resp: 4B Player06", ["in", "out"])

respLink        = ptAttribResponder(19, "resp: Link", ["ACA", "Personal"])

mapPlayer01     = ptAttribDynamicMap(20, "map: Player01")
mapPlayer02     = ptAttribDynamicMap(21, "map: Player02")
mapPlayer03     = ptAttribDynamicMap(22, "map: Player03")
mapPlayer04     = ptAttribDynamicMap(23, "map: Player04")
mapPlayer05     = ptAttribDynamicMap(24, "map: Player05")
mapPlayer06     = ptAttribDynamicMap(25, "map: Player06")

GUIDiag6a       = ptAttribGUIDialog(26, "GUI Dialog 6a")
respLinkOutSND  = ptAttribResponder(27, "resp: Link Out SND")

#====================================
# GUI Dialog globals

#----Dialog 4a ## Text MUST Be +10 From HotSpot ##
k4aVisitID          = 100
k4aQuitID           = 101
k4aDeleteID         = 102
k4aPlayer01         = 103
k4aPlayerTxt01      = 113
k4aPlayer02         = 104
k4aPlayerTxt02      = 114
k4aPlayer03         = 105
k4aPlayerTxt03      = 115
k4aPlayer04         = 106
k4aPlayerTxt04      = 116
k4aPlayer05         = 107
k4aPlayerTxt05      = 117
k4aPlayer06         = 108
k4aPlayerTxt06      = 118

#----Dialog 4b ## Text MUST Be +10 From HotSpot ##
k4bExploreID        = 200
k4bQuitID           = 201
k4bDeleteID         = 202
k4bPlayer01         = 203
k4bPlayerTxt01      = 213
k4bPlayer02         = 204
k4bPlayerTxt02      = 214
k4bPlayer03         = 205
k4bPlayerTxt03      = 215
k4bPlayer04         = 206
k4bPlayerTxt04      = 216
k4bPlayer05         = 207
k4bPlayerTxt05      = 217
k4bPlayer06         = 208
k4bPlayerTxt06      = 218

#----Dialog 4c
k4cYesID            = 300
k4cNoID             = 301
k4cStaticID         = 302

#----Dialog 4d
k4dYesID            = 400
k4dTextID           = 401

#----Dialog 5
k5PayID             = 500
k5VisitID           = 501
k5BackID            = 502
k5LinkID            = 503

#----Dialog 6
k6QuitID            = 600
k6BackID            = 601
k6PlayID            = 602
k6PayingID          = 603
k6NameID            = 604
k6InviteID          = 605
k6MaleID            = 606
k6FemaleID          = 607
k6PlayTxtID         = 608

#====================================
# Globals
gIsExplorer         = 0
gPlayerList         = None
gSelectedSlot       = 0
gClickedWrongSlot   = 0
gBlueColor          = ptColor(0.414, 0.449, 0.617, 1.0)
gTanColor           = ptColor(0.500, 0.449, 0.375, 1.0)
gExp_HotSpot        = [k4bPlayer01,k4bPlayer02,k4bPlayer03,k4bPlayer04,k4bPlayer05,k4bPlayer06]
gVis_HotSpot        = [k4aPlayer01,k4aPlayer02,k4aPlayer03,k4aPlayer04,k4aPlayer05,k4aPlayer06]
gExp_TxtBox         = [k4bPlayerTxt01,k4bPlayerTxt02,k4bPlayerTxt03,k4bPlayerTxt04,k4bPlayerTxt05,k4bPlayerTxt06]
gVis_TxtBox         = [k4aPlayerTxt01,k4aPlayerTxt02,k4aPlayerTxt03,k4aPlayerTxt04,k4aPlayerTxt05,k4aPlayerTxt06]
gExp_HiLite         = [resp4bPlayer01,resp4bPlayer02,resp4bPlayer03,resp4bPlayer04,resp4bPlayer05,resp4bPlayer06]
gVis_HiLite         = [resp4aPlayer01,resp4aPlayer02,resp4aPlayer03,resp4aPlayer04,resp4aPlayer05,resp4aPlayer06]
gMinusExplorer      = 203
gMinusVisitor       = 103

WebLaunchCmd = None

#====================================

class xDialogStartUp(ptResponder):
    ###########################
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5340
        self.version = 3
        self.ageLink = None
        print "xDialogStartUp: init  version = %d" % self.version

    ###########################
    def OnServerInitComplete(self):
        global gIsExplorer
        global gExp_HotSpot
        global gVis_HotSpot
        global gExp_TxtBox
        global gVis_TxtBox
        global gExp_HiLite
        global gVis_HiLite
        global WebLaunchCmd

        gIsExplorer = PtIsSubscriptionActive()

        if gIsExplorer:
            self.InitPlayerList(GUIDiag4b, gExp_HotSpot, gExp_TxtBox, gExp_HiLite)
        else:
            self.InitPlayerList(GUIDiag4a, gVis_HotSpot, gVis_TxtBox, gVis_HiLite)

        ptGUIControlEditBox(GUIDiag6.dialog.getControlFromTag(k6NameID)).setStringSize(63)
        ptGUIControlEditBox(GUIDiag6.dialog.getControlFromTag(k6InviteID)).setStringSize(63)
        
        if not gIsExplorer:
            ptGUIControlTextBox(GUIDiag6.dialog.getControlFromTag(k6PayingID)).setString("VISITOR Name")
            ptGUIControlTextBox(GUIDiag6.dialog.getControlFromTag(k6PlayTxtID)).setString("Visit URU")

        WebLaunchCmd = webbrowser.open_new

    ###########################
    def BeginAgeUnLoad(self,avatar):
        if GUIDiag4a.dialog.isEnabled():
            PtHideDialog("GUIDialog04a")
        if GUIDiag4b.dialog.isEnabled():
            PtHideDialog("GUIDialog04b")
        if GUIDiag4c.dialog.isEnabled():
            PtHideDialog("GUIDialog04c")
        if GUIDiag4d.dialog.isEnabled():
            PtHideDialog("GUIDialog04d")
        if GUIDiag5.dialog.isEnabled():
            PtHideDialog("GUIDialog05")
        if GUIDiag6.dialog.isEnabled():
            PtHideDialog("GUIDialog06")
        if GUIDiag6a.dialog.isEnabled():
            PtHideDialog("GUIDialog06a")

    ###########################
    def OnNotify(self,state,id,events):
        if id==(-1): ## callback from delete yes/no dialog (hopefully) ##
            if state:
                PtConsole("App.Quit")

    ###########################
    def OnGUINotify(self,id,control,event):
        global gIsExplorer
        global gSelectedSlot
        global gPlayerList
        global gExp_HotSpot
        global gMinusExplorer
        global gMinusVisitor
        global gClickedWrongSlot

        #print "xDialogStartUp: GUI Notify id=%d, event=%d control=" % (id,event),control
        if control:
            tagID = control.getTagID()

        #################################
        ##     Visitor Player List     ##
        #################################
        if id == GUIDiag4a.id:
            if event == kAction or event == kValueChanged:
                if  tagID == k4aVisitID: ## Visit Uru ##
                    PtHideDialog("GUIDialog04a")
                    PtShowDialog("GUIDialog05")

                elif  tagID == k4aQuitID: ## Quit ##
                    PtYesNoDialog(self.key,"Are you sure you want to quit?")

                elif  tagID == k4aDeleteID: ## Delete Visitor ##
                    print gSelectedSlot
                    print type(gSelectedSlot)
                    if gSelectedSlot:
                        deleteString = U"Would you like to delete the VISITOR " + unicode(gPlayerList[gSelectedSlot-gMinusVisitor][0]) + U"?"
                        ptGUIControlTextBox(GUIDiag4c.dialog.getControlFromTag(k4cStaticID)).setStringW(deleteString)
                        self.PlayerListNotify(GUIDiag4a, gVis_HotSpot, 0)
                        PtShowDialog("GUIDialog04c")
                    ## Or Else?? ##

                elif  tagID == k4aPlayer01: ## Click Event ##
                    if gPlayerList[0]:
                        self.SelectSlot(GUIDiag4a, tagID)
                    else:
                        PtHideDialog("GUIDialog04a")
                        PtShowDialog("GUIDialog06")

                elif  tagID == k4aPlayer02 or tagID == k4aPlayer03 or k4aPlayer04 or k4aPlayer05 or k4aPlayer06: ## Shortcut Skip to Create Visitor ##
                    gClickedWrongSlot = 1
                    PtHideDialog("GUIDialog04a")
                    PtShowDialog("GUIDialog05")

            elif event == kInterestingEvent:
                self.ToggleColor(GUIDiag4a, tagID)

        #################################
        ##    Explorer Player List     ##
        #################################
        elif id == GUIDiag4b.id:
            if event == kAction or event == kValueChanged:
                if  tagID == k4bExploreID: ## Explore Uru ##
                    if gSelectedSlot:
                        PtShowDialog("GUIDialog06a")
                        print "Player selected."
                        
                        # start setting active player (we'll link out when this operation completes)
                        playerID = gPlayerList[gSelectedSlot-gMinusExplorer][1]
                        print "Setting active player."
                        PtSetActivePlayer(playerID)
                        
                    ## Or Else?? ##

                elif  tagID == k4bQuitID: ## Quit ##
                    PtYesNoDialog(self.key,"Are you sure you want to quit?")

                elif  tagID == k4bDeleteID: ## Delete Explorer ##
                    if gSelectedSlot:
                        deleteString = U"Would you like to delete the EXPLORER " + unicode(gPlayerList[gSelectedSlot-gMinusExplorer][0]) + U"?"
                        ptGUIControlTextBox(GUIDiag4c.dialog.getControlFromTag(k4cStaticID)).setStringW(deleteString)
                        self.PlayerListNotify(GUIDiag4b, gExp_HotSpot, 0)
                        PtShowDialog("GUIDialog04c")
                    ## Or Else?? ##

                elif  tagID == k4bPlayer01: ## Click Event ##
                    errorString = "As an URU subscriber, you are an EXPLORER (not a VISITOR) and are free to enjoy complete and unlimited access to URU content."
                    ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                    PtShowDialog("GUIDialog04d")
                    self.ToggleColor(GUIDiag4b, k4bPlayer01)
                    self.PlayerListNotify(GUIDiag4b, gExp_HotSpot, 0)

                elif  (tagID == k4bPlayer02 or tagID == k4bPlayer03 or tagID == k4bPlayer04 or tagID == k4bPlayer05 or tagID == k4bPlayer06) and not (tagID == gSelectedSlot):
                    if gPlayerList[tagID-gMinusExplorer]:
                        self.SelectSlot(GUIDiag4b, tagID)
                    else:
                        PtHideDialog("GUIDialog04b")
                        PtShowDialog("GUIDialog06")

            elif event == kInterestingEvent: ## RollOver Event ##
                self.ToggleColor(GUIDiag4b, tagID)

        #################################
        ##        Delete Player        ##
        #################################
        elif id == GUIDiag4c.id:
            if event == kAction or event == kValueChanged:
                if  tagID == k4cYesID: ## Confirm Delete ##
                    playerID = 0
                    if gIsExplorer:
                        playerID = gPlayerList[gSelectedSlot-gMinusExplorer][1]
                    else:
                        playerID = gPlayerList[gSelectedSlot-gMinusVisitor][1]
                    PtDeletePlayer(playerID)

                elif  tagID == k4cNoID: ## Cancel Delete ##
                    if not (gSelectedSlot == k4bPlayer03) or not (gSelectedSlot == k4aPlayer03):
                        if gIsExplorer:
                            self.ToggleColor(GUIDiag4b, k4bPlayer03)
                        else:
                            self.ToggleColor(GUIDiag4a, k4aPlayer03)
                    PtHideDialog("GUIDialog04c")
                    if gIsExplorer:
                        self.PlayerListNotify(GUIDiag4b, gExp_HotSpot, 1)
                    else:
                        self.PlayerListNotify(GUIDiag4a, gVis_HotSpot, 1)

        #################################
        ##         Not Needed          ##
        #################################
        elif id == GUIDiag4d.id:
            if event == kAction or event == kValueChanged:
                if  tagID == k4dYesID: ## Continue ##
                    if gIsExplorer:
                        self.ToggleColor(GUIDiag4b, k4bPlayer03)
                        self.PlayerListNotify(GUIDiag4b, gExp_HotSpot, 1)
                    else:
                        self.ToggleColor(GUIDiag4a, k4aPlayer03)
                        self.PlayerListNotify(GUIDiag4a, gVis_HotSpot, 1)
                    PtHideDialog("GUIDialog04d")
                    ptGUIControlButton(GUIDiag6.dialog.getControlFromTag(k6PlayID)).enable()

        #################################
        ##         Nag Screen          ##  
        #################################
        elif id == GUIDiag5.id:
            if event == kAction or event == kValueChanged:
                if  tagID == k5PayID: ## Quit And Register ##
                    WebLaunchCmd("https://account.gametap.com/storefront/myst/login/login.do")

                elif  tagID == k5VisitID: ## Continue As Visitor ##
                    if gClickedWrongSlot:
                        gClickedWrongSlot = 0
                        PtHideDialog("GUIDialog05")
                        PtShowDialog("GUIDialog04a")
                    elif gPlayerList[0]:
                        PtShowDialog("GUIDialog06a")
                        print "Player selected."
                                                
                        # start setting active player (we'll link out when this operation completes)
                        playerID = gPlayerList[0][1]
                        print "Setting active player."
                        PtSetActivePlayer(playerID)
                    else:
                        PtHideDialog("GUIDialog05")
                        PtShowDialog("GUIDialog06")

                elif  tagID == k5BackID: ## Back To Player Select ##
                    PtHideDialog("GUIDialog05")
                    PtShowDialog("GUIDialog04a")

                elif  tagID == k5LinkID: ## Link ##
                    WebLaunchCmd("https://account.gametap.com/storefront/myst/login/login.do")

        #################################
        ##        Create Player        ##
        #################################
        elif id == GUIDiag6.id:
            if event == kAction or event == kValueChanged:
                if  tagID == k6QuitID: ## Quit ##
                    PtYesNoDialog(self.key,"Are you sure you want to quit?")

                elif  tagID == k6BackID: ## Back To Player Select ##
                    PtHideDialog("GUIDialog06")
                    if gIsExplorer:
                        PtShowDialog("GUIDialog04b")
                    else:
                        PtShowDialog("GUIDialog04a")

                elif  tagID == k6PlayID: ## Play ##
                    playerName = ptGUIControlEditBox(GUIDiag6.dialog.getControlFromTag(k6NameID)).getString()  #                 <---
                    playerNameW = ptGUIControlEditBox(GUIDiag6.dialog.getControlFromTag(k6NameID)).getStringW()  #                 <---
                    inviteCode = ptGUIControlEditBox(GUIDiag6.dialog.getControlFromTag(k6InviteID)).getString()

                    # since the code is in hex we know letters like i, l, and o can't be in the code so try
                    # converting any to 1 or 0 in case the user thought they were the wrong thing
                    inviteCode = inviteCode.lower().replace("i", "1").replace("l", "1").replace("o", "0")
                    
                    try:
                        playerName == playerNameW
                    except:
                        errorString = "Error, invalid Name. Please enter another."
                        ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                        PtShowDialog("GUIDialog04d")
                        if gIsExplorer:
                            self.ToggleColor(GUIDiag4b, k4bPlayer03)
                        else:
                            self.ToggleColor(GUIDiag4a, k4aPlayer03)
                        return

                    playerGender = ""
                    if ptGUIControlCheckBox(GUIDiag6.dialog.getControlFromTag(k6MaleID)).isChecked():
                        playerGender = "male"
                    if ptGUIControlCheckBox(GUIDiag6.dialog.getControlFromTag(k6FemaleID)).isChecked():
                        playerGender = "female"

                    if playerName == U"" or playerName == "":
                        errorString = "Error, you must enter a Name."
                        ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                        PtShowDialog("GUIDialog04d")
                        if gIsExplorer:
                            self.ToggleColor(GUIDiag4b, k4bPlayer03)
                        else:
                            self.ToggleColor(GUIDiag4a, k4aPlayer03)
                    elif playerGender == "":
                        errorString = "Error, you must select a gender."
                        ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                        PtShowDialog("GUIDialog04d")
                        if gIsExplorer:
                            self.ToggleColor(GUIDiag4b, k4bPlayer03)
                        else:
                            self.ToggleColor(GUIDiag4a, k4aPlayer03)
                    else:
                        fixedPlayerName = playerName.strip()
                        (fixedPlayerName, whitespacefixedcount) = re.subn("\s{2,}|[\t\n\r\f\v]", " ", fixedPlayerName)
                        
                        (fixedPlayerName, RogueCount,) = re.subn('[\x00-\x1f]', '', fixedPlayerName)
                        if RogueCount > 0 or whitespacefixedcount > 0:
                            if RogueCount > 0:
                                errorString = "Warning, you entered invalid characters in your player name.  The invalid characters have been removed, please make sure your player name is still what you want."
                            else:
                                errorString = "Warning, your player name has some incorrect formatting.  The formatting has been corrected, please make sure your player name is still what you want."
                            ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                            PtShowDialog("GUIDialog04d")
                            if gIsExplorer:
                                self.ToggleColor(GUIDiag4b, k4bPlayer03)
                            else:
                                self.ToggleColor(GUIDiag4a, k4aPlayer03)

                            ptGUIControlEditBox(GUIDiag6.dialog.getControlFromTag(k6NameID)).setString(fixedPlayerName)
                        else:
                            print "Creating Player"
                            PtShowDialog("GUIDialog06a")
                            ptGUIControlButton(GUIDiag6.dialog.getControlFromTag(k6PlayID)).disable()
                            PtCreatePlayer(fixedPlayerName, playerGender, inviteCode)  #                                                  <---

                elif  tagID == k6MaleID: ## Gender Male ##
                    if ptGUIControlCheckBox(GUIDiag6.dialog.getControlFromTag(k6FemaleID)).isChecked():
                        ptGUIControlCheckBox(GUIDiag6.dialog.getControlFromTag(k6FemaleID)).setChecked(0)

                elif  tagID == k6FemaleID: ## Gender Female ##
                    if ptGUIControlCheckBox(GUIDiag6.dialog.getControlFromTag(k6MaleID)).isChecked():
                        ptGUIControlCheckBox(GUIDiag6.dialog.getControlFromTag(k6MaleID)).setChecked(0)

    ###########################
    def OnAccountUpdate(self, opType, result, playerInt):
        global gIsExplorer
        global gExp_HotSpot
        global gVis_HotSpot
        global gExp_TxtBox
        global gVis_TxtBox
        global gExp_HiLite
        global gVis_HiLite
        global gPlayerList

        if result != 0:
            print "OnAccountUpdate type %u failed: %u" % (opType, result)
            PtHideDialog("GUIDialog06a")
            if gIsExplorer:
                self.ToggleColor(GUIDiag4b, k4bPlayer03)
            else:
                self.ToggleColor(GUIDiag4a, k4aPlayer03)

            if result == 12:
                errorString = "Error, this player name already exists."
                ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                PtShowDialog("GUIDialog04d")
            elif result == 15:
                errorString = "Invalid friend code. Please double check your email to make sure you typed the code in correctly."
                ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                PtShowDialog("GUIDialog04d")
            elif result == 28:
                errorString = "Invalid name. The name chosen is either reserved, illegal, or shorter than three characters."
                ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                PtShowDialog("GUIDialog04d")
            elif result == 44:
                errorString = "Invalid friend code. Unable to find an associated player."
                ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                PtShowDialog("GUIDialog04d")
            elif result == 45:
                errorString = "Friend has too many hoods."
                ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                PtShowDialog("GUIDialog04d")
            else:
                errorString = "There has been a Network error. Please try again. If problem persists, please contact support."
                ptGUIControlTextBox(GUIDiag4d.dialog.getControlFromTag(k4dTextID)).setString(errorString)
                PtShowDialog("GUIDialog04d")
            return
        
        elif opType == PtAccountUpdateType.kActivePlayer:
            print "Active player set."

            pythonBox = PtFindSceneobject("OptionsDialog", "GUI")
            pmlist = pythonBox.getPythonMods()
            for pm in pmlist:
                notify = ptNotify(self.key)
                notify.clearReceivers()
                notify.addReceiver(pm)
                notify.setActivate(1.0)
                notify.send()

            # setup the link to player's Relto
            self.ageLink = ptAgeLinkStruct()
            ageInfo = ptAgeInfoStruct()

            vault = ptVault()
            entry = vault.findChronicleEntry("InitialAvCustomizationsDone")
            if type(entry) != type(None):
                ageInfo.setAgeFilename("Personal")
            else:
                ageInfo.setAgeFilename("AvatarCustomization")
            self.ageLink.setAgeInfo(ageInfo)
            self.ageLink.setLinkingRules(PtLinkingRules.kOwnedBook)

            print "Linking to %s" % (self.ageLink.getAgeInfo().getAgeFilename())
            respLinkOutSND.run(self.key)
            ptNetLinkingMgr().linkToAge(self.ageLink)
            self.ageLink = None

        elif opType == PtAccountUpdateType.kCreatePlayer:
            print "Player created."	

            # setup the link to ACA
            self.ageLink = ptAgeLinkStruct()
            ageInfo = ptAgeInfoStruct()
            ageInfo.setAgeFilename("AvatarCustomization")
            self.ageLink.setAgeInfo(ageInfo)
            self.ageLink.setLinkingRules(PtLinkingRules.kBasicLink)
            

            # start setting active player (we'll link out once this operation completes)
            print "Setting active player."
            PtSetActivePlayer(playerInt)

        elif opType == PtAccountUpdateType.kDeletePlayer:
            if gIsExplorer:
                self.SelectSlot(GUIDiag4b, 0)
                self.InitPlayerList(GUIDiag4b, gExp_HotSpot, gExp_TxtBox, gExp_HiLite)
                self.ToggleColor(GUIDiag4b, k4bPlayer03)
            else:
                self.SelectSlot(GUIDiag4a, 0)
                self.InitPlayerList(GUIDiag4a, gVis_HotSpot, gVis_TxtBox, gVis_HiLite)
                self.ToggleColor(GUIDiag4a, k4aPlayer03)

            PtHideDialog("GUIDialog04c")

        elif opType == PtAccountUpdateType.kUpgradePlayer:
            print "xDialogStartUp: upgraded player %d" % playerInt
            if gIsExplorer:
                self.SelectSlot(GUIDiag4b, 0)
                self.InitPlayerList(GUIDiag4b, gExp_HotSpot, gExp_TxtBox, gExp_HiLite)
                self.ToggleColor(GUIDiag4b, k4bPlayer03)
            else:
                print "xDialogStartUp: we shouldn't have gotten here...apparently we upgraded a player without paying"
                self.SelectSlot(GUIDiag4a, 0)
                self.InitPlayerList(GUIDiag4a, gVis_HotSpot, gVis_TxtBox, gVis_HiLite)
                self.ToggleColor(GUIDiag4a, k4aPlayer03)

        else:
            print "AccountUpdate - Unknown: optype = %d, result = %d, playerInt = %d" % (opType, result, playerInt)

    ###########################
    def ToggleColor(self,dlgObj,tagID):
        global gTanColor
        global gBlueColor
        global gExp_HotSpot
        global gVis_HotSpot
        global gExp_HiLite
        global gVis_HiLite

        currentColor = ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID+10)).getForeColor()

        if gIsExplorer:
            idx = gExp_HotSpot.index(tagID)
            respToRun = gExp_HiLite[idx]
        else:
            idx = gVis_HotSpot.index(tagID)
            respToRun = gVis_HiLite[idx]

        if currentColor == gBlueColor:
            #print "toggle tagID(%d) off" % (tagID)
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID+10)).setForeColor(gTanColor)
            respToRun.run(self.key,state="out")
        else:
            #print "toggle tagID(%d) on" % (tagID)
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID+10)).setForeColor(gBlueColor)
            respToRun.run(self.key,state="in")

    ###########################
    def SelectSlot(self,dlgObj,tagID):
        global gSelectedSlot

        if tagID and tagID != gSelectedSlot: ## If there's a currently selected slot, return it to normal ##
            print "xDialogStartUp.SelectSlot: tagID = %d   gSelectedSlot = %d" % (tagID, gSelectedSlot)
            if gSelectedSlot:
                self.ToggleColor(dlgObj, gSelectedSlot)
                ptGUIControlButton(dlgObj.dialog.getControlFromTag(gSelectedSlot)).setNotifyOnInteresting(1)
                print "xDialogStartUp.SelectSlot: Setting old slot to Interesting"

            gSelectedSlot = tagID
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(gSelectedSlot)).setNotifyOnInteresting(0)
            print "xDialogStartUp.SelectSlot: Setting gSelectedSlot to new val and removing Interesting"
        elif tagID == 0:
            print "xDialogStartUp.SelectSlot: Setting gSelectedSlot to %d" % (tagID)
            gSelectedSlot = tagID

    ###########################
    def InitPlayerList(self,dlgObj,listHotSpot,listTxtBox,listHiLite):
        global gTanColor
        global gPlayerList

        gPlayerList = PtGetAccountPlayerList()
        ExplorerList = gPlayerList[1:]
        ExplorerList.sort(id_comp)
        if len(ExplorerList) > 1:
            del gPlayerList[1:]
            for Explorer in ExplorerList:
                gPlayerList.append(Explorer)

        print "xDialogStartUp.InitPlayerList Enter: gPlayerList = %s" % (str(gPlayerList))

        for tagID in listTxtBox: ## Setup The Slot Colors And Default Titles ##
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID)).setString("CREATE EXPLORER")
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(tagID)).setForeColor(gTanColor)
        for respToRun in listHiLite:
            respToRun.run(self.key,state="out")

        self.PlayerListNotify(dlgObj, listHotSpot, 1)
        
        basePath = PtGetUserPath() + U"\\Avatars\\"
        if gPlayerList[0]: ## Setup The Visitor Slot ##
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(listTxtBox[0])).setStringW(unicode(gPlayerList[0][0]))
            try:
                filename = basePath + unicode(gPlayerList[0][1]) + U".jpg"
                print U"xDialogStartUp: Trying to load \"" + filename + "\""
                theImage = PtLoadJPEGFromDisk(filename, 0, 0)
                mapPlayer01.textmap.drawImage(0, 0, theImage, 1)
                mapPlayer01.textmap.flush()
            except:
                print "xDialogStartUp: Load failed. This avatar probably doesn't have a snapshot"

        else:
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(listTxtBox[0])).setString("CREATE VISITOR")

        TextMaps = [mapPlayer01,mapPlayer02,mapPlayer03,mapPlayer04,mapPlayer05,mapPlayer06]
        for Tex in TextMaps:
            Tex.textmap.clearToColor(ptColor(0, 0, 0, 0))
            Tex.textmap.flush()

        for idx in range(1, len(gPlayerList[1:]) + 1): ## Setup The Explorer Slots ##
            player = gPlayerList[idx]
            ptGUIControlTextBox(dlgObj.dialog.getControlFromTag(listTxtBox[idx])).setStringW(unicode(player[0]))
            try:
                filename = basePath + unicode(player[1]) + U".jpg"
                print "xDialogStartUp: Trying to load \"" + filename + "\""
                theImage = PtLoadJPEGFromDisk(filename, 0, 0)
                TextMaps[idx].textmap.drawImage(0, 0, theImage, 0)
                TextMaps[idx].textmap.flush()
            except:
                print "xDialogStartUp: Load failed. This avatar probably doesn't have a snapshot"

        if gIsExplorer:
            try:
                gPlayerList[1]
            except IndexError:
                self.SelectSlot(dlgObj, 0)
            else:
                self.ToggleColor(dlgObj, listHotSpot[1])
                self.SelectSlot(dlgObj, listHotSpot[1])
        else:
            if gPlayerList[0]:
                self.ToggleColor(dlgObj, listHotSpot[0])
                self.SelectSlot(dlgObj, listHotSpot[0])
            else:
                self.SelectSlot(dlgObj, 0)

        if gIsExplorer and len(gPlayerList) > 0 and len(gPlayerList) < 6 and gPlayerList[0]:
            print "xDialogStartUp: upgrading player %d to explorer status" % gPlayerList[0][1]
            PtUpgradeVisitorToExplorer(gPlayerList[0][1])

        while len(gPlayerList) < 6:   ## Now That Slots Are Initialized, Fill Out The List ##
            gPlayerList.append(None)  ##    So We Don't Have Out Of Bounds Errors Later    ##
        print "xDialogStartUp.InitPlayerList Exit: gPlayerList = %s" % (str(gPlayerList))

    ###########################
    def PlayerListNotify(self,dlgObj,listHotSpot,toggle):
        global gSelectedSlot

        for tagID in listHotSpot:
            print "xDialogStartUp.PlayerListNotify: setting tagID (%d) to Interesting = %d" % (tagID, toggle)
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(tagID)).setNotifyOnInteresting(toggle)

        if toggle and gSelectedSlot:
            print "xDialogStartUp.PlayerListNotify: setting gSelectedSlot (%d) to not Interesting" % (gSelectedSlot)
            ptGUIControlButton(dlgObj.dialog.getControlFromTag(gSelectedSlot)).setNotifyOnInteresting(0)

###########################
def id_comp(elem1, elem2):
    return cmp(elem1[1], elem2[1])
