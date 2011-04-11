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

#include "hsTypes.h"
#include "hsGEnviron.h"
#include "../plSurface/hsGLayer.h"
#include "../plSurface/hsGMaterial.h"
//#include "hsG3DDevice.h"
//#include "plCamera.h"
#include "hsFogControl.h"
#include "../plGRenderProcs/hsGRenderProcs.h"
#if 0 // SCENENODE_DEFER
#include "hsScene.h"
#endif // SCENENODE_DEFER
#include "../plResMgr/hsResMgr.h"
#include "hsGStatGather3.h"
#include "../plResMgr/plKey.h"
#include "hsTimer.h"
#include "../plResMgr/plRefMsg.h"

const UInt16 hsGEnvironment::kSaveMagicNumber = 0x1f7a;
const UInt16 hsGEnvironment::kSaveVersion = 2;

hsScalar hsGEnvironment::fYonScale = 1.f;

hsGEnvironment::hsGEnvironment() 
	: fFlags(0), 
	fValidScale(1.f),
	fMap(nil), 
	fPos(0,0,0), 
	fRadius(0), 
	fFogDistance(0),
	fFogDepth(0),
	fCurrentDepth(0),
	fFogDensity(0),
	fDevCache(nil),
	fYon(0),
	fFogControl(nil)
{
	fFogColor.SetValue(hsColorRGBA().Set(0, 0, 0, hsScalar1));
	SetFogColor(fFogColor.GetValue());
	*fMapName = 0;
}

hsGEnvironment::~hsGEnvironment()
{ 
	hsRefCnt_SafeUnRef(fMap); 
	hsRefCnt_SafeUnRef(fDevCache);

	hsRefCnt_SafeUnRef(fFogControl);

	Int32 i;
	for (i=0; i<fFogStateStack.GetCount(); i++)
		delete fFogStateStack[i];

	for( i = 0; i < GetNumRenderProcs(); i++ )
		hsRefCnt_SafeUnRef(GetRenderProc(i));

}


#if 0 // SCENENODE_DEFER
hsBool32 hsGEnvironment::AddNode(hsSceneNode *node)
{
    return AddNodeKey(node->GetKey());
}
#endif // SCENENODE_DEFER

hsBool32 hsGEnvironment::AddNodeKey(plKey *key)
{
	Int32 i;
	for (i = 0; i < GetNumNodes(); i++)
	{
		if (GetNodeKey(i) == key) // nothing to do
			return false;
	}
    fNodeKeys.Append(key);

    hsSceneNode *node = (hsSceneNode *)(key->GetObjectPtr());
#if 0 // SCENENODE_DEFER
    if (node)
    {
		if (!(GetFlags() & hsGEnvironment::kFarOut))
        {
			node->SetEnvironment(this);
        }
		else
        {
			node->SetFarEnvironment(this);
        }
    }
#endif // SCENENODE_DEFER

	return true;
}

hsSceneNode* hsGEnvironment::GetNode(Int32 i) 
{ 
	return (hsSceneNode*)(fNodeKeys[i]->GetObjectPtr()); 
}

void hsGEnvironment::FogState::ValidateEnv(hsGEnvironment* env)
{
	char* msg = "hsGEnvironment not in reset state.";
	if( fFlags & kYonSet )
	{
		hsAssert(env->GetFlags() & hsGEnvironment::kYonSet, msg);
		hsAssert(fYon == env->YonState(), msg);
	}
	
	if( fFlags & kDistanceSet )
	{
		hsAssert(env->GetFlags() & hsGEnvironment::kFogDistanceSet, msg);
		hsAssert(fDistance == env->FogDistanceState(), msg);
	}

	if( fFlags & kDepthSet )
	{
		hsAssert(env->GetFlags() & hsGEnvironment::kFogDepthSet, msg);
		hsAssert(fDepth == env->FogDepthState(), msg);
	}

	if( fFlags & kDensitySet )
	{
		hsAssert(env->GetFlags() & hsGEnvironment::kFogDensitySet, msg);
		hsAssert(fDensity == env->FogDensityState(), msg);
	}

	if( fFlags & kColorSet )
	{
		hsAssert(env->GetFlags() & hsGEnvironment::kFogColorSet, msg);
		hsAssert(fColor == env->FogColorState(), msg);
	}

	if( fFlags & kClearSet )
	{
		hsAssert(env->GetFlags() & hsGEnvironment::kClearColorSet, msg);
		hsAssert(fClear == env->ClearColorState(), msg);
	}

	UInt32 envFogType = env->GetFlags() & hsGEnvironment::kFogTypeMask;
	switch( fFlags & kTypeMask )
	{
	case kLinear:
		hsAssert(envFogType == hsGEnvironment::kFogLinear, msg);
		break;
	case kExp:
		hsAssert(envFogType == hsGEnvironment::kFogExp, msg);
		break;
	case kExp2:
		hsAssert(envFogType == hsGEnvironment::kFogExp2, msg);
		break;
	default:
		hsAssert(false, msg);
		break;
	}
}

