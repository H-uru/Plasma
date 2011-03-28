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
Module: ahnyLinkBookGUIPopup
Age: global
Date: November, 2002
Author: Doug McBride
Shows the linking book GUI with the appropriate linking panel. 

# March 2003
This script now also provides the GUI for books taken off of the Personal Age Bookshelf. There are a few
differences between books on pedestals vs. books on the psnlBookshelf:
- The "Share Book" decal on the book itself is never available on a book from the psnlBookshelf
- Turned corners, indicating more pages in the book, are only available in books in the psnlBookshelf

# April 29, 2003
Major change over to using ptBook instead of LinkBooksGUI dialog
First phase - keep hi level structure, only replace the bring up of books
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
import string
import xLinkingBookDefs
from xPsnlVaultSDL import *
import time


# define the attributes that will be entered in max
actClickableBook = ptAttribActivator(1,"Actvtr: Clickable small book")
SeekBehavior = ptAttribBehavior(2, "Smart seek before GUI (optional)")
respLinkResponder = ptAttribResponder(3,"Rspndr: Link out")
TargetAge = ptAttribString(4, 'Name of Linking Panel', 'Teledahn')

actBookshelf = ptAttribActivator(5, "Bookshelf (Only used in PsnlAge)") #Leave blank unless it's a Personal Age Bookshelf
shareRegion = ptAttribActivator(6, "region in which the sharer must remain")
shareBookSeek = ptAttribBehavior(7,"smart seek & use book for share acceptance") # different, because the offerer's client links the offeree in this case

IsDRCStamped = ptAttribBoolean(10,"DRC Stamp",default=1)

respLinkSphere01 = ptAttribResponder(11,"sphere 01 resp")
respLinkSphere02 = ptAttribResponder(12,"sphere 02 resp")
respLinkSphere03 = ptAttribResponder(13,"sphere 03 resp")
respLinkSphere04 = ptAttribResponder(14,"sphere 04 resp")

# globals
LocalAvatar = None
OfferedBookMode = false
BookOfferer = None
stringAgeRequested = None
PageID_List  = []
SpawnPointName_Dict = {}
SpawnPointTitle_Dict = {}

OffereeWalking = false
ClosedBookToShare = 0

BookNumber = 0 # which book it is on the shelf. The Neighborhood book currently is 0. The Teledahn Book is currently 2.
CurrentPage = 1 # The last page the book was opened to. Read from an SDL. 

# the global ptBook object.... there can only be one book displayed at one time, so only one global needed (hopefully)
gLinkingBook = None
NoReenableBook = 0

kGrsnTeamBook = 99

