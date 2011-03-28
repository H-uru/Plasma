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
Module: psnlBookshelf
Age: Personal Age
Date: August 2002
Author: Bill Slease
This is the handler for the standard personal age bookshelf
Interfaces with xLinkingBookGUIPopup.py
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
import PlasmaControlKeys
import xLinkingBookDefs
from xPsnlVaultSDL import *

import xVisitorUtils #For non-subscription based players

# define the attributes that will be entered in max
#PALGUI = ptAttribGUIDialog(2,"The PAL GUI")
actBookshelf = ptAttribActivator(3, "Actvtr:Bookshelf")

actBook = ptAttribActivator(4,"Actvtr:Book",byObject=1)
respPresentBook = ptAttribResponder(5,"Rspndr:PresentBook",byObject=1)
respShelveBook = ptAttribResponder(6,"Rspndr:ShelveBook",byObject=1)
objLibrary = ptAttribSceneobjectList(7,"Objct:Books")

objTrays = ptAttribSceneobjectList(8,"Objct:Trays")
respDeleteBook = ptAttribResponder(9,"Rspndr:DeleteBook",byObject=1)
respReturnTray = ptAttribResponder(10,"Rspndr:ReturnTray",byObject=1)
actTray = ptAttribActivator(11,"Actvtr:Tray",byObject=1)

objLocks = ptAttribSceneobjectList(12,"Objct:Locks")
respOpenLock = ptAttribResponder(13,"Rspndr:OpenLock",byObject=1)
respCloseLock = ptAttribResponder(14,"Rspndr:CloseLock",byObject=1)
actLock = ptAttribActivator(15,"Actvtr:Lock",byObject=1)

animLockOpen = ptAttribAnimation(16,"open clasp anim",byObject=1)
animLockClose = ptAttribAnimation(17,"close clasp anim",byObject=1)

SeekBehavior = ptAttribBehavior(18, "Smart seek before GUI") # used to make user walk in front of shelf before using it
ShelfCamera = ptAttribSceneobject(19,"Bookshelf camera") # the camera used when engaging the shelf
respRaiseShelfClickable = ptAttribResponder(20,"Rspndr:Raise Clickable (LocalOnly)", netForce=0) # Bill's sneaky way to: 1) engage the bookshelf, and 2) keep others from using a shelf already in use by making it's movement "LocalOnly" in Maxs user properties
respLowerShelfClickable = ptAttribResponder(21,"Rspndr:Lower Clickable") #undoes the damage in previous step
actDisengageShelf = ptAttribActivator(22,"Actvtr: Disengage Shelf") # region detector around the SeekBehavior node (#18 above) which detects when a player walks away from the shelf. Only disengages if "exiter" is the current user
HutCamera = ptAttribSceneobject(23,"Hut circle camera") # the camera which was used before engaging the shelf

actLinkingBookGUIPopup = ptAttribNamedActivator(24, "Actvr: LinkingBook GUI") # incoming notifies from the open Linking Book GUI

actBookshelfExit = ptAttribActivator(25, "Actvr: Exit bookshelf")

# globals
#==============

# this array defines which age books are on this shelf and where on the shelf they appear
# to add a book, replace an element with the name of the age, for instance: change Link11 to BillsSuperCoolAge
# to change where a book appears on the shelf, change it's position in the array
linkLibrary = ["Neighborhood","Nexus","city","Link03","Link04","Cleft","Garrison","Teledahn","Kadish","Gira","Garden","Negilahn","Dereno","Payiferen","Tetsonot","Ercana","AhnonayCathedral","Ahnonay","Minkata","Jalak","Link21","Link22","Link23","Link24","Link25","Link26","Link27","Link28","Link29","Link30","Link31","Link32","Link33","Link34","Link35","Myst"]

objBookPicked = None
objLockPicked = None
boolLinkerIsMe = false
boolPresentAfterLockOpen = false
boolShelfBusy = false
SpawnPointName = None
SpawnPointTitle = None
LocalAvatar = None
ShelfAUserID = -1
ShelfABoolOperated = 0
miniKIrestore = 0
boolShelfInUse = 0
AgeStartedIn = None
IsChildLink = 0

stupidHackForLock = None

kPublicBooks = ("Nexus", "Cleft", "City") #These books cannot be linked to by guests, and cannot be deleted by the owner

# list of ages that show up in the city book
CityBookAges = { "BaronCityOffice": ["BaronCityOffice", "Default"], "Descent": ["dsntShaftFall"], "GreatZero": ["grtzGrtZeroLinkRm"], "spyroom": ["Spyroom", "Default"], "Kveer": ["Kveer", "Default"]}


