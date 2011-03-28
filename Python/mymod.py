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
"""Module: mymod
Age: global
Author: Mark DeForest
Date: Jan. 21, 2002
This is an example module based of a ptModifier.
To be used with Plasma20, Python File Component
"""

from Plasma import *
from PlasmaTypes import *
import cPickle

#=============================================================
# define the attributes that will be entered in max
#=============================================================
myinteger = ptAttribInt(1,'My integer',10)
openb = ptAttribBoolean(2, 'Open?')
thenumber = ptAttribFloat(3, 'The special number', 1.5, (1.0, 9.0))
colorname = ptAttribString(4, 'What is your favorite color?', 'Blue')
object = ptAttribSceneobject(5, 'Pick a sceneobject')
activator = ptAttribActivator(6, 'Activate me')
door = ptAttribResponder(7, 'A responder to run',['open','close'])
marquee_map = ptAttribDynamicMap(8, "Dynamic Texture Map for Marquee")
marquee_string = ptAttribString(9, "Marquee text")
marquee_speed = ptAttribFloat(10, "Marquee speed (in seconds)", 0.25)
doDance = ptAttribBehavior(11, "Dance behavior")

#----------
# globals
#----------
theobject = None
blue = ptColor().blue()
cyan = ptColor().cyan()
white = ptColor().white()
black = ptColor().black()

#----------
# constants
#----------
kMarqueeTimerId = 1

#====================================
# This is the class where my code is
#====================================
class mymod(ptModifier):
    "Example ptModifier"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 22332
        self.svMyNumber = 3
        self._scrollingtext = ""
        self.helperslist = []

    def OnFirstUpdate(self):
        "On the first update, do some initialization"
        self._scrollingtext = marquee_string.value
        PtAtTimeCallback(self.key,marquee_speed.value,kMarqueeTimerId)
        marquee_map.textmap.setClipping(0,0,128,128)
        marquee_map.textmap.setTextColor(cyan)
        marquee_map.textmap.setFont("Comic Sans MS", 16)

    def OnNotify(self,state,id,events):
        "Notify: should only be from the activator"
        global activator
        global openb
        global door
        global theobject
        PtDebugPrint("mymod: Notify event state=%f,id=%d,events=" % (state,id),events)
        # is this our activator notifying us?
        if state and id == activator.id:
            # first have the player do a little dance
            doDance.run(PtFindAvatar(events))

            # yes, then run the responder
            if openb.value:
                door.run(self.key,state='open')
                theobject = object.value
                self.svMyNumber += 1
                openb.value = false
            else:
                door.run(self.key,state='close')
                openb.value = true
            # get the avatar to create a myHelper class
            # find avatar
            for event in events:
                if event[0] == kCollisionEvent:
                    self.helperslist.append(myHelper(event[2]))   # save the last hitter
                if event[0] == kPickedEvent:
                    self.helperslist.append(myHelper(event[2]))   # save the last hitter
            # test the Save function
            savefile = open('pfile.txt','w')
            self.Save(savefile)
            savefile.close()

    def OnTimer(self,id):
        "Timer event: should only be to update the marquee"
        #PtDebugPrint("mymod: timer event id=%d" % (id))
        # if this the marquee update?
        if id == kMarqueeTimerId:
            #PtDebugPrint("mymod: marquee timer hit")
            # first erase the last thing we put up
            marquee_map.textmap.fillRect( 0, 0, 128, 128, black )
            marquee_map.textmap.drawText( 0, 0, self._scrollingtext )
            marquee_map.textmap.flush()
            # rotate the text
            self._scrollingtext = self._scrollingtext[1:] + self._scrollingtext[:1]
            # restart the timer
            PtAtTimeCallback(self.key,marquee_speed.value,kMarqueeTimerId)
        else:
            for helper in helperslist:
                helper.bump_count

    def Save(self,savefile):
        "Save variables that we need to be persistent"
        print "Save variables"
        cPickle.dump(self.svMyNumber,savefile)
        cPickle.dump(openb.value,savefile)
        # this will recursively go through all the elements of the list and pickle each item
        #   ... which should call __getinitargs__ and save that to construct a new one in the future
        cPickle.dump(self.helperslist,savefile)

    def Load(self, loadfile):
        "Load the persistent variables"
        print "Load variables"
        self.svMyNumber = cPickle.load(loadfile)
        openb.value = cPickle.load(loadfile)
        # this will recreate a list of myHelper objects
        self.helperslist = cPickle.load(loadfile)

    def OnPageLoad(self,what,who):
        if what==kLoaded:
            print "%s is finished loading" % (who)
        elif what == kUnloaded:
            print "%s is finished unloading" % (who)

#====================================
# Helper class
#====================================
class myHelper:
    "Just a class to show pickling of a class of our own making"
    def __init__(self,avatar,count=0,name=None):
        "constructor for a myHelper class"
        self.avatar = avatar
        self.count = count
        self.name = name

    def __getinitargs__(self):
        "get constructor arguments for Pickling support"
        return (self.avatar,self.count,self.name)

    def bump_count(self):
        "increment the counter"
        self.count += 1
