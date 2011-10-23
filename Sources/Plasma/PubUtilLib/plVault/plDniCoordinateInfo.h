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
#ifndef plDniCoordinateInfo_h_inc
#define plDniCoordinateInfo_h_inc

#include "hsConfig.h"
#include "hsStlUtils.h"
#include "pnFactory/plCreatable.h"

///////////////////////////////////////////////////////////////////

class hsStream;
class hsResMgr;


class plDniCoordinateInfo : public plCreatable
{
    static const UInt8 StreamVersion;

protected:
    // spherical coords (rho,theta,phi)
    int         fHSpans;        // horizontal distance in d'ni spans to point P from origin O
    int         fVSpans;        // vertical distance in d'ni spans to point P from origin O
    int         fTorans;        // angle in d'ni torans from the zero vector to line OP.

public:
    plDniCoordinateInfo();

    CLASSNAME_REGISTER( plDniCoordinateInfo );
    GETINTERFACE_ANY( plDniCoordinateInfo, plCreatable );

    int GetHSpans( void ) const { return fHSpans;}
    void    SetHSpans( int v ) { fHSpans = v; }
    int GetVSpans( void ) const { return fVSpans;}
    void    SetVSpans( int v ) { fVSpans = v;}
    int GetTorans( void ) const { return fTorans; }
    void    SetTorans( int v ) { fTorans = v; }

    void    CopyFrom( const plDniCoordinateInfo * other );
    void    Read( hsStream* s, hsResMgr* mgr );
    void    Write( hsStream* s, hsResMgr* mgr );

    // debug
    std::string AsStdString( int level=0 ) const;
};



#endif // plDniCoordinateInfo_h_inc
