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
from hanukkah import *
import time

def SetVaultSDLBool(sdlVar, setVar):
    agevault = ptAgeVault()
    if agevault:
        ageSDL = agevault.getAgeSDL()
        if ageSDL:
            if ageSDL.findVar(sdlVar).getBool() != setVar:
                ageSDL.findVar(sdlVar).setBool(setVar)
                agevault.updateAgeSDL(ageSDL)
                
def SetVaultSDLInt(sdlVar, setVar):
    agevault = ptAgeVault()
    if agevault:
        ageSDL = agevault.getAgeSDL()
        if ageSDL:
            if ageSDL.findVar(sdlVar).getInt() != setVar:
                ageSDL.findVar(sdlVar).setInt(setVar)
                agevault.updateAgeSDL(ageSDL)

class GoMePubNew(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 40006
        self.version = 1

    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        self.UpdateRecentVisitors()
        
        if PtGetPlayerList():
            return
        st = time.gmtime(PtGetDniTime())
        agevault = ptAgeVault()
        day1, day2, day3, day4, day5, day6, day7, day8, end = h[st.tm_year]
        agevault = ptAgeVault()
        if agevault:
            ageSDL = agevault.getAgeSDL()
            if ageSDL:
                if st.tm_mon == 1 and st.tm_mday <= 3:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 1)
                elif st.tm_mon == 3 and st.tm_mday >= 14 and st.tm_mday <=20:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 2)
                elif st.tm_mon == 4 and st.tm_mday >= 19 and st.tm_mday <=23:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 2)
                elif st.tm_mon == 6:
                    SetVaultSDLBool("gmpnPrideDecoVis", True)
                    SetVaultSDLInt("gmpnHolidayVis", 0)
                elif st.tm_mon == 10 and st.tm_mday > 15:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 4)
                elif st.tm_mon == 11:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 5)
                elif st.tm_mon == 12 and st.tm_mday >= 13 and st.tm_mday <= 28:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 6)
                elif st.tm_mon == 12 and st.tm_mday >= 29:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 1)
                else:
                    SetVaultSDLBool("gmpnPrideDecoVis", False)
                    SetVaultSDLInt("gmpnHolidayVis", 0)
                
                if ageSDL.findVar("gmpnMenorahFlame01").getBool() and st.tm_mon == 1:
                    day1, day2, day3, day4, day5, day6, day7, day8, end = h[st.tm_year-1]

                if st.tm_mon == day1.month and st.tm_mday == day1.day and st.tm_hour >= day1.hour and st.tm_min >= day1.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", False)
                    SetVaultSDLBool("gmpnMenorahFlame03", False)
                    SetVaultSDLBool("gmpnMenorahFlame04", False)
                    SetVaultSDLBool("gmpnMenorahFlame05", False)
                    SetVaultSDLBool("gmpnMenorahFlame06", False)
                    SetVaultSDLBool("gmpnMenorahFlame07", False)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)
                elif st.tm_mon == day2.month and st.tm_mday == day2.day and st.tm_hour >= day2.hour and st.tm_min >= day2.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", False)
                    SetVaultSDLBool("gmpnMenorahFlame04", False)
                    SetVaultSDLBool("gmpnMenorahFlame05", False)
                    SetVaultSDLBool("gmpnMenorahFlame06", False)
                    SetVaultSDLBool("gmpnMenorahFlame07", False)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)
                elif st.tm_mon == day3.month and st.tm_mday == day3.day and st.tm_hour >= day3.hour and st.tm_min >= day3.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", True)
                    SetVaultSDLBool("gmpnMenorahFlame04", False)
                    SetVaultSDLBool("gmpnMenorahFlame05", False)
                    SetVaultSDLBool("gmpnMenorahFlame06", False)
                    SetVaultSDLBool("gmpnMenorahFlame07", False)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)
                elif st.tm_mon == day4.month and st.tm_mday == day4.day and st.tm_hour >= day4.hour and st.tm_min >= day4.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", True)
                    SetVaultSDLBool("gmpnMenorahFlame04", True)
                    SetVaultSDLBool("gmpnMenorahFlame05", False)
                    SetVaultSDLBool("gmpnMenorahFlame06", False)
                    SetVaultSDLBool("gmpnMenorahFlame07", False)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)
                elif st.tm_mon == day5.month and st.tm_mday == day5.day and st.tm_hour >= day5.hour and st.tm_min >= day5.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", True)
                    SetVaultSDLBool("gmpnMenorahFlame04", True)
                    SetVaultSDLBool("gmpnMenorahFlame05", True)
                    SetVaultSDLBool("gmpnMenorahFlame06", False)
                    SetVaultSDLBool("gmpnMenorahFlame07", False)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)
                elif st.tm_mon == day6.month and st.tm_mday == day6.day and st.tm_hour >= day6.hour and st.tm_min >= day6.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", True)
                    SetVaultSDLBool("gmpnMenorahFlame04", True)
                    SetVaultSDLBool("gmpnMenorahFlame05", True)
                    SetVaultSDLBool("gmpnMenorahFlame06", True)
                    SetVaultSDLBool("gmpnMenorahFlame07", False)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)
                elif st.tm_mon == day7.month and st.tm_mday == day7.day and st.tm_hour >= day7.hour and st.tm_min >= day7.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", True)
                    SetVaultSDLBool("gmpnMenorahFlame04", True)
                    SetVaultSDLBool("gmpnMenorahFlame05", True)
                    SetVaultSDLBool("gmpnMenorahFlame06", True)
                    SetVaultSDLBool("gmpnMenorahFlame07", True)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)
                elif st.tm_mon == day8.month and st.tm_mday == day8.day and st.tm_hour >= day8.hour and st.tm_min >= day8.minute:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", True)
                    SetVaultSDLBool("gmpnMenorahFlame04", True)
                    SetVaultSDLBool("gmpnMenorahFlame05", True)
                    SetVaultSDLBool("gmpnMenorahFlame06", True)
                    SetVaultSDLBool("gmpnMenorahFlame07", True)
                    SetVaultSDLBool("gmpnMenorahFlame08", True)
                elif st.tm_mon == end.month and st.tm_mday == end.day:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", True)
                    SetVaultSDLBool("gmpnMenorahFlame01", True)
                    SetVaultSDLBool("gmpnMenorahFlame02", True)
                    SetVaultSDLBool("gmpnMenorahFlame03", True)
                    SetVaultSDLBool("gmpnMenorahFlame04", True)
                    SetVaultSDLBool("gmpnMenorahFlame05", True)
                    SetVaultSDLBool("gmpnMenorahFlame06", True)
                    SetVaultSDLBool("gmpnMenorahFlame07", True)
                    SetVaultSDLBool("gmpnMenorahFlame08", True)
                else:
                    SetVaultSDLBool("gmpnMenorahFlameShammash", False)
                    SetVaultSDLBool("gmpnMenorahFlame01", False)
                    SetVaultSDLBool("gmpnMenorahFlame02", False)
                    SetVaultSDLBool("gmpnMenorahFlame03", False)
                    SetVaultSDLBool("gmpnMenorahFlame04", False)
                    SetVaultSDLBool("gmpnMenorahFlame05", False)
                    SetVaultSDLBool("gmpnMenorahFlame06", False)
                    SetVaultSDLBool("gmpnMenorahFlame07", False)
                    SetVaultSDLBool("gmpnMenorahFlame08", False)

    def OnNotify(self, state, id, events):
        pass


    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        pass


    def RetryRecentVisitors(self):
        PtAtTimeCallback(self.key, 1, 1)



    def OnTimer(self, id):
        if (id == 10069):
            self.UpdateRecentVisitors()



    def UpdateRecentVisitors(self):
        try:
            AmCCR = ptCCRMgr().getLevel()
        except:
            AmCCR = 0
        if (not AmCCR):
            deviceNode = None
            deviceInbox = None
            playerlist = None
            avault = ptAgeVault()
            adevicesfolder = avault.getAgeDevicesFolder()
            adevices = adevicesfolder.getChildNodeRefList()
            for device in adevices:
                device = device.getChild()
                devicetn = device.upcastToTextNoteNode()
                if (devicetn and (devicetn.getTitle() == 'Visitor Imager')):
                    deviceNode = devicetn
                    break

            if (not deviceNode):
                self.RetryRecentVisitors()
            else:
                inboxes = deviceNode.getChildNodeRefList()
                for inbox in inboxes:
                    inbox = inbox.getChild()
                    inboxfolder = inbox.upcastToFolderNode()
                    if inboxfolder:
                        deviceInbox = inboxfolder
                        break

                if (not deviceInbox):
                    self.RetryRecentVisitors()
                else:
                    items = deviceInbox.getChildNodeRefList()
                    for item in items:
                        item = item.getChild()
                        itemtn = item.upcastToTextNoteNode()
                        if itemtn:
                            if (itemtn.getTitle() == 'Visitors, Visiteurs, Besucher'):
                                playerlist = itemtn
                                break
                            elif (itemtn.getTitle() == 'Most Recent Visitors'):
                                itemtn.setTitle('Visitors, Visiteurs, Besucher')
                                playerlist = itemtn
                                break

                    if playerlist:
                        currenttime = time.gmtime(PtGetDniTime())
                        currenttimestr = time.strftime('%m/%d/%Y %I:%M %p', currenttime)
                        playername = PtGetLocalPlayer().getPlayerName()
                        thetext = playerlist.getText()
                        if (PtDetermineKILevel() == kNormalKI):
                            while ((thetext.count('\n') + 1) > 15):
                                thetext = thetext[:thetext.rfind('\n')]

                            thetext = ((self.FormatPlayerInfo(currenttimestr, playername) + '\n') + thetext)
                        playerlist.setText(thetext)
                        playerlist.save()
                    else:
                        currenttime = time.gmtime(PtGetDniTime())
                        currenttimestr = time.strftime('%m/%d/%Y %I:%M %p', currenttime)
                        playername = PtGetLocalPlayer().getPlayerName()
                        if (PtDetermineKILevel() == kNormalKI):
                            thetext = (self.FormatPlayerInfo(currenttimestr, playername) + '\n')
                        else:
                            thetext = ''
                        playerlist = ptVaultTextNoteNode()
                        playerlist.setTitle('Visitors, Visiteurs, Besucher')
                        playerlist.setText(thetext)
                        deviceInbox.addNode(playerlist)



    def FormatPlayerInfo(self, playerdate, playername):
        return ((playerdate + (' ' * (30 - len(playerdate)))) + playername)



    def RemoveHiddenText(self, msg):
        if (type(msg) == type('')):
            newmsg = msg
            hiddenlist = []
            hidTxtStart = 0
            while (hidTxtStart >= 0):
                hidTxtStart = newmsg.find('<hiddentext>')
                if (hidTxtStart >= 0):
                    hidTxtEnd = newmsg.find('</hiddentext>')
                    if (hidTxtEnd >= 0):
                        hiddenlist.append(newmsg[(hidTxtStart + len('<hiddentext>')):hidTxtEnd])
                        newmsg = (newmsg[:hidTxtStart] + newmsg[(hidTxtEnd + len('</hiddentext>')):])

            return (newmsg,
             hiddenlist)
        return (msg,
         [])

    def OnBackdoorMsg(self, target, param):
        if ((target == 'settext') or (target == 'addtext')):
            param = param.replace('\\n', '\n')
            deviceNode = None
            deviceInbox = None
            playerlist = None
            avault = ptAgeVault()
            adevicesfolder = avault.getAgeDevicesFolder()
            adevices = adevicesfolder.getChildNodeRefList()
            for device in adevices:
                device = device.getChild()
                devicetn = device.upcastToTextNoteNode()
                if (devicetn and (devicetn.getTitle() == 'Visitor Imager')):
                    deviceNode = devicetn
                    break

            if deviceNode:
                inboxes = deviceNode.getChildNodeRefList()
                for inbox in inboxes:
                    inbox = inbox.getChild()
                    inboxfolder = inbox.upcastToFolderNode()
                    if inboxfolder:
                        deviceInbox = inboxfolder
                        break

                if deviceInbox:
                    items = deviceInbox.getChildNodeRefList()
                    for item in items:
                        item = item.getChild()
                        itemtn = item.upcastToTextNoteNode()
                        if itemtn:
                            if (itemtn.getTitle() == 'Visitors, Visiteurs, Besucher'):
                                playerlist = itemtn
                                break
                            elif (itemtn.getTitle() == 'Most Recent Visitors'):
                                itemtn.setTitle('Visitors, Visiteurs, Besucher')
                                playerlist = itemtn
                                break

                    if playerlist:
                        if (target == 'addtext'):
                            param = (param + playerlist.getText())
                        hid = self.RemoveHiddenText(param)
                        if (len(hid[1]) > 0):
                            param = (param + str(hid[1]))
                        playerlist.setText(param)
                        playerlist.save()
                    else:
                        playerlist = ptVaultTextNoteNode()
                        playerlist.setTitle('Visitors, Visiteurs, Besucher')
                        playerlist.setText(param)
                        deviceInbox.addNode(playerlist)
