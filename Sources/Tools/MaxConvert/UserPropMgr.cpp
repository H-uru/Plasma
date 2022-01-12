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

#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "UserPropMgr.h"
#include "MaxMain/hsMStringTokenizer.h"

#define REFMSG_USERPROP  (REFMSG_USER + 1)

UserPropMgr gUserPropMgr(GetCOREInterface());

UserPropMgr::UserPropMgr(Interface *ip) : 
nm()
{
    this->ip = ip;

    fQuickTable = nullptr;
    fQuickNode = nullptr;

    vProps = false;
}

UserPropMgr::~UserPropMgr()
{
    CloseQuickTable();
}

void UserPropMgr::SetUserPropFlag(INode *node, const MCHAR *name, const bool setFlag, const int32_t hFlag) 
{
    if (setFlag)
        SetUserProp(node, name, nullptr, hFlag);
    else
        ClearUserProp(node, name, hFlag);
}

void UserPropMgr::ClearUserPropALL(const MCHAR *name, const int32_t hFlag) 
{
    for (int i=0; i<GetSelNodeCount(); i++) 
    {
        ClearUserProp(GetSelNode(i),name,hFlag);
    }
}

void UserPropMgr::SelectUserPropFlagALL(INode *node, const MCHAR *name, const bool flag) {
    if (node) 
    {
        if (UserPropExists(node,name) == flag) ip->SelectNode(node,false);
    } else node = ip->GetRootNode();

    for (int i=0; i<node->NumberOfChildren(); i++) {
        SelectUserPropFlagALL(node->GetChildNode(i),name,flag);
    }
}


void UserPropMgr::DeSelectWithOut(const MCHAR *name, const MCHAR *value) {
    bool oldProps = vProps;
    vProps=false;
    MSTR val;
    INode *nodes[1];
    INodeTab nodeTab;
    for (int i=0; i<GetSelNodeCount(); i++) {
        if (value) {
            if (!(GetUserProp(GetSelNode(i),name,val) && _tcsicmp(val,value) == 0)) {
                nodes[0] = GetSelNode(i);
                nodeTab.Append(1,nodes);
            }
        } else if (!UserPropExists(GetSelNode(i),name)) {
            nodes[0] = GetSelNode(i);
            nodeTab.Append(1,nodes);
        }
    }
    vProps=oldProps;
    if (nodeTab.Count() > 0) ip->SelectNodeTab(nodeTab,false,false);
    
}

void UserPropMgr::RecursiveSelectAll(INode *node) {
    if (node) {
        if (!node->Selected()) ip->SelectNode(node,false);
    } else node = ip->GetRootNode();

    for (int i=0; i<node->NumberOfChildren(); i++) {
        RecursiveSelectAll(node->GetChildNode(i));
    }
}


void UserPropMgr::DeSelectUnAlike(INode *node) {
theHold.Begin();

ip->ThawSelection();

    RecursiveSelectAll();

    MSTR buf;
    GetUserPropBuffer(node,buf);

    hsMStringTokenizer toker(buf, _M(" \r\n"));
    toker.ParseQuotes(TRUE);
    MCHAR *tok;
    MSTR name;
    bool isName = true;
    while (tok=toker.next()) {
        if (isName) {
            if (*tok != '=') {
                name = tok;
                tok = toker.next();
                if (tok && *tok == '=') {
                    tok = toker.next();
                } else
                    tok = nullptr;
                DeSelectWithOut(name,tok);
            } else isName = false;
        } else {
            isName = true;
        }
    }


    MSTR undostr; undostr.printf(_M("Select"));
    theHold.Accept(undostr);

    ip->FreezeSelection();

    ip->RedrawViews(ip->GetTime());
}



int UserPropMgr::RecursiveCountAlike(INode *node, bool MatchAll) {
    int count=0;
    if (node) {
        if (!node->IsNodeHidden() && !node->IsFrozen() && IsAlike(node,MatchAll)) count++;
    } else node = ip->GetRootNode();

    for (int i=0; i<node->NumberOfChildren(); i++) {
        count += RecursiveCountAlike(node->GetChildNode(i),MatchAll);
    }
    return count;
}


int UserPropMgr::CountAlike(bool MatchAll) {
    return RecursiveCountAlike(nullptr, MatchAll);
}

