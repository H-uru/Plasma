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
#include "plErrorMsg.h"
#include "HeadSpin.h"
#include "hsExceptions.h"


#include "plErrorMsg.h"

static plErrorMsg nullMsg;


//////////////////////////////////////////////////////////////////////////////////////////////////
// Useful null object, to assign when I don't want to worry about checking for nil everywhere...
//////////////////////////////////////////////////////////////////////////////////////////////////
plErrorMsg *plErrorMsg::GetNull()
{
    return &nullMsg;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Non-destructive error handling
//////////////////////////////////////////////////////////////////////////////////////////////////
plErrorMsg::plErrorMsg(const char* label, const char* msg)
{
    Set(label, msg);
}

plErrorMsg::plErrorMsg(hsBool bogus) 
{ 
    Set(bogus);
}

plErrorMsg::plErrorMsg(hsBool bogus, const char* label, const char* msg) 
{ 
    Set(bogus, label, msg);
}

plErrorMsg::plErrorMsg(hsBool bogus, const char* label, const char* format, const char* str) 
{ 
    Set(bogus, label, format, str);
}

plErrorMsg::plErrorMsg(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
{ 
    Set(bogus, label, format, str1, str2);
}

plErrorMsg::plErrorMsg(hsBool bogus, const char* label, const char* format, int n) 
{ 
    Set(bogus, label, format, n);
}

plErrorMsg::plErrorMsg(hsBool bogus, const char* label, const char* format, int n, int m) 
{ 
    Set(bogus, label, format, n, m);
}

plErrorMsg::plErrorMsg(hsBool bogus, const char* label, const char* format, float f) 
{ 
    Set(bogus, label, format, f);
}

plErrorMsg &plErrorMsg::Set(const char* label, const char* msg)
{
    fBogus = true;
    if( label )
        hsStrncpy(fLabel, label, 256);
    else
        *fLabel = 0;
    if( msg )
    {
        hsAssert(strlen(msg) < PL_ERR_MSG_MAX_MSG, "Too long of an error message for plErrorMsg.");
        hsStrncpy(fMsg, msg, PL_ERR_MSG_MAX_MSG);
    }
    else
        *fMsg = 0;
    hsThrow( *this );

    return *this;
}

plErrorMsg &plErrorMsg::Set(hsBool bogus) 
{ 
    fBogus = bogus;
    fLabel[0] = 0; 
    fMsg[0] = 0; 

    return *this;
}

plErrorMsg &plErrorMsg::Set(hsBool bogus, const char* label, const char* msg) 
{ 
    if( fBogus = bogus )
    {
        if( label )
            hsStrncpy(fLabel, label, 256);
        else
            *fLabel = 0;
        if( msg )
        {
            hsAssert(strlen(msg) < PL_ERR_MSG_MAX_MSG, "Too long of an error message for plErrorMsg.");
            hsStrncpy(fMsg, msg, PL_ERR_MSG_MAX_MSG); 
        }
        else
            *fMsg = 0;
    }

    return *this;
}

plErrorMsg &plErrorMsg::Set(hsBool bogus, const char* label, const char* format, const char* str) 
{ 
    if( fBogus = bogus )
    {
        if( label )
            hsStrncpy(fLabel, label, 256);
        else
            *fLabel = 0;
        int length = sprintf(fMsg, format, str); 
        hsAssert(length < PL_ERR_MSG_MAX_MSG, "Too long of an error message for plErrorMsg.");
    }

    return *this;
}

plErrorMsg &plErrorMsg::Set(hsBool bogus, const char* label, const char* format, const char* str1, const char* str2) 
{ 
    if( fBogus = bogus )
    {
        if( label )
            hsStrncpy(fLabel, label, 256);
        else
            *fLabel = 0;
        int length = sprintf(fMsg, format, str1, str2); 
        hsAssert(length < PL_ERR_MSG_MAX_MSG, "Too long of an error message for plErrorMsg.");
    }

    return *this;
}

plErrorMsg &plErrorMsg::Set(hsBool bogus, const char* label, const char* format, int n) 
{ 
    if( fBogus = bogus )
    {
        if( label )
            hsStrncpy(fLabel, label, 256);
        else
            *fLabel = 0;
        int length = sprintf(fMsg, format, n); 
        hsAssert(length < PL_ERR_MSG_MAX_MSG, "Too long of an error message for plErrorMsg.");
    }

    return *this;
}

plErrorMsg &plErrorMsg::Set(hsBool bogus, const char* label, const char* format, int n, int m) 
{ 
    if( fBogus = bogus )
    {
        if( label )
            hsStrncpy(fLabel, label, 256);
        else
            *fLabel = 0;
        int length = sprintf(fMsg, format, n, m); 
        hsAssert(length < PL_ERR_MSG_MAX_MSG, "Too long of an error message for plErrorMsg.");
    }

    return *this;
}

plErrorMsg &plErrorMsg::Set(hsBool bogus, const char* label, const char* format, float f) 
{ 
    if( fBogus = bogus )
    {
        if( label )
            hsStrncpy(fLabel, label, 256);
        else
            *fLabel = 0;
        int length = sprintf(fMsg, format, f); 
        hsAssert(length < PL_ERR_MSG_MAX_MSG, "Too long of an error message for plErrorMsg.");
    }

    return *this;
}
