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
#include "HeadSpin.h"
#include "Max.h"
#include <commdlg.h>
#include <bmmlib.h>
#include <guplib.h>

#include "HeadSpin.h"
#include "plExportLogErrorMsg.h"
#include "hsExceptions.h"


//
// On our way out, be sure to close the error file
// ... and remind them of how many errors they created
//
plExportLogErrorMsg::~plExportLogErrorMsg()
{
    if ( fErrfile )
    {
        fprintf(fErrfile, "\n%d total number of error!!!! ", fNumberErrors);
        if ( fNumberErrors > 10 )
            if ( fNumberErrors > 20 )
                if ( fNumberErrors > 50 )
                    fprintf(fErrfile, "(CRISIS CRISIS!)");
                else
                    fprintf(fErrfile, "(which is a disaster!)");
            else
                fprintf(fErrfile, "(which is way too many!)");
        fclose(fErrfile);
    }
#ifdef ERRORLOG_ALWAYS_WRITE_SOMETHING
    else
    {
        fErrfile = hsFopen(fErrfile_name, "wt");
        setbuf(fErrfile, nil);
        fprintf(fErrfile, "No errors found! Good job.");
        fclose(fErrfile);
    }
#endif // ERRORLOG_ALWAYS_WRITE_SOMETHING
}


hsBool plExportLogErrorMsg::Show()
{
    if( GetBogus() )
    {
        IWriteErrorFile(GetLabel(),GetMsg());
    }
    return GetBogus();
}
hsBool plExportLogErrorMsg::Ask()
{
    if( GetBogus() )
    {
        IWriteErrorFile(GetLabel(),GetMsg());
    }
    return false;
}

hsBool plExportLogErrorMsg::CheckAndAsk()
{
    if( GetBogus() )
    {
        strncat(GetMsg(), " - File corruption possible!", 255);
        IWriteErrorFile(GetLabel(),GetMsg());
    }
    return GetBogus();
}

hsBool plExportLogErrorMsg::CheckAskOrCancel()
{
    if( GetBogus() )
    {
        IWriteErrorFile(GetLabel(),GetMsg());
    }
    return false;
}

hsBool plExportLogErrorMsg::CheckAndShow()
{
    if ( GetBogus() )
    {
        Show();
        Check();
    }

    return false;
}


hsBool plExportLogErrorMsg::Check()
{
    if( GetBogus() )
    {
        // ... how many ways can you say something is bad?
        strncat(GetMsg(), " !Output File Corrupt!", 255);
        IWriteErrorFile(GetLabel(),GetMsg());
        IDebugThrow();
    }

    return false;
}

//
// Not sure what to do here... it must be really bad if someone wants to Quit()
//
void plExportLogErrorMsg::Quit()
{
    if( GetBogus() )
    {
        strncat(GetMsg(), " -- Quit! (must be real bad!)", 255);
        IWriteErrorFile(GetLabel(),GetMsg());
        SetBogus(false);
        hsThrow( *this );
    }
}

//
// Write a string to the Error Log File, be sure its open before using
//
void plExportLogErrorMsg::IWriteErrorFile(const char* label, const char* msg)
{
    //make sure that there is a filename 
    if (fErrfile_name[0] != '\0')
    {
        // do we have it open, yet?
        if ( !fErrfile )
        {
            // must be the first write... open the error file
            fErrfile = hsFopen(fErrfile_name, "wt");
            setbuf(fErrfile, nil);
            fNumberErrors = 0;
        }
        fprintf(fErrfile, "%s: %s\n", label, msg);
        fNumberErrors++;    // oh, boy... another error to count
    }

   // Check to see if we are running an export server
   // If so, then pass the update on to the export server
   GUP* exportServerGup = OpenGupPlugIn(Class_ID(470000004,99));
   if(exportServerGup)
   {
      exportServerGup->Control(-5);  // means next control will be error msg
      char buf[1024];
      sprintf(buf, "%s: %s", label, msg);
      exportServerGup->Control((DWORD)buf);
      exportServerGup->Control(-7); // signal that we're done sending this update sequence
   }   
}

void plExportLogErrorMsg::IDebugThrow()
{
    try {
#if HS_BUILD_FOR_WIN32
        DebugBreak();
#endif // HS_BUILD_FOR_WIN32
    }
    catch(...)
    {
        hsThrow( *this );
    }
}
