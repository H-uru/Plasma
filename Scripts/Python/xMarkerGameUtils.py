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

 *==LICENSE==*
"""

from __future__ import annotations
import ast
import dataclasses
from contextlib import contextmanager
from typing import *

from Plasma import *
from PlasmaGame import *
from PlasmaGameConstants import *
from PlasmaTypes import *

import grtzMarkerGames

kLegacyChronicleName = "MarkerGameData"

@dataclasses.dataclass
class LegacyMarkerChronicle:
    """The singlular marker data chronicle used by the GameMgr code."""
    svrGameTypeID: int = -1
    svrGameTemplateID: str = ""
    CGZGameNum: int = -1
    svrGameClientID: int = -1
    numMarkers: int = 0
    numCapturedMarkers: int = 0
    isPlayerJoined: int = 0
    svrGameName: str = ""
    svrGameStarted: int = 0


def GetLegacyChronicle() -> LegacyMarkerChronicle:
    vault = ptVault()
    if chron := vault.findChronicleEntry(kLegacyChronicleName):
        value = chron.getValue().strip()
        if not value:
            return LegacyMarkerChronicle()
        try:
            data = ast.literal_eval(value)
        except SyntaxError:
            PtDebugPrint(f"xMarkerGameUtils.GetLegacyChronicle():\tSyntax error in: {value=}")
            return LegacyMarkerChronicle()

        if isinstance(data, dict):
            return LegacyMarkerChronicle(**data)
        PtDebugPrint(f"GetLegacyCronicle():\tThe chronicle {kLegacyChronicleName} did not contain a dict.")
    return LegacyMarkerChronicle()

def SaveLegacyChronicle(data: Optional[LegacyMarkerChronicle]):
    value = repr(dataclasses.asdict(data)) if data else ""
    ptVault().addChronicleEntry(
        kLegacyChronicleName,
        1,
        value
    )

@contextmanager
def LegacyMarkerData() -> LegacyMarkerChronicle:
    markerData = GetLegacyChronicle()
    try:
        yield markerData
    finally:
        SaveLegacyChronicle(markerData)

@dataclasses.dataclass
class CGZMTimes:
    start_time: Optional[int] = None
    best_time: Optional[int] = None

    def __str__(self) -> str:
        if self.start_time is not None and self.best_time is not None:
            return f"{self.start_time},{self.best_time}"
        elif self.start_time is not None:
            return f"{self.start_time}"
        else:
            return ""


def GetMarkerDataChronicle() -> ptVaultChronicleNode:
    vault = ptVault()
    if entry := vault.findChronicleEntry("MarkerBrain"):
        return entry

    vault.addChronicleEntry("MarkerBrain", 1, "")
    return vault.findChronicleEntry("MarkerBrain")

def FindNewStyleChronicle(name: str, *, create: bool = False) -> Optional[ptVaultChronicleNode]:
    root = GetMarkerDataChronicle()
    searchTemplate = ptVaultChronicleNode()
    searchTemplate.setName(name)
    if chron := root.findNode(searchTemplate):
        return chron.upcastToChronicleNode()
    if create:
        # Yeah, it's not a search template anymore. It's now a real boy^H^H^Hchronicle.
        root.addNode(searchTemplate)
        return searchTemplate
    return None

def GetNewStyleCaptureChronicle(name: str) -> ptVaultChronicleNode:
    questChron = FindNewStyleChronicle("Quest", create=True)
    captureTemplate = ptVaultChronicleNode()
    captureTemplate.setName(name)
    if chron := questChron.findNode(captureTemplate):
        return chron.upcastToChronicleNode()
    questChron.addNode(captureTemplate)
    return questChron

def GetNewStyleActiveQuest() -> Optional[int]:
    chron = GetMarkerDataChronicle()
    if chron.getValue() != "quest":
        PtDebugPrint("xMarkerGameUtils.GetNewStyleActiveQuest():\tMarkerBrain is not in quest mode", level=kDebugDumpLevel)
        return None
    if chron := FindNewStyleChronicle("ActiveQuest"):
        try:
            value = int(chron.getValue().strip())
        except ValueError:
            PtDebugPrint("xMarkerGameUtils.GetNewStyleActiveQuest():\tActiveQuest chronicle contained nonsense.")
        else:
            if value > 0:
                return value
    else:
        PtDebugPrint("xMarkerGameUtils.GetNewStyleActiveQuest():\tActiveQuest chronicle doesn't exist.", level=kDebugDumpLevel)
    return None

def SetNewStyleActiveQuest(name: Optional[str] = None) -> None:
    if name is None:
        name = "-1"
    chron = GetMarkerDataChronicle()
    chron.setValue("quest" if name != "-1" else "")
    chron = FindNewStyleChronicle("ActiveQuest", create=True)
    chron.setValue(name)

def GetCurrentTime() -> int:
    # No, your eyes do not deceive you. For some reason, the Cyan programmer who wrote the
    # CGZM code for MOUL elected to use the cavern time for the CGZM start and end times.
    # Why does this matter, you ask? Because the cavern time has DST, making it possible
    # for you to have a NEGATIVE completion time for CGZMs if you time it just right. Joy.
    # Not that you can generate a leaderboard on MOULa anyway, so it's moot point.
    if ptGmMarker.isSupported():
        return PtGetDniTime()
    else:
        return PtGetServerTime()

def GetCurrentCGZM() -> int:
    if ptGmMarker.isSupported():
        chron = GetLegacyChronicle()
        return chron.CGZGameNum
    elif chron := FindNewStyleChronicle("CGZ-Mission"):
        try:
            return int(chron.getValue().strip())
        except ValueError:
            pass
    return -1

def SetCurrentCGZM(missionID: int) -> None:
    if ptGmMarker.isSupported():
        chron = GetLegacyChronicle()
        chron.CGZGameNum = missionID
        SaveLegacyChronicle(chron)
    else:
        chron = GetMarkerDataChronicle()
        chron.setValue("quest" if missionID == -1 else "cgz")
        chron = FindNewStyleChronicle("CGZ-Mission", create=True)
        chron.setValue(str(missionID))

def GetCGZMTimes(missionID: int, *, forceLegacy: bool = False) -> CGZMTimes:
    vault = ptVault()
    if forceLegacy or ptGmMarker.isSupported():
        if chron := vault.findChronicleEntry(f"MG{missionID+1:02}"):
            data = filter(None, (i.strip() for i in chron.getValue().split(",")))

            # The legacy MOULa client stored these times as floats in the chronicle, but the game
            # returns whole seconds. Therefore, we're exposing ints. However, we have to manually
            # parse as float from the string before going to int. Otherwise, Python errors.
            startTime = int(float(next(data, 0.0)))
            bestTime = int(float(next(data, 0.0)))
            if next(data, None) is not None:
                PtDebugPrint("GetCGZMTimes():\tHmmm... unexpected extra data in the CGZM chronicle.")
            return CGZMTimes(startTime, bestTime)
    else:
        if missionID != GetCurrentCGZM():
            raise RuntimeError("You can only get the times for the current CGZM.")

        # Only start times can be retrieved for now.
        if startTimeChron := FindNewStyleChronicle("CGZ-StartTime"):
            try:
                startTime = int(startTimeChron.getValue().strip())
            except ValueError:
                pass
            else:
                return CGZMTimes(startTime)

    return CGZMTimes(0, 0)

def GetLegacyCGZMScore(missionID: int) -> int:
    times = GetCGZMTimes(missionID, forceLegacy=True)
    if score := times.best_time:
        return score
    return -1

def SetCGZMTimes(missionID: int, newTimes: CGZMTimes, *, legacyOnly: bool = False):
    # Always set the legacy stuff, just to be safe.
    prevTimes = GetCGZMTimes(missionID, forceLegacy=True)
    if newTimes.start_time is not None:
        prevTimes.start_time = newTimes.start_time
    if newTimes.best_time is not None:
        prevTimes.best_time = newTimes.best_time

    ptVault().addChronicleEntry(
        f"MG{missionID+1:02}",
        1,
        str(prevTimes)
    )

    # Set the new thing, if applicable.
    if not legacyOnly and not ptGmMarker.isSupported():
        if not missionID != GetCurrentCGZM():
            raise RuntimeError("Can only set the times for the current CGZM.")
        if newTimes.start_time is not None:
            startTimeChron = FindNewStyleChronicle("CGZ-StartTime", create=True)
            startTimeChron.setValue(str(newTimes.start_time))

def IsCGZMComplete() -> bool:
    missionID = GetCurrentCGZM()
    if missionID == -1:
        return False
    if ptGmMarker.isSupported():
        if markerData := GetLegacyChronicle():
            if markerData.svrGameTypeID == PtMarkerGameType.kMarkerGameCGZ:
                return markerData.numMarkers <= markerData.numCapturedMarkers
        return False
    else:
        chron = GetNewStyleCaptureChronicle("cgz")
        caps = set(filter(None, (i.strip() for i in chron.getValue().split(','))))
        return len(grtzMarkerGames.mgs[missionID]) <= len(caps)