class ahnyLinkBookGUIPopup(ptModifier):
    "The Linking Book GUI Popup python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5343
        version = 27
        minor = 4
        self.version = version
        PtDebugPrint("__init__ahnyLinkBookGUIPopup v%d.%d" % (version,minor))

    def OnServerInitComplete(self):
        # only in the personal age should actBookshelf be anything, so this should only happen in the personal age
        if actBookshelf:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("CurrentPage", 1,1)
            ageSDL.sendToClients("CurrentPage")

    def __del__(self):
        "destructor - get rid of any dialogs that we might have loaded"
        pass

    def OnNotify(self,state,id,events):
        global LocalAvatar
        global OfferedBookMode
        global BookOfferer
        global OffereeWalking 
        global stringAgeRequested
        global CurrentPage
        global BookNumber
        global SpawnPointName_Dict
        global SpawnPointTitle_Dict
        global gLinkingBook
        global ClosedBookToShare
        global NoReenableBook

        # if it's the share region, we only care if the offerer is LEAVING the region,
        # since he had to be inside it to trigger the clickable for the book anyway.
        # is it a clickable book on a pedestal?
        if id == actClickableBook.id and PtFindAvatar(events) == PtGetLocalAvatar():
            actClickableBook.disable()
            PtToggleAvatarClickability(false)
            LocalAvatar = PtFindAvatar(events)
            SeekBehavior.run(LocalAvatar)
            #self.IShowBookNoTreasure()
            OfferedBookMode = false
            BookOfferer = None

        # is it the seek behavior because we clicked on a book ourself?    
        elif id == SeekBehavior.id and PtFindAvatar(events) == PtGetLocalAvatar():
            print events
            for event in events:
                if event[0] == kMultiStageEvent and event[2] == kEnterStage: # Smart seek completed. Exit multistage, and show GUI.
                    SeekBehavior.gotoStage(LocalAvatar, -1) 
                    print "ahnyLinkBookGUIPopup: attempting to draw link panel gui"
                    self.IShowBookNoTreasure()
                    OfferedBookMode = false
                    BookOfferer = None
        
        else:
            for event in events:
                # is it from the OpenBook? (we only have one book to worry about)
                if event[0] == PtEventType.kBook:
                    print "ahnyLinkBookGUIPopup: BookNotify  event=%d, id=%d" % (event[1],event[2])
                    if event[1] == PtBookEventTypes.kNotifyImageLink:
                        if event[2] >= xLinkingBookDefs.kFirstLinkPanelID or event[2] == xLinkingBookDefs.kBookMarkID:
                            print "ahnyLinkBookGUIPopup:Book: hit linking panel %s" % (event[2])
                            self.HideBook(1)
                            respLinkSphere01.run(self.key,avatar=PtGetLocalAvatar(),netPropagate=0)
                            
                            #assume this is a normal pedestal book, (i.e. not the psnlBookshelf) Run the responder indicated in Max
                            
                            #respNum = self.GetCurrentSphere()

                            #print "respNum:", respNum

                            '''
                            # check if we need to reset sdl
                            sdl = xPsnlVaultSDL()
                            resetSDL = sdl["AhnySphereDelete"][0]
                            if resetSDL:
                                vault = ptVault()
                                iown = vault.getAgesIOwnFolder()
                                for i in iown.getChildNodeRefList():
                                    i = i.getChild()
                                    link = i.upcastToAgeLinkNode()
                                    info = link.getAgeInfo()
                                    if info:
                                        name = info.getAgeFilename()
                                        if name == "Ahnonay": # or name == "AhnySphere01" or name == "AhnySphere02" or name == "AhnySphere03" or name == "AhnySphere04":
                                            # its not working if we reset all of the sdl here
                                            # so we'll just create some chronicle vars and reset the sdl in the age
                                            print "attempting to reset sdl for", name
                                            asdl = info.getAgeSDL()
                                            sdr = asdl.getStateDataRecord()
                                            sdr.setFromDefaults(1)
                                            asdl.setStateDataRecord(sdr)
                                            asdl.save()
                                        elif name == "AhnySphere01" or name == "AhnySphere02" or name == "AhnySphere03" or name == "AhnySphere04":
                                            link.setVolatile(1)
##                                            entry = vault.findChronicleEntry("Reset" + name)
##                                            if entry:
##                                                entry.chronicleSetValue("1")
##                                                entry.save()
##                                            else:
##                                                vault.addChronicleEntry("Reset" + name, 0, "1")
                                            link.save()

                                sdl["AhnySphereDelete"] = (0,)
                                respNum = 1
                            
                            if (respNum == 1 or respNum == 0):
                                respLinkSphere01.run(self.key,avatar=PtGetLocalAvatar())
                            elif (respNum == 2):
                                respLinkSphere02.run(self.key,avatar=PtGetLocalAvatar())
                            elif (respNum == 3):
                                respLinkSphere03.run(self.key,avatar=PtGetLocalAvatar())
                            elif (respNum == 4):
                                respLinkSphere04.run(self.key,avatar=PtGetLocalAvatar())
                            else:
                                print"Whoa - invalid current sphere!"
                                return
                            '''
        
                    elif event[1] == PtBookEventTypes.kNotifyShow:
                        PtDebugPrint("ahnyLinkBookGUIPopup:Book: NotifyShow",level=kDebugDumpLevel)
                        # re-allow KI and BB
                        PtSendKIMessage(kEnableKIandBB,0)
                        # should we be on a different page?
                        if CurrentPage > 1:
                            PtDebugPrint("ahnyLinkBookGUIPopup: going to page %d (ptBook page %d)" % (CurrentPage,(CurrentPage-1)*2),level=kDebugDumpLevel)
                            gLinkingBook.goToPage((CurrentPage-1)*2)
                    
                    elif event[1] == PtBookEventTypes.kNotifyHide:
                        PtDebugPrint("ahnyLinkBookGUIPopup:Book: NotifyHide",level=kDebugDumpLevel)
                        if not ClosedBookToShare:
                            PtToggleAvatarClickability(true)
                            if (OfferedBookMode and BookOfferer):
                                avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                                PtNotifyOffererLinkRejected(avID)
                                PtDebugPrint("ahnyLinkBookGUIPopup: rejected link, notifying offerer as such",level=kDebugDumpLevel)
                                OfferedBookMode = false
                                BookOfferer = None
        
                        if not NoReenableBook:
                            actClickableBook.enable()
        
                        ClosedBookToShare = 0
        
                    elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                        PtDebugPrint("ahnyLinkBookGUIPopup:Book: NotifyCheckUncheck",level=kDebugDumpLevel)
                        pass

    def IShowBookNoTreasure(self):
        global gLinkingBook
        global SpawnPointName_Dict
        global SpawnPointTitle_Dict
        global kGrsnTeamBook
        
        try:
            params = xLinkingBookDefs.xAgeLinkingBooks["Ahnonay"]

            if len(params) == 6:
                sharable,width,height,stampdef,bookdef,gui = params
            elif len(params) == 5:
                sharable,width,height,stampdef,bookdef = params
                gui = "BkBook"
            else:
                return

            print bookdef
            PtSendKIMessage(kDisableKIandBB,0)
            gLinkingBook = ptBook(bookdef,self.key)
            gLinkingBook.setSize( width, height )
            gLinkingBook.setGUI(gui)
            gLinkingBook.show(1)

        except LookupError:
            print "ahnyLinkBookGUIPopup: could not find age Ahnonay's linking panel"

        '''
        showOpen = 0
        #If this is a normal pedestal book, the panel is defined in the Max GUI. If this is the personal Age Bookshelf, the panel is passed in a note from psnlBookshelf. Determine the source here.
        if len(actBookshelf.value) == 0:
            agePanel = TargetAge.value
            showOpen = 1  # start pedestal books open
        else:
            agePanel = stringAgeRequested
            showOpen = 0    # start bookself books closed
        # did we find an agePanel to link with?
        if agePanel:
            try:
                params = xLinkingBookDefs.xAgeLinkingBooks[agePanel]
                if len(params) == 6:
                    sharable,width,height,stampdef,bookdef,gui = params
                elif len(params) == 5:
                    sharable,width,height,stampdef,bookdef = params
                    gui = "BkBook"
                else:
                    return
                if not IsDRCStamped.value:
                    stampdef = xLinkingBookDefs.NoDRCStamp
                if sharable:
                        PtDebugPrint("ahnyLinkBookGUIPopup: %s's book definition can't be shared" % (agePanel),level=kErrorLevel)
                else:
                    bookdef = bookdef % ('', stampdef)
                SpawnPointName_Dict[0] = "LinkInPointDefault"
                SpawnPointTitle_Dict[0] = agePanel
                PtSendKIMessage(kDisableKIandBB,0)
                gLinkingBook = ptBook(bookdef,self.key)
                gLinkingBook.setSize( width, height )
                # make sure there is a cover to show
                if not showOpen:
                    if not self.IsThereACover(bookdef):
                        showOpen = 1
                gLinkingBook.setGUI(gui)
                gLinkingBook.show(showOpen)
            except LookupError:
                PtDebugPrint("ahnyLinkBookGUIPopup: could not find age %s's linking panel" % (agePanel),level=kErrorLevel)
        else:
            PtDebugPrint("ahnyLinkBookGUIPopup: no age link panel" % (agePanel),level=kErrorLevel)
        '''

    def IsThereACover(self,bookHtml):
        # search the bookhtml string looking for a cover
        idx = bookHtml.find('<cover')
        if idx > 0:
            return 1
        return 0

    def HideBook(self, islinking = 0):
        global gLinkingBook
        global NoReenableBook

        if islinking:
            NoReenableBook = 1
        else:
            NoReenableBook = 0
        
        PtToggleAvatarClickability(true) # enable me as clickable
        if gLinkingBook:
            gLinkingBook.hide()
        

    #
    #
    #
    #
    #
    #
    # utility functions:

    def IGetAgeFilename(self):
        try:
            name = xLinkingBookDefs.xLinkDestinations[TargetAge.value][0]
        except:
            PtDebugPrint("IGetAgeFilename(): " + TargetAge.value + " is missing from the xLinkDestinations table, attempting to use it as the value")
            name = TargetAge.value
        return name

    def IGetAgeInstanceName(self):
        try:
            name = xLinkingBookDefs.xLinkDestinations[TargetAge.value][0]
        except:
            PtDebugPrint("IGetAgeInstanceName(): " + TargetAge.value + " is missing from the xLinkDestinations table, attempting to use it as the value")
            name = TargetAge.value
        return name

    def IGetAgeSpawnPoint(self):
        try:
            name = xLinkingBookDefs.xLinkDestinations[TargetAge.value][1]
        except:
            PtDebugPrint("IGetAgeSpawnPoint(): " + TargetAge.value + " is missing from the xLinkDestinations table, attempting to use an empty string as the value")
            name = ""
        return name

    def OnTimer(self,id):
        global gLinkingBook
        global kGrsnTeamBook
        if id == kGrsnTeamBook:
            print "\nahnyLinkBookGUIPopup.OnTimer:Got timer callback. Removing popup for a grsn team book."
            gLinkingBook.hide()

    def GetCurrentSphere(self):
        vault = ptVault()
        myAges = vault.getAgesIOwnFolder()
        myAges = myAges.getChildNodeRefList()
        for ageInfo in myAges:
            link = ageInfo.getChild()
            link = link.upcastToAgeLinkNode()
            info = link.getAgeInfo()
            if not info:
                continue
            ageName = info.getAgeFilename()

            if ageName == "Ahnonay":
                ahnySDL = info.getAgeSDL()
                ahnyRecord = ahnySDL.getStateDataRecord()
                currentSphere = ahnyRecord.findVar("ahnyCurrentSphere")
                curSphere = currentSphere.getInt(0)
                return curSphere
        
