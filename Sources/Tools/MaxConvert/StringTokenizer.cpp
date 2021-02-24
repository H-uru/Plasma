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

#include <cstring>

#include "StringTokenizer.h"

// String Tokenizer routines
StringTokenizer::StringTokenizer() {
    qAsTok = true;
    inQuote = false;
    this->string = this->seps = nullptr;
}
StringTokenizer::StringTokenizer(const char *string, const char *seps) {
    qAsTok = true;
    inQuote = false;
    this->string = new char[strlen(string)+1];
    strcpy(this->string,string);
    numSeps = strlen(seps);
    this->seps = new char[numSeps+1];
    strcpy(this->seps,seps);
    this->tok = this->string;
    if (isSep(*tok)) next();
};
StringTokenizer::~StringTokenizer() {
    delete string;
    delete seps;
}
bool StringTokenizer::hasMoreTokens() {
    return (*tok != '\0');
};
char *StringTokenizer::next() {
    if (*tok == '\0')
        return nullptr;
    char *cur = tok;
    while (*tok != '\0' && !isSep(*tok)) tok++;
    if (*tok != '\0') {
        *tok = '\0';
        tok++;
    }
    while (*tok != '\0' && isSep(*tok)) tok++;
    return cur;
};
bool StringTokenizer::isSep(char c) {
    if (!qAsTok || !inQuote) {
        for (int i=0; i<numSeps; i++) {
            if (seps[i] == c) return true;
        }
    }
    if (qAsTok && c=='\"') {
        inQuote = !inQuote;
        return true;
    }
    return false;
};
void StringTokenizer::reset(const char *string, const char *seps) {
    if (this->string) delete this->string;
    this->string = new char[strlen(string)+1];
    strcpy(this->string,string);
    if (this->seps) delete this->seps;
    numSeps = strlen(seps);
    this->seps = new char[numSeps+1];
    strcpy(this->seps,seps);
    this->tok = this->string;
    if (isSep(*tok)) next();
}

void StringTokenizer::ParseQuotes(bool qAsTok) {
    this->qAsTok = qAsTok;
}
