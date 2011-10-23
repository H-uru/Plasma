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
#ifndef plPXStream_h_inc
#define plPXStream_h_inc

#include "hsStream.h"
#include <NxPhysics.h>
#include <NxStream.h>

// A super simple wrapper to convert a Plasma stream into a PhysX one
class plPXStream : public NxStream
{
public:
    plPXStream(hsStream* s) : fStream(s) {}

    virtual NxU8        readByte() const { return fStream->ReadByte(); }
    virtual NxU16       readWord() const { return fStream->ReadLE16(); }
    virtual NxU32       readDword() const { return fStream->ReadLE32(); }
    virtual float       readFloat() const { return fStream->ReadLEScalar(); }
    virtual double      readDouble() const { return fStream->ReadLEDouble(); }
    virtual void        readBuffer(void* buffer, NxU32 size) const { fStream->Read(size, buffer); }

    virtual NxStream&   storeByte(NxU8 b) { fStream->WriteByte(b); return *this; }
    virtual NxStream&   storeWord(NxU16 w) { fStream->WriteLE16(w); return *this; }
    virtual NxStream&   storeDword(NxU32 d) { fStream->WriteLE32(d); return *this; }
    virtual NxStream&   storeFloat(NxReal f) { fStream->WriteLEScalar(f); return *this; }
    virtual NxStream&   storeDouble(NxF64 f) { fStream->WriteLEDouble(f); return *this; }
    virtual NxStream&   storeBuffer(const void* buffer, NxU32 size) { fStream->Write(size, buffer); return *this; }

protected:
    hsStream* fStream;
};

#endif // plPXStream_h_inc
