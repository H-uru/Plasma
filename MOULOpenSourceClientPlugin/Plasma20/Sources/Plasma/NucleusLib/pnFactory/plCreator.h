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

#ifndef plCreator_inc
#define plCreator_inc

#include "plClassIndexMacros.h"
#include "plCreatableIndex.h"
#include "plFactory.h"

class plCreatable;

class plCreator
{
private:
protected:

public:
	plCreator() { }
	virtual ~plCreator() { }

	virtual plCreatable*	Create() const = 0;
	virtual UInt16			ClassIndex() = 0;
	virtual const char*		ClassName() const = 0;
	virtual hsBool			HasBaseClass(UInt16 hBase) = 0;

	friend class plFactory;
};

#define REGISTER_CREATABLE( plClassName )											\
																					\
class plClassName##__Creator : public plCreator										\
{																					\
public:																				\
	plClassName##__Creator()														\
	{																				\
		plFactory::Register( CLASS_INDEX_SCOPED(plClassName), this);				\
		plClassName::SetClassIndex(ClassIndex());									\
	}																				\
	virtual ~plClassName##__Creator()												\
	{																				\
		plFactory::UnRegister(CLASS_INDEX_SCOPED(plClassName), this);				\
	}																				\
																					\
	virtual hsBool HasBaseClass(UInt16 hBase) { return plClassName::HasBaseClass(hBase); }	\
																					\
	virtual UInt16 ClassIndex() { return CLASS_INDEX_SCOPED(plClassName); }			\
	virtual const char* ClassName() const { return #plClassName; }					\
																					\
	virtual plCreatable* Create() const { return TRACKED_NEW plClassName; }			\
																					\
};																					\
static plClassName##__Creator	static##plClassName##__Creator;						\
UInt16 plClassName::plClassName##ClassIndex = 0;									//

#define REGISTER_NONCREATABLE( plClassName )										\
																					\
class plClassName##__Creator : public plCreator										\
{																					\
public:																				\
	plClassName##__Creator()														\
	{																				\
		plFactory::Register( CLASS_INDEX_SCOPED(plClassName), this);				\
		plClassName::SetClassIndex(ClassIndex());									\
	}																				\
	virtual ~plClassName##__Creator()												\
	{																				\
		plFactory::UnRegister(CLASS_INDEX_SCOPED(plClassName), this);				\
	}																				\
																					\
	virtual hsBool HasBaseClass(UInt16 hBase) { return plClassName::HasBaseClass(hBase); }	\
																					\
	virtual UInt16 ClassIndex() { return CLASS_INDEX_SCOPED(plClassName); }			\
	virtual const char* ClassName() const { return #plClassName; }					\
																					\
	virtual plCreatable*		Create() const { return nil; }						\
																					\
};																					\
static plClassName##__Creator	static##plClassName##__Creator;						\
UInt16 plClassName::plClassName##ClassIndex = 0;									//

#define DECLARE_EXTERNAL_CREATABLE( plClassName )									\
																					\
class plClassName##__Creator : public plCreator										\
{																					\
public:																				\
	plClassName##__Creator()														\
	{																				\
	}																				\
	virtual ~plClassName##__Creator()												\
	{																				\
		plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);		\
	}																				\
	void Register()																	\
	{																				\
		plFactory::Register( EXTERN_CLASS_INDEX_SCOPED(plClassName), this);			\
		plClassName::SetClassIndex(ClassIndex());									\
	}																				\
	void UnRegister()																\
	{																				\
		plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);		\
	}																				\
																					\
	virtual hsBool HasBaseClass(UInt16 hBase) { return plClassName::HasBaseClass(hBase); }	\
																					\
	virtual UInt16 ClassIndex() { return EXTERN_CLASS_INDEX_SCOPED(plClassName); }	\
	virtual const char* ClassName() const { return #plClassName; }					\
																					\
	virtual plCreatable* Create() const { return TRACKED_NEW plClassName; }			\
																					\
};																					\
static plClassName##__Creator	static##plClassName##__Creator;						\
UInt16 plClassName::plClassName##ClassIndex = 0;									//

#define REGISTER_EXTERNAL_CREATABLE(plClassName)									\
static##plClassName##__Creator.Register();											//

#define UNREGISTER_EXTERNAL_CREATABLE(plClassName)									\
plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), &static##plClassName##__Creator);

#define REGISTER_EXTERNAL_NONCREATABLE( plClassName )								\
																					\
class plClassName##__Creator : public plCreator										\
{																					\
public:																				\
	plClassName##__Creator()														\
	{																				\
		plFactory::Register( EXTERN_CLASS_INDEX_SCOPED(plClassName), this);			\
		plClassName::SetClassIndex(ClassIndex());									\
	}																				\
	virtual ~plClassName##__Creator()												\
	{																				\
		plFactory::UnRegister(EXTERN_CLASS_INDEX_SCOPED(plClassName), this);		\
	}																				\
																					\
	virtual hsBool HasBaseClass(UInt16 hBase) { return plClassName::HasBaseClass(hBase); }	\
																					\
	virtual UInt16 ClassIndex() { return EXTERN_CLASS_INDEX_SCOPED(plClassName); }	\
	virtual const char* ClassName() const { return #plClassName; }					\
																					\
	virtual plCreatable*		Create() const { return nil; }						\
																					\
};																					\
static plClassName##__Creator	static##plClassName##__Creator;						\
UInt16 plClassName::plClassName##ClassIndex = 0;									//


#endif // plCreator_inc
