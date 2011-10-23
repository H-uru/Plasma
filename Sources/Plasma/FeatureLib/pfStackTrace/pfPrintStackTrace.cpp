#include "pfPrintStackTrace.h"
#include "pfStackTrace.h"
#include "pfMapFile.h"
#include <stdio.h>
#include <string.h>

#pragma optimize( "y", off )

//-----------------------------------------------------------------------------

using namespace dev;

//-----------------------------------------------------------------------------

/**
 * Prints stack trace to user defined buffer.
 * Always terminates the buffer with 0.
 */
void printStackTrace( char* buffer, int bufferSize, unsigned long stackPtr, unsigned long opPtr )
{
    // find out map file name
    char modname[500];
    MapFile::getModuleMapFilename( modname, sizeof(modname) );

    // parse map file
    static char buf[5000];
    MapFile map( modname );
    switch ( map.error() )
    {
    case MapFile::ERROR_OPEN:   sprintf( buf, "Failed to open map file %s\n", modname ); break;
    case MapFile::ERROR_READ:   sprintf( buf, "Error while reading map file %s(%i)\n", modname, map.line() ); break;
    case MapFile::ERROR_PARSE:  sprintf( buf, "Parse error in map file %s(%i)\n", modname, map.line() ); break;
    default:                    break;
    }

    // print stack trace to buffer
    if ( !map.error() )
    {
        MapFile* maps[] = {&map};
        StackTrace::printStackTrace( maps, 1, 1, 16, buf, sizeof(buf), stackPtr, opPtr );
    }
    else
    {
        // 9.5.2002 mcn - Even if we can't open the map file, still print out the stack trace for later reference
        StackTrace::printStackTrace( 0, 0, 1, 16, buf, sizeof(buf), stackPtr, opPtr );
    }

    // copy to user buffer
    if ( bufferSize > 0 )
    {
        if( buffer[ 0 ] == 0 )
            strncpy( buffer, buf, bufferSize );
        else
            strncat( buffer, buf, bufferSize );
        buffer[bufferSize-1] = 0;
    }
}

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
