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
// UserPropMgr.cpp

#include "hsTypes.h"
#include "UserPropMgr.h"
#include "hsStringTokenizer.h"
#include "hsUtils.h"
#include "hsHashTable.h"

#define REFMSG_USERPROP  (REFMSG_USER + 1)

const UInt32 UserPropMgr::kQuickSize = 150001;//199999;

UserPropMgr gUserPropMgr(GetCOREInterface());

UserPropMgr::UserPropMgr(Interface *ip) : 
nm(0)
{
	this->ip = ip;

	fQuickTable = nil;
	fQuickNode = nil;

	vProps = false;
}

UserPropMgr::~UserPropMgr()
{
	CloseQuickTable();
}

void UserPropMgr::SetUserPropFlag(INode *node, const char *name, const BOOL setFlag, const Int32 hFlag) 
{
	if (setFlag) SetUserProp(node,name,NULL,hFlag);
	else ClearUserProp(node,name,hFlag);
}

void UserPropMgr::ClearUserPropALL(const char *name, const Int32 hFlag) 
{
	for (int i=0; i<GetSelNodeCount(); i++) 
	{
		ClearUserProp(GetSelNode(i),name,hFlag);
	}
}

void UserPropMgr::SelectUserPropFlagALL(INode *node, const char *name, const BOOL flag) {
	if (node) 
	{
		if (UserPropExists(node,name) == flag) ip->SelectNode(node,false);
	} else node = ip->GetRootNode();

	for (int i=0; i<node->NumberOfChildren(); i++) {
		SelectUserPropFlagALL(node->GetChildNode(i),name,flag);
	}
}


