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
#include "plAutoUIBase.h"

#include "max.h"
#include "plComponentBase.h"
#include "plComponentReg.h"

class plAutoUIComp : public plAutoUIBase
{
public:
	plAutoUIComp(plAutoUIClassDesc *cd);

	/////////////////////////////////////////////////////////////////////////////////////
	// Get the value of a control.  Pass in the id and your 'this' pointer.
	//
	hsBool   GetCheckBox(Int16 id, plComponentBase *comp);
	hsScalar GetFloatSpinner(Int16 id, plComponentBase *comp);
	int      GetIntSpinner(Int16 id, plComponentBase *comp);
	TSTR     GetEditBox(Int16 id, plComponentBase *comp);
	INode*   GetPickNode(Int16 id, plComponentBase *comp, int idx);

	// Get the count for a parameter that takes an index
	int Count(Int16 id, plComponentBase *comp);

	/////////////////////////////////////////////////////////////////////////////////////
	// Max/internal functions
	//
	// Called by the ClassDesc.
	void BeginEditParams(IObjParam *ip, ReferenceMaker *obj, ULONG flags, Animatable *prev);
	void EndEditParams(IObjParam *ip, ReferenceMaker *obj, ULONG flags, Animatable *prev);
};

class plAutoUIClassDesc : public plComponentClassDesc
{
public:
	virtual bool IsAutoUI() { return true; }
	virtual bool IsObsolete()	{ return true; }

	plAutoUIComp *autoComp;
	void BeginEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev)
	{
		ClassDesc2::BeginEditParams(ip, obj, flags, prev);
		if (autoComp) autoComp->BeginEditParams(ip, obj, flags, prev);
	}
	void EndEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev)
	{
		if (autoComp) autoComp->EndEditParams(ip, obj, flags, prev);
		ClassDesc2::EndEditParams(ip, obj, flags, prev);
	}
	void CreateAutoRollup(IParamBlock2 *pb)
	{
		if (autoComp)
			autoComp->CreateAutoRollup(pb);
	}
	void DestroyAutoRollup()
	{
		if (autoComp)
			autoComp->DestroyAutoRollup();
	}
};

void plExternalComponentReg(ClassDesc *desc);

#define AUTO_CLASS_DESC(classname, varname, longname, shortname, category, id)	\
class classname##ClassDesc : public plAutoUIClassDesc							\
{																				\
	FUNC_CLASS_DESC(classname, longname, shortname, category, id)				\
	classname##ClassDesc() { plExternalComponentReg(this); }					\
};																				\
DECLARE_CLASS_DESC(classname, varname)

// 
void ReleaseGlobals();

//
// Categories
//
#define COMP_TYPE_KAHLO "Kahlo"
