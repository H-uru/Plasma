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
