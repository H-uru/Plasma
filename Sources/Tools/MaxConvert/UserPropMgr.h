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
#include <unordered_set>

#include "MaxMain/MaxCompat.h"

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

    void SetUserPropFlag(INode *node, const MCHAR *name, const bool setFlag, const int32_t hFlag=kMe);

    void SelectUserPropFlagALL(INode *node, const MCHAR *name, const bool flag);
    void ClearUserProp(INode *node, const MCHAR *name, const int32_t hFlag=kMe);
    void ClearUserPropALL(const MCHAR *name, const int32_t hFlag=kMe);
    void SetUserPropFlagALL(const MCHAR *name, const bool setFlag, const int32_t hFlag=kMe);
    bool GetUserPropFlagALL(const MCHAR *name, bool &isSet, const int32_t hFlag=kMe);

    bool GetUserProp(INode *node, const MCHAR *name, MSTR &value, const int32_t hFlag=kMe);
    void SetUserProp(INode *node, const MCHAR *name, const MCHAR *value, const int32_t hFlag=kMe);
    bool UserPropExists(INode *node, const MCHAR *name, const int32_t hFlag=kMe);

    bool GetUserPropString(INode *node, const MCHAR *name, MSTR &value, const int32_t hFlag=kMe);
    void SetUserPropString(INode *node, const MCHAR *name, const MCHAR *value, const int32_t hFlag=kMe);
    bool GetUserPropFloat(INode *node, const MCHAR *name, float &value, const int32_t hFlag=kMe);
    void SetUserPropFloat(INode *node, const MCHAR *name, const float value, const int32_t hFlag=kMe);
    bool GetUserPropInt(INode *node, const MCHAR *name, int &value, const int32_t hFlag=kMe);
    void SetUserPropInt(INode *node, const MCHAR *name, const int value, const int32_t hFlag=kMe);
    bool GetUserPropStringList(INode *node, const MCHAR *name, int &num, MSTR list[]);
    bool GetUserPropIntList(INode *node, const MCHAR *name, int &num, int list[]);
    bool GetUserPropFloatList(INode *node, const MCHAR *name, int &num, float list[]);

    bool GetUserPropStringALL(const MCHAR *name, MSTR &value, const int32_t hFlag=kMe);
    void SetUserPropStringALL(const MCHAR *name, const MCHAR *value, const int32_t hFlag=kMe);
    bool GetUserPropStringListALL(const MCHAR *name, int &num, MSTR list[]);
    bool GetUserPropIntListALL(const MCHAR *name, int &num, int *list);
    bool GetUserPropFloatListALL(const MCHAR *name, int &num, float *list);

    bool GetNodeNameALL(MSTR &name);
    void SetNodeNameALL(const MCHAR *name);

    void LoadVirtualProps(bool reset=true);
    void DestroyVirtualProps();
    bool IsVirtual();

    int GetSelNodeCount();
    INode *GetSelNode(int i);

    int GetUserPropCount(INode *node);
    void GetUserPropBuffer(INode *node, MSTR &buf);
    void SetUserPropBuffer(INode *node, const MSTR &buf);

    bool IsAlike(INode *node, bool MatchAll=true);
    int CountAlike(bool MatchAll=true);
    void DeSelectUnAlike(INode *node = nullptr);

    Interface *GetInterface() { return ip; }

    void OpenQuickTable();
    void CloseQuickTable();

private:
    INode* GetAncestorIfNeeded(INode* node, const int32_t hFlag);
    void DeSelectWithOut(const MCHAR* name, const MCHAR* value);
    void RecursiveSelectAll(INode *node = nullptr);
    int RecursiveCountAlike(INode *node = nullptr, bool MatchAll=true);
    bool IsMatch(const MCHAR *val1, const MCHAR *val2);
    bool vProps;
    MSTR vbuf;
    MSTR vname;

    class QuickPair
    {
    public:
        static void SetBuffer(MCHAR* buf);
    protected:
        static MCHAR* fBuffer;
        const MCHAR* fKey;
        const MCHAR* fVal;
    public:
        QuickPair() : fKey(), fVal() { }
        ~QuickPair() { }

        void SetKey(const MCHAR* k) { fKey = k; }
        void SetVal(const MCHAR* v) { fVal = v; }

        uint32_t GetHash() const;

        bool GetVal(MSTR& value) const;

        bool operator==(const QuickPair& other) const;
    };

    friend struct std::hash<QuickPair>;

    std::unordered_set<QuickPair>* fQuickTable;
    static const uint32_t kQuickSize;
    INode* fQuickNode;
    void IBuildQuickTable(INode* node);
    bool ICheckQuickEntry(const MCHAR *key, MSTR &value);

    Interface *ip;

};

namespace std
{
    template <>
    struct hash<UserPropMgr::QuickPair>
    {
        size_t operator()(const UserPropMgr::QuickPair& pair) const
        {
            return pair.GetHash();
        }
    };
}

#endif
