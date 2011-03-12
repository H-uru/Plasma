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
#ifndef PLINDEXFILE_H
#define PLINDEXFILE_H

#include "../pnKeyedObject/plUoid.h"
#include <vector>	
	//---------------------------------------------------------------
	// These Classes are used to Read and Write the Index file for the Database
	// Records are kept the same size Currently 20 Bytes
	// At the End of the file, the Strings are stored
	// plIxStrTbl is used to Read and Write the String section of the Index
	//---------------------------------------------------------------

	//---------------------------------------------------------------
	// Main header Entry, One per Index file
	//---------------------------------------------------------------
class plIndexFileHeader
{
public:
	plIndexFileHeader(); // I buried Paul
	~plIndexFileHeader(){}

	UInt32 fTimeStamp[2];
	UInt32 fStrTblStartPos;		// where the String table starts in the Index file
	UInt16 fStrTblKnt;			// how many strings in the string table
	UInt16 fMajorVersion;
	UInt16 fMinorVersion;
	UInt16 fExportLocal;       // was this file exported locally or downloaded from server?

	void	Write(hsStream *s);
	void	Read(hsStream *s);
};

	//---------------------------------------------------------------
	// Room Entry One Entry per Room
	//---------------------------------------------------------------
class plIndexFileRoom
{
public:
	plIndexFileRoom() {	memset(this,1,sizeof(this)); } // no virtuals...relax
	~plIndexFileRoom(){}

	plUoid	fRmUoid;
	UInt16	fTypesInRoom;
	UInt16	fFiller1_16;
	UInt32	fFiller2_32;
	UInt32	fFiller3_32;

	void	Write(hsStream *s); 
	void	Read(hsStream *s); 
};

	//---------------------------------------------------------------
	// Type Entry One Entry per Type in each Room
	//---------------------------------------------------------------

class plIndexFileType
{
public:
	plIndexFileType() {	memset(this,1,sizeof(this)); } // no virtuals...relax
	~plIndexFileType(){}

	plUoid	fTypeUoid;
	UInt16	fNumKeys;
	UInt32	fFiller1_32;
	UInt32	fFiller2_32;

	void	Write(hsStream *s); 
	void	Read(hsStream *s); 
};

	//---------------------------------------------------------------
	// Key Entry One Entry per Type in each Room
	//---------------------------------------------------------------

class plIndexFileKey
{
public:
	plIndexFileKey() {	memset(this,1,sizeof(this)); } // no virtuals...relax
	~plIndexFileKey(){}
	
	
	plUoid	fKeyUoid;
	UInt32	fStartPos;
	UInt32	fDataLen;
	UInt16	fNameIx;	// Index into string table of name
	UInt16	fFiller1_16;

	void	Write(hsStream *s)	; 
	void	Read(hsStream *s); 
};

	//---------------------------------------------------------------
	// String Table, Lives at the end of the Index File
	//---------------------------------------------------------------
class plIxStrTbl
{
	std::vector<const char *>fStringTbl;
	char *fpStrings;
	UInt32 fTabSize;		// buffer size for strings
	
public:

	plIxStrTbl()	:fpStrings(nil), fTabSize(0){}
	~plIxStrTbl();

	Int16	FindString(const char *p);	// returns -1 if not found, otherwise the index from zero
	UInt16	AddString(const char *p);
	UInt16	NumStrings()				{	return fStringTbl.size();	}
	const char * GetString(UInt16 x)	{	return fStringTbl[x];		}

	void	Write(hsStream *s);
	void	Read(hsStream *s);		
};

class plLinkRecord
{
public:
	plLinkRecord(UInt16 a, UInt16 d, UInt16 r) : fAgeIx(a),	fDistIx(d), fRoomIx(r){}
	~plLinkRecord(){}
		
	UInt16	fAgeIx;		// Index into string table
	UInt16	fDistIx;
	UInt16	fRoomIx;
	UInt32	fTimeStamp[2];
};

#endif
