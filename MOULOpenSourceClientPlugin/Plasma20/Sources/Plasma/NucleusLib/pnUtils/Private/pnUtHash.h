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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtHash.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTHASH_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtHash.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTHASH_H


/****************************************************************************
*
*   Macros
*
***/

// Define a field inside an object that is used to link it into a hash table
#define HASHLINK(object) THashLink< object >

// Define a POINTER to a hash table, not a hash table
#define HASHTABLE(object,key) THashTable< object, key >

// Define a hash table:
// - starts with kSlotMinCount rows
// - can grow to kDefaultSlotMaxCount rows
// (hash table grows when a row contains more than kGrowOnListSize entries
#define HASHTABLEDECL(object,key,link) THashTableDecl< object, key, offsetof(object,link), 0 >

// Define a hash table in situations when a forward reference prevents use of HASHTABLEDECL
// - Size characteristics are identical to HASHTABLEDECL
#define HASHTABLEDYN(object,key) THashTableDyn< object, key >

// Define a hash table with:
// - starts with <size>
// - row table never grows
#define HASHTABLEDECLSIZE(object,key,link,size) THashTableDecl<object, key, offsetof(object,link), size >


/****************************************************************************
*
*   Forward declarations
*
***/

template<class T>
class THashLink;

template<class T>
class TBaseHashTable;


/****************************************************************************
*
*   THashLink
*
***/

template<class T>
class THashLink {
    friend class TBaseHashTable<T>;

private:
    unsigned m_hash;
    LINK(T)  m_linkToFull;
    LINK(T)  m_linkToSlot;

public:
    inline bool IsLinked () const;
    inline T * Next ();
    inline const T * Next () const;
    inline T * Prev ();
    inline const T * Prev () const;
    inline void Unlink ();
};

//===========================================================================
template<class T>
bool THashLink<T>::IsLinked () const {
    return m_linkToFull.IsLinked();
}

//===========================================================================
template<class T>
T * THashLink<T>::Next () {
    return m_linkToFull.Next();
}

//===========================================================================
template<class T>
const T * THashLink<T>::Next () const {
    return m_linkToFull.Next();
}

//===========================================================================
template<class T>
T * THashLink<T>::Prev () {
    return m_linkToFull.Prev();
}

//===========================================================================
template<class T>
const T * THashLink<T>::Prev () const {
    return m_linkToFull.Prev();
}

//===========================================================================
template<class T>
void THashLink<T>::Unlink () {
    m_linkToFull.Unlink();
    m_linkToSlot.Unlink();
}


/****************************************************************************
*
*   TBaseHashTable
*
***/

template<class T>
class TBaseHashTable {

private:
    enum { kSlotMinCount = 8 };
    enum { kDefaultSlotMaxCount = 1024 };
    enum { kGrowOnListSize = 5 };

    LIST(T)            m_fullList;
    int                m_linkOffset;
    FARRAYOBJ(LIST(T)) m_slotListArray;
    unsigned           m_slotMask;  // always set to a power of two minus one
    unsigned           m_slotMaxCount;

    inline bool CheckGrowTable (LIST(T) * slotList);
    inline const THashLink<T> & GetLink (const T * object) const;
    inline THashLink<T> & GetLink (T * object);
    inline void SetSlotCount (unsigned count);

protected:
    inline unsigned GetHash (const T * object) const;
    inline unsigned & GetHash (T * object);
    inline const LIST(T) & GetSlotList (unsigned hash) const;
    inline LIST(T) & GetSlotList (unsigned hash);
    inline void SetLinkOffset (int linkOffset, unsigned maxSize);
    inline void SetSlotMaxCount (unsigned count);

public:
    inline TBaseHashTable ();
    inline TBaseHashTable (const TBaseHashTable<T> & source);
    inline TBaseHashTable<T> & operator= (const TBaseHashTable<T> & source);

    inline void Add (T * object, unsigned hash);
    inline void Clear ();
    inline void Delete (T * object);
    inline T * Head ();
    inline const T * Head () const;
    inline T * Next (const T * object);
    inline const T * Next (const T * object) const;
    inline void Order (T * linkedObject, ELinkType linkType, T * existingObject);
    inline T * Prev (const T * object);
    inline const T * Prev (const T * object) const;
    inline T * Tail ();
    inline const T * Tail () const;
    inline void Unlink (T * object);

};

//===========================================================================
template<class T>
TBaseHashTable<T>::TBaseHashTable () {
    m_slotMask     = 0;
    m_slotMaxCount = kDefaultSlotMaxCount;
    // more initialization done during call to SetLinkOffset()
}

