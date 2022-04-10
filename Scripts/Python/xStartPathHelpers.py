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

from __future__ import annotations
from Plasma import *
from typing import Literal, Optional

kACAChronicle = "InitialAvCustomizationsDone"
kCleftChronicle = "CleftSolved"
kPathChronicle = "StartPathChosen"
kTutorialPathValue = "cleft"
kAdvancedPathValue = "relto"

def _GetPathValue() -> Optional[str]:
    start = ptVault().findChronicleEntry(kPathChronicle)
    if start is None:
        return None

    value = start.chronicleGetValue()
    assert value in {kTutorialPathValue, kAdvancedPathValue}, f"Invalid path value {value}"
    return value

def IsTutorialPath() -> bool:
    """Returns True if the player's journey starts in the Cleft."""
    return _GetPathValue() == kTutorialPathValue

def IsAdvancedPath() -> bool:
    """Returns True if the player's journey starts in Relto."""
    value = _GetPathValue()

    # Old characters will lack the path chronicle, so they go on the advanced path.
    return value is None or value == kAdvancedPathValue

def IsCleftSolved() -> bool:
    """Determines whether or not the Cleft has been solved."""
    entry = ptVault().findChronicleEntry(kCleftChronicle)
    return entry is not None and entry.chronicleGetValue() == "yes"

def SelectPath(path: Literal[kTutorialPathValue, kAdvancedPathValue]) -> None:
    assert path in {kTutorialPathValue, kAdvancedPathValue}, "Path must be cleft or relto"
    # It's ok if the path chronicle already exists, this will check for an existing chronicle
    # and change the value.
    ptVault().addChronicleEntry(kPathChronicle, 1, path)

def StartInACA() -> bool:
    """Returns if the first Age the player links to should be AvatarCustomization."""
    entry = ptVault().findChronicleEntry(kACAChronicle)
    return entry is None or entry.chronicleGetValue() != "1"

def StartInCleft() -> bool:
    """Returns if the first Age the player links to should be Cleft. This means that the player
       does not have a Relto book yet!"""
    return not StartInACA() and IsTutorialPath() and not IsCleftSolved()

def StartInRelto() -> bool:
    """Returns if the first Age the player links to should be Relto."""
    return not StartInACA() and (IsAdvancedPath() or IsCleftSolved())