bool UserPropMgr::IsMatch(const MCHAR *val1, const MCHAR *val2) {
    if (_tcsicmp(val1, val2) == 0)
        return true;

    hsMStringTokenizer toker(val1, _M(" ,@"));
    MCHAR *tok;

    while (tok=toker.next()) {
        hsMStringTokenizer toker2(val2, _M(" ,@"));
        bool found = false;
        MCHAR *tok2;
        while ((tok2=toker2.next()) && !found) {
            if (tok[0] >= '1' && tok[0] <= '0') {
                if (_tcsicmp(tok, tok2) == 0) 
                    found = true;
            } else if (toker.HasMoreTokens()) {
                if (_tcsicmp(tok, tok2) == 0)
                    found = true;
            } else {
                if (_tcsnicmp(tok, tok2, _tcslen(tok)) == 0)
                    found = true;
            }
        }
        if (!found) return false;
    }
    return true;
}


bool UserPropMgr::IsAlike(INode *node, bool MatchAll) {
    MSTR buf;
    GetUserPropBuffer(node,buf);

    bool oldProps = vProps;
    vProps=false;

    hsMStringTokenizer toker(buf, _M(" \r\n"));
    toker.ParseQuotes(TRUE);
    MCHAR* tok;
    MSTR name;
    MSTR value;
    MSTR tval;
    bool match = MatchAll;
    bool isName = true;
    tok = toker.next();
    while (tok && (match==MatchAll)) {
        if (isName) {
            if (*tok != '=') {
                name = tok;
                tok = toker.next();
                if (tok && *tok == '=') {
                    tok = toker.next();
                    if (tok) value = tok;
                    else value = _M("");
                    tok = toker.next();
                } else value = _M("");
                if (GetUserProp(node,name,tval)) match = IsMatch(value,tval);
                else match = false;
                continue;
            } else isName = false;
        } else {
            isName = true;
        }
        tok=toker.next();
    }

    if (match==MatchAll) {
        if (!vname.isNull()) match = IsMatch(vname,node->GetName());
    }

    vProps = oldProps;
    return match;
}

int UserPropMgr::GetUserPropCount(INode *node) {
    MSTR buf;

    GetUserPropBuffer(node,buf);

    int numProps = 0;

    hsMStringTokenizer toker(buf, _M(" \r\n"));
    toker.ParseQuotes(TRUE);
    MCHAR *tok;
    bool isName = true;
    while (tok=toker.next()) {
        if (isName) {
            if (*tok != _M('=')) {
                numProps++;
            } else isName = false;
        } else {
            isName = true;
        }
    }

    return numProps;
}

void UserPropMgr::GetUserPropBuffer(INode *node, MSTR &buf) {
    if (vProps) buf = vbuf;
    else if (node) node->GetUserPropBuffer(buf);
    else buf = _M("");
}

void UserPropMgr::SetUserPropBuffer(INode *node, const MSTR &buf) 
{
    // QuickTable invalidate
    if (node && node == fQuickNode)
    {
        fQuickNode = nullptr;
    }

    if (vProps)
    {
        vbuf = buf;
    }
    else if (node)
    {
        node->SetUserPropBuffer(buf);
        node->NotifyDependents(FOREVER, PART_ALL, REFMSG_USERPROP);
    }
}

void UserPropMgr::SetUserPropFlagALL(const MCHAR *name, const bool setFlag, const int32_t hFlag) 
{
    for (int i=0; i<GetSelNodeCount();i++) 
    {
        SetUserPropFlag(GetSelNode(i),name,setFlag,hFlag);
    }
}
bool UserPropMgr::GetUserPropFlagALL(const MCHAR *name, bool &isSet, const int32_t hFlag)
 {
    isSet = UserPropMgr::UserPropExists(GetSelNode(0),name,hFlag);

    for (int i=0; i<GetSelNodeCount(); i++) {
        if (isSet != UserPropMgr::UserPropExists(GetSelNode(i),name,hFlag)) return FALSE;
    }
    return TRUE;
}

INode* UserPropMgr::GetAncestorIfNeeded(INode* node, const int32_t hFlag)
{
    if (hFlag == kParent)
    {
        if (!(node->IsRootNode() || node->GetParentNode()->IsRootNode()))
            node = node->GetParentNode();
    }
    else
    if (hFlag == kRoot)
    {
        while (!(node->IsRootNode() || node->GetParentNode()->IsRootNode()))
            node = node->GetParentNode();
    }
    return node;
}


