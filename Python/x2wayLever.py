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
Module: x2wayLever
Age: global
Date: April 2002
Author: Bill Slease
reusable 2-position lever
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
varstring = ptAttribString(1,"Lever Name")
actTo2  = ptAttribActivator(2,"Actvtr: Move to Pos #2")
respTo2 = ptAttribResponder(3,"Rspndr: Move to Pos #2")
actTo1  = ptAttribActivator(4,"Actvtr: Move to Pos #1")
respTo1 = ptAttribResponder(5,"Rspndr: Move to Pos #1")

class x2wayLever(ptResponder):

    def __init__(self):
        # run parent class init
        ptResponder.__init__(self)
        self.id = 5004
        
        version = 1
        self.version = version
        PtDebugPrint("__init__x2wayLever v.", version)

    def OnFirstUpdate(self):
        if self.SDL == None:
            PtDebugPrint("x2wayLever.OnFirstUpdate():\tERROR---missing SDL (%s)" % varstring.value)
            return

        self.SDL.setDefault("LvrPos",(0,)) # local only: set default lever position, Load() will correct if necessary
        actTo1.disable() # set default activator state, Load() will correct if necessary
        #PtDebugPrint("x2wayLever.OnFirstUpdate():\tself.SDL[LvrPos]=%d (%s)" % (self.SDL["LvrPos"][0],varstring.value))
    
    #Load() gets called after OnFirstUpdate IF anyone has updated the server's SDL (SDL.setDefault above is local only)
    def Load(self):
        leverPos = self.SDL["LvrPos"][0]
        if leverPos == 0:   # lever in position no.1
            actTo1.disable()
            actTo2.enable()
        else:               # lever in position no.2
            actTo1.enable()
            actTo2.disable()
        PtDebugPrint("x2wayLever.Load():\tself.SDL[LvrPos]=%d (%s)" % (self.SDL["LvrPos"][0],varstring.value))
        
    def OnNotify(self,state,id,events):
    
        if state:
            leverPos = self.SDL["LvrPos"][0]
            if id==actTo2.id or id==actTo1.id:
                if leverPos:
                    actTo1.disable()
                    respTo1.run(self.key,events=events) # play one-shot, lever anim and sound, enable pos 1 detector
                else:
                    actTo2.disable()
                    respTo2.run(self.key,events=events) # play one-shot, lever anim and sound, enable pos 2 detector
            elif id==respTo2.id or id==respTo1.id:
                leverPos = -(leverPos-1)
                self.SDL["LvrPos"] = (leverPos,)
                PtDebugPrint("x2wayLever.OnNotify:\tsending notify: '%s' moving to position %d" % (varstring.value,leverPos+1))
                note = ptNotify(self.key)
                note.setActivate(1.0)
                note.addVarNumber(varstring.value,leverPos)
                note.send()
                PtDebugPrint("x2wayLever.OnNotify():\tself.SDL[LvrPos]=%d (%s)" % (self.SDL["LvrPos"][0],varstring.value))
            else:
                PtDebugPrint("x2wayLever.OnNotify:\tERROR: unanticipated message source.")
            return