void hsGEnvironment::FogState::SetFromEnv(hsGEnvironment* env)
{
	if( env->GetFlags() & hsGEnvironment::kYonSet )
	{
		fYon = env->YonState();
		fFlags |= kYonSet;
	}
	if( env->GetFlags() & hsGEnvironment::kFogDistanceSet )
	{
		fDistance = env->FogDistanceState();
		fFlags |= kDistanceSet;
	}
	if( env->GetFlags() & hsGEnvironment::kFogDepthSet )
	{
		fDepth = env->FogDepthState();
		fFlags |= kDepthSet;
	}
	if( env->GetFlags() & hsGEnvironment::kFogDensitySet )
	{
		fDensity = env->FogDensityState();
		fFlags |= kDensitySet;
	}
	if( env->GetFlags() & hsGEnvironment::kFogColorSet )
	{
		fColor = env->FogColorState();
		fFlags |= kColorSet;
	}
	if( env->GetFlags() & hsGEnvironment::kClearColorSet )
	{
		fClear = env->ClearColorState();
		fFlags |= kClearSet;
	}

	fFlags &= ~kTypeMask;
	switch( env->GetFlags() & hsGEnvironment::kFogTypeMask )
	{
	case hsGEnvironment::kFogLinear:
		fFlags |= kLinear;
		break;
	case hsGEnvironment::kFogExp:
		fFlags |= kExp;
		break;
	case hsGEnvironment::kFogExp2:
		fFlags |= kExp2;
		break;
	default:
		hsAssert(false, "Fog type should at least default at load");
		break;
	}
}

void hsGEnvironment::FogState::SetToEnv(hsGEnvironment* env)
{
	if( fFlags & kYonSet )
		env->SetYon(fYon);
	else
		env->UnSetYon();
	
	if( fFlags & kDistanceSet )
		env->SetFogDistance(fDistance);
	else
		env->UnSetFogDistance();

	if( fFlags & kDepthSet )
	{
		env->SetFogDepth(fDepth);
		env->SetCurrentDepth(fDepth.GetValue());
	}
	else
		env->UnSetFogDepth();

	if( fFlags & kDensitySet )
		env->SetFogDensity(fDensity);
	else
		env->UnSetFogDensity();

	if( fFlags & kColorSet )
		env->SetFogColor(fColor);
	else
		env->UnSetFogColor();

	if( fFlags & kClearSet )
		env->SetClearColor(fClear);
	else
		env->UnSetClearColor();

	switch( fFlags & kTypeMask )
	{
	case kLinear:
		env->SetFogType(hsGEnvironment::kFogLinear);
		break;
	case kExp:
		env->SetFogType(hsGEnvironment::kFogExp);
		break;
	case kExp2:
		env->SetFogType(hsGEnvironment::kFogExp2);
		break;
	default:
		hsAssert(false, "Setting lack of fog type");
		env->SetFogType(0);
		break;
	}
}

const UInt16 hsGEnvironment::FogState::kSaveMagicNumber = 0x7385;
const UInt16 hsGEnvironment::FogState::kSaveVersion = 1;

void hsGEnvironment::FogState::Save(hsStream *stream, hsResMgr* mgr)
{
    stream->WriteSwap16(kSaveMagicNumber);
    stream->WriteSwap16(kSaveVersion);

	hsScalar currSecs = hsTimer::GetSysSeconds();
    stream->WriteSwap32(fFlags);
	fColor.Write(stream, currSecs);
	fClear.Write(stream, currSecs);
	fDistance.WriteScalar(stream, currSecs);
	fDensity.WriteScalar(stream, currSecs);
	fDepth.WriteScalar(stream, currSecs);
	fYon.WriteScalar(stream, currSecs);
}

void hsGEnvironment::FogState::Load(hsStream *stream, hsResMgr* mgr)
{
    UInt16 magic = stream->ReadSwap16();
	hsAssert(magic == kSaveMagicNumber, "Bad magic number in hsGEnvironment::FogState on load.");

    UInt16 version = stream->ReadSwap16();
    hsAssert(version == kSaveVersion, "Bad version in hsGEnvironment::FogState on load.");

	hsScalar currSecs = hsTimer::GetSysSeconds();
    fFlags = stream->ReadSwap32();
	fColor.Read(stream, currSecs);
	fClear.Read(stream, currSecs);
	fDistance.ReadScalar(stream, currSecs);
	fDensity.ReadScalar(stream, currSecs);
	fDepth.ReadScalar(stream, currSecs);
	fYon.ReadScalar(stream, currSecs);
}


void hsGEnvironment::SetResetState()
{
	fResetState.SetFromEnv(this);
}