void UserPropMgr::ClearUserProp(INode *node, const MCHAR *name, const int32_t hFlag) 
{
    node = GetAncestorIfNeeded(node,hFlag);

    // QuickTable invalidate
    if (node && node == fQuickNode)
    {
        fQuickNode = nullptr;
    }

    MSTR buf;
    GetUserPropBuffer(node,buf);

    hsMStringTokenizer toker(buf, _M(" \r\n"));
    toker.ParseQuotes(TRUE);
    MCHAR *tok;
    bool isName = true;
    while (tok=toker.next()) 
    {
        if (isName) 
        {
            if (*tok != '=') 
            {
                if (_tcsicmp(tok, name) == 0)
                {
                    MCHAR *tok2 = toker.next();
                    if (tok2) 
                    {
                        if (*tok2 == '=')
                        {
                            tok2 = toker.next();
                            if (tok2)
                            {
                                tok2 = toker.next();
                                if (tok2)
                                {
                                    buf.remove((int)(tok-toker.fString), (int)(tok2-tok));
                                }
                                else
                                {
                                    buf.remove((int)(tok-toker.fString));
                                }
                            }
                            else
                            {
                                buf.remove((int)(tok-toker.fString));
                            }
                        }
                        else 
                        {
                            buf.remove((int)(tok-toker.fString), (int)(tok2-tok));
                        }
                    } 
                    else
                    {
                        buf.remove((int)(tok-toker.fString));
                    }
                    break;
                }
            } 
            else
            {
                isName = false;
            }
        }
        else
        {
            isName = true;
        }
    }
    if (vProps) 
    {
        vbuf = buf;
    }
    else 
    {
        node->SetUserPropBuffer(buf);
        node->NotifyDependents(FOREVER, PART_ALL, REFMSG_USERPROP);
    }
};

bool UserPropMgr::GetUserProp(INode *node, const MCHAR *name, MSTR &value, const int32_t hFlag)
{
    node = GetAncestorIfNeeded(node,hFlag);

    // QuickTable lookup
    if (node && fQuickTable)
    {
        if (node != fQuickNode)
            IBuildQuickTable(node);
        return ICheckQuickEntry(name,value);
    }

    MSTR buf;
    GetUserPropBuffer(node,buf);

    hsMStringTokenizer toker(buf, _M(" \r\n"));
    toker.ParseQuotes(TRUE);
    MCHAR *tok;
    bool isName = true;
    while (tok=toker.next()) 
    {
        if (isName)
        {
            if (*tok != '=')
            {
                if (_tcsicmp(tok,name) == 0)
                {
                    tok = toker.next();
                    if (tok && *tok == '=')
                    {
                        tok = toker.next();
                        if (tok) value = tok;
                        else value = _M("");
                        return true;
                    }
                    else 
                    {
                        value = _M("");
                        return true;
                    }
                }
            } 
            else 
                isName = false;
        } 
        else
        {
            isName = true;
        }
    }
    return false;
}

void UserPropMgr::SetUserProp(INode *node, const MCHAR *name, const MCHAR *value, const int32_t hFlag) 
{
    node = GetAncestorIfNeeded(node,hFlag);

    // QuickTable invalidate
    if (node && node == fQuickNode)
    {
        fQuickNode = nullptr;
    }

    MSTR buf;
    GetUserPropBuffer(node,buf);

    hsMStringTokenizer toker(buf, _M(" \r\n"));
    toker.ParseQuotes(TRUE);
    MCHAR *tok;
    bool isName = true;
    while (tok=toker.next())
    {
        if (isName) 
        {
            if (*tok != '=') 
            {
                if (!_tcsicmp(tok,name) == 0)
                {
                    MCHAR *tok2 = toker.next();
                    if (tok2)
                    {
                        if (*tok2 == '=')
                        {
                            tok2 = toker.next();
                            if (tok2) 
                            {
                                tok2 = toker.next();
                                if (tok2)
                                {
                                    buf.remove((int)(tok-toker.fString), (int)(tok2-tok));
                                } 
                                else
                                {
                                    buf.remove((int)(tok-toker.fString));
                                }
                            } 
                            else
                            {
                                buf.remove((int)(tok-toker.fString));
                            }
                        } 
                        else
                        {
                            buf.remove((int)(tok-toker.fString), (int)(tok2-tok));
                        }
                    } 
                    else
                    {
                        buf.remove((int)(tok-toker.fString));
                    }
                    break;
                }
            } 
            else 
            {
                isName = false;
            }
        } 
        else
        {
            isName = true;
        }
    }
    if (buf.last(_M('\n')) < buf.length()-1)
    {
        // better start with a separator
        buf += _M("\r\n");
    }
    buf += name;
    if (value && *value)
    {
        buf += _M(" = ");
        if (_tcschr(value, _M(' ')))
        {
            buf += _M("\"");
            buf += value;
            buf += _M("\"");
        }
        else
        {
            buf += value;
        }
    }
    buf += _M("\r\n");
    if (vProps)
    {
        vbuf = buf;
    }
    else
    {
        node->SetUserPropBuffer(buf);
        node->NotifyDependents(FOREVER, PART_ALL, REFMSG_USERPROP);
    }
}


