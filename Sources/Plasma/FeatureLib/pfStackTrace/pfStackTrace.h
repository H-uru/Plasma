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
#ifndef _DEV_STACKTRACE_H
#define _DEV_STACKTRACE_H


namespace dev
{


class MapFile;


/** Stack tracing utility. */
class StackTrace
{
public:
	/**
	 * Prints formatted call stack to the user buffer.
	 * Always terminates the user buffer with 0.
	 *
	 * @param map Array of pointers to map files.
	 * @param maps Number of map files.
	 * @param initLevel Number of functions to skip before starting the tracing.
	 * @param maxDepth Maximum number of levels in the stack trace.
	 * @param buffer [out] Output buffer for the formatted stack trace.
	 * @param bufferSize Size of the output buffer.
	 * @return Needed buffer size.
	 */
	static int	printStackTrace( MapFile** map, int maps,
					int initLevel, int maxDepth,
					char* buffer, int bufferSize, unsigned long stackPtr = 0, unsigned long opPtr = 0 );
};


} // dev


#endif // _DEV_STACKTRACE_H
/*
 * Copyright (c) 2001 Jani Kajala
 *
 * Permission to use, copy, modify, distribute and sell this
 * software and its documentation for any purpose is hereby
 * granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Jani Kajala makes no representations about the suitability 
 * of this software for any purpose. It is provided "as is" 
 * without express or implied warranty.
 */
