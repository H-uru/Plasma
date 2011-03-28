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
Module: clftYeeshaPage08
Age: Cleft
Date: May 2003
Author: Adam Van Ornum
Manages and records the finding of Yeesha Pages
!!! NOTE: This file only applies to the cleft but is only used in the global xYeeshaPagesGUI.max file. !!!
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *


# define the attributes that will be entered in max
actClickableBook = ptAttribNamedActivator(1,"Act: Clickable Yeesha Page")
GUIDialogObject = ptAttribSceneobject(2, "GUIDialog scene object")
RespOpen = ptAttribResponder(3, "Open Responder")
RespLoop = ptAttribResponder(4, "Loop Responder")
RespClose = ptAttribResponder(5, "Close Responder")

#Linking Books GUI tags
DialogName="YeeshaPageGUI"

kPageButton = 100

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

isOpen = 0


class clftYeeshaPage08(ptModifier):
    "The Yeesha Page 08 cleft imager python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5312
        self.version = 1
        print "__init__clftYeeshaPage08 v.", self.version


    def OnFirstUpdate(self):
        PtLoadDialog(DialogName, self.key)
        pass
 

    def __del__(self):
        "destructor - get rid of any dialogs that we might have loaded"
        #~ PtUnloadDialog(DialogName)

        
    def OnNotify(self,state,id,events):
        global LocalAvatar
        global isOpen
        
        if id == actClickableBook.id and state and PtWasLocallyNotified(self.key):
            #if not PtIsDialogLoaded(DialogName):
            #    PtLoadDialog(DialogName,self.key)

            self.SetStdGUIVisibility(0)
            PtShowDialog(DialogName)
            RespOpen.run(self.key)
            isOpen = 1

        elif id == actClickableBook.id and not state and PtWasLocallyNotified(self.key):
            if not isOpen:
                self.SetStdGUIVisibility(0)
                PtShowDialog(DialogName)
                RespOpen.run(self.key)
                isOpen = 1
                
        elif id == RespOpen.id:
            RespLoop.run(self.key)


    def OnGUINotify(self,id,control,event):
        global isOpen
        btnID = 0
        if isinstance(control,ptGUIControlButton):
            btnID = control.getTagID()

        if event == kShowHide:
            if control.isEnabled():
                #control.show()
                if self.GotPage():
                    mydialog = PtGetDialogFromString(DialogName)
                    ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage08)).disable()

        elif event == kAction and btnID == kYeeshaPage08:
            PtDebugPrint("DEBUG: clftYeeshaPage08.OnGUINotify():\tPicked up page")

            RespClose.run(self.key)
            isOpen = 0
            PtHideDialog(DialogName)
            self.SetStdGUIVisibility(1)
    
            if self.GotPage(): 
                PtDebugPrint ("DEBUG: clftYeeshaPage08.py: You've already found Yeesha Page #8. Move along. Move along.")
                return
            else:
                PtDebugPrint ("DEBUG: clftYeeshaPage08.py: Yeesha Page #8 is new to you.")       
                PtDebugPrint ("DEBUG: clftYeeshaPage08.py: Trying to update the value of the SDL variable %s to 1" % ("YeeshaPage8"))
                vault = ptVault()
                if type(vault) != type(None): #is the Vault online?    
                    psnlSDL = vault.getPsnlAgeSDL()
                    if psnlSDL:
                        YeeshaPageVar = psnlSDL.findVar("YeeshaPage8")    
                        YeeshaPageVar.setInt(1)
                        vault.updatePsnlAgeSDL (psnlSDL)
                        mydialog = PtGetDialogFromString(DialogName)
                        ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage08)).disable()
                        PtSendKIMessageInt(kStartBookAlert,0)

        elif event == kAction and btnID == kYeeshaPageCancel:
            RespClose.run(self.key)
            isOpen = 0
            PtHideDialog(DialogName)
            self.SetStdGUIVisibility(1)


    def GotPage(self):
        vault = ptVault()
        if type(vault) != type(None): #is the Vault online?    
            psnlSDL = vault.getPsnlAgeSDL()
            if psnlSDL:
                YeeshaPageVar = psnlSDL.findVar("YeeshaPage8")    
                PtDebugPrint ("DEBUG: clftYeeshaPage08.py: The previous value of the SDL variable %s is %s" % ("YeeshaPage8", YeeshaPageVar.getInt()))
                if YeeshaPageVar.getInt() != 0: 
                    PtDebugPrint ("DEBUG: clftYeeshaPage08.py: You've already found Yeesha Page #8. Move along. Move along.")
                    return 1
                else:
                    return 0
            else:
                PtDebugPrint("ERROR: clftYeeshaPage08: Error trying to access the Chronicle psnlSDL. psnlSDL = %s" % ( psnlSDL))
                return 0
        else:
            PtDebugPrint("ERROR: clftYeeshaPage08: Error trying to access the Vault. Can't access YeeshaPageChanges chronicle." )
            return 0


    def SetStdGUIVisibility(self, visible):
        global DialogName

        if visible:
            GUIDialogObject.value.draw.enable()
        else:
            mydialog = PtGetDialogFromString(DialogName)

            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage01)).hide()
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage02)).hide()
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage03)).hide() 
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage04)).hide() 
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage05)).hide()
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage06)).hide()
            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage07)).hide()
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

            ptGUIControlButton(mydialog.getControlFromTag(kYeeshaPage08)).show()

            GUIDialogObject.value.draw.disable()