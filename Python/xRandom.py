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
# xRandom  - A module where you can specify the maximum number of
#            repetitions in a "random" series.
#
# May 2003 - Adam Van Ornum

import whrandom
import math

_MAX_SERIES = 1
_MAX_ITERATIONS = 100
_series_length = 0
_lastvalue = None

def seed(var = 0):
    if type(var) == type(1):
        whrandom.seed(var)
    else:
        whrandom.seed()

def randint(start, stop):
    global _lastvalue
    global _series_length
    global _MAX_SERIES
    
    newInt = whrandom.randint(start, stop)

    if type(_lastvalue) != type(None) and newInt == _lastvalue:
        if _series_length >= _MAX_SERIES:
            iter = 0
            while newInt == _lastvalue and iter < _MAX_ITERATIONS:
                newInt = whrandom.randint(start, stop)
                iter = iter + 1
            if newInt == _lastvalue and iter >= _MAX_ITERATIONS:
                raise "Problem with randomness: over max series length and can't find a new number"
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
    
    if type(var) == type(1):
        _MAX_SERIES = var

def shuffle(theList):
    if type(theList) == type([]):
        n = len(theList)
        nmo = n - 1
        numIter = int(n * math.log(n))

        for x in range(numIter):
            idx1 = whrandom.randint(0, nmo)
            idx2 = whrandom.randint(0, nmo)

            while idx1 == idx2:
                idx1 = whrandom.randint(0, nmo)
                idx2 = whrandom.randint(0, nmo)

            theList[idx1], theList[idx2] = theList[idx2], theList[idx1]

class xRandom:
    def __init__(self, seed = 0, maxseries = 2):
        if type(maxseries) == type(1):
            self._MAX_SERIES = maxseries
        else:
            self._MAX_SERIES = 2

        self._series_length = 0
        self._lastvalue = None
        self._currentSequence = []
        self._range = None

    def seed(self, var = 0):
        whrandom.seed(var)

    def randint(self, start, stop):
        newInt = whrandom.randint(start, stop)

        if type(_lastvalue) != type(None) and newInt == self._lastvalue:
            if self._series_length >= self._MAX_SERIES:
                while newInt == _lastvalue:
                    newInt = whrandom.randint(start, stop)
                self._lastvalue = newInt
                self._series_length = 1
            else:
                self._series_length = self._series_length + 1
        else:
            self._lastvalue = newInt
            self._series_length = 1

        return _lastvalue

    def setmaxseries(self, var = 2):
        if type(var) == type(1):
            self._MAX_SERIES = var

    def setrange(self, rmin, rmax):
        self._range = ( min(rmin, rmax), max(rmin, rmax) )

    def getUniqueInt(self):
        numTries = 0
        if type(self._range) == type( (0,) ):
            newInt = whrandom.randint(self._range[0], self._range[1])
            numTries += 1

            if (self._range[1] - self._range[0] + 1) <= len(self._currentSequence):
                self._currentSequence = [newInt]
                print "numTries:", numTries
                return newInt
            else:
                while newInt in self._currentSequence:
                    newInt = whrandom.randint(self._range[0], self._range[1])
                    numTries += 1

                self._currentSequence.append(newInt)
                print "numTries:", numTries
                return newInt
        else:
            print "numTries:", numTries
            return None
            