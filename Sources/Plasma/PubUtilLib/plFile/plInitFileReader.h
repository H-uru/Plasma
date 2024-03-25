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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plInitFileReader - Helper class that parses a standard-format .ini file //
//                     and allows you to specify derived classes to handle  //
//                     interpreting specific portions.                      //
//                                                                          //
//// Usage ///////////////////////////////////////////////////////////////////
//                                                                          //
//  First create a set of derived classes from plInitSectionReader          //
//  (or plInitSectionTokenReader, to be easier) that will parse the lines   //
//  for each section of your .ini file. Then create a C-style array of      //
//  pointers to instances of each reader, the first being the default       //
//  reader, and ending with a nil pointer (see below). Finally, create      //
//  a plInitFileReader with the array you created and it'll parse the       //
//  given file (or stream) and call your readers as needed. You can also    //
//  optionally pass in a uint32_t for userData that will be passed on to each //
//  reader in turn.                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plInitFileReader_h
#define _plInitFileReader_h

#include "HeadSpin.h"
#include "hsStream.h"

#include <memory>
#include <vector>

//// Base Section Class //////////////////////////////////////////////////////
//  Define a derived version of this for each section of your init file.

class plInitSectionReader
{
    public:

        // Override this to define what [string] your section starts with
        virtual const char  *GetSectionName() const = 0;

        // Override this to parse each line in your section. Return false to abort parsing
        virtual bool        ParseLine( const char *line, uint32_t userData ) = 0;
        
        // Override this if you're defining an unhandled section reader
        virtual void SetSectionName(const char* section) {}
};

//// Semi-Derived Class //////////////////////////////////////////////////////
//  Half-way derived class for parsing lines by tokens rather than pure 
//  strings.

template<typename CharT>
class hsBasicStringTokenizer;
using hsStringTokenizer = hsBasicStringTokenizer<char>;

class plInitSectionTokenReader : public plInitSectionReader
{
    protected:

        const char  *fSeparators;

        // Override this to parse each token in your section. Return false to abort parsing
        virtual bool        IParseToken( const char *token, hsStringTokenizer *tokenizer, uint32_t userData ) = 0;

    public:

        plInitSectionTokenReader( const char *separators = ",=\t" );

        // Overridden for you. Override IParseToken()
        bool        ParseLine(const char *line, uint32_t userData) override;
};

//// Main Reader Class ///////////////////////////////////////////////////////
//  Create one of these and add an array of derived versions of the above
//  reader to parse your init file.

class plInitFileReader
{
    protected:

        hsStream            *fStream;
        std::unique_ptr<hsStream> fOurStream;
        char                *fCurrLine;
        uint32_t              fLineSize;

        plInitSectionReader             *fCurrSection;
        std::vector<plInitSectionReader *> fSections;
        plInitSectionReader* fUnhandledSection;
        
        void    IInitReaders( plInitSectionReader **readerArray );

    public:

        // The array passed in should be an array of pointers to plInitSectionReader,
        // with the last pointer being nil (denoting the end of the array). The first
        // element of the array will be the "default" section--i.e. if there is no section
        // header at the top of the file, that reader will be used.

        plInitFileReader( plInitSectionReader **readerArray, uint16_t lineSize = 256 );
        plInitFileReader( const char *fileName, plInitSectionReader **readerArray, uint16_t lineSize = 256 );
        plInitFileReader( hsStream *stream, plInitSectionReader **readerArray, uint16_t lineSize = 256 );
        virtual ~plInitFileReader();

        void SetUnhandledSectionReader(plInitSectionReader* reader) { fUnhandledSection = reader; }

        bool    Open( const char *fileName );
        bool    Open( hsStream *stream );
        bool    Parse( uint32_t userData = 0 );

        bool    IsOpen() const { return fStream != nullptr; }
};

#endif //_plInitFileReader_h