void UserPropMgr::DeSelectWithOut(const char *name, const char *value) {
	BOOL oldProps = vProps;
	vProps=false;
	TSTR val;
	INode *nodes[1];
	INodeTab nodeTab;
	for (int i=0; i<GetSelNodeCount(); i++) {
		if (value) {
			if (!(GetUserProp(GetSelNode(i),name,val) && !stricmp(val,value))) {
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

	TSTR buf;
	GetUserPropBuffer(node,buf);

	hsStringTokenizer toker(buf," \r\n");
	toker.ParseQuotes(TRUE);
	char *tok;
	TSTR name;
	bool isName = true;
	while (tok=toker.next()) {
		if (isName) {
			if (*tok != '=') {
				name = tok;
				tok = toker.next();
				if (tok && *tok == '=') {
					tok = toker.next();
				} else tok = NULL;
				DeSelectWithOut(name,tok);
			} else isName = false;
		} else {
			isName = true;
		}
	}


	TSTR undostr; undostr.printf("Select");
	theHold.Accept(undostr);

	ip->FreezeSelection();

	ip->RedrawViews(ip->GetTime());
}



int UserPropMgr::RecursiveCountAlike(INode *node, BOOL MatchAll) {
	int count=0;
	if (node) {
		if (!node->IsNodeHidden() && !node->IsFrozen() && IsAlike(node,MatchAll)) count++;
	} else node = ip->GetRootNode();

	for (int i=0; i<node->NumberOfChildren(); i++) {
		count += RecursiveCountAlike(node->GetChildNode(i),MatchAll);
	}
	return count;
}


int UserPropMgr::CountAlike(BOOL MatchAll) {
	return RecursiveCountAlike(NULL, MatchAll);
}

BOOL UserPropMgr::IsMatch(const char *val1, const char *val2) {
	if (!stricmp(val1,val2)) return true;
	hsStringTokenizer toker(val1," ,@");
	char *tok;

	while (tok=toker.next()) {
		hsStringTokenizer toker2(val2," ,@");
		BOOL found = false;
		char *tok2;
		while ((tok2=toker2.next()) && !found) {
			if (tok[0] >= '1' && tok[0] <= '0') {
				if (!stricmp(tok,tok2)) found = true;
			} else if (toker.HasMoreTokens()) {
				if (!stricmp(tok,tok2)) found = true;if (!stricmp(tok,tok2)) found = true;
			} else {
				if (!strnicmp(tok,tok2,strlen(tok))) found = true;
			}
		}
		if (!found) return false;
	}
	return true;
}


BOOL UserPropMgr::IsAlike(INode *node, BOOL MatchAll) {
	TSTR buf;
	GetUserPropBuffer(node,buf);

	BOOL oldProps = vProps;
	vProps=false;

	hsStringTokenizer toker(buf," \r\n");
	toker.ParseQuotes(TRUE);
	char *tok;
	TSTR name;
	TSTR value;
	TSTR tval;
	BOOL match = MatchAll;
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
					else value = "";
					tok = toker.next();
				} else value = "";
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
	TSTR buf;

	GetUserPropBuffer(node,buf);

	int numProps = 0;

	hsStringTokenizer toker(buf," \r\n");
	toker.ParseQuotes(TRUE);
	char *tok;
	bool isName = true;
	while (tok=toker.next()) {
		if (isName) {
			if (*tok != '=') {
				numProps++;
			} else isName = false;
		} else {
			isName = true;
		}
	}

	return numProps;
}

void UserPropMgr::GetUserPropBuffer(INode *node, TSTR &buf) {
	if (vProps) buf = vbuf;
	else if (node) node->GetUserPropBuffer(buf);
	else buf = "";
}

void UserPropMgr::SetUserPropBuffer(INode *node, const TSTR &buf) 
{
	// QuickTable invalidate
	if (node && node == fQuickNode)
	{
		fQuickNode = nil;
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

void UserPropMgr::SetUserPropFlagALL(const char *name, const BOOL setFlag, const Int32 hFlag) 
{
	for (int i=0; i<GetSelNodeCount();i++) 
	{
		SetUserPropFlag(GetSelNode(i),name,setFlag,hFlag);
	}
}
BOOL UserPropMgr::GetUserPropFlagALL(const char *name, BOOL &isSet, const Int32 hFlag)
 {
	isSet = UserPropMgr::UserPropExists(GetSelNode(0),name,hFlag);

	for (int i=0; i<GetSelNodeCount(); i++) {
		if (isSet != UserPropMgr::UserPropExists(GetSelNode(i),name,hFlag)) return FALSE;
	}
	return TRUE;
}

INode* UserPropMgr::GetAncestorIfNeeded(INode* node, const Int32 hFlag)
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


void UserPropMgr::ClearUserProp(INode *node, const char *name, const Int32 hFlag) 
{
	node = GetAncestorIfNeeded(node,hFlag);

	// QuickTable invalidate
	if (node && node == fQuickNode)
	{
		fQuickNode = nil;
	}

	TSTR buf;
	GetUserPropBuffer(node,buf);

	hsStringTokenizer toker(buf," \r\n");
	toker.ParseQuotes(TRUE);
	char *tok;
	bool isName = true;
	while (tok=toker.next()) 
	{
		if (isName) 
		{
			if (*tok != '=') 
			{
				if (!stricmp(tok,name)) 
				{
					char *tok2 = toker.next();
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
									buf.remove(tok-toker.fString,tok2-tok);
								}
								else
								{
									buf.remove(tok-toker.fString);
								}
							}
							else
							{
								buf.remove(tok-toker.fString);
							}
						}
						else 
						{
							buf.remove(tok-toker.fString,tok2-tok);
						}
					} 
					else
					{
						buf.remove(tok-toker.fString);
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

BOOL UserPropMgr::GetUserProp(INode *node, const char *name, TSTR &value, const Int32 hFlag)
{
	node = GetAncestorIfNeeded(node,hFlag);

	// QuickTable lookup
	if (node && fQuickTable)
	{
		if (node != fQuickNode)
			IBuildQuickTable(node);
		return ICheckQuickEntry(name,value);
	}

	TSTR buf;
	GetUserPropBuffer(node,buf);

	hsStringTokenizer toker(buf," \r\n");
	toker.ParseQuotes(TRUE);
	char *tok;
	bool isName = true;
	while (tok=toker.next()) 
	{
		if (isName)
		{
			if (*tok != '=')
			{
				if (!stricmp(tok,name))
				{
					tok = toker.next();
					if (tok && *tok == '=')
					{
						tok = toker.next();
						if (tok) value = tok;
						else value = "";
						return true;
					}
					else 
					{
						value = "";
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

void UserPropMgr::SetUserProp(INode *node, const char *name, const char *value, const Int32 hFlag) 
{
	node = GetAncestorIfNeeded(node,hFlag);

	// QuickTable invalidate
	if (node && node == fQuickNode)
	{
		fQuickNode = nil;
	}

	TSTR buf;
	GetUserPropBuffer(node,buf);

	hsStringTokenizer toker(buf," \r\n");
	toker.ParseQuotes(TRUE);
	char *tok;
	bool isName = true;
	while (tok=toker.next())
	{
		if (isName) 
		{
			if (*tok != '=') 
			{
				if (!stricmp(tok,name)) 
				{
					char *tok2 = toker.next();
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
									buf.remove(tok-toker.fString,tok2-tok);
								} 
								else
								{
									buf.remove(tok-toker.fString);
								}
							} 
							else
							{
								buf.remove(tok-toker.fString);
							}
						} 
						else
						{
							buf.remove(tok-toker.fString,tok2-tok);
						}
					} 
					else
					{
						buf.remove(tok-toker.fString);
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
	if (buf.last('\n') < buf.length()-1)
	{
		// better start with a separator
		buf += "\r\n";
	}
	buf += name;
	if (value && *value)
	{
		buf += " = ";
		if (strchr(value,' '))
		{
			buf += "\"";
			buf += value;
			buf += "\"";
		}
		else
		{
			buf += value;
		}
	}
	buf += "\r\n";
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


BOOL UserPropMgr::GetUserPropString(INode *node, const char *name, TSTR &value, const Int32 hFlag)
{
	 return GetUserProp(node,name,value,hFlag);
}
void UserPropMgr::SetUserPropString(INode *node, const char *name, const char *value, const Int32 hFlag) 
{
	SetUserProp(node,name,value,hFlag);
}
BOOL UserPropMgr::GetUserPropFloat(INode *node, const char *name, float &value, const Int32 hFlag)
{
	TSTR valStr;
	if (GetUserProp(node,name,valStr,hFlag)) 
	{
		value = (float)atof(valStr);
		return TRUE;
	}
	return FALSE;
}
void UserPropMgr::SetUserPropFloat(INode *node, const char *name, const float value, const Int32 hFlag) 
{
	char valStr[50];
	if (sprintf(valStr,"%g",value)) SetUserProp(node,name,valStr,hFlag);
}
BOOL UserPropMgr::GetUserPropInt(INode *node, const char *name, int &value, const Int32 hFlag)
{
	TSTR valStr;
	if (GetUserProp(node,name,valStr,hFlag)) {
		value = atoi(valStr);
		return TRUE;
	}
	return FALSE;
}
void UserPropMgr::SetUserPropInt(INode *node, const char *name, const int value, const Int32 hFlag) 
{
	char valStr[50];
	if (sprintf(valStr,"%d",value)) SetUserProp(node,name,valStr,hFlag);
}

BOOL UserPropMgr::UserPropExists(INode *node, const char *name, const Int32 hFlag) 
{
	TSTR value;
	return GetUserProp(node,name,value,hFlag);
}

BOOL UserPropMgr::GetUserPropStringList(INode *node, const char *name, int &num, TSTR list[]) {
	TSTR sdata;
	if (UserPropMgr::GetUserPropString(node,name,sdata)) {
		num=0;
		hsStringTokenizer toker(sdata,", ");
		char *tok;
		while ( tok = toker.next() ) {
			list[num] = tok;
			num++;
		}
		return true;
	} else return false;
}

BOOL UserPropMgr::GetUserPropIntList(INode *node, const char *name, int &num, int list[]) {
	TSTR sdata;
	if (UserPropMgr::GetUserPropString(node,name,sdata)) {
		num=0;
		hsStringTokenizer toker(sdata,", ");
		char *tok;
		while ( tok = toker.next() ) {
			list[num] = atoi(tok);
			num++;
		}
		return true;
	} else return false;
}

BOOL UserPropMgr::GetUserPropFloatList(INode *node, const char *name, int &num, float list[]) {
	TSTR sdata;
	if (UserPropMgr::GetUserPropString(node,name,sdata)) {
		num=0;
		hsStringTokenizer toker(sdata,", ");
		char *tok;
		while ( tok = toker.next() ) {
			list[num] = (float)atof(tok);
			num++;
		}
		return true;
	} else return false;
}

BOOL UserPropMgr::GetUserPropStringALL(const char *name, TSTR &value, const Int32 hFlag)
{
	BOOL propSet  = UserPropMgr::GetUserPropString(GetSelNode(0),name,value,hFlag);

	TSTR tvalue;
	int i=1;
	BOOL propMixed = FALSE;
	while (i < GetSelNodeCount() && !propMixed) {
		if (propSet ^ UserPropMgr::GetUserPropString(GetSelNode(i),name,tvalue,hFlag)) propMixed = TRUE;
		propMixed = (!(value == tvalue));
		i++;
	}

	return (!propMixed);
}
void UserPropMgr::SetUserPropStringALL(const char *name, const char *value, const Int32 hFlag) 
{
	for (int i=0; i<GetSelNodeCount(); i++) {
		UserPropMgr::SetUserPropString(GetSelNode(i),name,value,hFlag);
	}
}

BOOL UserPropMgr::GetUserPropStringListALL(const char *name, int &num, TSTR list[]) {
	TSTR val;
	GetUserPropStringList(GetSelNode(0),name,num,list);
	return GetUserPropStringALL(name,val);
}

BOOL UserPropMgr::GetUserPropIntListALL(const char *name, int &num, int *list) {
	TSTR val;
	GetUserPropIntList(GetSelNode(0),name,num,list);
	return GetUserPropStringALL(name,val);
}

BOOL UserPropMgr::GetUserPropFloatListALL(const char *name, int &num, float *list) {
	TSTR val;
	GetUserPropFloatList(GetSelNode(0),name,num,list);
	return GetUserPropStringALL(name,val);
}

BOOL UserPropMgr::GetNodeNameALL(TSTR &name) {
	if (vProps) name = vname;
	else if (ip->GetSelNodeCount() == 1) name = ip->GetSelNode(0)->GetName();
	else return false;

	return true;
}

void UserPropMgr::SetNodeNameALL(const char *name) {
	if (vProps) {
		vname = name;
	} else {
		if (ip->GetSelNodeCount() > 1) {
			TSTR uName;
			for (int i=0; i<ip->GetSelNodeCount(); i++) {
				uName = name;
				ip->MakeNameUnique(uName);
				ip->GetSelNode(i)->SetName(uName);
			}
		} else ip->GetSelNode(0)->SetName((char*)name);
	}
}


void UserPropMgr::LoadVirtualProps(BOOL reset) {
	if (reset)
	{
		vbuf = "";
		vname = "";
	}
	vProps = true;
}
void UserPropMgr::DestroyVirtualProps() {
	vProps = false;
}
BOOL UserPropMgr::IsVirtual() {
	return vProps;
}

int UserPropMgr::GetSelNodeCount() {
	if (vProps) return 1;
	else return ip->GetSelNodeCount();
}
INode *UserPropMgr::GetSelNode(int i) {
	if (vProps) return NULL;
	else return ip->GetSelNode(i);
}


void UserPropMgr::OpenQuickTable()
{
		if (!fQuickTable)
		{
		fQuickTable = TRACKED_NEW hsHashTable<QuickPair>(kQuickSize);
		}
	fQuickNode = nil;
}

void UserPropMgr::CloseQuickTable()
		{
#ifdef HS_DEBUGGING
	if (fQuickNode && fQuickTable)
			{
		char str[256];
		sprintf(str,"%d Hash Collisions reported\n",fQuickTable->CollisionCount());
		hsStatusMessage(str);
			}
#endif

	delete fQuickTable;
	fQuickTable = nil;
	fQuickNode = nil;
	QuickPair::SetBuffer(nil);
		}

void UserPropMgr::IBuildQuickTable(INode* node)
{
	if (fQuickTable && fQuickNode != node)
	{
		fQuickNode = node;

		// clear old QuickTable
		fQuickTable->clear();

		// build new one
		TSTR buf;
		GetUserPropBuffer(node,buf);

		hsStringTokenizer toker(buf," \r\n");
		toker.ParseQuotes(TRUE);

		char *tok;
		bool inName = false;
		bool isName = true;
		while ( inName || (tok=toker.next()) ) 
		{
			if (isName) 
			{
				if (*tok != '=') 
				{
					QuickPair qPair;
					qPair.SetKey(tok);
				
					tok = toker.next();
					if (tok && *tok == '=') 
					{
						tok = toker.next();
						qPair.SetVal(tok);

						inName = false;
					}
					else 
					{
						qPair.SetVal(nil);
						inName = (tok != 0);
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
		toker.fString = nil;
	}
}

BOOL UserPropMgr::ICheckQuickEntry(const char *key, TSTR &value)
{
	QuickPair q;
	q.SetKey(key);
	hsHashTableIterator<QuickPair> it = fQuickTable->find(q);
	return it->GetVal(value);
	}


char* UserPropMgr::QuickPair::fBuffer = nil;

void UserPropMgr::QuickPair::SetBuffer(char* buf)
{
	delete [] fBuffer;
	fBuffer = buf;
	}

UInt32 UserPropMgr::QuickPair::GetHash() const
{
	const char * k = fKey;
	int len = k ? strlen(k) : 0;
	int h;
	for (h=len; len--;) 
	{
		h = ((h<<5)^(h>>27))^tolower(*k++);
	}
	return h;
}

hsBool UserPropMgr::QuickPair::GetVal(TSTR& value)
	{
	if (fKey)
		{
		value = fVal ? fVal : "";
		return true;
		}
			else
			{
		return false;
		}
	}

bool UserPropMgr::QuickPair::operator==(const QuickPair& other) const
{
	return !_stricmp(fKey,other.fKey);
}