bool UserPropMgr::GetUserPropString(INode *node, const MCHAR *name, MSTR &value, const int32_t hFlag)
{
     return GetUserProp(node,name,value,hFlag);
}
void UserPropMgr::SetUserPropString(INode *node, const MCHAR *name, const MCHAR *value, const int32_t hFlag) 
{
    SetUserProp(node,name,value,hFlag);
}
bool UserPropMgr::GetUserPropFloat(INode *node, const MCHAR *name, float &value, const int32_t hFlag)
{
    MSTR valStr;
    if (GetUserProp(node,name,valStr,hFlag)) 
    {
        value = (float)_ttof(valStr);
        return true;
    }
    return false;
}
void UserPropMgr::SetUserPropFloat(INode *node, const MCHAR *name, const float value, const int32_t hFlag) 
{
    MSTR valStr;
    if (valStr.printf(_M("%g"), value))
        SetUserProp(node, name, valStr, hFlag);
}
bool UserPropMgr::GetUserPropInt(INode *node, const MCHAR *name, int &value, const int32_t hFlag)
{
    MSTR valStr;
    if (GetUserProp(node,name,valStr,hFlag)) {
        value = _ttoi(valStr);
        return true;
    }
    return false;
}
void UserPropMgr::SetUserPropInt(INode *node, const MCHAR *name, const int value, const int32_t hFlag) 
{
    MSTR valStr;
    if (valStr.printf(_M("%d"), value))
        SetUserProp(node, name, valStr, hFlag);
}

bool UserPropMgr::UserPropExists(INode *node, const MCHAR *name, const int32_t hFlag) 
{
    MSTR value;
    return GetUserProp(node,name,value,hFlag);
}

bool UserPropMgr::GetUserPropStringList(INode *node, const MCHAR *name, int &num, MSTR list[]) {
    MSTR sdata;
    if (UserPropMgr::GetUserPropString(node,name,sdata)) {
        num=0;
        hsMStringTokenizer toker(sdata, _M(", "));
        MCHAR *tok;
        while ( tok = toker.next() ) {
            list[num] = tok;
            num++;
        }
        return true;
    } else return false;
}

bool UserPropMgr::GetUserPropIntList(INode *node, const MCHAR *name, int &num, int list[]) {
    MSTR sdata;
    if (UserPropMgr::GetUserPropString(node,name,sdata)) {
        num=0;
        hsMStringTokenizer toker(sdata, _M(", "));
        MCHAR *tok;
        while ( tok = toker.next() ) {
            list[num] = _ttoi(tok);
            num++;
        }
        return true;
    } else return false;
}

bool UserPropMgr::GetUserPropFloatList(INode *node, const MCHAR *name, int &num, float list[]) {
    MSTR sdata;
    if (UserPropMgr::GetUserPropString(node,name,sdata)) {
        num=0;
        hsMStringTokenizer toker(sdata, _M(", "));
        MCHAR *tok;
        while ( tok = toker.next() ) {
            list[num] = (float)_ttof(tok);
            num++;
        }
        return true;
    } else return false;
}

bool UserPropMgr::GetUserPropStringALL(const MCHAR *name, MSTR &value, const int32_t hFlag)
{
    bool propSet  = UserPropMgr::GetUserPropString(GetSelNode(0),name,value,hFlag);

    MSTR tvalue;
    int i=1;
    bool propMixed = FALSE;
    while (i < GetSelNodeCount() && !propMixed) {
        if (propSet ^ UserPropMgr::GetUserPropString(GetSelNode(i),name,tvalue,hFlag)) propMixed = TRUE;
        propMixed = (!(value == tvalue));
        i++;
    }

    return (!propMixed);
}
void UserPropMgr::SetUserPropStringALL(const MCHAR *name, const MCHAR *value, const int32_t hFlag) 
{
    for (int i=0; i<GetSelNodeCount(); i++) {
        UserPropMgr::SetUserPropString(GetSelNode(i),name,value,hFlag);
    }
}

bool UserPropMgr::GetUserPropStringListALL(const MCHAR *name, int &num, MSTR list[]) {
    MSTR val;
    GetUserPropStringList(GetSelNode(0),name,num,list);
    return GetUserPropStringALL(name,val);
}

