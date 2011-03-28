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
kYeeshaPage01 = 201
kYeeshaPage02 = 202
kYeeshaPage03 = 203
kYeeshaPage04 = 204
kYeeshaPage05 = 205
kYeeshaPage06 = 206
kYeeshaPage07 = 207
kYeeshaPage08 = 208
kYeeshaPage09 = 209
kYeeshaPage10 = 210
kYeeshaPage12 = 212
kYeeshaPage13 = 213
kYeeshaPage14 = 214
kYeeshaPage15 = 215
kYeeshaPage16 = 216
kYeeshaPage17 = 217
kYeeshaPage18 = 218
kYeeshaPage19 = 219
kYeeshaPage20 = 220
kYeeshaPage21 = 221
kYeeshaPage22 = 222
kYeeshaPage23 = 223
kYeeshaPage24 = 224
kYeeshaPage25 = 225
kYeeshaPageCancel = 299

YeeshaPageIDList = [ kYeeshaPage01, kYeeshaPage02, kYeeshaPage03, kYeeshaPage04,\
                    kYeeshaPage05, kYeeshaPage06, kYeeshaPage07, kYeeshaPage08,\
                    kYeeshaPage09, kYeeshaPage10, kYeeshaPage12, kYeeshaPage13,\
                    kYeeshaPage14, kYeeshaPage15, kYeeshaPage16, kYeeshaPage17,\
                    kYeeshaPage18, kYeeshaPage19, kYeeshaPage20, kYeeshaPage21, kYeeshaPage22, kYeeshaPage23, kYeeshaPage24, kYeeshaPage25 ]


class xYeeshaPages(ptModifier):
    "The Yeesha Page python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5225
        version = 6
        self.version = version
        print "__init__xYeeshaPages v.", version


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

        if event == 2 and btnID in YeeshaPageIDList:
            print "xYeeshaPages.OnGUINotify():\tPicked up page number: ", PageNumber.value
#            PtUnloadDialog(DialogName)
            PtHideDialog(DialogName)
            
            vault = ptVault()
            if type(vault) != type(None): #is the Vault online?
                
                psnlSDL = vault.getPsnlAgeSDL()
                if psnlSDL:
                    YeeshaPageVar = psnlSDL.findVar("YeeshaPage" + str(PageNumber.value))
                    
                    PtDebugPrint ("xYeeshaPages.py: The previous value of the SDL variable %s is %s" % ("YeeshaPage" + str(PageNumber.value), YeeshaPageVar.getInt()))
    
                    if YeeshaPageVar.getInt() != 0: 
                        PtDebugPrint ("xYeeshaPages.py: You've already found Yeesha Page #%s. Move along. Move along." % (PageNumber.value))
                        return
                        
                    else:
                        PtDebugPrint ("xYeeshaPages.py: Yeesha Page #%s is new to you." % (PageNumber.value))
                        
                        PtDebugPrint ("xYeeshaPages.py: Trying to update the value of the SDL variable %s to 1" % ("YeeshaPage" + str(PageNumber.value)))
                        YeeshaPageVar.setInt(4)
                        vault.updatePsnlAgeSDL (psnlSDL)

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
                    
            else:
                PtDebugPrint("xYeeshaPages: Error trying to access the Vault. Can't access YeeshaPageChanges chronicle." )

        elif event == 2 and btnID == kYeeshaPageCancel:
            PtHideDialog(DialogName)
    


    def IDrawLinkPanel(self):
        global DialogName
        mydialog = PtGetDialogFromString(DialogName)

        #first hide them all
#        print "PageNumber = ", PageNumber.value
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage01)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage02)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage03)).hide() 
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage04)).hide() 
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage05)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage06)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage07)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage08)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage09)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage10)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage12)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage13)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage14)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage15)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage16)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage17)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage18)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage19)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage20)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage21)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage22)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage23)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage24)).hide()
        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage25)).hide()

        #now draw correct panel
        if (PageNumber.value) == 1:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage01)).show()
        elif (PageNumber.value) == 2:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage02)).show()
        elif (PageNumber.value) == 3:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage03)).show()
        elif (PageNumber.value) == 4:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage04)).show()
        elif (PageNumber.value) == 5:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage05)).show()
        elif (PageNumber.value) == 6:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage06)).show()
        elif (PageNumber.value) == 7:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage07)).show()
        elif (PageNumber.value) == 8:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage08)).show()
        elif (PageNumber.value) == 9:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage09)).show()
        elif (PageNumber.value) == 10:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage10)).show()
        elif (PageNumber.value) == 12:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage12)).show()
        elif (PageNumber.value) == 13:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage13)).show()
        elif (PageNumber.value) == 14:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage14)).show()
        elif (PageNumber.value) == 15:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage15)).show()
        elif (PageNumber.value) == 16:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage16)).show()
        elif (PageNumber.value) == 17:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage17)).show()
        elif (PageNumber.value) == 18:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage18)).show()
        elif (PageNumber.value) == 19:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage19)).show()
        elif (PageNumber.value) == 20:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage20)).show()
        elif (PageNumber.value) == 21:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage21)).show()
        elif (PageNumber.value) == 22:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage22)).show()
        elif (PageNumber.value) == 23:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage23)).show()
        elif (PageNumber.value) == 24:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage24)).show()
        elif (PageNumber.value) == 25:
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage25)).show()


        else:
            print "xYeeshaPages.IDrawLinkPanel():\tERROR: couldn't find page named ",PageNumber.value
        return

#
#    def OnTimer(self,id):
#        if id == kStartFadeoutID:
#            PtFadeOut(kFadeOutSecs,1)
#            print "\txYeeshaPages.OnTimer(): Linking the player from Cleft to Relto.  FadeOut over", kFadeOutSecs," seconds."
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


