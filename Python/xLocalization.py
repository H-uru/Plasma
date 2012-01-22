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
This module contains all the strings that need to localized (excluding the KI)
"""

from Plasma import *
from PlasmaVaultConstants import *
from PlasmaConstants import *

if PtGetLanguage() == PtLanguage.kEnglish:
    import xKIEnglish
    xKI = xKIEnglish
    
    import xRPSEnglish
    xRPS = xRPSEnglish
    
    import xNexusEnglish
    xNexus = xNexusEnglish
    
    import xACAEnglish
    xACA = xACAEnglish
    
    import xCensorEnglish
    xCensor = xCensorEnglish
    
    import xEnglish
    xGlobal = xEnglish
    
    import xJournalBookDefsEnglish
    xJournalBookDefs = xJournalBookDefsEnglish
    
    import xDynTextDefsEnglish
    xDynTextDefs = xDynTextDefsEnglish

elif PtGetLanguage() == PtLanguage.kFrench:
    import xKIFrench
    xKI = xKIFrench
    
    import xRPSFrench
    xRPS = xRPSFrench
    
    import xNexusFrench
    xNexus = xNexusFrench
    
    import xACAFrench
    xACA = xACAFrench
    
    import xCensorEnglish
    xCensor = xCensorEnglish
    
    import xFrench
    xGlobal = xFrench
    
    import xJournalBookDefsFrench
    xJournalBookDefs = xJournalBookDefsFrench
    
    import xDynTextDefsFrench
    xDynTextDefs = xDynTextDefsFrench

elif PtGetLanguage() == PtLanguage.kGerman:
    import xKIGerman
    xKI = xKIGerman
    
    import xRPSGerman
    xRPS = xRPSGerman
    
    import xNexusGerman
    xNexus = xNexusGerman
    
    import xACAGerman
    xACA = xACAGerman
    
    import xCensorEnglish
    xCensor = xCensorEnglish
    
    import xGerman
    xGlobal = xGerman
    
    import xJournalBookDefsGerman
    xJournalBookDefs = xJournalBookDefsGerman
    
    import xDynTextDefsGerman
    xDynTextDefs = xDynTextDefsGerman

elif PtGetLanguage() == PtLanguage.kSpanish:
    import xKISpanish
    xKI = xKISpanish
    
    import xRPSSpanish
    xRPS = xRPSSpanish
    
    import xNexusSpanish
    xNexus = xNexusSpanish
    
    import xACASpanish
    xACA = xACASpanish
    
    import xCensorEnglish
    xCensor = xCensorEnglish
    
    import xSpanish
    xGlobal = xSpanish
    
    import xJournalBookDefsSpanish
    xJournalBookDefs = xJournalBookDefsSpanish
    
    import xDynTextDefsSpanish
    xDynTextDefs = xDynTextDefsSpanish

elif PtGetLanguage() == PtLanguage.kItalian:
    import xKIItalian
    xKI = xKIItalian
    
    import xRPSItalian
    xRPS = xRPSItalian
    
    import xNexusItalian
    xNexus = xNexusItalian
    
    import xACAItalian
    xACA = xACAItalian
    
    import xCensorEnglish
    xCensor = xCensorEnglish
    
    import xItalian
    xGlobal = xItalian
    
    import xJournalBookDefsItalian
    xJournalBookDefs = xJournalBookDefsItalian
    
    import xDynTextDefsItalian
    xDynTextDefs = xDynTextDefsItalian

else:
    # default to english if we don't know the language
    import xKIEnglish
    xKI = xKIEnglish
    
    import xRPSEnglish
    xRPS = xRPSEnglish
    
    import xNexusEnglish
    xNexus = xNexusEnglish
    
    import xACAEnglish
    xACA = xACAEnglish
    
    import xCensorEnglish
    xCensor = xCensorEnglish
    
    import xEnglish
    xGlobal = xEnglish
    
    import xJournalBookDefsEnglish
    xJournalBookDefs = xJournalBookDefsEnglish
    
    import xDynTextDefsEnglish
    xDynTextDefs = xDynTextDefsEnglish


def FolderIDToFolderName(folderid):
    "Returns the standard folder name based on folder ID"
    try:
        return xGlobal.xFolderIDToFolderName[folderid]
    except LookupError:
        return "?UNKNOWN?"

def MemberStatusString():
    "returns a string of what type of neighborhood member this person is"
    ageInfo = ptAgeInfoStruct()
    ageInfo.setAgeFilename("Neighborhood")
    if ptVault().amAgeCzar(ageInfo):
        return xGlobal.xMayorOfNeighborhood
    return xGlobal.xMemberOfNeighborhood
