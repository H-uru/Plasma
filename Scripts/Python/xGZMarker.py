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
Module: xGZMarker
Age: global
Date: Nov. 6, 2003
Author: Mark DeForest
This is what is put on the GZ markers for the player to capture to calibrate their KIs
"""
MaxVersionNumber = 2
MinorVersionNumber = 1

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *

aStringVarName = ptAttribString(1,"Age SDL Vis Name")
aGZSerialNumber = ptAttribInt(2,"GZ a unique serial number (1 thru n)")
aGZRegionVis = ptAttribActivator(3,"Region detector when marker is inRange")
aGZSoundResponder = ptAttribResponder(4, "Sound responder for marker",statelist=["SoundOn","SoundOff"])


AgeStartedIn = None


kChronicleGZMarkersAquired = "GZMarkersAquired"

gSoundRespWasCalled = 0


class xGZMarker(ptMultiModifier):

    def __init__(self):
        ptMultiModifier.__init__(self)
        self.id = 205
        self.version = MaxVersionNumber
        PtDebugPrint("__xGZMarker: Max version %d - minor version %d" % (MaxVersionNumber,MinorVersionNumber),level=kDebugDumpLevel)

    def OnFirstUpdate(self):
        global AgeStartedIn
        AgeStartedIn = PtGetAgeName()
        # not much to do here
            
    def OnServerInitComplete(self):
        # we are initially disabled
        self.DisableObject()

    def IsSDLEnabled(self):
        "Test to see if the SDL is enabled"
        if AgeStartedIn != PtGetAgeName():
            return 0
        ageSDL = PtGetAgeSDL()
        try:
            PtDebugPrint("xGZMarker: SDL is %d" % (ageSDL[aStringVarName.value][0]),level=kDebugDumpLevel)
            return ageSDL[aStringVarName.value][0]
        except:
            pass
        return 0

    def IsMarkerAvailable(self):
        # setting the max in the GZ marker chronicle
        vault = ptVault()
        # is there a chronicle for the GZ games?
        entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
        if entry is not None:
            markers = entry.getValue()
            markerIdx = aGZSerialNumber.value - 1
            if markerIdx >= 0 and markerIdx < len(markers):
                if markers[markerIdx] == kGZMarkerAvailable:
                    # we're active!
                    PtDebugPrint("xGZMarker: marker %d available"%(aGZSerialNumber.value),level=kDebugDumpLevel)
                    return 1
                else:
                    PtDebugPrint("xGZMarker: marker not available - at %d is a '%s'"%(aGZSerialNumber.value,markers[markerIdx]),level=kDebugDumpLevel)
            else:
                PtDebugPrint("xGZMarker - ERROR marker serial number invalid (%d) " % (aGZSerialNumber.value))
                PtDebugPrint("xGZMarker - current markers are '%s'" % (markers))
        else:
            PtDebugPrint("xGZMarker - ERROR marker chronicle not found ")
        return 0

    def EnableObject(self):
        PtDebugPrint("DEBUG: xGZMarker.EnableObject:  Attempting to enable drawing and collision on %s..." % self.sceneobject.getName(),level=kDebugDumpLevel)
        self.sceneobject.draw.enable()
        self.sceneobject.physics.suppress(False)

    def DisableObject(self):
        PtDebugPrint("DEBUG: xGZMarker.DisableObject:  Attempting to disable drawing and collision on %s..." % self.sceneobject.getName(),level=kDebugDumpLevel)
        self.sceneobject.draw.disable()
        self.sceneobject.physics.suppress(True)

    def OnNotify(self,state,id,events):
        "Notify from region sensor or from the KI"
        global gSoundRespWasCalled

        # if this is from the region sensor, then determine if this
        if id == aGZRegionVis.id:
            for event in events:
                if event[0] == kCollisionEvent:
                    if event[2] == PtGetLocalAvatar():
                        if event[1] == 1:
                            PtDebugPrint("xGZMarker: enter region",level=kDebugDumpLevel)
                            if self.IsSDLEnabled():
                                if self.IsMarkerAvailable():
                                    # make the marker visible
                                    self.EnableObject()
                                    aGZSoundResponder.run(self.key, state="SoundOn",netPropagate=0)
                                    gSoundRespWasCalled = 1
                                    PtSendKIGZMarkerMsg(aGZSerialNumber.value,self.key)
                        else:
                            PtDebugPrint("xGZMarker: exit region",level=kDebugDumpLevel)
                            # just disable it
                            self.DisableObject()
                            if gSoundRespWasCalled:
                                aGZSoundResponder.run(self.key, state="SoundOff",netPropagate=0)
                                gSoundRespWasCalled = 0
                            PtSendKIMessage(kGZOutRange,0)
        elif id == -1:
            for event in events:
                if event[0] == kVariableEvent and event[1] == "Captured":
                    self.DisableObject()
                    if gSoundRespWasCalled:
                        aGZSoundResponder.run(self.key, state="SoundOff",netPropagate=0)
                        gSoundRespWasCalled = 0
