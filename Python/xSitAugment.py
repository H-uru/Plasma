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
Module: xSitAugment
Age: global
Date: May 2002
Author: Pete Gage
allows user to select what sit component messages get passed on to another script
"""

from Plasma import *
from PlasmaTypes import *
import PlasmaControlKeys

# define the attributes that will be entered in max
SitAct = ptAttribActivator(1, " Sit Component ",netForce=1)
Camera = ptAttribSceneobject(2," Sit Camera ")
trig0 = ptAttribBoolean(3,"Trigger at Start Sit",0)
trig1 = ptAttribBoolean(4,"Trigger at End Sit",1)
trig2 = ptAttribBoolean(5,"Trigger at Start Stand",0)
trig3 = ptAttribBoolean(6,"Trigger at End Stand",0)
untrig0 = ptAttribBoolean(7,"Untrigger at Start Sit",0)
untrig1 = ptAttribBoolean(8,"Untrigger at End Sit",0)
untrig2 = ptAttribBoolean(9,"Untrigger at Start Stand",1)
untrig3 = ptAttribBoolean(10,"Untrigger at End Stand",0)
resp0 = ptAttribResponder(11, "optional responder @ start sit")
resp1 = ptAttribResponder(12, "optional responder @ end sit")
resp2 = ptAttribResponder(13, "opt. responder @ start stand")
resp3 = ptAttribResponder(14, "opt. responder @ end stand")

# ---------
# globals

LocalAvatar = None

class xSitAugment(ptModifier):
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5116
        
        version = 1
        self.version = version
        print "__init__xSitAugment v.", version

    def OnFirstUpdate(self):
        pass

    def OnNotify(self,state,id,events):
        "Activated... "
        global LocalAvatar
        if state and id == SitAct.id:# and PtWasLocallyNotified(self.key):
            #LocalAvatar = PtFindAvatar(events)

            callback=0 # which stage are we at.
        
            virtCam = ptCamera()

            checktrig="trig%i" % callback
            if checktrig.value: # checkbox is true, set camera
                virtCam.save(Camera.sceneobject.getKey())

            checkuntrig="untrig%i" % callback
            if checkuntrig.value: # checkbox is true, unset camera
                virtCam.restore(Camera.sceneobject.getKey())

            checkresp="resp%i" % callback
            if checkresp.value != None: # responder for this stage. Play it.
                checkresp.run(self.key)

### below is scrap 
    def OnGUINotify(self,id,control,event):
        "Notifications from the vignette"
        print "GUI Notify id=%d, event=%d control=" % (id,event),control
        if control.getTagID() == kExit: #off
            self.IQuitDialog()
        #elif event == kDialogLoaded:
        #    # if the dialog was just loaded then show it
        #    control.show()


    def OnControlKeyEvent(self,controlKey,activeFlag):
        "Control key events... anything we're interested in?"
        print "Got controlKey event %d and its activeFlage is %d" % (controlKey,activeFlag)
        if controlKey == PlasmaControlKeys.kKeyExitMode:
            self.IQuitDialog()

    def IStartDialog(self):
        "Start the Dialog"
        global LocalAvatar
        Activate.disable() # disable the activator
        PtLoadDialog(Vignette.value,self.key)
        if ( PtIsDialogLoaded(Vignette.value) ):
            PtShowDialog(Vignette.value)
            print "dialog: %s goes up" % Vignette.value
        # get control key events
        PtEnableControlKeyEvents(self.key)

    def IQuitSitting(self):
        "Disengage and exit"
        global LocalAvatar
        # exit every thing
        if type(Vignette.value) != type(None) and Vignette.value != "":
            PtHideDialog(Vignette.value)
            print "Dialog: %s goes down" % Vignette.value
        else:
            print "WTH!!!"
        #disable the Control key events
        PtDisableControlKeyEvents(self.key)
        # re-enable the dialog for someone else to use
        Activate.enable()
