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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSpareList.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSPARELIST_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSpareList.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSPARELIST_H



#ifdef HS_DEBUGGING
#define SPARELIST_TRACK_MEMORY
#endif


/****************************************************************************
*
*   CBaseSpareList
*
***/

class CBaseSpareList {
public:
    CBaseSpareList ();

protected:
    struct SpareNode {
        SpareNode * spareNext;
    };
    SpareNode * m_spareHead;

    void * Alloc (unsigned objectSize, const char typeName[]);
    void CleanUp (const char typeName[]);
    void Free (void * object, unsigned objectSize);

private:
    union AllocNode {
        AllocNode * allocNext;
        qword       align;
    };
    AllocNode * m_allocHead;
    unsigned    m_chunkSize;

    #ifdef SPARELIST_TRACK_MEMORY
    unsigned m_unfreedObjects;
    #endif

    void GrowSpareList (unsigned objectSize, const char typeName[]);
};


/****************************************************************************
*
*   TSpareList
*
***/

template<class T>
class TSpareList : public CBaseSpareList {
private:
    enum { OBJECT_SIZE = MAX(sizeof(T), sizeof(SpareNode)) };

public:
    ~TSpareList () { CleanUp(); }

    void * Alloc ();
    void   CleanUp ();
    void   Delete (T * node);
    void   Free (T * node);
    T *    New ();
};


//===========================================================================
template<class T>
void * TSpareList<T>::Alloc () {
    return CBaseSpareList::Alloc(OBJECT_SIZE, typeid(T).name());
}

//===========================================================================
template<class T>
void TSpareList<T>::CleanUp () {
    CBaseSpareList::CleanUp(typeid(T).name());
}

//===========================================================================
template<class T>
void TSpareList<T>::Delete (T * node) {
    node->~T();
    CBaseSpareList::Free(node, OBJECT_SIZE);
}

//===========================================================================
template<class T>
void TSpareList<T>::Free (T * node) {
    CBaseSpareList::Free(node, OBJECT_SIZE);
}

//===========================================================================
template<class T>
T * TSpareList<T>::New () {
    return new(CBaseSpareList::Alloc(OBJECT_SIZE, typeid(T).name())) T;
}
