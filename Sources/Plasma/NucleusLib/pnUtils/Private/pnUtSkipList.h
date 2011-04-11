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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSkipList.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSKIPLIST_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSkipList.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSKIPLIST_H


/*****************************************************************************
*
*   Macros
*
***/

#define SKIPLIST(type, keyType, keyField, cmp)		TSkipList< type, keyType, offsetof(type, keyField), cmp >
#define SKIPLIST_NUMERIC(type, keyType, keyField)	SKIPLIST(type, keyType, keyField, TSkipListNumericCmp<keyType>)
#define SKIPLIST_STRING(type, keyType, keyField)	SKIPLIST(type, keyType, keyField, TSkipListStringCmp<keyType>)
#define SKIPLIST_STRINGI(type, keyType, keyField)	SKIPLIST(type, keyType, keyField, TSkipListStringCmpI<keyType>)



/*****************************************************************************
*
*   Typedefs
*
***/

typedef void * SkipListTag;


/*****************************************************************************
*
*   Comparers
*
***/


template<class K>
class TSkipListNumericCmp {
public:
	static bool Eq (const K & a, const K & b) { return a == b; }
	static bool Lt (const K & a, const K & b) { return a < b;  }
};

template<class K>
class TSkipListStringCmp {
public:
	static bool Eq (const K & a, const K & b) { return StrCmp(a, b, (unsigned)-1) == 0; }
	static bool Lt (const K & a, const K & b) { return StrCmp(a, b, (unsigned)-1) < 0;  }
};

template<class K>
class TSkipListStringCmpI {
public:
	static bool Eq (const K & a, const K & b) { return StrCmpI(a, b, (unsigned)-1) == 0; }
	static bool Lt (const K & a, const K & b) { return StrCmpI(a, b, (unsigned)-1) < 0;  }
};


/*****************************************************************************
*
*   TSkipList
*
***/

template<class T, class K, unsigned keyOffset, class Cmp>
class TSkipList {
private:
	enum { kMaxLevels = 32 };

	template<class T, class K>
	struct TNode {
		const K *     key;
		T *           object;
		unsigned      level;
		TNode<T, K> * prev;
		TNode<T, K> * next[1];      // variable size array
	};
	typedef TNode<T,K> Node;

	unsigned	m_level;
	Node *		m_head;
	Node *		m_stop;
	unsigned	m_randomBits;
	unsigned	m_randomsLeft;

	Node *		AllocNode (unsigned levels);
	void		FreeNode (Node * node);
	unsigned	RandomLevel ();

public:
    inline TSkipList ();
    inline ~TSkipList ();
    inline void Clear ();
    inline void Delete (T * object);
    inline T *  Find (const K & key, SkipListTag * tag = nil) const;
    inline T *  FindNext (SkipListTag * tag) const;
    inline T *  Head (SkipListTag * tag) const;
    inline T *  Next (SkipListTag * tag) const;
    inline T *  Prev (SkipListTag * tag) const;
    inline T *  Tail (SkipListTag * tag) const;
    inline void Link (T * object);
    inline void Unlink (T * object);
    inline void Unlink (SkipListTag * tag);
    inline void UnlinkAll ();
    
    #ifdef HS_DEBUGGING
    inline void Print () const;
    #endif
};


