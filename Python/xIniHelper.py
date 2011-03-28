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
"""
This module contains the routines to read and write an .ini file
"""

from Plasma import *

import os

kComment = 1
kBlankLine = 2
kCommandValue = 3
kIgnore = 4

class iniEntry:
    def __init__(self,line):
        # translate 'line' into data
        self.command = None
        self.values = []
        if len(line) == 0:
            # just ignore
            self.type = kBlankLine
        elif line[0] == '#':
            self.type = kComment
            self.comment = line[:-1]
        elif line[0] == '\n':
            self.type = kBlankLine
        else:
            self.type = kCommandValue
            words = line.split()
            if len(words) > 0:
                self.command = words[0]
                if len(words) > 1:
                    accStr = ""
                    for word in words[1:]:
                        if word[0] == '"':
                            accStr = word
                        elif word[-1:] == '"':
                            accStr += " "+word
                            self.values.append(accStr)
                            accStr = ""
                        elif len(accStr) > 0:
                            accStr += " "+word
                        else:
                            self.values.append(word)
                    if len(accStr) > 0:
                        self.values.append(accStr)
            else:
                self.type = kComment
                self.comment = "# <Uknown command> '%s'" % line

    def __repr__(self):
        if self.type == kBlankLine:
            return "[b]\n"
        elif self.type == kIgnore:
            return "[i]"
        elif self.type == kComment:
            return "[c]"+self.comment + "\n"
        elif self.type == kCommandValue:
            line = self.command
            for v in self.values:
                line += " " + v
            return "[v]"+line + "\n"
        else:
            return "[unknown entry]\n"

    def setValue(self,idx,value):
        if len(self.values) > idx:
            self.values[idx] = value
        else:
            for i in range(idx):
                if len(self.values) <= i:
                    self.values.append('')
            self.values.append(value)

    def getValue(self,idx):
        if len(self.values) > idx:
            return self.values[idx]
        return None

class iniFile:
    def __init__(self,filename):
        self.entries = []
        if filename:
            try:
                f = ptStream()
                f.open(filename,"r")
                lines = f.readlines()
                for l in lines:
                    self.entries.append(iniEntry(l))
                f.close()
            except:
                print "[INI processing] Error while reading %s" % (filename)

    def __repr__(self):
        line = ""
        for entry in self.entries:
            line += `entry`
        return line

    def isEmpty(self):
        if len(self.entries) == 0:
            return 1
        return 0

    def addEntry(self,line):
        self.entries.append(iniEntry(line))

    def removeEntry(self,idx):
        try:
            del self.entries[idx]
        except IndexError:
            pass

    def findByCommand(self,cmd,idx=0):
        idx = 0
        for entry in self.entries[idx:]:
            if entry.command == cmd:
                return entry,idx
            idx += 1
        return None,-1

    def findByFirstValue(self,value,idx=0):
        idx = 0
        for entry in self.entries[idx:]:
            if len(entry.values) > 0 and entry.values[0] == value:
                return entry,idx
            idx += 1
        return None,-1

    def findByLastValue(self,value,idx=0):
        idx = 0
        for entry in self.entries[idx:]:
            vlist = entry.values[-1:]
            if len(vlist) > 0:
                if entry.values[-1:][0] == value:
                    return entry,idx
            idx += 1
        return None,-1

    def findByAnyValue(self,value,idx=0):
        idx = 0
        for entry in self.entries[idx:]:
            for v in entry.values:
                if v == value:
                    return entry,idx
            idx += 1
        return None,-1

    def writeFile(self,filename):
        f = ptStream()
        f.open(filename,"w")
        lines = []
        for entry in self.entries:
            if entry.type == kBlankLine:
                lines.append(os.linesep)
            elif entry.type == kComment:
                lines.append(entry.comment+os.linesep)
            elif entry.type == kCommandValue:
                l = entry.command
                for v in entry.values:
                    l += " " + v
                lines.append(l+os.linesep)
        f.writelines(lines)
        f.close()
