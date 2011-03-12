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
#include "hsStringTable.h"
#include "stdlib.h"
#if HS_BUILD_FOR_PS2 || __MWERKS__ || HS_BUILD_FOR_UNIX
#include <ctype.h>
#endif
//
// hsStringTable
//

hsStringTable::Node::Node(char c) : sib(0), kid(0), chr(c), data(0) {};
hsStringTable::Node::~Node() { };
void* hsStringTable::Node::GetData() { return data; }
void hsStringTable::Node::SetData(void* v) { data = v; }

hsStringTable::~hsStringTable() 
{
	RemoveNode(root.kid);
}

void hsStringTable::Reset() 
{
	RemoveNode(root.kid);
	root.kid = nil;
}


//
// Find a the Node associated with a given string
//
hsStringTable::Node* hsStringTable::Find(const char* str)
{
	return (str && *str) ? FindRecur(root.kid, str) : nil;
}

//
// Find the unique node matching the given prefix. Returns the node
// at the first point where a unique path can not be determined
// char* str is filled out with the matching path
//
hsStringTable::Node* hsStringTable::FindPartial(char* str, Int32 len) const
{
	return (str && *str) ? FindPartialRecur(root.kid, str, len) : nil;
}

//
// Find and Add if needs a node for str and register it with data
//
void hsStringTable::Register(const char* str, void* data) 
{
	Node* node = FindRecur(root.kid, str, true);
	if (!node) 
	{
		node = AddRecur(&root, str);
	}
	node->SetData(data);
}

//
// Iterate through the tree and call the callback function on each child node
// of the fromNode (or the root if fromNode is nil)
//
hsBool hsStringTable::Iterate(hsStringTableCallback* callback, Node* fromNode)
{
	if (!fromNode)
		fromNode = &root;
	return IterateRecur(fromNode->kid,callback);
}

//
// Recursively find node, create if needed?
//
hsStringTable::Node* hsStringTable::FindRecur(Node* root, const char* str, hsBool createIfNeeded)
{
	if (!root || !str) return nil;
	if (tolower(root->chr)==tolower(*str)) 
	{
		if (*(str+1)) 
		{
			Node* node = FindRecur(root->kid, str+1, createIfNeeded);
			if (!node && createIfNeeded) node = AddRecur(root, str+1);
			return node;
		} 
		else 
		{
			return root;
		}
	} 
	else 
	{
		return FindRecur(root->sib,str,createIfNeeded);
	}
}

//
// Recursively find the unique partial match
//
hsStringTable::Node* hsStringTable::FindPartialRecur(Node* root, char* str, Int32 len) const
{
	if (!root || !str) 
	{
		return nil;
	}

	if (tolower(root->chr)==tolower(*str)) 
	{
		if (len) 
		{
			*str = root->chr;
		}
		
		if (*(str+1)) 
		{
			Node* node = FindPartialRecur(root->kid,str+1,len-1);
			//if (!node) node = AddRecur(root,str+1);
			return node;
		} 
		else 
		{
			return FindLeafRecur(root, str, len);
		}
	} 
	else 
	{
		return FindPartialRecur(root->sib, str, len);
	}
}

//
// Follow the current node as far as possible towards a unique leaf
//
hsStringTable::Node* hsStringTable::FindLeafRecur(Node* root, char* str, Int32 len) const
{
	if (root->data || !root->kid || root->kid->sib) 
	{
		if (len) 
		{
			*(str+1) = 0;
		}
		return root;
	} 
	else 
	{
		if (len) 
		{
			*(str+1) = len>1 ? root->kid->chr : 0;
		}
		return FindLeafRecur(root->kid,str+1,len-1);
	}
}


//
// Add a string to the tree
//
hsStringTable::Node* hsStringTable::AddRecur(Node* root, const char* str) 
{
	Node* node = TRACKED_NEW Node(*str);
	node->sib = root->kid;
	root->kid = node;
	if (*(str+1)) return AddRecur(node,str+1);
	else return node;
}

//
// Remove a node recursive from the tree
//
void hsStringTable::RemoveNode(Node* root) 
{
	if (!root) return;
	RemoveNode(root->kid);
	RemoveNode(root->sib);
	delete root;
}

//
// Recurse through tree and call callback on each node
//
hsBool hsStringTable::IterateRecur(Node* root, hsStringTableCallback* callback)
{
	if (!root)
		return true;
	if (root->data)
	{
		if (!callback(root))
			return false;
	}
	if (root->kid)
	{
		if (!IterateRecur(root->kid,callback))
			return false;
	}
	if (root->sib)
	{
		if (!IterateRecur(root->sib,callback))
			return false;
	}
	return true;
}
