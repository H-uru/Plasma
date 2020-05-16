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
from PlasmaNetConstants import *
import xLinkingBookDefs
from xPsnlVaultSDL import *
import time


# define the attributes that will be entered in max
actClickableBook = ptAttribActivator(1,"Actvtr: Clickable small book")
SeekBehavior = ptAttribBehavior(2, "Smart seek before GUI (optional)", 0, 0)
respLinkResponder = ptAttribResponder(3,"Rspndr: Link out")
TargetAge = ptAttribString(4, 'Name of Linking Panel', 'Teledahn')

actBookshelf = ptAttribActivator(5, "Bookshelf (Only used in PsnlAge)") #Leave blank unless it's a Personal Age Bookshelf
shareRegion = ptAttribActivator(6, "region in which the sharer must remain")
shareBookSeek = ptAttribBehavior(7,"smart seek & use book for share acceptance") # different, because the offerer's client links the offeree in this case

IsDRCStamped = ptAttribBoolean(10,"DRC Stamp",default=1)

ForceThirdPerson = ptAttribBoolean(11, "Force 3rd person", default = 0)


# globals
OfferedBookMode = False
BookOfferer = None
stringAgeRequested = None
PageID_List  = []
SpawnPointName_Dict = {}
SpawnPointTitle_Dict = {}

OffereeWalking = False
ClosedBookToShare = 0

BookNumber = 0 # which book it is on the shelf. The Neighborhood book currently is 0. The Teledahn Book is currently 2.
CurrentPage = 1 # The last page the book was opened to. Read from an SDL. 

# the global ptBook object.... there can only be one book displayed at one time, so only one global needed (hopefully)
gLinkingBook = None
NoReenableBook = 0

kGrsnTeamBook = 99

kFirstPersonEnable = 1
kFirstPersonEnableTime = 0.5

pelletCaveGUID = None


