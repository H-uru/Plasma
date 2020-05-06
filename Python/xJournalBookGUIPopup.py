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

import xJournalBookDefs


# define the attributes that will be entered in max
actClickableBook    = ptAttribActivator(1,"Actvtr: Clickable small book")
SeekBehavior        = ptAttribBehavior(2, "Smart seek before GUI (optional)")
JournalName         = ptAttribString(3, "Name of Journal (deprecated)", "")  # Phasing this out; we don't want to use global Python for data.
Dynamic             = ptAttribBoolean(4,"Read Data From Vault?",default=0)
StartOpen           = ptAttribBoolean(10,"Start Opened?",default=0)

BookWidth           = ptAttribFloat(11,"Book Width Scaling",default=1.0)
BookHeight          = ptAttribFloat(12,"Book Height Scaling",default=1.0)
LocPath             = ptAttribString(13,"Localization Path for Journal Contents",default="Global.Journals.Empty")
GUIType             = ptAttribString(14,"Book GUI Type",default="bkBook")

# globals
LocalAvatar = None

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

    def OnNotify(self, state, id, events):
        global LocalAvatar

        # is it a clickable book on a pedestal?
        if id == actClickableBook.id:
            if PtWasLocallyNotified(self.key) and state:
                PtToggleAvatarClickability(False)
                if SeekBehavior.value is not None: #remember, smart seek before GUI is optional. 
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
                        PtToggleAvatarClickability(True)
                    elif event[1] == PtBookEventTypes.kNotifyNextPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyNextPage",level=kDebugDumpLevel)
                    elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyPreviousPage",level=kDebugDumpLevel)
                    elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyCheckUncheck",level=kDebugDumpLevel)
                        pass

    def IShowBook(self):
        self.JournalBook = None

        # This lookup should be removed once all PFMs are converted to specify their details
        if JournalName.value:
            PtDebugPrint("xJournalBookGUIPopup: deprecated journal format, using name '%s'" % (JournalName.value), level=kErrorLevel)
            try:
                params = xJournalBookDefs.xJournalBooks[JournalName.value]
                JournalIdent = JournalName.value
                if len(params) == 4:
                    BookWidth.value,BookHeight.value,LocPath.value,GUIType.value = params
                else:
                    BookWidth.value,BookHeight.value,LocPath.value = params
            except LookupError:
                PtDebugPrint("xJournalBookGUIPopup: could not find journal parameters for '%s'" % (JournalName.value), level=kErrorLevel)
                return
        else:
            JournalIdent = LocPath.value

        # compile journal text
        if Dynamic.value:
            inbox = ptVault().getGlobalInbox()
            inboxChildList = inbox.getChildNodeRefList()
            for child in inboxChildList:
                PtDebugPrint("xJournalBookGUIPopupDyn: looking at node " + str(child), level=kDebugDumpLevel)
                node = child.getChild()
                folderNode = node.upcastToFolderNode()
                if folderNode:
                    PtDebugPrint("xJournalBookGUIPopupDyn: node is named %s" % (folderNode.getFolderName()), level=kDebugDumpLevel)
                    if folderNode.getFolderName() == "Journals":
                        folderNodeChildList = folderNode.getChildNodeRefList()
                        for folderChild in folderNodeChildList:
                            PtDebugPrint("xJournalBookGUIPopupDyn: looking at child node " + str(folderChild), level=kDebugDumpLevel)
                            childNode = folderChild.getChild()
                            textNode = childNode.upcastToTextNoteNode()
                            if textNode:
                                PtDebugPrint("xJournalBookGUIPopupDyn: child node is named %s" % (textNode.getTitle()), level=kDebugDumpLevel)
                                # TODO: Convert this to use LocPath.value and migrate node values in DB if necessary once all PFMs
                                #  are converted to use LocalizationPaths
                                if textNode.getTitle() == JournalIdent:
                                    journalContents = textNode.getText()
                                    PtDebugPrint("xJournalBookGUIPopupDyn: journal contents are '%s'" % (journalContents), level=kDebugDumpLevel)
        else:
            journalContents = PtGetLocalizedString(LocPath.value)

        if not journalContents:
            PtDebugPrint("WARNING - EMPTY JOURNAL: JournalName.value = '%s' LocPath = '%s'" % (JournalName.value, LocPath.value), level=kDebugDumpLevel)

        # hide the KI
        PtSendKIMessage(kDisableKIandBB, 0)

        # now build the book
        self.JournalBook = ptBook(journalContents, self.key)
        self.JournalBook.setSize(BookWidth.value, BookHeight.value)
        self.JournalBook.setGUI(GUIType.value)

        # make sure there is a cover to show
        if not StartOpen.value and not self.IsThereACover(journalContents):
            self.JournalBook.show(1)
        else:
            self.JournalBook.show(StartOpen.value)


    def IsThereACover(self, bookHtml):
        # search the bookhtml string looking for a cover
        idx = bookHtml.find('<cover')
        if idx >= 0:
            return 1
        return 0
