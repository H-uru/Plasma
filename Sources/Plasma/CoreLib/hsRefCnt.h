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
#ifndef hsRefCnt_Defiend
#define hsRefCnt_Defiend

#include <atomic>
#include <cstddef>

class hsRefCnt {
private:
    std::atomic<int> fRefCnt;

public:
                hsRefCnt(int initRefs = 1);
    virtual     ~hsRefCnt();

    inline int  RefCnt() const { return fRefCnt; }
    void        UnRef(const char* tag = nullptr);
    void        Ref(const char* tag = nullptr);

    // Useless, but left here for debugging compatibility with AtomicRef
    void        TransferRef(const char* oldTag, const char* newTag);


    // The stored reference count of an hsRefCnt-derived object should never
    // be copied! Therefore, if you want a copyable hsRefCnt-based class, you
    // should implement your own copy constructor / assignment operator.
    hsRefCnt(const hsRefCnt &) = delete;
    hsRefCnt &operator=(const hsRefCnt &) = delete;
};

#define hsRefCnt_SafeRef(obj)   do { if (obj) (obj)->Ref(); } while (0)
#define hsRefCnt_SafeUnRef(obj) do { if (obj) (obj)->UnRef(); } while (0)

#define hsRefCnt_SafeAssign(dst, src)   \
        do {                            \
            hsRefCnt_SafeRef(src);      \
            hsRefCnt_SafeUnRef(dst);    \
            dst = src;                  \
        } while (0)

struct hsStealRef_Type {};
constexpr hsStealRef_Type hsStealRef;

template <class _Ref>
class hsWeakRef;

template <class _Ref>
class hsRef
{
public:
    hsRef() : fObj() { }
    hsRef(std::nullptr_t) : fObj() { }
    hsRef(_Ref *obj, hsStealRef_Type) : fObj(obj) { }
    hsRef(const hsRef<_Ref> &copy) : fObj(copy.fObj) { if (fObj) fObj->Ref(); }
    hsRef(hsRef<_Ref> &&move) : fObj(move.fObj) { move.fObj = nullptr; }

    ~hsRef() { if (fObj) fObj->UnRef(); }

    hsRef<_Ref> &operator=(const hsRef<_Ref> &copy)
    {
        if (copy.fObj)
            copy.fObj->Ref();
        if (fObj)
            fObj->UnRef();
        fObj = copy.fObj;
        return *this;
    }

    hsRef<_Ref> &operator=(hsRef<_Ref> &&move)
    {
        if (fObj)
            fObj->UnRef();
        fObj = move.fObj;
        move.fObj = nullptr;
        return *this;
    }

    hsRef<_Ref> &operator=(std::nullptr_t)
    {
        if (fObj)
            fObj->UnRef();
        fObj = nullptr;
        return *this;
    }

    inline hsRef(const hsWeakRef<_Ref> &weak);
    inline hsRef<_Ref> &operator=(const hsWeakRef<_Ref> &weak);

    bool operator==(const hsRef<_Ref> &other) const { return fObj == other.fObj; }
    bool operator!=(const hsRef<_Ref> &other) const { return fObj != other.fObj; }
    bool operator> (const hsRef<_Ref> &other) const { return fObj >  other.fObj; }
    bool operator< (const hsRef<_Ref> &other) const { return fObj <  other.fObj; }
    bool operator>=(const hsRef<_Ref> &other) const { return fObj >= other.fObj; }
    bool operator<=(const hsRef<_Ref> &other) const { return fObj <= other.fObj; }
    inline bool operator==(const hsWeakRef<_Ref> &other) const;
    inline bool operator!=(const hsWeakRef<_Ref> &other) const;
    bool operator==(_Ref *other) const { return fObj == other; }
    bool operator!=(_Ref *other) const { return fObj != other; }

    _Ref &operator*() const { return *fObj; }
    _Ref *operator->() const { return fObj; }
    _Ref *Get() const { return fObj; }

    operator bool() const { return fObj != nullptr; }
    bool operator!() const { return fObj == nullptr; }

    void Steal(_Ref *obj)
    {
        if (fObj)
            fObj->UnRef();
        fObj = obj;
    }

private:
    _Ref *fObj;
};

template <class _Ref>
class hsWeakRef
{
public:
    hsWeakRef() : fObj() { }
    hsWeakRef(std::nullptr_t) : fObj() { }
    hsWeakRef(_Ref *obj) : fObj(obj) { }

    template <class _OtherRef>
    hsWeakRef(const hsWeakRef<_OtherRef> &copy) : fObj(copy.Get()) { }

    template <class _OtherRef>
    hsWeakRef(const hsRef<_OtherRef> &copy) : fObj(copy.Get()) { }

    hsWeakRef<_Ref> &operator=(_Ref *obj)
    {
        fObj = obj;
        return *this;
    }

    template <class _OtherRef>
    hsWeakRef<_Ref> &operator=(const hsWeakRef<_OtherRef> &obj)
    {
        fObj = obj.Get();
        return *this;
    }

    template <class _OtherRef>
    hsWeakRef<_Ref> &operator=(const hsRef<_OtherRef> &obj)
    {
        fObj = obj.Get();
        return *this;
    }

    bool operator==(const hsWeakRef<_Ref> &other) const { return fObj == other.fObj; }
    bool operator!=(const hsWeakRef<_Ref> &other) const { return fObj != other.fObj; }
    bool operator==(const hsRef<_Ref> &other) const { return fObj == other.Get(); }
    bool operator!=(const hsRef<_Ref> &other) const { return fObj != other.Get(); }
    bool operator==(_Ref *other) const { return fObj == other; }
    bool operator!=(_Ref *other) const { return fObj != other; }

    _Ref &operator*() const { return *fObj; }
    _Ref *operator->() const { return fObj; }
    _Ref *Get() const { return fObj; }

    operator bool() const { return fObj != nullptr; }
    bool operator!() const { return fObj == nullptr; }

private:
    _Ref *fObj;
};

template <class _Ref>
hsRef<_Ref>::hsRef(const hsWeakRef<_Ref> &weak) : fObj(weak.Get())
{
    if (fObj)
        fObj->Ref();
}

template <class _Ref>
hsRef<_Ref> &hsRef<_Ref>::operator=(const hsWeakRef<_Ref> &weak)
{
    _Ref* weakObj = weak.Get();
    if (weakObj)
        weakObj->Ref();
    if (fObj)
        fObj->UnRef();
    fObj = weakObj;
    return *this;
}

template <class _Ref>
bool hsRef<_Ref>::operator==(const hsWeakRef<_Ref> &other) const
{
    return fObj == other.Get();
}

template <class _Ref>
bool hsRef<_Ref>::operator!=(const hsWeakRef<_Ref> &other) const
{
    return fObj != other.Get();
}

#endif
