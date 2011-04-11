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
#ifndef hsFiles_Defined
#define hsFiles_Defined

#include "hsStream.h"
#include <stdio.h>

#if HS_BUILD_FOR_UNIX
	#include <limits.h>
	#define kFolderIterator_MaxPath		PATH_MAX
	#include <unistd.h>
	#define SetCurrentDirectory chdir
#elif !HS_BUILD_FOR_PS2
	#define kFolderIterator_MaxPath		_MAX_PATH
#else
	#define kFolderIterator_MaxPath		255
#endif

#if HS_BUILD_FOR_MAC
	#include <Files.h>
	#include <Script.h>
#endif


#if HS_BUILD_FOR_WIN32
# define PATH_SEPARATOR		'\\'
# define WPATH_SEPARATOR	L'\\'
# define PATH_SEPARATOR_STR	"\\"
# define WPATH_SEPARATOR_STR L"\\"
#elif HS_BUILD_FOR_UNIX
# define PATH_SEPARATOR		'/'
# define WPATH_SEPARATOR	L'/'
# define PATH_SEPARATOR_STR	"/"
# define WPATH_SEPARATOR_STR L"/"
#endif


///////////////////////////////////////////////////////////////////////
#if !HS_BUILD_FOR_PS2

class hsFile {
	hsFile&		operator=(const hsFile&);		// disallow assignment
protected:
	char*		fPathAndName;
	FILE*		fFILE;
public:
				hsFile();
				hsFile(const char pathAndName[]);
	virtual		~hsFile();

	const char*	GetName();
	virtual const char*	GetPathAndName();
	virtual void	SetPathAndName(const char pathAndName[]);

	virtual FILE*	OpenFILE(const char mode[], hsBool throwIfFailure = false);
	virtual hsStream* OpenStream(const char mode[], hsBool throwIfFailure = false);

	virtual void	Close();	// called automatically in the destructor
};
typedef hsFile	hsUnixFile;	// for compatibility

#if HS_BUILD_FOR_MAC
	class hsMacFile : public hsFile {
		enum {
			kRefNum_Dirty,
			kPathName_Dirty
		};
		FSSpec		fSpec;
		Int16		fRefNum;
		UInt16		fFlags;

		void 			SetSpecFromName();
		void 			SetNameFromSpec();
	public:
					hsMacFile();
					hsMacFile(const FSSpec* spec);
					hsMacFile(const char pathAndName[]);
		virtual		~hsMacFile();

		const FSSpec*	GetSpec() const { return &fSpec; }
		void			SetSpec(const FSSpec* spec);
		hsBool		Create(OSType creator, OSType fileType, ScriptCode scriptCode = smSystemScript);
		hsBool		OpenDataFork(SInt8 permission, Int16* refnum);

		//	Overrides
		virtual const char*	GetPathAndName();
		virtual void	SetPathAndName(const char pathAndName[]);
		virtual hsStream* OpenStream(const char mode[], hsBool throwIfFailure = false);
		virtual void	Close();
	};
	typedef hsMacFile	hsOSFile;
#else
	typedef hsFile		hsOSFile;
#endif
#endif // HS_BUILD_FOR_PS2
///////////////////////////////////////////////////////////////////////

class hsFolderIterator {
	char		fPath[kFolderIterator_MaxPath];
	struct hsFolderIterator_Data* fData;
   bool fCustomFilter;
public:
#ifdef HS_BUILD_FOR_WIN32
   hsFolderIterator(const char path[] = nil, bool useCustomFilter=false);
#else
   hsFolderIterator(const char path[] = nil, bool unused=true);
   hsFolderIterator(const struct FSSpec* spec);	// Alt constructor
#endif
	virtual		~hsFolderIterator();

	const char*	GetPath() const { return fPath; }
	void			SetPath(const char path[]);

	void			Reset();
	hsBool		NextFile();
	hsBool		NextFileSuffix(const char suffix[]);
	const char*	GetFileName() const;
	int			GetPathAndName(char pathandname[] = nil);
	hsBool		IsDirectory( void ) const;

	FILE*		OpenFILE(const char mode[]);

#if HS_BUILD_FOR_MAC
	void			SetMacFolder(const char path[]);
	void			SetMacFolder(OSType folderType);
	void			SetMacFolder(Int16 vRefNum, Int32 dirID);
	hsBool		NextMacFile(OSType targetFileType, OSType targetCreator);
	const struct FSSpec* GetMacSpec() const;
	OSType		GetMacFileType() const;
	OSType		GetMacCreator() const;
#elif HS_BUILD_FOR_WIN32
	void		SetWinSystemDir(const char subdir[]);	// e.g. "Fonts"
	void		SetFileFilterStr(const char filterStr[]);	// e.g. "*.*"
#endif
};

#ifdef HS_BUILD_FOR_WIN32
// only implemented on Win32 for now
class hsWFolderIterator {
	wchar		fPath[kFolderIterator_MaxPath];
	struct		hsWFolderIterator_Data* fData;
	bool		fCustomFilter;
public:
	hsWFolderIterator(const wchar path[] = nil, bool useCustomFilter=false);
	virtual		~hsWFolderIterator();

	const wchar*	GetPath() const { return fPath; }
	void			SetPath(const wchar path[]);

	void			Reset();
	hsBool			NextFile();
	hsBool			NextFileSuffix(const wchar suffix[]);
	const wchar*	GetFileName() const;
	int				GetPathAndName(wchar pathandname[] = nil);
	hsBool			IsDirectory( void ) const;

	FILE*			OpenFILE(const wchar mode[]);

	void		SetWinSystemDir(const wchar subdir[]);	// e.g. "Fonts"
	void		SetFileFilterStr(const wchar filterStr[]);	// e.g. "*.*"
};
#endif

#endif
