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

#ifndef plCreatable_inc
#define plCreatable_inc

#include "HeadSpin.h"

#include "hsRefCnt.h"

class plCreator;
class hsStream;
class hsResMgr;


class plCreatable : public hsRefCnt
{
public:
    virtual const char*         ClassName() const = 0;
    virtual plCreatable*        GetInterface(uint16_t hClass) { return nullptr; }
    virtual const plCreatable*  GetConstInterface(uint16_t hClass) const { return nullptr; }
    static bool                 HasBaseClass(uint16_t hBase) { return false; }
    virtual uint16_t            ClassIndex() const = 0;

    virtual void Read(hsStream* s, hsResMgr* mgr) {}
    virtual void Write(hsStream* s, hsResMgr* mgr) {}

    // WriteVersion writes the current version of this creatable and ReadVersion will read in
    // any previous version.
    virtual void ReadVersion(hsStream* s, hsResMgr* mgr) {
        hsAssert(false, "ReadVersion not implemented!");
    }
    virtual void WriteVersion(hsStream* s, hsResMgr* mgr) {
        hsAssert(false, "WriteVersion not implemented!");
    }
};


// Macros:
//  NOTE: Comfortable use of these macros assumes the compiler is comfortable
//        eating a spurious semi-colon (;) following a curly brace. If that
//        isn't the case, they can easily be wrapped in something like
//        do { original macro } while(0) or the like.
//
//  Normal setup for a class:
//  In public section of class declaration, insert the following two macros:
//      CLASSNAME_REGISTER( myClassName );
//      GETINTERFACE_ANY( myClassName, classIWasDerivedFromName );
//  Then in the *Creatable.h file for that library (e.g. plSurfaceCreatable.h), add macro
//      REGISTER_CREATABLE( myClassName )
//  Finally, add an enum to the plCreatableIndex.h file using CLASS_INDEX(className)
//      ( e.g. CLASS_INDEX(hsGMaterial) )
//
//  CLASSNAME_REGISTER( plClassName ) - Sets up identification for this
//      class. The exposed methods are
//      static  uint16_t Index() - returns the index for that class.
//      virtual uint16_t ClassIndex() - returns index for this object's class.
//      static plClassName* Convert(plCreatable* c) - if c exposes an interface
//          as plClassName, return that, else nullptr. Incs the ref count of the object.
//      static plClassName* ConvertNoRef(plCreatable* c) - Same as Convert(), but
//          doesn't inc the ref count.
//  Insert into public section of class definition.
//
//  Normally one of the next 3 macros should follow CLASSNAME_REGISTER
//  GETINTERFACE_ANY - allows an interface to an object as plClassName if an object 
//      is or is derived from plClassName.
//  GETINTERFACE_EXACT - allows an interface as plClassName only if the type is 
//      exactly of type plClassName
//  GETINTERFACE_NONE - Never provide an interface as plClassName.
//  Instead of using these macros, the class can provide a method
//      virtual plCreatable* GetInterface(uint16_t hClass) which returns an object of
//      type matching class handle hClass.
//  Insert into public section of class definition (like right after CLASSNAME_REGISTER).
//
//  REGISTER_CREATABLE( plClassName ) - normal creatable type, any you can instantiate.
//  or
//  REGISTER_NONCREATABLE( plClassName ) - can't be created either because it's pure virtual
//      or just doesn't want to be creatable. It's Create member returns nullptr. But Convert
//      may return an interface, depending on the GETINTERFACE above.
//  - This line is the only exposure to the plCreator.
//  This will define a Creator for class plClassName, instantiate it as a static, and register
//  it with the Factory. The registration also sets the class index value in the plCreator
//  subclass, as well as in the class being registered. 
//  Put after includes in the *Creatable.h file for the library the class belongs to..
//
//  USAGE:
//  There is a method of identifying an object's type. You should rarely need it,
//  using Convert() instead.
//  ClassIndex() the class handle is an immutable index to this class. It provides an 
//      instantaneous lookup. It may be stored, loaded, sent over the wire, etc.
//
//  Create()
//  If you know what type object you want to create at compile time, use
//      new <ObjectType>()
//  But if you have a class index at run-time (e.g. loaded from file), use
//      plCreatable* plFactory::Create(hClass); 
//  The ultra-safe way to do this is:
//      plCreatable* tmp = plFactory::Create(idx);
//      plWantClassName* p = plWantClassName::Convert(tmp);
//      hsRefCnt_SafeUnRef(tmp);
//
//  If you have a fred interface to an object f, and want a wilma interface, use
//      fred* f = new fred();  more likely f was passed in.
//      wilma* w = wilma::Convert(f)
//  NOTE that two strange things may be true here:
//      1) f != nullptr, w == nullptr
//          either fred's not really derived from wilma, 
//          or fred doesn't like to be cast down,
//          or wilma just doesn't want to expose an interface.
//      2) f != nullptr, w != nullptr, and f != w
//          fred has pulled a sneaky and created a wilma to return.
//          so unrelated classes can still "Convert" as one another.
//
//
////////////////////////////
// EAp - 01/10/2003
// Added macros to support multiple AUX interfaces primarily,
// but they are not limited to that. Usage example:
// 
//  plBeginInterfaceMap( plMyClass, plBaseClass );
//      plAddInterfaceAux( plFooClass, fFooMember );
//      plAddInterfaceAux( plBarClass, fBarMember );
//      plAddInterface( plSomeOtherClass );
//  plEndInterfaceMap();
//


