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
Module: dsntWatsonGUIPopup
Age: Descent
Date: January 2004

Special version of xJournalBooksGUIPopup, for use in Expansion 1
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaConstants import *
from PlasmaKITypes import *
import string
import xLocalization


# define the attributes that will be entered in max
actClickableBook = ptAttribActivator(1,"Actvtr: Clickable small book")
SeekBehavior = ptAttribBehavior(2, "Smart seek before GUI (optional)")
JournalName = ptAttribString(3, 'Name of Journal', '')
StartOpen = ptAttribBoolean(10,"Start Opened",default=0)

# globals
LocalAvatar = None

# the global ptBook object.... there can only be one book displayed at one time, so only one global needed (hopefully)
gJournalBook = None

# stores a list of the last pages of each journal we had open, so we go to the same place
kLastPageOpenChronicle = "LastJournalPageArray"
kChronicleType = 0

class dsntWatsonGUIPopup(ptModifier):
    "Watson's Journal GUI Popup python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5656
        self.version = 1
        self.curPage = 0
    
    def IGetLastPageMap(self):
        vault = ptVault()
        chronicle = vault.findChronicleEntry(kLastPageOpenChronicle)
        chronicleValue = ""
        if type(chronicle) != type(None):
            chronicle = chronicle.upcastToChronicleNode()
            chronicleValue = chronicle.chronicleGetValue()
        values = chronicleValue.split(",")
        try:
            values.remove('') # remove the trailing blank item if there is one
        except:
            pass
        if len(values) == 0:
            # values is an empty list, but if we return it stuff breaks
            # so lets just return an empty dictionary instead
            return {}
        map = {}
        for item in values:
            parts = item.split(":")
            try:
                if parts[0] == "" or parts[1] == "":
                    PtDebugPrint("xJournalBookGUIPopup.IGetLastPageMap(): Item "+str(item)+" is probably in the old format, skipping")
                    continue # possibly old format, so skip it
            except:
                PtDebugPrint("xJournalBookGUIPopup.IGetLastPageMap(): Item "+str(item)+" is probably in the old format, skipping")
                continue # possibly old format, so skip it
            key = parts[0]
            value = int(parts[1])
            map[key] = value
        return map
    
    def IWriteLastPageMap(self,map):
        vault = ptVault()
        chronicleValue = ""
        keys = xLocalization.xJournalBookDefs.xJournalBooks.keys()
        for key in keys:
            chronicleValue += key + ":" + str(map[key]) + ","
        vault.addChronicleEntry(kLastPageOpenChronicle,kChronicleType,chronicleValue)

    def OnServerInitComplete(self):
        values = self.IGetLastPageMap()

        numJournals = len(xLocalization.xJournalBookDefs.xJournalBooks)
        if len(values) < numJournals:
            # expand the list so it has an entry for all the journals
            PtDebugPrint("xJournalBookGUIPopup: Not enough page entries in the chronicle value, fleshing it out")
            keys = xLocalization.xJournalBookDefs.xJournalBooks.keys()
            for key in keys:
                try:
                    page = values[key] # if this fails, then the book is missing and needs to be added
                except:
                    values[key] = 1
            self.IWriteLastPageMap(values)

    def __del__(self):
        "destructor - get rid of any dialogs that we might have loaded"
        pass

    def OnNotify(self,state,id,events):
        global LocalAvatar

        # is it a clickable book on a pedestal?
        if id == actClickableBook.id:
            if PtWasLocallyNotified(self.key) and state:
                PtToggleAvatarClickability(false)
                if type(SeekBehavior.value) != type(None): #remember, smart seek before GUI is optional. 
                    PtDebugPrint("xJournalBookGUIPopup: Smart seek used",level=kDebugDumpLevel)
                    LocalAvatar = PtFindAvatar(events)
                    SeekBehavior.run(LocalAvatar)
                    return
                self.IShowBook()
                return

        # is it the seek behavior because we clicked on a book ourself?    
        elif id == SeekBehavior.id:
            if PtWasLocallyNotified(self.key):
                for event in events:
                    if event[0] == kMultiStageEvent and event[2] == kEnterStage: # Smart seek completed. Exit multistage, and show GUI.
                        SeekBehavior.gotoStage(LocalAvatar, -1) 
                        PtDebugPrint("xJournalBookGUIPopup: attempting to draw link panel gui",level=kDebugDumpLevel)
                        self.IShowBook()

        # else was it one of the unknown id types? like scene input interface or from a ptBook?
        else:
            for event in events:
                # is it from the OpenBook? (we only have one book to worry about)
                if event[0] == PtEventType.kBook:
                    PtDebugPrint("xJournalBookGUIPopup: BookNotify  event=%d, id=%d" % (event[1],event[2]),level=kDebugDumpLevel)
                    if event[1] == PtBookEventTypes.kNotifyShow:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyShow",level=kDebugDumpLevel)
                        if self.curPage > 1:
                            PtDebugPrint("xJournalBookGUIPopup: The last page that "+JournalName.value+" was on was page "+str(self.curPage)+". Going to that page")
                            gJournalBook.open(0)
                            gJournalBook.goToPage(self.curPage)
                    if event[1] == PtBookEventTypes.kNotifyHide:
                        PtToggleAvatarClickability(true)
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyHide",level=kDebugDumpLevel)
                        PtDebugPrint("xJournalBookGUIPopup: Saving the last page we were at to the chronicle: "+str(self.curPage))
                        lastPageList = self.IGetLastPageMap()
                        lastPageList[JournalName.value] = self.curPage
                        self.IWriteLastPageMap(lastPageList)
                    elif event[1] == PtBookEventTypes.kNotifyNextPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyNextPage",level=kDebugDumpLevel)
                        self.curPage = gJournalBook.getCurrentPage()
                        vault = ptVault()
                        entry = vault.findChronicleEntry("WatsonJournal")
                        if type(entry) == type(None):
                            vault.addChronicleEntry("WatsonJournal",1,"%d" % (1))
                            PtDebugPrint("Chronicle entry WatsonJournal not present, adding entry and setting to 1")
                        else:
                            PtDebugPrint("Chronicle entry WatsonJournal already present, will do nothing")
                    elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyPreviousPage",level=kDebugDumpLevel)
                        self.curPage = gJournalBook.getCurrentPage()
                    elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyCheckUncheck",level=kDebugDumpLevel)
                        pass

    def IShowBook(self):
        "Show the linking book with no treasure pages"
        global gJournalBook
        global JournalName
        global StartOpen
        startOpened = StartOpen.value
        journalName = JournalName.value
        if journalName:
            try:
                lastPageList = self.IGetLastPageMap()
                lastPage = lastPageList[journalName]
                self.curPage = lastPage
                params = xLocalization.xJournalBookDefs.xJournalBooks[journalName]
                if len(params) == 4:
                    width,height,module,gui = params
                else:
                    width,height,module = params
                    gui = "BkBook"
                gJournalBook = ptBook(module.xJournalContents,self.key)
                gJournalBook.setSize(width,height)
                # make sure there is a cover to show
                if not startOpened:
                    if not self.IsThereACover(module.xJournalContents):
                        startOpened = 1
                gJournalBook.setGUI(gui)
                gJournalBook.allowPageTurning(true)
                gJournalBook.show(startOpened)
            except LookupError:
                PtDebugPrint("xJournalBookGUIPopup: could not find journal %s's contents" % (journalName),level=kErrorLevel)
        else:
            PtDebugPrint("xJournalBookGUIPopup: no journal name",level=kErrorLevel)

    def IsThereACover(self,bookHtml):
        # search the bookhtml string looking for a cover
        idx = bookHtml.find('<cover')
        if idx >= 0:
            return 1
        return 0
