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
# Enum class
# Written by Will Ware, 2001/08/23
# Modified by Adam Van Ornum, 4/17/2003

import types, string, exceptions

class EnumException(exceptions.Exception):
    pass

class Enum:
    def __init__(self, enumStr):
        lookup = { }
        i = 0
        uniqueNames = [ ]
        uniqueValues = [ ]
        enumList = enumStr.split(",")
        for x in enumList:
            x = x.split("=")
            if len(x) == 2:
                x = ( x[0].strip(), int(x[1].strip()) )
            else:
                x = x[0].strip()
                
            if type(x) == types.TupleType:
                x, i = x
            if type(x) != types.StringType:
                raise EnumException, "enum name is not a string: " + x
            if type(i) != types.IntType:
                raise EnumException, "enum value is not an integer: " + i
            if x in uniqueNames:
                raise EnumException, "enum name is not unique: " + x
            if i in uniqueValues:
                raise EnumException, "enum value is not unique for " + x
            uniqueNames.append(x)
            uniqueValues.append(i)
            lookup[x] = i
            i = i + 1

        self.lookup = lookup


    def __getattr__(self, attr):
        if not self.lookup.has_key(attr):
            raise AttributeError
        return self.lookup[attr]

    def __getitem__(self, sub):
        if not self.lookup.has_key(sub):
            raise AttributeError
        return self.lookup[sub]

    def __len__(self):
        return len(self.lookup)

    def Length(self):
        return len(self.lookup)

    def ToString(self, x):
        for key in self.lookup.keys():
            if self.lookup[key] == x:
                return key

if __name__ == "__main__":
    animal = Enum("Cow, Pig, Dog = 5, Cat, Lizard")

    print animal.Cow
    print animal["Cow"]
    print animal.Pig
    print animal.Dog
    print animal.Cat
    print animal.Lizard