class xLinkingBookGUIPopup(ptModifier):
    "The Linking Book GUI Popup python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5221
        version = 27
        minor = 4
        self.version = version
        PtDebugPrint("__init__xLinkingBookGUIPopup v%d.%d" % (version,minor))
        
        #Load Explorer Only Dialog (to display when visitors try to display explorer only content)


    def OnServerInitComplete(self):
        # only in the personal age should actBookshelf be anything, so this should only happen in the personal age
        if len(actBookshelf.value) != 0:
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("CurrentPage", 1,1)
            ageSDL.sendToClients("CurrentPage")



    def OnNotify(self,state,id,events):
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
        if id==shareRegion.id:
            if PtWasLocallyNotified(self.key):
                for event in events:
                    if (event[0]==kCollisionEvent and not event[1]): # False being an exit
                        PtDebugPrint("xLinkingBookGUIPopup: exited book offer region",level=kDebugDumpLevel)
                        PtClearOfferBookMode()
                        self.HideBook()
                return

        # am I walking here as a result of accepting the offered book
        elif id==shareBookSeek.id:
            PtDebugPrint("xLinkingBookGUIPopup: notified of share book seek beh",level=kDebugDumpLevel)
            for event in events:
                PtDebugPrint("xLinkingBookGUIPopup: event[0]=",event[0],level=kDebugDumpLevel)
                PtDebugPrint("xLinkingBookGUIPopup: event[1]=",event[1],level=kDebugDumpLevel)
                PtDebugPrint("xLinkingBookGUIPopup: event[2]=",event[2],level=kDebugDumpLevel)
                
                if event[0] == kMultiStageEvent and event[2] == kEnterStage and OffereeWalking: # Smart seek completed. Exit multistage, and show GUI.
                    OffereeWalking = False
                    PtDebugPrint("xLinkingBookGUIPopup: accepted link, notifying offerer of such",level=kDebugDumpLevel)
                    OfferedBookMode = False
                    avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                    PtNotifyOffererLinkCompleted(avID) 
                    BookOfferer = None
                    PtToggleAvatarClickability(True)
                    return

        # is it the bookshelf in the personal age?    
        elif id==actBookshelf.id:
            if PtWasLocallyNotified(self.key):
                #determine which page the book is turned to
                ageSDL = PtGetAgeSDL()
                ShelfABoolOperated = ageSDL["ShelfABoolOperated"][0]            
                # This book came from the psnlBookshelf
                for event in events:
                    if event[0] == kVariableEvent:
                        if event[1][:8] == "Volatile" or event[1][:11] == "NotVolatile":
                            return
                        stringAgeRequested = event[1].split(",")[0]
                        TargetAge.value = stringAgeRequested  #Save this for later identification (i.e. visitor age checking)
                        
                        try:
                            BookNumber = int(event[1].split(",")[1])
                            CurrentPage = ageSDL["CurrentPage"][BookNumber]
                            PtDebugPrint("xLinkingBookGUIPopup: The book was previously bookmarked on page #%d" % (CurrentPage),level=kDebugDumpLevel)
                        except:
                            PtDebugPrint("xLinkingBookGUIPopup: Somebody lost my bookmark. Assuming CurrentPage = 1",level=kErrorLevel)
                            CurrentPage = 1
                        idRequestor = event[3]
                        PtDebugPrint("xLinkingBookGUI.OnNotify():\tpsnlBookshelf user id %d selected book %s from the shelf" % (idRequestor, stringAgeRequested),level=kDebugDumpLevel)
                        self.IShowBookTreasure()
                        OfferedBookMode = False
                        BookOfferer = None

        # is it a clickable book on a pedestal?
        elif id == actClickableBook.id:
            if PtWasLocallyNotified(self.key) and state:
                actClickableBook.disable()
                PtToggleAvatarClickability(False)
                if SeekBehavior.value is not None: #remember, smart seek before GUI is optional. 
                    PtDebugPrint("xLinkingBookGUIPopup: Smart seek used",level=kDebugDumpLevel)
                    reportingAvatar = PtFindAvatar(events)
                    SeekBehavior.run(reportingAvatar)
                    return
                self.IShowBookNoTreasure()
                OfferedBookMode = False
                BookOfferer = None
                return
            elif not PtWasLocallyNotified(self.key) and state:
                if SeekBehavior.value is not None: #remember, smart seek before GUI is optional. 
                    PtDebugPrint("xLinkingBookGUIPopup: Smart seek used",level=kDebugDumpLevel)
                    reportingAvatar = PtFindAvatar(events)
                    SeekBehavior.run(reportingAvatar)
                    return

        # is it the seek behavior because we clicked on a book ourself?    
        elif id == SeekBehavior.id:
            reportingAvatar = PtFindAvatar(events)
            if not reportingAvatar:
                return
            
            for event in events:
                if event[0] == kMultiStageEvent and event[2] == kEnterStage: # Smart seek completed. Exit multistage, and show GUI.
                    SeekBehavior.gotoStage(reportingAvatar, -1)
                    
                    reportingClient = PtGetClientIDFromAvatarKey(reportingAvatar.getKey())
                    localClient     = PtGetLocalClientID()
                    if reportingClient == localClient:
                        PtDebugPrint("xLinkingBookGUIPopup: attempting to draw link panel gui",level=kDebugDumpLevel)
                        self.IShowBookNoTreasure()
                        OfferedBookMode = False
                        BookOfferer = None

        elif id == respLinkResponder.id:
            # age panel names are misleading, this is really for the Pellet Cave
            agePanel = TargetAge.value
            if agePanel == "BahroCaveUpper":
                self.LinkToPelletCave("upper")
                return
            elif agePanel == "BahroCaveLower":
                self.LinkToPelletCave("lower")
                return
        # else was it one of the unknown id types? like scene input interface or from a ptBook?
        else:
            for event in events:
                # is it a notification from the scene input interface?
                if event[0] == kOfferLinkingBook: # this can come locally (to re-draw the panel after someone has accepted or rejected it) or...
                    PtDebugPrint("xLinkingBookGUIPopup: got offer book notification",level=kDebugDumpLevel)
                    # make sure this is intended for us
                    localAv = PtGetLocalClientID()
                    messageAv = event[3]
                    if (messageAv != localAv):
                        PtDebugPrint("xLinkingBookGUIPopup: offered book message for someone else",level=kDebugDumpLevel)
                        return
                    PtDebugPrint("xLinkingBookGUIPopup: offered book message for me",level=kDebugDumpLevel)
                    if (event[2] == -999): # if the offerer is recinding the book offer, hide the panel, or...
                        if (OffereeWalking): # too late, they already accepted
                            return
                        OfferedBookMode = False
                        BookOfferer = None
                        self.HideBook()
                        return
                    elif (event[2] == 999): # the book is being offered by someone else
                        BookOfferer = event[1]
                        PtDebugPrint("xLinkingBookGUIPopup: offered book by %s" % (BookOfferer.getName()),level=kDebugDumpLevel)
                        avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                        if ptVault().getIgnoreListFolder().playerlistHasPlayer(avID):
                            OfferedBookMode = False
                            PtNotifyOffererLinkRejected(avID) 
                            BookOfferer = None
                            return
                        else:
                            OfferedBookMode = True
                    PtToggleAvatarClickability(False)
                    self.IShowBookNoTreasure()
                    return

                # is it from the OpenBook? (we only have one book to worry about)
                elif event[0] == PtEventType.kBook:
                    PtDebugPrint("xLinkingBookGUIPopup: BookNotify  event=%d, id=%d" % (event[1],event[2]))
                    if event[1] == PtBookEventTypes.kNotifyImageLink:
                        if event[2] == xLinkingBookDefs.kShareBookLinkID:
                            # shares can only be done with pedestal books (not bookshelf books)
                            PtDebugPrint("xLinkingBookGUIPopup:Book: hit share panel; age filename = %s, age instancename = %s" % (self.IGetAgeFilename(),self.IGetAgeInstanceName()))
                            PtSetOfferBookMode(self.key, self.IGetAgeFilename(), self.IGetAgeInstanceName())
                            PtSetShareSpawnPoint(self.IGetAgeSpawnPoint())
                            if self.IGetAgeFilename() == "PelletBahroCave":
                                PtDebugPrint("using Adam's plasma-magical 'PtSetShareAgeInstanceGuid' with pelletcaveguid = ",pelletCaveGUID)
                                PtSetShareAgeInstanceGuid(pelletCaveGUID)
                            ClosedBookToShare = 1
                            self.HideBook()

                        elif event[2] >= xLinkingBookDefs.kFirstLinkPanelID or event[2] == xLinkingBookDefs.kBookMarkID:
                            PtDebugPrint("xLinkingBookGUIPopup:Book: hit linking panel %s" % (event[2]))


                            #Tye: Did this to try and fix linking issues (i.e. moving while trying to link)
                            PtDisableMovementKeys() 

                            if (OfferedBookMode and BookOfferer):
                                self.HideBook()
                                self.DoErcanaAndAhnonayStuff()
                                #walk to the book and use it
                                avatar=PtGetLocalAvatar()
                                avatar.avatar.setReplyKey(self.key)
                                shareBookSeek.run(avatar)
                                OffereeWalking = True
                                avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                                PtNotifyOffererLinkAccepted(avID)
                                ClosedBookToShare = 1
                                PtDebugPrint("xLinkingBookGUIPopup: seeking avatar to use book offered")
                            else:
                                self.HideBook(1)
                                self.DoErcanaAndAhnonayStuff()
                                if len(actBookshelf.value) == 0:  #Pedestal Book
                                    #assume this is a normal pedestal book, (i.e. not the psnlBookshelf) Run the responder indicated in Max
                                    if len(respLinkResponder.value) == 0:
                                        note = ptNotify(self.key)
                                        note.setActivate(1.0)
                                        note.addVarNumber("LinkOut", 1)
                                        note.send()
                                    else:
                                        agePanel = TargetAge.value
                                        #PtDebugPrint("agePanel = ",agePanel)
                                        if agePanel in xLinkingBookDefs.CityBookLinks:
                                            self.IDoCityLinksChron(agePanel)
                                        respLinkResponder.run(self.key,avatar=PtGetLocalAvatar(),netPropagate=0)

                                else:  #Bookshelf Book
                                    if ptVault().amOwnerOfCurrentAge():
                                        #remember which page I was on for the Bookshelf
                                        PtDebugPrint("xLinkingBookGUIPopup: Placing a bookmark for book #%d on page %d" % (BookNumber,CurrentPage),level=kDebugDumpLevel)
                                        ageSDL = PtGetAgeSDL()
                                        ageSDL.setIndex("CurrentPage",BookNumber,CurrentPage)

                                    # if we're going to tomahna then make sure the chronicle reflects that
                                    linkTitle = SpawnPointTitle_Dict[event[2]]
                                    if linkTitle == "Tomahna" or linkTitle == "Cleft":
                                        vault = ptVault()
                                        entry = vault.findChronicleEntry("TomahnaLoad")
                                        if entry is not None:
                                            if linkTitle == "Tomahna":
                                                entry.chronicleSetValue("yes")
                                            else:
                                                entry.chronicleSetValue("no")
                                    
                                    # this book was taken off the personal age bookshelf. Send a note telling to link
                                    note = ptNotify(self.key)
                                    note.setActivate(1.0)
                                    note.addVarNumber("ILink" + "," + SpawnPointName_Dict[event[2]] + "," + SpawnPointTitle_Dict[event[2]], -1.0)
                                    note.netPropagate(0)
                                    note.send()
                            # bye-bye
                    elif event[1] == PtBookEventTypes.kNotifyShow:
                        PtDebugPrint("xLinkingBookGUIPopup:Book: NotifyShow",level=kDebugDumpLevel)
                        # re-allow KI and BB
                        PtSendKIMessage(kDisableKIandBB,0)
                        # should we be on a different page?
                        if CurrentPage > 1:
                            PtDebugPrint("xLinkingBookGUIPopup: going to page %d (ptBook page %d)" % (CurrentPage,(CurrentPage-1)*2),level=kDebugDumpLevel)
                            gLinkingBook.goToPage((CurrentPage-1)*2)
                    elif event[1] == PtBookEventTypes.kNotifyHide:
                        PtDebugPrint("xLinkingBookGUIPopup:Book: NotifyHide",level=kDebugDumpLevel)
                        PtSendKIMessage(kEnableKIandBB,0)
                        if not ClosedBookToShare:
                            PtToggleAvatarClickability(True)
                            if (OfferedBookMode and BookOfferer):
                                avID = PtGetClientIDFromAvatarKey(BookOfferer.getKey())
                                PtNotifyOffererLinkRejected(avID)
                                PtDebugPrint("xLinkingBookGUIPopup: rejected link, notifying offerer as such",level=kDebugDumpLevel)
                                OfferedBookMode = False
                                BookOfferer = None

                        if ForceThirdPerson.value:
                            #cam = ptCamera()
                            #cam.enableFirstPersonOverride()
                            PtAtTimeCallback(self.key, kFirstPersonEnableTime, kFirstPersonEnable)

                        if not NoReenableBook:
                            actClickableBook.enable()

                        ClosedBookToShare = 0

                        #if we got this book from the Personal Age bookshelf, roll back the book tray
                        if len(actBookshelf.value) != 0:
                            PtDebugPrint("xLinkingBookGUIPopup: Roll back the personal age book.",level=kDebugDumpLevel)
                            note = ptNotify(self.key)
                            note.setActivate(1.0)
                            note.addVarNumber("IShelveBook", 1)
                            note.send()
                            if ptVault().amOwnerOfCurrentAge():
                                #remember which page I was on
                                PtDebugPrint("xLinkingBookGUIPopup: Placing a bookmark for book #%d on page %d" % (BookNumber,CurrentPage),level=kDebugDumpLevel)
                                ageSDL = PtGetAgeSDL()
                                ageSDL.setIndex("CurrentPage",BookNumber,CurrentPage)
                    elif event[1] == PtBookEventTypes.kNotifyNextPage:
                        PtDebugPrint("xLinkingBookGUIPopup:Book: NotifyNextPage  new current page=%d" % (CurrentPage+1),level=kDebugDumpLevel)
                        CurrentPage += 1
                    elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                        PtDebugPrint("xLinkingBookGUIPopup:Book: NotifyPreviousPage new current page=%d" % (CurrentPage-1),level=kDebugDumpLevel)
                        CurrentPage -= 1
                    elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                        PtDebugPrint("xLinkingBookGUIPopup:Book: NotifyCheckUncheck",level=kDebugDumpLevel)
                        pass


    def IShowBookNoTreasure(self):
        "Show the linking book with no treasure pages"
        global gLinkingBook
        global SpawnPointName_Dict
        global SpawnPointTitle_Dict
        global kGrsnTeamBook
        global pelletCaveGUID
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
            #this new section is specific to Tomahna
            #it checks if a Tomahna panel is used to link with,
            #if so it sets (or creates and sets, if not already present) a Tomahna chronicle entry to yes,
            #which is then read in by Cleft.py to load in the Tomahna pages
            if agePanel == "Cleft":
                vault = ptVault()
                if not vault.amOwnerOfCurrentAge():
                    agePanel = "DisabledDesert"
            elif agePanel == "TomahnaFromCleft":
                vault = ptVault()
                entry = vault.findChronicleEntry("TomahnaLoad")
                if entry is not None:
                    entry.chronicleSetValue("yes")
                    entry.save()
                    PtDebugPrint("Chronicle entry TomahnaLoad already added, setting to yes")
                else:
                    sdl = xPsnlVaultSDL()
                    sdl["CleftVisited"] = (1,)
                    vault.addChronicleEntry("TomahnaLoad",1,"yes")
                    PtDebugPrint("Chronicle entry TomahnaLoad not present, adding entry and setting to yes")
                respLinkResponder.run(self.key,avatar=PtGetLocalAvatar())
                return
            elif agePanel == "grsnTeamRmPurple" or agePanel == "grsnTeamRmYellow":
                PtAtTimeCallback(self.key,5,kGrsnTeamBook)
            # age panel names are misleading, this is really for the Pellet Cave
            elif agePanel == "BahroCaveUpper":
                pelletCaveGUID = str(self.OnClickToLinkToPelletCaveFromErcana())
                PtDebugPrint("pelletCaveGUID = ",pelletCaveGUID)
            elif agePanel == "BahroCaveLower":
                pelletCaveGUID = str(self.OnClickToLinkToPelletCaveFromAhnonay())
                PtDebugPrint("pelletCaveGUID = ",pelletCaveGUID)
            try:
                params = xLinkingBookDefs.xAgeLinkingBooks[agePanel]
                if len(params) == 6:
                    sharable,width,height,stampdef,bookdef,gui = params
                elif len(params) == 5:
                    sharable,width,height,stampdef,bookdef = params
                    gui = "BkBook"
                else:
                    # this is a treasure book
                    linkingPanel = params
                    self.IShowBahroBook(linkingPanel)
                    return
                if not IsDRCStamped.value:
                    stampdef = xLinkingBookDefs.NoDRCStamp
                if sharable:
                    try:
                        bookdef = bookdef % ('', stampdef,self.IAddShare())
                    except:
                        PtDebugPrint("xLinkingBookGUIPopup: %s's book definition can't be shared" % (agePanel),level=kErrorLevel)
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
                PtDebugPrint("xLinkingBookGUIPopup: could not find age %s's linking panel" % (agePanel),level=kErrorLevel)
        else:
            PtDebugPrint("xLinkingBookGUIPopup: no age link panel" % (agePanel),level=kErrorLevel)


    def IShowBookTreasure(self):
        PtDebugPrint("Show the linking book with all its treasure pages")
        global gLinkingBook
        global CurrentPage
        global SpawnPointTitle_Dict
        global SpawnPointName_Dict
        showOpen = 0
        #If this is a normal pedestal book, the panel is defined in the Max GUI. If this is the personal Age Bookshelf, the panel is passed in a note from psnlBookshelf. Determine the source here.
        if len(actBookshelf.value) == 0:
            agePanel = TargetAge.value
            showOpen = 1  # start pedestal books open
            fromBookshelf = 0
        else:
            agePanel = stringAgeRequested
            fromBookshelf = 1
            showOpen = 0    # start bookself books closed
            # but if bookmarked... then turn to that page
            if CurrentPage > 1:
                showOpen = 1  # start the book at the page where they left off

        # did we find an agePanel to link with?
        PtDebugPrint("agePanel = ",agePanel)
        if agePanel:
            if agePanel == "Cleft":
                vault = ptVault()
                entry = vault.findChronicleEntry("TomahnaLoad")
                if entry is not None:
                    agePanel = "CleftWithTomahna"

            # build the SpawnPointName/Title lists
            
            if ((agePanel == "Cleft") or (agePanel == "CleftWithTomahna")) and not ptVault().amOwnerOfCurrentAge():
                PtDebugPrint("xLinkingBookGUI.IShowBookTreasure(): You're a visitor trying to use the owner's Cleft book. You get the void.")
                SpawnPointTitle_Dict = {xLinkingBookDefs.kFirstLinkPanelID: 'DisabledDesert'}
                SpawnPointName_Dict = {xLinkingBookDefs.kFirstLinkPanelID: 'DisabledDesert'}

            elif (agePanel == "CleftWithTomahna") and ptVault().amOwnerOfCurrentAge():
                PtDebugPrint("setting up cleft and tomahna spawn points")
                SpawnPointTitle_Dict = {xLinkingBookDefs.kFirstLinkPanelID: 'Tomahna', xLinkingBookDefs.kFirstLinkPanelID + 1: 'Cleft'}
                SpawnPointName_Dict = {xLinkingBookDefs.kFirstLinkPanelID: 'SpawnPointTomahna01', xLinkingBookDefs.kFirstLinkPanelID + 1: 'LinkInPointDefault'}

            elif agePanel == "city":
                self.BuildCityBook()
                agePanel = SpawnPointTitle_Dict[xLinkingBookDefs.kFirstLinkPanelID]
                PtDebugPrint("agePanel2 = ",agePanel)
            elif agePanel == "Neighborhood":
                agevault = ptAgeVault()
                nblink = self.GetOwnedAgeLink(agevault, "Neighborhood")
                if not nblink:
                    SpawnPointTitle_Dict = {xLinkingBookDefs.kFirstLinkPanelID: 'Default'}
                    SpawnPointName_Dict = {xLinkingBookDefs.kFirstLinkPanelID: 'LinkInPointDefault'}
                else:
                    self.BuildTreasureLinks(agePanel)
            else:
                PtDebugPrint("setting up spawn points")
                self.BuildTreasureLinks(agePanel)

            PtDebugPrint(SpawnPointTitle_Dict)
            PtDebugPrint(SpawnPointName_Dict)
            
            # start with the first page
            
            try:
                params = xLinkingBookDefs.xAgeLinkingBooks[agePanel]
                if len(params) == 6:
                    sharable,width,height,stampdef,bookdef,gui = params
                elif len(params) == 5:
                    sharable,width,height,stampdef,bookdef = params
                    gui = "BkBook"
                else:
                    if not fromBookshelf:
                        # this is a treasure book
                        linkingPanel = params
                        self.IShowBahroBook(linkingPanel)
                        return
                    else:
                        sharable, width, height, stampdef, gui = (0, 1.0, 1.0, xLinkingBookDefs.NoDRCStamp, "BkBook")
                        bookdef = xLinkingBookDefs.BookStart1 + xLinkingBookDefs.DRCStampHolder + xLinkingBookDefs.NoShare + xLinkingBookDefs.LinkStart + params + xLinkingBookDefs.LinkEnd

                if 'NotPossible' in SpawnPointTitle_Dict.values():
                   bookdef = xLinkingBookDefs.BookStart1 + xLinkingBookDefs.DRCStampHolder + xLinkingBookDefs.NoShare + xLinkingBookDefs.LinkStart + 'xLinkPanelBlackVoid' + xLinkingBookDefs.LinkEndNoLink
                   sharable = 0
                if 'DisabledDesert' in SpawnPointTitle_Dict.values():
                   bookdef = xLinkingBookDefs.BookStart1 + xLinkingBookDefs.DRCStampHolder + xLinkingBookDefs.NoShare + xLinkingBookDefs.LinkStart + 'xLinkPanelCleftDesertDisabled' + xLinkingBookDefs.LinkEndNoLink
                   sharable = 0
                
                if not IsDRCStamped.value:
                    stampdef = xLinkingBookDefs.NoDRCStamp
            except LookupError:
                PtDebugPrint("xLinkingBookGUIPopup: could not find age %s's linking panel or SpawnPoint" % (agePanel),level=kErrorLevel)
                return

            # bookmark link
            if fromBookshelf and xLinkingBookDefs.kBookMarkID in SpawnPointTitle_Dict.keys():
                if SpawnPointTitle_Dict[xLinkingBookDefs.kBookMarkID] == 'JCSavePoint':
                    bookmark = xLinkingBookDefs.JCBookMark
                elif SpawnPointTitle_Dict[xLinkingBookDefs.kBookMarkID] == 'SCSavePoint':
                    bookmark = xLinkingBookDefs.SCBookMark
                else:
                    bookmark = ''
                    PtDebugPrint("xLinkingBookGUIPopup: sorry, don't recognize your bookmark spawn point title")
            else:
                bookmark = ''
            
            if sharable:
                try:
                    allPagesDef = bookdef % (bookmark, stampdef,self.IAddShare())
                except:
                    PtDebugPrint("xLinkingBookGUIPopup: %s's book definition can't be shared" % (agePanel),level=kErrorLevel)
            else:
                if agePanel == "CleftWithTomahna" and ptVault().amOwnerOfCurrentAge():
                    allPagesDef = bookdef % (bookmark, stampdef, xLinkingBookDefs.kFirstLinkPanelID, xLinkingBookDefs.kFirstLinkPanelID + 1)
                else:
                    allPagesDef = bookdef % (bookmark, stampdef)
            # build the rest of the pages into the book
            #linkID = xLinkingBookDefs.kFirstLinkPanelID + 1
            if agePanel != "CleftWithTomahna":
                for linkID in SpawnPointTitle_Dict.keys():
                    if linkID == xLinkingBookDefs.kFirstLinkPanelID or linkID == xLinkingBookDefs.kBookMarkID:
                        continue
                    try:
                        pagedef = xLinkingBookDefs.xLinkingPages[SpawnPointTitle_Dict[linkID]]
                        # pages are not sharable at the moment, not sure if they ever will
                        try:
                            allPagesDef += pagedef % (linkID)
                        except:
                            PtDebugPrint("xLinkingBookGUIPopup: Treasure page %s's book definition doesn't look like a page???" % (SpawnPointTitle_Dict[linkID]),level=kErrorLevel)
                    except LookupError:
                        PtDebugPrint("xLinkingBookGUIPopup: could not find treasure book page %s's linking panel" % (SpawnPointTitle_Dict[linkID]),level=kErrorLevel)
                    
            if allPagesDef != "":
                PtDebugPrint(allPagesDef)
                PtSendKIMessage(kDisableKIandBB,0)
                gLinkingBook = ptBook(allPagesDef,self.key)
                gLinkingBook.setSize( width, height )
                # make sure there is a cover to show
                if not showOpen:
                    if not self.IsThereACover(allPagesDef):
                        showOpen = 1
                gLinkingBook.setGUI(gui)
                vault = ptVault()
                if vault.amOwnerOfCurrentAge():
                   gLinkingBook.allowPageTurning(1)
                else:
                    gLinkingBook.allowPageTurning(0)
                gLinkingBook.show(showOpen)
            else:
                PtDebugPrint("xLinkingBookGUIPopup: couldn't find the book definition for %s???" % (agePanel),level=kErrorLevel)
        else:
            PtDebugPrint("xLinkingBookGUIPopup: no age link panel" % (agePanel),level=kErrorLevel)


    def IGetHoodLinkNode(self):
        vault = ptVault()
        folder = vault.getAgesIOwnFolder()
        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            if link is not None:
                info = link.getAgeInfo()
            if not info: continue
            ageName = info.getAgeFilename()
            if ageName == "Neighborhood":
                return link
        return None


    def IGetHoodInfoNode(self):
        link = self.IGetHoodLinkNode()
        if link is None:
            return None
        info = link.getAgeInfo()
        return info


    def BuildCityBook(self):
        global SpawnPointTitle_Dict
        global SpawnPointName_Dict
        global CurrentPage

        SpawnPointTitle_Dict = {}
        SpawnPointName_Dict = {}
        
        # This section is neccessary because of the chronicle hack.
        # Because a player can see only their chron, and not another player's,
        # a visitor would see their linking pages in the owner's city book.
        # So for now, visitors will only see the black void in an owner's book.
        if not ptVault().amOwnerOfCurrentAge():
            SpawnPointName_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "NotPossible"
            SpawnPointTitle_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "NotPossible"
            CurrentPage = 1
            return
        
        # citybook links have been changed to Hood childages, so we search there now
        #vault = ptAgeVault()
        #OwnedAges = vault.getAgesIOwnFolder().getChildNodeRefList()
        childAgeFolder = self.IGetHoodInfoNode().getChildAgesFolder()
        HoodChildAges = childAgeFolder.getChildNodeRefList()
        spawnPoints = []

        CityLinks = self.IGetCityLinksChron()
        for tmpLink in CityLinks:
            if tmpLink in xLinkingBookDefs.CityBookLinks:
                spName = tmpLink
                spTitle = xLinkingBookDefs.xLinkDestinations[tmpLink][1]
                tmpSp = ptSpawnPointInfo(spName, spTitle)
                spawnPoints.append(tmpSp)

        for NodeRef in HoodChildAges:
            tmpLink = NodeRef.getChild().upcastToAgeLinkNode()
            if tmpLink:
                linkAge = tmpLink.getAgeInfo().getAgeFilename()
                PtDebugPrint("BuildCityBook():  linkAge = ",linkAge)
                sps = tmpLink.getSpawnPoints()
                for sp in sps:
                    PtDebugPrint("before, sp title = ",sp.getTitle())
                    PtDebugPrint("before, sp name = ",sp.getName())

