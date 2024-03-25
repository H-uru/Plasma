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
#ifndef plNetCommonHelpers_h_inc
#define plNetCommonHelpers_h_inc

#include "HeadSpin.h"

#include <map>
#include <string>
#include <vector>

#include "pnFactory/plCreatable.h"

namespace ST { class string; }

////////////////////////////////////////////////////////////////////

class plCreatableListHelper : public plCreatable
{
    enum { kDefaultCompressionThreshold = 255 }; // bytes
    enum Flags
    {
        kWantCompression    = 1<<0,
        kCompressed         = 1<<1,
        kWritten            = 1<<2,
    };
    uint8_t                             fFlags;
    std::map<uint16_t,plCreatable*>     fItems;
    mutable std::vector<plCreatable*>   fManagedItems;
    uint32_t  fCompressionThreshold;  // NOT WRITTEN
    std::string fWritten;
    void IClearItems();

public:
    plCreatableListHelper();
    ~plCreatableListHelper() { IClearItems();}

    CLASSNAME_REGISTER( plCreatableListHelper );
    GETINTERFACE_ANY( plCreatableListHelper, plCreatable );

    void    Read(hsStream* s, hsResMgr* mgr) override;
    void    Write(hsStream* s, hsResMgr* mgr) override;
    void    Clear() { IClearItems();    }
    void    CopyFrom( const plCreatableListHelper * other, bool manageItems );

    void    SetWantCompression( bool v ) { if ( v ) fFlags|=kWantCompression; else fFlags&=~kWantCompression; }
    bool    WantCompression() const { return ( fFlags&kWantCompression )!=0; }
    uint32_t  GetCompressionThreshold() const { return fCompressionThreshold; }
    void    SetCompressionThreshold( uint32_t v ) { fCompressionThreshold=v; }
    
    // support for generic arguments
    void    AddItem( uint16_t id, plCreatable * item, bool manageItem=false );
    void    AddItem( uint16_t id, const plCreatable * item, bool manageItem=false );
    plCreatable* GetItem( uint16_t id, bool unManageItem=false ) const;
    void    RemoveItem( uint16_t id, bool unManageItem=false );
    bool    ItemExists( uint16_t id ) const;
    size_t  GetNumItems() const { return fItems.size();}
    // helpers for typed arguments
    void    AddString( uint16_t id, const char * value );
    void    AddString( uint16_t id, std::string & value );
    ST::string  GetString( uint16_t id );
    void    AddInt( uint16_t id, int32_t value );
    int32_t   GetInt( uint16_t id );
    void    AddDouble( uint16_t id, double value );
    double  GetDouble( uint16_t id );
    void    GetItemsAsVec( std::vector<plCreatable*>& out );
    void    GetItems( std::map<uint16_t,plCreatable*>& out );
};

#endif // plNetCommonHelpers_h_inc
