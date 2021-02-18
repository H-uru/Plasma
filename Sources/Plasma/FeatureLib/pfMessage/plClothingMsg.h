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
#ifndef plClothingMsg_inc
#define plClothingMsg_inc

#include "pnMessage/plRefMsg.h"
#include "hsStream.h"
#include "hsColorRGBA.h"

class hsResMgr;

class plClothingMsg : public plMessage
{
protected:
    uint32_t fCommands;

public:
    plKey fItemKey;
    hsColorRGBA fColor;
    uint8_t fLayer;
    uint8_t fDelta;
    float fWeight;

    plClothingMsg() : fCommands(), fLayer(), fDelta(), fWeight() { fColor.Set(1.f, 1.f, 1.f, 1.f); }
    ~plClothingMsg() {}

    CLASSNAME_REGISTER( plClothingMsg );
    GETINTERFACE_ANY( plClothingMsg, plMessage );

    enum commands
    {
        kAddItem =              0x0001,
        kRemoveItem =           0x0002,
        kUpdateTexture =        0x0004,
        kTintItem =             0x0008,
        kRetry =                0x0010,
        kTintSkin =             0x0020,
        kBlendSkin =            0x0040,
        kMorphItem =            0x0080,
        kSaveCustomizations =   0x0100,
    };

    bool GetCommand(uint32_t command) { return fCommands & command; }
    void AddCommand(uint32_t command) { fCommands |= command; }
    bool ResendUpdate() { return fCommands != kUpdateTexture; }

    // IO 
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
    
    // WriteVersion writes the current version of this creatable and ReadVersion will read in
    // any previous version.
    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
};

class plElementRefMsg : public plGenRefMsg
{
public:
    ST::string  fElementName;
    uint32_t    fLayer;

    plElementRefMsg() : plGenRefMsg(), fLayer(1) {}
    plElementRefMsg(const plKey &r, uint8_t c, int which, int type, const ST::string &name, uint8_t layer) : plGenRefMsg(r, c, which, type)
    {
        fLayer = layer;
        fElementName = name;
    }

    CLASSNAME_REGISTER( plElementRefMsg );
    GETINTERFACE_ANY( plElementRefMsg, plGenRefMsg );
};

class plClothingUpdateBCMsg : public plMessage
{
public:
    plClothingUpdateBCMsg();
    ~plClothingUpdateBCMsg() {}

    CLASSNAME_REGISTER( plClothingUpdateBCMsg );
    GETINTERFACE_ANY( plClothingUpdateBCMsg, plMessage );   

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

#endif // plClothingMsg_inc