#define CLASSNAME_REGISTER(plClassName)                                     \
public:                                                                     \
    const char* ClassName() const override {                                \
        return #plClassName;                                                \
    }                                                                       \
private:                                                                    \
    static const uint16_t plClassName##ClassIndex;                          \
public:                                                                     \
    uint16_t ClassIndex() const override {                                  \
        return plClassName::Index();                                        \
    }                                                                       \
    static uint16_t Index() {                                               \
        return plClassName##ClassIndex;                                     \
    }                                                                       \
    static plClassName* ConvertNoRef(plCreatable* c) {                      \
        plClassName* retVal = c                                             \
            ? static_cast<plClassName*>(                                    \
                    c->GetInterface(plClassName##ClassIndex))               \
            : nullptr;                                                      \
        return retVal;                                                      \
    }                                                                       \
    static const plClassName* ConvertNoRef(const plCreatable* c) {          \
        const plClassName* retVal = c                                       \
            ? static_cast<const plClassName*>(                              \
                    c->GetConstInterface(plClassName##ClassIndex))          \
            : nullptr;                                                      \
        return retVal;                                                      \
    }                                                                       \
    static plClassName* Convert(plCreatable* c) {                           \
        plClassName* retVal = ConvertNoRef(c);                              \
        hsRefCnt_SafeRef(retVal);                                           \
        return retVal;                                                      \
    }



#define GETINTERFACE_ANY(plClassName, plBaseName)                           \
    static bool HasBaseClass(uint16_t hBaseClass) {                         \
        if (hBaseClass == plClassName##ClassIndex)                          \
            return true;                                                    \
        else                                                                \
            return plBaseName::HasBaseClass(hBaseClass);                    \
    }                                                                       \
    plCreatable* GetInterface(uint16_t hClass) override {                   \
        if (hClass == plClassName##ClassIndex)                              \
            return this;                                                    \
        else                                                                \
            return plBaseName::GetInterface(hClass);                        \
    }                                                                       \
    const plCreatable* GetConstInterface(uint16_t hClass) const override {  \
        if (hClass == plClassName##ClassIndex)                              \
            return this;                                                    \
        else                                                                \
            return plBaseName::GetConstInterface(hClass);                   \
    }



#define GETINTERFACE_EXACT(plClassName)                                     \
    static bool HasBaseClass(uint16_t hBaseClass) {                         \
        return hBaseClass == plClassName##ClassIndex;                       \
    }                                                                       \
    plCreatable* GetInterface(uint16_t hClass) override {                   \
        return hClass == plClassName##ClassIndex ? this : nullptr;          \
    }                                                                       \
    const plCreatable* GetConstInterface(uint16_t hClass) const override {  \
        return hClass == plClassName##ClassIndex ? this : nullptr;          \
    }



#define GETINTERFACE_NONE(plClassName)                                      \
    static bool HasBaseClass(uint16_t hBaseClass) { return false; }         \
    plCreatable* GetInterface(uint16_t hClass) override {                   \
        return nullptr;                                                     \
    }                                                                       \
    const plCreatable* GetConstInterface(uint16_t hClass) const override {  \
        return nullptr;                                                     \
    }


//
// Macro for converting to base class OR a class member
//
#define GETINTERFACE_ANY_AUX(plClassName, plBaseName, plAuxClassName, plAuxClassMember) \
    static bool HasBaseClass(uint16_t hBaseClass) {                         \
        if (hBaseClass == plClassName##ClassIndex)                          \
            return true;                                                    \
        else                                                                \
            return plBaseName::HasBaseClass(hBaseClass);                    \
    }                                                                       \
    plCreatable* GetInterface(uint16_t hClass) override {                   \
        if (hClass == plClassName##ClassIndex)                              \
            return this;                                                    \
        else if (hClass == plAuxClassName::Index())                         \
            return &plAuxClassMember;                                       \
        else                                                                \
            return plBaseName::GetInterface(hClass);                        \
    }                                                                       \
    const plCreatable* GetConstInterface(uint16_t hClass) const override {  \
        if (hClass == plClassName##ClassIndex)                              \
            return this;                                                    \
        else if (hClass == plAuxClassName::Index())                         \
            return &plAuxClassMember;                                       \
        else                                                                \
            return plBaseName::GetConstInterface(hClass);                   \
    }

#define plBeginInterfaceMap(plClassName, plBaseName)                        \
    static bool HasBaseClass(uint16_t hBaseClass) {                         \
        if (hBaseClass == plClassName##ClassIndex)                          \
            return true;                                                    \
        else                                                                \
            return plBaseName::HasBaseClass(hBaseClass);                    \
    }                                                                       \
    plCreatable* GetInterface(uint16_t hClass) override {                   \
        /* NOTE: pulling const off the ptr should be ok, right? */          \
        return const_cast<plCreatable*>(GetConstInterface(hClass));         \
    }                                                                       \
    const plCreatable* GetConstInterface(uint16_t hClass) const override {  \
        typedef plBaseName MyBaseClass;                                     \
        if (hClass == plClassName##ClassIndex)                              \
            return this

#define plAddInterface(plClassName)                                         \
        else if (hClass == plClassName::Index())                            \
            return plClassName::GetConstInterface(hClass)

#define plAddInterfaceAux(plAuxClassName, plAuxClassMember)                 \
        else if (hClass == plAuxClassName::Index())                         \
            return &plAuxClassMember

#define plEndInterfaceMap()                                                 \
        else                                                                \
            return MyBaseClass::GetConstInterface(hClass);                  \
    }


#endif // plCreatable_inc
