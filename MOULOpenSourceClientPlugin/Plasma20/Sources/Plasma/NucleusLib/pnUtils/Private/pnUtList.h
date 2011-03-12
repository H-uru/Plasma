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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtList.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTLIST_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtList.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTLIST_H


/****************************************************************************
*
*   Constants
*
***/

enum ELinkType {
    kListUnlinked,
    kListLinkAfter,
    kListLinkBefore,
    kListHead = kListLinkAfter,
    kListTail = kListLinkBefore
};


/****************************************************************************
*
*   Macros
*
***/

#define  LINK(class)            TLink< class >
#define  LIST(class)            TList< class >
#define  LISTDECL(class,field)  TListDecl< class, offsetof(class,field) >
#define  LISTDYN(class)         TListDyn< class >


/****************************************************************************
*
*   Forward declarations
*
***/

template<class T>
class TLink;

template<class T>
class TList;


/****************************************************************************
*
*   CBaseLink
*
***/

class CBaseLink {
    friend class CBaseList;

    inline static bool   TermCheck (byte * ptr);
    inline static byte * TermMark (byte * ptr);
    inline static byte * TermUnmarkAlways (byte * ptr);

protected:
    CBaseLink * volatile m_prevLink;
    byte * volatile      m_next;

    inline int  CalcLinkOffset () const;
    inline void InitializeLinks ();
    inline void InitializeLinksWithOffset (int linkOffset);
    inline void InsertAfter (byte * node, CBaseLink * prevLink, int linkOffset);
    inline void InsertBefore (byte * node, CBaseLink * nextLink);
    inline byte * Next () const;
    inline byte * NextIgnoreTerm () const;
    inline byte * NextUnchecked () const;
    inline CBaseLink * NextLink () const;
    inline CBaseLink * NextLink (int linkOffset) const;
    inline byte * Prev () const;
    inline void UnlinkFromNeighbors ();

public:
    inline CBaseLink ();
    inline CBaseLink (const CBaseLink & source);
    inline ~CBaseLink ();
    inline CBaseLink & operator= (const CBaseLink & source);
    inline bool IsLinked () const;
    inline void Unlink ();

};

//===========================================================================
CBaseLink::CBaseLink () {
    InitializeLinks();
}

//===========================================================================
CBaseLink::CBaseLink (const CBaseLink & source) {
    ref(source);
#ifdef HS_DEBUGGING
    if (source.IsLinked())
        FATAL("No copy constructor");
#endif
    InitializeLinks();
}

//===========================================================================
CBaseLink::~CBaseLink () {
    UnlinkFromNeighbors();
}

//===========================================================================
CBaseLink & CBaseLink::operator= (const CBaseLink & source) {
    ref(source);
#ifdef HS_DEBUGGING
    FATAL("No assignment operator");
#endif
    return *this;
}

//===========================================================================
int CBaseLink::CalcLinkOffset () const {
    return (int)((byte *)this - m_prevLink->NextIgnoreTerm());
}

//===========================================================================
void CBaseLink::InitializeLinks () {
    ASSERT(!((unsigned_ptr)this & 3));
    m_prevLink = this;
    m_next     = TermMark((byte *)this);
}

//===========================================================================
void CBaseLink::InitializeLinksWithOffset (int linkOffset) {
    m_prevLink = this;
    m_next     = TermMark((byte *)this - linkOffset);
}

//===========================================================================
void CBaseLink::InsertAfter (byte * node, CBaseLink * prevLink, int linkOffset) {
    UnlinkFromNeighbors();
    m_prevLink = prevLink;
    m_next     = prevLink->m_next;
    prevLink->NextLink(linkOffset)->m_prevLink = this;
    prevLink->m_next                           = node;
}

//===========================================================================
void CBaseLink::InsertBefore (byte * node, CBaseLink * nextLink) {
    UnlinkFromNeighbors();
    m_prevLink = nextLink->m_prevLink;
    m_next     = m_prevLink->m_next;
    nextLink->m_prevLink->m_next = node;
    nextLink->m_prevLink         = this;
}

