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
// UserPropMgr

#ifndef _USERPROPMGR_H_
#define _USERPROPMGR_H_

#include "Max.h"
#include "hsTypes.h"

template <class T> class hsHashTable;

class UserPropMgr {
public:
	enum
	{
		kMe = 0,
		kParent,
		kRoot
	};

	UserPropMgr();	// No Default Constructor!
	UserPropMgr(Interface *ip);
	~UserPropMgr();

	NameMaker *nm;

	void SetUserPropFlag(INode *node, const char *name, const BOOL setFlag, const Int32 hFlag=kMe);

	void SelectUserPropFlagALL(INode *node, const char *name, const BOOL flag);
	void ClearUserProp(INode *node, const char *name, const Int32 hFlag=kMe);
	void ClearUserPropALL(const char *name, const Int32 hFlag=kMe);
	void SetUserPropFlagALL(const char *name, const BOOL setFlag, const Int32 hFlag=kMe);
	BOOL GetUserPropFlagALL(const char *name, BOOL &isSet, const Int32 hFlag=kMe);

	BOOL GetUserProp(INode *node, const char *name, TSTR &value, const Int32 hFlag=kMe);
	void SetUserProp(INode *node, const char *name, const char *value, const Int32 hFlag=kMe);
	BOOL UserPropExists(INode *node, const char *name, const Int32 hFlag=kMe);

	BOOL GetUserPropString(INode *node, const char *name, TSTR &value, const Int32 hFlag=kMe);
	void SetUserPropString(INode *node, const char *name, const char *value, const Int32 hFlag=kMe);
	BOOL GetUserPropFloat(INode *node, const char *name, float &value, const Int32 hFlag=kMe);
	void SetUserPropFloat(INode *node, const char *name, const float value, const Int32 hFlag=kMe);
	BOOL GetUserPropInt(INode *node, const char *name, int &value, const Int32 hFlag=kMe);
	void SetUserPropInt(INode *node, const char *name, const int value, const Int32 hFlag=kMe);
	BOOL GetUserPropStringList(INode *node, const char *name, int &num, TSTR list[]);
	BOOL GetUserPropIntList(INode *node, const char *name, int &num, int list[]);
	BOOL GetUserPropFloatList(INode *node, const char *name, int &num, float list[]);

	BOOL GetUserPropStringALL(const char *name, TSTR &value, const Int32 hFlag=kMe);
	void SetUserPropStringALL(const char *name, const char *value, const Int32 hFlag=kMe);
	BOOL GetUserPropStringListALL(const char *name, int &num, TSTR list[]);
	BOOL GetUserPropIntListALL(const char *name, int &num, int *list);
	BOOL GetUserPropFloatListALL(const char *name, int &num, float *list);

	BOOL GetNodeNameALL(TSTR &name);
	void SetNodeNameALL(const char *name);

	void LoadVirtualProps(BOOL reset=true);
	void DestroyVirtualProps();
	BOOL IsVirtual();

	int GetSelNodeCount();
	INode *GetSelNode(int i);

	int GetUserPropCount(INode *node);
	void GetUserPropBuffer(INode *node, TSTR &buf);
	void SetUserPropBuffer(INode *node, const TSTR &buf);

	BOOL IsAlike(INode *node, BOOL MatchAll=true);
	int CountAlike(BOOL MatchAll=true);
	void DeSelectUnAlike(INode *node=NULL);

	Interface *GetInterface() { return ip; }

	void OpenQuickTable();
	void CloseQuickTable();

private:
	INode* GetAncestorIfNeeded(INode* node, const Int32 hFlag);
	void DeSelectWithOut(const char *name, const char *value);
	void RecursiveSelectAll(INode *node = NULL);
	int RecursiveCountAlike(INode *node = NULL, BOOL MatchAll=true);
	BOOL IsMatch(const char *val1, const char *val2);
	BOOL vProps;
	TSTR vbuf;
	TSTR vname;

	class QuickPair
	{
	public:
		static void SetBuffer(char* buf);
	protected:
		static char* fBuffer;
		const char* fKey;
		const char* fVal;
	public:
		QuickPair() : fKey(nil), fVal(nil) { }
		~QuickPair() { }

		void SetKey(const char* k) { fKey = k; }
		void SetVal(const char* v) { fVal = v; }

		UInt32 GetHash() const;

		hsBool GetVal(TSTR& value);

		bool operator==(const QuickPair& other) const;
	};
	hsHashTable<QuickPair>* fQuickTable;
	static const UInt32 kQuickSize;
	INode* fQuickNode;
	void IBuildQuickTable(INode* node);
	BOOL ICheckQuickEntry(const char *key, TSTR &value);

	Interface *ip;

};

#endif