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

#ifndef plDXShader_inc
#define plDXShader_inc

#include "plDXDeviceRef.h"
#include <string_theory/string>

class plShader;
class plDXPipeline;

class plDXShader : public plDXDeviceRef
{
protected:
    plShader*           fOwner;
    ST::string          fErrorString;
    plDXPipeline*       fPipe;

    HRESULT             IOnError(HRESULT hr, const char* errStr);
    void                ISetError(const char* errStr) { fErrorString = errStr; }

    virtual HRESULT     ICreate(plDXPipeline* pipe) = 0;
    virtual HRESULT     ISetConstants(plDXPipeline* pipe) = 0; // On error, sets error string.

public:
    plDXShader(plShader* owner);
    virtual ~plDXShader();

    ST::string      GetErrorString() const { return fErrorString; }
    void            SetOwner(plShader* owner);
};

#endif // plDXShader_inc
