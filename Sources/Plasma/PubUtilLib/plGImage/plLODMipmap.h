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

#ifndef plLODMipmap_inc
#define plLODMipmap_inc

#include "plMipmap.h"

class plLODMipmap : public plMipmap
{
protected:
    enum 
    {
        kRefBase    = 0
    };

    enum
    {
        kNumLODs    = 5
    };

    plMipmap*       fBase;
    int             fLOD;

    hsGDeviceRef*   fDeviceRefs[kNumLODs];

    void            ISetup();
    void            ISetupCurrLevel();
    void            IMarkDirty();
    void            INilify();


public:
    plLODMipmap();
    plLODMipmap(plMipmap* mip);
    virtual ~plLODMipmap();
    
    CLASSNAME_REGISTER( plLODMipmap );
    GETINTERFACE_ANY( plLODMipmap, plMipmap );

    bool MsgReceive(plMessage *msg) override;

    void            SetLOD(int lod);
    int             GetLOD() const { return fLOD; }

    hsGDeviceRef*   GetDeviceRef() const override;
    void            SetDeviceRef(hsGDeviceRef *const devRef) override;

    void    Reset() override;

    void    Read(hsStream *s, hsResMgr *mgr) override;
    void    Write(hsStream *s, hsResMgr *mgr) override;

    plMipmap*   Clone() const override { return fBase->Clone(); }
    void        CopyFrom(const plMipmap *source) override;

    void    Composite(plMipmap *source, uint16_t x, uint16_t y, CompositeOptions *options = nullptr) override;

    void    ScaleNicely(uint32_t *destPtr, uint16_t destWidth, uint16_t destHeight,
                        uint16_t destStride, plMipmap::ScaleFilter filter) const override;

    bool    ResizeNicely(uint16_t newWidth, uint16_t newHeight, plMipmap::ScaleFilter filter) override;

    void    SetCurrLevel(uint8_t level) override;

    const plMipmap* GetBase() const { return fBase; }
};


#endif // plLODMipmap_inc
