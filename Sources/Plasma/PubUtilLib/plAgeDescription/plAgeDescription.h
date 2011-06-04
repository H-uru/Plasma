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
#ifndef PL_AGE_DESCRIPTION_H
#define PL_AGE_DESCRIPTION_H

#include "hsTypes.h"
#include "hsTemplates.h"
#include "hsUtils.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../pnKeyedObject/plUoid.h"
#include "../plFile/plInitFileReader.h"

//
// Age Definition File Reader/Writer
//
class hsStream;

class plAgePage
{
	protected:
		char	*fName;
		UInt32	fSeqSuffix;
		Byte	fFlags;

	public:

		static const UInt32	kInvalidSeqSuffix;

		enum Flags
		{
			kPreventAutoLoad	= 0x01,
			kLoadIfSDLPresent	= 0x02,
			kIsLocalOnly		= 0x04,
			kIsVolatile			= 0x08,
		};

		plAgePage( const char *name, UInt32 seqSuffix, Byte flags );
		plAgePage( char *stringFrom );
		plAgePage( const plAgePage &src );
		plAgePage();
		~plAgePage();

		const char	*GetName( void ) const { return fName; }
		UInt32		GetSeqSuffix( void ) const { return fSeqSuffix; }
		Byte		GetFlags( void ) const { return fFlags; }

		void		SetSeqSuffix( UInt32 s ) { fSeqSuffix = s; }
		void		SetFlags(Byte f, bool on=true);

		hsBool		SetFromString( const char *string );
		char		*GetAsString( void ) const;

		plAgePage &operator=( const plAgePage &src );
};

// Derived from plInitSectionTokenReader so we can do nifty things with reading the files

class plAgeDescription : public plInitSectionTokenReader
{
private:

	char	*fName;

	Int32				fPageIterator;
	hsTArray<plAgePage>	fPages;

	plUnifiedTime fStart;

	float fDayLength;
	short fMaxCapacity;
	short	fLingerTime;		// seconds game instance should linger after last player leaves. -1 means never exit.

	Int32	fSeqPrefix;
	UInt32	fReleaseVersion;	// 0 for pre-release, 1+ for actual released ages
	
	static char	*fCommonPages[];

	void	IInit( void );
	void	IDeInit( void );

	// Overload for plInitSectionTokenReader
	virtual hsBool		IParseToken( const char *token, hsStringTokenizer *tokenizer, UInt32 userData );

public:
	static char kAgeDescPath[];

	plAgeDescription();
	plAgeDescription( const char *fileNameToReadFrom );
	plAgeDescription( const plAgeDescription &src )
	{
		IInit();
		CopyFrom( src );
	}
	~plAgeDescription();

	bool ReadFromFile( const char *fileNameToReadFrom )	;
	void Read(hsStream* stream);
	void Write(hsStream* stream) const;
	
	// Overload for plInitSectionTokenReader
	virtual const char	*GetSectionName( void ) const;

	const char	*GetAgeName( void ) const { return fName; }
	void		SetAgeNameFromPath( const char *path );
	void		SetAgeName(const char* ageName) { delete [] fName; fName=hsStrcpy(ageName);	}

	// Page list
	void	ClearPageList();
	void	RemovePage( const char *page );
	void	AppendPage( const char *name, int seqSuffix = -1, Byte flags = 0 );

	void		SeekFirstPage( void );
	plAgePage	*GetNextPage( void );
	int			GetNumPages() const { return fPages.GetCount(); }
	plAgePage	*FindPage( const char *name ) const;
	bool FindLocation(const plLocation& loc) const;
	plLocation	CalcPageLocation( const char *page ) const;

	// Getters
	short GetStartMonth() const { return fStart.GetMonth(); }
	short GetStartDay() const { return fStart.GetDay(); }
	short GetStartYear() const { return fStart.GetYear(); }
	short GetStartHour() const { return fStart.GetHour(); }
	short GetStartMinute() const { return fStart.GetMinute(); }
	short GetStartSecond() const { return fStart.GetSecond(); }
	short GetMaxCapacity() const { return fMaxCapacity; }
	short GetLingerTime() const { return fLingerTime;}

	float	GetDayLength() const { return fDayLength; }

	Int32	GetSequencePrefix( void ) const { return fSeqPrefix; }
	UInt32	GetReleaseVersion( void ) const { return fReleaseVersion; }
	hsBool	IsGlobalAge( void ) const { return ( fSeqPrefix < 0 ) ? true : false; }

	// Setters
	hsBool SetStart(short year, short month, short day, short hour, short minute, short second)
		{ return fStart.SetTime(year,month,day,hour,minute,second); }

	void SetDayLength(const float l) { fDayLength = l; }
	void SetMaxCapacity(const short m) { fMaxCapacity=m; }
	void SetLingerTime(const short v) { fLingerTime=v;}
	void SetSequencePrefix( Int32 p ) { fSeqPrefix = p; }
	void SetReleaseVersion( UInt32 v ) { fReleaseVersion = v; }

	// calculations
	double GetAgeElapsedDays(plUnifiedTime earthCurrentTime) const;
	double GetAgeElapsedSeconds(const plUnifiedTime & earthCurrentTime) const;
	int GetAgeTimeOfDaySecs(const plUnifiedTime& earthCurrentTime) const;
	float GetAgeTimeOfDayPercent(const plUnifiedTime& earthCurrentTime) const;
	
	// Static functions for the available common pages
	enum CommonPages
	{
		kTextures = 0,
		kGlobal,
		kNumCommonPages
	};

	static const char *GetCommonPage( int pageType );

	void	AppendCommonPages( void );
	void	CopyFrom(const plAgeDescription& other);

	plAgeDescription &operator=( const plAgeDescription &src )
	{
		CopyFrom( src );
		return *this;
	}
};



#endif //PL_AGE_DESCRIPTION_H
