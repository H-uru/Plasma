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

#ifndef plFactory_inc
#define plFactory_inc

#include "hsRefCnt.h"
#include "HeadSpin.h"
#ifdef PLFACTORY_PRIVATE
    #include <vector>
#endif

class plCreator;
class plCreatable;
class hsStream;
class hsResMgr;

class plFactory : public hsRefCnt
{
#ifdef PLFACTORY_PRIVATE
private:
    std::vector<plCreator*> fCreators;

    void                IForceShutdown();
    void                IUnRegister(uint16_t hClass);
    uint16_t              IRegister(uint16_t hClass, plCreator* worker);
    bool                IIsEmpty();
    uint16_t              IGetNumClasses();
    plCreatable*        ICreate(uint16_t hClass);
    bool                IDerivesFrom(uint16_t hBase, uint16_t hDer);
    bool                IIsValidClassIndex(uint16_t hClass);

    static bool         ICreateTheFactory();
    static void         IShutdown();

    plFactory();
    ~plFactory();
#endif

public:
    // Don't use this unless you're initializing a DLL
    friend class plClient;
    static plFactory*   GetTheFactory();


    static uint16_t       Register(uint16_t hClass, plCreator* worker); // returns hClass
    static void         UnRegister(uint16_t hClass, plCreator* worker);

    static bool         CanCreate(uint16_t hClass);   // return true if creator exists. doesn't assert
    static plCreatable* Create(uint16_t hClass);

    static bool         DerivesFrom(uint16_t hBase, uint16_t hDer);

    static uint16_t       GetNumClasses();

    static uint16_t       FindClassIndex(const char* className);      // slow lookup for things like console

    static bool         IsValidClassIndex(uint16_t hClass);

    // Don't call this unless you're a DLL being initialized.
    static void         SetTheFactory(plFactory* fac);

    static const char   *GetNameOfClass(uint16_t type);
};



#endif // plFactory_inc
