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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnIni/Private/pnIniChange.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNINI_PRIVATE_PNINICHANGE_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnIni/Private/pnIniChange.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNINI_PRIVATE_PNINICHANGE_H


/*****************************************************************************
*
*   Change notifications for files in the "./Config" directory
*
***/

struct IniChangeReg;

typedef void (* FIniFileChangeCallback)(const wchar fullPath[]);

void IniChangeInitialize (
	const wchar				dir[] = L"Config"
);
void IniChangeDestroy ();

void IniChangeAdd (
    const wchar             filename[],     // just filename, no path or extension
    FIniFileChangeCallback  callback,
    IniChangeReg **         reg
);
void IniChangeRemove (
    IniChangeReg *          reg,
    bool                    wait
);
void IniChangeSignal (
    IniChangeReg *          reg,
    bool                    wait
);
