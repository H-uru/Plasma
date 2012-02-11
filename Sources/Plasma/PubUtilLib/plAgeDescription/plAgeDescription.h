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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef PL_AGE_DESCRIPTION_H
#define PL_AGE_DESCRIPTION_H

#include "HeadSpin.h"
#include "hsTemplates.h"

#include "plUnifiedTime/plUnifiedTime.h"
#include "pnKeyedObject/plUoid.h"
#include "plFile/plInitFileReader.h"

//
// Age Definition File Reader/Writer
//
class hsStream;

class plAgePage
{
    protected:
        char    *fName;
        uint32_t  fSeqSuffix;
        uint8_t    fFlags;

    public:

        static const uint32_t kInvalidSeqSuffix;

        enum Flags
        {
            kPreventAutoLoad    = 0x01,
            kLoadIfSDLPresent   = 0x02,
            kIsLocalOnly        = 0x04,
            kIsVolatile         = 0x08,
        };

        plAgePage( const char *name, uint32_t seqSuffix, uint8_t flags );
        plAgePage( char *stringFrom );
        plAgePage( const plAgePage &src );
        plAgePage();
        ~plAgePage();

        const char  *GetName( void ) const { return fName; }
        uint32_t      GetSeqSuffix( void ) const { return fSeqSuffix; }
        uint8_t        GetFlags( void ) const { return fFlags; }

        void        SetSeqSuffix( uint32_t s ) { fSeqSuffix = s; }
        void        SetFlags(uint8_t f, bool on=true);

        hsBool      SetFromString( const char *string );
        char        *GetAsString( void ) const;

        plAgePage &operator=( const plAgePage &src );
};

// Derived from plInitSectionTokenReader so we can do nifty things with reading the files

class plAgeDescription : public plInitSectionTokenReader
{
private:

    char    *fName;

    int32_t               fPageIterator;
    hsTArray<plAgePage> fPages;

    plUnifiedTime fStart;

    float fDayLength;
    short fMaxCapacity;
    short   fLingerTime;        // seconds game instance should linger after last player leaves. -1 means never exit.

    int32_t   fSeqPrefix;
    uint32_t  fReleaseVersion;    // 0 for pre-release, 1+ for actual released ages
    
    static const char* fCommonPages[];

    void    IInit( void );
    void    IDeInit( void );

    // Overload for plInitSectionTokenReader
    virtual hsBool      IParseToken( const char *token, hsStringTokenizer *tokenizer, uint32_t userData );

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

    bool ReadFromFile( const char *fileNameToReadFrom ) ;
    void Read(hsStream* stream);
    void Write(hsStream* stream) const;

    // Overload for plInitSectionTokenReader
    virtual const char  *GetSectionName( void ) const;

    const char  *GetAgeName( void ) const { return fName; }
    void        SetAgeNameFromPath( const char *path );
    void        SetAgeName(const char* ageName) { delete [] fName; fName=hsStrcpy(ageName); }

    // Page list
    void    ClearPageList();
    void    RemovePage( const char *page );
    void    AppendPage( const char *name, int seqSuffix = -1, uint8_t flags = 0 );

    void        SeekFirstPage( void );
    plAgePage   *GetNextPage( void );
    int         GetNumPages() const { return fPages.GetCount(); }
    plAgePage   *FindPage( const char *name ) const;
    bool FindLocation(const plLocation& loc) const;
    plLocation  CalcPageLocation( const char *page ) const;

    // Getters
    short GetStartMonth() const { return fStart.GetMonth(); }
    short GetStartDay() const { return fStart.GetDay(); }
    short GetStartYear() const { return fStart.GetYear(); }
    short GetStartHour() const { return fStart.GetHour(); }
    short GetStartMinute() const { return fStart.GetMinute(); }
    short GetStartSecond() const { return fStart.GetSecond(); }
    short GetMaxCapacity() const { return fMaxCapacity; }
    short GetLingerTime() const { return fLingerTime;}

    float   GetDayLength() const { return fDayLength; }

    int32_t   GetSequencePrefix( void ) const { return fSeqPrefix; }
    uint32_t  GetReleaseVersion( void ) const { return fReleaseVersion; }
    hsBool  IsGlobalAge( void ) const { return ( fSeqPrefix < 0 ) ? true : false; }

    // Setters
    hsBool SetStart(short year, short month, short day, short hour, short minute, short second)
        { return fStart.SetTime(year,month,day,hour,minute,second); }

    void SetDayLength(const float l) { fDayLength = l; }
    void SetMaxCapacity(const short m) { fMaxCapacity=m; }
    void SetLingerTime(const short v) { fLingerTime=v;}
    void SetSequencePrefix( int32_t p ) { fSeqPrefix = p; }
    void SetReleaseVersion( uint32_t v ) { fReleaseVersion = v; }

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

    void    AppendCommonPages( void );
    void    CopyFrom(const plAgeDescription& other);

    plAgeDescription &operator=( const plAgeDescription &src )
    {
        CopyFrom( src );
        return *this;
    }
};



#endif //PL_AGE_DESCRIPTION_H
