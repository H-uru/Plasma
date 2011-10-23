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

#ifdef PLFACTORY_PRIVATE
    #include "hsTemplates.h"
#endif
#include "hsRefCnt.h"
#include "hsTypes.h"

class plCreator;
class plCreatable;
class hsStream;
class hsResMgr;

class plFactory : public hsRefCnt
{
#ifdef PLFACTORY_PRIVATE
private:
    hsTArray<plCreator*>        fCreators;

    void                IForceShutdown();
    void                IUnRegister(UInt16 hClass);
    UInt16              IRegister(UInt16 hClass, plCreator* worker);
    hsBool              IIsEmpty();
    UInt16              IGetNumClasses();
    plCreatable*        ICreate(UInt16 hClass);
    hsBool              IDerivesFrom(UInt16 hBase, UInt16 hDer);
    hsBool              IIsValidClassIndex(UInt16 hClass);

    static hsBool       ICreateTheFactory();
    static void         IShutdown();

    plFactory();
    ~plFactory();
#endif

public:
    // Don't use this unless you're initializing a DLL
    friend class plClient;
    static plFactory*   GetTheFactory();


    static UInt16       Register(UInt16 hClass, plCreator* worker); // returns hClass
    static void         UnRegister(UInt16 hClass, plCreator* worker);

    static bool         CanCreate(UInt16 hClass);   // return true if creator exists. doesn't assert
    static plCreatable* Create(UInt16 hClass);

    static hsBool       DerivesFrom(UInt16 hBase, UInt16 hDer);

    static UInt16       GetNumClasses();

    static UInt16       FindClassIndex(const char* className);      // slow lookup for things like console

    static hsBool       IsValidClassIndex(UInt16 hClass);

    // Don't call this unless you're a DLL being initialized.
    static void         SetTheFactory(plFactory* fac);

    static const char   *GetNameOfClass(UInt16 type);

#ifdef HS_DEBUGGING
    void                IValidate(UInt16 keyIndex);
    static  void        Validate(UInt16 keyIndex);
#endif
};



#endif // plFactory_inc