bool UserPropMgr::GetUserPropIntListALL(const MCHAR *name, int &num, int *list) {
    MSTR val;
    GetUserPropIntList(GetSelNode(0),name,num,list);
    return GetUserPropStringALL(name,val);
}

bool UserPropMgr::GetUserPropFloatListALL(const MCHAR *name, int &num, float *list) {
    MSTR val;
    GetUserPropFloatList(GetSelNode(0),name,num,list);
    return GetUserPropStringALL(name,val);
}

bool UserPropMgr::GetNodeNameALL(MSTR &name) {
    if (vProps) name = vname;
    else if (ip->GetSelNodeCount() == 1) name = ip->GetSelNode(0)->GetName();
    else return false;

    return true;
}

void UserPropMgr::SetNodeNameALL(const MCHAR *name) {
    if (vProps) {
        vname = name;
    } else {
        if (ip->GetSelNodeCount() > 1) {
            MSTR uName;
            for (int i=0; i<ip->GetSelNodeCount(); i++) {
                uName = name;
                ip->MakeNameUnique(uName);
                ip->GetSelNode(i)->SetName(uName);
            }
        } else ip->GetSelNode(0)->SetName((MCHAR*)name);
    }
}


void UserPropMgr::LoadVirtualProps(bool reset) {
    if (reset)
    {
        vbuf = _M("");
        vname = _M("");
    }
    vProps = true;
}
void UserPropMgr::DestroyVirtualProps() {
    vProps = false;
}
bool UserPropMgr::IsVirtual() {
    return vProps;
}

int UserPropMgr::GetSelNodeCount() {
    if (vProps) return 1;
    else return ip->GetSelNodeCount();
}
INode *UserPropMgr::GetSelNode(int i) {
    if (vProps)
        return nullptr;
    else
        return ip->GetSelNode(i);
}


void UserPropMgr::OpenQuickTable()
{
    if (!fQuickTable)
    {
        fQuickTable = new std::unordered_set<QuickPair>;
    }
    fQuickNode = nullptr;
}

void UserPropMgr::CloseQuickTable()
{
    delete fQuickTable;
    fQuickTable = nullptr;
    fQuickNode = nullptr;
    QuickPair::SetBuffer(nullptr);
}

void UserPropMgr::IBuildQuickTable(INode* node)
{
    if (fQuickTable && fQuickNode != node)
    {
        fQuickNode = node;

        // clear old QuickTable
        fQuickTable->clear();

        // build new one
        MSTR buf;
        GetUserPropBuffer(node,buf);

        hsMStringTokenizer toker(buf, _M(" \r\n"));
        toker.ParseQuotes(TRUE);

        MCHAR *tok;
        bool inName = false;
        bool isName = true;
        while ( inName || (tok=toker.next()) ) 
        {
            if (isName) 
            {
                if (*tok != _M('='))
                {
                    QuickPair qPair;
                    qPair.SetKey(tok);
                
                    tok = toker.next();
                    if (tok && *tok == _M('='))
                    {
                        tok = toker.next();
                        qPair.SetVal(tok);

                        inName = false;
                    }
                    else 
                    {
                        qPair.SetVal(nullptr);
                        inName = (tok != nullptr);
                    }

                    fQuickTable->insert(qPair);
                } 
                else 
                {
                    isName = false;
                }
            } 
            else 
            {
                isName = true;
            }
        }

        // QuickPair owns the tok'd buffer now
        QuickPair::SetBuffer(toker.fString);
        toker.fString = nullptr;
    }
}

bool UserPropMgr::ICheckQuickEntry(const MCHAR *key, MSTR &value)
{
    QuickPair q;
    q.SetKey(key);
    auto it = fQuickTable->find(q);
    if (it != fQuickTable->end())
        return it->GetVal(value);
    return false;
}


MCHAR* UserPropMgr::QuickPair::fBuffer = nullptr;

void UserPropMgr::QuickPair::SetBuffer(MCHAR* buf)
{
    delete [] fBuffer;
    fBuffer = buf;
    }

uint32_t UserPropMgr::QuickPair::GetHash() const
{
    const MCHAR * k = fKey;
    int len = k ? _tcslen(k) : 0;
    int h;
    for (h=len; len--;) 
    {
        h = ((h<<5)^(h>>27))^tolower(*k++);
    }
    return h;
}

bool UserPropMgr::QuickPair::GetVal(MSTR& value) const
    {
    if (fKey)
        {
        value = fVal ? fVal : _M("");
        return true;
        }
            else
            {
        return false;
        }
    }

bool UserPropMgr::QuickPair::operator==(const QuickPair& other) const
{
    return _tcsicmp(fKey, other.fKey) == 0;
}