#                if linkAge == "city" or linkAge == "Descent":
#                    spawnPoints.extend(tmpLink.getSpawnPoints())
#                elif linkAge == "GreatZero":
#                    sps = tmpLink.getSpawnPoints()
#                    for sp in sps:
#                        if sp.getTitle() == "grtzGrtZeroLinkRm":
#                            spawnPoints.append(sp)
#                            break
#                elif linkAge == "BaronCityOffice":
#                    bcosp = ptSpawnPointInfo("BaronCityOffice", "LinkInPointDefault")
#                    spawnPoints.append(bcosp)
#                elif linkAge == "spyroom":
#                    spysp = ptSpawnPointInfo("Spyroom", "LinkInPointDefault")
#                    spawnPoints.append(spysp)

        x = xLinkingBookDefs.kFirstLinkPanelID
        for sp in spawnPoints:
            PtDebugPrint("after, sp title = ",sp.getTitle())
            PtDebugPrint("after, sp name = ",sp.getName())
            if sp.getTitle() in xLinkingBookDefs.CityBookLinks:
                SpawnPointTitle_Dict[x] = sp.getTitle()
                if sp.getName() == "p":
                    PtDebugPrint("shouldn't be a 'p' as the spawnpoint name")
                    SpawnPointName_Dict[x] = "LinkInPointDefault"
                else:
                    SpawnPointName_Dict[x] = sp.getName()
                x += 1

        if CurrentPage > len(SpawnPointName_Dict.keys()):
            CurrentPage = 1


    def IDoCityLinksChron(self,agePanel):
        CityLinks = []
        vault = ptVault()
        entryCityLinks = vault.findChronicleEntry("CityBookLinks")
        if entryCityLinks is not None:
            valCityLinks = entryCityLinks.chronicleGetValue()
            PtDebugPrint("valCityLinks = ",valCityLinks)
            CityLinks = valCityLinks.split(",")
            PtDebugPrint("CityLinks = ",CityLinks)
            if agePanel not in CityLinks:
                NewLinks = valCityLinks + "," + agePanel
                entryCityLinks.chronicleSetValue(NewLinks)
                entryCityLinks.save()
                PtDebugPrint("xLinkingBookGUIPopup.IDoCityLinksChron():  setting citylinks chron entry to include: ",agePanel)
                valCityLinks = entryCityLinks.chronicleGetValue()
                CityLinks = valCityLinks.split(",")
                PtDebugPrint("xLinkingBookGUIPopup.IDoCityLinksChron():  citylinks now = ",CityLinks)
            else:
                PtDebugPrint("xLinkingBookGUIPopup.IDoCityLinksChron():  do nothing, citylinks chron already contains: ",agePanel)
        else:
            vault.addChronicleEntry("CityBookLinks",0,agePanel)
            PtDebugPrint("xLinkingBookGUIPopup.IDoCityLinksChron():  creating citylinks chron entry and adding: ",agePanel)
        
        psnlSDL = xPsnlVaultSDL()
        GotBook = psnlSDL["psnlGotCityBook"][0]
        if not GotBook:
            psnlSDL["psnlGotCityBook"] = (1,)
            PtDebugPrint("xLinkingBookGUIPopup.IDoCityLinksChron():  setting SDL for city book to 1")


    def IGetCityLinksChron(self):
        CityLinks = []
        vault = ptVault()
        entryCityLinks = vault.findChronicleEntry("CityBookLinks")
        if entryCityLinks is not None:
            valCityLinks = entryCityLinks.chronicleGetValue()
            PtDebugPrint("xLinkingBookGUIPopup.IGetCityLinksChron(): valCityLinks = ",valCityLinks)
            CityLinks = valCityLinks.split(",")
        return CityLinks	


    def IShowBahroBook(self,linkingPanel):
        global gLinkingBook
        global SpawnPointName_Dict
        global SpawnPointTitle_Dict
        global OfferedBookMode
        PtDebugPrint("xLinkingBookGUIPopup: It's a Bahro Linking Tablet (BLT), so we get to do fun stuff now!")
        agePanel = TargetAge.value
        # share book image is on the "first" page while the linking panel is on the "second" page
        if PtIsSinglePlayerMode() or OfferedBookMode:
            bookdef = '<font size=10>' + xLinkingBookDefs.BahroNoShare +'<pb>' + xLinkingBookDefs.TransLinkStart + linkingPanel + xLinkingBookDefs.LinkEnd
        else:
            bookdef = '<font size=10>' + xLinkingBookDefs.BahroShare +'<pb>' + xLinkingBookDefs.TransLinkStart + linkingPanel + xLinkingBookDefs.LinkEnd
        SpawnPointName_Dict[0] = "LinkInPointDefault"
        SpawnPointTitle_Dict[0] = agePanel
        PtSendKIMessage(kDisableKIandBB,0)
        gLinkingBook = ptBook(bookdef,self.key)
        gLinkingBook.setSize( 1, 1 )
        gLinkingBook.setGUI("bkBahroRockBook")
        gLinkingBook.show(1) # this book is always open

    def IAddShare(self):
        global OfferedBookMode
        # if a pedestal book? (not a bookSelf book)
        if len(actBookshelf.value) == 0:
            # if we are not in offer book mode already
            if not OfferedBookMode:
                # yes, then return the bookHTML for a share book icon
                return xLinkingBookDefs.ShareBook
        # otherwise return empty string
        return ""

    def IsThereACover(self,bookHtml):
        # search the bookhtml string looking for a cover
        idx = bookHtml.find('<cover')
        if idx > 0:
            return 1
        return 0

    def BuildTreasureLinks(self,ageRequested):
        PtDebugPrint("BuildTreasureLinks")
        global SpawnPointName_Dict
        global SpawnPointTitle_Dict
        global CurrentPage
        
        # Step 1: Find this age's spawnPoints
        vault = ptAgeVault()
        OwnedAges = vault.getAgesIOwnFolder().getChildNodeRefList()
        spawnPoints = None
        for NodeRef in OwnedAges:
            tmpLink = NodeRef.getChild().upcastToAgeLinkNode()
            if tmpLink:
                # make sure Micah doesn't try to break it by not waiting for the AgeInfo to be downloaded
                if tmpLink.getAgeInfo() is not None:
                    if ageRequested == tmpLink.getAgeInfo().getAgeFilename():
                        spawnPoints = tmpLink.getSpawnPoints()
                        break

        if ageRequested == "Ahnonay":
            ageVault = ptAgeVault()
            ageInfoNode = ageVault.getAgeInfo()
            ageInfoChildren = ageInfoNode.getChildNodeRefList()
            for ageInfoChildRef in ageInfoChildren:
                ageInfoChild = ageInfoChildRef.getChild()
                folder = ageInfoChild.upcastToFolderNode()
                if folder and folder.folderGetName() == "AgeData":
                    ageDataChildren = folder.getChildNodeRefList()
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "AhnonaySpawnPoints":
                            spawns = chron.getValue().split(";")
                            spawnPoints = []
                            for spawn in spawns:
                                spawnInfo = spawn.split(",")
                                spawnPointInfo = ptSpawnPointInfo(spawnInfo[0], spawnInfo[1])
                                spawnPoints.append(spawnPointInfo)
                            break

        SpawnPointName_Dict = {}
        SpawnPointTitle_Dict = {}
        PtDebugPrint ("xLinkingBookGUI.BuildTreasureLinks():The %s book has the following %s pages: " % (ageRequested, len(spawnPoints)))
        # assume that we didn't find the original link
        HasFoundOriginalBook = False
        # Step 2: Determine what other links they have to this age.
        for spawnPoint in spawnPoints:
            #toss this is it's the "default" link. We've already processed that in step #1 above
            if spawnPoint.getTitle() == "Default": 
                HasFoundOriginalBook = True
                PtDebugPrint("\tPage #1: You've found the original book. The first panel shows %s" % (ageRequested),level=kDebugDumpLevel)
                # goes in the front of the list
                SpawnPointName_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "LinkInPointDefault"
                SpawnPointTitle_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "Default"
            elif spawnPoint.getTitle() == "JCSavePoint" or spawnPoint.getTitle() == "SCSavePoint":
                if len(actBookshelf.value) > 0:
                    SpawnPointTitle_Dict[xLinkingBookDefs.kBookMarkID] = spawnPoint.getTitle()
                    SpawnPointName_Dict[xLinkingBookDefs.kBookMarkID] = spawnPoint.getName()
            else:
                if HasFoundOriginalBook:
                    page = len(SpawnPointTitle_Dict) + xLinkingBookDefs.kFirstLinkPanelID
                else:
                    page = len(SpawnPointTitle_Dict) + xLinkingBookDefs.kFirstLinkPanelID + 1
                    
                PtDebugPrint("\tPage #%s: spawnpoint: %s, LinkPanel/Title: %s" % (page,spawnPoint.getName(),spawnPoint.getTitle()))
                SpawnPointName_Dict[page] = spawnPoint.getName()
                SpawnPointTitle_Dict[page] = spawnPoint.getTitle()
        # if we didn't find the default (original) then put the NotPossible link
        PtDebugPrint("HasFoundOriginalBook = ",HasFoundOriginalBook)
        if not HasFoundOriginalBook:
            if ageRequested == "Neighborhood":
                PtDebugPrint("\tPage #1: You didn't find the original book, but you're looking at the neighborhood. The first panel shows %s" % (ageRequested))
                # goes in the front of the list
                SpawnPointName_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "LinkInPointDefault"
                SpawnPointTitle_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "Default"
            else:
                PtDebugPrint("\tPage #1: You haven't found the original book. The first panel shows black.")
                SpawnPointName_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "NotPossible"
                SpawnPointTitle_Dict[xLinkingBookDefs.kFirstLinkPanelID] = "NotPossible"

        if CurrentPage > len(SpawnPointName_Dict.keys()):
            CurrentPage = 1

    def HideBook(self, islinking = 0):
        global gLinkingBook
        global NoReenableBook

        if islinking:
            NoReenableBook = 1
        else:
            NoReenableBook = 0
        
        PtToggleAvatarClickability(True) # enable me as clickable
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
            PtDebugPrint("\nxLinkingBookGUIPopup.OnTimer:Got timer callback. Removing popup for a grsn team book.")
            gLinkingBook.hide()

        elif id == kFirstPersonEnable:
            cam = ptCamera()
            cam.enableFirstPersonOverride()


    def GetOwnedAgeLink(self, vault, age):
        PAL = vault.getAgesIOwnFolder()
        if PAL is not None:
            contents = PAL.getChildNodeRefList()
            for content in contents:
                link = content.getChild().upcastToAgeLinkNode()
                info = link.getAgeInfo()
                if not info:
                    continue
                ageName = info.getAgeFilename()
                #PtDebugPrint("found %s, looking for %s" % (ageName, age))
                if ageName == age:
                    return link

        return None


    def DoErcanaAndAhnonayStuff(self):
        agePanel = TargetAge.value
        if agePanel == "Ercana" or agePanel == "AhnonayCathedral" or agePanel == "Ahnonay":
            if agePanel == "Ercana":
                ageFileName = "Ercana"
                ageInstanceName = "Er'cana"
            elif agePanel == "AhnonayCathedral" or agePanel == "Ahnonay":
                ageFileName = "AhnonayCathedral"
                ageInstanceName = "Ahnonay Cathedral"
            self.CheckAndRegisterAge(ageFileName, ageInstanceName)
            self.FindOrCreateGUIDChron(ageFileName)


    def CheckAndRegisterAge(self, ageFileName, ageInstanceName):
        PtDebugPrint("CheckAndRegisterAge for: ",ageFileName,"; ",ageInstanceName)
        vault = ptVault()
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename(ageFileName)
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if not ageLinkNode:
            info = ptAgeInfoStruct()
            info.setAgeFilename(ageFileName)
            info.setAgeInstanceName(ageInstanceName)

            playerName = PtGetClientName()
            ageGuid = PtGuidGenerate()
            userDefName = ""
            desc = ""

            if playerName[-1] == "s" or playerName[-1] == "S":
                userDefName = "%s'" % playerName
                desc = "%s' %s" % (playerName, info.getAgeInstanceName())
            else:
                userDefName = "%s's" % playerName
                desc = "%s's %s" % (playerName, info.getAgeInstanceName())

            info.setAgeInstanceGuid(ageGuid)
            info.setAgeUserDefinedName(userDefName)
            info.setAgeDescription(desc)

            link = ptAgeLinkStruct()
            link.setAgeInfo(info)

            ptVault().registerOwnedAge(link)
            PtDebugPrint("Registered age - ", ageFileName)


    def FindOrCreateGUIDChron(self, ageFileName):
        PtDebugPrint("FindOrCreateGUIDChron for: ",ageFileName)
        GUIDChronFound = 0
        ageDataFolder = None
        
        vault = ptVault()
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename(ageFileName)
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            ageInfoChildren = ageInfoNode.getChildNodeRefList()
            for ageInfoChildRef in ageInfoChildren:
                ageInfoChild = ageInfoChildRef.getChild()
                folder = ageInfoChild.upcastToFolderNode()
                if folder and folder.folderGetName() == "AgeData":
                    ageDataFolder = folder
                    ageDataChildren = folder.getChildNodeRefList()
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "PelletCaveGUID":
                            GUIDChronFound = 1
                            PtDebugPrint("found pellet cave GUID: ", chron.getValue())
                            return
                    #return

        pelletCaveGUID = ""
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("PelletBahroCave")
        ageLinkNode = vault.getOwnedAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            pelletCaveGUID = ageInfoNode.getAgeInstanceGuid()
            PtDebugPrint("found pelletCaveGUID age chron, = ",pelletCaveGUID)
        
        if not ageDataFolder:
            PtDebugPrint("no ageDataFolder...")
            ageStruct = ptAgeInfoStruct()
            ageStruct.setAgeFilename(ageFileName)
            ageLinkNode = vault.getOwnedAgeLink(ageStruct)
            if ageLinkNode:
                PtDebugPrint("got ageLinkNode, created AgeData folder")
                ageInfoNode = ageLinkNode.getAgeInfo()
                ageDataFolder = ptVaultFolderNode(0)
                ageDataFolder.folderSetName("AgeData")
                ageInfoNode.addNode(ageDataFolder)

        if not GUIDChronFound:
            PtDebugPrint("creating PelletCave GUID chron")
            newNode = ptVaultChronicleNode(0)
            newNode.chronicleSetName("PelletCaveGUID")
            newNode.chronicleSetValue(pelletCaveGUID)
            ageDataFolder.addNode(newNode)
            PtDebugPrint("created pelletCaveGUID age chron, = ",pelletCaveGUID)


    def OnClickToLinkToPelletCaveFromErcana(self):
        ageVault = ptAgeVault()
        ageInfoNode = ageVault.getAgeInfo()

        ageInfoChildren = ageInfoNode.getChildNodeRefList()
        for ageInfoChildRef in ageInfoChildren:
            ageInfoChild = ageInfoChildRef.getChild()
            folder = ageInfoChild.upcastToFolderNode()
            if folder and folder.folderGetName() == "AgeData":
                ageDataChildren = folder.getChildNodeRefList()
                for ageDataChildRef in ageDataChildren:
                    ageDataChild = ageDataChildRef.getChild()
                    chron = ageDataChild.upcastToChronicleNode()
                    if chron and chron.getName() == "PelletCaveGUID":
                        PtDebugPrint("Found pellet cave guid - ", chron.getValue())
                        return chron.getValue()
                    return ""


    def OnClickToLinkToPelletCaveFromAhnonay(self):
        ageStruct = ptAgeInfoStruct()
        ageStruct.setAgeFilename("AhnonayCathedral")
        vault = ptAgeVault()
        ageLinkNode = vault.getSubAgeLink(ageStruct)
        if ageLinkNode:
            ageInfoNode = ageLinkNode.getAgeInfo()
            ageInfoChildren = ageInfoNode.getChildNodeRefList()
            for ageInfoChildRef in ageInfoChildren:
                ageInfoChild = ageInfoChildRef.getChild()
                folder = ageInfoChild.upcastToFolderNode()
                if folder and folder.folderGetName() == "AgeData":
                    ageDataChildren = folder.getChildNodeRefList()
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "PelletCaveGUID":
                            PtDebugPrint("Found pellet cave guid - ", chron.getValue())
                            return chron.getValue()
                    return ""


    def LinkToPelletCave(self,spawnPt):
        caveInfo = ptAgeInfoStruct()
        caveInfo.setAgeFilename("PelletBahroCave")
        caveInfo.setAgeInstanceName("PelletBahroCave")
        caveInfo.setAgeInstanceGuid(pelletCaveGUID)
        caveLink = ptAgeLinkStruct()
        caveLink.setAgeInfo(caveInfo)
        caveInfo = caveLink.getAgeInfo()
        caveInstance = caveInfo
        if caveInstance is None:
            PtDebugPrint("pellet cave instance is none, aborting link")
            return;
        info = ptAgeInfoStruct()
        info.copyFrom(caveInstance)
        als = ptAgeLinkStruct()
        spawnPoint = ptSpawnPointInfo()
        if spawnPt == "upper":
            spawnPoint.setName("LinkInPointDefault")
        elif spawnPt == "lower":
            spawnPoint.setName("LinkInPointLower")
        als.setAgeInfo(info)
        als.setSpawnPoint(spawnPoint)
        als.setLinkingRules(PtLinkingRules.kBasicLink)
        PtDebugPrint("-- linking to pellet cave --")
        linkMgr = ptNetLinkingMgr()
        linkMgr.linkToAge(als)


