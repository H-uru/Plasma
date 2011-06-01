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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsTypes.h"
#include "hsUtils.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plJPEG/plJPEG.h"
#include "plGImage/plPNG.h"
#include "plGImage/plMipmap.h"

#include "plClientResMgr.h"


//// Singleton Instance ///////////////////////////////////////////////////////

plClientResMgr& plClientResMgr::Instance(void)
{
    static plClientResMgr theInstance;
    return theInstance;
}

plClientResMgr::plClientResMgr()
{
    this->ClientResources = TRACKED_NEW std::map<std::string, plMipmap*>;
}

plClientResMgr::~plClientResMgr()
{
    if (this->ClientResources) {
        std::map<std::string, plMipmap*>::iterator it;

        for (it = this->ClientResources->begin(); it != this->ClientResources->end(); ++it) {
            it->second->UnRef();
        }

        delete this->ClientResources;
    }
}

void plClientResMgr::ILoadResources(const char* resfile)
{
    if (!resfile) {
        return;
    }

    wchar* wFilename = hsStringToWString(resfile);
    hsUNIXStream in;

    if (in.Open(wFilename, L"rb")) {
        UInt32 header = in.ReadSwap32();
        UInt32 version = in.ReadSwap32();
        UInt32 num_resources = 0;

        switch (version) {
            case 1:
                num_resources = in.ReadSwap32();

                for (int i = 0; i < num_resources; i++) {
                    plMipmap* res_data = NULL;
                    UInt32 res_size = 0;
                    char* tmp_name = in.ReadSafeStringLong();
                    std::string res_name = std::string(tmp_name);
                    std::string res_type = res_name.substr(res_name.length() - 4, 4);
                    delete tmp_name;

                    // Version 1 doesn't encode format, so we'll try some simple
                    // extension sniffing
                    if (res_type == ".png") {
                        // Read resource stream size, but the PNG has that info in the header
                        // so it's not needed
                        res_size = in.ReadSwap32();
                        res_data = plPNG::Instance().ReadFromStream(&in);
                    } else if (res_type == ".jpg") {
                        // Don't read resource stream size, as plJPEG's reader will need it
                        res_data = plJPEG::Instance().ReadFromStream(&in);
                    } else {
                        // Original Myst5 format only is known to support Targa,
                        // so default fallback is targa
                        // TODO - Add plTarga::ReadFromStream()
                    }

                    (*this->ClientResources)[res_name] = res_data;
                }

                break;
            default:
                break;
        }

        in.Close();
    }

    delete wFilename;
}

plMipmap* plClientResMgr::getResource(const char* resname)
{
    plMipmap* resmipmap = NULL;
    std::map<std::string, plMipmap*>::iterator it = this->ClientResources->find(resname);

    if (it != this->ClientResources->end()) {
        resmipmap = it->second;
    } else {
        hsAssert(resmipmap, "Unknown client resource requested.");
    }

    return resmipmap;
}
