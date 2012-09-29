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

import re

# Plasma engine.
from PlasmaVaultConstants import *


## Helper class for autocompletion in the KI.
class AutocompleteState:

    def __init__(self):
        self.prefix = ""
        self.candidates = list()
        self.lastCandidateId = None
        self.space_match = re.compile("\\s+", re.UNICODE)
        self.word_match = re.compile("\\S+", re.UNICODE)

    def pickFirst(self, text, nameList):
        if not text or not nameList:
            return None

        names = set()

        for name in nameList:
            nospace = self.space_match.sub('', name.lower())
            names.add((nospace, name))

        text_lower = text.lower()
        words = [(m.start(), m.end()) for m in self.word_match.finditer(text_lower)]
        words.reverse()
        suffix = ""

        for (word_start, word_end) in words:
            suffix = text_lower[word_start:word_end] + suffix

            self.candidates = [normal for (nospace, normal) in names
                               if nospace.startswith(suffix)]

            if self.candidates:
                self.candidates.sort()
                self.lastCandidateId = 0
                self.prefix = text[:word_start]
                return self.prefix + self.candidates[0]

        return None

    def pickNext(self, text):
        if not text or not self.candidates or self.lastCandidateId is None:
            return None

        lastCandidate = self.candidates[self.lastCandidateId]
        if not text.endswith(lastCandidate):
            return None

        nextCandidateId = (self.lastCandidateId + 1) % len(self.candidates)
        self.lastCandidateId = nextCandidateId
        return self.prefix + self.candidates[nextCandidateId]


## Helper function to prioritize online players in lists.
def CMPplayerOnline(playerA, playerB):
    elPlayerA = playerA.getChild()
    elPlayerB = playerB.getChild()
    if elPlayerA is not None and elPlayerB is not None:
        if elPlayerA.getType() is PtVaultNodeTypes.kPlayerInfoNode and elPlayerB.getType() is PtVaultNodeTypes.kPlayerInfoNode:
            elPlayerA = elPlayerA.upcastToPlayerInfoNode()
            elPlayerB = elPlayerB.upcastToPlayerInfoNode()
            if elPlayerA.playerIsOnline() and elPlayerB.playerIsOnline():
                return cmp(elPlayerA.playerGetName().lower(), elPlayerB.playerGetName().lower())
            if elPlayerA.playerIsOnline():
                return -1
            if elPlayerB.playerIsOnline():
                return 1
            return cmp(elPlayerA.playerGetName().lower(), elPlayerB.playerGetName().lower())
    return 0


## Helper function to sort nodes according to modification date.
def CMPNodeDate(nodeA, nodeB):
    elNodeA = nodeA.getChild()
    elNodeB = nodeB.getChild()
    if elNodeA is not None and elNodeB is not None:
        if elNodeA.getModifyTime() > elNodeB.getModifyTime():
            return -1
        else:
            return 1
    return 0