void hsGEnvironment::Reset()
{
	fResetState.SetToEnv(this);

	Int32 i;
	for (i=0; i<fFogStateStack.GetCount(); i++)
		delete fFogStateStack[i];
	fFogStateStack.Reset();
}

void hsGEnvironment::ValidateInResetState()
{
	fResetState.ValidateEnv(this);

	hsAssert(fFogStateStack.GetCount() == 0, "Bad fog state stack.");
}

void hsGEnvironment::Save(hsStream *stream, hsResMgr* mgr)
{
	hsKeyedObject::Save(stream, mgr);

    stream->WriteSwap16(kSaveMagicNumber);
    stream->WriteSwap16(kSaveVersion);

    stream->WriteSwap32(fFlags);

	if( fFlags & hsGEnvironment::kCenterSet )
		fPos.Write(stream);

    if( fFlags & hsGEnvironment::kRadiusSet )
		stream->WriteSwapScalar(fRadius);

    FogState tempState;
    tempState.SetFromEnv(this);

    tempState.Save(stream, mgr);
    fResetState.Save(stream, mgr);

    UInt32 numFogStates = fFogStateStack.GetCount();
    stream->WriteSwap32(numFogStates);
    for (UInt32 i = 0; i < numFogStates; i++)
    {
        fFogStateStack[i]->Save(stream, mgr);
    }
}

void hsGEnvironment::Load(hsStream *stream, hsResMgr* mgr)
{
    // Clear old data
	UInt32 i;
	for (i=0; i<fFogStateStack.GetCount(); i++)
		delete fFogStateStack[i];
	fFogStateStack.Reset();

    // Load
	hsKeyedObject::Load(stream, mgr);

    UInt16 magic = stream->ReadSwap16();
	hsAssert(magic == kSaveMagicNumber, "Bad magic number in hsGEnvironment on load.");

    UInt16 version = stream->ReadSwap16();
    hsAssert(version == kSaveVersion, "Bad version in hsGEnvironment on load.");

    fFlags = stream->ReadSwap32();

	if( fFlags & hsGEnvironment::kCenterSet )
		fPos.Read(stream);
    
    if( fFlags & hsGEnvironment::kRadiusSet )
        fRadius = stream->ReadSwapScalar();
    
    FogState tempState;
    tempState.Load(stream, mgr);
    tempState.SetToEnv(this);
    fResetState.Load(stream, mgr);

    UInt32 numFogStates = stream->ReadSwap32();
    for (i = 0; i < numFogStates; i++)
    {
        FogState *newFogState = new FogState;
        newFogState->Load(stream, mgr);
        fFogStateStack.Append(newFogState);
    }
}

void hsGEnvironment::Update(hsScalar s, const hsPoint3& vPos)
{
	if( fFlags & kFogDepthSet )
		fFogDepth.Update(s);
	if( fFlags & kFogDensitySet )
		fFogDensity.Update(s);
	if( fFlags & kFogColorSet )
		fFogColor.Update(s);
	if( fFlags & kClearColorSet )
		fClearColor.Update(s);
	if( fFlags & kYonSet )
		fYon.Update(s);
	if( fFlags & kMapSet )
	{
		hsGStatGather3::UpdateMaterialBegin();
		fMap->Update(s);
		hsGStatGather3::UpdateMaterialEnd();
		
		if( fFlags & kClearColorAmbient )
			SetClearColor(fMap->GetLayer(0)->GetAmbientColor());

		if( fFlags & kFogColorAmbient )
			SetFogColor(fMap->GetLayer(0)->GetAmbientColor());
		else
		if( fFlags & kFogColorDiffuse )
			SetFogColor(fMap->GetLayer(0)->GetColor());
	}

	if( fFlags & kFogDistanceSet )
	{
		fFogDistance.Update(s);


		hsVector3 del(&vPos, &fPos);
		hsScalar dist = del.Magnitude();
		
		hsScalar yon = hsMaximum(dist, fFogDistance.GetValue());

		SetYon(yon);

		hsScalar depth = fFogDistance.GetValue();
		if( fFlags & kFogDepthSet )
			depth *= fFogDepth.GetValue();

		depth += yon - dist;
		depth /= yon;

		if( depth > hsScalar1 )
			depth = hsScalar1;

		fCurrentDepth = depth;
		fFlags |= kCurrentDepthSet;
	}
	else if( fFlags & kFogDepthSet )
	{
		fCurrentDepth = fFogDepth.GetValue();
		fFlags |= kCurrentDepthSet;
	}
	else
		fFlags &= ~kCurrentDepthSet;

	if( fValidScale != fYonScale )
	{
		if( fDevCache )
			fDevCache->Validate(false);
		fValidScale = fYonScale;
	}
}

