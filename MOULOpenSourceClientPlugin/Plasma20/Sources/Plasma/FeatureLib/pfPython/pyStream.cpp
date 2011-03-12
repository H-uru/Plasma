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
//////////////////////////////////////////////////////////////////////
//
// pyStream   - a wrapper class to provide interface to the File stream stuff
//
//////////////////////////////////////////////////////////////////////

#include "pyStream.h"

#include "../plFile/plEncryptedStream.h"

pyStream::pyStream()
: fStream( nil )
{
}

pyStream::~pyStream()
{
	Close();
}


hsBool pyStream::Open(const wchar* fileName, const wchar* flags)
{
	// make sure its closed first
	Close();

	if (fileName)
	{
		if (flags)
		{
			bool readflag = false;
			bool writeflag = false;
			bool encryptflag = false;
			int i;
			for (i=0 ; i < wcslen(flags) ; i++ )
			{
				if ( flags[i] == L'r' || flags[i] == L'R' )
					readflag = true;
				if ( flags[i] == L'w' || flags[i] == L'W' )
					writeflag = true;
				if ( flags[i] == L'e' || flags[i] == L'E' )
					encryptflag = true;
			}
			// if there is a write flag, it takes priorty over read
			if (writeflag)
			{
				// force encryption?
				if (encryptflag)
				{
					fStream = TRACKED_NEW plEncryptedStream;
					fStream->Open(fileName, L"wb");
				}
				else
					fStream = plEncryptedStream::OpenEncryptedFileWrite(fileName);
			}
			else
				fStream = plEncryptedStream::OpenEncryptedFile(fileName);
			return true;
		}
	}
	return false;
}

std::vector<std::string> pyStream::ReadLines()
{
	
	// read all the lines in the file and put in a python list object
	// create the list
	std::vector<std::string> pyPL;

	if (fStream)
	{
		char buf[4096];
		
		while (!fStream->AtEnd())
		{
			if (fStream->ReadLn(buf, sizeof(buf), 0, 0))
				pyPL.push_back(buf);
		}
	}

	return pyPL;
}

hsBool pyStream::WriteLines(const std::vector<std::string> & lines)
{
	if (fStream)
	{
		int i;
		for ( i=0 ; i<lines.size() ; i++ )
		{
			std::string element = lines[i];
			fStream->Write(element.length(),element.c_str());
		}
		return true;
	}

	return false;
}


void pyStream::Close()
{
	if (fStream)
	{
		fStream->Close();
		delete fStream;
	}
	fStream = nil;
}

hsBool pyStream::IsOpen()
{
	if (fStream)
		return true;
	return false;
}
