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
#define plIndexFile_cpp		// for version numbers in plVersion.h

#include "plIndexFile.h"
#include "hsStream.h"
#include "plVersion.h"

//--------------------
// plIndexFileHeader
//--------------------

plIndexFileHeader::plIndexFileHeader()
{
	fTimeStamp[ 0 ] = fTimeStamp[ 1 ] = 0;
	fStrTblStartPos = 0;
	fStrTblKnt		= 0;
	fMajorVersion	= plVersion::GetMajorVersion();
	fMinorVersion	= plVersion::GetMinorVersion();
	fExportLocal = 0;
}

void plIndexFileHeader::Write(hsStream *s)	
{	
	s->WriteSwap32(2,fTimeStamp);
	s->WriteSwap32(fStrTblStartPos); 
	s->WriteSwap16(fStrTblKnt);
	s->WriteSwap16(fMajorVersion);
	s->WriteSwap16(fMinorVersion);
	s->WriteSwap16(fExportLocal);
} 
void plIndexFileHeader::Read(hsStream *s)	
{	
	s->ReadSwap32(2,fTimeStamp);
	fStrTblStartPos = s->ReadSwap32(); 
	fStrTblKnt		= s->ReadSwap16();
	fMajorVersion	= s->ReadSwap16();
	fMinorVersion	= s->ReadSwap16();
	fExportLocal = s->ReadSwap16();
} 

//--------------------
// plIndexFileRoom
//--------------------

void plIndexFileRoom::Write(hsStream *s)	
{	
	fRmUoid.Write(s);
	s->WriteSwap32(fTypesInRoom); 
	s->WriteSwap16(fFiller1_16);
	s->WriteSwap32(fFiller2_32);
	s->WriteSwap32(fFiller3_32);
} 
void plIndexFileRoom::Read(hsStream *s)	
{	
	fRmUoid.Read(s);
	fTypesInRoom = s->ReadSwap32(); 
	s->ReadSwap16();
	s->ReadSwap32();
	s->ReadSwap32();
} 
//--------------------
// plIndexFileType
//--------------------


void plIndexFileType::Write(hsStream *s)
{	
	fTypeUoid.Write(s);
	s->WriteSwap32(fNumKeys); 
	s->WriteSwap32(fFiller1_32);
	s->WriteSwap32(fFiller2_32);
}
 
void plIndexFileType::Read(hsStream *s)	
{	
	fTypeUoid.Read(s);
	fNumKeys = s->ReadSwap32(); 
	s->ReadSwap32();
	s->ReadSwap32();
} 
//--------------------
// plIndexFileKey
//--------------------


void plIndexFileKey::Write(hsStream *s)	
{	
	fKeyUoid.Write(s);
	s->WriteSwap32(fStartPos); 
	s->WriteSwap32(fDataLen);
	s->WriteSwap16(fNameIx);
	s->WriteSwap16(fFiller1_16);
}


void plIndexFileKey::Read(hsStream *s)
{	
	fKeyUoid.Read(s);
	fStartPos = s->ReadSwap32(); 
	fDataLen = s->ReadSwap32(); 
	fNameIx = s->ReadSwap16();
	s->ReadSwap16();
}


 

//--------------------
// plIxStrTbl
//--------------------

plIxStrTbl::~plIxStrTbl() 
{ 
	if (fpStrings) delete []fpStrings;  // if strings came from elsewhere, not our responsibility
} 

UInt16 plIxStrTbl::AddString(const char *p)
{	Int16 ix = FindString(p); 
	if (ix != -1) 
		return ix; // duplicate
	fStringTbl.push_back(p); return fStringTbl.size() - 1; 
}

Int16 plIxStrTbl::FindString(const char *p)
{
	for (int i=0; i < fStringTbl.size(); i++)
	{	
		if (!_stricmp(p,fStringTbl[i]))
			return i;
	}
	return -1;
}

void plIxStrTbl::Write(hsStream *s)	
{	
	for (int i=0; i < fStringTbl.size(); i++)
	{	Int32 len= fStringTbl[i] ? strlen(fStringTbl[i]) : 0;
		hsAssert(len < 256,"Name string too long");
		UInt8 l = (UInt8) len;
		s->WriteByte(l);						// FUTURE, don't really need length!
		if (len)
		{
			s->Write(len, fStringTbl[i]);
		}
		s->WriteByte(0);	// Null terminate
	}
}

void plIxStrTbl::Read(hsStream *s)
{	UInt32 pos = s->GetPosition();
	s->FastFwd();
	fTabSize = s->GetPosition() - pos;	// Get size of table
	s->SetPosition(pos);
	fpStrings = new char[fTabSize];
	hsAssert(fpStrings,"new failed");
	s->Read(fTabSize,fpStrings);			// Read all the string in

	char *p = fpStrings;
	while (p < fpStrings + fTabSize)
	{
		UInt8 len = *p;
		p++;
		hsAssert(p < fpStrings + fTabSize,"String Index error");
		fStringTbl.push_back(p);
		p += len + 1; // past len and NULL
	};
}
		