void hsGEnvironment::SetTimedFogDistance(const hsScalar g, const hsScalar s) 
{ 
	if( fFlags & kFogDistanceSet )
	{
		fFogDistance.Update(hsTimer::GetSysSeconds());
		fFogDistance.SetGoal(g); 
		fFogDistance.SetDuration(s); 
		fFogDistance.StartClock(hsTimer::GetSysSeconds()); 
	}
	else
		SetFogDistance(g);
}

void hsGEnvironment::SetTimedFogDepth(const hsScalar g, const hsScalar s) 
{ 
	if( fFlags & kFogDepthSet )
	{
		fFogDepth.Update(hsTimer::GetSysSeconds());
		fFogDepth.SetGoal(g); 
		fFogDepth.SetDuration(s); 
		fFogDepth.StartClock(hsTimer::GetSysSeconds()); 
	}
	else
		SetFogDepth(g);
}

void hsGEnvironment::SetTimedFogDensity(const hsScalar g, const hsScalar s) 
{ 
	if( fFlags & kFogDensitySet )
	{
		fFogDensity.Update(hsTimer::GetSysSeconds());
		fFogDensity.SetGoal(g); 
		fFogDensity.SetDuration(s); 
		fFogDensity.StartClock(hsTimer::GetSysSeconds()); 
	}
	else
		SetFogDensity(g);
}

void hsGEnvironment::SetTimedFogColor(const hsColorRGBA& g, const hsScalar s) 
{ 
	if( fFlags & kFogColorSet )
	{
		fFogColor.Update(hsTimer::GetSysSeconds());
		fFogColor.SetGoal(g); 
		fFogColor.SetDuration(s); 
		fFogColor.StartClock(hsTimer::GetSysSeconds()); 
	}
	else
		SetFogColor(g);
}

void hsGEnvironment::SetTimedClearColor(const hsColorRGBA& g, const hsScalar s) 
{ 
	if( fFlags & kClearColorSet )
	{
		fClearColor.Update(hsTimer::GetSysSeconds());
		fClearColor.SetGoal(g); 
		fClearColor.SetDuration(s); 
		fClearColor.StartClock(hsTimer::GetSysSeconds()); 
	}
	else
		SetClearColor(g);
}

void hsGEnvironment::SetTimedYon(const hsScalar g, const hsScalar s) 
{ 
	if( fFlags & kYonSet )
	{
		fYon.Update(hsTimer::GetSysSeconds());
		fYon.SetGoal(g); 
		fYon.SetDuration(s);
		fYon.StartClock(hsTimer::GetSysSeconds());
	}
	else
		SetYon(g);
}

void hsGEnvironment::SetMapName(const char *n)
{
	if( n )
	{
		hsAssert(strlen(n) < 255, "Environment name overflow");
		fFlags |= kMapSet; 
		strcpy(fMapName, n);
	}
	else
	{
		fFlags &= ~kMapSet;
		*fMapName = 0;
	}
}

void hsGEnvironment::SetMap(hsGMaterial *m) 
{ 
	fFlags |= kMapSet; 
	hsRefCnt_SafeAssign(fMap, m); 
}

void hsGEnvironment::SetCenter(const hsPoint3 &p) 
{ 
	fFlags |= kCenterSet; fPos = p; 
}

void hsGEnvironment::SetRadius(hsScalar r) 
{ 
	fFlags |= kRadiusSet; 
	fRadius = r; 
}

void hsGEnvironment::SetFogDistance(hsScalar f) 
{ 
	if( fDevCache &&( f != fFogDistance.GetValue()) )
		fDevCache->Validate(false);
	fFlags |= kFogDistanceSet; 
	fFogDistance.SetValue(f);
}

void hsGEnvironment::SetCurrentDepth(hsScalar f) 
{ 
	if( fDevCache && (f != fCurrentDepth) )
		fDevCache->Validate(false);
	fFlags |= kCurrentDepthSet; 
	fCurrentDepth = f;
}

void hsGEnvironment::SetFogDepth(hsScalar f) 
{ 
	if( fDevCache && (f != fFogDepth.GetValue()) )
		fDevCache->Validate(false);
	fFlags |= kFogDepthSet; 
	fFogDepth.SetValue(f); 
}

void hsGEnvironment::SetFogDensity(hsScalar f) 
{ 
	if( fDevCache && (f != fFogDensity.GetValue()) )
		fDevCache->Validate(false);
	fFlags |= kFogDensitySet; 
	fFogDensity.SetValue(f); 
}

void hsGEnvironment::SetFogColor(const hsColorRGBA &c) 
{ 
	if( fDevCache 
		&& (
			(c.r != fFogColor.GetValue().r) 
			|| (c.g != fFogColor.GetValue().g) 
			|| (c.b != fFogColor.GetValue().b) 
			) )
		fDevCache->Validate(false);
	fFlags |= kFogColorSet; 
	fFogColor.SetValue(c); 
}

