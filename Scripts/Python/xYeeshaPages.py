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
Module: xYeeshaPages
Age: global
Date: January 2002
Author: Doug McBride
Manages and records the finding of Yeesha Pages
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
from xStartPathHelpers import *


# define the attributes that will be entered in max
actClickableBook = ptAttribActivator(1,"Act: Clickable Yeesha Page")
PageNumber = ptAttribInt(2, "Yeesha Page Number")

#Linking Books GUI tags
DialogName="YeeshaPageGUI"
kPageButton = 100

kReltoLinkID = 1
kStartFadeoutID = 2

kWaitFadeoutSecs = 3.0
kFadeOutSecs = 2.0
kReltoLinkSecs = 4.0

#Note: when adding more Ages to this script, be sure to also update IDrawLinkPanel() module below
kYeeshaPage = list(range(200, 298, 1))
kYeeshaPageCancel = 299

class xYeeshaPages(ptModifier):
    "The Yeesha Page python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5225
        version = 6
        self.version = version
        PtDebugPrint("__init__xYeeshaPages v.", version)


    def OnFirstUpdate(self):
        PtLoadDialog(DialogName, self.key)
        pass


    def __del__(self):
        "destructor - get rid of any dialogs that we might have loaded"
        PtUnloadDialog(DialogName)

         
    def OnServerInitComplete(self):
        ageName = PtGetAgeName()
        if ageName == "Cleft":
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("clftIsCleftDone",1,1)
            ageSDL.sendToClients("clftIsCleftDone")


    def OnNotify(self,state,id,events):
        global LocalAvatar

        if state and id == actClickableBook.id and PtWasLocallyNotified(self.key):
            PtLoadDialog(DialogName,self.key)
            if ( PtIsDialogLoaded(DialogName) ):
                self.IDrawLinkPanel()
                PtShowDialog(DialogName)                        
                    

    def OnGUINotify(self,id,control,event):

        if event == kExitMode:
            PtHideDialog(DialogName)
            return
        
        btnID = 0
        if isinstance(control,ptGUIControlButton):
            btnID = control.getTagID()

        if event == 2 and btnID in kYeeshaPage:
            PtDebugPrint("xYeeshaPages.OnGUINotify():\tPicked up page number: ", PageNumber.value)
#            PtUnloadDialog(DialogName)
            PtHideDialog(DialogName)
            
            vault = ptVault()
                
            psnlSDL = vault.getPsnlAgeSDL()
            if psnlSDL:
                YeeshaPageVar = psnlSDL.findVar("YeeshaPage" + str(PageNumber.value))
                
                PtDebugPrint ("xYeeshaPages.py: The previous value of the SDL variable %s is %s" % ("YeeshaPage" + str(PageNumber.value), YeeshaPageVar.getInt()))

                if StartInCleft():
                    PtFindSceneobject("YeeshaPageButton","GUI").runAttachedResponder(1)
                else:
                    PtFindSceneobject("YeeshaPageButton","GUI").runAttachedResponder(0)

                if YeeshaPageVar.getInt() != 0: 
                    PtDebugPrint ("xYeeshaPages.py: You've already found Yeesha Page #%s. Move along. Move along." % (PageNumber.value))
                    return
                    
                else:
                    PtDebugPrint ("xYeeshaPages.py: Yeesha Page #%s is new to you." % (PageNumber.value))
                    
                    PtDebugPrint ("xYeeshaPages.py: Trying to update the value of the SDL variable %s to 1" % ("YeeshaPage" + str(PageNumber.value)))
                    YeeshaPageVar.setInt(4)
                    vault.updatePsnlAgeSDL(psnlSDL)

                    if not StartInCleft():
                        PtSendKIMessageInt(kStartBookAlert,0)

                    if (PageNumber.value) == 25:
                        #Cleft is done, set SDL to start link back to Relto
                        actClickableBook.disableActivator()
                        PtSendKIMessage(kDisableKIandBB,0)
                        ageSDL = PtGetAgeSDL()
                        ageSDL["clftIsCleftDone"] = (1,)
                        vault = ptVault()
                        vault.addChronicleEntry("CleftSolved",1,"yes")
                        PtDebugPrint("Chronicle updated with variable 'CleftSolved'.",level=kDebugDumpLevel)
                        #PtAtTimeCallback(self.key,kWaitFadeoutSecs,kStartFadeoutID)

            else:
                PtDebugPrint("xYeeshaPages: Error trying to access the Chronicle psnlSDL. psnlSDL = %s" % ( psnlSDL))

        elif event == 2 and btnID == kYeeshaPageCancel:
            PtHideDialog(DialogName)
    


    def IDrawLinkPanel(self):
        global DialogName
        mydialog = PtGetDialogFromString(DialogName)

        #first hide them all
#        PtDebugPrint("PageNumber = ", PageNumber.value)
        for x in kYeeshaPage:
            try:
                ctrl = mydialog.getControlModFromTag(x)
            except KeyError:
                continue
            else:
                ctrl.hide()

        #now draw correct panel
        mydialog.getControlModFromTag(kYeeshaPage[PageNumber.value]).show()
        return

#
#    def OnTimer(self,id):
#        if id == kStartFadeoutID:
#            PtFadeOut(kFadeOutSecs,1)
#            PtDebugPrint("\txYeeshaPages.OnTimer(): Linking the player from Cleft to Relto.  FadeOut over", kFadeOutSecs," seconds.")
#            PtAtTimeCallback(self.key,kReltoLinkSecs,kReltoLinkID)
#        elif id == kReltoLinkID:  
#            #link back to Relto now
#            linkmgr = ptNetLinkingMgr()
#            ageLink = ptAgeLinkStruct()
#            ageInfo = ageLink.getAgeInfo()
#            temp = ptAgeInfoStruct()
#            temp.copyFrom(ageInfo)
#            ageInfo = temp
#            ageInfo.setAgeFilename("Personal")
#            ageLink.setAgeInfo(ageInfo)
#            ageLink.setLinkingRules(PtLinkingRules.kOwnedBook)
#            linkmgr.linkToAge(ageLink)


