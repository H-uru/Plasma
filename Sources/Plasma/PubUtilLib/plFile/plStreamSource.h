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
#ifndef plStreamSource_h_inc
#define plStreamSource_h_inc

#include "hsStream.h"
#include "hsStlUtils.h"

// A class for holding and accessing file streams. The preloader will insert
// files in here once they are loaded. In internal builds, if a requested file
// is not found, it will be retrieved from disk.
class plStreamSource
{
private:
	struct fileData
	{
		std::wstring	fFilename; // includes path
		std::wstring	fDir; // parent directory
		std::wstring	fExt;
		hsStream*		fStream; // we own this pointer, so clean it up
	};
	std::map<std::wstring, fileData> fFileData; // key is filename

	void ICleanup(); // closes all file pointers and cleans up after itself
	void IBreakupFilename(std::wstring filename, std::wstring& dir, std::wstring& ext);

	plStreamSource() {}
public:
	~plStreamSource() {ICleanup();}

	// Force a cleanup of all data (some apps need to get at those file again, and they can't while we have them open)
	void Cleanup() {ICleanup();}

	// File access functions
	hsStream* GetFile(std::wstring filename); // internal builds will read from disk if it doesn't exist
	std::vector<std::wstring> GetListOfNames(std::wstring dir, std::wstring ext); // internal builds merge from disk

	// For other classes to insert files (takes ownership of the stream if successful)
	bool InsertFile(std::wstring filename, hsStream* stream);

	// Instance handling
	static plStreamSource* GetInstance();
};

#endif // plStreamSource_h_inc