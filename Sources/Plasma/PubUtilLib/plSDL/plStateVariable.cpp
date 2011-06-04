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
#include <float.h>
#include "hsStream.h"
#include "hsTimer.h"
#include "hsStlUtils.h"
#include "plSDL.h"

#include "../pnProduct/pnProduct.h"
#include "../pnFactory/plCreatable.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plKeyImp.h"
#include "../pnNetCommon/plNetApp.h"
#include "../pnNetCommon/pnNetCommon.h"

#include "../plResMgr/plResManager.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plUnifiedTime/plClientUnifiedTime.h"


#include "../plResMgr/plResManager.h"
#include "../plUnifiedTime/plClientUnifiedTime.h"


/*****************************************************************************
*
*	VALIDATE_WITH_FALSE_RETURN
*	Used in var getters and setters to validate incoming parameters and 
*	bail in a non-fatal way on error.
*
***/

#define VALIDATE_WITH_FALSE_RETURN(cond)	\
	if (!(cond)) {							\
		plSDLMgr::GetInstance()->GetNetApp()->DebugMsg("SDL var. Validation failed: "#cond);	\
		ASSERT(!(cond));					\
		return false;						\
	}										//




//
// generic creatable which holds any kind of creatable blob
//
class plSDLCreatableStub : public plCreatable
{
private:
	UInt16 fClassIndex;
public:
	void* fData;
	int	fDataLen;

	plSDLCreatableStub(UInt16 classIndex, int len) : fClassIndex(classIndex),fData(nil),fDataLen(len) {}
	~plSDLCreatableStub() { delete [] fData; }

	const char*			ClassName() const { return "SDLCreatable";	}
	UInt16				ClassIndex() const { return fClassIndex;	}

	void Read(hsStream* s, hsResMgr* mgr) { delete [] fData; fData = TRACKED_NEW char[fDataLen]; s->Read(fDataLen, fData); }
	void Write(hsStream* s, hsResMgr* mgr) { s->Write(fDataLen, fData); }
};

/////////////////////////////////////////////////////
// plStateVarNotificationInfo
/////////////////////////////////////////////////////

void plStateVarNotificationInfo::Read(hsStream* s, UInt32 readOptions)
{
	UInt8 saveFlags=s->ReadByte();	// unused
	char* hint=s->ReadSafeString();
	if (hint && !(readOptions & plSDL::kSkipNotificationInfo))
		fHintString = (const char*)hint;
	// we're done with it...
	delete [] hint;
}

void plStateVarNotificationInfo::Write(hsStream* s, UInt32 writeOptions) const
{
	UInt8 saveFlags=0;				// unused	
	s->WriteSwap(saveFlags);
	s->WriteSafeString(fHintString.c_str());
}

/////////////////////////////////////////////////////
// plStateVariable
/////////////////////////////////////////////////////
bool plStateVariable::ReadData(hsStream* s, float timeConvert, UInt32 readOptions)
{
	UInt8 saveFlags;
	s->ReadSwap(&saveFlags);
	if (saveFlags & plSDL::kHasNotificationInfo)
	{
		GetNotificationInfo().Read(s, readOptions);
	}
	return true;
}

bool plStateVariable::WriteData(hsStream* s, float timeConvert, UInt32 writeOptions) const
{
	bool writeNotificationInfo = ((writeOptions & plSDL::kSkipNotificationInfo)==0);

	UInt8 saveFlags=0;
	if (writeNotificationInfo)
		saveFlags |= plSDL::kHasNotificationInfo;

	s->WriteSwap(saveFlags);
	if (writeNotificationInfo)
	{
		GetNotificationInfo().Write(s, writeOptions);
	}
	return true;
}

/////////////////////////////////////////////////////
// plSimpleStateVariable
/////////////////////////////////////////////////////

void plSimpleStateVariable::IInit() 
{
	SetDirty(false);
	SetUsed(false);
	fBy=nil;
	fS=nil;
	fI=nil;
	fF=nil;
	fD=nil;
	fB=nil;
	fU=nil;
	fS32=nil;
	fC=nil;
	fT=nil;
	fTimeStamp.ToEpoch();	
}

//
// delete memory
//

#define DEALLOC(type, var)	\
	case type:	\
		delete [] var;	\
		break;

void plSimpleStateVariable::IDeAlloc()
{
	int cnt = fVar.GetAtomicCount()*fVar.GetCount();
	int type = fVar.GetAtomicType();
	switch (type)
	{
	DEALLOC (plVarDescriptor::kInt, fI)
	DEALLOC (plVarDescriptor::kAgeTimeOfDay, fF)
	DEALLOC (plVarDescriptor::kShort, fS)
	DEALLOC (plVarDescriptor::kByte, fBy)
	DEALLOC (plVarDescriptor::kFloat, fF)
	DEALLOC (plVarDescriptor::kTime, fT)
	DEALLOC (plVarDescriptor::kDouble, fD)
	DEALLOC (plVarDescriptor::kBool, fB)
	DEALLOC (plVarDescriptor::kKey, fU)
	DEALLOC (plVarDescriptor::kString32, fS32)
	case plVarDescriptor::kCreatable:
		{
			if(fC)
			{
				int i;
				// delete each creatable
				for(i=0;i<cnt; i++)
					delete fC[i];
				// delete creatable array
				delete [] fC;
			}
		}
		break;
	default:
		hsAssert(false, xtl::format("undefined atomic type:%d var:%s cnt:%d", 
			type, GetName() ? GetName() : "?", GetCount()).c_str());
		break;
	};

}

//
// alloc memory
//

#define SDLALLOC(typeName, type, var)	\
	case typeName:	\
		var = TRACKED_NEW type[cnt];	\
		break;

void plSimpleStateVariable::Alloc(int listSize)
{
	if (listSize != -1)
		fVar.SetCount(listSize);
	
	IDeAlloc();

	IInit();
	
	int cnt = fVar.GetAtomicCount()*fVar.GetCount();
	if (cnt)
	{
		switch (fVar.GetAtomicType())
		{
		SDLALLOC(plVarDescriptor::kInt, int, fI)
		SDLALLOC(plVarDescriptor::kAgeTimeOfDay, float, fF)
		SDLALLOC(plVarDescriptor::kByte, byte, fBy)
		SDLALLOC(plVarDescriptor::kShort, short, fS)
		SDLALLOC(plVarDescriptor::kFloat, float, fF)
		SDLALLOC(plVarDescriptor::kDouble, double, fD)
		SDLALLOC(plVarDescriptor::kBool, bool, fB)
		SDLALLOC(plVarDescriptor::kCreatable, plCreatable*, fC)
		case plVarDescriptor::kTime:
			fT = TRACKED_NEW plClientUnifiedTime[cnt];
			break;
		case plVarDescriptor::kKey:
			fU = TRACKED_NEW plUoid[cnt];
			break;
		case plVarDescriptor::kString32:
			fS32 = TRACKED_NEW plVarDescriptor::String32[cnt];
			break;
		default:
			hsAssert(false, "undefined atomic type");
			break;
		};
	}

	Reset();
}

#define RESET(typeName, type, var)	\
	case typeName:	\
		for(i=0;i<cnt;i++)	\
			var[i]=0;	\
		break;
void plSimpleStateVariable::Reset()
{
	int i, cnt = fVar.GetAtomicCount()*fVar.GetCount();
	if (cnt)
	{
		switch (fVar.GetAtomicType())
		{
		RESET(plVarDescriptor::kInt, int, fI)
		RESET(plVarDescriptor::kAgeTimeOfDay, float, fF)
		RESET(plVarDescriptor::kByte, byte, fBy)
		RESET(plVarDescriptor::kShort, short, fS)
		RESET(plVarDescriptor::kFloat, float, fF)
		RESET(plVarDescriptor::kDouble, double, fD)
		RESET(plVarDescriptor::kBool, bool, fB)
		RESET(plVarDescriptor::kCreatable, plCreatable*, fC)
		case plVarDescriptor::kTime:
			break;
		case plVarDescriptor::kKey:
			break;
		case plVarDescriptor::kString32:
			for(i=0;i<cnt;i++)	
				*fS32[i]=0;	
			break;
		default:
			hsAssert(false, "undefined atomic type");
			break;
		};
	}
}

//
// Copy the descriptor settings and allocate list
//
void plSimpleStateVariable::CopyFrom(plVarDescriptor* v)
{
	if (v)
	{
		plSimpleVarDescriptor* simV=(plSimpleVarDescriptor*)v;
		if (simV)
			fVar.CopyFrom(simV);
		else
			fVar.CopyFrom(v);	// copy base class

		Alloc();	
	}
}

void plSimpleStateVariable::TimeStamp( const plUnifiedTime & ut/*=plUnifiedTime::GetCurrentTime()*/ )
{
	fTimeStamp = ut;
}

//
// Set value from string.  Used to set default values which are specified as strings.
//
bool plSimpleStateVariable::SetFromString(const char* valueConst, int idx, bool timeStampNow)
{
	if (!valueConst)
		return false;

	std::string value = valueConst;
	plVarDescriptor::Type type=fVar.GetAtomicType();
	switch(type)
	{
	case plVarDescriptor::kAgeTimeOfDay:
	case plVarDescriptor::kTime:
	case plVarDescriptor::kDouble:
	case plVarDescriptor::kFloat:
	case plVarDescriptor::kInt:
	case plVarDescriptor::kShort:
	case plVarDescriptor::kByte:
		{	
			// handles value in the form "(i,j,k)" for vectors
			static char	seps[] = "( ,)";
			char* ptr = strtok( (char*)value.c_str(), seps );
			int i=idx*fVar.GetAtomicCount();
			while (ptr)
			{
				if ((type==plVarDescriptor::kInt) && fI)
					fI[i++] = atoi(ptr);
				else if (type==plVarDescriptor::kShort && fS)
					fS[i++] = (short)atoi(ptr);
				else if (type==plVarDescriptor::kByte && fBy)
					fBy[i++] = (byte)atoi(ptr);
				else if ( (type==plVarDescriptor::kFloat || type==plVarDescriptor::kAgeTimeOfDay) && fF)
					fF[i++] = (float)atof(ptr);
				else if ( (type==plVarDescriptor::kDouble || type==plVarDescriptor::kTime) && fD)
					fD[i++] = atof(ptr);
				ptr = strtok(nil, seps);
			}
		}
		break;
	case plVarDescriptor::kBool:
		{	
			// handles value in the form "(i,j,k)" for things like vectors
			static char	seps[] = "( ,)";
			char* ptr = strtok( (char*)value.c_str(), seps );
			int i=idx*fVar.GetAtomicCount();
			while (ptr)
			{
				if (!stricmp(ptr, "true"))
					fB[i++]=true;
				else
				if (!stricmp(ptr, "false"))
					fB[i++]=false;
				else
					fB[i++] = (atoi(ptr) != 0);
				ptr = strtok(nil, seps);
			}
		}
		break;
	case plVarDescriptor::kString32:
		{	
			// handles value in the form "(i,j,k)" for things like vectors
			static char	seps[] = "( ,)";
			char* ptr = strtok( (char*)value.c_str(), seps );
			int i=idx*fVar.GetAtomicCount();
			while (ptr)
			{
				hsStrncpy(fS32[i++], ptr, 32);
				ptr = strtok(nil, seps);
			}
		}
		break;
	default:
		return false;	// err
	}	

	IVarSet(timeStampNow);
	return true;	// ok
}

//
// Called when a var has been set with a value
//
void plSimpleStateVariable::IVarSet(bool timeStampNow/*=true*/)
{
	if (timeStampNow)
		TimeStamp();
	SetDirty(true); 
	SetUsed(true);
}

//
// Get value as string.
//
char* plSimpleStateVariable::GetAsString(int idx) const
{
	int j;
	std::string str;
	if (fVar.GetAtomicCount()>1)
		str = str + "(";

	plVarDescriptor::Type type=fVar.GetAtomicType();
	switch(type)
	{
	case plVarDescriptor::kAgeTimeOfDay:
	case plVarDescriptor::kTime:
	case plVarDescriptor::kDouble:
	case plVarDescriptor::kFloat:
	case plVarDescriptor::kInt:
	case plVarDescriptor::kByte:
	case plVarDescriptor::kShort:
		{	
			// handles value in the form "(i,j,k)" for vectors
			int i=idx*fVar.GetAtomicCount();
			for(j=0;j<fVar.GetAtomicCount();j++)
			{
				if (type==plVarDescriptor::kInt)
					str.append( xtl::format( "%d", fI[i++]) );
				else if (type==plVarDescriptor::kShort)
					str.append( xtl::format( "%d", fS[i++]) );
				else if (type==plVarDescriptor::kByte)
					str.append( xtl::format( "%d", fBy[i++]) );
				else if (type==plVarDescriptor::kFloat  || type==plVarDescriptor::kAgeTimeOfDay)
					str.append( xtl::format( "%.3f", fF[i++]) );
				else if (type==plVarDescriptor::kDouble)
					str.append( xtl::format( "%.3f", fD[i++]) );
				else if (type==plVarDescriptor::kTime)
				{
					double tmp;
					Get(&tmp, i++);
					str.append( xtl::format( "%.3f", tmp) );
				}

				if (j==fVar.GetAtomicCount()-1)
				{
					if (j)
						str += ")";
				}
				else
					str += ",";
			}
		}
		break;
	case plVarDescriptor::kBool:
		{	
			// handles value in the form "(i,j,k)" for things like vectors
			int i=idx*fVar.GetAtomicCount();
			for(j=0;j<fVar.GetAtomicCount();j++)
			{
				str.append( xtl::format( "%s", fB[i++] ? "true" : "false") );

				if (j==fVar.GetAtomicCount()-1)
				{
					if (j)
						str += ")";
				}
				else
					str += ",";
			}
		}
		break;
	case plVarDescriptor::kString32:
		{	
			// handles value in the form "(i,j,k)" for things like vectors
			int i=idx*fVar.GetAtomicCount();
			for(j=0;j<fVar.GetAtomicCount();j++)
			{
				str.append( xtl::format( "%s", fS32[i++]) );

				if (j==fVar.GetAtomicCount()-1)
				{
					if (j)
						str += ")";
				}
				else
					str += ",";
			}
		}
		break;
	default:
		{	
			// handles value in the form "(i,j,k)" for things like vectors
			int i=idx*fVar.GetAtomicCount();
			for(j=0;j<fVar.GetAtomicCount();j++)
			{
				str.append( xtl::format( "%s", "other") );

				if (j==fVar.GetAtomicCount()-1)
				{
					if (j)
						str += ")";
				}
				else
					str += ",";
			}
		}
		break;
	}	

	return hsStrcpy(str.c_str());
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromRGB(plVarDescriptor::Type newType)
{
	switch(newType)
	{
	case plVarDescriptor::kRGBA:
		{
			// rgb to rgba
			int i,j;
			float* newF = TRACKED_NEW float[fVar.GetCount()*4];		// make more space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy with alpha=0 by default
				for(i=0;i<3;i++)
					newF[j*4+i] = fF[j*fVar.GetAtomicCount()+i];
				newF[j*4+3] = 0;
			}
			delete [] fF;	// delete old
			fF = newF;		// use new
		}
		break;
	case plVarDescriptor::kRGBA8:
		{
			// rgb to rgba8
			int i,j;
			byte * newB = TRACKED_NEW byte [fVar.GetCount()*4];		// make more space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy with alpha=0 by default
				for(i=0;i<3;i++)
					newB[j*4+i] = byte(fF[j*fVar.GetAtomicCount()+i]*255+.5);
				newB[j*4+3] = 0;
			}
			delete [] fF;	// delete old
			fBy = newB;		// use new
		}
		break;
	case plVarDescriptor::kRGB8:
		{
			// rgb to rgb8
			int i,j;
			byte * newB = TRACKED_NEW byte [fVar.GetCount()*3];		// make space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy with alpha=0 by default
				for(i=0;i<3;i++)
					newB[j*3+i] = byte(fF[j*fVar.GetAtomicCount()+i]*255+.5);
			}
			delete [] fF;	// delete old
			fBy = newB;		// use new
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

bool plSimpleStateVariable::IConvertFromRGB8(plVarDescriptor::Type newType)
{
	switch(newType)
	{
	case plVarDescriptor::kRGBA:
		{
			// rgb8 to rgba
			int i,j;
			float* newF = TRACKED_NEW float[fVar.GetCount()*4];		// make more space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy with alpha=0 by default
				for(i=0;i<3;i++)
					newF[j*4+i] = fBy[j*fVar.GetAtomicCount()+i]/255.f;
				newF[j*4+3] = 0;
			}
			delete [] fBy;	// delete old
			fF = newF;		// use new
		}
		break;
	case plVarDescriptor::kRGB:
		{
			// rgb8 to rgb
			int i,j;
			float* newF = TRACKED_NEW float[fVar.GetCount()*3];		// make more space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy with alpha=0 by default
				for(i=0;i<3;i++)
					newF[j*3+i] = fBy[j*fVar.GetAtomicCount()+i]/255.f;
			}
			delete [] fBy;	// delete old
			fF = newF;		// use new
		}
		break;
	case plVarDescriptor::kRGBA8:
		{
			// rgb8 to rgba8
			int i,j;
			byte * newB = TRACKED_NEW byte [fVar.GetCount()*4];		// make more space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy with alpha=0 by default
				for(i=0;i<3;i++)
					newB[j*4+i] = fBy[j*fVar.GetAtomicCount()+i];
				newB[j*4+3] = 0;
			}
			delete [] fBy;	// delete old
			fBy = newB;		// use new
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromRGBA(plVarDescriptor::Type newType)
{
	switch(newType)
	{
	case plVarDescriptor::kRGB:
		{
			// rgba to rgb
			int i,j;
			float* newF = TRACKED_NEW float[fVar.GetCount()*3];		// make less space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy and ignore alpha
				for(i=0;i<3;i++)
					newF[j*3+i] = fF[j*fVar.GetAtomicCount()+i];
			}
			delete [] fF;	// delete old
			fF = newF;		// use new
		}
		break;
	case plVarDescriptor::kRGB8:
		{
			// rgba to rgb8
			int i,j;
			byte* newB = TRACKED_NEW byte[fVar.GetCount()*3];		// make less space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy and ignore alpha
				for(i=0;i<3;i++)
					newB[j*3+i] = byte(fF[j*fVar.GetAtomicCount()+i]*255+.5);
			}
			delete [] fF;	// delete old
			fBy = newB;		// use new
		}
		break;
	case plVarDescriptor::kRGBA8:
		{
			// rgba to rgba8
			int i,j;
			byte* newBy = TRACKED_NEW byte [fVar.GetCount()*4];		// make less space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy and ignore alpha
				for(i=0;i<4;i++)
					newBy[j*4+i] = byte(fF[j*fVar.GetAtomicCount()+i]*255+.5);
			}
			delete [] fF;	// delete old
			fBy = newBy;		// use new
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromRGBA8(plVarDescriptor::Type newType)
{
	switch(newType)
	{
	case plVarDescriptor::kRGB:
		{
			// rgba8 to rgb
			int i,j;
			float* newF = TRACKED_NEW float[fVar.GetCount()*3];		// make less space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy and ignore alpha
				for(i=0;i<3;i++)
					newF[j*3+i] = fBy[j*fVar.GetAtomicCount()+i]/255.f;
			}
			delete [] fBy;	// delete old
			fF = newF;		// use new
		}
		break;
	case plVarDescriptor::kRGB8:
		{
			// rgba8 to rgb8
			int i,j;
			byte* newB = TRACKED_NEW byte[fVar.GetCount()*3];		// make less space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy and ignore alpha
				for(i=0;i<3;i++)
					newB[j*3+i] = fBy[j*fVar.GetAtomicCount()+i];
			}
			delete [] fBy;	// delete old
			fBy = newB;		// use new
		}
		break;
	case plVarDescriptor::kRGBA:
		{
			// rgba8 to rgba
			int i,j;
			float* newF = TRACKED_NEW float[fVar.GetCount()*4];		// make less space
			for(j=0;j<fVar.GetCount(); j++)
			{
				// recopy and ignore alpha
				for(i=0;i<4;i++)
					newF[j*4+i] = fBy[j*fVar.GetAtomicCount()+i]/255.f;
			}
			delete [] fBy;	// delete old
			fF = newF;		// use new
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromInt(plVarDescriptor::Type newType)
{
	int j;
	switch(newType)
	{
	case plVarDescriptor::kFloat:
		{
			// int to float
			float* newF = TRACKED_NEW float[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newF[j] = (float)(fI[j]);
			delete [] fI;
			fF = newF;
		}
		break;
	case plVarDescriptor::kShort:
		{
			// int to short
			short* newS = TRACKED_NEW short[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newS[j] = short(fI[j]);
			delete [] fI;
			fS = newS;
		}
		break;
	case plVarDescriptor::kByte:
		{
			// int to byte
			byte* newBy = TRACKED_NEW byte[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newBy[j] = byte(fI[j]);
			delete [] fI;
			fBy = newBy;
		}
		break;
	case plVarDescriptor::kDouble:
		{
			// int to double
			double * newD = TRACKED_NEW double[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newD[j] = fI[j];
			delete [] fI;
			fD = newD;
		}
		break;
	case plVarDescriptor::kBool:
		{
			// int to bool
			bool * newB = TRACKED_NEW bool[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newB[j] = (fI[j]!=0);
			delete [] fI;
			fB = newB;
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromShort(plVarDescriptor::Type newType)
{
	int j;
	switch(newType)
	{
	case plVarDescriptor::kFloat:
		{
			float* newF = TRACKED_NEW float[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newF[j] = fS[j];
			delete [] fS;
			fF = newF;
		}
		break;
	case plVarDescriptor::kInt:
		{
			int* newI = TRACKED_NEW int[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newI[j] = short(fS[j]);
			delete [] fS;
			fI = newI;
		}
	case plVarDescriptor::kByte:
		{
			byte* newBy = TRACKED_NEW byte[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newBy[j] = byte(fS[j]);
			delete [] fS;
			fBy = newBy;
		}
		break;
	case plVarDescriptor::kDouble:
		{
			double * newD = TRACKED_NEW double[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newD[j] = fS[j];
			delete [] fS;
			fD = newD;
		}
		break;
	case plVarDescriptor::kBool:
		{
			bool * newB = TRACKED_NEW bool[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newB[j] = (fS[j]!=0);
			delete [] fS;
			fB = newB;
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromByte(plVarDescriptor::Type newType)
{
	int j;
	switch(newType)
	{
	case plVarDescriptor::kFloat:
		{
			float* newF = TRACKED_NEW float[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newF[j] = fBy[j];
			delete [] fBy;
			fF = newF;
		}
		break;
	case plVarDescriptor::kInt:
		{
			int* newI = TRACKED_NEW int[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newI[j] = short(fBy[j]);
			delete [] fBy;
			fI = newI;
		}
	case plVarDescriptor::kShort:
		{
			short* newS = TRACKED_NEW short[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newS[j] = fBy[j];
			delete [] fBy;
			fS = newS;
		}
		break;
	case plVarDescriptor::kDouble:
		{
			double * newD = TRACKED_NEW double[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newD[j] = fBy[j];
			delete [] fBy;
			fD = newD;
		}
		break;
	case plVarDescriptor::kBool:
		{
			bool * newB = TRACKED_NEW bool[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newB[j] = (fBy[j]!=0);
			delete [] fBy;
			fB = newB;
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromFloat(plVarDescriptor::Type newType)
{
	int j;
	switch(newType)
	{
	case plVarDescriptor::kInt:
		{
			int* newI = TRACKED_NEW int[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newI[j] = (int)(fF[j]+.5f);	// round to nearest int
			delete [] fF;
			fI = newI;
		}
		break;
	case plVarDescriptor::kShort:
		{
			short* newS = TRACKED_NEW short[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newS[j] = (short)(fF[j]+.5f);	// round to nearest int
			delete [] fF;
			fS = newS;
		}
		break;
	case plVarDescriptor::kByte:
		{
			byte* newBy = TRACKED_NEW byte[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newBy[j] = (byte)(fF[j]+.5f);	// round to nearest int
			delete [] fF;
			fBy = newBy;
		}
		break;
	case plVarDescriptor::kDouble:
		{
			double* newD = TRACKED_NEW double[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newD[j] = fF[j];
			delete [] fF;
			fD = newD;
		}
		break;
	case plVarDescriptor::kBool:
		{
			bool* newB = TRACKED_NEW bool[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newB[j] = (fF[j]!=0);
			delete [] fF;
			fB = newB;
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromDouble(plVarDescriptor::Type newType)
{
	int j;
	switch(newType)
	{
	case plVarDescriptor::kInt:
		{
			int* newI = TRACKED_NEW int[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newI[j] = (int)(fD[j]+.5f);	// round to nearest int
			delete [] fD;
			fI = newI;
		}
		break;
	case plVarDescriptor::kShort:
		{
			short* newS = TRACKED_NEW short[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newS[j] = (short)(fD[j]+.5f);	// round to nearest int
			delete [] fD;
			fS = newS;
		}
		break;
	case plVarDescriptor::kByte:
		{
			byte* newBy = TRACKED_NEW byte[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newBy[j] = (byte)(fD[j]+.5f);	// round to nearest int
			delete [] fD;
			fBy = newBy;
		}
		break;
	case plVarDescriptor::kFloat:
		{
			float* newF = TRACKED_NEW float[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newF[j] = (float)(fD[j]);
			delete [] fD;
			fF = newF;
		}
		break;
	case plVarDescriptor::kBool:
		{
			bool* newB = TRACKED_NEW bool[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newB[j] = (fD[j]!=0);
			delete [] fD;
			fB = newB;
		}
		break;
	default:
		return false;	// err
	}
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromBool(plVarDescriptor::Type newType)
{
	int j;
	switch(newType)
	{
	case plVarDescriptor::kInt:
		{
			int* newI = TRACKED_NEW int[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newI[j] = (fB[j] == true ? 1 : 0);
			delete [] fB;
			fI = newI;
		}
		break;
	case plVarDescriptor::kShort:
		{
			short* newS = TRACKED_NEW short[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newS[j] = (fB[j] == true ? 1 : 0);
			delete [] fB;
			fS = newS;
		}
		break;
	case plVarDescriptor::kByte:
		{
			byte* newBy = TRACKED_NEW byte[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newBy[j] = (fB[j] == true ? 1 : 0);
			delete [] fB;
			fBy = newBy;
		}
		break;
	case plVarDescriptor::kFloat:
		{
			float* newF = TRACKED_NEW float[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newF[j] = (fB[j] == true ? 1.f : 0.f);
			delete [] fB;
			fF = newF;
		}
		break;
	case plVarDescriptor::kDouble:
		{
			double* newD= TRACKED_NEW double[fVar.GetCount()];
			for(j=0;j<fVar.GetCount(); j++)
				newD[j] = (fB[j] == true ? 1.f : 0.f);
			delete [] fB;
			fD = newD;
		}
		break;
	default:
		return false;	// err
	}
	
	return true;
}

//
// return false on err
//
bool plSimpleStateVariable::IConvertFromString(plVarDescriptor::Type newType)
{
	int j;
	switch(newType)
	{
		case plVarDescriptor::kBool:
		// string to bool
			for(j=0;j<fVar.GetCount(); j++)
			{
				if (!stricmp(fS32[j], "true") || !stricmp(fS32[j], "1"))
					fB[j]=true;
				else
					if (!stricmp(fS32[j], "false") || !stricmp(fS32[j], "0"))
						fB[j]=false;
					else
						return false;	// err
			}
		break;
			
		case plVarDescriptor::kInt:
		// string to int
			for(j=0;j<fVar.GetCount(); j++)
			{
				fI[j] = atoi(fS32[j]);
			}
		break;
			
		case plVarDescriptor::kFloat:
		// string to float
			for(j=0;j<fVar.GetCount(); j++)
			{
				fF[j] = (float) atof(fS32[j]);
			}
		break;

		default:
			return false;	// err
	}		
				
	return true;
}

//
// Try to convert my value to another type.
// return false on err.
// called when a data record is read in
//
bool plSimpleStateVariable::ConvertTo(plSimpleVarDescriptor* toVar, bool force )
{
	// NOTE: 'force' has no meaning here really, so it is not inforced.

	plVarDescriptor::Type newType = toVar->GetType(); 

	int cnt = toVar->GetCount() ? toVar->GetCount() : fVar.GetCount();

	if (cnt > fVar.GetCount()) {
		#if BUILD_TYPE == BUILD_TYPE_DEV
			FATAL("SDL Convert: array size increased, conversion loses data");
		#endif
		// Reallocate new memory (destroys existing variable state)
		Alloc(cnt);
		// match types now
		fVar.SetCount(cnt);
		fVar.SetType(toVar->GetType());
		fVar.SetAtomicType(toVar->GetAtomicType());
		return true;
	}

	fVar.SetCount(cnt);	// convert count

	// types are already the same, done.
	if (fVar.GetType()==newType )
		return true;

	hsLogEntry( plNetApp::StaticDebugMsg( "SSV(%p) converting %s from %s to %s",
		this, fVar.GetName(), fVar.GetTypeString(), toVar->GetTypeString() ) );

	switch(fVar.GetType())	// original type
	{
		// FROM RGB
	case plVarDescriptor::kRGB:
		if (!IConvertFromRGB(newType))
			return false;
		break;

		// FROM RGBA
	case plVarDescriptor::kRGBA:
		if (!IConvertFromRGBA(newType))
			return false;
		break;

		// FROM RGB8
	case plVarDescriptor::kRGB8:
		if (!IConvertFromRGB8(newType))
			return false;
		break;

		// FROM RGBA8
	case plVarDescriptor::kRGBA8:
		if (!IConvertFromRGBA8(newType))
			return false;
		break;

		// FROM INT
	case plVarDescriptor::kInt:
		if (!IConvertFromInt(newType))
			return false;
		break;

		// FROM SHORT
	case plVarDescriptor::kShort:
		if (!IConvertFromShort(newType))
			return false;
		break;

		// FROM Byte
	case plVarDescriptor::kByte:
		if (!IConvertFromByte(newType))
			return false;
		break;

		// FROM FLOAT
	case plVarDescriptor::kFloat:
		if (!IConvertFromFloat(newType))
			return false;
		break;

		// FROM DOUBLE
	case plVarDescriptor::kDouble:
		if (!IConvertFromDouble(newType))
			return false;
		break;

		// FROM BOOL
	case plVarDescriptor::kBool:
		if (!IConvertFromBool(newType))
			return false;
		break;

		// FROM STRING32
	case plVarDescriptor::kString32:
		if (!IConvertFromString(newType))
			return false;
		break;

	default:
		return false;	// err	
	}	

	// match types now
	fVar.SetType(toVar->GetType());
	fVar.SetAtomicType(toVar->GetAtomicType());

	return true;	// ok
}

/////////////////////////////////////////////////////////////
// SETTERS
/////////////////////////////////////////////////////////////

bool plSimpleStateVariable::Set(float v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kAgeTimeOfDay)
	{
		hsAssert(false, "AgeTime variables are read-only, can't set");
	}
	else
	if (fVar.GetType()==plVarDescriptor::kFloat)
	{
		fF[idx]=v;
		IVarSet();
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

bool plSimpleStateVariable::Set(double v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kDouble)
	{
		fD[idx]=v;
		IVarSet();
		return true;
	}

	if (fVar.GetType()==plVarDescriptor::kTime)
	{	// convert from, 
		fT[idx].SetFromGameTime(v, hsTimer::GetSysSeconds());		
		IVarSet();
		return true;
	}

	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

// floatvector
bool plSimpleStateVariable::Set(float* v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kAgeTimeOfDay)
	{
		hsAssert(false, "AgeTime variables are read-only, can't set");
	}
	else	
	if (fVar.GetAtomicType()==plVarDescriptor::kFloat)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			fF[cnt+i]=v[i];
		IVarSet();
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

// bytevector
bool plSimpleStateVariable::Set(byte* v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kByte)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			fBy[cnt+i]=v[i];
		IVarSet();
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

// 
bool plSimpleStateVariable::Set(double* v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kDouble)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			fD[cnt+i]=v[i];
		IVarSet();
		return true;
	}

	if (fVar.GetAtomicType()==plVarDescriptor::kTime)
	{
		double secs=hsTimer::GetSysSeconds();
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			fT[cnt+i].SetFromGameTime(v[i], secs);		
		IVarSet();
		return true;
	}

	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

bool plSimpleStateVariable::Set(int v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kInt)
	{
		fI[idx]=v;
		IVarSet();
		return true;
	}
	else
	if (fVar.GetType()==plVarDescriptor::kBool)
		return Set((bool)(v?true:false), idx);	// since 'true' is coming in as an int not bool
	else
	if (fVar.GetType()==plVarDescriptor::kShort)
		return Set((short)v, idx);
	else
	if (fVar.GetType()==plVarDescriptor::kByte)
		return Set((byte)v, idx);

	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

bool plSimpleStateVariable::Set(short v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kShort)
	{
		fS[idx]=v;
		IVarSet();
		return true;
	}
	else
	if (fVar.GetType()==plVarDescriptor::kInt)
		return Set((int)v, idx);
	else
	if (fVar.GetType()==plVarDescriptor::kByte)
		return Set((byte)v, idx);

	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

bool plSimpleStateVariable::Set(byte v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kByte)
	{
		fBy[idx]=v;
		IVarSet();
		return true;
	}
	else
	if (fVar.GetType()==plVarDescriptor::kBool)
		return Set((bool)(v?true:false), idx);
	else
	if (fVar.GetType()==plVarDescriptor::kInt)
		return Set((int)v, idx);
	else
	if (fVar.GetType()==plVarDescriptor::kShort)
		return Set((short)v, idx);

	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

bool plSimpleStateVariable::Set(const char* v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (v && fVar.GetType()==plVarDescriptor::kString32)
	{
		hsAssert(hsStrlen(v)<32, "string length overflow");
		hsStrncpy(fS32[idx], v, 32);
		IVarSet();
		return true;
	}
	hsAssert(false, v ? "passing wrong value type to SDL variable" : "trying to set nil string");
	return false;
}

bool plSimpleStateVariable::Set(bool v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kBool)
	{
		fB[idx]=v;
		IVarSet();
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

bool plSimpleStateVariable::Set(const plKey& v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kKey)
	{
		if(v) 
		{
			fU[idx] = v->GetUoid();
		} 
		else 
		{
			fU[idx] = plUoid();
		}
		IVarSet();
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

bool plSimpleStateVariable::Set(plCreatable* v, int idx)
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kCreatable)
	{
		// copy creatable via stream
		hsRAMStream stream;
		if(v)
		{
			hsgResMgr::ResMgr()->WriteCreatable(&stream, v);
			stream.Rewind();
		}
		plCreatable* copy = v ? hsgResMgr::ResMgr()->ReadCreatable(&stream): nil;
		hsAssert(!v || copy, "failed to create creatable copy");
		fC[idx]=copy;
		IVarSet();
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable");
	return false;
}

/////////////////////////////////////////////////////////////
// GETTERS
/////////////////////////////////////////////////////////////

bool plSimpleStateVariable::Get(int* value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());
	
	if (fVar.GetAtomicType()==plVarDescriptor::kInt)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fI[cnt+i];
		return true;
	}
	
	if (fVar.GetAtomicType()==plVarDescriptor::kShort)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fS[cnt+i];
		return true;
	}

	if (fVar.GetAtomicType()==plVarDescriptor::kByte)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fBy[cnt+i];
		return true;
	}

	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

bool plSimpleStateVariable::Get(short* value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kShort)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fS[cnt+i];
		return true;
	}
	
	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

bool plSimpleStateVariable::Get(byte* value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kByte)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fBy[cnt+i];
		return true;
	}
	
	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

// float or floatVector
bool plSimpleStateVariable::Get(float* value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kAgeTimeOfDay)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
		{
			if (plNetClientApp::GetInstance())
				fF[cnt+i] = plNetClientApp::GetInstance()->GetCurrentAgeTimeOfDayPercent();
			value[i]=fF[cnt+i];
		}
		return true;
	}

	if (fVar.GetAtomicType()==plVarDescriptor::kFloat)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fF[cnt+i];
		return true;
	}

	if (fVar.GetAtomicType()==plVarDescriptor::kDouble)	
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=(float)fD[cnt+i];
		return true;
	}

	if (fVar.GetAtomicType()==plVarDescriptor::kTime)	// && fIsUsed)
	{
		double secs=hsTimer::GetSysSeconds();
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
		{
			double tmp;
			fT[cnt+i].ConvertToGameTime(&tmp, secs);
			value[i] = (float)tmp;
		}
		return true;
	}

	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

// double
bool plSimpleStateVariable::Get(double* value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kDouble)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fD[cnt+i];
		return true;
	}

	if (fVar.GetAtomicType()==plVarDescriptor::kTime)
	{
		double secs=hsTimer::GetSysSeconds();
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			fT[cnt+i].ConvertToGameTime(&value[i], secs);

		return true;
	}

	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

bool plSimpleStateVariable::Get(bool* value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kBool)
	{
		int i;
		int cnt=fVar.GetAtomicCount()*idx;
		for(i=0;i<fVar.GetAtomicCount();i++)
			value[i]=fB[cnt+i];
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

bool plSimpleStateVariable::Get(plKey* value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kKey)
	{
		if(!(fU[idx] == plUoid()))	// compare to default "nil uoid"
		{
			*value = hsgResMgr::ResMgr()->FindKey(fU[idx]);
			if (*value)
			{
				const plUoid& newUoid = (*value)->GetUoid();
				if (stricmp(newUoid.GetObjectName(), fU[idx].GetObjectName()) != 0)
				{
					// uoid names don't match... chances are the key changed in the local data after the key was written to the sdl
					// do a search by name, which takes longer, to get the correct key
					std::vector<plKey> foundKeys;
					plKeyFinder::Instance().ReallyStupidSubstringSearch(fU[idx].GetObjectName(), fU[idx].GetClassType(), foundKeys, fU[idx].GetLocation());
					// not really sure what we can do if it finds MORE then one (they are supposed to be unique names), so just grab the
					// first one and return it
					if (foundKeys.size() >= 1)
						*value = foundKeys[0];
					else
						*value = nil;
				}
			}
		} else {
			*value = nil;
		}
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

bool plSimpleStateVariable::Get(char value[], int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetType()==plVarDescriptor::kString32)
	{
		hsStrcpy(value, fS32[idx]);
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

bool plSimpleStateVariable::Get(plCreatable** value, int idx) const
{
	VALIDATE_WITH_FALSE_RETURN(idx < fVar.GetCount());

	if (fVar.GetAtomicType()==plVarDescriptor::kCreatable)
	{
		*value = nil;
		plCreatable* v = fC[idx];
		if (v)
		{
			*value = plFactory::Create(v->ClassIndex());
			hsAssert(*value, "failed to create creatable copy");
			hsRAMStream stream;
			v->Write(&stream, hsgResMgr::ResMgr());
			stream.Rewind();
			(*value)->Read(&stream, hsgResMgr::ResMgr());
		}
		return true;
	}
	hsAssert(false, "passing wrong value type to SDL variable"); 
	return false;
}

/////////////////////////////////////////////////////////////

const char* plSimpleStateVariable::GetKeyName(int idx) const
{
	if (fVar.GetAtomicType()==plVarDescriptor::kKey)
	{
		if(!(fU[idx] == plUoid()))	// compare to default "nil uoid"
		{
			return fU[idx].GetObjectName();
		}
	}
	hsAssert(false, "passing wrong value type to SDL variable"); 
	return "(nil)";
}

#pragma optimize( "g", off )	// disable float optimizations
bool plSimpleStateVariable::IWriteData(hsStream* s, float timeConvert, int idx, UInt32 writeOptions) const
{
#ifdef HS_DEBUGGING
	if (!IsUsed())
	{
		// hsAssert(false, "plSimpleStateVariable::WriteData doesn't contain data?");
		plNetApp::StaticWarningMsg("plSimpleStateVariable::WriteData Var %s doesn't contain data?",
			GetName());
	}
#endif

	int j=idx*fVar.GetAtomicCount();
	int i;
	switch(fVar.GetAtomicType())
	{
	case plVarDescriptor::kAgeTimeOfDay:
		// don't need to write out ageTime, since it's computed on the fly when Get is called
		break;
	case plVarDescriptor::kInt:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->WriteSwap32(fI[j+i]);
		break;
	case plVarDescriptor::kShort:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->WriteSwap16(fS[j+i]);
		break;
	case plVarDescriptor::kByte:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->WriteByte(fBy[j+i]);
		break;
	case plVarDescriptor::kFloat:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->WriteSwapScalar(fF[j+i]);
		break;
	case plVarDescriptor::kTime:
		for(i=0;i<fVar.GetAtomicCount();i++)
		{
			if (timeConvert != 0.0)
			{
				double utDouble=fT[j+i].GetSecsDouble();
				hsDoublePrecBegin
				utDouble += timeConvert;
				hsDoublePrecEnd
				plUnifiedTime ut(utDouble);
				ut.Write(s);
			}
			else
				fT[j+i].Write(s);
		}
		break;
	case plVarDescriptor::kDouble:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->WriteSwapDouble(fD[j+i]);
		break;
	case plVarDescriptor::kBool:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->Writebool(fB[j+i]);
		break;
	case plVarDescriptor::kKey:
		for(i=0;i<fVar.GetAtomicCount();i++)
			fU[j+i].Write(s);
		break;
	case plVarDescriptor::kString32:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->Write(32, fS32[j+i]);
		break;		
	case plVarDescriptor::kCreatable:
		{
			hsAssert(fVar.GetAtomicCount()==1, "invalid atomic count");
			plCreatable* cre = fC[j];
			s->WriteSwap16(cre ? cre->ClassIndex() : 0x8000);	// creatable class index
			if (cre)
			{
				hsRAMStream ramStream;
				cre->Write(&ramStream, hsgResMgr::ResMgr());
				s->WriteSwap32(ramStream.GetEOF());		// write length
				cre->Write(s, hsgResMgr::ResMgr());		// write data
			}
		}
		break;
	}
	return true;
}

bool plSimpleStateVariable::IReadData(hsStream* s, float timeConvert, int idx, UInt32 readOptions) 
{	
	int j=idx*fVar.GetAtomicCount();
	int i;
	switch(fVar.GetAtomicType())
	{
	case plVarDescriptor::kAgeTimeOfDay:
		// don't need to read in ageTime, since it's computed on the fly when Get is called
		break;
	case plVarDescriptor::kInt:
		for(i=0;i<fVar.GetAtomicCount();i++)
			fI[j+i]=s->ReadSwap32();
		break;
	case plVarDescriptor::kShort:
		for(i=0;i<fVar.GetAtomicCount();i++)
			fS[j+i]=s->ReadSwap16();
		break;
	case plVarDescriptor::kByte:
		for(i=0;i<fVar.GetAtomicCount();i++)
			fBy[j+i]=s->ReadByte();
		break;
	case plVarDescriptor::kFloat:
		for(i=0;i<fVar.GetAtomicCount();i++)
			fF[j+i]=s->ReadSwapScalar();
		break;
	case plVarDescriptor::kTime:
		for(i=0;i<fVar.GetAtomicCount();i++)
		{
			fT[j+i].Read(s);
			if (timeConvert != 0.0)
			{
				hsDoublePrecBegin
				double newUt = (fT[j+i].GetSecsDouble() + timeConvert);
				hsDoublePrecEnd
				hsAssert(newUt>=0, "negative unified time");
				fT[j+i].SetSecsDouble(newUt);
			}
		}
		break;
	case plVarDescriptor::kDouble:
		for(i=0;i<fVar.GetAtomicCount();i++)
			fD[j+i]=s->ReadSwapDouble();
		break;
	case plVarDescriptor::kBool:
		for(i=0;i<fVar.GetAtomicCount();i++)
			fB[j+i]=s->Readbool();
		break;
	case plVarDescriptor::kKey:
		for(i=0;i<fVar.GetAtomicCount();i++)
		{
			fU[j+i].Invalidate();
			fU[j+i].Read(s);
		}
		break;
	case plVarDescriptor::kString32:
		for(i=0;i<fVar.GetAtomicCount();i++)
			s->Read(32, fS32[j+i]);
		break;	
	case plVarDescriptor::kCreatable:
		{
			hsAssert(fVar.GetAtomicCount()==1, "invalid atomic count");
			UInt16 hClass = s->ReadSwap16();	// class index
			if (hClass != 0x8000)
			{				
				UInt32 len = s->ReadSwap32();	// length	
				if (plFactory::CanCreate(hClass))
				{
					delete fC[j];
					fC[j] = plFactory::Create(hClass);
				}
				else
				{
					plSDLCreatableStub* stub = TRACKED_NEW plSDLCreatableStub(hClass, len);
					fC[j] = stub;
				}
				fC[j]->Read(s, hsgResMgr::ResMgr());	// data			
			}
		}
		break;
	default:
		hsAssert(false, "invalid atomic type");
		return false;
	}
	
	return true;
}
#pragma optimize( "", on )	// restore optimizations to their defaults

bool plSimpleStateVariable::WriteData(hsStream* s, float timeConvert, UInt32 writeOptions) const
{
#ifdef HS_DEBUGGING
	if (!IsUsed())
	{
		// hsAssert(false, "plSimpleStateVariable::WriteData Var doesn't contain data?");
		plNetApp::StaticWarningMsg("plSimpleStateVariable::WriteData Var %s doesn't contain data?",
			GetName());
	}
#endif

	// write base class data
	plStateVariable::WriteData(s, timeConvert, writeOptions);	

	// check if the same as default
	bool sameAsDefaults=false;
	if (!GetVarDescriptor()->IsVariableLength())
	{
		plSimpleStateVariable def;
		def.fVar.CopyFrom(&fVar);	// copy descriptor
		def.Alloc();				// and rest
		def.SetFromDefaults(false /* timeStamp */);		// may do nothing if nor default
		sameAsDefaults = (def == *this);
	}

	bool writeTimeStamps = (writeOptions & plSDL::kWriteTimeStamps)!=0;
	bool writeDirtyFlags = (writeOptions & plSDL::kDontWriteDirtyFlag)==0;
	bool forceDirtyFlags = (writeOptions & plSDL::kMakeDirty)!=0;
	bool wantTimeStamp   = (writeOptions & plSDL::kTimeStampOnRead)!=0;
	bool needTimeStamp   = (writeOptions & plSDL::kTimeStampOnWrite)!=0;
	forceDirtyFlags = forceDirtyFlags || (!sameAsDefaults && (writeOptions & plSDL::kDirtyNonDefaults)!=0);

	// write save flags
	UInt8 saveFlags = 0;
	saveFlags |= writeTimeStamps ? plSDL::kHasTimeStamp : 0;
	saveFlags |= forceDirtyFlags || (writeDirtyFlags && IsDirty()) ? plSDL::kHasDirtyFlag : 0;
	saveFlags |= wantTimeStamp ? plSDL::kWantTimeStamp : 0;
	saveFlags |= needTimeStamp ? plSDL::kHasTimeStamp : 0;

	if (sameAsDefaults)
		saveFlags |= plSDL::kSameAsDefault;
	s->WriteSwap(saveFlags);
	
	if (needTimeStamp) {
		// timestamp on write
		fTimeStamp.ToCurrentTime();
		fTimeStamp.Write(s);
	}
	else if (writeTimeStamps) {
		// write time stamps
		fTimeStamp.Write(s);
	}

	// write var data
	if (!sameAsDefaults)
	{
		// list size
		if (GetVarDescriptor()->IsVariableLength())
			s->WriteSwap32(GetVarDescriptor()->GetCount());		// have to write out as long since we don't know how big the list is

		// list
		int i;
		for(i=0;i<fVar.GetCount();i++)
			if (!IWriteData(s, timeConvert, i, writeOptions))
				return false;
	}

	return true;
}

// assumes var is created from the right type of descriptor (count, type, etc.)
bool plSimpleStateVariable::ReadData(hsStream* s, float timeConvert, UInt32 readOptions)
{
	// read base class data
	plStateVariable::ReadData(s, timeConvert, readOptions);

	plUnifiedTime ut;
	ut.ToEpoch();
	
	UInt8 saveFlags;
	s->ReadSwap(&saveFlags);

	bool isDirty = ( saveFlags & plSDL::kHasDirtyFlag )!=0;
	bool setDirty = ( isDirty && ( readOptions & plSDL::kKeepDirty ) ) || ( readOptions & plSDL::kMakeDirty );
	bool wantTimestamp = isDirty &&
		plSDLMgr::GetInstance()->AllowTimeStamping() &&
		(	( saveFlags & plSDL::kWantTimeStamp ) ||
			( readOptions & plSDL::kTimeStampOnRead )	);

	if (saveFlags & plSDL::kHasTimeStamp)
		ut.Read(s);
	else if ( wantTimestamp )
		ut.ToCurrentTime();

	if (!(saveFlags & plSDL::kSameAsDefault))
	{
		setDirty = setDirty || ( readOptions & plSDL::kDirtyNonDefaults )!=0;

		// read list size
		if (GetVarDescriptor()->IsVariableLength())
		{
			UInt32 cnt;
			s->ReadSwap(&cnt);		// have to read as long since we don't know how big the list is

			if (cnt>=0 && cnt<plSDL::kMaxListSize)
				fVar.SetCount(cnt);
			else
				return false;
		
			Alloc();		// alloc after setting count
		}
		else
		{
			hsAssert(fVar.GetCount(), "empty var?");
		}
	}

	// compare timestamps
	if (fTimeStamp > ut)
		return true;
		
	if ( (saveFlags & plSDL::kHasTimeStamp) || (readOptions & plSDL::kTimeStampOnRead) )
		TimeStamp(ut);
	
	// read list
	if (!(saveFlags & plSDL::kSameAsDefault))
	{
		int i;
		for(i=0;i<fVar.GetCount();i++)
			if (!IReadData(s, timeConvert, i, readOptions))
				return false;
	}
	else
	{
		Reset();
		SetFromDefaults(false);
	}

	SetUsed( true );
	SetDirty( setDirty );

	return true;
}

void plSimpleStateVariable::CopyData(const plSimpleStateVariable* other, UInt32 writeOptions/*=0*/)
{
	// use stream as a medium
	hsRAMStream stream;
	other->WriteData(&stream, 0, writeOptions);
	stream.Rewind();
	ReadData(&stream, 0, writeOptions);
}

//
// send notification msg if necessary, called internally
//

#define NOTIFY_CHECK(type, var)		\
case type:	\
	for(i=0;i<cnt;i++)	\
		if (hsABS(var[i] - other->var[i])>d)	\
		{	\
			notify=true;	\
			break;	\
		}	\
	break;	

void plSimpleStateVariable::NotifyStateChange(const plSimpleStateVariable* other, const char* sdlName)
{
	if (fChangeNotifiers.size()==0)
		return;

	bool different=!(*this == *other);
	bool notify=false;
	int numNotifiers=0;
	if (different)
	{
		StateChangeNotifiers::iterator it=fChangeNotifiers.begin();
		for( ; it!=fChangeNotifiers.end(); it++)
		{
			float d=(*it).fDelta;
			if (d==0 && different)		// delta of 0 means notify on any change
			{
				notify=true;
			}
			else
			{
				int i;
				int cnt = fVar.GetAtomicCount()*fVar.GetCount();
				switch(fVar.GetAtomicType())
				{
					NOTIFY_CHECK(plVarDescriptor::kInt, fI)
					NOTIFY_CHECK(plVarDescriptor::kShort, fS)
					NOTIFY_CHECK(plVarDescriptor::kByte, fBy)
					NOTIFY_CHECK(plVarDescriptor::kFloat, fF)
					NOTIFY_CHECK(plVarDescriptor::kDouble, fD)
				}			
			}
			if (notify)
			{
				numNotifiers += (*it).fKeys.size();
				(*it).SendNotificationMsg(other /* src */, this /* dst */, sdlName);
			}
		}
	}

	if (plNetObjectDebuggerBase::GetInstance() && plNetObjectDebuggerBase::GetInstance()->GetDebugging())
	{
		plNetObjectDebuggerBase::GetInstance()->LogMsg(
			xtl::format("Var %s did %s send notification difference. Has %d notifiers with %d recipients.", 
				GetName(), !notify ? "NOT" : "", fChangeNotifiers.size(), numNotifiers).c_str());
	}

}
	
//
// Checks to see if data contents are the same on two matching vars.
//

#define EQ_CHECK(type, var)		\
case type:	\
	for(i=0;i<cnt;i++)	\
		if (var[i]!=other.var[i])	\
			return false;	\
	break;	

bool plSimpleStateVariable::operator==(const plSimpleStateVariable &other) const
{
	hsAssert(fVar.GetType() == other.GetVarDescriptor()->GetType(), "type mismatch in equality check");
	hsAssert(fVar.GetAtomicCount() == other.GetVarDescriptor()->GetAsSimpleVarDescriptor()->GetAtomicCount(),
		"atomic cnt mismatch in equality check");
	
	if (GetCount() != other.GetCount())
		return false;

	int i;
	int cnt = fVar.GetAtomicCount()*fVar.GetCount();
	switch(fVar.GetAtomicType())
	{
		EQ_CHECK(plVarDescriptor::kAgeTimeOfDay, fF)
		EQ_CHECK(plVarDescriptor::kInt, fI)
		EQ_CHECK(plVarDescriptor::kFloat, fF)
		EQ_CHECK(plVarDescriptor::kTime, fT)
		EQ_CHECK(plVarDescriptor::kDouble, fD)
		EQ_CHECK(plVarDescriptor::kBool, fB)
		EQ_CHECK(plVarDescriptor::kKey, fU)
		EQ_CHECK(plVarDescriptor::kCreatable, fC)
		EQ_CHECK(plVarDescriptor::kShort, fS)
		EQ_CHECK(plVarDescriptor::kByte, fBy)		
	case plVarDescriptor::kString32:
		for(i=0;i<cnt;i++)
			if (stricmp(fS32[i],other.fS32[i]))
				return false;
		break;	
	default:
		hsAssert(false, "invalid atomic type");
		return false;
		break;
	}
	
	return true;
}

//
// Add and coalate
//
void plSimpleStateVariable::AddStateChangeNotification(plStateChangeNotifier& n)
{
	StateChangeNotifiers::iterator it=fChangeNotifiers.begin();
	for( ; it != fChangeNotifiers.end(); it++)
	{
		if ((*it).fDelta==n.fDelta)
		{
			// merged into an existing entry
			(*it).AddNotificationKeys(n.fKeys);
			return;
		}
	}

	// add new entry
	fChangeNotifiers.push_back(n);
}

//
// remove entries with this key 
//
void plSimpleStateVariable::RemoveStateChangeNotification(plKey notificationObj)
{
	StateChangeNotifiers::iterator it=fChangeNotifiers.end();
	for(; it != fChangeNotifiers.begin();)
	{
		it--;
		int size=(*it).RemoveNotificationKey(notificationObj);
		if (size==0)
			it=fChangeNotifiers.erase(it);	// iterator is moved to item after this one
	}
}

//
// remove entries which match
//
void plSimpleStateVariable::RemoveStateChangeNotification(plStateChangeNotifier n)
{
	StateChangeNotifiers::iterator it=fChangeNotifiers.end();
	for(; it != fChangeNotifiers.begin();)
	{
		it--;
		if ( (*it).fDelta==n.fDelta)
		{
			int size=(*it).RemoveNotificationKeys(n.fKeys);
			if (size==0)
				it=fChangeNotifiers.erase(it);	// iterator is moved to item after this one
		}
	}
}

//
//
//
void plSimpleStateVariable::DumpToObjectDebugger(bool dirtyOnly, int level) const
{
	plNetObjectDebuggerBase* dbg = plNetObjectDebuggerBase::GetInstance();
	if (!dbg)
		return;

	std::string pad;
	int i;
	for(i=0;i<level; i++)
		pad += "   ";

	std::string logMsg = xtl::format( "%sSimpleVar, name:%s[%d]", pad.c_str(), GetName(), GetCount());
	if (GetCount()>1)
	{
		dbg->LogMsg(logMsg.c_str());	// it's going to be a long msg, so print it on its own line
		logMsg = "";
	}
	
	pad += "\t";
	for(i=0;i<GetCount(); i++)
	{
		char* s=GetAsString(i);
		if (fVar.GetAtomicType() == plVarDescriptor::kTime)
		{
			const char* p=fT[i].PrintWMillis();
			logMsg += xtl::format( "%sVar:%d gameTime:%s pst:%s ts:%s", 
				pad.c_str(), i, s ? s : "?", p, fTimeStamp.Format("%c").c_str() );
		}
		else
		{
			logMsg +=xtl::format( "%sVar:%d value:%s ts:%s", 
				pad.c_str(), i, s ? s : "?", fTimeStamp.AtEpoch() ? "0" : fTimeStamp.Format("%c").c_str() );
		}
		delete [] s;

		if ( !dirtyOnly )
			logMsg += xtl::format( " dirty:%d", IsDirty() );

		dbg->LogMsg(logMsg.c_str());
		logMsg = "";
	}
}

void plSimpleStateVariable::DumpToStream(hsStream* stream, bool dirtyOnly, int level) const
{
	std::string pad;
	int i;
	for(i=0;i<level; i++)
		pad += "   ";

	std::string logMsg = xtl::format( "%sSimpleVar, name:%s[%d]", pad.c_str(), GetName(), GetCount());
	if (GetCount()>1)
	{
		stream->WriteString(logMsg.c_str());	// it's going to be a long msg, so print it on its own line
		logMsg = "";
	}
	
	pad += "\t";
	for(i=0;i<GetCount(); i++)
	{
		char* s=GetAsString(i);
		if (fVar.GetAtomicType() == plVarDescriptor::kTime)
		{
			const char* p=fT[i].PrintWMillis();
			logMsg += xtl::format( "%sVar:%d gameTime:%s pst:%s ts:%s", 
				pad.c_str(), i, s ? s : "?", p, fTimeStamp.Format("%c").c_str() );
		}
		else
		{
			logMsg +=xtl::format( "%sVar:%d value:%s ts:%s", 
				pad.c_str(), i, s ? s : "?", fTimeStamp.AtEpoch() ? "0" : fTimeStamp.Format("%c").c_str() );
		}
		delete [] s;

		if ( !dirtyOnly )
			logMsg += xtl::format( " dirty:%d", IsDirty() );

		stream->WriteString(logMsg.c_str());
		logMsg = "";
	}
}

//
// set var to its defalt value
//
void plSimpleStateVariable::SetFromDefaults(bool timeStampNow)
{
	int i;
	for(i=0;i<GetCount();i++)
		SetFromString(GetVarDescriptor()->GetDefault(), i, timeStampNow);
}

///////////////////////////////////////////////////////////////////////////////
// plSDStateVariable
///////////////////////////////////////////////////////////////////////////////

plSDStateVariable::plSDStateVariable(plSDVarDescriptor* sdvd) : fVarDescriptor(nil)
{ 
	Alloc(sdvd);
}

plSDStateVariable::~plSDStateVariable()
{
	IDeInit();
}

//
// resize the array of state data records
//
void plSDStateVariable::Resize(int cnt)
{
	int origCnt=GetCount();
	// when sending, this is always freshly allocated. if you then set it to zero,
	// the change won't be sent, even though the version on the server might not be zero
	// for now, we're just not going to do this optimization
	// we could, however, change it to (origCnt==cnt==0), because for sizes other
	// than zero the bug won't happen
//	if (origCnt==cnt)
//		return;		// no work to do

	// shrinking
	if (cnt<origCnt)
	{
		int i;
		for(i=cnt;i<origCnt;i++)
			delete fDataRecList[i];
	}

	fDataRecList.resize(cnt);

	// growing
	if (cnt>origCnt)
	{
		int i;
		for(i=origCnt;i<cnt;i++)
			fDataRecList[i] = TRACKED_NEW plStateDataRecord(fVarDescriptor->GetStateDescriptor());
	}

	SetDirty(true);
	SetUsed(true);
}

//
// create/allocate data records based on the given SDVarDesc
//
void plSDStateVariable::Alloc(plSDVarDescriptor* sdvd, int listSize)
{
	if (sdvd==fVarDescriptor)
	{
		// trick to not have to delete and recreate fVarDescriptor
		fVarDescriptor=nil;
		IDeInit();	
		fVarDescriptor=sdvd;
	}
	else
		IDeInit();	// will delete fVarDescriptor

	if (sdvd) 
	{
		if (fVarDescriptor==nil)
		{
			fVarDescriptor = TRACKED_NEW plSDVarDescriptor;
			fVarDescriptor->CopyFrom(sdvd);
		}

		int cnt = listSize==-1 ? sdvd->GetCount() : listSize;
		fDataRecList.resize(cnt); 
		int j;
		for (j=0;j<cnt; j++)
			InsertStateDataRecord(TRACKED_NEW plStateDataRecord(sdvd->GetStateDescriptor()), j);
	}
}

//
// help alloc fxn
//
void plSDStateVariable::Alloc(int listSize)
{
	Alloc(fVarDescriptor, listSize);
}

//
// delete all records
//
void plSDStateVariable::IDeInit()
{
	DataRecList::iterator it;
	for (it=fDataRecList.begin(); it != fDataRecList.end(); it++)
		delete *it;
	fDataRecList.clear();
	delete fVarDescriptor;
	fVarDescriptor=nil;
}

//
// Make 'this' into a copy of 'other'.
//
void plSDStateVariable::CopyFrom(plSDStateVariable* other, UInt32 writeOptions/*=0*/)
{
	// IDeInit();
	Alloc(other->GetSDVarDescriptor(), other->GetCount());
	int i;
	for(i=0; i<other->GetCount(); i++)
		fDataRecList[i]->CopyFrom(*other->GetStateDataRecord(i),writeOptions);
}

//
// Find the data items which are dirty in 'other' and 
// copy them to my corresponding item.
// Requires that records have the same descriptor.
//
void plSDStateVariable::UpdateFrom(plSDStateVariable* other, UInt32 writeOptions/*=0*/)
{
	hsAssert(!stricmp(other->GetSDVarDescriptor()->GetName(), fVarDescriptor->GetName()), 
		xtl::format("var descriptor mismatch in UpdateFrom, name %s,%s ver %d,%d",
		GetName(), other->GetName()).c_str());
	Resize(other->GetCount());	// make sure sizes match

	bool dirtyOnly = (writeOptions & plSDL::kDirtyOnly);

	int i;
	for(i=0; i<other->GetCount(); i++)
	{
		if ( (dirtyOnly && other->GetStateDataRecord(i)->IsDirty()) ||
			 (!dirtyOnly &&other->GetStateDataRecord(i)->IsUsed()) )
			fDataRecList[i]->UpdateFrom(*other->GetStateDataRecord(i), writeOptions);
	}
}

//
// Convert all my stateDataRecords to the type defined by the 'other var'
//
void plSDStateVariable::ConvertTo(plSDStateVariable* otherSDVar, bool force )
{
	plStateDescriptor* otherSD=otherSDVar->GetSDVarDescriptor()->GetStateDescriptor();

	hsLogEntry( plNetApp::StaticDebugMsg( "SDSV(%p) converting %s from %s to %s (force:%d)",
		this, fVarDescriptor->GetName(), fVarDescriptor->GetTypeString(),
		otherSDVar->GetSDVarDescriptor()->GetTypeString(), force ) );

	int j;
	for(j=0;j<GetCount(); j++)
	{
		GetStateDataRecord(j)->ConvertTo( otherSD, force );
	}
}		

bool plSDStateVariable::IsDirty() const
{
	if (plStateVariable::IsDirty())
		return true;

	int j;
	for(j=0;j<GetCount(); j++)
		if (GetStateDataRecord(j)->IsDirty())
			return true;
	return false;
}

int plSDStateVariable::GetDirtyCount() const
{
	int cnt=0;
	int j;
	for(j=0;j<GetCount(); j++)
		if (GetStateDataRecord(j)->IsDirty())
			cnt++;
	return cnt;
}

bool plSDStateVariable::IsUsed() const
{
	if (plStateVariable::IsUsed())
		return true;
	
	int j;
	for(j=0;j<GetCount(); j++)
		if (GetStateDataRecord(j)->IsUsed())
			return true;
	return false;
}

int plSDStateVariable::GetUsedCount() const
{
	int cnt=0;
	int j;
	for(j=0;j<GetCount(); j++)
		if (GetStateDataRecord(j)->IsUsed())
			cnt++;
	return cnt;
}

void plSDStateVariable::GetUsedDataRecords(ConstDataRecList* recList) const
{
	recList->clear();
	int j;
	for(j=0;j<GetCount(); j++)
		if (GetStateDataRecord(j)->IsUsed())
			recList->push_back(GetStateDataRecord(j));
}

void plSDStateVariable::GetDirtyDataRecords(ConstDataRecList* recList) const
{
	recList->clear();
	int j;
	for(j=0;j<GetCount(); j++)
		if (GetStateDataRecord(j)->IsDirty())
			recList->push_back(GetStateDataRecord(j));
}

//
// read all SDVars
//
bool plSDStateVariable::ReadData(hsStream* s, float timeConvert, UInt32 readOptions)
{
	plStateVariable::ReadData(s, timeConvert, readOptions);

	UInt8 saveFlags;
	s->ReadSwap(&saveFlags);	// unused

	// read total list size
	if (GetVarDescriptor()->IsVariableLength())
	{
		UInt32 total;
		s->ReadSwap(&total);
		Resize(total);
	}
	
	// read dirty list size	
	int cnt;
	plSDL::VariableLengthRead(s, 
		GetVarDescriptor()->IsVariableLength() ? 0xffffffff : GetVarDescriptor()->GetCount(), &cnt);

	// if we are reading the entire list in, then we don't need to read each index
	bool all = (cnt==fDataRecList.size());

	// read list
	int i;
	for(i=0;i<cnt; i++)
	{
		int idx;
		if (!all)
			plSDL::VariableLengthRead(s, 
				GetVarDescriptor()->IsVariableLength() ? 0xffffffff : GetVarDescriptor()->GetCount(), &idx);
		else
			idx=i;
		
		if (idx<fDataRecList.size())
			fDataRecList[idx]->Read(s, timeConvert, readOptions);
		else
			return false;
	}
	return true;
}

//
// write all SDVars
//
bool plSDStateVariable::WriteData(hsStream* s, float timeConvert, UInt32 writeOptions) const
{	
	plStateVariable::WriteData(s, timeConvert, writeOptions);

	UInt8 saveFlags=0;	// unused	
	s->WriteSwap(saveFlags);

	// write total list size
	UInt32 total=GetCount();
	if (GetVarDescriptor()->IsVariableLength())
		s->WriteSwap(total);

	// write dirty list size
	bool dirtyOnly = (writeOptions & plSDL::kDirtyOnly) != 0;
	int writeCnt = dirtyOnly ? GetDirtyCount() : GetUsedCount();
	plSDL::VariableLengthWrite(s, 
		GetVarDescriptor()->IsVariableLength() ? 0xffffffff : GetVarDescriptor()->GetCount(), writeCnt);

	// if we are writing the entire list in, then we don't need to read each index
	bool all = (writeCnt==fDataRecList.size());

	// write list
	int i, written=0;
	for(i=0;i<total;i++)
	{
		if ( (dirtyOnly && fDataRecList[i]->IsDirty()) || 
			(!dirtyOnly && fDataRecList[i]->IsUsed()) )
		{
			if (!all)
				plSDL::VariableLengthWrite(s, 
					GetVarDescriptor()->IsVariableLength() ? 0xffffffff : GetVarDescriptor()->GetCount(), i);	// idx
			fDataRecList[i]->Write(s, timeConvert, dirtyOnly);	// item
			written++;
		}
	}
	hsAssert(writeCnt==written, "write mismatch");
	return true;
}

//
//
//
void plSDStateVariable::DumpToObjectDebugger(bool dirtyOnly, int level) const
{
	plNetObjectDebuggerBase* dbg = plNetObjectDebuggerBase::GetInstance();
	if (!dbg)
		return;

	std::string pad;
	int i;
	for(i=0;i<level; i++)
		pad += "   ";

	int cnt = dirtyOnly ? GetDirtyCount() : GetUsedCount();
	dbg->LogMsg(xtl::format( "%sSDVar, name:%s dirtyOnly:%d count:%d", 
		pad.c_str(), GetName(), dirtyOnly, cnt).c_str());

	for(i=0;i<GetCount();i++)
	{
		if ( (dirtyOnly && fDataRecList[i]->IsDirty()) || 
			(!dirtyOnly && fDataRecList[i]->IsUsed()) )
		{
			fDataRecList[i]->DumpToObjectDebugger(nil, dirtyOnly, level+1);
		}
	}
}

void plSDStateVariable::DumpToStream(hsStream* stream, bool dirtyOnly, int level) const
{
	std::string pad;
	int i;
	for(i=0;i<level; i++)
		pad += "   ";

	int cnt = dirtyOnly ? GetDirtyCount() : GetUsedCount();
	stream->WriteString(xtl::format( "%sSDVar, name:%s dirtyOnly:%d count:%d", 
		pad.c_str(), GetName(), dirtyOnly, cnt).c_str());

	for(i=0;i<GetCount();i++)
	{
		if ( (dirtyOnly && fDataRecList[i]->IsDirty()) || 
			(!dirtyOnly && fDataRecList[i]->IsUsed()) )
		{
			fDataRecList[i]->DumpToStream(stream, nil, dirtyOnly, level+1);
		}
	}
}

//
// Checks to see if data contents are the same on two matching vars.
//
bool plSDStateVariable::operator==(const plSDStateVariable &other) const
{
	hsAssert(GetSDVarDescriptor()->GetStateDescriptor() == other.GetSDVarDescriptor()->GetStateDescriptor(),
		"SD var descriptor mismatch in equality check");

	if (GetCount() != other.GetCount())
		return false;	// different list sizes

	int i;
	for(i=0;i<GetCount(); i++)
	{
		if (! (*GetStateDataRecord(i) == *other.GetStateDataRecord(i)))
			return false;
	}

	return true;
}

void plSDStateVariable::SetFromDefaults(bool timeStampNow)
{
	int i;
	for(i=0;i<GetCount(); i++)
		GetStateDataRecord(i)->SetFromDefaults(timeStampNow);
}

void plSDStateVariable::TimeStamp( const plUnifiedTime & ut/*=plUnifiedTime::GetCurrentTime()*/ )
{
	hsAssert( false, "not impl" );
}

void plSDStateVariable::FlagNewerState(const plSDStateVariable& other, bool respectAlwaysNew)
{
	int i;
	for(i=0;i<GetCount(); i++)
		GetStateDataRecord(i)->FlagNewerState(*other.GetStateDataRecord(i), respectAlwaysNew);
}

void plSDStateVariable::FlagAlwaysNewState()
{
	int i;
	for(i=0;i<GetCount(); i++)
		GetStateDataRecord(i)->FlagAlwaysNewState();
}