void hsGEnvironment::SetClearColor(const hsColorRGBA &c) 
{ 
	fFlags |= kClearColorSet; 
	fClearColor.SetValue(c); 
}

void hsGEnvironment::SetYon(hsScalar f) 
{ 
	if( fDevCache && (f != fYon.GetValue()) )
		fDevCache->Validate(false);
	fFlags |= kYonSet; 
	fYon.SetValue(f); 
}

void hsGEnvironment::SetFogDistance(const hsTimedValue<hsScalar>& f) 
{ 
	if( fDevCache )
		fDevCache->Validate(false);
	fFlags |= kFogDistanceSet; 
	fFogDistance = f;
}

void hsGEnvironment::SetFogDepth(const hsTimedValue<hsScalar>& f) 
{ 
	if( fDevCache )
		fDevCache->Validate(false);
	fFlags |= kFogDepthSet; 
	fFogDepth = f; 
}

void hsGEnvironment::SetFogDensity(const hsTimedValue<hsScalar>& f) 
{ 
	if( fDevCache )
		fDevCache->Validate(false);
	fFlags |= kFogDensitySet; 
	fFogDensity = f; 
}

void hsGEnvironment::SetFogColor(const hsTimedValue<hsColorRGBA>& c) 
{ 
	if( fDevCache )
		fDevCache->Validate(false);
	fFlags |= kFogColorSet; 
	fFogColor = c; 
}

void hsGEnvironment::SetClearColor(const hsTimedValue<hsColorRGBA>& c) 
{ 
	fFlags |= kClearColorSet; 
	fClearColor = c; 
}

void hsGEnvironment::SetYon(const hsTimedValue<hsScalar>& f) 
{ 
	if( fDevCache )
		fDevCache->Validate(false);
	fFlags |= kYonSet; 
	fYon = f; 
}

void hsGEnvironment::SetFogType(UInt32 t) 
{ 
	fFlags &= ~kFogTypeMask; 
	fFlags |= (t & kFogTypeMask); 
}

void hsGEnvironment::SetFogColorAmbient(hsBool32 on)
{
	if( on )
	{
		fFlags &= ~kFogColorDiffuse;
		fFlags |= kFogColorAmbient;
	}
	else
		fFlags &= ~kFogColorAmbient;
}

void hsGEnvironment::SetFogColorDiffuse(hsBool32 on)
{
	if( on )
	{
		fFlags &= ~kFogColorAmbient;
		fFlags |= kFogColorDiffuse;
	}
	else
		fFlags &= ~kFogColorDiffuse;
}

void hsGEnvironment::SetClearColorAmbient(hsBool32 on)
{
	if( on )
		fFlags |= kClearColorAmbient;
	else
		fFlags &= ~kClearColorAmbient;
}

void hsGEnvironment::SetOverride(hsBool32 on)
{
	if( on )
		fFlags |= kOverride;
	else
		fFlags &= ~kOverride;
}

void hsGEnvironment::SetIsFar(hsBool32 on)
{
	if( on )
		fFlags |= kFarOut;
	else
		fFlags &= ~kFarOut;
}

void hsGEnvironment::SetSortObjects(hsBool32 on)
{
	if( on )
		fFlags |= kSortObjects;
	else
		fFlags &= ~kSortObjects;
}

void hsGEnvironment::SetHasFogControl(hsBool32 on)
{
	if( on )
		fFlags |= kFogControl;
	else
		fFlags &= ~kFogControl;
}

void hsGEnvironment::SetFogControl(hsFogControl* fc)
{
	hsRefCnt_SafeAssign(fFogControl, fc);
}

void hsGEnvironment::SetDeviceCache(hsGDevEnvCache *p) 
{ 
	hsRefCnt_SafeAssign(fDevCache, p); 
}

hsGEnvironment *hsGEnvironment::Copy(hsGEnvironment *env)
{
	if( env->GetFlags() & kMapSet )
		SetMap(env->GetMap());
	else
		fFlags &= ~kMapSet;
	if( env->GetFlags() & kCenterSet )
		SetCenter(env->GetCenter());
	else
		fFlags &= ~kCenterSet;
	if( env->GetFlags() & kRadiusSet )
		SetRadius(env->GetRadius());
	else
		fFlags &= ~kRadiusSet;
	if( env->GetFlags() & kFogDepthSet )
		SetFogDepth(env->GetFogDepth());
	else
		fFlags &= ~kFogDepthSet;
	if( env->GetFlags() & kFogDensitySet )
		SetFogDensity(env->GetFogDensity());
	else
		fFlags &= ~kFogDensitySet;
	if( env->GetFlags() & kFogColorSet )
		SetFogColor(env->GetFogColor());
	else
		fFlags &= ~kFogColorSet;
	if( env->GetFlags() & kClearColorSet )
		SetClearColor(env->GetClearColor());
	else
		fFlags &= ~kClearColorSet;
	if( env->GetFlags() & kYonSet )
		SetYon(env->GetUnscaledYon());
	else
		fFlags &= ~kYonSet;

	if( env->GetFlags() & kCurrentDepthSet )
	{
		fCurrentDepth = env->GetCurrentDepth();
		fFlags |= kCurrentDepthSet;
	}

	fFlags &= ~hsGEnvironment::kFogTypeMask;
	fFlags |= env->GetFlags() & hsGEnvironment::kFogTypeMask;

	SetOverride(env->GetOverride());

	SetDeviceCache(env->GetDeviceCache());

	if( env->GetFlags() & kCacheInvalid )
		fFlags |= kCacheInvalid;
	else
		fFlags &= ~kCacheInvalid;

	int i;
	for( i = 0; i < env->GetNumRenderProcs(); i++ )
		AddRenderProc(env->GetRenderProc(i));

	return this;
}

