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
#include "plLoc.h"
#include "../pnKeyedObject/hsKeyedObject.h"


plLocFileParser::plLocFileParser(): fThrowBack(nil), fThrowBackLevel(0) 	
{	fpFile = fopen(LOCTXTFILE,"rt");			// NEED to figure out where to put this
}

int plLocFileParser::NextLine(char **pP)
{
	
	int levelKnt=0;
	*pP = 0;
		// If someone previously threwback a line...use it...
	if (ThrowBackAvailable())	
	{	*pP = fThrowBack;
		int rtnLevel = fThrowBackLevel;
		ClearThrowBack();
		return rtnLevel;
	}
		// Get a New Line from the file
	char str[512];
	while(fgets(str,512 - 1,fpFile))
	{
		if (*str == '#')				// indicates a missing file message, ignore
			continue;
		
		int len = strlen(str);
		str[len - 1] = 0;			// clobber new line
		//----------------------------------------------
		// If its the database file, remember the name, and skip it.
		//----------------------------------------------
		int i=0;
		while (str[i] == '\t')
		{	levelKnt++;
			i++;

		}
		*pP = hsStrcpy(str + levelKnt);	// Allocate a string copy, advance past TABS
		return levelKnt;
	}
	return LOC_EOF;
}
	

	




