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
#ifndef PL_COMPONENT_REG_H
#define PL_COMPONENT_REG_H

#include "plComponentMgr.h"

class TargetValidator;
extern TargetValidator gTargetValidator;
enum { kTargs };

class plComponentClassDesc : public ClassDesc2
{
public:
	int 		IsPublic()		{ return 0; }
	SClass_ID	SuperClassID()	{ return HELPER_CLASS_ID; }
	HINSTANCE	HInstance()		{ return hInstance; }
	BOOL		NeedsToSave()	{ return TRUE; }

	virtual bool IsAutoUI()		{ return false; }
	virtual bool IsObsolete()	{ return false; }
};

//
// This creates the class description for a component.
//
// classname - Name of a plComponent derived class (not a string)
// varname   - Name for a static instance of this desc that will be declared
// longname  - Public name for the modifier
// shortname - Internal name for the modifier.  No spaces.  Will be the prefix
//             for the Max name of components of this type.
// category  - Category this component will show up in in the helper panel.  This
//             is just a string, but the COMP_TYPE #defines below should be used
//             to promote sharing
// id        - Max ClassID for the component
//
// The ClassID will automatically be registered with the description manager.
//
#define CLASS_DESC(classname, varname, longname, shortname, category, id)	\
class classname##ClassDesc : public plComponentClassDesc					\
{																			\
	FUNC_CLASS_DESC(classname, longname, shortname, category, id)			\
	CONSTRUCT_DESC(classname)												\
};																			\
DECLARE_CLASS_DESC(classname, varname)

#define OBSOLETE_CLASS_DESC(classname, varname, longname, shortname, category, id)	\
class classname##ClassDesc : public plComponentClassDesc					\
{																			\
	FUNC_CLASS_DESC(classname, longname, shortname, category, id)			\
	CONSTRUCT_DESC(classname)												\
	virtual bool IsObsolete() { return true; }								\
};																			\
DECLARE_CLASS_DESC(classname, varname)

#define FUNC_CLASS_DESC(classname, longname, shortname, category, id)	\
public:																	\
	void*			Create(BOOL loading) { return new classname##; }	\
	const TCHAR*	ClassName()		{ return _T(longname); }			\
	Class_ID 		ClassID()		{ return id; }						\
	const TCHAR* 	Category()		{ return _T(category); }			\
	const TCHAR*	InternalName()	{ return _T(shortname); }

#define CONSTRUCT_DESC(classname) \
	classname##ClassDesc() { plComponentMgr::Inst().Register(this); }

#define DECLARE_CLASS_DESC(classname, varname)	\
static classname##ClassDesc varname;			\
TARG_BLOCK(classname, varname)

//
// OBSOLETE_CLASS macro
//	Usually, when you're making a component obsolete, you don't want to bother with any
//	class defs or paramBlock defs anymore, you just want them to all be empty. This
//	macro simplifies that process greatly: just delete everything about your class 
//	EXCEPT the CLASS_DESC line, then change CLASS_DESC to OBSOLETE_CLASS. This macro
//	will handle the rest for you! -mcn
//
#define OBSOLETE_CLASS( classname, descname, strname, sn, type, classid ) class classname : public plComponent { public:	classname##(); hsBool Convert( plMaxNode *n, plErrorMsg *e ) { return true; } }; \
							OBSOLETE_CLASS_DESC( classname, descname, strname, sn, type, classid ) \
							classname##::##classname() { fClassDesc = &##descname##; fClassDesc->MakeAutoParamBlocks(this); } \
							ParamBlockDesc2 classname##blk ( plComponent::kBlkComp, _T("##descname##"), 0, &##descname##, P_AUTO_CONSTRUCT, plComponent::kRefComp, end );

//
// Creates the targets paramblock for a component
//
#define TARG_BLOCK(classname, varname)						\
static ParamBlockDesc2 g##classname##TargsBlock				\
(															\
	plComponentBase::kBlkTargs, _T("targets"), 0, &varname,	\
	P_AUTO_CONSTRUCT, plComponentBase::kRefTargs,			\
															\
	kTargs, _T("targets"), TYPE_INODE_TAB, 0, 0, 0,			\
		end,												\
	end														\
);

// Component categories
#define COMP_TYPE_MISC		"Misc"
#define COMP_TYPE_TYPE		"Type"
#define COMP_TYPE_LOGIC		"Logic"
#define COMP_TYPE_PHYSICAL	"Physics"
#define COMP_TYPE_GRAPHICS	"Render"
#define COMP_TYPE_WATER		"Water"
#define COMP_TYPE_FOOTPRINT	"FootPrint"
#define COMP_TYPE_SHADOW	"Shadow"
#define COMP_TYPE_DISTRIBUTOR	"Distributor"
#define COMP_TYPE_AUDIO		"Audio"
#define COMP_TYPE_PARTICLE	"Particle System"
#define COMP_TYPE_AVATAR	"Avatar"
#define COMP_TYPE_DETECTOR	"Detector"
#define COMP_TYPE_VOLUME	"Region"
#define COMP_TYPE_IGNORE	"Ignore"
#define COMP_TYPE_PHYS_CONSTRAINTS "Physic Constraints"
#define COMP_TYPE_PHYS_TERRAINS "Navigables"
#define COMP_TYPE_GUI		"GUI"
#define COMP_TYPE_CAMERA	"Camera"
#define COMP_TYPE_SHADERS	"Vertex/Pixel Shaders"

#endif //PL_COMPONENT_REG_H