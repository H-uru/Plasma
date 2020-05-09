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
# xRandom  - A module where you can specify the maximum number of
#            repetitions in a "random" series.
#
# May 2003 - Adam Van Ornum

import random
import math

_MAX_SERIES = 1
_MAX_ITERATIONS = 100
_series_length = 0
_lastvalue = None

def seed(var = 0):
    if isinstance(var, int):
        random.seed(var)
    else:
        random.seed()

def randint(start, stop):
    global _lastvalue
    global _series_length
    global _MAX_SERIES
    
    newInt = random.randint(start, stop)

    if _lastvalue is not None and newInt == _lastvalue:
        if _series_length >= _MAX_SERIES:
            iter = 0
            while newInt == _lastvalue and iter < _MAX_ITERATIONS:
                newInt = random.randint(start, stop)
                iter = iter + 1
            if newInt == _lastvalue and iter >= _MAX_ITERATIONS:
                raise RuntimeError("Problem with randomness: over max series length and can't find a new number")
            _lastvalue = newInt
            _series_length = 1
        else:
            _series_length = _series_length + 1
    else:
        _lastvalue = newInt
        _series_length = 1

    return _lastvalue

def setmaxseries(var = 2):
    global _MAX_SERIES
    
    if isinstance(var, int):
        _MAX_SERIES = var

def shuffle(theList):
    if isinstance(theList, list):
        n = len(theList)
        nmo = n - 1
        numIter = int(n * math.log(n))

        for x in range(numIter):
            idx1 = random.randint(0, nmo)
            idx2 = random.randint(0, nmo)

            while idx1 == idx2:
                idx1 = random.randint(0, nmo)
                idx2 = random.randint(0, nmo)

            theList[idx1], theList[idx2] = theList[idx2], theList[idx1]

class xRandom:
    def __init__(self, seed = 0, maxseries = 2):
        if isinstance(maxseries, int):
            self._MAX_SERIES = maxseries
        else:
            self._MAX_SERIES = 2

        self._series_length = 0
        self._lastvalue = None
        self._currentSequence = []
        self._range = None

    def seed(self, var = 0):
        random.seed(var)

    def randint(self, start, stop):
        newInt = random.randint(start, stop)

        if _lastvalue is not None and newInt == self._lastvalue:
            if self._series_length >= self._MAX_SERIES:
                while newInt == _lastvalue:
                    newInt = random.randint(start, stop)
                self._lastvalue = newInt
                self._series_length = 1
            else:
                self._series_length = self._series_length + 1
        else:
            self._lastvalue = newInt
            self._series_length = 1

        return _lastvalue

    def setmaxseries(self, var = 2):
        if isinstance(var, int):
            self._MAX_SERIES = var

    def setrange(self, rmin, rmax):
        self._range = ( min(rmin, rmax), max(rmin, rmax) )

    def getUniqueInt(self):
        numTries = 0
        if isinstance(self._range, tuple):
            newInt = random.randint(self._range[0], self._range[1])
            numTries += 1

            if (self._range[1] - self._range[0] + 1) <= len(self._currentSequence):
                self._currentSequence = [newInt]
                PtDebugPrint("numTries:", numTries)
                return newInt
            else:
                while newInt in self._currentSequence:
                    newInt = random.randint(self._range[0], self._range[1])
                    numTries += 1

                self._currentSequence.append(newInt)
                PtDebugPrint("numTries:", numTries)
                return newInt
        else:
            PtDebugPrint("numTries:", numTries)
            return None
            