//===========================================================================
template<class T>
TBaseHashTable<T>::TBaseHashTable (const TBaseHashTable<T> & source) {
    ref(source);
#ifdef HS_DEBUGGING
    FATAL("No copy constructor");
#endif
    TBaseHashTable();
}

//===========================================================================
template<class T>
TBaseHashTable<T> & TBaseHashTable<T>::operator= (const TBaseHashTable<T> & source) {
    ref(source);
#ifdef HS_DEBUGGING
    FATAL("No assignment operator");
#endif
    return *this;
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::Add (T * object, unsigned hash) {
    GetHash(object) = hash;

    LIST(T) * list = &GetSlotList(hash);
    if (CheckGrowTable(list))
        list = &GetSlotList(hash);

    m_fullList.Link(object);
    list->Link(object);
}

//===========================================================================
template<class T>
bool TBaseHashTable<T>::CheckGrowTable (LIST(T) * list) {

    unsigned nextCount = (m_slotMask + 1) * 2;
    if (nextCount > m_slotMaxCount)
        return false;

    unsigned listCount = 0;
    for (T * curr = list->Head(); curr; curr = list->Next(curr))
        ++listCount;

    if (listCount + 1 < kGrowOnListSize)
        return false;

    SetSlotCount(nextCount);

    return true;
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::Clear () {
    m_fullList.Clear();
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::Delete (T * object) {
    DEL(object);
}

//===========================================================================
template<class T>
unsigned TBaseHashTable<T>::GetHash (const T * object) const {
    return GetLink(object).m_hash;
}

//===========================================================================
template<class T>
unsigned & TBaseHashTable<T>::GetHash (T * object) {
    return GetLink(object).m_hash;
}

//===========================================================================
template<class T>
const THashLink<T> & TBaseHashTable<T>::GetLink (const T * object) const {
    return *(const THashLink<T> *)((const byte *)object + m_linkOffset);
}

//===========================================================================
template<class T>
THashLink<T> & TBaseHashTable<T>::GetLink (T * object) {
    return *(THashLink<T> *)((byte *)object + m_linkOffset);
}

//===========================================================================
template<class T>
const LIST(T) & TBaseHashTable<T>::GetSlotList (unsigned hash) const {
    return m_slotListArray[hash & m_slotMask];
}

//===========================================================================
template<class T>
LIST(T) & TBaseHashTable<T>::GetSlotList (unsigned hash) {
    return m_slotListArray[hash & m_slotMask];
}

//===========================================================================
template<class T>
T * TBaseHashTable<T>::Head () {
    return m_fullList.Head();
}

//===========================================================================
template<class T>
const T * TBaseHashTable<T>::Head () const {
    return m_fullList.Head();
}

//===========================================================================
template<class T>
T * TBaseHashTable<T>::Next (const T * object) {
    return m_fullList.Next(object);
}

//===========================================================================
template<class T>
const T * TBaseHashTable<T>::Next (const T * object) const {
    return m_fullList.Next(object);
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::Order (T * linkedObject, ELinkType linkType, T * existingObject) {
    THashLink<T> & link = GetLink(linkedObject);
    ref(link);
    ASSERT(link.m_linkToFull.IsLinked());
    m_fullList.Link(linkedObject, linkType, existingObject);
}

//===========================================================================
template<class T>
T * TBaseHashTable<T>::Prev (const T * object) {
    return m_fullList.Prev(object);
}

//===========================================================================
template<class T>
const T * TBaseHashTable<T>::Prev (const T * object) const {
    return m_fullList.Prev(object);
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::SetLinkOffset (int linkOffset, unsigned maxSize) {
    ASSERT(!m_fullList.Head());
    ASSERT(!m_slotListArray.Count());
    ASSERT(!m_slotMask);

    m_linkOffset = linkOffset;
    m_fullList.SetLinkOffset(m_linkOffset + offsetof(THashLink<T>, m_linkToFull));

    if (!m_slotMask)
        SetSlotCount(max(kSlotMinCount, MathNextPow2(maxSize)));
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::SetSlotCount (unsigned count) {
    ASSERT(!(count & (count - 1)));  // power of two
    ASSERT(count >= 2);
    
    if (count == m_slotMask + 1)
        return;
    m_slotMask = count - 1;

    m_slotListArray.ZeroCount();
    m_slotListArray.SetCount(count);
    for (unsigned loop = 0; loop < count; ++loop)
        m_slotListArray[loop].SetLinkOffset(m_linkOffset + offsetof(THashLink<T>, m_linkToSlot));

    for (T * curr = Head(); curr; curr = Next(curr))
        GetSlotList(GetHash(curr)).Link(curr);           
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::SetSlotMaxCount (unsigned count) {
    if (count)
        m_slotMaxCount = max(kSlotMinCount, count);
}

//===========================================================================
template<class T>
T * TBaseHashTable<T>::Tail () {
    return m_fullList.Tail();
}

//===========================================================================
template<class T>
const T * TBaseHashTable<T>::Tail () const {
    return m_fullList.Tail();
}

//===========================================================================
template<class T>
void TBaseHashTable<T>::Unlink (T * object) {
    THashLink<T> & link = GetLink(object);
    link.Unlink();
}


/****************************************************************************
*
*   THashTable
*
***/

template<class T, class K>
class THashTable : public TBaseHashTable<T> {

public:
    inline void Add (T * object);
    inline void Add (T * object, unsigned hash);
    inline T * Find (const K & key);
    inline T * FindNext (const K & key, T * object);
    inline const T * Find (const K & key) const;
    inline const T * FindNext (const K & key, const T * object) const;
    inline T * Unduplicate (T * object, const K & key);
    
};

//===========================================================================
template<class T, class K>
inline void THashTable<T,K>::Add (T * object) {
    TBaseHashTable<T>::Add(object, object->GetHash());
}

//===========================================================================
template<class T, class K>
inline void THashTable<T,K>::Add (T * object, unsigned hash) {
    TBaseHashTable<T>::Add(object, hash);
}

//===========================================================================
template<class T, class K>
T * THashTable<T,K>::Find (const K & key) {
    return (T *)((const THashTable<T,K> *)this)->Find(key);
}

//===========================================================================
template<class T, class K>
T * THashTable<T,K>::FindNext (const K & key, T * object) {
    return (T *)((const THashTable<T,K> *)this)->FindNext(key, object);
}

//===========================================================================
template<class T, class K>
const T * THashTable<T,K>::Find (const K & key) const {
    unsigned        hash     = key.GetHash();
    const LIST(T) & slotList = GetSlotList(hash);
    for (const T * curr = slotList.Head(); curr; curr = slotList.Next(curr))
        if ((GetHash(curr) == hash) && (*curr == key))
            return curr;
    return nil;
}

//===========================================================================
template<class T, class K>
const T * THashTable<T,K>::FindNext (const K & key, const T * object) const {
    unsigned        hash     = key.GetHash();
    const LIST(T) & slotList = GetSlotList(hash);
    for (const T * curr = slotList.Next(object); curr; curr = slotList.Next(curr))
        if ((GetHash(curr) == hash) && (*curr == key))
            return curr;
    return nil;
}

//===========================================================================
template<class T, class K>
T * THashTable<T,K>::Unduplicate (T * object, const K & key) {
    T * existing = Find(key);
    if (existing) {
        DEL(object);
        return existing;
    }
    else {
        Add(object);
        return object;
    }
}


/****************************************************************************
*
*   THashTableDecl
*
***/

template<class T, class K, int linkOffset, unsigned maxSize>
class THashTableDecl : public THashTable<T,K> {

public:
    inline THashTableDecl ();

};

//===========================================================================
template<class T, class K, int linkOffset, unsigned maxSize>
THashTableDecl<T,K,linkOffset,maxSize>::THashTableDecl () {
    SetLinkOffset(linkOffset, maxSize);
    SetSlotMaxCount(maxSize);
}


/****************************************************************************
*
*   THashTableDyn
*
***/

template<class T, class K>
class THashTableDyn : public THashTable<T,K> {

public:
    void Initialize (int linkOffset, unsigned maxSize = 0);

};

//===========================================================================
template<class T, class K>
void THashTableDyn<T,K>::Initialize (int linkOffset, unsigned maxSize) {
    SetLinkOffset(linkOffset, maxSize);
    SetSlotMaxCount(maxSize);
}


/****************************************************************************
*
*   THashKeyVal
*
***/

template <class T>
class THashKeyVal {
public:
    THashKeyVal () : m_value(0) { }
    THashKeyVal (const T & value) : m_value(value) { }
    bool operator== (const THashKeyVal & rhs) const {
        return m_value == rhs.m_value;
    }
    unsigned GetHash () const {
        CHashValue hash(&m_value, sizeof(m_value));
        return hash.GetHash();
    }
    const T & GetValue () const { return m_value; }
    void SetValue (const T & value) { m_value = value; }

protected:
    T m_value;
};


/****************************************************************************
*
*   CHashKeyStrPtr / CHashKeyStrPtrI
*
***/

//===========================================================================
template<class C>
class THashKeyStrBase {
public:
    const C * GetString () const {
        return m_str;
    }

protected:
    THashKeyStrBase () : m_str(nil) { }
    THashKeyStrBase (const C str[]) : m_str(str) { }
    virtual ~THashKeyStrBase () { }

    const C * m_str;
};

//===========================================================================
template<class C>
class THashKeyStrCmp : public THashKeyStrBase<C> {
public:
    bool operator== (const THashKeyStrCmp & rhs) const {
        return StrCmp(m_str, rhs.m_str) == 0;
    }
    unsigned GetHash () const {
        return StrHash(m_str);
    }

protected:
    THashKeyStrCmp () { }
    THashKeyStrCmp (const C str[]) : THashKeyStrBase<C>(str) { }
};

//===========================================================================
template<class C>
class THashKeyStrCmpI : public THashKeyStrBase<C> {
public:
    bool operator== (const THashKeyStrCmpI & rhs) const {
        return StrCmpI(m_str, rhs.m_str) == 0;
    }
    unsigned GetHash () const {
        return StrHashI(m_str);
    }
protected:

    THashKeyStrCmpI () { }
    THashKeyStrCmpI (const C str[]) : THashKeyStrBase<C>(str) { }
};


/****************************************************************************
*
*   THashKeyStrPtr
*
***/

template <class C, class T>
class THashKeyStrPtr : public T {
public:
    THashKeyStrPtr () { }
    THashKeyStrPtr (const C str[]) : T(str) { }
    void SetString (const C str[]) {
        m_str = str;
    }
};

typedef THashKeyStrPtr< wchar, THashKeyStrCmp<wchar> >  CHashKeyStrPtr;
typedef THashKeyStrPtr< wchar, THashKeyStrCmpI<wchar> > CHashKeyStrPtrI;
typedef THashKeyStrPtr< char, THashKeyStrCmp<char> >    CHashKeyStrPtrChar;
typedef THashKeyStrPtr< char, THashKeyStrCmpI<char> >   CHashKeyStrPtrCharI;


/****************************************************************************
*
*   THashKeyStr
*
***/

template <class C, class T>
class THashKeyStr : public T {
public:
    THashKeyStr () { }
    THashKeyStr (const C str[]) { SetString(str); }
    THashKeyStr (const THashKeyStr &);              // intentionally unimplemented
    THashKeyStr & operator= (const THashKeyStr &);  // intentionally unimplemented
    ~THashKeyStr () {
        SetString(nil);
    }
    void SetString (const C str[]) {  // deprecated
        if (m_str)
            FREE(const_cast<C *>(m_str));
        if (str)
            m_str = StrDup(str);
        else
            m_str = nil;
    }
};

typedef THashKeyStr< wchar, THashKeyStrCmp<wchar> >  CHashKeyStr;
typedef THashKeyStr< wchar, THashKeyStrCmpI<wchar> > CHashKeyStrI;
typedef THashKeyStr< char, THashKeyStrCmp<char> >    CHashKeyStrChar;
typedef THashKeyStr< char, THashKeyStrCmpI<char> >   CHashKeyStrCharI;


/****************************************************************************
*
*   CHashValue
*
***/

class CHashValue {
private:
    static const dword s_hashTable[];

    dword m_result;

    inline void Construct () { m_result = 0x325d1eae; }

public:
    static dword LookupHashBits (unsigned value) { ASSERT(value < 0x100); return s_hashTable[value]; }

    inline CHashValue () { Construct() ; }
    inline CHashValue (const CHashValue & source) { m_result = source.m_result; }
    inline CHashValue (const void * data, unsigned bytes) { Construct(); Hash(data, bytes); }
    inline CHashValue & operator= (const CHashValue & source) { m_result = source.m_result; return *this; }
    inline bool operator== (const CHashValue & source) const { return (m_result == source.m_result); }

    inline dword GetHash () const { return m_result; }

    __forceinline void Hash   (const void * data, unsigned bytes);
    __forceinline void Hash8  (unsigned data);
    __forceinline void Hash16 (unsigned data);
    __forceinline void Hash32 (unsigned data);

};

//===========================================================================
void CHashValue::Hash (const void * data, unsigned bytes) {
    for (const byte * curr = (const byte *)data, * term = curr + bytes; curr != term; ++curr)
        Hash8(*curr);
}

//===========================================================================
void CHashValue::Hash8 (unsigned data) {
    m_result += s_hashTable[m_result >> 24] ^ (m_result >> 6) ^ s_hashTable[data & 0xff];
}

//===========================================================================
void CHashValue::Hash16 (unsigned data) {
    Hash8(data);
    Hash8(data >> 8);
}

//===========================================================================
void CHashValue::Hash32 (unsigned data) {
    Hash8(data);
    Hash8(data >> 8);
    Hash8(data >> 16);
    Hash8(data >> 24);
}
