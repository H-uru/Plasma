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
#ifndef _DEV_MAPFILEENTRY_H
#define _DEV_MAPFILEENTRY_H


namespace dev
{


/** An entry in the map file. */
class MapFileEntry
{
public:
	/** Class constants. */
	enum Constants
	{
		/** Maximum number of characters in map file entry name. */
		MAX_NAME = 256
	};

	///
	MapFileEntry();

	/** Creates an entry with specified section, offset, length and name. */
	MapFileEntry( long section, long offset, long length, const char* name );

	/** Returns section of the entry. */
	long		section() const;

	/** Returns offset of the entry. */
	long		offset() const;

	/** Returns length of the entry (only defined for segments). */
	long		length() const;

	/** Returns name of the entry. */
	const char*	name() const;

	/** Returns true if the offset of this entry is before the other one. */
	bool		operator<( const MapFileEntry& other ) const;

private:
	long	m_sec;
	long	m_addr;
	long	m_len;
	char	m_name[MAX_NAME+1];
};


} // dev


#endif // _DEV_MAPFILEENTRY_H
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
