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
#include "pnNetCommon/pnNetCommon.h"

/////////////////////////////////////////////////////////////////////////////////
// SDL MGR
/////////////////////////////////////////////////////////////////////////////////

//
//
//
plSDLMgr::plSDLMgr() : fSDLDir("SDL"), fNetApp(), fBehaviorFlags()
{

}

//
//
//
plSDLMgr::~plSDLMgr()
{
    DeInit();
}

bool plSDLMgr::Init( uint32_t behaviorFlags )
{
    fBehaviorFlags = behaviorFlags;
    plSDLParser parser;
    return parser.Parse();
}

void plSDLMgr::DeInit()
{
    IDeleteDescriptors(&fDescriptors);
}

//
// delete all descriptors
//
void plSDLMgr::IDeleteDescriptors(plSDL::DescriptorList* dl)
{
    for (plStateDescriptor* sd : *dl)
        delete sd;
    dl->clear();
}


//
// STATIC
//
plSDLMgr* plSDLMgr::GetInstance()
{
    static plSDLMgr gSDLMgr;
    return &gSDLMgr;
}

//
// search latest and legacy descriptors for one that matches.
// if version is -1, search for latest descriptor with matching name
//
plStateDescriptor* plSDLMgr::FindDescriptor(const ST::string& name, int version, const plSDL::DescriptorList * dl) const
{
    if (name.empty())
        return nullptr;

    if ( !dl )
        dl = &fDescriptors;

    plStateDescriptor* sd = nullptr;

    plSDL::DescriptorList::const_iterator it;

    int highestFound = -1;
    for(it=(*dl).begin(); it!= (*dl).end(); it++)
    {
        if (!(*it)->GetName().compare_i(name) )
        {
            if ( (*it)->GetVersion()==version )
            {
                sd = *it;
                break;
            }
            else if ( version==plSDL::kLatestVersion && (*it)->GetVersion()>highestFound )
            {
                sd = *it;
                highestFound = (*it)->GetVersion();
            }
        }
    }

    return sd;
}

//
// write latest descriptors to a stream.
// return number of bytes
//
int plSDLMgr::Write(hsStream* s, const plSDL::DescriptorList* dl)
{
    int pos=s->GetPosition();

    if (dl == nullptr)
        dl=&fDescriptors;

    size_t num = dl->size();
    hsAssert(num < std::numeric_limits<uint16_t>::max(), "Too many descriptors");
    s->WriteLE16(uint16_t(num));

    plSDL::DescriptorList::const_iterator it;
    for (const plStateDescriptor* desc : *dl)
        desc->Write(s);

    int bytes=s->GetPosition()-pos;
    if (fNetApp)
    {
        hsLogEntry(fNetApp->DebugMsg("Writing {} SDL descriptors, {} bytes", num, bytes));
    }
    return bytes;
}

//
// read descriptors into provided list 
// return number of bytes
//
int plSDLMgr::Read(hsStream* s, plSDL::DescriptorList* dl)
{
    int pos=s->GetPosition();

    if (dl == nullptr)
        dl=&fDescriptors;

    // clear dl
    IDeleteDescriptors(dl);

    uint16_t num;
    try
    {       
        // read dtor list
        s->ReadLE16(&num);

        int i;
        for(i=0;i<num;i++)
        {
            plStateDescriptor* sd=new plStateDescriptor;
            if (sd->Read(s))
                dl->push_back(sd);
            else
                delete sd; // well that sucked
        }
    }
    catch (std::exception &e)
    {
        if (fNetApp)
        {
            hsLogEntry(fNetApp->DebugMsg("Something bad happened while reading SDLMgr data: {}", e.what()));
        }
        else
        {
            DebugMsg("Something bad happened while reading SDLMgr data: {}", e.what());
        }
        return 0;
    }
    catch (...)
    {
        if (fNetApp)
        {
            hsLogEntry(fNetApp->DebugMsg("Something bad happened while reading SDLMgr data"));
        }
        return 0;
    }

    int bytes=s->GetPosition()-pos;
    if (fNetApp)
    {
        hsLogEntry(fNetApp->DebugMsg("Reading {} SDL descriptors, {} bytes", num, bytes));
    }
    return bytes;
}