void hsGEnvironment::MixEnvirons(hsGEnvironment *env, hsGEnvironment *def)
{
	if( env && (env->GetFlags() & kMapSet) )
	{
		SetMap(env->GetMap());
	}
	else if( def && (def->GetFlags() & kMapSet) )
	{
		hsGMaterial *map = def->GetMap();
		if( map )
		{
			SetMap(def->GetMap());
		}
		else
		{
			def->SetMapName(nil);
			hsRefCnt_SafeUnRef(fMap);
			fMap = nil;
			fFlags &= ~kMapSet;
		}
	}
	else
	{
		hsRefCnt_SafeUnRef(fMap);
		fMap = nil;
		fFlags &= ~kMapSet;
	}
	if( env && (env->GetFlags() & kCenterSet) )
		SetCenter(env->GetCenter());
	else if( def &&(def->GetFlags() & kCenterSet) )
		SetCenter(def->GetCenter());
	else
		fFlags &= ~kCenterSet;
	if( env && (env->GetFlags() & kRadiusSet) )
		SetRadius(env->GetRadius());
	else if( def &&(def->GetFlags() & kRadiusSet) )
		SetRadius(def->GetRadius());
	else
		fFlags &= ~kRadiusSet;

	if( env && (env->GetFlags() & kFogDepthSet) )
		SetFogDepth(env->GetFogDepth());
	else if( def &&(def->GetFlags() & kFogDepthSet) )
		SetFogDepth(def->GetFogDepth());
	else
		fFlags &= ~kFogDepthSet;

	if( env &&(env->GetFlags() & kCurrentDepthSet) )
	{
		fCurrentDepth = env->GetCurrentDepth();
		fFlags |= kCurrentDepthSet;
	}
	else
	if( def &&(def->GetFlags() & kCurrentDepthSet) )
	{
		fCurrentDepth = def->GetCurrentDepth();
		fFlags |= kCurrentDepthSet;
	}
	else
		fFlags &= ~kCurrentDepthSet;

	fFlags &= ~kFogTypeMask;
	if( env && (env->GetFlags() & kFogTypeMask)  )
		fFlags |= env->GetFlags() & kFogTypeMask;
	else if( def && (def->GetFlags() & kFogTypeMask) )
		fFlags |= def->GetFlags() & kFogTypeMask;
	else
		fFlags &= ~kFogTypeMask;

	if( env && (env->GetFlags() & kFogDensitySet) )
		SetFogDensity(env->GetFogDensity());
	else if( def &&(def->GetFlags() & kFogDensitySet) )
		SetFogDensity(def->GetFogDensity());
	else
		SetFogDensity(hsScalar1);

	if( env &&(env->GetFlags() & kFogColorSet) )
		SetFogColor(env->GetFogColor());
	else if( def &&(def->GetFlags() & kFogColorSet) )
		SetFogColor(def->GetFogColor());
	else
	{
		hsColorRGBA col;
		col.r = col.g = col.b = col.a = 0;
		SetFogColor(col);
	}
	if( env &&(env->GetFlags() & kClearColorSet) )
		SetClearColor(env->GetClearColor());
	else if( def &&(def->GetFlags() & kClearColorSet) )
		SetClearColor(def->GetClearColor());
	else
		fFlags &= ~kClearColorSet;

	if( env && (env->GetFlags() & kYonSet) )
		SetYon(env->GetUnscaledYon());
	else if( def &&(def->GetFlags() & kYonSet) )
		SetYon(def->GetUnscaledYon());
	else
		fFlags &= ~kYonSet;

	if( env && (env->GetOverride()) )
		SetOverride(true);
	else if( def &&(def->GetOverride()) )
		SetOverride(true);
	else
		SetOverride(false);

	if( env && env->GetDeviceCache() )
		SetDeviceCache(env->GetDeviceCache());
	else if( def && def->GetDeviceCache() )
		SetDeviceCache(def->GetDeviceCache());
	else
		SetDeviceCache(nil);

	int i;
	for( i = 0; i < GetNumRenderProcs(); i++ )
	{
		hsRefCnt_SafeUnRef(GetRenderProc(i));
	}
	fRenderProcs.Reset();
	if( env )
	{
		for( i = 0; i < env->GetNumRenderProcs(); i++ )
			AddRenderProc(env->GetRenderProc(i));
	}
	if( def )
	{
		for( i = 0; i < def->GetNumRenderProcs(); i++ )
			AddRenderProc(def->GetRenderProc(i));
	}

}

