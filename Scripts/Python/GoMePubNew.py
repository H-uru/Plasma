from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import time

class GoMePubNew(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 40006
        self.version = 1

    def OnFirstUpdate(self):
        pass


    def OnServerInitComplete(self):
        self.UpdateRecentVisitors()



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