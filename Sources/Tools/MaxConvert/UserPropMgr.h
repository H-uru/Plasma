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
// UserPropMgr

#ifndef _USERPROPMGR_H_
#define _USERPROPMGR_H_

#include "HeadSpin.h"

template <class T> class hsHashTable;
class Interface;
class NameMaker;
class INode;

class UserPropMgr {
public:
    enum
    {
        kMe = 0,
        kParent,
        kRoot
    };

    UserPropMgr();  // No Default Constructor!
    UserPropMgr(Interface *ip);
    ~UserPropMgr();

    NameMaker *nm;

    void SetUserPropFlag(INode *node, const char *name, const bool setFlag, const int32_t hFlag=kMe);

    void SelectUserPropFlagALL(INode *node, const char *name, const bool flag);
    void ClearUserProp(INode *node, const char *name, const int32_t hFlag=kMe);
    void ClearUserPropALL(const char *name, const int32_t hFlag=kMe);
    void SetUserPropFlagALL(const char *name, const bool setFlag, const int32_t hFlag=kMe);
    bool GetUserPropFlagALL(const char *name, bool &isSet, const int32_t hFlag=kMe);

    bool GetUserProp(INode *node, const char *name, TSTR &value, const int32_t hFlag=kMe);
    void SetUserProp(INode *node, const char *name, const char *value, const int32_t hFlag=kMe);
    bool UserPropExists(INode *node, const char *name, const int32_t hFlag=kMe);

    bool GetUserPropString(INode *node, const char *name, TSTR &value, const int32_t hFlag=kMe);
    void SetUserPropString(INode *node, const char *name, const char *value, const int32_t hFlag=kMe);
    bool GetUserPropFloat(INode *node, const char *name, float &value, const int32_t hFlag=kMe);
    void SetUserPropFloat(INode *node, const char *name, const float value, const int32_t hFlag=kMe);
    bool GetUserPropInt(INode *node, const char *name, int &value, const int32_t hFlag=kMe);
    void SetUserPropInt(INode *node, const char *name, const int value, const int32_t hFlag=kMe);
    bool GetUserPropStringList(INode *node, const char *name, int &num, TSTR list[]);
    bool GetUserPropIntList(INode *node, const char *name, int &num, int list[]);
    bool GetUserPropFloatList(INode *node, const char *name, int &num, float list[]);

    bool GetUserPropStringALL(const char *name, TSTR &value, const int32_t hFlag=kMe);
    void SetUserPropStringALL(const char *name, const char *value, const int32_t hFlag=kMe);
    bool GetUserPropStringListALL(const char *name, int &num, TSTR list[]);
    bool GetUserPropIntListALL(const char *name, int &num, int *list);
    bool GetUserPropFloatListALL(const char *name, int &num, float *list);

    bool GetNodeNameALL(TSTR &name);
    void SetNodeNameALL(const char *name);

    void LoadVirtualProps(bool reset=true);
    void DestroyVirtualProps();
    bool IsVirtual();

    int GetSelNodeCount();
    INode *GetSelNode(int i);

    int GetUserPropCount(INode *node);
    void GetUserPropBuffer(INode *node, TSTR &buf);
    void SetUserPropBuffer(INode *node, const TSTR &buf);

    bool IsAlike(INode *node, bool MatchAll=true);
    int CountAlike(bool MatchAll=true);
    void DeSelectUnAlike(INode *node=NULL);

    Interface *GetInterface() { return ip; }

    void OpenQuickTable();
    void CloseQuickTable();

private:
    INode* GetAncestorIfNeeded(INode* node, const int32_t hFlag);
    void DeSelectWithOut(const char *name, const char *value);
    void RecursiveSelectAll(INode *node = NULL);
    int RecursiveCountAlike(INode *node = NULL, bool MatchAll=true);
    bool IsMatch(const char *val1, const char *val2);
    bool vProps;
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

        uint32_t GetHash() const;

        bool GetVal(TSTR& value);

        bool operator==(const QuickPair& other) const;
    };
    hsHashTable<QuickPair>* fQuickTable;
    static const uint32_t kQuickSize;
    INode* fQuickNode;
    void IBuildQuickTable(INode* node);
    bool ICheckQuickEntry(const char *key, TSTR &value);

    Interface *ip;

};

#endif