//===========================================================================
bool CBaseLink::IsLinked () const {
    return (m_prevLink != this);
}

//===========================================================================
byte * CBaseLink::Next () const {
    return TermCheck(m_next) ? nil : m_next;
}

//===========================================================================
byte * CBaseLink::NextIgnoreTerm () const {
    return TermUnmarkAlways(m_next);
}

//===========================================================================
byte * CBaseLink::NextUnchecked () const {
    return m_next;
}

//===========================================================================
CBaseLink * CBaseLink::NextLink () const {
    return (CBaseLink *)(NextIgnoreTerm() + CalcLinkOffset());
}

//===========================================================================
CBaseLink * CBaseLink::NextLink (int linkOffset) const {
    ASSERT(linkOffset == CalcLinkOffset());
    return (CBaseLink *)(NextIgnoreTerm() + linkOffset);
}

//===========================================================================
byte * CBaseLink::Prev () const {
    return m_prevLink->m_prevLink->Next();
}

//===========================================================================
bool CBaseLink::TermCheck (byte * ptr) {
    return (unsigned_ptr)ptr & 1;
}

//===========================================================================
byte * CBaseLink::TermMark (byte * ptr) {
    // Converts an unmarked pointer to a marked pointer
    ASSERT(!TermCheck(ptr));
    return (byte *)((unsigned_ptr)ptr + 1);
}

//===========================================================================
byte * CBaseLink::TermUnmarkAlways (byte * ptr) {
    // Returns an unmarked pointer regardless of whether the source pointer
    // was marked on unmarked
    return (byte *)((unsigned_ptr)ptr & ~1);
}

//===========================================================================
void CBaseLink::Unlink () {
    UnlinkFromNeighbors();
    InitializeLinks();
}

//===========================================================================
void CBaseLink::UnlinkFromNeighbors () {
    NextLink()->m_prevLink = m_prevLink;
    m_prevLink->m_next     = m_next;
}


/****************************************************************************
*
*   TLink
*
***/

template<class T>
class TLink : public CBaseLink {

public:
    inline T * Next ();
    inline const T * Next () const;
    inline T * NextUnchecked ();
    inline const T * NextUnchecked () const;
    inline T * Prev ();
    inline const T * Prev () const;

};

//===========================================================================
template<class T>
T * TLink<T>::Next () {
    return (T *)CBaseLink::Next();
}

//===========================================================================
template<class T>
const T * TLink<T>::Next () const {
    return (const T *)CBaseLink::Next();
}

//===========================================================================
template<class T>
T * TLink<T>::NextUnchecked () {
    return (T *)CBaseLink::NextUnchecked();
}

//===========================================================================
template<class T>
const T * TLink<T>::NextUnchecked () const {
    return (const T *)CBaseLink::NextUnchecked();
}

//===========================================================================
template<class T>
T * TLink<T>::Prev () {
    return (T *)CBaseLink::Prev();
}

//===========================================================================
template<class T>
const T * TLink<T>::Prev () const {
    return (const T *)CBaseLink::Prev();
}


/****************************************************************************
*
*   CBaseList
*
***/

class CBaseList {

private:
    enum { LINK_OFFSET_UNINIT = 0xDDDDDDDD };

    int       m_linkOffset;
    CBaseLink m_terminator;

protected:
    inline CBaseLink * GetLink (const byte * node) const;
    inline byte * Head () const;
    inline bool IsLinked (const byte * node) const;
    inline void Link (byte * node, ELinkType linkType, byte * existingNode);
    void Link (CBaseList * list, byte * afterNode, byte * beforeNode, ELinkType linkType, byte * existingNode);
    inline byte * Next (const byte * node) const;
    inline byte * NextUnchecked (const byte * node) const;
    inline byte * Prev (byte * node) const;
    inline byte * Tail () const;
    inline void   Unlink (byte * node);

public:
    inline CBaseList ();
    inline CBaseList (const CBaseList & source);
    inline ~CBaseList ();
    inline CBaseList & operator= (const CBaseList & source);
    inline void SetLinkOffset (int linkOffset);
    void UnlinkAll ();

};

