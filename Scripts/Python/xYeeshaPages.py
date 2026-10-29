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

actClickableBook = ptAttribActivator(1,"Act: Clickable Yeesha Page")
PageNumber = ptAttribInt(2, "Yeesha Page Number")

# This file controls the GUI when finding a Yeesha Page.
# Add yeesha page textures to be loaded by name here. (Mapping Page Number: Texture)
# Just like link panels, they should probably be in BkBookImages or derivatives thereof.
# For the equivalent place to add it to the relto book, see xLinkingBookDefs.py
YeeshaPageSketches = {
    1: "xyeeshapagealphasketch01*1#0.hsm",
    2: "xyeeshapagealphasketch02*1#0.hsm",
    3: "xyeeshapagealphasketch03*1#0.hsm",
    4: "xyeeshapagealphasketch04*1#0.hsm",
    5: "xyeeshapagealphasketch07*1#0.hsm",
    6: "xyeeshapagealphasketch06*1#0.hsm",
    7: "xyeeshapagealphasketch05*1#0.hsm",
    8: "xyeeshapagealphasketch12*1#0.hsm",
    9: "xyeeshapagealphasketch09*1#0.hsm",
    10: "xyeeshapagealphasketch10*1#0.hsm",
    12: "xyeeshapagealphasketch08*1#0.hsm",
    13: "xyeeshapagealphasketch13*1#0.hsm",
    14: "xyeeshapagealphasketchfireplace*1#0.hsm",
    15: "xyeeshapagealphasketch15*1#0.hsm",
    16: "xyeeshapagealphasketchfiremarbles*1#0.hsm",
    17: "xyeeshapagealphasketchlushrelto*1#0.hsm",
    18: "xyeeshapagealphasketchclock*1#0.hsm",
    19: "xyeeshapagealphasketchbirds*1#0.hsm",
    20: "xyeeshapagealphasketchcalendar*1#0.hsm",
    21: "xyeeshapagealphasketchleaf*1#0.hsm",
    22: "xyeeshapagealphasketchgrass*1#0.hsm",
    23: "xyeeshapagealphasketchercaplant*1#0.hsm",
    24: "xyeeshapagealphasketchstorm*1#0.hsm",
    25: "xyeeshapagealphasketch14*1#0.hsm",
    26: "xYeeshaPageVeelay7.dds",
    32: "xYeeshaPageGreenBushes.dds",
}

kDialogName = "YeeshaPageGUI"
kLayerName = "YeeshaPageSketch"

kYeeshaPage = 201 # GUI element for the sketch
kYeeshaPageHide = [208] # GUI elements to be hidden. 208 is for cleft imager page
kYeeshaPageCancel = 299

class xYeeshaPages(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5225
        version = 6
        self.version = version
        PtDebugPrint("__init__xYeeshaPages v.", version)


    def OnFirstUpdate(self):
        PtLoadDialog(kDialogName, self.key)
        pass


    def __del__(self):
        PtUnloadDialog(kDialogName)

         
    def OnServerInitComplete(self):
        ageName = PtGetAgeName()
        if ageName == "Cleft":
            ageSDL = PtGetAgeSDL()
            ageSDL.setFlags("clftIsCleftDone",1,1)
            ageSDL.sendToClients("clftIsCleftDone")


    def OnNotify(self,state,id,events):
        if state and id == actClickableBook.id and PtWasLocallyNotified(self.key):
            PtLoadDialog(kDialogName,self.key)
            if PtIsDialogLoaded(kDialogName):
                self.IDrawLinkPanel()
                PtShowDialog(kDialogName)                        
                    

    def OnGUINotify(self,id,control,event):
        if event == kExitMode:
            PtHideDialog(kDialogName)
            return
        
        btnID = 0
        if isinstance(control,ptGUIControlButton):
            btnID = control.getTagID()

        if event == 2 and btnID == kYeeshaPage:
            PtDebugPrint("xYeeshaPages.OnGUINotify():\tPicked up page number: ", PageNumber.value)
            PtHideDialog(kDialogName)
            
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

            else:
                PtDebugPrint("xYeeshaPages: Error trying to access the Chronicle psnlSDL. psnlSDL = %s" % ( psnlSDL))

        elif event == 2 and btnID == kYeeshaPageCancel:
            PtHideDialog(kDialogName)


    def IDrawLinkPanel(self):
        mydialog = PtGetDialogFromString(kDialogName)
        for x in kYeeshaPageHide:
            try:
                ctrl = mydialog.getControlModFromTag(x)
            except KeyError:
                continue
            else:
                ctrl.hide()

        if image_name := YeeshaPageSketches.get(PageNumber.value):
            if image := PtFindImage(image_name):
                if layer := PtFindLayer(kLayerName, "GUI", kDialogName):
                    layer.setTexture(image[0])
