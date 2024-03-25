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
RespGlow = ptAttribResponder(6, "Glow Responder",['GlowSound','PageTurn'])

#Linking Books GUI tags
DialogName="YeeshaPageGUI"

kPageButton = 100

kYeeshaPage = list(range(200, 298, 1))
kYeeshaPageCancel = 299

isOpen = 0


class clftYeeshaPage08(ptModifier):
    "The Yeesha Page 08 cleft imager python code"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5312
        self.version = 1
        PtDebugPrint("__init__clftYeeshaPage08 v.", self.version)


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
            PtLoadDialog(DialogName,self.key)
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

        if event == kAction and btnID == kYeeshaPage[8]:
            PtDebugPrint("DEBUG: clftYeeshaPage08.OnGUINotify():\tPicked up page")

            RespClose.run(self.key)
            isOpen = 0
            PtHideDialog(DialogName)
            self.SetStdGUIVisibility(1)
            RespGlow.run(self.key, state='GlowSound')
    
            if self.GotPage(): 
                PtDebugPrint ("DEBUG: clftYeeshaPage08.py: You've already found Yeesha Page #8. Move along. Move along.")
                return
            else:
                PtDebugPrint ("DEBUG: clftYeeshaPage08.py: Yeesha Page #8 is new to you.")       
                PtDebugPrint ("DEBUG: clftYeeshaPage08.py: Trying to update the value of the SDL variable %s to 4" % ("YeeshaPage8"))
                vault = ptVault()  
                psnlSDL = vault.getPsnlAgeSDL()
                if psnlSDL:
                    YeeshaPageVar = psnlSDL.findVar("YeeshaPage8")    
                    YeeshaPageVar.setInt(4)
                    vault.updatePsnlAgeSDL(psnlSDL)
                    mydialog = PtGetDialogFromString(DialogName)
                    PtSendKIMessageInt(kStartBookAlert,0)

        elif event == kAction and btnID == kYeeshaPageCancel:
            RespClose.run(self.key)
            isOpen = 0
            PtHideDialog(DialogName)
            self.SetStdGUIVisibility(1)


    def GotPage(self):
        vault = ptVault()  
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


    def SetStdGUIVisibility(self, visible):
        global DialogName

        if visible:
            GUIDialogObject.value.draw.enable()
        else:
            mydialog = PtGetDialogFromString(DialogName)

            for x in kYeeshaPage:
                try:
                    ctrl = mydialog.getControlModFromTag(x)
                except KeyError:
                    continue
                else:
                    ctrl.hide()

            mydialog.getControlModFromTag(kYeeshaPage[8]).show()

            GUIDialogObject.value.draw.disable()