//===========================================================================
CBaseList::CBaseList () {
    m_linkOffset = LINK_OFFSET_UNINIT;
}

//===========================================================================
CBaseList::CBaseList (const CBaseList & source) {
    m_linkOffset = LINK_OFFSET_UNINIT;   
    ref(source);
}

//===========================================================================
CBaseList::~CBaseList () {
    if (m_terminator.IsLinked())
        UnlinkAll();
}

//===========================================================================
CBaseList & CBaseList::operator= (const CBaseList & source) {
    ref(source);
    return *this;
}

//===========================================================================
CBaseLink * CBaseList::GetLink (const byte * node) const {
    return (CBaseLink *)(node + m_linkOffset);
}

//===========================================================================
byte * CBaseList::Head () const {
    return m_terminator.Next();
}

//===========================================================================
bool CBaseList::IsLinked (const byte * node) const {
    ASSERT(node);
    return GetLink(node)->IsLinked();
}

//===========================================================================
void CBaseList::Link (byte * node, ELinkType linkType, byte * existingNode) {
    ASSERT(node != existingNode);
    ASSERT(m_linkOffset != LINK_OFFSET_UNINIT);
    ASSERT((linkType == kListLinkAfter) || (linkType == kListLinkBefore));
    if (linkType == kListLinkAfter)
        GetLink(node)->InsertAfter(node, existingNode ? GetLink(existingNode) : &m_terminator, m_linkOffset);
    else
        GetLink(node)->InsertBefore(node, existingNode ? GetLink(existingNode) : &m_terminator);
}

//===========================================================================
byte * CBaseList::Next (const byte * node) const {
    ASSERT(node);
    return GetLink(node)->Next();
}

//===========================================================================
byte * CBaseList::NextUnchecked (const byte * node) const {
    ASSERT(node);
    return GetLink(node)->NextUnchecked();
}

//===========================================================================
byte * CBaseList::Prev (byte * node) const {
    ASSERT(node);
    return GetLink(node)->Prev();
}

//===========================================================================
void CBaseList::SetLinkOffset (int linkOffset) {
    ASSERT(!Head());

    m_linkOffset = linkOffset;
    m_terminator.InitializeLinksWithOffset(linkOffset);
}

//===========================================================================
byte * CBaseList::Tail () const {
    return m_terminator.Prev();
}

//===========================================================================
void CBaseList::Unlink (byte * node) {
    ASSERT(node);
    GetLink(node)->Unlink();
}


/****************************************************************************
*
*   TList
*
***/

template<class T>
class TList : public CBaseList {

private:
    inline T * NewFlags (unsigned flags, ELinkType linkType, T * existingNode, const char file[], int line);

public:
    inline void Clear ();
    inline void Delete (T * node);
    inline T * Head ();
    inline const T * Head () const;
    inline bool IsLinked (const T * node) const;
    inline void Link (T * node, ELinkType linkType = kListTail, T * existingNode = nil);
    inline void Link (TList<T> * list, ELinkType linkType = kListTail, T * existingNode = nil);
    inline void Link (TList<T> * list, T * afterNode, T * beforeNode, ELinkType linkType = kListTail, T * existingNode = nil);
    inline T * New (ELinkType linkType = kListTail, T * existingNode = nil, const char file[] = nil, int line = 0);
    inline T * NewZero (ELinkType linkType = kListTail, T * existingNode = nil, const char file[] = nil, int line = 0);
    inline T * Next (const T * node);
    inline const T * Next (const T * node) const;
    inline T * NextUnchecked (const T * node);
    inline const T * NextUnchecked (const T * node) const;
    inline T * Prev (const T * node);
    inline const T * Prev (const T * node) const;
    inline T * Tail ();
    inline const T * Tail () const;
    inline void Unlink (T * node);

};

//===========================================================================
template<class T>
void TList<T>::Clear () {
    for (T * curr; (curr = Head()) != nil; Delete(curr))
        ;
}

