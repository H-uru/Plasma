/*==LICENSE==*

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

*==LICENSE==*/

#include "hsIniHelper.h"
#include "hsStringTokenizer.h"

hsIniEntry::hsIniEntry(ST::string line):
fCommand(), fComment() {
    if(line.size() == 0) {
        fType = kBlankLine;
    } else if(line[0] == '#') {
        fType = kComment;
        fComment = line.after_first('#');
    } else if(line == "\n") {
        fType = kBlankLine;
    } else {
        fType = kCommandValue;
        hsStringTokenizer tokenizer = hsStringTokenizer(line.c_str(), " ");
        char *str;
        int i = 0;
        while((str = tokenizer.next())) {
            if (i==0) {
                fCommand = str;
            } else {
                fValues.push_back(str);
            }
            i++;
        }
    }
}

void hsIniEntry::setValue(size_t idx, ST::string value) {
    if (fValues.size() >= idx) {
        fValues[idx] = value;
    } else {
        for (int i=fValues.size(); i<idx; i++) {
            fValues.push_back("");
        }
        fValues.push_back(value);
    }
}

std::optional<ST::string> hsIniEntry::getValue(size_t idx) {
    if (fValues.size() < idx) {
        return std::optional<ST::string>();
    } else {
        return fValues[idx];
    }
}


hsIniFile::hsIniFile(plFileName filename) {
    
    this->filename = filename;
    readFile();
}


hsIniFile::hsIniFile(hsStream& stream) {
    readStream(stream);
}

void hsIniFile::readStream(hsStream& stream) {
    ST::string line;
    while(stream.ReadLn(line)) {
        std::shared_ptr<hsIniEntry> entry = std::make_shared<hsIniEntry>(line);
        fEntries.push_back(entry);
    }
}

void hsIniFile::writeFile() {
    hsAssert(filename.GetSize() > 0, "writeFile requires contructor with filename");
    
    hsBufferedStream s;
    s.Open(filename, "w");
    writeStream(s);
    s.Close();
}

void hsIniFile::readFile() {
    hsAssert(filename.GetSize() > 0, "writeFile requires contructor with filename");
    
    fEntries.clear();
    
    hsBufferedStream s;
    s.Open(filename);
    readStream(s);
    s.Close();
}

void hsIniFile::writeFile(plFileName filename) {
    hsBufferedStream s;
    s.Open(filename, "w");
    writeStream(s);
    s.Close();
}

void hsIniFile::writeStream(hsStream& stream) {
    for (std::shared_ptr<hsIniEntry> entry: fEntries) {
        switch (entry->fType) {
            case hsIniEntry::kBlankLine:
                stream.WriteSafeString("\n");
                break;
            case hsIniEntry::kComment:
                stream.WriteSafeString("#" + entry.get()->fComment + "\n");
                break;
            case hsIniEntry::kCommandValue:
                ST::string line = entry->fCommand;
                for (ST::string value: entry->fValues) {
                    line += " " + value;
                }
                line += "\n";
                stream.WriteString(line);
                break;
        }
    }
}

std::shared_ptr<hsIniEntry> hsIniFile::findByCommand(ST::string command) {
    for (std::shared_ptr<hsIniEntry> entry: fEntries) {
        if(entry->fCommand == command) {
            return entry;
        }
    }
    return std::shared_ptr<hsIniEntry>(nullptr);
}
