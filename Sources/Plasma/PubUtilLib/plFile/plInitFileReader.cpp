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
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plInitFileReader.h"

#include "hsStream.h"

#include "hsStringTokenizer.h"
#include "plEncryptedStream.h"


plInitSectionTokenReader::plInitSectionTokenReader( const char *separators ) : fSeparators( separators )
{
}

bool        plInitSectionTokenReader::ParseLine( const char *line, uint32_t userData )
{
    hsStringTokenizer izer( line, fSeparators );

    char *token = izer.next();
    return IParseToken( token, &izer, userData );
}

void    plInitFileReader::IInitReaders( plInitSectionReader **readerArray )
{
    for (size_t i = 0; readerArray[i] != nullptr; i++)
        fSections.emplace_back(readerArray[i]);

    hsAssert(!fSections.empty(), "No sections for initFileReader");

    fCurrSection = fSections[ 0 ];
}

plInitFileReader::plInitFileReader( plInitSectionReader **readerArray, uint16_t lineSize )
    : fOurStream()
{
    fCurrLine = nullptr;
    fLineSize = lineSize;
    fStream = nullptr;
    IInitReaders( readerArray );
    fUnhandledSection = nullptr;
}

plInitFileReader::plInitFileReader( const char *fileName, plInitSectionReader **readerArray, uint16_t lineSize )
    : fOurStream()
{
    fCurrLine = nullptr;
    fLineSize = lineSize;
    fStream = nullptr;
    IInitReaders( readerArray );
    if( !Open( fileName ) )
        hsAssert( false, "Constructor open for plInitFileReader failed!" );
    fUnhandledSection = nullptr;
}

plInitFileReader::plInitFileReader( hsStream *stream, plInitSectionReader **readerArray, uint16_t lineSize )
    : fOurStream()
{
    fCurrLine = nullptr;
    fLineSize = lineSize;
    fStream = nullptr;
    IInitReaders( readerArray );
    if( !Open( stream ) )
        hsAssert( false, "Constructor open for plInitFileReader failed!" );
    fUnhandledSection = nullptr;
}

plInitFileReader::~plInitFileReader()
{
    delete [] fCurrLine;
}

bool    plInitFileReader::Open( const char *fileName )
{
    if (fStream != nullptr)
    {
        hsAssert( false, "Unable to open initFileReader; already open" );
        return false;
    }

    fOurStream = plEncryptedStream::OpenEncryptedFile( fileName );

    if (fOurStream == nullptr)
        return false;

    fStream = fOurStream.get();

    return true;
}

bool    plInitFileReader::Open( hsStream *stream )
{
    if (fStream != nullptr)
    {
        hsAssert( false, "Unable to open initFileReader; already open" );
        return false;
    }

    fStream = stream;
    return true;
}

bool    plInitFileReader::Parse( uint32_t userData )
{
    hsAssert(fStream != nullptr, "Nil stream in initFileReader::Parse(); file not yet open?");

    if (fCurrLine == nullptr)
        fCurrLine = new char[ fLineSize + 1 ];

    // Start parsing lines
    while( fStream->ReadLn( fCurrLine, fLineSize ) )
    {
        // puts( fCurrLine );

        // Is line a section header?
        if( fCurrLine[ 0 ] == '[' )
        {
            // Yes--match against our sections and switch to the given one
            char *end = strchr( fCurrLine, ']' );
            if (end != nullptr)
                *end = 0;

            bool foundSection = false;
            for (plInitSectionReader* section : fSections)
            {
                if (stricmp(section->GetSectionName(), &fCurrLine[1]) == 0)
                {
                    fCurrSection = section;
                    foundSection = true;
                    break;
                }
            }

            if (!foundSection && fUnhandledSection)
            {
                fCurrSection = fUnhandledSection;
                fCurrSection->SetSectionName(&fCurrLine[1]);
            }
        }
        else
        {
            // Nope, just a line, pass to our current section tokenizer
            if( !fCurrSection->ParseLine( fCurrLine, userData ) )
                return false;
        }
    }

    return true;
}
