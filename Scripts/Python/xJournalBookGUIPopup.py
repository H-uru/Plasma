# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/
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

import re
from typing import *
import webbrowser
import zlib

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

# Konstants
kJournalPageChronName = "LastJournalPage"

# pfJournalBook's EsHTML will only allow links with integer events.
# This isn't very friendly for age creators, so we'll let them specify
# a url by doing <a href="https://foo.com">, and this regex will
# convert that to an integer id. It will do the same for any tag, but
# only <img> and <movie> are useful.
_URL_REGEX = re.compile(r"(?P<href>href\s*=\s*(?P<quote>[\"\']?)\s*?(?P<url>https?:\/\/[^\s>]+)\s*?(?P=quote))(?=[^<]*>)", re.IGNORECASE)
_HOST_REGEX = re.compile(R"(?:https?:\/\/)?(?:www\.)?([^\/?#]+).*", re.IGNORECASE)

class JournalPageCache(NamedTuple):
    JournalHash: str
    PageNum: Union[int, str]


class xJournalBookGUIPopup(ptModifier):
    "The Journal Book GUI Popup python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 203
        self.version = 1

        self.JournalBook: Optional[ptBook] = None
        self._JournalHash: Optional[str] = None
        self._OpenToPage: Optional[int] = None

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
                    if event[1] == PtBookEventTypes.kNotifyImageLink:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyImageLink",level=kDebugDumpLevel)
                        try:
                            url = self.links[event[2]]
                        except IndexError:
                            pass
                        else:
                            def open_url(result):
                                if result == PtConfirmationResult.Yes:
                                    webbrowser.open_new_tab(url)

                            hostname = _HOST_REGEX.sub(R"\1", url)
                            PtDebugPrint(f"xJournalBookGUIPopup: NotifyImageLink: Prompting user to open {hostname=} {url=} {event[2]=}", level=kWarningLevel)
                            PtLocalizedYesNoDialog(open_url, "KI.Messages.OpenHyperlink", hostname)
                    elif event[1] == PtBookEventTypes.kNotifyShow:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyShow",level=kDebugDumpLevel)
                        # disable the KI
                        PtSendKIMessage(kDisableKIandBB,0)
                        # open to requested page
                        # use an explicit test against None because the walrus tests for truthiness
                        if self._OpenToPage is not None:
                            PtDebugPrint(f"xJournalBookGUIPopup:Book: NotifyShow - opening to page {self._OpenToPage}", level=kWarningLevel)
                            self.JournalBook.open(self._OpenToPage)
                    elif event[1] == PtBookEventTypes.kNotifyHide:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyHide",level=kDebugDumpLevel)
                        # re-enable KI
                        PtSendKIMessage(kEnableKIandBB,0)
                        # re-enable our avatar
                        PtToggleAvatarClickability(True)
                    elif event[1] == PtBookEventTypes.kNotifyNextPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyNextPage",level=kDebugDumpLevel)
                        self.ISavePage(self.JournalBook.getCurrentPage())
                    elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyPreviousPage",level=kDebugDumpLevel)
                        self.ISavePage(self.JournalBook.getCurrentPage())
                    elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyCheckUncheck",level=kDebugDumpLevel)
                    elif event[1] == PtBookEventTypes.kNotifyClose:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyClose", level=kDebugDumpLevel)
                        # if we're closing but not hiding, that means the user manually went to the
                        # cover, so clear out the saved page such that the next "show" of this book
                        # will open to the cover.
                        if not event[2]:
                            self.ISavePage(page=None)
                    elif event[1] == PtBookEventTypes.kNotifyOpen:
                        PtDebugPrint("xJournalBookGUIPopup:Book: NotifyOpen", level=kDebugDumpLevel)
                        self.ISavePage(self.JournalBook.getCurrentPage())

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
        journalContents = ""
        if Dynamic.value:
            inbox = ptVault().getGlobalInbox()
            journalTemplate = ptVaultFolderNode(0)
            journalTemplate.setFolderName("Journals")
            if journalFolderNode := inbox.findNode(journalTemplate):
                textTemplate = ptVaultTextNoteNode(0)
                textTemplate.setTitle(JournalIdent)
                if textNoteNode := journalFolderNode.findNode(textTemplate):
                    textNoteNode = textNoteNode.upcastToTextNoteNode()
                    journalContents = textNoteNode.getText()
        else:
            journalContents = PtGetLocalizedString(LocPath.value)

        if not journalContents:
            PtDebugPrint("WARNING - EMPTY JOURNAL: JournalName.value = '%s' LocPath = '%s'" % (JournalName.value, LocPath.value), level=kErrorLevel)

        # hide the KI
        PtSendKIMessage(kDisableKIandBB, 0)

        # update the hash for automatic page flipping
        self._JournalHash = f"{zlib.crc32(JournalIdent.encode()):08x},{zlib.crc32(journalContents.encode()):08x}"
        PtDebugPrint(f"xJournalBookGUIPopup.IShowBook(): {self._JournalHash=}", level=kDebugDumpLevel)
        self._OpenToPage = self._CachedPage

        # now build the book
        self.JournalBook = ptBook(self.IPreprocessJournalContents(journalContents), self.key)
        self.JournalBook.setSize(BookWidth.value, BookHeight.value)
        self.JournalBook.setGUI(GUIType.value)

        # make sure there is a cover to show
        startOpen = StartOpen.value or not self.IsThereACover(journalContents)
        self.JournalBook.show(startOpen)

    def IPreprocessJournalContents(self, journalContents: str):
        self.links = []
        def replace_url(match: re.Match) -> str:
            url = match.group("url")
            event = len(self.links)

            PtDebugPrint(f"xJournalBookGUIPopup.IPreprocessJournalContents(): Found {url=} {event=}")
            self.links.append(url)
            return f"link={event}"

        newContent = _URL_REGEX.sub(replace_url, journalContents)
        PtDebugPrint(
            "xJournalBookGuiPopup.IPreprocessJournalContents(): The journal content has been preprocessed to:",
            newContent,
            level=kDebugDumpLevel
        )
        return newContent

    def IsThereACover(self, bookHtml):
        return "<cover" in bookHtml

    def IIterateJournalCache(self, cache: str):
        for i in cache.split():
            entry = i.split(':')
            if len(entry) < 2:
                continue
            yield JournalPageCache(entry[0], entry[1])

    def IStringifyJournalCache(self, cache: Iterable[JournalPageCache]) -> str:
        return ' '.join(f"{i.JournalHash}:{i.PageNum}" for i in cache)

    @property
    def _CachedPage(self) -> Optional[int]:
        vault = ptVault()
        if chron := vault.findChronicleEntry(kJournalPageChronName):
            journals = self.IIterateJournalCache(chron.getValue())
            if pageNum := next((i.PageNum for i in journals if i.JournalHash == self._JournalHash), None):
                try:
                    pageNum = int(pageNum)
                except ValueError:
                    PtDebugPrint(f"xJournalBookGUIPopup._CachedPage(): Unable to convert {pageNum=} to integer")
                else:
                    PtDebugPrint(f"xJournalBookGUIPopup._CachedPage(): {pageNum=}", level=kDebugDumpLevel)
                    return pageNum

    def ISavePage(self, page: Union[int, None]) -> None:
        vault = ptVault()
        if chron := vault.findChronicleEntry(kJournalPageChronName):
            journals = [i for i in self.IIterateJournalCache(chron.getValue()) if i.JournalHash != self._JournalHash]
            if page is not None:
                journals.insert(0, JournalPageCache(self._JournalHash, page))

            # Limit the chronicle to no more than 1024 characters due to some servers only allowing the
            # Text_1 field to be that long.
            chronValue = self.IStringifyJournalCache(journals)
            while len(chronValue) > 1024 and journals:
                journals.pop()
                chronValue = self.IStringifyJournalCache(journals)

            chron.setValue(chronValue)
        elif page is not None:
            journals = [JournalPageCache(self._JournalHash, page)]
            chronValue = self.IStringifyJournalCache(journals)
            vault.addChronicleEntry(kJournalPageChronName, 0, chronValue)
        else:
            PtDebugPrint("xJournalBookGUIPopup.ISavePage(): Got a request to remove the saved page, but the chronicle doesn't exist?")