/*****************************************************************************
*
*   TSkipList private member functions
*
***/

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
typename TSkipList<T,K,keyOffset,Cmp>::TNode<T,K> * TSkipList<T,K,keyOffset,Cmp>::AllocNode (unsigned level) {

	unsigned size = offsetof(Node, next) + (level + 1) * sizeof(Node);
	Node * node = (Node *)ALLOC(size);
	node->level = level;
	return node;
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
void TSkipList<T,K,keyOffset,Cmp>::FreeNode (TNode<T,K> * node) {

	FREE(node);
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
unsigned TSkipList<T,K,keyOffset,Cmp>::RandomLevel () {

	unsigned level = 0;
	unsigned bits  = 0;

	while (!bits) {
		bits = m_randomBits % 4;
		if (!bits)
			++level;
		m_randomBits >>= 2;
		m_randomsLeft -= 2;
		if (!m_randomsLeft) {
			m_randomBits  = RandUnsigned();
			m_randomsLeft = 30;
		}
	}

	return level;
}


/*****************************************************************************
*
*   TSkipList public member functions
*
***/

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
TSkipList<T,K,keyOffset,Cmp>::TSkipList () {

	m_level       = 0; 
	m_head        = AllocNode(kMaxLevels);
	m_stop        = AllocNode(0);
	m_randomBits  = RandUnsigned();
	m_randomsLeft = 30;

	// Initialize header and stop skip node pointers
	m_stop->prev    = m_head;
	m_stop->object  = nil;
	m_stop->next[0] = nil;
	m_head->object  = nil;
	for (unsigned index = 0; index < kMaxLevels; ++index)
		m_head->next[index] = m_stop;
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
TSkipList<T,K,keyOffset,Cmp>::~TSkipList () {

	UnlinkAll();
	ASSERT(m_stop->prev == m_head);
	FreeNode(m_head);
	FreeNode(m_stop);
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
void TSkipList<T,K,keyOffset,Cmp>::Clear () {

	Node * ptr = m_head->next[0];
	while (ptr != m_stop) {
		Node * next = ptr->next[0];
		DEL(ptr->object);
		FreeNode(ptr);
		ptr = next;
	}

	m_stop->prev = m_head;
	for (unsigned index = 0; index < kMaxLevels; ++index)
		m_head->next[index] = m_stop;
	m_level = 0;
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
void TSkipList<T,K,keyOffset,Cmp>::Delete (T * object) {

	Unlink(object);
	DEL(object);
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
T * TSkipList<T,K,keyOffset,Cmp>::Find (const K & key, SkipListTag * tag) const {

	Node * node = m_head;

	m_stop->key = &key;
	for (int level = (int)m_level; level >= 0; --level)
		while (Cmp::Lt(*node->next[level]->key, key))
			node = node->next[level];

	node = node->next[0];
	if (node != m_stop && Cmp::Eq(*node->key, *m_stop->key)) {
		if (tag)
			*tag = node;
		return node->object;
	}
	else {
		if (tag)
			*tag = nil;
		return nil;
	}
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
T * TSkipList<T,K,keyOffset,Cmp>::FindNext (SkipListTag * tag) const {

	Node * node = (Node *)*tag;

	m_stop->key = node->key;
	for (int level = (int)node->level; level >= 0; --level)
		while (Cmp::Lt(*node->next[level]->key, *m_stop->key))
			node = node->next[level];

	node = node->next[0];
	if (node != m_stop && Cmp::Eq(*node->key, *m_stop->key)) {
		*tag = node;
		return node->object;
	}
	else {
		*tag = nil;
		return nil;
	}
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
T * TSkipList<T,K,keyOffset,Cmp>::Head (SkipListTag * tag) const {

	ASSERT(tag);
	Node * first = m_head->next[0];
	if (first == m_stop) {
		*tag = nil;
		return nil;
	}

	*tag = first;
	return first->object;
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
T * TSkipList<T,K,keyOffset,Cmp>::Next (SkipListTag * tag) const {

	ASSERT(tag);
	Node * node = (Node *)*tag;
	ASSERT(node);
	if (node->next[0] == m_stop) {
		*tag = nil;
		return nil;
	}

	*tag = node->next[0];
	return node->next[0]->object;
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
T * TSkipList<T,K,keyOffset,Cmp>::Prev (SkipListTag * tag) const {

	ASSERT(tag);
	Node * node = (Node *)*tag;
	ASSERT(node);
	if (node->prev == m_head) {
		*tag = nil;
		return nil;
	}

	*tag = node->prev;
	return node->prev->object;
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
T * TSkipList<T,K,keyOffset,Cmp>::Tail (SkipListTag * tag) const {

	ASSERT(tag);
	Node * last = m_stop->prev;
	if (last == m_head) {
		*tag = nil;
		return nil;
	}

	*tag = last;
	return last->object;
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
void TSkipList<T,K,keyOffset,Cmp>::Link (T * object) {

	const K * key = (const K *)((const byte *)object + keyOffset);

	// Find the node's insertion point
	m_stop->key = key;
	Node * update[kMaxLevels];
	Node * node = m_head;
	for (int level = (int)m_level; level >= 0; --level) {
		while (Cmp::Lt(*node->next[level]->key, *key))
			node = node->next[level];
		update[level] = node;
	}
	node = node->next[0];

	{
		// Select a level for the skip node
		unsigned newLevel = RandomLevel();
		if (newLevel > m_level) {
			if (m_level < kMaxLevels - 1) {
				newLevel         = ++m_level;
				update[newLevel] = m_head;
			}
			else
				newLevel = m_level;
		}

		// Create the node and insert it into the skip list
		Node * node  = AllocNode(newLevel);
		node->key    = key;
		node->object = object;
		for (unsigned level = newLevel; level >= 1; --level) {
			node->next[level] = update[level]->next[level];
			update[level]->next[level] = node;
		}
		node->prev               = update[0];
		node->next[0]            = update[0]->next[0];
		update[0]->next[0]->prev = node;
		update[0]->next[0]       = node;
	}
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
void TSkipList<T,K,keyOffset,Cmp>::Unlink (T * object) {

	const K * key = (const K *)((const byte *)object + keyOffset);

	Node * node = m_head;
	Node * update[kMaxLevels];
	int    level = m_level;

	for (;;) {	
		// Find the node being unlinked
		m_stop->key = key;
		for (; level >= 0; --level) {
			while (Cmp::Lt(*node->next[level]->key, *key))
				node = node->next[level];
			update[level] = node;
		}
		node = node->next[0];

		// Node wasn't found so do nothing
		if (*node->key != *key || node == m_stop)
			return;
			
		if (node->object == object)
			break;
	}
	
	// Update all links
	for (level = m_level; level >= 1; --level) {
		if (update[level]->next[level] != node)
			continue;
		update[level]->next[level] = node->next[level];
	}
	ASSERT(update[0]->next[0] == node);
	node->next[0]->prev = update[0];
	update[0]->next[0]  = node->next[0];

	// Update header
	while (m_level && m_head->next[m_level] == m_stop)
		--m_level;

	FreeNode(node);
}

//============================================================================
template<class T, class K, unsigned keyOffset, class Cmp>
void TSkipList<T,K,keyOffset,Cmp>::UnlinkAll () {

	Node * ptr = m_head->next[0];
	while (ptr != m_stop) {
		Node * next = ptr->next[0];
		FreeNode(ptr);
		ptr = next;
	}

	m_stop->prev = m_head;
	for (unsigned index = 0; index < kMaxLevels; ++index)
		m_head->next[index] = m_stop;
	m_level = 0;
}
