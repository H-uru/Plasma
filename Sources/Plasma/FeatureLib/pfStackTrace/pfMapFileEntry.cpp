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
#include "pfMapFileEntry.h"
#include <string.h>

//-----------------------------------------------------------------------------

namespace dev
{


MapFileEntry::MapFileEntry()
{
	m_sec = 0;
	m_addr = 0; 
	m_len = 0;
	m_name[0] = 0;
}

MapFileEntry::MapFileEntry( long section, long offset, long length, const char* name )
{
	m_sec = section;
	m_addr = offset;
	m_len = length;

	strncpy( m_name, name, MAX_NAME ); 
	m_name[MAX_NAME] = 0;
}

long MapFileEntry::section() const
{
	return m_sec;
}

long MapFileEntry::offset() const
{
	return m_addr;
}

long MapFileEntry::length() const
{
	return m_len;
}

const char* MapFileEntry::name() const
{
	return m_name;
}

bool MapFileEntry::operator<( const MapFileEntry& other ) const
{
	if ( m_sec < other.m_sec )
		return true;
	if ( m_sec > other.m_sec )
		return false;
	return m_addr < other.m_addr;
}


} // dev
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
