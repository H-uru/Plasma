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

#ifndef plCreator_inc
#define plCreator_inc

#include <type_traits>

#include "plClassIndexMacros.h"
#include "plCreatableIndex.h"
#include "plFactory.h"

class plCreatable;
class hsKeyedObject;

class plCreator
{
private:
protected:

public:
    plCreator() { }
    virtual ~plCreator() { }

    virtual plCreatable*  Create() const = 0;
    virtual uint16_t      ClassIndex() = 0;
    virtual const char*   ClassName() const = 0;
    virtual bool          HasBaseClass(uint16_t hBase) = 0;

    template<typename _CreatableT, uint16_t _CreatableIDX>
    static constexpr bool VerifyKeyedIndex()
    {
        if constexpr (_CreatableIDX < KEYED_OBJ_DELINEATOR)
            return std::is_base_of_v<hsKeyedObject, _CreatableT>;
        return true;
    }

    template<typename _CreatableT, uint16_t _CreatableIDX>
    static constexpr bool VerifyNonKeyedIndex()
    {
        if constexpr (_CreatableIDX >= KEYED_OBJ_DELINEATOR)
            return !(std::is_base_of_v<hsKeyedObject, _CreatableT>);
        return true;
    }
};

#define VERIFY_CREATABLE(plClassName)                                                         \
                                                                                              \
static_assert(plCreator::VerifyKeyedIndex<plClassName, CLASS_INDEX_SCOPED(plClassName)>(),    \
              #plClassName " is in the KeyedObject section of plCreatableIndex but "          \
              "does not derive from hsKeyedObject.");                                         \
                                                                                              \
static_assert(plCreator::VerifyNonKeyedIndex<plClassName, CLASS_INDEX_SCOPED(plClassName)>(), \
              #plClassName " is in the non-KeyedObject section of plCreatableIndex but "      \
              "derives from hsKeyedObject.")                                                  //


#define VERIFY_EXTERNAL_CREATABLE(plClassName)                                               \
                                                                                              \
static_assert(plCreator::VerifyKeyedIndex<plClassName, EXTERN_CLASS_INDEX_SCOPED(plClassName)>(),    \
              #plClassName " is in the KeyedObject section of plCreatableIndex but "          \
              "does not derive from hsKeyedObject.");                                         \
                                                                                              \
static_assert(plCreator::VerifyNonKeyedIndex<plClassName, EXTERN_CLASS_INDEX_SCOPED(plClassName)>(), \
              #plClassName " is in the non-KeyedObject section of plCreatableIndex but "      \
              "derives from hsKeyedObject.")                                                  //


#define REGISTER_CREATABLE( plClassName )                                           \
                                                                                    \
