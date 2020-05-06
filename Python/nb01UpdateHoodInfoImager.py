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
Module: nb01UpdateImagerPlayerList.py
Age: Neighborhood
Date: October 2007
Script to manage updating the player list on the hood info imager
"""

from Plasma import *
from PlasmaTypes import *
import time

HoodInfoImagerScript = ptAttribActivator(1, "Hood Info imager script")

class nb01UpdateHoodInfoImager(ptResponder):
    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5262

        self.version = 2
        PtDebugPrint("__init__nb01UpdateHoodInfoImager v.", self.version)

    def IGetDeviceInbox(self):
        deviceNode = None
        deviceInbox = None

        # find the device
        avault = ptAgeVault()
        adevicesfolder = avault.getAgeDevicesFolder()
        adevices = adevicesfolder.getChildNodeRefList()
        for device in adevices:
            device = device.getChild()
            devicetn = device.upcastToTextNoteNode()
            if devicetn and devicetn.getTitle() == "D'ni  Imager Right" or devicetn.getTitle() == "D'ni  Imager Right<p>":
                deviceNode = devicetn
                break

        # if we have the device then find the inbox
        if deviceNode:
            inboxes = deviceNode.getChildNodeRefList()
            for inbox in inboxes:
                inbox = inbox.getChild()
                inboxfolder = inbox.upcastToFolderNode()
                if inboxfolder:
                    deviceInbox = inboxfolder
                    break
        return deviceInbox

    def IUpdatePlayerList(self, playername):
        deviceInbox = self.IGetDeviceInbox()
        playerlist = None
        
        # if we have the inbox then look for the heek score note
        if deviceInbox:
            items = deviceInbox.getChildNodeRefList()
            for item in items:
                item = item.getChild()
                itemtn = item.upcastToTextNoteNode()
                if itemtn:
                    if itemtn.getTitle() == "Visitors, Visiteurs, Besucher":
                        playerlist = itemtn
                        break
                    elif itemtn.getTitle() == "Most Recent Visitors":
                        itemtn.setTitle("Visitors, Visiteurs, Besucher")
                        playerlist = itemtn
                        break

            # if we have the text note then update it, otherwise create it
            if playerlist:
                currenttime = time.gmtime(PtGetDniTime())
                currenttimestr = time.strftime("%m/%d/%Y %I:%M %p", currenttime)
                thetext = playerlist.getText()
                if (thetext.count("\n") + 1) > 15:
                    thetext = thetext[:thetext.rfind("\n")]
                thetext = currenttimestr + (" " * (30 - len(currenttimestr))) + playername + "\n" + thetext
                
                playerlist.setText(thetext)
                playerlist.forceSave()
            else:
                currenttime = time.gmtime(PtGetDniTime())
                currenttimestr = time.strftime("%m/%d/%Y %I:%M %p", currenttime)
                thetext = currenttimestr + (" " * (30 - len(currenttimestr))) + playername
                
                playerlist = ptVaultTextNoteNode(0)
                playerlist.setTitle("Visitors, Visiteurs, Besucher")
                playerlist.setText(thetext)
                deviceInbox.addNode(playerlist)

        if playerlist and playerlist.getID() > 0:
            sname = "Update=%d" % (playerlist.getID())
            self.ISendNotify(HoodInfoImagerScript.value, sname, 1.0)

    def IUpdatePublicHoodDate(self):
        # The MOULa server sorts the public hood list by date last set public.
        # We want to have it sorted by date last linked to, so that hoods that
        # are in active use don't drop off the list. Since we can't modify the
        # server we fake this by updating the 'set public' timestamp every time
        # someone links in.
        # On all known servers, setting from true to true is sufficient to
        # update the timestamp even though it's not an actual change.
        infoNode = ptAgeVault().getAgeInfo()
        if infoNode.isPublic():
            ptVault().setAgePublic(infoNode, True)

    def IFixupScoreLine(self, scorerow):
        retVal = (None, None)
        
        scorestring = scorerow.strip()
        splitindex = scorestring.rfind("\t")

        if splitindex != -1:
            name = scorestring[:splitindex].strip()
            score = scorestring[splitindex + 1:].strip()
            
            if len(name) > 0 and score.isdigit():
                retVal = (name, score)

        return retVal

    def IFormatScoreLine(self, name, score):
        scorestr = str(score)
        return name + "\t" + scorestr + "\n"

    def IUpdateHoodImager(self):
        ptGameScore.findAgeScores("PelletDrop", self.key)

    def OnGameScoreMsg(self, msg):
        if isinstance(msg, ptGameScoreListMsg):
            try:
                score = msg.getScores()[0]
                if msg.getName() == "PelletDrop":
                    self.IUpdateHoodPelletScore(score)
            except Exception as detail:
                PtDebugPrint("nb01UpdateHoodInfoImager.OnGameScoreMsg(): " + detail)

    def IUpdateHoodPelletScore(self, score):
        hoodpoints = score.getPoints()
        hoodpelletscore = 0
        deviceInbox = self.IGetDeviceInbox()
        if deviceInbox is not None:
            items = deviceInbox.getChildNodeRefList()
            for item in items:
                item = item.getChild()
                itemtn = item.upcastToTextNoteNode()
                if itemtn:
                    if itemtn.getTitle() == "Neighborhood Pellet Score":
                        hoodpelletscore = itemtn
                        break
            if hoodpelletscore:
                newText = str(hoodpoints)
                hoodpelletscore.setText(newText)
                hoodpelletscore.forceSave()
            else:
                hoodpelletscore = ptVaultTextNoteNode(0)
                hoodpelletscore.setTitle("Neighborhood Pellet Score")
                newText = str(hoodpoints)
                hoodpelletscore.setText(newText)
                deviceInbox.addNode(hoodpelletscore)

            if hoodpelletscore and hoodpelletscore.getID() > 0:
                sname = "Update=%d" % (hoodpelletscore.getID())
                self.ISendNotify(HoodInfoImagerScript.value, sname, 1.0)

    def IUpdatePelletScores(self, newplayername, pelletscore):
        deviceInbox = self.IGetDeviceInbox()
        if deviceInbox is not None:
            pelletscores = None
            items = deviceInbox.getChildNodeRefList()
            for item in items:
                item = item.getChild()
                itemtn = item.upcastToTextNoteNode()
                if itemtn:
                    if itemtn.getTitle() == "Pellet Scores":
                        pelletscores = itemtn
                        break
            if pelletscores:
                current = pelletscores.getText()
                curscorelist = current.split("\n")
                formattedList = []
                for scorerow in curscorelist:
                    if scorerow != "":
                        (playername, score) = self.IFixupScoreLine(scorerow)
                        if playername and score:
                            formattedList.append([int(score), playername])

                formattedList.append([pelletscore, newplayername])
                formattedList.sort()
                formattedList.reverse()

                curindex = 0
                newText = ""
                for score in formattedList:
                    newText = newText + self.IFormatScoreLine(score[1], score[0])

                    curindex += 1
                    if curindex > 20:
                        break
                    
                pelletscores.setText(newText)
                pelletscores.forceSave()
            else:
                playername = PtGetLocalPlayer().getPlayerName()
                thetext = "%s\t%i\n" % (newplayername, pelletscore)
                pelletscores = ptVaultTextNoteNode(0)
                pelletscores.setTitle("Pellet Scores")
                pelletscores.setText(thetext)
                deviceInbox.addNode(pelletscores)

            if pelletscores and pelletscores.getID() > 0:
                sname = "Update=%d" % (pelletscores.getID())
                PtDebugPrint("Sending notify to update node: ", pelletscores.getID())
                self.ISendNotify(HoodInfoImagerScript.value, sname, 1.0)
            else:
                PtDebugPrint("Not sending notify because we don't have a valid pelletscore node")

            self.IUpdateHoodImager()

    def ISendNotify(self, receiver, name, value):
        notify = ptNotify(self.key)
        notify.clearReceivers()
        if isinstance(receiver, list):
            for key in receiver:
                notify.addReceiver(key)
        else:
            notify.addReceiver(receiver)
        notify.netPropagate(1)
        notify.netForce(1)
        notify.setActivate(1.0)
        notify.addVarNumber(name, value)
        notify.send()

    def OnServerInitComplete(self):
        PtDebugPrint("nb01UpdateHoodInfoImager.OnServerInitComplete()")

        try:
            AmCCR = ptCCRMgr().getLevel()
        except:
            AmCCR = 0

        if not AmCCR:
            self.IUpdatePublicHoodDate()

            sname = "Join=%s" % (PtGetLocalPlayer().getPlayerName())
            self.ISendNotify(self.key, sname, 1.0)

            PtDebugPrint("nb01UpdateHoodInfoImager.OnServerInitComplete: Sent player join update notify")

    def OnNotify(self,state,id,events):
        for event in events:
            if event[0] == kVariableEvent:
                if self.sceneobject.isLocallyOwned():
                    PtDebugPrint("nb01UpdateHoodInfoImager.OnNotify: I am owner so I'll update the imager")
                    if event[1][:5] == "Join=":
                        playername = event[1][5:]
                        self.IUpdatePlayerList(playername)
                        PtDebugPrint("nb01UpdateHoodInfoImager.OnNotify: Updated player list")
                    elif event[1][:6] == "Score=":
                        playername = event[1][6:]
                        score = int(event[3])
                        self.IUpdatePelletScores(playername, score)
                        PtDebugPrint("nb01UpdateHoodInfoImager.OnNotify: Updated pellet scores")
