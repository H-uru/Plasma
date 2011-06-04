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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtPriQ.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTPRIQ_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtPriQ.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTPRIQ_H


/****************************************************************************
*
*   Macros
*
***/

#define PRIORITY_TIME(class)            TPriorityTime< class >
#define PRIORITY_NUMERIC(class,type)    TPriorityNumeric< class,type >

#define PRIQ(class,priority)            TPriorityQueue< class,priority >
#define PRIQDECL(class,priority,field)  TPriorityQueueDecl< class,priority,offsetof(class,field) >
#define PRIQDYN(class,priority)         TPriorityQueueDyn< class,priority >


/****************************************************************************
*
*   class TPriorityQueue
*
***/

template<class C, class P>
class TBasePriority;

template<class C, class P>
class TPriorityQueue {

public:
    TPriorityQueue ();
    ~TPriorityQueue ();

    C * const & operator[] (unsigned index) const;

    void Clear ();
    unsigned Count () const;
    C * Delete (C * object);
    C * Dequeue ();
    void Enqueue (C * object);
    C * const * Ptr () const;
    C * Root () const;
    C * const * Term () const;
    void UnlinkAll ();

public:
    // Intentionally unimplemented
    TPriorityQueue (TPriorityQueue const &);
    TPriorityQueue const & operator= (TPriorityQueue const &);

protected:
    void SetLinkOffset (int offset);

private:
    unsigned IndexChild  (unsigned index) const;
    unsigned IndexParent (unsigned index) const;
    void Link (unsigned index);
    P * Priority (C * object);
    P const * Priority (C const * object) const;
    void Remove (unsigned index);
    void Unlink (unsigned index);

    enum { LINK_OFFSET_UNINIT = 0xdddddddd };

    int m_linkOffset;
    ARRAY(C *) m_array;

    friend TBasePriority<C,P>;
};

//===========================================================================
template<class C, class P>
inline C * const & TPriorityQueue<C,P>::operator[] (unsigned index) const {
    return m_array[index];
}

//===========================================================================
template<class C, class P>
inline TPriorityQueue<C,P>::TPriorityQueue () :
    m_linkOffset(LINK_OFFSET_UNINIT) {
}

//===========================================================================
template<class C, class P>
inline TPriorityQueue<C,P>::~TPriorityQueue () {
    UnlinkAll();
}

//===========================================================================
template<class C, class P>
inline void TPriorityQueue<C,P>::Clear () {

    // Deleting an object could cause other objects in the queue to be deleted
    // so we can't make any assumptions about indices or counts of items in the array
    while (C * head = Dequeue())
        DEL(head);

    m_array.Clear();
}

//===========================================================================
template<class C, class P>
inline unsigned TPriorityQueue<C,P>::Count () const {
    return m_array.Count();
}

//===========================================================================
template<class C, class P>
C * TPriorityQueue<C,P>::Delete (C * object) {

    // get the object's priority queue and position
    P * priority = Priority(object);
    const TPriorityQueue<C,P> * queue = priority->GetLink();
    unsigned index = priority->GetIndex();

    // delete the object
    DEL(object);

    // return the next object in that queue
    if (queue && (index < queue->Count()))
        return (*queue)[index];
    else
        return nil;

}

//===========================================================================
template<class C, class P>
C * TPriorityQueue<C,P>::Dequeue () {
    if (!m_array.Count())
        return nil;
    C * value = m_array[0];
    Remove(0);
    return value;
}

//===========================================================================
template<class C, class P>
void TPriorityQueue<C,P>::Enqueue (C * object) {
    P * priority = Priority(object);

    // Verify that the object is not already linked into a priority queue.
    // The original implementation of this function silently refused to
    // enqueue at a new priority if the object was already in this queue.
    // Since this behavior requires callers to check whether the object is
    // already enqueued, we now simply assert that.
    ASSERT(!priority->IsLinked());

    unsigned index  = m_array.Add(object);
    unsigned parent = IndexParent(index);

    // shift value toward root
    while (index && priority->IsPriorityHigher(*Priority(m_array[parent]))) {
        m_array[index] = m_array[parent];
        Link(index);
        index  = parent;
        parent = IndexParent(index);
    }

    // assign and link the new value
    m_array[index] = object;
    Link(index);
}

//===========================================================================
template<class C, class P>
inline unsigned TPriorityQueue<C,P>::IndexChild (unsigned index) const {
    return (index << 1) + 1;
}

//===========================================================================
template<class C, class P>
inline unsigned TPriorityQueue<C,P>::IndexParent (unsigned index) const {
    return (index - 1) >> 1;
}

//===========================================================================
template<class C, class P>
inline void TPriorityQueue<C,P>::Link (unsigned index) {
    Priority(m_array[index])->Link(this, index);
}

//===========================================================================
template<class C, class P>
inline P * TPriorityQueue<C,P>::Priority (C * object) {
    ASSERT(m_linkOffset != LINK_OFFSET_UNINIT);
    return (P *)((byte *)object + m_linkOffset);
}

//===========================================================================
template<class C, class P>
inline P const * TPriorityQueue<C,P>::Priority (C const * object) const {
    ASSERT(m_linkOffset != LINK_OFFSET_UNINIT);
    return (P const *)((byte const *)object + m_linkOffset);
}

//===========================================================================
template<class C, class P>
inline C * const * TPriorityQueue<C,P>::Ptr () const {
    return m_array.Ptr();
}

