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
#ifndef _DEV_MAPFILE_H
#define _DEV_MAPFILE_H


namespace dev
{


class MapFileEntry;


/** 
 * Linker generated module map file parser.
 */
class MapFile
{
public:
	/** Error code. */
	enum ErrorType
	{
		/** No error. */
		ERROR_NONE,
		/** File open failed. */
		ERROR_OPEN,
		/** File reading failed. */
		ERROR_READ,
		/** Syntax error. */
		ERROR_PARSE
	};

	/** Reads a map file. */
	explicit MapFile( const char* filename );

	///
	~MapFile();

	/** Returns preferred load address. */
	long				loadAddress() const;

	/** Returns ith entry from the map file. */
	const MapFileEntry&	getEntry( int i ) const;

	/** Returns ith segment from the map file. */
	const MapFileEntry&	getSegment( int i ) const;

	/** Returns number of segments in the map file. */
	int					segments() const;

	/** Returns number of entries in the map file. */
	int					entries() const;

	/** Returns error code or 0 (ERROR_NONE) if no error. */
	ErrorType			error() const;

	/** Returns line number of last successful read character. */
	int					line() const;

	/** 
	 * Finds entry which contains specified address. 
	 * @return Entry index or -1 if not found.
	 */
	int					findEntry( long addr ) const;

	/** 
	 * Returns current module name, with map extension.
	 * The output buffer is always 0-terminated.
	 */
	static void			getModuleMapFilename( char* buffer, int bufferSize );

private:
	class MapFileImpl;
	MapFileImpl* m_this;

	MapFile( const MapFile& );
	MapFile& operator=( const MapFile& );
};


} // dev


#endif // _DEV_MAPFILE_H
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