//===========================================================================
template<class T>
void TList<T>::Delete (T * node) {
    DEL(node);
}

//===========================================================================
template<class T>
T * TList<T>::Head () {
    return (T *)CBaseList::Head();
}

//===========================================================================
template<class T>
const T * TList<T>::Head () const {
    return (const T *)CBaseList::Head();
}

//===========================================================================
template<class T>
bool TList<T>::IsLinked (const T * node) const {
    return CBaseList::IsLinked((const byte *)node);
}

//===========================================================================
template<class T>
void TList<T>::Link (T * node, ELinkType linkType, T * existingNode) {
    CBaseList::Link((byte *)node, linkType, (byte *)existingNode);
}

//===========================================================================
template<class T>
void TList<T>::Link (TList<T> * list, ELinkType linkType, T * existingNode) {
    CBaseList::Link(list, nil, nil, linkType, (byte *)existingNode);
}

//===========================================================================
template<class T>
void TList<T>::Link (TList<T> * list, T * afterNode, T * beforeNode, ELinkType linkType, T * existingNode) {
    CBaseList::Link(list, (byte *)afterNode, (byte *)beforeNode, linkType, (byte *)existingNode);
}

//===========================================================================
template<class T>
inline T * TList<T>::Next (const T * node) {
    return (T *)CBaseList::Next((byte *)node);
}

//===========================================================================
template<class T>
inline const T * TList<T>::Next (const T * node) const {
    return (const T *)CBaseList::Next((byte *)node);
}

//===========================================================================
template<class T>
inline T * TList<T>::NextUnchecked (const T * node) {
    return (T *)CBaseList::NextUnchecked((byte *)node);
}

//===========================================================================
template<class T>
inline const T * TList<T>::NextUnchecked (const T * node) const {
    return (const T *)CBaseList::NextUnchecked((byte *)node);
}

//===========================================================================
template<class T>
inline T * TList<T>::New (ELinkType linkType, T * existingNode, const char file[], int line) {
    return NewFlags(0, linkType, existingNode, file, line);
}

//===========================================================================
template<class T>
inline T * TList<T>::NewFlags (unsigned flags, ELinkType linkType, T * existingNode, const char file[], int line) {
    if (!file) {
        file = __FILE__;
        line = __LINE__;
    }
    T * node = new(MemAlloc(sizeof(T), flags, file, line)) T;
    if (linkType != kListUnlinked)
        Link(node, linkType, existingNode);
    return node;
}

//===========================================================================
template<class T>
inline T * TList<T>::NewZero (ELinkType linkType, T * existingNode, const char file[], int line) {
    return NewFlags(MEM_ZERO, linkType, existingNode, file, line);
}

//===========================================================================
template<class T>
inline T * TList<T>::Prev (const T * node) {
    return (T *)CBaseList::Prev((byte *)node);
}

//===========================================================================
template<class T>
inline const T * TList<T>::Prev (const T * node) const {
    return (const T *)CBaseList::Prev((byte *)node);
}

//===========================================================================
template<class T>
inline T * TList<T>::Tail () {
    return (T *)CBaseList::Tail();
}

//===========================================================================
template<class T>
inline const T * TList<T>::Tail () const {
    return (const T *)CBaseList::Tail();
}

//===========================================================================
template<class T>
inline void TList<T>::Unlink (T * node) {
    CBaseList::Unlink((byte *)node);
}


/****************************************************************************
*
*   TListDecl
*
***/

template<class T, int linkOffset>
class TListDecl : public TList<T> {

public:
    inline TListDecl ();

};

//===========================================================================
template<class T, int linkOffset>
TListDecl<T,linkOffset>::TListDecl () {
    SetLinkOffset(linkOffset);
}


/****************************************************************************
*
*   TListDyn
*
***/

template<class T>
class TListDyn : public TList<T> {

public:
    void Initialize (int linkOffset);

};

//===========================================================================
template<class T>
void TListDyn<T>::Initialize (int linkOffset) {
    SetLinkOffset(linkOffset);
}
