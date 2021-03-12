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

#include "plSDL.h"

#include "hsStream.h"

#include "pnNetCommon/plNetApp.h"

const uint8_t plStateDescriptor::kVersion=1;      // for Read/Write format

/////////////////////////////////////////////////////////////////////////////////
// STATE DESC
/////////////////////////////////////////////////////////////////////////////////

plStateDescriptor::~plStateDescriptor() 
{ 
    IDeInit();
}

void plStateDescriptor::IDeInit()
{
    int i;
    for(i=0;i<fVarsList.size();i++)
        delete fVarsList[i];
    fVarsList.clear();
}

plVarDescriptor* plStateDescriptor::FindVar(const ST::string& name, int* idx) const
{
    VarsList::const_iterator it;
    for(it=fVarsList.begin(); it != fVarsList.end(); it++)
    {
        if (!(*it)->GetName().compare_i(name))
        {
            if (idx)
                *idx = it-fVarsList.begin();
            return *it;
        }
    }

    return nullptr;
}


//
// Usage: The GameServer reads and write state descriptors along with each saved game
// 
bool plStateDescriptor::Read(hsStream* s)
{
    uint8_t rwVersion = s->ReadByte();
    if (rwVersion != kVersion)
    {
        plNetApp::StaticWarningMsg("StateDescriptor Read/Write version mismatch, mine {}, read {}", kVersion, rwVersion);
        return false;
    }

    IDeInit();

    fName = s->ReadSafeString();

    uint16_t version=s->ReadLE16();
    fVersion=version;

    uint16_t numVars=s->ReadLE16();
    fVarsList.reserve(numVars);

    int i;
    for(i=0;i<numVars; i++)
    {
        uint8_t SDVar=s->ReadByte();      
        plVarDescriptor* var = nullptr;
        if (SDVar)
            var = new plSDVarDescriptor;
        else
            var = new plSimpleVarDescriptor;
        if (var->Read(s))
            fVarsList.push_back(var);
        else
            return false;
    }
    return true;
}

//
// Usage: The GameServer reads and write state descriptors alon with each saved game
// 
void plStateDescriptor::Write(hsStream* s) const
{
    s->WriteByte(kVersion);
    
    s->WriteSafeString(fName);

    s->WriteLE16((uint16_t)fVersion);

    size_t numVars = fVarsList.size();
    hsAssert(numVars < std::numeric_limits<uint16_t>::max(), "Too many variables");
    s->WriteLE16(uint16_t(numVars));

    VarsList::const_iterator it;
    for (const plVarDescriptor* desc : fVarsList) {
        auto SDVar = (uint8_t)(desc->GetAsSDVarDescriptor() != nullptr);
        s->WriteByte(SDVar);
        desc->Write(s);
    }
}

