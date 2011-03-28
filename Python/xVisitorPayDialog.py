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
Module: xVisitorPayDialog
Age: global
Date: May 2006
Author: Tye Hooley
"""

from Plasma import *
from PlasmaTypes import *

import webbrowser

#act = ptAttribActivator(2,"Activator")
#string  = ptAttribString(1,"Name of Age to Clear")

# These are the ID values specified in the max file
# (taken from the xDialogStartUp.py file).
k13VisitID         = 700
k13PayID           = 701
k13LinkID          = 702

WebLaunchCmd = None

class xVisitorPayDialog(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5345
        
        version = 1
        self.version = version
        print "__init__ xVisitorPayDialog v. ", version

    def OnServerInitComplete(self):
        global WebLaunchCmd

        WebLaunchCmd = webbrowser.open_new

    def OnFirstUpdate(self):
        #Added for Free vs Play Content
        #
        # Note:
        #   This was added for externally instanciated dialog box calls
        #   for dialog 05.  This should not affect any functionality 
        #   To the "startup" age.  Eventually this or a similar dialog
        #   should be moved to a seperate file.
        
        #print "xVisitorPayDialog-->OnFirstUpdate() -- self.key = ", self.key
        #print "xVisitorPayDialog-->OnFirstUpdate() -- Started in age ",  PtGetAgeName()

        #Necessary to give intercept GUI Notifies to this file!
        if PtGetAgeName() != "StartUp":
            PtLoadDialog("GUIDialog13", self.key)

    def OnNotify(self,state,id,events):
        #if id==(-1): ## callback from delete yes/no dialog (hopefully) ##
        #    if state:
        #        PtConsole("App.Quit")
        pass

    
    def OnGUINotify(self, id, control, event):
        global WebLaunchCmd
        #print "-->xVisitorPayDialog: GUI Notify id=%d, event=%d control=" % (id,event),control

        if not control:  #Exit if not fired from a control!
            return
        
        #Get the ID from the control
        tagID = control.getTagID()  
        
        if event == kAction or event == kValueChanged:
            if  tagID == k13VisitID: ## Continue ##
                PtHideDialog("GUIDialog13")

            elif  tagID == k13PayID: ## Quit And Register ##
                WebLaunchCmd("https://account.gametap.com/storefront/myst/login/login.do")

            elif  tagID == k13LinkID: ## Link ##
                WebLaunchCmd("https://account.gametap.com/storefront/myst/login/login.do")

            else:
                PtDebugPrint("DEBUG: xVisitorPayDialog-->OnGUINotify -- Unknown Control Encountered")
        
       