class psnlBookshelf(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5012

        version = 10
        self.version = version
        print "__init__psnlBookshelf v.", version
        PtLoadDialog(xVisitorUtils.kVisitorNagDialog)


    def __del__(self):
        PtUnloadDialog(xVisitorUtils.kVisitorNagDialog)


    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()

        self.initComplete = 0
        self.UsingBook = 0
        

        global boolShelfBusy
        
        # hide tray clickables
        for tray in objTrays.value:
            tray.draw.disable()
        
        #make all books invisible (tagged unclickable by default) then IUpdate to draw/enable appropriate books
        for book in objLibrary.value:
            book.draw.disable()
        boolShelfBusy = false

        # this section moved to OnServerInitComplete
        #####
        #self.IUpdateLocksAndTrays()
        ##self.IUpdateLinks()
        
        ## if I'm a visitor to this personal age, don't let me mess with book locks or delete books
        #vault = ptVault()
        #if not ( vault.inMyPersonalAge() ):
        #    actLock.disable()
        #    actTray.disable()
        #####

        return


    def OnServerInitComplete(self):
        global stupidHackForLock

        ageVault = ptAgeVault()
        ageInfoNode = ageVault.getAgeInfo()
        locked = 1

        ageInfoChildren = ageInfoNode.getChildNodeRefList()
        for ageInfoChildRef in ageInfoChildren:
            ageInfoChild = ageInfoChildRef.getChild()
            folder = ageInfoChild.upcastToFolderNode()
            if folder and folder.folderGetName() == "AgeData":
                ageDataChildren = folder.getChildNodeRefList()
                for ageDataChildRef in ageDataChildren:
                    ageDataChild = ageDataChildRef.getChild()
                    chron = ageDataChild.upcastToChronicleNode()
                    if chron and chron.getName() == "AhnonayLocked":
                        locked = bool(int(chron.getValue()))
                break

        vault = ptVault()
        if not vault.inMyPersonalAge():
            if stupidHackForLock == None:
                stupidHackForLock = locked

        self.IUpdateLocksAndTrays()
        #self.IUpdateLinks()
        
        # if I'm a visitor to this personal age, don't let me mess with book locks or delete books
        if not ( vault.inMyPersonalAge() ):
            actLock.disable()
            actTray.disable()

        solo = true
        if len(PtGetPlayerList()):
            solo = false

        self.IUpdateLinks()

        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL.setNotify(self.key, "ShelfAUserID", 0.0)
        
            ageSDL.setFlags("ShelfABoolOperated",1,1)
            ageSDL.setFlags("ShelfAUserID",1,1)
        
            ageSDL.sendToClients("ShelfABoolOperated")
            ageSDL.sendToClients("ShelfAUserID")
        
            ageSDL.setFlags("CurrentPage", 1,1)
            ageSDL.sendToClients("CurrentPage")

            ShelfABoolOperated = ageSDL["ShelfABoolOperated"][0]
            if not solo and ShelfABoolOperated:
                actBookshelf.disable()
                print "psnlBookshelf.Load():\tShelfABoolOperated=%d, disabling shelf clickable" % ShelfABoolOperated
            else:
                print "psnlBookshelf.Load():\tShelfABoolOperated=%d but no one else here...correcting" % ShelfABoolOperated
                self.IResetShelf()

        self.initComplete = 1


    def AvatarPage(self, avObj, pageIn, lastOut):
        "reset Shelf A accessibility if Shelf A user quits or crashes"
        global boolScopeOperated
        
        if pageIn:
            return
            
        avID = PtGetClientIDFromAvatarKey(avObj.getKey())
        
        if AgeStartedIn == PtGetAgeName():
            try:
                ageSDL = PtGetAgeSDL()
                if avID == ageSDL["ShelfAUserID"][0]:
                    print "psnlBookshelf.AvatarPage(): Bookshelf A operator paged out, reenabled Bookshelf."
                    self.IResetShelf()            
                else:
                    return
            except:
                # probably couldn't find the age SDL yet..
                pass


    def OnAgeVaultEvent(self,event,tupdata):
        PtDebugPrint("psnlBookshelf.OnAgeVaultEvent()\t:OnAgeKIEvent recvd. Event=%d and data= " % (event),tupdata)
        if event == PtVaultCallbackTypes.kVaultConnected:
            print "psnlBookshelf: kVaultConnected event"
            # tupdata is ()
            #~ pass
        elif event == PtVaultCallbackTypes.kVaultNodeSaved:
            print "psnlBookshelf.OnAgeVaultEvent()\t: kVaultNodeSaved event (id=%d,type=%d)" % (tupdata[0].getID(),tupdata[0].getType())
            # tupdata is ( ptVaultNode )
            #~ pass
        elif event == PtVaultCallbackTypes.kVaultNodeRefAdded:
            print "psnlBookshelf.OnAgeVaultEvent()\t: kVaultNodeRefAdded event (childID=%d,parentID=%d)" % (tupdata[0].getChildID(),tupdata[0].getParentID())
            # tupdata is ( ptVaultNodeRef )
            if self.initComplete:
                self.IUpdateLinks()
                self.IUpdateLocksAndTrays()
        elif event == PtVaultCallbackTypes.kVaultRemovingNodeRef:
            print "psnlBookshelf.OnAgeVaultEvent()\t: kVaultRemovingNodeRef event (childID=%d,parentID=%d)" % (tupdata[0].getChildID(),tupdata[0].getParentID())
            # tupdata is ( ptVaultNodeRef )
            #~ pass
        elif event == PtVaultCallbackTypes.kVaultNodeRefRemoved:
            print "psnlBookshelf.OnAgeVaultEvent()\t: kVaultNodeRefRemoved event (childID,parentID) ",tupdata 
            # tupdata is ( childID, parentID )
            #~ pass
        elif event == PtVaultCallbackTypes.kVaultNodeInitialized:
            print "psnlBookshelf.OnAgeVaultEvent()\t: kVaultNodeInitialized event (id=%d,type=%d)", (tupdata[0].getID(),tupdata[0].getType())
            # tupdata is ( ptVaultNode )
            #~ pass
        elif event == PtVaultCallbackTypes.kVaultOperationFailed:
            print "psnlBookshelf.OnAgeVaultEvent()\t: kVaultOperationFailed event  (operation,resultCode) ",tupdata
            #tupdata is ( operation, resultCode )
            #~ pass
        else:
            PtDebugPrint("psnlBookshelf: OnAgeVaultEvent - unknown event!")

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname == "ShelfAUserID":
            ageSDL = PtGetAgeSDL()
            if type(ageSDL) != type(None):
                if ageSDL["ShelfAUserID"][0] == -1:
                    actBookshelf.enable()

    def OnNotify(self,state,id,events):
        global objBookPicked
        global boolLinkerIsMe
        global objLockPicked
        global boolPresentAfterLockOpen
        global boolShelfBusy
        global ShelfAUserID
        global ShelfABoolOperated
        global SpawnPointName
        global SpawnPointTitle
        global miniKIrestore
        global boolShelfInUse
        global stupidHackForLock
        
        #~ print "state:",state," id:",id," events:",events

        if id == actBookshelfExit.id:
            self.IDisengageShelf()
            return
        
        vault = ptVault()
        
        if id == SeekBehavior.id and PtGetLocalAvatar() == PtFindAvatar(events):
            for event in events:
                if event[0] == kMultiStageEvent and event[1] == 0: # Smart seek completed. Exit multistage, and show GUI.
                    LocalAvatar = PtFindAvatar(events)
                    SeekBehavior.gotoStage(LocalAvatar, -1) 
                    print "psnlBookshelf.OnNotify():\tengaging bookshelf"

                    #PtFadeLocalAvatar(1)
                    LocalAvatar.draw.disable()
                    # set camera to Shelf Camera
                    virtCam = ptCamera()
                    virtCam.save(ShelfCamera.sceneobject.getKey())

                    if PtIsDialogLoaded("KIMini"):
                        miniKI = PtGetDialogFromString("KIMini")
                        if miniKI.isEnabled():
                            miniKIrestore = 1
                            PtHideDialog("KIMini")

                    PtAtTimeCallback(self.key, .5, 1)
                    #PtGetControlEvents(true,self.key)

##        # We don't care about this any more, it is being handled using control key events
##        if id == actDisengageShelf.id:
##            theavatar = PtFindAvatar(events)
##            AvatarWhoWalkedAway = PtGetClientIDFromAvatarKey(theavatar.getKey())
##            #~ print "psnlBookshelf.OnNotify: Avatar %s walked away from the shelf." % (AvatarWhoWalkedAway)
##
##            if AgeStartedIn == PtGetAgeName():
##                ageSDL = PtGetAgeSDL()
##                CurrentBookshelfUser = ageSDL["ShelfAUserID"][0]
##                #~ print "psnlBookshelf.OnNotify: Player %s was previously using the shelf." % (CurrentBookshelfUser)
##    
##                if AvatarWhoWalkedAway == CurrentBookshelfUser:
##                    PtDebugPrint ("psnlBookshelf.OnNotify: Player %s is done with the bookshelf." % (CurrentBookshelfUser))
##                    self.IResetShelf()             
##                    
##                    avatar = PtGetLocalAvatar()   
##                    myID = PtGetClientIDFromAvatarKey(avatar.getKey())
##        
##                    #~ print "I think my ID is: ", myID
##        
##                    if myID == AvatarWhoWalkedAway:
##                        print "I was the Shelf User, and I'm done with the Shelf now."
##        
##                        PtFadeLocalAvatar(0)
##                        #reeneable first person
##                        cam = ptCamera()
##                        cam.enableFirstPersonOverride()
##                        # go back to the Hut Circle Cam
##                        virtCam = ptCamera()
##                        virtCam.save(HutCamera.sceneobject.getKey())                
       
                
        if id == actLinkingBookGUIPopup.id:
            
            for event in events:
                if event[0] == kVariableEvent:
                    print "psnlBookshelf: Received a message from the Book GUI: ", event[1]
                    if event[1] == "IShelveBook" and type(objBookPicked) != type(None):
                        self.IShelveBook()
                        
                        
                    if event[1].split(",")[0] == "ILink": # parse the spawn point info off the entire note (which comes through as "ILink, SpawnPointName,SpawnPointTitle")
                        
                        LinkerID = event[3]
                        #~ print "LinkerID = ", LinkerID
                        avatar = PtGetLocalAvatar()   
                        myID = PtGetClientIDFromAvatarKey(avatar.getKey())
                        #~ print "myID = ", myID
                        self.IShelveBook()
                        if LinkerID == myID:
                            
                            #reenable First person before linking out
                            cam = ptCamera()
                            cam.enableFirstPersonOverride()
                            
                            #Un-fade local avatar
                            #PtFadeLocalAvatar(0)
                            avatar.draw.enable()
                            
                            #Go back to the hut circle cam to avoid re-fade out nastiness
                            virtCam = ptCamera()
                            virtCam.save(HutCamera.sceneobject.getKey())

                            # don't re-eable the movement keys when we are linking out...
                            ##PtEnableMovementKeys()
                            PtGetControlEvents(false,self.key)                            
                            
                            SpawnPointName = event[1].split(",")[1]
                            SpawnPointTitle = event[1].split(",")[2]
                            
                            print "psnlBookshelf: SpawnPointName = ", SpawnPointName," SpawnPointTitle = ", SpawnPointTitle

                            self.IResetShelf()
                            self.SendNote(0)
                            self.ILink()
                    
                    #~ stringAgeRequested = event[1]
                    #~ idRequestor = event[3]
                    #~ PtDebugPrint("xLinkingBookGUI.OnNotify():\tpsnlBookshelf user id %d selected book %s from the shelf" % (idRequestor, stringAgeRequested))
             
            #~ self.IShelveBook()
 
        if id == (-1):
            if events[0][1] == 'BookShelfBusy':
                print "psnlBookShelf: Notified about bookshelf use."
                boolShelfInUse = events[0][3]
            else:
                for event in events:
                    if event[0] == kVariableEvent:
                        print event[1], event[3]
                        if event[1] == "YesNo" and event[3] == 1:
                            link = self.IGetLinkFromBook()
                            
                            bookAge = self.IGetAgeFromBook()
                            if bookAge == "Ahnonay" or bookAge == "AhnonayCathedral":
                                ageVault = ptAgeVault()
                                ageInfoNode = ageVault.getAgeInfo()
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
                                            if chron and chron.getName() == "AhnonayVolatile":
                                                if ( vault.inMyPersonalAge() ):
                                                    chron.setValue("1")
                                        break

                                PtDebugPrint("DEBUG: psnlBookshelf.OnNotify:\tSending volatile notify (hopefully)")
                                note = ptNotify(self.key)
                                note.setActivate(1.0)
                                note.addVarNumber("VolatileAhnonay", 1)
                                note.send()

                                note = ptNotify(self.key)
                                note.setActivate(1.0)
                                note.addVarNumber("VolatileAhnonayCathedral", 1)
                                note.send()
                                
                                ageVault = ptAgeVault()
                                PAL = ageVault.getAgesIOwnFolder()
                                contents = PAL.getChildNodeRefList()
                                for content in contents:
                                    link = content.getChild()
                                    link = link.upcastToAgeLinkNode()
                                    info = link.getAgeInfo()
                                    if info and info.getAgeFilename() == "AhnonayCathedral":
                                        # found our link
                                        print "psnlBookshelf.IGetLinkFromBook():\tfound Owned link", info.getAgeFilename()
                                        link.setVolatile(True)
                                        link.save()

                                #bookName = objBookPicked.getName()
                                for bookName in ["ShelfA_book17","ShelfA_book18"]:
                                    for rkey,rvalue in respDeleteBook.byObject.items():
                                        parent = rvalue.getParentKey()
                                        if parent:
                                            if bookName == parent.getName():
                                                respDeleteBook.run(self.key,objectName=rkey)
                                                BookNumber = linkLibrary.index(bookAge)
                                                ageSDL = PtGetAgeSDL()                                    
                                                ageSDL.setIndex("CurrentPage",BookNumber,1)
                                                print "Setting CurrentPage var of book %s to 1" % BookNumber
                                                break
                                objBookPicked = None
                                return
                            else:
                                # volatile it
                                link.setVolatile(True)
                                link.save()

                            PtDebugPrint("DEBUG: psnlBookshelf.OnNotify:\tSending volatile notify (hopefully)")
                            note = ptNotify(self.key)
                            note.setActivate(1.0)
                            note.addVarNumber("Volatile" + bookAge, 1)
                            note.send()
                            bookName = objBookPicked.getName()
                            for rkey,rvalue in respDeleteBook.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if bookName == parent.getName():
                                        respDeleteBook.run(self.key,objectName=rkey)
                                        BookNumber = linkLibrary.index(bookAge)
                                        ageSDL = PtGetAgeSDL()                                    
                                        ageSDL.setIndex("CurrentPage",BookNumber,1)
                                        print "Setting CurrentPage var of book %s to 1" % BookNumber
                                        break
                            objBookPicked = None
                            return
                boolShelfBusy = false
                self.IUpdateLinks()


        # in no other cases do we want to take action on state = 0 events
        if not state:
            return

        if id==actBookshelf.id:
            if PtFindAvatar(events) == PtGetLocalAvatar() and PtWasLocallyNotified(self.key) and not boolShelfInUse:
                actBookshelf.disable() # want the Shelf clickable to be disabled for all clients
                print "psnlBookshelf: disabling clickable"
                print "psnlBookshelf: Firing clickable responder"
                respRaiseShelfClickable.run(self.key,netPropagate=0)
                self.SendNote(1)
                self.IUpdateLinks()
                for event in events:
                    if event[0] == kPickedEvent:
                        if event[1]: #entry event
                            # Disable First Person Camera
                            cam = ptCamera()
                            cam.undoFirstPerson()
                            cam.disableFirstPersonOverride()
                            PtRecenterCamera()
                            LocalAvatar = PtFindAvatar(events)
                            SeekBehavior.run(LocalAvatar)                        
                            #~ PtDebugPrint ("Bookshelf clicked. Disable it and smartseek.")
                            
                            ShelfABoolOperated = 1  # me! I'm the operator
                            if AgeStartedIn == PtGetAgeName():
                                ageSDL = PtGetAgeSDL()
                                ageSDL["ShelfABoolOperated"] = (1,)
                                avID = PtGetClientIDFromAvatarKey(LocalAvatar.getKey())
                                ageSDL["ShelfAUserID"] = (avID,)
                                ShelfAUserID = avID
                                print "psnlBookshelf.OnNotify:\twrote SDL - Bookshelf A user id = ", avID
                                PtDisableMovementKeys()
                            #self.IUpdateLinks()
                            #PtShowDialog(kPALDialogName)
                        #elif event[1] == 0: #exit event
                            #PtHideDialog(kPALDialogName)
                        break
                    
        if id==actBook.id:
            if PtWasLocallyNotified(self.key):
                boolLinkerIsMe = true

            ageSDL = PtGetAgeSDL()
            CurrentUser = ageSDL["ShelfAUserID"][0]
            avatar = PtGetLocalAvatar()
            myID = PtGetClientIDFromAvatarKey(avatar.getKey())

            #if myID != CurrentUser and boolLinkerIsMe:
            #    PtDebugPrint("DEBUG: psnlBookshelf.OnNotify: actBook notify, I'm not current user so I can't click books")
            #    return
            
            boolShelfBusy = true
            
            actTray.disable()
            actBook.disable()
            actLock.disable()
            
            # kPickedEvent looks like [ 2, pickflag, picker, picked ]  where picker and picked are SceneObject instances
            for event in events:
                if event[0]==kPickedEvent:
                    objBookPicked = event[3]
                    bookName = objBookPicked.getName()
                    print "psnlBookshelf.OnNotify():\tplayer picked book named ", bookName
                    try:
                        index = objLibrary.value.index(objBookPicked)
                    except:
                        print "psnlBookshelf.OnNotify():\tERROR -- couldn't find ", objBookPicked, " in objLibrary"
                        return

                    if self.IGetAgeFromBook() == "city" and PtIsSinglePlayerMode():
                        ageVault = ptAgeVault()
                        citylink = self.GetOwnedAgeLink(ageVault, "city")
                        bcolink = self.GetOwnedAgeLink(ageVault, "BaronCityOffice")
                        
                        citylinklocked = citylink and citylink.getLocked()
                        bcolinklocked = bcolink and bcolink.getLocked()

                        index = linkLibrary.index("city")
                        objLock = objLocks.value[index]
                        lockName = objLock.getName()

                        # show as locked if both are locked, or one is locked and the other doesn't exist
                        if ( type(citylinklocked) == type(None) or citylinklocked) and ( type(bcolinklocked) == type(None) or bcolinklocked):
                            # find lock associated with this book
                            objLockPicked = objLocks.value[index]
                            lockName = objLockPicked.getName()
                            # find the corresponding lock open responder modifier
                            for rkey,rvalue in respOpenLock.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if lockName == parent.getName():
                                        respOpenLock.run(self.key,objectName=rkey)
                                        break
                            boolPresentAfterLockOpen = true
                            break
                        else:
                            bookName = objBookPicked.getName()
                            # find the corresponding responder modifier and present the book to the player
                            for rkey,rvalue in respPresentBook.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if bookName == parent.getName():
                                        respPresentBook.run(self.key,objectName=rkey)
                                        break
                        self.UsingBook = 1
                        actBookshelfExit.disable()
                        return
                    
                    elif self.IGetAgeFromBook() == "Ahnonay":
                        locked = 0
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
                                    if chron and chron.getName() == "AhnonayLocked":
                                        locked = bool(int(chron.getValue()))

                        if not vault.inMyPersonalAge():
                            if stupidHackForLock != None:
                                locked = stupidHackForLock

                        if locked:
                            # find lock associated with this book
                            objLockPicked = objLocks.value[index]
                            lockName = objLockPicked.getName()
                            # find the corresponding lock open responder modifier
                            for rkey,rvalue in respOpenLock.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if lockName == parent.getName():
                                        respOpenLock.run(self.key,objectName=rkey)
                                        break
                            boolPresentAfterLockOpen = true
                            break
                        else:
                            bookName = objBookPicked.getName()
                            # find the corresponding responder modifier and present the book to the player
                            for rkey,rvalue in respPresentBook.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if bookName == parent.getName():
                                        respPresentBook.run(self.key,objectName=rkey)
                                        break
                        
                        self.UsingBook = 1
                        actBookshelfExit.disable()
                        return
                    
                    link = self.IGetLinkFromBook()
                    if type(link) == type(None):
                        return
                    if type(link) != type(ptVaultAgeLinkNode()) or link.getLocked():
                        # find lock associated with this book

                        objLockPicked = objLocks.value[index]
                        lockName = objLockPicked.getName()
                        # find the corresponding lock open responder modifier
                        for rkey,rvalue in respOpenLock.byObject.items():
                            parent = rvalue.getParentKey()
                            if parent:
                                if lockName == parent.getName():
                                    respOpenLock.run(self.key,objectName=rkey)
                                    break
                        boolPresentAfterLockOpen = true
                        break
                    else:
                        bookName = objBookPicked.getName()
                        # find the corresponding responder modifier and present the book to the player
                        for rkey,rvalue in respPresentBook.byObject.items():
                            parent = rvalue.getParentKey()
                            if parent:
                                if bookName == parent.getName():
                                    respPresentBook.run(self.key,objectName=rkey)
                                    break
            self.UsingBook = 1
            actBookshelfExit.disable()
            return
        
        if id==respPresentBook.id and type(objBookPicked) != type(None):
            # book is finished presenting - now link
            if boolLinkerIsMe:
                #~ self.ILink()
                # tell linking book GUI which age to present
                stringShowMeAge = self.IGetAgeFromBook()
                PtDebugPrint("psnlBookshelf.OnNotify():\tsend message - show client %d age %s" % (ShelfAUserID,stringShowMeAge) )

                note = ptNotify(self.key)
                note.setActivate(1.0)
                note.addVarNumber(stringShowMeAge + "," + str(objLibrary.value.index(objBookPicked)),ShelfAUserID)
                note.send()

            # for now, just shelve the book again
            #~ self.IShelveBook()
            return
            
        if id==respShelveBook.id:
            if self.IGetAgeFromBook() == "city" and PtIsSinglePlayerMode():
                ageVault = ptAgeVault()
                citylink = self.GetOwnedAgeLink(ageVault, "city")
                bcolink = self.GetOwnedAgeLink(ageVault, "BaronCityOffice")
                
                citylinklocked = citylink and citylink.getLocked()
                bcolinklocked = bcolink and bcolink.getLocked()

                index = linkLibrary.index("city")
                objLock = objLocks.value[index]
                lockName = objLock.getName()

                # show as locked if both are locked, or one is locked and the other doesn't exist
                if ( type(citylinklocked) == type(None) or citylinklocked) and ( type(bcolinklocked) == type(None) or bcolinklocked):
                    lockName = objLockPicked.getName()
                    # find the corresponding responder modifier
                    for rkey,rvalue in respCloseLock.byObject.items():
                        parent = rvalue.getParentKey()
                        if parent:
                            if lockName == parent.getName():
                                respCloseLock.run(self.key,objectName=rkey)
                                objLockPicked = None
                                break

                #~ elif not boolLinkerIsMe:
                else:
                    boolShelfBusy = false
                    self.IUpdateLinks()
                return
            link = self.IGetLinkFromBook()
            if type(link) == type(None):
                return
                
            if type(link) == type("") and link == "Ahnonay":
                locked = 0
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
                            if chron and chron.getName() == "AhnonayLocked":
                                locked = bool(int(chron.getValue()))
                        break

                if not vault.inMyPersonalAge():
                    if stupidHackForLock != None:
                        locked = stupidHackForLock

                if locked:
                    lockName = objLockPicked.getName()
                    print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting clasp to locked: ",lockName
                    for rkey,rvalue in respCloseLock.byObject.items():
                        parent = rvalue.getParentKey()
                        if parent:
                            if lockName == parent.getName():
                                respCloseLock.run(self.key,objectName=rkey)
                                objLockPicked = None
                                return
                else:
                    boolShelfBusy = false
                    self.IUpdateLinks()
                    return                    

            if type(link) == type(ptAgeLinkStruct()) or link.getLocked(): #close the clasp
                lockName = objLockPicked.getName()
                # find the corresponding responder modifier
                for rkey,rvalue in respCloseLock.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            respCloseLock.run(self.key,objectName=rkey)
                            objLockPicked = None
                            break

            #~ elif not boolLinkerIsMe:
            else:
                boolShelfBusy = false
                self.IUpdateLinks()
            return

        if id==actLock.id:
            boolShelfBusy = true
            
            actTray.disable()
            actBook.disable()
            actLock.disable()
            
            # kPickedEvent looks like [ 2, pickflag, picker, picked ]  where picker and picked are SceneObject instances
            for event in events:
                if event[0]==kPickedEvent:
                    objLockPicked = event[3]

                    try:
                        index = objLocks.value.index(objLockPicked)
                    except:
                        print "psnlBookshelf.OnNotify():\tERROR -- couldn't find ", objLockPicked, " in objLocks"
                        return
                    objBookPicked = objLibrary.value[index]
                    lockName = objLockPicked.getName()
                    agename = self.IGetAgeFromBook()

                    if agename == "city" and PtIsSinglePlayerMode():
                        agevault = ptAgeVault()
                        citylink = self.GetOwnedAgeLink(agevault, "city")
                        bcolink = self.GetOwnedAgeLink(agevault, "BaronCityOffice")

                        citylinklocked = citylink and citylink.getLocked()
                        bcolinklocked = bcolink and bcolink.getLocked()

                        if ( type(citylinklocked) == type(None) or citylinklocked) and ( type(bcolinklocked) == type(None) or bcolinklocked):
                            for rkey,rvalue in respOpenLock.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if lockName == parent.getName():
                                        respOpenLock.run(self.key,objectName=rkey)
                                        break

                            citylink and citylink.setLocked(0)
                            bcolink and bcolink.setLocked(0)
                        else:
                            for rkey,rvalue in respCloseLock.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if lockName == parent.getName():
                                        respCloseLock.run(self.key,objectName=rkey)
                                        break

                            citylink and citylink.setLocked(1)
                            bcolink and bcolink.setLocked(1)
                                
                        if ( vault.inMyPersonalAge() ):
                            citylink and citylink.save()
                            bcolink and bcolink.save()
                        
                        return
                        
                    link = self.IGetLinkFromBook()
                    if type(link) == type(None):
                        return
                    lockName = objLockPicked.getName()
                    
                    if type(link) == type("") and link == "Ahnonay":
                        ageVault = ptAgeVault()
                        ageInfoNode = ageVault.getAgeInfo()
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
                                    if chron and chron.getName() == "AhnonayLocked":
                                        locked = bool(int(chron.getValue()))
                                break

                        if not vault.inMyPersonalAge():
                            if stupidHackForLock != None:
                                locked = stupidHackForLock

                        print locked
                        if locked:
                            # find the corresponding open clasp responder modifier
                            for rkey,rvalue in respOpenLock.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if lockName == parent.getName():
                                        respOpenLock.run(self.key,objectName=rkey)
                                        break
                            for ageDataChildRef in ageDataChildren:
                                ageDataChild = ageDataChildRef.getChild()
                                chron = ageDataChild.upcastToChronicleNode()
                                if chron and chron.getName() == "AhnonayLocked":
                                    if ( vault.inMyPersonalAge() ):
                                        print "setting lock to 0"
                                        chron.setValue("0")
                        else:
                            # find the corresponding close clasp responder modifier
                            for rkey,rvalue in respCloseLock.byObject.items():
                                parent = rvalue.getParentKey()
                                if parent:
                                    if lockName == parent.getName():
                                        respCloseLock.run(self.key,objectName=rkey)
                                        break
                            for ageDataChildRef in ageDataChildren:
                                ageDataChild = ageDataChildRef.getChild()
                                chron = ageDataChild.upcastToChronicleNode()
                                if chron and chron.getName() == "AhnonayLocked":
                                    if ( vault.inMyPersonalAge() ):
                                        print "setting lock to 1"
                                        chron.setValue("1")

                        if not vault.inMyPersonalAge():
                            stupidHackForLock = not stupidHackForLock

                        return

                    if type(link) != type(ptVaultAgeLinkNode()):
                        self.IUpdateLinks()
                        return
                    if link.getLocked():
                        # find the corresponding open clasp responder modifier
                        for rkey,rvalue in respOpenLock.byObject.items():
                            parent = rvalue.getParentKey()
                            if parent:
                                if lockName == parent.getName():
                                    respOpenLock.run(self.key,objectName=rkey)
                                    break
                        link.setLocked(False)
                        if ( vault.inMyPersonalAge() ):
                            link.save()
                    else:
                        # find the corresponding close clasp responder modifier
                        for rkey,rvalue in respCloseLock.byObject.items():
                            parent = rvalue.getParentKey()
                            if parent:
                                if lockName == parent.getName():
                                    respCloseLock.run(self.key,objectName=rkey)
                                    break
                        link.setLocked(True)
                        if ( vault.inMyPersonalAge() ):
                            link.save()
                    break
            return
                    
        if id==respOpenLock.id:
            if boolPresentAfterLockOpen:
                bookName = objBookPicked.getName()
                # find the corresponding responder modifier and present the book to the player
                for rkey,rvalue in respPresentBook.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if bookName == parent.getName():
                            respPresentBook.run(self.key,objectName=rkey)
                            break
                boolPresentAfterLockOpen = false
            else:
                boolShelfBusy=false
                self.IUpdateLinks()
            return
        
        if id==respCloseLock.id:
            #~ if not boolLinkerIsMe:
            boolShelfBusy = false
            self.IUpdateLinks() # after someone links, need to run this to reenable clickables
            return

#
#
#    Feb 2003 converted delete functionality to mark volatilenessticity
#
#
        if id==actTray.id:
            boolShelfBusy = true
            actTray.disable()
            actBook.disable()
            actLock.disable()

            # kPickedEvent looks like [ 2, pickflag, picker, picked ]  where picker and picked are SceneObject instances
            for event in events:
                if event[0]==kPickedEvent:
                    objTrayPicked = event[3]
                    break                    

            # find the corresponding responder modifier and begin deletion
            # responders and animations are on books who are parents of the trays...
            try:
                index = objTrays.value.index(objTrayPicked)
            except:
                print "psnlBookshelf.OnNotify():\tERROR -- couldn't find ", objTrayPicked, " in objTrays"
                return

            objBookPicked = objLibrary.value[index]
            objLockPicked = objLocks.value[index]

            link = self.IGetLinkFromBook()
            if not link:
                return

            bookAge = self.IGetAgeFromBook()

            if bookAge == "Ahnonay" or bookAge == "AhnonayCathedral":
                ageVault = ptAgeVault()
                ageInfoNode = ageVault.getAgeInfo()
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
                            if chron and chron.getName() == "AhnonayVolatile":
                                volatile = chron.getValue()
                        break

                if int(volatile):
                    for ageDataChildRef in ageDataChildren:
                        ageDataChild = ageDataChildRef.getChild()
                        chron = ageDataChild.upcastToChronicleNode()
                        if chron and chron.getName() == "AhnonayVolatile":
                            if ( vault.inMyPersonalAge() ):
                                chron.setValue("0")
                                break

                    PtDebugPrint("DEBUG: psnlBookshelf.OnNotify:\tSending notvolatile notify (hopefully)")
                    note = ptNotify(self.key)
                    note.setActivate(1.0)
                    note.addVarNumber("NotVolatileAhnonay", 1)
                    note.send()

                    note = ptNotify(self.key)
                    note.setActivate(1.0)
                    note.addVarNumber("NotVolatileAhnonayCathedral", 1)
                    note.send()

                    ageVault = ptAgeVault()
                    PAL = ageVault.getAgesIOwnFolder()
                    contents = PAL.getChildNodeRefList()
                    for content in contents:
                        link = content.getChild()
                        link = link.upcastToAgeLinkNode()
                        info = link.getAgeInfo()
                        if info and info.getAgeFilename() == "AhnonayCathedral":
                            # found our link
                            print "psnlBookshelf.IGetLinkFromBook():\tfound Owned link", info.getAgeFilename()
                            link.setVolatile(False)
                            link.save()

                    #bookName = objBookPicked.getName()
                    for bookName in ["ShelfA_book17","ShelfA_book18"]:
                        for rkey,rvalue in respReturnTray.byObject.items():
                            parent = rvalue.getParentKey()
                            if parent:
                                if bookName == parent.getName():
                                    respReturnTray.run(self.key,objectName=rkey)
                                    break
                    objBookPicked = None
                elif vault.inMyPersonalAge():
                    PtYesNoDialog(self.key, PtGetLocalizedString("Personal.Bookshelf.DeleteBook"))

                return

            if link.getVolatile():
                # unvolatile it
                link.setVolatile(False)
                link.save()

                PtDebugPrint("DEBUG: psnlBookshelf.OnNotify:\tSending notvolatile notify (hopefully)")
                note = ptNotify(self.key)
                note.setActivate(1.0)
                note.addVarNumber("NotVolatile" + bookAge, 1)
                note.send()

                bookName = objBookPicked.getName()
                for rkey,rvalue in respReturnTray.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if bookName == parent.getName():
                            respReturnTray.run(self.key,objectName=rkey)
                            break
                objBookPicked = None
            elif vault.inMyPersonalAge():
                if bookAge == "Neighborhood":
                    #Don't allow visitors (i.e. non-subscribers) to delete their neighborhood
                    if not PtIsSubscriptionActive():
                        PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                        actTray.enable()
                        actBook.enable()
                        actLock.enable()
                        return
                    
                    PtYesNoDialog(self.key, PtGetLocalizedString("Personal.Bookshelf.DeleteNeighborhoodBook"))
                else:
                    PtYesNoDialog(self.key, PtGetLocalizedString("Personal.Bookshelf.DeleteBook"))
##                # volatile it
##                link.setVolatile(True)
##                link.save()
##
##                bookAge = self.IGetAgeFromBook()
##
##                PtDebugPrint("DEBUG: psnlBookshelf.OnNotify:\tSending volatile notify (hopefully)")
##                note = ptNotify(self.key)
##                note.setActivate(1.0)
##                note.addVarNumber("Volatile" + bookAge, 1)
##                note.send()
##
##                bookName = objBookPicked.getName()
##                for rkey,rvalue in respDeleteBook.byObject.items():
##                    parent = rvalue.getParentKey()
##                    if parent:
##                        if bookName == parent.getName():
##                            respDeleteBook.run(self.key,objectName=rkey)
##                            break

        if id==respReturnTray.id or id==respDeleteBook.id:
            boolShelfBusy = false
            self.IUpdateLinks() 
            return


    def IGetLinkFromBook(self, spTitle = None):
        "returns link element associated with global objBookPicked or None"
        global CityBookAges
        global IsChildLink
        
        ageName = self.IGetAgeFromBook()
        print "psnlBookshelf.IGetLinkFromBook(): before city lookup, ageName = ",ageName

        isCityLink = 0
        if ageName == "city":
            isCityLink = 1
            for age, splist in CityBookAges.items():
                if spTitle in splist:
                    ageName = age
                    break
        print "psnlBookshelf.IGetLinkFromBook(): after city lookup, ageName = ",ageName        	        

        if type(ageName) == type(None):
            print "psnlBookshelf.IGetLinkFromBook():\tERROR -- conversion from book to link element failed"
            return None

        if ageName == "Ahnonay":
            print "psnlBookshelf.IGetLinkFromBook(): Going to Ahnonay... special case."
            return "Ahnonay"

        hoodInfo = self.IGetHoodInfoNode()
        if hoodInfo:
            childAgeFolder = hoodInfo.getChildAgesFolder()
            contents = childAgeFolder.getChildNodeRefList()
            for content in contents:
                link = content.getChild()
                link = link.upcastToAgeLinkNode()
                info = link.getAgeInfo()
                if info and info.getAgeFilename() == ageName:
                    if ageName == "Garrison":
                        continue
                    else:
                        # found our link
                        print "psnlBookshelf.IGetLinkFromBook():\tfound Child link", info.getAgeFilename()
                        IsChildLink = 1
                        return link

        if isCityLink:
            # if we got here then we're a city link but we couldn't find the child age
            # so we're going to hack it a little bit and force it
            IsChildLink = 1

            ageInfo = ptAgeInfoStruct()
            ageInfo.setAgeFilename(ageName)
            ageInfo.setAgeInstanceName("Ae'gura")

            ageLink = ptAgeLinkStruct()
            ageLink.setAgeInfo(ageInfo)
            
            return ageLink

        ageVault = ptAgeVault()
        PAL = ageVault.getAgesIOwnFolder()
        contents = PAL.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            info = link.getAgeInfo()
            if info and info.getAgeFilename() == ageName:
                # found our link
                print "psnlBookshelf.IGetLinkFromBook():\tfound Owned link", info.getAgeFilename()
                IsChildLink = 0
                return link

        print "psnlBookshelf.IGetLinkFromBook():\tERROR -- couldn't find link to", ageName
        print "info = ",info
        print "info.getAgeFilename() = ",info.getAgeFilename()
        print "spTitle = ",spTitle
        return None
    
    def SendNote(self, bool):
        notify = ptNotify(self.key)
        notify.clearReceivers()                
        notify.addReceiver(self.key)        
        notify.netPropagate(1)
        notify.netForce(1)
        notify.setActivate(1.0)
        notify.addVarNumber('BookShelfBusy',bool)
        notify.send()

    def IUpdateLocksAndTrays(self):
        global CityBookAges
        
        ageVault = ptAgeVault()
        PAL = ageVault.getAgesIOwnFolder()
        contents = PAL.getChildNodeRefList()

        # check for the dang city book and do stuff
        if self.HasCityBook():
            citylink = self.GetOwnedAgeLink(ageVault, "city")
            #bcolink = self.GetOwnedAgeLink(ageVault, "BaronCityOffice")
            bcolink = self.IGetHoodChildLink("BaronCityOffice")
            
            citylinklocked = citylink and citylink.getLocked()
            bcolinklocked = bcolink and bcolink.getLocked()

            index = linkLibrary.index("city")
            objLock = objLocks.value[index]
            lockName = objLock.getName()

            # show as locked if both are locked, or one is locked and the other doesn't exist
            if ( type(citylinklocked) == type(None) or citylinklocked) and ( type(bcolinklocked) == type(None) or bcolinklocked):
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting clasp to locked: ",lockName
                for rkey,rvalue in respCloseLock.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            respCloseLock.run(self.key,objectName=rkey,fastforward=1)
            else:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting clasp to unlocked: ",lockName
                for rkey,rvalue in respOpenLock.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            respOpenLock.run(self.key,objectName=rkey,fastforward=1)
        
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            info = link.getAgeInfo()
            if not info:
                continue
            ageName = info.getAgeFilename()
            try:
                index = linkLibrary.index(ageName)    
            except:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tno matching book for KI's link to:", ageName, "...skipping to next"
                continue

            if ((ageName == "city" or ageName == "BaronCityOffice") and PtIsSinglePlayerMode()) or (ageName in CityBookAges.keys()):
                continue

            if ageName == "Cleft":
                if not link.getLocked():
                    link.setLocked(True)
                    vault = ptVault()
                    if vault.inMyPersonalAge():
                        link.save()
                continue

                
            objLock = objLocks.value[index]
            lockName = objLock.getName()
            if link.getLocked():
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting clasp to locked: ",lockName
                for rkey,rvalue in respCloseLock.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            respCloseLock.run(self.key,objectName=rkey,fastforward=1)
            else:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting clasp to unlocked: ",lockName
                for rkey,rvalue in respOpenLock.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            respOpenLock.run(self.key,objectName=rkey,fastforward=1)
                        
            # trays are children of the books...so to manipulate the trays we manipulate the books
            objBook = objLibrary.value[index]
            bookName = objBook.getName()
            if link.getVolatile():
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting booktray to VOLATILE: ",bookName
                for rkey,rvalue in respDeleteBook.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        #~ print trayName,"==",rkey#[:len(trayName)]
                        if bookName == parent.getName():
                            respDeleteBook.run(self.key,objectName=rkey,fastforward=1)
                            #~ print "got here"
            else:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting booktray to NOT volatile: ",bookName
                for rkey,rvalue in respReturnTray.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if bookName == parent.getName():
                            respReturnTray.run(self.key,objectName=rkey,fastforward=1)

        ## Ahnonay Hackage!
        guid = None
        locked = 0
        volatile = 0
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
                    if chron and chron.getName() == "AhnonayLink":
                        guid = chron.getValue()
                    elif chron and chron.getName() == "AhnonayLocked":
                        locked = chron.getValue()
                    elif chron and chron.getName() == "AhnonayVolatile":
                        volatile = chron.getValue()

        if guid != None:
            try:
                index = linkLibrary.index("Ahnonay")    
            except:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tno matching book for KI's link to: Ahnonay... skipping to next"
                return

            objLock = objLocks.value[index]
            lockName = objLock.getName()
            if int(locked):
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting clasp to locked: ",lockName
                for rkey,rvalue in respCloseLock.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            respCloseLock.run(self.key,objectName=rkey,fastforward=1)
            else:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting clasp to unlocked: ",lockName
                for rkey,rvalue in respOpenLock.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            respOpenLock.run(self.key,objectName=rkey,fastforward=1)
                        
            # trays are children of the books...so to manipulate the trays we manipulate the books
            objBook = objLibrary.value[index]
            bookName = objBook.getName()
            if int(volatile):
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting booktray to VOLATILE: ",bookName
                for rkey,rvalue in respDeleteBook.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        #~ print trayName,"==",rkey#[:len(trayName)]
                        if bookName == parent.getName():
                            respDeleteBook.run(self.key,objectName=rkey,fastforward=1)
                            #~ print "got here"
            else:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tsetting booktray to NOT volatile: ",bookName
                for rkey,rvalue in respReturnTray.byObject.items():
                    parent = rvalue.getParentKey()
                    if parent:
                        if bookName == parent.getName():
                            respReturnTray.run(self.key,objectName=rkey,fastforward=1)

        return


    def IUpdateLinks(self):
        global stupidHackForLock
        global CityBookAges

        # ensure all books and trays are not clickable
        #  we're going to display books player has links to and enable clickables on books and associated trays
        actTray.disable()
        actBook.disable()
        actLock.disable()
        
        vault = ptVault()
        boolInMyAge = vault.inMyPersonalAge()

        # check for the dang city book and do stuff
        if self.HasCityBook():
            ageVault = ptAgeVault()
            citylink = self.GetOwnedAgeLink(ageVault, "city")
            #bcolink = self.GetOwnedAgeLink(ageVault, "BaronCityOffice")
            bcolink = self.IGetHoodChildLink("BaronCityOffice")

            citylinklocked = citylink and citylink.getLocked()
            bcolinklocked = bcolink and bcolink.getLocked()

            index = linkLibrary.index("city")
            objBook = objLibrary.value[index]
            objBook.draw.enable()

            bookName = objBook.getName()
            for key,value in actBook.byObject.items():
                parent = value.getParentKey()
                if parent:
                    if bookName == parent.getName():
                        actBook.enable(objectName=key)
                        break

            # find and enable the corresponding clickable modifier for the book's tray and lock if owner
            objTray = objTrays.value[index]
            trayName = objTray.getName()
            objLock = objLocks.value[index]
            lockName = objLock.getName()
            if ( boolInMyAge ):
                for key,value in actLock.byObject.items():
                    parent = value.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            actLock.enable(objectName=key)
                            break
        
        ageVault = ptAgeVault()
        PAL = ageVault.getAgesIOwnFolder()
        if type(PAL) != type(None):
            contents = PAL.getChildNodeRefList()

            for content in contents:
                link = content.getChild()
                link = link.upcastToAgeLinkNode()
                info = link.getAgeInfo()
                if not info:
                    continue
                ageName = info.getAgeFilename()

                if (ageName == "city" or ageName == "BaronCityOffice") or (ageName in CityBookAges.keys()):
                    continue
            
#                if (ageName == "Garrison"):
#                    parent = info.getParentAgeLink()
#                    parentinfo = parent.getAgeInfo()
#                    parentname = parentinfo.getAgeFilename()
#                    if parentname == "Neighborhood":
#                        print "psnlBookshelf.IUpdateLinks(): link found for public Garrison, ignoring..."
#                        continue
                
                try:
                    index = linkLibrary.index(ageName)    
                except:
                    print "psnlBookshelf.IUpdateLinks():\tno matching book for KI's link to:", ageName, "...skipping to next"
                    continue
    
                if ageName == "Cleft":
                    ageSDL = PtGetAgeSDL()
                    boolCleftVisited = ageSDL["CleftVisited"][0]
                    if not boolCleftVisited:
                        continue

                # show the book
                objBook = objLibrary.value[index]
                objBook.draw.enable()
                
                if boolShelfBusy:
                    # not safe to enable clickables
                    return

                # find and enable the corresponding clickable modifier for the book
                print "psnlBookshelf.IUpdateLinks():\tageName: ",ageName," boolInMyAge: ",boolInMyAge," getLocked(): ",link.getLocked()," getVolatile(): ",link.getVolatile()
                if link.getVolatile() or ((not boolInMyAge) and (link.getLocked() or ageName == "Cleft")):
                    bookName = objBook.getName()
                    for key,value in actBook.byObject.items():
                        parent = value.getParentKey()
                        if parent:
                            if bookName == parent.getName():
                                print "%s book: DISABLED" % (bookName)
                                actBook.disable(objectName=key)
                                break
                    if (not boolInMyAge) and (link.getLocked() or ageName == "Cleft"):
                        # owner of book has locked this one -- go on to next link element
                        continue
                
                if not link.getVolatile():
                    bookName = objBook.getName()
                    for key,value in actBook.byObject.items():
                        parent = value.getParentKey()
                        if parent:
                            if bookName == parent.getName():
                                print "%s book: ENABLED" % (bookName)
                                actBook.enable(objectName=key)
                                break
    
                # find and enable the corresponding clickable modifier for the book's tray and lock if owner
                objTray = objTrays.value[index]
                trayName = objTray.getName()
                objLock = objLocks.value[index]
                lockName = objLock.getName()

                if ( boolInMyAge ):
                    #First make sure that we disable locks (i.e sharing) and trays (i.e. deletion) on all public books
                    if ageName in kPublicBooks:
                        continue
                

                    for key,value in actLock.byObject.items():
                        parent = value.getParentKey()
                        if parent:
                            if lockName == parent.getName():
                                actLock.enable(objectName=key)
                                break
                    
                    for key,value in actTray.byObject.items():
                        parent = value.getParentKey()
                        if parent:
                            if trayName == parent.getName():
                                actTray.enable(objectName=key)
                                break
                    
        else:
            print "psnlBookshelf: The PAL folder is missing"


        ## Ahnonay Hackage!
        guid = None
        locked = 0
        volatile = 0
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
                    if chron and chron.getName() == "AhnonayLink":
                        guid = chron.getValue()
                    elif chron and chron.getName() == "AhnonayLocked":
                        locked = bool(int(chron.getValue()))
                    elif chron and chron.getName() == "AhnonayVolatile":
                        volatile = chron.getValue()

        if guid != None:
            ageName = "Ahnonay"
            
            try:
                index = linkLibrary.index("Ahnonay")    
            except:
                print "psnlBookshelf.IUpdateLocksAndTrays():\tno matching book for KI's link to: Ahnonay... skipping to next"
                return

            # show the book
            objBook = objLibrary.value[index]
            objBook.draw.enable()
            
            if boolShelfBusy:
                # not safe to enable clickables
                return

            if not vault.inMyPersonalAge():
                if stupidHackForLock != None:
                    locked = stupidHackForLock

            # find and enable the corresponding clickable modifier for the book
            if int(volatile) or ((not boolInMyAge) and locked):
                bookName = objBook.getName()
                for key,value in actBook.byObject.items():
                    parent = value.getParentKey()
                    if parent:
                        if bookName == parent.getName():
                            print "%s book: DISABLED" % (bookName)
                            actBook.disable(objectName=key)
                            break
                if (not boolInMyAge) and int(locked):
                    # owner of book has locked this one -- go on to next link element
                    return

            if not int(volatile):
                bookName = objBook.getName()
                for key,value in actBook.byObject.items():
                    parent = value.getParentKey()
                    if parent:
                        if bookName == parent.getName():
                            print "%s book: ENABLED" % (bookName)
                            actBook.enable(objectName=key)
                            break

            # find and enable the corresponding clickable modifier for the book's tray and lock if owner
            objTray = objTrays.value[index]
            trayName = objTray.getName()
            objLock = objLocks.value[index]
            lockName = objLock.getName()

            if boolInMyAge:
                #First make sure that we disable locks (i.e sharing) and trays (i.e. deletion) on all public books
                if ageName in kPublicBooks:
                    return
            

                for key,value in actLock.byObject.items():
                    parent = value.getParentKey()
                    if parent:
                        if lockName == parent.getName():
                            actLock.enable(objectName=key)
                            break
                
                for key,value in actTray.byObject.items():
                    parent = value.getParentKey()
                    if parent:
                        if trayName == parent.getName():
                            actTray.enable(objectName=key)
                            break

    def CheckForCityBookSpawnPoint(self, agefilename, sptitle):
        global CityBookAges
        
        if agefilename in CityBookAges.keys():
            splist = CityBookAges[agefilename]
            if sptitle in splist:
                return 1
        return 0


    def ILink(self):
        global SpawnPointName
        global SpawnPointTitle
        
        link = self.IGetLinkFromBook(SpawnPointTitle)
        if type(link) == type(None):
            print "psnlBookshelf.ILink():\tERROR -- conversion from book to link failed -- aborting"
            return
        elif type(link) == type("") and link == "Ahnonay":
            info = ptAgeInfoStruct()
            info.setAgeFilename("Ahnonay")
            info.setAgeInstanceName("Ahnonay")
            guid = None
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
                        if chron and chron.getName() == "AhnonayLink":
                            guid = chron.getValue()
                            print guid
                            break
                    break
            info.setAgeInstanceGuid(guid)

            link = ptAgeLinkStruct()
            link.setAgeInfo(info)
            
        elif link.getAgeInfo().getAgeFilename() == "Neighborhood" and not PtIsSubscriptionActive():
            info = ptAgeInfoStruct()
            info.setAgeFilename("Neighborhood")
            info.setAgeInstanceGuid("366f9aa1-c4c9-4c4c-a23a-cbe6896cc3b9")
            link = ptAgeLinkStruct()
            link.setAgeInfo(info)

        info = link.getAgeInfo()
        ageName = info.getAgeFilename()

        # do the link
        print "psnlBookshelf.ILink():\tattempting link to %s(%s)" % (info.getAgeFilename(),info.getAgeInstanceName())

        print "Ilink: SpawnPointTitle = ", SpawnPointTitle, "; SpawnPointName = ", SpawnPointName
        spnpnt = None
        
        if type(link) == type(ptAgeLinkStruct()):
            als = link
        else:
            als = link.asAgeLinkStruct()
            
            spawnPoints = link.getSpawnPoints()
            for sp in spawnPoints:
                if (sp.getTitle() == SpawnPointTitle and sp.getName() == SpawnPointName) or self.CheckForCityBookSpawnPoint(ageName, SpawnPointTitle):
                    print "found spawn point: %s, %s" % (sp.getTitle(), sp.getName())
                    if sp.getName() == "BigRoomLinkInPoint":
                        print "oops, found spawnpt for GZ BigRoom, we don't want to link there via city book, so we'll ignore it"
                        break
                    spnpnt = sp
                    break

        if not spnpnt:
            spnpnt = ptSpawnPointInfo(SpawnPointTitle, SpawnPointName)
            
        print "spnpnt.getTitle() = ",spnpnt.getTitle()
        print "spnpnt.getName() = ",spnpnt.getName()
        
        als.setSpawnPoint(spnpnt)

        vault = ptVault()
        
        # If in my personal age, link with kOwnedBook rules.
        #   This will startup a new, private age instance for me.
        if (vault.inMyPersonalAge()):
            if ageName == "Ahnonay" or (ageName == "Neighborhood" and not PtIsSubscriptionActive()):
                als.setLinkingRules( PtLinkingRules.kBasicLink )
            elif IsChildLink:
                print "psnlBookshelf.ILink(): using kChildAgeBook rules for link to: ",ageName
                als.setLinkingRules( PtLinkingRules.kChildAgeBook )
                als.setParentAgeFilename("Neighborhood")
            else:
                print "psnlBookshelf.ILink(): using kOwnedBook rules for link to: ",ageName
                als.setLinkingRules( PtLinkingRules.kOwnedBook )
        # Otherwise, always use kOriginalBook rules.
        #   The engine will handle whether player becomes co-owner or not.
        else:
            #als.setLinkingRules( PtLinkingRules.kOriginalBook )
            # not using co-ownership right now so visitors use visit books unless going to neighborhood
            if ageName == "Neighborhood":
                als.setLinkingRules( PtLinkingRules.kOriginalBook )
            else:
                als.setLinkingRules( PtLinkingRules.kBasicLink )

        linkMgr = ptNetLinkingMgr()         
        linkMgr.linkToAge(als)
        print "ILink Done"


    def IShelveBook(self):
        "returns a picked book to the shelf"
        global objBookPicked
        bookName = objBookPicked.getName()
        for rkey,rvalue in respShelveBook.byObject.items():
            parent = rvalue.getParentKey()
            if parent:
                if bookName == parent.getName():
                    respShelveBook.run(self.key,objectName=rkey)
                    break

        self.UsingBook = 0
        actBookshelfExit.enable()

                
    def IGetAgeFromBook(self):
        "returns age name associated with global objBookPicked or None"
        global objBookPicked

        # find where book object is in object library
        try:
            index = objLibrary.value.index(objBookPicked)
        except:
            print "psnlBookshelf.IUpdateLinks():\tERROR -- couldn't find ", objBookPicked, " in objLibrary"
            return None            
        print "psnlBookshelf.IGetAgeFromBook():\tpicked book goes to ", linkLibrary[index]
        return linkLibrary[index]


    def IResetShelf(self):
        global ShelfABoolOperated
        global ShelfAUserID
        
        PtDebugPrint ("psnlBookshelf.IResetShelf:\tResetting shelf")
        ShelfABoolOperated = 0
        ShelfAUserID = -1
        respLowerShelfClickable.run(self.key)
        actBookshelf.enable()
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            ageSDL["ShelfABoolOperated"] = (0,)
            ageSDL["ShelfAUserID"] = (-1,)


    def IDisengageShelf(self):
        global miniKIrestore
        
        self.SendNote(0)
        if AgeStartedIn == PtGetAgeName() and not self.UsingBook:
            ageSDL = PtGetAgeSDL()
            CurrentBookshelfUser = ageSDL["ShelfAUserID"][0]
            #~ print "psnlBookshelf.OnNotify: Player %s was previously using the shelf." % (CurrentBookshelfUser)
    
            PtDebugPrint ("psnlBookshelf.OnNotify: Player %s is done with the bookshelf." % (CurrentBookshelfUser))
            self.IResetShelf()             
                    
            avatar = PtGetLocalAvatar()   
            myID = PtGetClientIDFromAvatarKey(avatar.getKey())
        
            #~ print "I think my ID is: ", myID
        
            if myID == CurrentBookshelfUser:
                print "I was the Shelf User, and I'm done with the Shelf now."
    
                #PtFadeLocalAvatar(0)
                avatar.draw.enable()
                #reeneable first person
                cam = ptCamera()
                cam.enableFirstPersonOverride()
                # go back to the Hut Circle Cam
                virtCam = ptCamera()
                virtCam.save(HutCamera.sceneobject.getKey())
                PtEnableMovementKeys()
                actBookshelfExit.disable()
                PtGetControlEvents(false,self.key)

                if miniKIrestore:
                    miniKIrestore = 0
                    PtShowDialog("KIMini")


    def OnControlKeyEvent(self,controlKey,activeFlag):
        if controlKey == PlasmaControlKeys.kKeyExitMode or controlKey == PlasmaControlKeys.kKeyMoveBackward:
            self.IDisengageShelf()


    def OnTimer(self, id):
        if id == 1:
            PtGetControlEvents(true,self.key)
            actBookshelfExit.enable()


    def IGetHoodLinkNode(self):
        vault = ptVault()
        folder = vault.getAgesIOwnFolder()
        contents = folder.getChildNodeRefList()
        for content in contents:
            link = content.getChild()
            link = link.upcastToAgeLinkNode()
            if type(link) != type(None):
                info = link.getAgeInfo()
            if not info:
                continue
            ageName = info.getAgeFilename()
            if ageName == "Neighborhood":
                return link
        return None


    def IGetHoodInfoNode(self):
        link = self.IGetHoodLinkNode()
        if type(link) == type(None):
            return None
        info = link.getAgeInfo()
        return info


    def IGetHoodChildLink(self, age):
        hoodInfo = self.IGetHoodInfoNode()
        if hoodInfo:
            childAgeFolder = hoodInfo.getChildAgesFolder()
            if type(childAgeFolder) != type(None):
                contents = childAgeFolder.getChildNodeRefList()
                #childAgeLinkNodes = []
                #GZLinkNode = None
                for content in contents:
                    link = content.getChild().upcastToAgeLinkNode()
                    info = link.getAgeInfo()
                    if not info:
                        continue
                    ageName = info.getAgeFilename()
                    #print "name = ",name
                    if ageName == age:
                        return link
        return None    


    def HasCityBook(self):
        #####
        #  COMMENT OUT THIS LINE TO RESTORE THE CITY BOOK:
        #return 0
        #####
        global CityBookAges
        
        vault = ptVault()
        if not vault.amOwnerOfCurrentAge():
            ageSDL = PtGetAgeSDL()
            GotBook = ageSDL["psnlGotCityBook"][0]
            if GotBook:
                print "psnlBookshelf.HasCityBook(): owner has the city book"
                return 1
            else:
                print "psnlBookshelf.HasCityBook(): owner does NOT have city book"
                return 0

        CityLinks = []
        vault = ptVault()
        entryCityLinks = vault.findChronicleEntry("CityBookLinks")
        if type(entryCityLinks) != type(None):
            valCityLinks = entryCityLinks.chronicleGetValue()
            print "valCityLinks = ",valCityLinks
            CityLinks = valCityLinks.split(",")
            print "CityLinks = ",CityLinks
            for tmpLink in CityLinks:
                if tmpLink in xLinkingBookDefs.CityBookLinks:
                    return 1
        else:
            print "can't find CityBookLinks chron entry"
        return 0

        vault = ptAgeVault()
        # look for city book age links
        for age, splist in CityBookAges.items():
            #agelink = self.GetOwnedAgeLink(vault, age)
            agelink = self.IGetHoodChildLink(age)
            print "age = ",age
            print "agelink = ",agelink

            if type(agelink) != type(None):
                spawnPoints = agelink.getSpawnPoints()

                for sp in spawnPoints:
                    print "sp = ",sp
                    print "sp.getTitle = ",sp.getTitle()
                    print "sp.getName = ",sp.getName()
                    if sp.getTitle() in splist:
                        print "found a city book link:", age, sp.getTitle()
                        return 1

        # look for a city treasure link, but it's a Hood childage now
        #agelink = self.GetOwnedAgeLink(vault, "city")
        agelink = self.IGetHoodChildLink(age)
        print "age = the city"
        print "agelink = ",agelink

        if type(agelink) != type(None):
            spawnPoints = agelink.getSpawnPoints()

            for sp in spawnPoints:
                print "sp = ",sp
                print "sp.getTitle = ",sp.getTitle()
                print "sp.getName = ",sp.getName()
                if sp.getTitle() in xLinkingBookDefs.CityBookLinks:
                    print "found a city book link: city", sp.getTitle()
                    return 1

        # no BCO or city treasure link
        print "found no city book links"
        return 0


    def GetOwnedAgeLink(self, vault, age):
        PAL = vault.getAgesIOwnFolder()
        if type(PAL) != type(None):
            contents = PAL.getChildNodeRefList()
            for content in contents:
                link = content.getChild().upcastToAgeLinkNode()
                info = link.getAgeInfo()
                if not info:
                    continue
                ageName = info.getAgeFilename()
                print "found %s, looking for %s" % (ageName, age)
                if ageName == age:
                    return link

        return None