VERIFY_CREATABLE(plClassName);                                                      \
class plClassName##__Creator : public plCreator                                     \
{                                                                                   \
public:                                                                             \
    plClassName##__Creator()                                                        \
    {                                                                               \
        plFactory::Register(CLASS_INDEX_SCOPED(plClassName), this);                 \
    }                                                                               \
    virtual ~plClassName##__Creator()                                               \
    {                                                                               \
        plFactory::UnRegister(CLASS_INDEX_SCOPED(plClassName), this);               \
    }                                                                               \
                                                                                    \
    bool HasBaseClass(uint16_t hBase) override { return plClassName::HasBaseClass(hBase); }  \
                                                                                    \
    uint16_t ClassIndex() override { return CLASS_INDEX_SCOPED(plClassName); }      \
    const char* ClassName() const override { return #plClassName; }                 \
                                                                                    \
    plCreatable* Create() const override { return new plClassName; }                \
                                                                                    \
};                                                                                  \
static plClassName##__Creator   static##plClassName##__Creator;                     \
const uint16_t plClassName::plClassName##ClassIndex = CLASS_INDEX_SCOPED(plClassName);  \
VERIFY_CREATABLE(plClassName)                                                       //


#define REGISTER_NONCREATABLE( plClassName )                                        \
                                                                                    \
class plClassName##__Creator : public plCreator                                     \
{                                                                                   \
public:                                                                             \
    plClassName##__Creator()                                                        \
    {                                                                               \
        plFactory::Register(CLASS_INDEX_SCOPED(plClassName), this);                 \
    }                                                                               \
    virtual ~plClassName##__Creator()                                               \
    {                                                                               \
        plFactory::UnRegister(CLASS_INDEX_SCOPED(plClassName), this);               \
    }                                                                               \
                                                                                    \
    bool HasBaseClass(uint16_t hBase) override { return plClassName::HasBaseClass(hBase); }  \
                                                                                    \
    uint16_t ClassIndex() override { return CLASS_INDEX_SCOPED(plClassName); }      \
    const char* ClassName() const override { return #plClassName; }                 \
                                                                                    \
    plCreatable*        Create() const override { return nullptr; }                 \
                                                                                    \
};                                                                                  \
static plClassName##__Creator   static##plClassName##__Creator;                     \
const uint16_t plClassName::plClassName##ClassIndex = CLASS_INDEX_SCOPED(plClassName);  \
VERIFY_CREATABLE(plClassName)                                                       //


#define DECLARE_EXTERNAL_CREATABLE( plClassName )                                   \
                                                                                    \
class plClassName##__Creator : public plCreator                                     \
{                                                                                   \
public:                                                                             \
    plClassName##__Creator()                                                        \
    {                                                                               \
    }                                                                               \
    virtual ~plClassName##__Creator()                                               \
    {                                                                               \
        plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);        \
    }                                                                               \
    void Register()                                                                 \
    {                                                                               \
        plFactory::Register(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);          \
    }                                                                               \
    void UnRegister()                                                               \
    {                                                                               \
        plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);        \
    }                                                                               \
                                                                                    \
    bool HasBaseClass(uint16_t hBase) override { return plClassName::HasBaseClass(hBase); }  \
                                                                                    \
    uint16_t ClassIndex() override { return EXTERN_CLASS_INDEX_SCOPED(plClassName); }  \
    const char* ClassName() const override { return #plClassName; }                 \
                                                                                    \
    plCreatable* Create() const override { return new plClassName; }                \
                                                                                    \
};                                                                                  \
static plClassName##__Creator   static##plClassName##__Creator;                     \
const uint16_t plClassName::plClassName##ClassIndex = EXTERN_CLASS_INDEX_SCOPED(plClassName);  \
VERIFY_EXTERNAL_CREATABLE(plClassName)                                              //


#define REGISTER_EXTERNAL_CREATABLE(plClassName)                                    \
static##plClassName##__Creator.Register()                                           //


#define UNREGISTER_EXTERNAL_CREATABLE(plClassName)                                  \
plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), &static##plClassName##__Creator)


#define REGISTER_EXTERNAL_NONCREATABLE( plClassName )                               \
                                                                                    \
class plClassName##__Creator : public plCreator                                     \
{                                                                                   \
public:                                                                             \
    plClassName##__Creator()                                                        \
    {                                                                               \
        plFactory::Register(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);          \
    }                                                                               \
    virtual ~plClassName##__Creator()                                               \
    {                                                                               \
        plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);        \
    }                                                                               \
                                                                                    \
    bool HasBaseClass(uint16_t hBase) override { return plClassName::HasBaseClass(hBase); }  \
                                                                                    \
    uint16_t ClassIndex() override { return EXTERN_CLASS_INDEX_SCOPED(plClassName); }  \
    const char* ClassName() const override { return #plClassName; }                 \
                                                                                    \
    plCreatable*        Create() const override { return nullptr; }                 \
                                                                                    \
};                                                                                  \
static plClassName##__Creator   static##plClassName##__Creator;                     \
const uint16_t plClassName::plClassName##ClassIndex = EXTERN_CLASS_INDEX_SCOPED(plClassName);  \
VERIFY_EXTERNAL_CREATABLE(plClassName)                                              //


#endif // plCreator_inc