void hsGEnvironment::Blend()
{
	if( fFogControl )
		fFogControl->Blend();
}

void hsGEnvironment::Restore()
{
	if( fFogControl )
		fFogControl->Restore();
}

void hsGEnvironment::Init(hsSceneNode* node)
{
	if (!node)
	{
		char str[256];
		sprintf(str, "hsGEnvironment %s initted with nil room.  Weirdness may result.",
			GetKeyName());
		HSDebugProc(str);
	}
	else
	{
		if( fFogControl )
			fFogControl->Init(node);
	}

    SetResetState();
}

void hsGEnvironment::SaveFogState()
{
	FogState* curFogState = new FogState;
	curFogState->SetFromEnv(this);
	fFogStateStack.Push(curFogState);
}

void hsGEnvironment::RestoreFogState()
{
	FogState* curFogState = fFogStateStack.GetCount() ? fFogStateStack.Pop() : nil;
	if (curFogState)
	{
		curFogState->SetToEnv(this);
	}
	delete curFogState;
}

void hsGEnvironment::Read(hsStream *stream)
{
	fFlags = stream->ReadSwap32();

	if( fFlags & hsGEnvironment::kFogDistanceSet )
	{
		hsScalar d = stream->ReadSwapScalar();
		SetFogDistance(d);
	}
	if( fFlags & hsGEnvironment::kFogDepthSet )
	{
		hsScalar d = stream->ReadSwapScalar();
		SetFogDepth(d);
		SetCurrentDepth(d);
	}
	if( fFlags & hsGEnvironment::kFogDensitySet )
	{
		hsScalar d = stream->ReadSwapScalar();
		SetFogDensity(d);
	}
	if( fFlags & hsGEnvironment::kFogColorSet )
	{
		hsColorRGBA c;
		c.r = stream->ReadSwapScalar();
		c.g = stream->ReadSwapScalar();
		c.b = stream->ReadSwapScalar();
		c.a = stream->ReadSwapScalar();
		SetFogColor(c);
	}
	if( fFlags & hsGEnvironment::kClearColorSet )
	{
		hsColorRGBA c;
		c.r = stream->ReadSwapScalar();
		c.g = stream->ReadSwapScalar();
		c.b = stream->ReadSwapScalar();
		c.a = stream->ReadSwapScalar();
		SetClearColor(c);
	}
	if( fFlags & hsGEnvironment::kYonSet )
	{
		hsScalar d = stream->ReadSwapScalar();
		SetYon(d);
	}

	if( fFlags & hsGEnvironment::kCenterSet )
		fPos.Read(stream);
	if( fFlags & hsGEnvironment::kRadiusSet )
		fRadius = stream->ReadSwapScalar();
	// still don't write the map out

	if( !(fFlags & kFogTypeMask) )
	{
		if( GetFogDepth() <= 0.5f )
			fFlags |= kFogLinear;
		else
			fFlags |= kFogExp;
	}
}

void hsGEnvironment::Write(hsStream *stream)
{
	stream->WriteSwap32(fFlags);

	if( fFlags & hsGEnvironment::kFogDistanceSet )
		stream->WriteSwapScalar(GetFogDistance());
	if( fFlags & hsGEnvironment::kFogDepthSet )
		stream->WriteSwapScalar(GetFogDepth());
	if( fFlags & hsGEnvironment::kFogDensitySet )
		stream->WriteSwapScalar(GetFogDensity());
	if( fFlags & hsGEnvironment::kFogColorSet )
	{
		hsColorRGBA c = GetFogColor();
		stream->WriteSwapScalar(c.r);
		stream->WriteSwapScalar(c.g);
		stream->WriteSwapScalar(c.b);
		stream->WriteSwapScalar(c.a);
	}
	if( fFlags & hsGEnvironment::kClearColorSet )
	{
		hsColorRGBA c = GetClearColor();
		stream->WriteSwapScalar(c.r);
		stream->WriteSwapScalar(c.g);
		stream->WriteSwapScalar(c.b);
		stream->WriteSwapScalar(c.a);
	}
	if( fFlags & hsGEnvironment::kYonSet )
		stream->WriteSwapScalar(GetUnscaledYon());

	if( fFlags & hsGEnvironment::kCenterSet )
		fPos.Write(stream);
	if( fFlags & hsGEnvironment::kRadiusSet )
		stream->WriteSwapScalar(fRadius);
	// still don't write the map out

}