//===========================================================================
template<class C, class P>
void TPriorityQueue<C,P>::Remove (unsigned index) {

    // reset the priority link fields
    Unlink(index);

    // save the terminal leaf node
    C * value    = m_array.Pop();
    P * priority = Priority(value);

    const unsigned count = m_array.Count();
    if (count == index)
        return;

    // rebalance upwards from the position of the deleted entry
    unsigned parent;
    unsigned entry = index;
    if (entry && priority->IsPriorityHigher(*Priority(m_array[parent = IndexParent(entry)]))) {
        do {
            m_array[entry] = m_array[parent];
            Link(entry);
            entry = parent;
        } while (entry && priority->IsPriorityHigher(*Priority(m_array[parent = IndexParent(entry)])));
        m_array[entry] = value;
        Link(entry);

        entry    = index;
        value    = m_array[index];
        priority = Priority(value);
    }

    // rebalance downwards from the position of the deleted entry
    for (;;) {
        unsigned child = IndexChild(entry);
        if (child >= count)
            break;

        unsigned sibling = child + 1;
        if ( (sibling < count) &&
             (Priority(m_array[sibling])->IsPriorityHigher(*Priority(m_array[child]))) )
            child = sibling;

        if (priority->IsPriorityHigher(*Priority(m_array[child])))
            break;

        m_array[entry] = m_array[child];
        Link(entry);
        entry = child;
    }

    m_array[entry] = value;
    Link(entry);
}

//===========================================================================
template<class C, class P>
inline C * TPriorityQueue<C,P>::Root () const {
    return m_array.Count() ? m_array[0] : nil;
}

//===========================================================================
template<class C, class P>
inline void TPriorityQueue<C,P>::SetLinkOffset (int offset) {
    ASSERT(m_linkOffset == LINK_OFFSET_UNINIT);
    m_linkOffset = offset;
}

//===========================================================================
template<class C, class P>
inline C * const * TPriorityQueue<C,P>::Term () const {
    return m_array.Term();
}

//===========================================================================
template<class C, class P>
inline void TPriorityQueue<C,P>::Unlink (unsigned index) {
    Priority(m_array[index])->Link(nil, 0);
}

//===========================================================================
template<class C, class P>
inline void TPriorityQueue<C,P>::UnlinkAll () {
    for (unsigned loop = m_array.Count(); loop--; )
        Unlink(loop);
    m_array.ZeroCount();
}


/****************************************************************************
*
*   TPriorityQueueDecl
*
***/

template<class C, class P, int linkOffset>
class TPriorityQueueDecl : public TPriorityQueue<C,P> {
public:
    TPriorityQueueDecl () { SetLinkOffset(linkOffset); }
};


/****************************************************************************
*
*   TPriorityQueueDyn
*
***/

template<class C, class P>
class TPriorityQueueDyn : public TPriorityQueue<C,P> {
public:
    void Initialize (int linkOffset) { SetLinkOffset(linkOffset); }
};


/****************************************************************************
*
*   class TBasePriority
*
***/

template<class C, class P>
class TBasePriority {

public:
    TBasePriority () : m_queue(nil), m_index(0) { }
    virtual ~TBasePriority () { Unlink(); }

    void Unlink   ()       { if (m_queue) m_queue->Remove(m_index); }
    bool IsLinked () const { return m_queue != nil; }

public:
    TBasePriority (const TBasePriority &);
    const TBasePriority & operator= (const TBasePriority &);

protected:
    void Relink ();

private:
    void Link (TPriorityQueue<C,P> * queue, unsigned index);
    const TPriorityQueue<C,P> * GetLink () const { return m_queue; }
    unsigned GetIndex () const { return m_index; }

private:
    TPriorityQueue<C,P> * m_queue;
    unsigned              m_index;

    friend TPriorityQueue<C,P>; 
};

//===========================================================================
template<class C, class P>
inline void TBasePriority<C,P>::Link (TPriorityQueue<C,P> * queue, unsigned index) {
    m_queue = queue;
    m_index = index;
}

//===========================================================================
template<class C, class P>
void TBasePriority<C,P>::Relink () {

    // cache m_queue, since m_queue->Remove() will set it to nil
    TPriorityQueue<C,P> * queue = m_queue;
    if (!queue)
        return;
    C * object = (*queue)[m_index];
    queue->Remove(m_index);
    queue->Enqueue(object);
}


/****************************************************************************
*
*   class TPriorityNumeric
*
***/

template<class C,class T>
class TPriorityNumeric : public TBasePriority< C, TPriorityNumeric<C,T> > {

public:
    TPriorityNumeric () : m_value(0) { }
    TPriorityNumeric (T value) : m_value(value) { }
    void Set (T value) {
        if (value == m_value)
            return;
        m_value = value;
        Relink();
    }
    T Get () const {
        return m_value;
    }
    bool IsPriorityHigher (const TPriorityNumeric<C,T> & source) {
        return m_value > source.m_value;
    }
    bool IsPriorityHigher (T value) const {
        return m_value > value;
    }

private:
    T m_value;
};

/****************************************************************************
*
*   class TPriorityTime
*
***/

template<class C>
class TPriorityTime : public TBasePriority< C, TPriorityTime<C> > {

public:
    TPriorityTime () : m_time(0) { }
    TPriorityTime (unsigned time) : m_time(time) { }

    void Set (unsigned time) {
        if (m_time == time)
            return;
        m_time = time;
        Relink();
    }
    unsigned Get () const {
        return m_time;
    }
    bool IsPriorityHigher (const TPriorityTime<C> & source) const {
        return (int)(m_time - source.m_time) < 0;
    }
    bool IsPriorityHigher (unsigned time) const {
        return (int)(m_time - time) < 0;
    }

private:
    unsigned m_time;
};
