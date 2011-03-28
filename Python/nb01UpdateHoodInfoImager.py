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
        print "__init__nb01UpdateHoodInfoImager v.", self.version

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
                playername = PtGetLocalPlayer().getPlayerName()
                thetext = currenttimestr + (" " * (30 - len(currenttimestr))) + playername
                
                playerlist = ptVaultTextNoteNode(0)
                playerlist.setTitle("Visitors, Visiteurs, Besucher")
                playerlist.setText(thetext)
                deviceInbox.addNode(playerlist)

        if playerlist and playerlist.getID() > 0:
            sname = "Update=%d" % (playerlist.getID())
            self.ISendNotify(HoodInfoImagerScript.value, sname, 1.0)

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
        hoodScoreList = ptScoreMgr().getCurrentAgeScores("PelletDrop")
        if hoodScoreList:
            hoodpoints = hoodScoreList[0].getValue()
            hoodpelletscore = 0
            deviceInbox = self.IGetDeviceInbox()
            if type(deviceInbox) != type(None):
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
        if type(deviceInbox) != type(None):
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
                thetext = str(playername) + "\t" + str(pelletscore) + "\n"
                pelletscores = ptVaultTextNoteNode(0)
                pelletscores.setTitle("Pellet Scores")
                pelletscores.setText(thetext)
                deviceInbox.addNode(pelletscores)

            if pelletscores and pelletscores.getID() > 0:
                sname = "Update=%d" % (pelletscores.getID())
                print "Sending notify to update node: ", pelletscores.getID()
                self.ISendNotify(HoodInfoImagerScript.value, sname, 1.0)
            else:
                print "Not sending notify because we don't have a valid pelletscore node"

            self.IUpdateHoodImager()

    def ISendNotify(self, receiver, name, value):
        notify = ptNotify(self.key)
        notify.clearReceivers()
        if type(receiver) == type([]):
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
        print "nb01UpdateHoodInfoImager.OnServerInitComplete()"

        try:
            AmCCR = ptCCRMgr().getLevel()
        except:
            AmCCR = 0

        if not AmCCR:
            sname = "Join=%s" % (PtGetLocalPlayer().getPlayerName())
            self.ISendNotify(self.key, sname, 1.0)

            print "nb01UpdateHoodInfoImager.OnServerInitComplete: Sent player join update notify"

    def OnNotify(self,state,id,events):
        for event in events:
            if event[0] == kVariableEvent:
                if self.sceneobject.isLocallyOwned():
                    print "nb01UpdateHoodInfoImager.OnNotify: I am owner so I'll update the imager"
                    if event[1][:5] == "Join=":
                        playername = event[1][5:]
                        self.IUpdatePlayerList(playername)
                        print "nb01UpdateHoodInfoImager.OnNotify: Updated player list"
                    elif event[1][:6] == "Score=":
                        playername = event[1][6:]
                        score = int(event[3])
                        self.IUpdatePelletScores(playername, score)
                        print "nb01UpdateHoodInfoImager.OnNotify: Updated pellet scores"
