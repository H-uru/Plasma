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
"""Module: clftGetPersonalBook
Age: Cleft
Date: October 2002
Author: Doug McBride
Displays Player Book GUI
Plays appropriate book animation, based on player's gender
Sets Chronical Entry to ensure that players who have "solved" the cleft can't return.
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from xPsnlVaultSDL import *
import xLinkingBookDefs
import os


# define the attributes that will be entered in max
actClickableBook = ptAttribActivator(1, "Clickable to link")
SmartSeek = ptAttribBehavior(2, "Smart Seek Before GUI")
MultiBeh = ptAttribBehavior(3, "MultiBeh of avatar getting book")
BookAnimMale = ptAttribAnimation(4, "Book Animation - Male Height")
BookAnimFemale = ptAttribAnimation(5, "Book Animation - Female Height")
respLinkResponder = ptAttribResponder(6, "Linking responder")
respLinkOutNew = ptAttribResponder(7, "resp:  new linkout")


# global variables
LocalAvatar = None
YeeshaBook = None
gAreWeLinkingOut = 0

kLinkRespID = 7


class clftGetPersonalBook(ptResponder):

    def __init__(self):
        ptResponder.__init__(self)
        self.id = 5219
        self.version = 10
        PtDebugPrint("__init__clftGetPersonalBook v%d.%d" % (self.version,2),level=kWarningLevel)

    def OnFirstUpdate(self):
        pass
        #~ vault = ptVault()
        #~ entry = vault.findChronicleEntry("JourneyClothProgress")
        #~ if entry is not None:
            #~ FoundJCs = entry.getValue()
            #~ if "Z" in FoundJCs:
                #~ PtPageOutNode("clftYeeshaBookVis")
                #~ PtDebugPrint("clftGetPersonalBook: Paging out the Yeesha Book in the bookroom",level=kDebugDumpLevel)


    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        ageSDL.setNotify(self.key,"clftIsCleftDone",0.0)

    
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname == "clftIsCleftDone":
            ageSDL = PtGetAgeSDL()
            boolCleftDone = ageSDL["clftIsCleftDone"][0]
            if boolCleftDone:
                respLinkOutNew.run(self.key,avatar=PtGetLocalAvatar())


    def OnNotify(self,state,id,events):
        global LocalAvatar
        global YeeshaBook
        global gAreWeLinkingOut

        # is it from an unknown id? Probably from the ptBook
        if id < 0:
            for event in events:
                # is it from the YeeshaBook? (we only have one book to worry about)
                if event[0] == PtEventType.kBook:
                    PtDebugPrint("clftGetPersonalBook: BookNotify  event=%d, id=%d" % (event[1],event[2]),level=kDebugDumpLevel)
                    if event[1] == PtBookEventTypes.kNotifyImageLink:
                        if event[2] == xLinkingBookDefs.kYeeshaBookLinkID:
                            PtDebugPrint("clftGetPersonalBook:Book: hit linking panel",level=kDebugDumpLevel)
                            gAreWeLinkingOut = 1
                            YeeshaBook.hide()
                            self.ILinktoPersonalAge()
                    elif event[1] == PtBookEventTypes.kNotifyShow:
                        pass
                    elif event[1] == PtBookEventTypes.kNotifyHide:
                        PtDebugPrint("clftGetPersonalBook:Book: NotifyHide",level=kDebugDumpLevel)
                        # don't really care if they close the book, but re-enable the clickable for them
                        if not gAreWeLinkingOut:
                            actClickableBook.enable()
                            # only re-enable the KI and BB if they are not linking out
                            PtSendKIMessage(kEnableKIandBB,0)
                        pass
                    elif event[1] == PtBookEventTypes.kNotifyNextPage:
                        pass
                    elif event[1] == PtBookEventTypes.kNotifyPreviousPage:
                        pass
                    elif event[1] == PtBookEventTypes.kNotifyCheckUnchecked:
                        pass

        if PtWasLocallyNotified(self.key):
            
            # click on the book?
            if state and id == actClickableBook.id:
                # disable the book... need to re-enable if they cancel
                actClickableBook.disable()
                # prevent Martin from hitting the option menu and playing the live movie
                PtSendKIMessage(kDisableKIandBB,0)
                gAreWeLinkingOut = 0
                LocalAvatar = PtFindAvatar(events)
                SmartSeek.run(LocalAvatar)

            # smart seek is done
            if id == SmartSeek.id:
                for event in events:
                    # if smart seek completed. Exit multistage, and show GUI.
                    if event[0] == kMultiStageEvent and event[2] == kEnterStage:
                        SmartSeek.gotoStage(LocalAvatar, -1) 
                        YeeshaBook = ptBook(xLinkingBookDefs.xYeeshaBookNoShare,self.key)
                        YeeshaBook.setSize( xLinkingBookDefs.YeeshaBookSizeWidth, xLinkingBookDefs.YeeshaBookSizeHeight )
                        YeeshaBook.show(1)

            # picking up the book is beginning
            if id == MultiBeh.id:
                for event in events:
                    if event[0] == kMultiStageEvent and event[2] == kEnterStage:
                        avatar = PtGetLocalAvatar()
                        currentgender = avatar.avatar.getAvatarClothingGroup()
                        cam = ptCamera()
                        cam.disableFirstPersonOverride()
                        cam.undoFirstPerson()
                        if currentgender == 1:
                            PtDebugPrint("Playing female book animation")
                            BookAnimFemale.animation.play()
                        elif currentgender == 0:
                            PtDebugPrint("Playing male book animation")
                            BookAnimMale.animation.play()
                        else:
                            PtDebugPrint("clftGetPersonalBook: unreadable gender or special character.",level=kErrorLevel)
                            BookAnimMale.animation.play()

    def ILinktoPersonalAge(self):
        global LocalAvatar
        # start the alert of the personal book blinking
        PtSendKIMessage(kStartBookAlert,0)
        #~ PtDebugPrint("trying to get book.")
        MultiBeh.run(LocalAvatar)
        self.SolveCleft()
        PtAtTimeCallback(self.key, 8, kLinkRespID) 

    def SolveCleft(self):
        vault = ptVault()
        vault.addChronicleEntry("CleftSolved", 1, "yes")
        PtDebugPrint("Chronicle updated with variable 'CleftSolved'.", level=kDebugDumpLevel)
        PtSendKIMessage(kEnableEntireYeeshaBook, 0)
        PtFindSceneobject("microBlackBarBody", "GUI").draw.enable()
        psnlSDL = xPsnlVaultSDL()
        if psnlSDL:
            psnlSDL["YeeshaPage25"] = (4,)

    def OnTimer(self, id):
        if id == kLinkRespID:
            respLinkResponder.run(self.key, avatar=PtGetLocalAvatar())
            PtSendKIMessage(kEnableKIandBB, 0)
            PtGetLocalAvatar().avatar.setDontPanicLink(False)
