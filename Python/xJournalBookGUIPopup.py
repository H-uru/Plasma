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
Module: xLinkingBookGUIPopup
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

import xJournalBookDefs


# define the attributes that will be entered in max
actClickableBook    = ptAttribActivator(1,"Actvtr: Clickable small book")
SeekBehavior        = ptAttribBehavior(2, "Smart seek before GUI (optional)")
JournalName         = ptAttribString(3, "Name of Journal", "")
StartOpen           = ptAttribBoolean(10,"Start Opened?",default=0)
Dynamic             = ptAttribBoolean(4,"Read Data From Vault?",default=0)

# globals
LocalAvatar = None

# the global ptBook object.... there can only be one book displayed at one time, so only one global needed (hopefully)
gJournalBook = None

class xJournalBookGUIPopup(ptModifier):
    "The Journal Book GUI Popup python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 203
        self.version = 1

    def OnFirstUpdate(self):
        pass

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
                        # disable the KI
                        PtSendKIMessage(kDisableKIandBB,0)
                    if event[1] == PtBookEventTypes.kNotifyHide:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyHide",level=kDebugDumpLevel)
                        # re-enable KI
                        PtSendKIMessage(kEnableKIandBB,0)
                        # re-enable our avatar
                        PtToggleAvatarClickability(true)
                    elif event[1] == PtBookEventTypes.kNotifyNextPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyNextPage",level=kDebugDumpLevel)
                    elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyPreviousPage",level=kDebugDumpLevel)
                    elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyCheckUncheck",level=kDebugDumpLevel)
                        pass

    def IShowBook(self):
        global gJournalBook

        if JournalName.value != "":
            try:
                params = xJournalBookDefs.xJournalBooks[JournalName.value]
                if len(params) == 4:
                    width,height,locPath,gui = params
                else:
                    width,height,locPath = params
                    gui = "BkBook"
            except LookupError:
                PtDebugPrint("xJournalBookGUIPopup: could not find journal %s's contents" % (JournalName.value),level=kErrorLevel)
                return

            journalContents = "I'm an empty book"

            # compile journal text
            if Dynamic.value:
                inbox = ptVault().getGlobalInbox()
                inboxChildList = inbox.getChildNodeRefList()
                for child in inboxChildList:
                    PtDebugPrint("xJournalBookGUIPopupDyn: looking at node " + str(child),level=kDebugDumpLevel)
                    node = child.getChild()
                    folderNode = node.upcastToFolderNode()
                    if type(folderNode) != type(None):
                        PtDebugPrint("xJournalBookGUIPopupDyn: node is named %s" % (folderNode.getFolderName()),level=kDebugDumpLevel)
                        if folderNode.getFolderName() == "Journals":
                            folderNodeChildList = folderNode.getChildNodeRefList()
                            for folderChild in folderNodeChildList:
                                PtDebugPrint("xJournalBookGUIPopupDyn: looking at child node " + str(folderChild),level=kDebugDumpLevel)
                                childNode = folderChild.getChild()
                                textNode = childNode.upcastToTextNoteNode()
                                if type(textNode) != type(None):
                                    PtDebugPrint("xJournalBookGUIPopupDyn: child node is named %s" % (textNode.getTitle()),level=kDebugDumpLevel)
                                    if textNode.getTitle() == JournalName.value:
                                        journalContents = textNode.getText()
                                        PtDebugPrint("xJournalBookGUIPopupDyn: journal contents are '%s'" % (journalContents),level=kDebugDumpLevel)
            else:
                journalContents = PtGetLocalizedString(locPath)
            
            if journalContents == U"":
                print U"WARNING - EMPTY JOURNAL: JournalName.value = " + JournalName.value + U" locPath = " + locPath
            
            # hide the KI
            PtSendKIMessage(kDisableKIandBB,0)
            
            # now build the book
            gJournalBook = ptBook(journalContents,self.key)
            gJournalBook.setSize(width,height)
            gJournalBook.setGUI(gui)

            # make sure there is a cover to show
            if not StartOpen.value and not self.IsThereACover(journalContents):
                gJournalBook.show(1)
            else:
                gJournalBook.show(StartOpen.value)

        else:
            PtDebugPrint("xJournalBookGUIPopup: no journal name",level=kErrorLevel)

    def IsThereACover(self,bookHtml):
        # search the bookhtml string looking for a cover
        idx = bookHtml.find('<cover')
        if idx >= 0:
            return 1
        return 0