void hsGEnvironment::Write(hsStream* stream, hsResMgr* group)
{
    Write(stream);

	if( fFlags & hsGEnvironment::kHasRenderProcs )
	{
		int n = GetNumRenderProcs();
		stream->WriteSwap32(n);
		int i;
		for( i = 0; i < n; i++ )
		{
hsAssert(0,"Its Pauls fault");
//			plFactory::LabelAndWrite(stream, group, GetRenderProc(i));
		}
	}

    if ((GetFlags() & hsGEnvironment::kEnvironMapSet) == hsGEnvironment::kEnvironMapSet)
    {
        group->WriteKey(stream, GetMap());
    }        
    
    stream->WriteSwap32(fNodeKeys.GetCount());
    Int32 i;
    for (i = 0; i < fNodeKeys.GetCount(); i++)
    {
		group->WriteKey(stream, fNodeKeys[i]);
    }

	if( fFlags & hsGEnvironment::kFogControl )
		IWriteFogControl(stream, group);
}

void hsGEnvironment::Read(hsStream* stream, hsResMgr* group)
{
    Read(stream);
    
	if( fFlags & hsGEnvironment::kHasRenderProcs )
	{
		int n = stream->ReadSwap32();
		int i;
		for( i = 0; i < n; i++ )
		{
hsAssert(0,"Its Pauls fault");
//			hsGRenderProcs* rp = hsGRenderProcs::ConvertNoRef(plFactory::CreateAndRead(stream, group));
//			AddRenderProc(rp);
//			hsRefCnt_SafeUnRef(rp);
		}
	}

    if ((GetFlags() & hsGEnvironment::kEnvironMapSet) == hsGEnvironment::kEnvironMapSet)
    {
		plRefMsg* refMsg = new plRefMsg(GetKey(), plRefMsg::kOnCreate);
		group->ReadKeyNotifyMe(stream,refMsg);
    }
    
    hsAssert(fNodeKeys.GetCount() == 0, "fNodeKeys not empty in hsGEnvironment::Read.");
    Int32 nodeCount = stream->ReadSwap32();
    hsAssert(nodeCount > 0, "Environment node not in any rooms in hsGEnvironment::Read.");
    Int32 i;
    for (i = 0; i < nodeCount; i++)
    {
        plKey *key = group->ReadKey(stream);
        AddNodeKey(key);
    }

	if( fFlags & hsGEnvironment::kFogControl )
		IReadFogControl(stream, group);
}

void hsGEnvironment::IReadFogControl(hsStream* s, hsResMgr* mgr)
{
hsAssert(0,"Its Pauls fault");
//	hsFogControl* fc = hsFogControl::ConvertNoRef(plFactory::CreateAndRead(s, mgr));
//	SetFogControl(fc);
//	hsRefCnt_SafeUnRef(fc);

}

void hsGEnvironment::IWriteFogControl(hsStream* s, hsResMgr* mgr)
{
hsAssert(0,"Its Pauls fault");
//	plFactory::LabelAndWrite(s, mgr, fFogControl);
}

void hsGEnvironment::Push(hsG3DDevice* d)
{
#if 0 // Nuking old device - mf
	int i;

	for( i = 0; i < GetNumRenderProcs(); i++ )
		d->AddRenderProc(GetRenderProc(i));
#endif // Nuking old device - mf
}

void hsGEnvironment::Pop(hsG3DDevice* d)
{
#if 0 // Nuking old device - mf
	int i;
	for( i = 0; i < GetNumRenderProcs(); i++ )
		d->RemoveRenderProc(GetRenderProc(i));
#endif // Nuking old device - mf
}

void hsGEnvironment::AddRenderProc(hsGRenderProcs* rp) 
{ 
	hsRefCnt_SafeRef(rp); 
	fRenderProcs.Append(rp); 
	fFlags |= hsGEnvironment::kHasRenderProcs;
} 

hsGRenderProcs* hsGEnvironment::GetRenderProc(int i) 
{ 
	return fRenderProcs[i]; 
} 

UInt32 hsGEnvironment::GetNumRenderProcs() 
{ 
	return fRenderProcs.GetCount(); 
}

hsScalar hsGEnvironment::SetYonScale(hsScalar s)
{
	const hsScalar kMinYonScale = 0.05f;
	const hsScalar kMaxYonScale = 100.f;

	if( s < kMinYonScale )
		s = kMinYonScale;
	else if( s > kMaxYonScale )
		s = kMaxYonScale;
	return fYonScale = s;
}

hsBool hsGEnvironment::MsgReceive(plMessage* msg)
{
	plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
	
	if( refMsg )
	{

        hsGMaterial *mat = hsGMaterial::ConvertNoRef(refMsg->GetRef());
        SetMap(mat);
	}
	return false;
}