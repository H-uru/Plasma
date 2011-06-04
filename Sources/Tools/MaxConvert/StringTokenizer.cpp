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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
// StringTokenizer.cpp

#include "StringTokenizer.h"
#include "string.h"

// String Tokenizer routines
StringTokenizer::StringTokenizer() {
	qAsTok = true;
	inQuote = false;
	this->string = this->seps = 0;
}
StringTokenizer::StringTokenizer(const char *string, const char *seps) {
	qAsTok = true;
	inQuote = false;
	this->string = TRACKED_NEW char[strlen(string)+1];
	strcpy(this->string,string);
	numSeps = strlen(seps);
	this->seps = TRACKED_NEW char[numSeps+1];
	strcpy(this->seps,seps);
	this->tok = this->string;
	if (isSep(*tok)) next();
};
StringTokenizer::~StringTokenizer() {
	delete string;
	delete seps;
}
hsBool StringTokenizer::hasMoreTokens() {
	return (*tok != '\0');
};
char *StringTokenizer::next() {
	if (*tok == '\0') return NULL;
	char *cur = tok;
	while (*tok != '\0' && !isSep(*tok)) tok++;
	if (*tok != '\0') {
		*tok = '\0';
		tok++;
	}
	while (*tok != '\0' && isSep(*tok)) tok++;
	return cur;
};
hsBool StringTokenizer::isSep(char c) {
	if (!qAsTok || !inQuote) {
		for (Int32 i=0; i<numSeps; i++) {
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
	this->string = TRACKED_NEW char[strlen(string)+1];
	strcpy(this->string,string);
	if (this->seps) delete this->seps;
	numSeps = strlen(seps);
	this->seps = TRACKED_NEW char[numSeps+1];
	strcpy(this->seps,seps);
	this->tok = this->string;
	if (isSep(*tok)) next();
}

void StringTokenizer::ParseQuotes(hsBool qAsTok) {
	this->qAsTok = qAsTok;
}
