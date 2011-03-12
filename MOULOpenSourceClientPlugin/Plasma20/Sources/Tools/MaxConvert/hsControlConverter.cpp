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
#include "hsWindows.h"
#include <commdlg.h>
#include <math.h>

//#include "Max.h"
#include "../MaxMain/plMaxNode.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "istdplug.h"
#include "texutil.h"
#include "iparamb2.h"
#include "modstack.h"
#include "keyreduc.h"

#include "HeadSpin.h"

#include "hsMaxLayerBase.h"
#include "../../../Sources/Plasma/PubUtilLib/plInterp/plController.h"
#include "../../../Sources/Plasma/PubUtilLib/plInterp/hsInterp.h"
#include "../MaxExport/plErrorMsg.h"
#include "UserPropMgr.h"
#include "hsConverterUtils.h"
#include "hsControlConverter.h"
#include "hsMaterialConverter.h"
#include "hsExceptionStack.h"
#include "../MaxExport/plErrorMsg.h"
#include "../../Tools/MaxComponent/plNoteTrackAnim.h"
#include "../MaxComponent/plCameraComponents.h"
#include "../MaxComponent/plAnimComponent.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

extern UserPropMgr gUserPropMgr;

hsControlConverter& hsControlConverter::Instance()
{
	static hsControlConverter the_instance;

	return the_instance;
}

hsControlConverter::hsControlConverter() : fConverterUtils(hsConverterUtils::Instance())
{
}

void hsControlConverter::Init(plErrorMsg* msg)
{
	hsGuardBegin("hsControlConverter::Init");

    fInterface = GetCOREInterface();
    fErrorMsg = msg;

	fTicksPerFrame	= ::GetTicksPerFrame();	/*160*/
	fFrameRate		= ::GetFrameRate();		/*30*/
	fTicksPerSec		= fTicksPerFrame*fFrameRate;

	Interval interval = fInterface->GetAnimRange();
	fStartFrame	= interval.Start()/fTicksPerFrame;
	fEndFrame	= interval.End()/fTicksPerFrame;
	fNumFrames	= fEndFrame-fStartFrame+1;
	fAnimLength = (float)(fNumFrames-1)/fFrameRate;

	fWarned = false;

    fForceLocal = false;

	fSegStart = fSegEnd = -1;

	hsGuardEnd;
}

void hsControlConverter::DeInit()
{
}

// dummy class that ApplyKeyReduction needs
class KRStatus : public KeyReduceStatus
{
	void Init(int total) {}
	int Progress(int p) { return KEYREDUCE_CONTINUE; }
};

void hsControlConverter::ReduceKeys(Control *control, hsScalar threshold)
{
	if (control == nil || threshold <= 0)
		return;

	KRStatus status;
	if (control->IsLeaf())
	{
		if (control->IsKeyable())
		{
			IKeyControl *keyCont = GetKeyControlInterface(control);
			if (keyCont->GetNumKeys() > 2)
			{
				IKey *key1 = (IKey*)TRACKED_NEW UInt8[keyCont->GetKeySize()];
				IKey *key2 = (IKey*)TRACKED_NEW UInt8[keyCont->GetKeySize()];
				keyCont->GetKey(0, key1);
				keyCont->GetKey(keyCont->GetNumKeys() - 1, key2);

				// We want the interval to be one frame past the start and one frame 
				// before the end, to guarantee we leave the first and last keys 
				// alone. This will make sure a looping anim still lines up, and 
				// also prevents us from removing the controller entirely and thinking
				// this channel just isn't animated at all.
				//
				// Also, I think this is a Max bug (since we're using Max's key reduce 
				// function, and the same error happens without our plugins), but if 
				// your range is only one frame short of the end of the anim, some 
				// bones get flipped on that 2nd-to-last frame. So you get a single 
				// frame with something like your arm pointing in the opposite 
				// direction at the elbow.
				TimeValue start = key1->time + GetTicksPerFrame();
				TimeValue end = key2->time - 2 * GetTicksPerFrame();
				if (start < end)
				{
					Interval interval(start, end);
					ApplyKeyReduction(control, interval, threshold, GetTicksPerFrame(), &status);
				}
				delete [] (UInt8*)key1;
				delete [] (UInt8*)key2;
			}
		}
	}
	else
	{
		int i;
		for (i = 0; i < control->NumSubs(); i++)
			ReduceKeys((Control*)control->SubAnim(i), threshold);
	}
}

plController *hsControlConverter::ConvertTMAnim(plSceneObject *obj, plMaxNode *node, hsAffineParts *parts, 
												hsScalar start /* = -1 */, hsScalar end /* = -1 */)
{
	Control* maxTm = node->GetTMController();
	plController *tmc = hsControlConverter::Instance().MakeTransformController(maxTm, node, start, end);

	if (tmc)
	{
		const plCoordinateInterface *ci = obj->GetCoordinateInterface();
		if(ci)
		{
			const hsMatrix44& loc2Par = ci->GetLocalToParent();

			gemAffineParts ap;
			decomp_affine(loc2Par.fMap, &ap); 

			AP_SET((*parts), ap);
		}
	}

	return tmc;
}

hsBool hsControlConverter::HasKeyTimes(Control* ctl)
{
	hsGuardBegin("hsControlConverter::HasKeyTimes");

	if( !ctl )
	{
		return false;
	}

	return (ctl->NumKeys() > 1);
	hsGuardEnd;
}


plLeafController* hsControlConverter::MakeMatrix44Controller(StdUVGen* uvGen, const char* nodeName)
{
	hsGuardBegin("hsControlConverter::MakeMatrix44Controller");

	if( !uvGen )
		return nil;

	ISetSegRange(-1, -1);

	Tab<TimeValue> kTimes;
	kTimes.ZeroCount();
	Control* uScaleCtl = nil;
	Control* vScaleCtl = nil;
	Control* uOffCtl= nil;
	Control* vOffCtl = nil;
	Control* rotCtl = nil;
	GetControllerByName(uvGen, TSTR("U Offset"), uOffCtl);
	GetControllerByName(uvGen, TSTR("V Offset"), vOffCtl);
	GetControllerByName(uvGen, TSTR("U Tiling"), uScaleCtl);
	GetControllerByName(uvGen, TSTR("V Tiling"), vScaleCtl);
	GetControllerByName(uvGen, TSTR("Angle"), rotCtl);

	// new with Max R2, replacing "Angle", but it doesn't hurt to look...
	Control* uAngCtl = nil;
	Control* vAngCtl = nil;
	Control* wAngCtl = nil; 
	GetControllerByName(uvGen, TSTR("U Angle"), uAngCtl);
	GetControllerByName(uvGen, TSTR("V Angle"), vAngCtl);
	GetControllerByName(uvGen, TSTR("W Angle"), wAngCtl);

	int i;

	CompositeKeyTimes(uOffCtl, kTimes);
	CompositeKeyTimes(vOffCtl, kTimes);
	CompositeKeyTimes(uScaleCtl, kTimes);
	CompositeKeyTimes(vScaleCtl, kTimes);
	CompositeKeyTimes(rotCtl, kTimes);
	CompositeKeyTimes(uAngCtl, kTimes);
	CompositeKeyTimes(vAngCtl, kTimes);
	CompositeKeyTimes(wAngCtl, kTimes);

	const float kMaxRads = 30.f * hsScalarPI / 180.f;
	MaxSampleAngles(nodeName, uAngCtl, kTimes, kMaxRads);
	MaxSampleAngles(nodeName, vAngCtl, kTimes, kMaxRads);
	MaxSampleAngles(nodeName, wAngCtl, kTimes, kMaxRads);

	if( kTimes.Count()<2 )
	{
		return nil;
	}

	plLeafController* ctrl = TRACKED_NEW plLeafController;
	ctrl->AllocKeys(kTimes.Count(), hsKeyFrame::kMatrix44KeyFrame);
	TimeValue resetTime = fConverterUtils.GetTime(fInterface);
	for( i=0; i < kTimes.Count(); i++)
	{
		Interval v;
		uvGen->Update(kTimes[i], v);

		// Get key
		float secs	= (float)kTimes[i]/fTicksPerSec;
		int frameNum= kTimes[i]/fTicksPerFrame;
		hsAssert(frameNum <= hsKeyFrame::kMaxFrameNumber, "Anim is too long.");

		fErrorMsg->Set((frameNum < fStartFrame || frameNum > fEndFrame), nodeName, 
			"Warning: Skipping keyframes outside of animation interval").CheckAndAsk();
		
		hsMatrix44Key *key = ctrl->GetMatrix44Key(i);
		StdUVGenToHsMatrix44(&key->fValue, uvGen, true);	
		key->fFrame = frameNum;
	}

	return ctrl;
	hsGuardEnd;
}

plLeafController* hsControlConverter::MakeMatrix44Controller(Control* prsControl)
{
	hsGuardBegin("hsControlConverter::MakeMatrix44Controller");

	ISetSegRange(-1, -1);

	Tab<TimeValue> kTimes;
	kTimes.ZeroCount();

    Control* posCtl = nil;
	Control* scaleCtl = nil;
	Control* rotCtl = nil;
    posCtl = prsControl->GetPositionController();
    rotCtl = prsControl->GetRotationController();
    scaleCtl = prsControl->GetScaleController();
    int i;

	CompositeKeyTimes(posCtl, kTimes);
	CompositeKeyTimes(scaleCtl, kTimes);
	CompositeKeyTimes(rotCtl, kTimes);

	if( kTimes.Count()<2 )
	{
		return nil;
	}

	plLeafController* ctrl = TRACKED_NEW plLeafController;
	ctrl->AllocKeys(kTimes.Count(), hsKeyFrame::kMatrix44KeyFrame);
	TimeValue resetTime = fConverterUtils.GetTime(fInterface);;
	for( i=0; i < kTimes.Count(); i++)
	{
		// Get key
		float secs	= (float)kTimes[i]/fTicksPerSec;
		int frameNum= kTimes[i]/fTicksPerFrame;
		hsAssert(frameNum <= hsKeyFrame::kMaxFrameNumber, "Anim is too long.");
	
        Matrix3 maxXform;
        maxXform.IdentityMatrix();
        Interval valid = FOREVER;
        prsControl->GetValue(fConverterUtils.GetTime(fInterface), &maxXform, valid, CTRL_RELATIVE);

		hsMatrix44Key *key = ctrl->GetMatrix44Key(i);
        Matrix3ToHsMatrix44(&maxXform, &key->fValue);
		key->fFrame = frameNum;
	}

	return ctrl;
	hsGuardEnd;
}


//
// Create a plScalarController and store the nodes parm behavior in it.
//
plLeafController* hsControlConverter::MakeScalarController(Control* control, plMaxNode* node, 
														   hsScalar start /* = -1 */, hsScalar end /* = -1 */)
{
	hsGuardBegin("hsControlConverter::MakeScalarController");

	if (control == NULL)
		return NULL;

	ISetSegRange(start, end);

	return ICreateScalarController(node, control);

	hsGuardEnd;
}

plController* hsControlConverter::MakeColorController(Control* control, plMaxNode* node, 
													  hsScalar start /* = -1 */, hsScalar end /* = -1 */)
{
	return MakePosController(control, node, start, end);
}
//
// Create a plPosController and store the nodes parm behavior in it.
//
plController* hsControlConverter::MakePosController(Control* control, plMaxNode* node, 
													hsScalar start /* = -1 */, hsScalar end /* = -1 */)
{
	hsGuardBegin("hsControlConverter::MakePosController");

	if (control == NULL)
		return NULL;

	ISetSegRange(start, end);

	plController* hsCont;

	if (control->IsLeaf())
	{
		hsCont = ICreateSimplePosController(node, control);
	}
	else
	{
		hsCont = TRACKED_NEW plCompoundController;
		fErrorMsg->Set(control->NumSubs()!=3, node->GetName(), "compound should have 3 subs").Check();

		if (control->ClassID() == Class_ID(POSITIONNOISE_CONTROL_CLASS_ID,0) )
		{
			MessageBox(GetActiveWindow(), node->GetName(), 
			"Warning: Noise position controller not supported.  Ignoring.", MB_OK);
			return hsCont;
		}
		
		hsBool keep = false;
		for (int i=0; i<3; i++)
		{
			Control* sub = (Control*)control->SubAnim(i);
			plLeafController* sc = ICreateScalarController(node, sub);
			((plCompoundController*)hsCont)->SetController(i, sc);
			if (sc)
			{
				keep = true;
			}
		}
		if (!keep)
		{
			delete hsCont;
			hsCont = nil;
		}
	}

	return hsCont;
	hsGuardEnd;
}

plController *hsControlConverter::MakeScaleController(Control *control, plMaxNode* node, 
													  hsScalar start /* = -1 */, hsScalar end /* = -1 */)
{
	ISetSegRange(start, end);

	if (control->IsLeaf())
	{
		// Simple scale: linear, bezier, tcb
		plLeafController* sc = ICreateSimpleScaleController(node, control);
		return sc;
	}
	else
	{
		// compound scale: noise
		if (control->ClassID() == Class_ID(SCALENOISE_CONTROL_CLASS_ID,0) )
		{
			MessageBox(GetActiveWindow(), node->GetName(), 
					   "Warning: Noise scale controller not supported.  Ignoring.", MB_OK);
		}
	}
	return NULL;
}

plController *hsControlConverter::MakeRotController(Control *control, plMaxNode *node, hsBool camRot /* = false */, 
													hsScalar start /* = -1 */, hsScalar end /* = -1 */)
{
	ISetSegRange(start, end);

	if (control->IsLeaf())
	{
		// simple rot: linear, smooth, tcb
		plLeafController* rc = ICreateSimpleRotController(node, control, camRot);
		return rc;
	}
	else
	{
		// compound rot: euler or noise
		if (control->NumSubs())
		{
			if (control->ClassID() == Class_ID(ROTATIONNOISE_CONTROL_CLASS_ID,0) )
			{
				MessageBox(GetActiveWindow(), node->GetName(), 
					"Warning: Noise rotation controller not supported.  Ignoring.", MB_OK);
				return nil;
			}
			if (fErrorMsg->Set(control->ClassID() != Class_ID(EULER_CONTROL_CLASS_ID,0), 
				node->GetName(), "Expecting euler rot ctrler").CheckAndAsk())
				return nil;

			if (fErrorMsg->Set(control->NumSubs() != 3, node->GetName(), "Rot compound controller should have 3 subcontrollers").CheckAndAsk())
				return nil;

			plCompoundController* rc = TRACKED_NEW plCompoundController;
			int i;
			for (i=0; i<3; i++)
			{
				Control* sub = (Control*)control->SubAnim(i);
				plLeafController* sc = ICreateScalarController(node, sub);
				rc->SetController(i, sc);
			}

			//
			// Check if we need to fixup euler due to missing subcontrollers
			//
			int numRotConts;
			for(numRotConts=0,i=0; i<3; i++)
				if (rc->GetController(i))
					numRotConts++;
			if (numRotConts>0 && numRotConts<3)
			{
				// Someone has deleted 1 or 2 of the subcontrollers
				// Add a key at the start and end of the missing tracks
				Interval interval = fInterface->GetAnimRange();
				TimeValue startTime = interval.Start();	// in ticks
				TimeValue endTime = interval.End();		// in ticks

				hsStatusMessage("Fixing up euler controller due to missing subcontrollers\n");
				for(i=0; i<3; i++)
				{
					if (!rc->GetController(i))
					{
						Control* sub = (Control*)control->SubAnim(i);
						if (!sub)
							continue;
						sub->AddNewKey(startTime, ADDKEY_INTERP);
						sub->AddNewKey(endTime, ADDKEY_INTERP);
						plLeafController* sc = ICreateScalarController(node, sub);
						if (sc)
							rc->SetController(i, sc);
						else
						{
							fErrorMsg->Set(true, "Scalar Controller Error", "nil plScalar controller").Show();
							fErrorMsg->Set();
							return rc;
						}
										
					}
				}
				for(numRotConts=0,i=0; i<3; i++)
					if (rc->GetController(i))
						numRotConts++;
		
				if(numRotConts != 3)
				{
					fErrorMsg->Set(true, "Euler Fixup Error", "Euler fixup failed.").Show();
					fErrorMsg->Set();
					return rc;
				}
			}
			else if (numRotConts == 0) // No sub controllers, no point in having the compound controller then
			{
				delete rc;
				rc = nil;
			}

			return rc;
		}
	}
	return NULL;
}

void hsControlConverter::ScalePositionController(plController* ctl, hsScalar scale)
{
	plLeafController* simp = plLeafController::ConvertNoRef(ctl);
	plCompoundController* comp;
	int i;
	if( simp )
	{
		for( i = 0; i < simp->GetNumKeys(); i++ )
		{
			hsPoint3Key* key = simp->GetPoint3Key(i);
			if (key)
			{
				key->fValue *= scale;
			}

			hsBezPoint3Key* bezKey = simp->GetBezPoint3Key(i);
			if (bezKey)
			{
				bezKey->fInTan *= scale;
				bezKey->fOutTan *= scale;
				bezKey->fValue *= scale;
			}
		}
	}
	else if( comp = plCompoundController::ConvertNoRef(ctl) )
	{
		for( i = 0; i < 3; i++ )
		{
			ScalePositionController(comp->GetController(i), scale);
		}
	}
}

void hsControlConverter::MaxSampleAngles(const char* nodeName, Control* ctl, Tab<TimeValue>& kTimes, hsScalar maxRads)
{
	hsGuardBegin("hsControlConverter::MaxSampleAngles");

	if( !ctl )
	{
		return;
	}

	Tab<TimeValue> rTimes;
	Tab<TimeValue> sTimes;
	rTimes.ZeroCount();
	IGetControlSampleTimes(ctl, 0, ctl->NumKeys(), rTimes, maxRads);

	int iR;
	for( iR = 0; iR < rTimes.Count(); iR++ )
	{
		int iK;
		for( iK = 0; iK < kTimes.Count(); iK++ )
		{
			if( kTimes[iK] >= rTimes[iR] )
				break;
		}
		if( kTimes[iK] != rTimes[iR] )
			kTimes.Insert(iK, 1, rTimes.Addr(iR));
	}

	hsGuardEnd;
}

plCompoundController *hsControlConverter::MakeTransformController(Control *control, plMaxNode *node, 
																  hsScalar start /* = -1 */, hsScalar end /* = -1 */)
{
	hsGuardBegin("hsControlConverter::MakeTransformController");

	if (!control)
		return NULL;

	ISetSegRange(start, end);

	Class_ID cid = control->ClassID();
	if (cid == Class_ID(PRS_CONTROL_CLASS_ID,0) ||
		cid == Class_ID(LOOKAT_CONTROL_CLASS_ID,0))
	{
		int n = control->NumSubs();
		if(n != 3)
		{
			fErrorMsg->Set(true, "Transform Controller Error", "Transform controller doesn't have 3 sub controllers").Show();
			fErrorMsg->Set();
			return NULL;
		}

		plCompoundController *tmc = TRACKED_NEW plCompoundController;
		for (int i=0; i<n; i++)
		{
			Control* sub = (Control*)control->SubAnim(i);
			if (sub)
			{
				IConvertSubTransform(sub, control->SubAnimName(i), node, tmc, start, end);
			}
		}

		if (cid == Class_ID(LOOKAT_CONTROL_CLASS_ID,0))
		{
			hsTArray<hsG3DSMaxKeyFrame> kfArray;
			IAddPartsKeys(control, &kfArray, node);
			hsBool ignoreFOV = false;
			for (int i = 0; i < node->NumAttachedComponents(); i++)
			{
				if (node->GetAttachedComponent(i)->ClassID() == ANIMCAM_CMD_CID)
				{
					plCameraAnimCmdComponent* pAnimComp = (plCameraAnimCmdComponent*)node->GetAttachedComponent(i);
					ignoreFOV = pAnimComp->IgnoreFOV();
					break;
				}
			}
			if (!ignoreFOV)
				IExportAnimatedCameraFOV(node, &kfArray);	
		}

		if (tmc->GetPosController() || tmc->GetRotController() || tmc->GetScaleController())
			return tmc;
		else
		{
			delete tmc;
			return NULL;
		}
	}
	return NULL;

	hsGuardEnd;
}

void hsControlConverter::ISetSegRange(hsScalar start, hsScalar end)
{
	fSegStart = (start >= 0 ? fTicksPerSec * start : fInterface->GetAnimRange().Start());
	fSegEnd = (end >= 0 ? fTicksPerSec * end : fInterface->GetAnimRange().End());
}


void hsControlConverter::IConvertSubTransform(Control *control, char *ctlName, plMaxNode *node, plCompoundController *tmc,
											  hsScalar start, hsScalar end)
{
	if (control)
	{
		ControllerType ct = IGetControlType(ctlName);

		switch(ct)
		{
		case ctrlTypePosition:
			{
				if(tmc->GetPosController() != nil)
				{
					fErrorMsg->Set(true, "Position Controller Error", "Non-nil position controller").Show();
					fErrorMsg->Set();
					return;
				}
				tmc->SetPosController(MakePosController(control, node, start, end));
			}
			break;
		case ctrlTypeRollAngle:
		case ctrlTypeRotation:
			{
				if(tmc->GetRotController() != nil)
				{
					fErrorMsg->Set(true, "Position Controller Error", "Non-nil Rotation controller").Show();
					fErrorMsg->Set();
					return;
				}
				hsBool camRot = (ct == ctrlTypeRollAngle);
				tmc->SetRotController(MakeRotController(control, node, camRot, start, end));
			}
			break;
		case ctrlTypeScale:
			{
				if(tmc->GetScaleController() != nil)
				{
					fErrorMsg->Set(true, "Scale Controller Error", "Non-nil Scale Controller").Show();
					fErrorMsg->Set();
					return;
				}
				tmc->SetScaleController(MakeScaleController(control, node, start, end));							
			}
			break;
		default:
            /*
			if (plExp.GetLogFile())
				fprintf(plExp.GetLogFile(),"%s unknown ctrl type=%d\n", node->GetName(), (int)ct);
               */
            break;
		}
	}
}

//
//
//
plLeafController* hsControlConverter::ICreateSimpleRotController(plMaxNode* node, Control* control, hsBool camRot)
{
	hsGuardBegin("hsControlConverter::ICreateSimpleRotController");

	return ICreateQuatController(node, control, true, camRot);

	hsGuardEnd;
}

plLeafController* hsControlConverter::ICreateSimpleScaleController(plMaxNode* node, Control* control)
{
	hsGuardBegin("hsControlConverter::ICreateSimpleScaleController");

	return ICreateScaleValueController(node, control);

	hsGuardEnd;
}


//
//
//
plLeafController* hsControlConverter::ICreateQuatController(plMaxNode* node, Control* control, bool rotation, hsBool camRot)
{
	hsGuardBegin("hsControlConverter::ICreateQuatController");

	Int32 startIdx, endIdx;
	IKeyControl* ikeys = GetKeyControlInterface(control);
	if ( ikeys && IGetRangeCoverKeyIndices(node ? node->GetName() : nil, control, startIdx, endIdx)>1 )
	{
		if(!(control->IsKeyable()))
		{
			fErrorMsg->Set(true, "Quat Controller Creation Error", "Control is not keyable.").Show();
			fErrorMsg->Set();
			return NULL;
		}

		IKey* key=(IKey*)(new byte[ikeys->GetKeySize()]);
		plLeafController* pc = TRACKED_NEW plLeafController;

		UInt8 compressLevel = node->GetAnimCompress();
		UInt8 keyType;
		if (compressLevel == plAnimCompressComp::kCompressionHigh)
			keyType = hsKeyFrame::kCompressedQuatKeyFrame32;
		else if (compressLevel == plAnimCompressComp::kCompressionLow)
			keyType = hsKeyFrame::kCompressedQuatKeyFrame64;
		else
			keyType = hsKeyFrame::kQuatKeyFrame;

		pc->AllocKeys(endIdx - startIdx + 1, keyType);
		for(int i = startIdx; i <= endIdx; i++)
		{
			// Get key
			ikeys->GetKey(i, key);
			const float kMaxRads = hsScalarPI*  0.5f;
			Tab<TimeValue> kTimes;
			kTimes.ZeroCount();
			if( rotation )
				IGetControlSampleTimes(control, i, i, kTimes, kMaxRads);
			else
				kTimes.Append(1, &key->time);

			int k;
			for( k = 0; k < kTimes.Count(); k++ )
			{
				if (keyType == hsKeyFrame::kQuatKeyFrame)
				{
					hsQuatKey *hsKey = pc->GetQuatKey(i - startIdx);
					ICreateHSInterpKey(control, key, kTimes[k], hsKey, node, camRot);
				}
				else if (keyType == hsKeyFrame::kCompressedQuatKeyFrame64)
				{
					hsQuatKey tempKey;
					ICreateHSInterpKey(control, key, kTimes[k], &tempKey, node, camRot);
					hsCompressedQuatKey64 *compKey = pc->GetCompressedQuatKey64(i - startIdx);
					compKey->fFrame = tempKey.fFrame;
					compKey->SetQuat(tempKey.fValue);
				}
				else
				{
					hsQuatKey tempKey;
					ICreateHSInterpKey(control, key, kTimes[k], &tempKey, node, camRot);
					hsCompressedQuatKey32 *compKey = pc->GetCompressedQuatKey32(i - startIdx);
					compKey->fFrame = tempKey.fFrame;
					compKey->SetQuat(tempKey.fValue);
				}
			}
		}
		delete [] key;

		return pc;
	}

	return nil;
	hsGuardEnd;
}


//
//
//
plLeafController* hsControlConverter::ICreateScaleValueController(plMaxNode* node, Control* control)
{
	hsGuardBegin("hsControlConverter::ICreateScaleValueController");

	//plMaxNode* xformParent = GetXformParent(node);
	Int32 startIdx, endIdx;
	IKeyControl* ikeys = GetKeyControlInterface(control);
	if ( ikeys && IGetRangeCoverKeyIndices(node ? node->GetName() : nil, control, startIdx, endIdx)>1 )
	{
		if(!(control->IsKeyable()))
		{
			fErrorMsg->Set(true, "Scale Value Controller Creation Error", "Control is not keyable").Show();
			fErrorMsg->Set();
			return NULL;
		}

		IKey* key=(IKey*)(new byte [ikeys->GetKeySize()]);
		plLeafController* pc = TRACKED_NEW plLeafController;
		pc->AllocKeys(endIdx - startIdx + 1, GetKeyType(control));
		for(int i = startIdx; i <= endIdx; i++)
		{
			// Get key
			ikeys->GetKey(i, key);
			hsScaleKey *hsKey = pc->GetScaleKey(i - startIdx);
			if (hsKey)
				ICreateHSInterpKey(control, key, key->time, hsKey, node);

			hsBezScaleKey *bezKey = pc->GetBezScaleKey(i - startIdx);
			if (bezKey)
				ICreateHSInterpKey(control, key, key->time, bezKey, node);
		}
		delete [] key;
		return pc;
	}
	
	return nil;
	hsGuardEnd;
}

//
//
//
plLeafController* hsControlConverter::ICreateScalarController(plMaxNode* node, Control* control)
{
	hsGuardBegin("hsControlConverter::ICreateScalarController");

	Int32 startIdx, endIdx;
	IKeyControl* ikeys = GetKeyControlInterface(control);
	if ( ikeys && IGetRangeCoverKeyIndices(node ? node->GetName() : nil, control, startIdx, endIdx)>1 )
	{
		if(!(control->IsKeyable()))
		{
			fErrorMsg->Set(true, "Scale Value Controller Creation Error", "Control is not keyable").Show();
			fErrorMsg->Set();
			return NULL;
		}

		IKey* key=(IKey*)(new byte [ikeys->GetKeySize()]);
		plLeafController* pc = TRACKED_NEW plLeafController;
		pc->AllocKeys(endIdx - startIdx + 1, GetKeyType(control));
		for(int i = startIdx; i <= endIdx; i++)
		{
			// Get key
			ikeys->GetKey(i, key);
			hsScalarKey *hsKey = pc->GetScalarKey(i - startIdx);
			if (hsKey)
				ICreateHSInterpKey(control, key, key->time, hsKey);

			hsBezScalarKey *bezKey = pc->GetBezScalarKey(i - startIdx);
			if (bezKey)
				ICreateHSInterpKey(control, key, key->time, bezKey);
		}

		delete [] key;
		return pc;
	}
	return nil;
	hsGuardEnd;
}


//
//
//
plLeafController* hsControlConverter::ICreateSimplePosController(plMaxNode* node, Control* control)
{
	hsGuardBegin("hsControlConverter::ICreateSimplePosController");

	IKeyControl* ikeys = GetKeyControlInterface(control);
	Int32 startIdx, endIdx;
	if ( ikeys && IGetRangeCoverKeyIndices(node ? node->GetName() : nil, control, startIdx, endIdx)>1 )
	{
		if(!(control->IsKeyable()))
		{
			fErrorMsg->Set(true, "Simple Position Controller Creation Error", "Control is not keyable").Show();
			fErrorMsg->Set();
			return NULL;
		}

		IKey* key=(IKey*)(new byte [ikeys->GetKeySize()]);
		plLeafController* pc = TRACKED_NEW plLeafController;
		pc->AllocKeys(endIdx - startIdx + 1, GetKeyType(control));
		for(int i = startIdx; i <= endIdx; i++)
		{
			// Get key
			ikeys->GetKey(i, key);
			hsPoint3Key *hsKey = pc->GetPoint3Key(i - startIdx);
			if (hsKey)
				ICreateHSInterpKey(control, key, key->time, hsKey);

			hsBezPoint3Key *bezKey = pc->GetBezPoint3Key(i - startIdx);
			if (bezKey)
				ICreateHSInterpKey(control, key, key->time, bezKey);
		}
		delete [] key;
		return pc;
	}

	return nil;
	hsGuardEnd;
}

//
// Create a hsKey and store the nodes LTM in it.
// Recurses along all subcontrollers.
//
int hsControlConverter::IAddPartsKeys(Control* control, 
				  hsTArray <hsG3DSMaxKeyFrame>* kfArray, 
				  plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::IAddPartsKeys");

	Int32 startIdx, endIdx;
	if (control->IsLeaf())
	{
		IKeyControl* ikeys = GetKeyControlInterface(control);
		int num = ikeys ? IGetRangeCoverKeyIndices(node ? node->GetName() : nil, control, startIdx, endIdx) : 0;
		
		if (num<2)
		{
			return 0;
		}

		if(!control->IsKeyable())
		{
			fErrorMsg->Set(true, "Add Parts Keys Creation Error", "Control is not keyable").Show();
			fErrorMsg->Set();
			return 0;
		}

		int i,j;

		//
		// Traverse all keys of controller
		//
		IKey* key=(IKey*)(new byte [ikeys->GetKeySize()]);
		hsBool mb=false;
		plMaxNode* xformParent = GetXformParent(node);
		for(i = startIdx; i <= endIdx; i++)
		{
			// Get key
			ikeys->GetKey(i, key);
			hsScalar frameTime = key->time / GetTicksPerSec();
			int frameNum = key->time / GetTicksPerFrame();
			hsAssert(frameNum <= hsKeyFrame::kMaxFrameNumber, "Anim is too long.");

			// Check if we already have a hsG3dsMaxKey at this frameNum
			int found=FALSE;
			for(j=0; j<kfArray->GetCount(); j++)
			{
				hsG3DSMaxKeyFrame* k = &(*kfArray)[j];
				if (k->fFrame == frameNum)
				{
					found = TRUE;
					break;
				}
			}
			if (found==TRUE)
				// Skip this key, there's one already there
				continue;

			//
			// Compute AffineParts
			//
			hsMatrix44 tXform = node->GetLocalToParent44(key->time);

			gemAffineParts ap;
			decomp_affine(tXform.fMap, &ap); 
			hsAffineParts parts;
			AP_SET(parts, ap);

			// Init new keyframe
			hsG3DSMaxKeyFrame hKey;
			hKey.fParts = parts;
			hKey.fFrame = frameNum;

			// Add key to list
			kfArray->Append(hKey);
		}
		delete [] key;
	}
	else
	{
		int i;
		for (i = 0; i < control->NumSubs(); i++)
			IAddPartsKeys((Control *)control->SubAnim(i), kfArray, node);
	}

	return kfArray->GetCount();
	hsGuardEnd;
}

Matrix3 hsControlConverter::StdUVGenToMatrix3(StdUVGen* uvGen)
{
	Matrix3 retVal(true);
	if( uvGen )
		uvGen->GetUVTransform(retVal);

	retVal = Inverse(IFlipY()) * retVal * IFlipY();

	return retVal;
}

//
//
// returns 0 if identity, 1 otherwise
// takes into account the implicit transform of v -> 1-v in meshconvert:setuvs()
//		
bool hsControlConverter::StdUVGenToHsMatrix44(hsMatrix44* hsMat, StdUVGen* uvGen, bool preserveOffset)
{
	hsGuardBegin("hsControlConverter::StdUVGenToHsMatrix44");

	Matrix3 uvXform;
	uvGen->GetUVTransform(uvXform);

	uvXform = Inverse(IFlipY()) * uvXform * IFlipY();
	Matrix3ToHsMatrix44(&uvXform, hsMat);

	if( !preserveOffset )
	{
		int i;
		for( i = 0; i < 2; i++ )
		{
			if( fabsf(hsMat->fMap[i][3]) > 1.f )
				hsMat->fMap[i][3] -= hsScalar(int(hsMat->fMap[i][3]));
		}
	}

	return ( !hsMat->IsIdentity() );
	hsGuardEnd;
}

void hsControlConverter::IGetControlSampleTimes(Control* control, int iLo, int iHi, Tab<TimeValue>& kTimes, float maxRads)
{
	hsGuardBegin("hsControlConverter::IGetControlSampleTimes");

	kTimes.ZeroCount();

	if( !control )
	{
		return;
	}

	Class_ID cID = control->ClassID();
	SClass_ID sID = control->SuperClassID();

	if( iLo < 0 )
		iLo = 0;
	int num = control->NumKeys();
	iHi++;
	if( iHi > num )
		iHi = num;


	IKeyControl* ikeys = GetKeyControlInterface(control);
	IKey* key=(IKey*)(new byte [ikeys->GetKeySize()]);
	IKey* lastKey=(IKey*)(new byte [ikeys->GetKeySize()]);

	int i;
	for( i = iLo; i < iHi; i++ )
	{
		TimeValue t = control->GetKeyTime(i);

		if( !i )
		{
			kTimes.Append(1, &t);
			continue;
		}
		int nSamp = 1;
		float rads = 0;
		// following code will work, except that rotations are stored
		// relative to previous key, so we'd need to end off with something
		// like for i = 1; i < n; i++ )
		//		key[i] = key[i-1] * key[i]
		// or pass in the previous key and do it here.
		///////////////////////////////////////
		ikeys->GetKey(i-1, lastKey);
		ikeys->GetKey(i, key);
		if( cID == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0) )  
		{
			ITCBRotKey* tcbRotKey = (ITCBRotKey*)key;
			rads = tcbRotKey->val.angle;
		}
		else 
		if( cID == Class_ID(LININTERP_ROTATION_CLASS_ID, 0) )  
		{
			ILinRotKey* linRotKey = (ILinRotKey*)key;

			Point3 axis;
			AngAxisFromQ(linRotKey->val, &rads, axis);
		}
		else 
		if( cID == Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0) )  
		{
			IBezQuatKey* bezRotKey = (IBezQuatKey*)key;

			Point3 axis;
			AngAxisFromQ(bezRotKey->val, &rads, axis);
		}
		else 
		if( cID == Class_ID(TCBINTERP_FLOAT_CLASS_ID, 0) )
		{
			ITCBFloatKey* fKey = (ITCBFloatKey*)key;

			rads = fKey->val;
			fKey = (ITCBFloatKey*)lastKey;
			rads -= fKey->val;
		}
		else
		if( cID == Class_ID(LININTERP_FLOAT_CLASS_ID, 0) )
		{
			ILinFloatKey* fKey = (ILinFloatKey*)key;
			rads = fKey->val;
			fKey = (ILinFloatKey*)lastKey;
			rads -= fKey->val;
		}
		else
		if( cID == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID, 0) )
		{
			IBezFloatKey* fKey = (IBezFloatKey*)key;
			rads = fKey->val;
			fKey = (IBezFloatKey*)lastKey;
			rads -= fKey->val;
		}

		nSamp = int(fabs(rads / maxRads) + 0.9f);
		if( nSamp < 2 )
		{
			kTimes.Append(1, &t);
			continue;
		}


		TimeValue t0 = control->GetKeyTime(i-1);
		int j;
		for( j = 0; j < nSamp; j++ )
		{
			float p = float(j+1) / float(nSamp);
			TimeValue ti = t0 + TimeValue(p*  (t - t0));
			kTimes.Append(1, &ti);
		}
		///////////////////////////////////////
	}

	delete [] key;
	delete [] lastKey;
	
	hsGuardEnd;
}

#if 0
		// following code will work, except that TCB (but not Euler) rotations are stored
		// relative to previous key, so we'd need to end off with something
		// like for i = 1; i < n; i++ )
		//		key[i] = key[i-1]*  key[i]
		// or pass in the previous key and do it here.
		Quat quat;
		///////////////////////////////////////
		if( cID == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0) )  
		{
			ITCBRotKey* tcbRotKey = (ITCBRotKey*)mKey;
			quat = QFromAngAxis(tcbRotKey->val.angle, tcbRotKey->val.axis);
		}
		else if( cID == Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0) ) 
		{
			IBezQuatKey* bezRotKey = (IBezQuatKey*)mKey;
			quat = bezRotKey->val;
		}
		else if( cID == Class_ID(LININTERP_ROTATION_CLASS_ID, 0) ) 
		{
			ILinRotKey* linRotKey = (ILinRotKey*)mKey;
			quat = linRotKey->val;
		}
		else if( cID == Class_ID(EULER_CONTROL_CLASS_ID, 0) )
		{
			float eul[3];

			int i;
			for( i = 0; i < 3; i++ )
			{
				Control* subCntl = (Control*)control->SubAnim(i);
				if( fErrorMsg->Set(!(subCntl && (subCntl->ClassID() == Class_ID(TCBINTERP_FLOAT_CLASS_ID, 0))), node->GetName(), "Bad sub-controller type for animation").CheckAndAsk() )
				{
					eul[i] = 0.f;
					continue;
				}

				ITCBFloatKey* fKey = (ITCBFloatKey*)mKey;
				eul[i] = fKey->val;
			}
			
			EulerToQuat(eul, quat);
		}
		hbKey->fValue.Set(-quat.x, -quat.y, -quat.z, quat.w);

		///////////////////////////////////////	
#endif // try getting from key

//
// Create an hsKeyFrame from a 3DSMax key
//
Int32 hsControlConverter::ICreateHSInterpKey(Control* control, IKey* mKey, TimeValue keyTime, hsKeyFrame* baseKey, plMaxNode* node, hsBool rotQuat)
{
	hsGuardBegin("hsControlConverter::ICreateHSInterpKey");

	Class_ID cID = control->ClassID();
	SClass_ID sID = control->SuperClassID();
	char* nodeName = node ? node->GetName() : nil;

	// BEZ
	if (cID == Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0) ||
		cID == Class_ID(HYBRIDINTERP_COLOR_CLASS_ID,0) ||
		cID == Class_ID(HYBRIDINTERP_POINT3_CLASS_ID,0) )
	{
		IBezPoint3Key*bKey = (IBezPoint3Key*)mKey;				
		hsBezPoint3Key* hbKey = (hsBezPoint3Key*)baseKey;
		hbKey->fValue.Set(bKey->val.x, bKey->val.y, bKey->val.z);	// color should be 0 to 1
		hbKey->fInTan.Set(bKey->intan.x, bKey->intan.y, bKey->intan.z);
		hbKey->fOutTan.Set(bKey->outtan.x, bKey->outtan.y, bKey->outtan.z);
	}
	else if (cID == Class_ID(HYBRIDINTERP_SCALE_CLASS_ID,0))
	{
		IBezScaleKey*bKey = (IBezScaleKey*)mKey;
		hsBezScaleKey* hbKey = (hsBezScaleKey*)baseKey;
		hsMatrix44 tXform;
		IGetUnEasedLocalTM(node, control, &tXform, keyTime);
		gemAffineParts ap;
		decomp_affine(tXform.fMap, &ap); 

		hbKey->fValue.fS.Set(ap.k.x, ap.k.y, ap.k.z);
		hbKey->fValue.fQ.Set(ap.u.x, ap.u.y, ap.u.z, ap.u.w);
		hbKey->fInTan.Set(bKey->intan.x, bKey->intan.y, bKey->intan.z);
		hbKey->fOutTan.Set(bKey->outtan.x, bKey->outtan.y, bKey->outtan.z);
	}

	else if (cID == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0) && !rotQuat)
	{
		IBezFloatKey* bKey = (IBezFloatKey*)mKey;					
		hsBezScalarKey* hbKey = (hsBezScalarKey*)baseKey;
		hbKey->fValue = bKey->val;
		hbKey->fInTan = bKey->intan;
		hbKey->fOutTan= bKey->outtan;
	}

	else
	// LIN
	if (cID == Class_ID(LININTERP_POSITION_CLASS_ID,0))
	{
		ILinPoint3Key*bKey = (ILinPoint3Key*)mKey;				
		hsPoint3Key* hbKey = (hsPoint3Key*)baseKey;
		hbKey->fValue.Set(bKey->val.x, bKey->val.y, bKey->val.z);	
	}
	else if (sID == SClass_ID(CTRL_ROTATION_CLASS_ID) || (cID == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0) && rotQuat))	// all rotations
	{
		hsQuatKey* hbKey = (hsQuatKey*)baseKey;

		// get rot values from Matrix and use quat slerp.
		// could try getting rot values from key 
		hsMatrix44 tXform;
		IGetUnEasedLocalTM(node, control, &tXform, keyTime);
		gemAffineParts ap;
		decomp_affine(tXform.fMap, &ap); 

		hbKey->fValue.Set(ap.q.x, ap.q.y, ap.q.z, ap.q.w);

		IEnableEaseCurves(control, true);	// re-enable
	}
	else if (cID == Class_ID(LININTERP_SCALE_CLASS_ID,0) )
	{
		ILinScaleKey*bKey = (ILinScaleKey*)mKey;
		hsScaleKey* hbKey = (hsScaleKey*)baseKey;
		hsMatrix44 tXform;
		IGetUnEasedLocalTM(node, control, &tXform, keyTime);
		gemAffineParts ap;
		decomp_affine(tXform.fMap, &ap); 

		hbKey->fValue.fS.Set(ap.k.x, ap.k.y, ap.k.z);
		hbKey->fValue.fQ.Set(ap.u.x, ap.u.y, ap.u.z, ap.u.w);	
	}
	else
	if (cID == Class_ID(LININTERP_FLOAT_CLASS_ID,0) )
	{
		ILinFloatKey* bKey = (ILinFloatKey*)mKey;					
		hsScalarKey* hbKey = (hsScalarKey*)baseKey;
		hbKey->fValue = bKey->val;
	}
	else
	// TCB
	if (cID == Class_ID(TCBINTERP_POSITION_CLASS_ID,0) ||
		cID == Class_ID(TCBINTERP_POINT3_CLASS_ID, 0)		)
	{
		ITCBPoint3Key*bKey = (ITCBPoint3Key*)mKey;				
		hsPoint3Key* hbKey = (hsPoint3Key*)baseKey;
		hbKey->fValue.Set(bKey->val.x, bKey->val.y, bKey->val.z);
	}
	else
	if (cID == Class_ID(TCBINTERP_FLOAT_CLASS_ID,0) )
	{
		ITCBFloatKey* bKey = (ITCBFloatKey*)mKey;					
		hsScalarKey* hbKey = (hsScalarKey*)baseKey;
		hbKey->fValue = bKey->val;
	}
	else if (cID == Class_ID(TCBINTERP_SCALE_CLASS_ID,0) )
	{
		ITCBScaleKey*bKey = (ITCBScaleKey*)mKey;
		hsScaleKey* hbKey = (hsScaleKey*)baseKey;
		hsMatrix44 tXform;
		IGetUnEasedLocalTM(node, control, &tXform, keyTime);
		gemAffineParts ap;
		decomp_affine(tXform.fMap, &ap); 

		hbKey->fValue.fS.Set(ap.k.x, ap.k.y, ap.k.z);
		hbKey->fValue.fQ.Set(ap.u.x, ap.u.y, ap.u.z, ap.u.w);	
	}
	else
	{
		fErrorMsg->Set(true, nodeName, "unknown controller type?").Check();
		return 0;	// failed
	}

	int frameNum = keyTime / GetTicksPerFrame();
	hsAssert(frameNum <= hsKeyFrame::kMaxFrameNumber, "Anim is too long.");
	baseKey->fFrame = frameNum;
	
	return 1;		// did it
	hsGuardEnd;
}

UInt8 hsControlConverter::GetKeyType(Control* control, hsBool rotQuat)
{
	Class_ID cID = control->ClassID();
	SClass_ID sID = control->SuperClassID();

	if (cID == Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0) ||
		cID == Class_ID(HYBRIDINTERP_COLOR_CLASS_ID,0) ||
		cID == Class_ID(HYBRIDINTERP_POINT3_CLASS_ID,0) )
	{
		return hsKeyFrame::kBezPoint3KeyFrame;
	}
	else if (cID == Class_ID(HYBRIDINTERP_SCALE_CLASS_ID,0))
	{
		return hsKeyFrame::kBezScaleKeyFrame;
	}
	else if (cID == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0) && !rotQuat)
	{
		return hsKeyFrame::kBezScalarKeyFrame;
	}
	else if (cID == Class_ID(LININTERP_POSITION_CLASS_ID,0))
	{
		return hsKeyFrame::kPoint3KeyFrame;
	}
	else if (sID == SClass_ID(CTRL_ROTATION_CLASS_ID) || (cID == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0) && rotQuat))	// all rotations
	{
		return hsKeyFrame::kQuatKeyFrame;
	}
	else if (cID == Class_ID(LININTERP_SCALE_CLASS_ID,0) )
	{
		return hsKeyFrame::kScaleKeyFrame;
	}
	else if (cID == Class_ID(LININTERP_FLOAT_CLASS_ID,0) )
	{
		return hsKeyFrame::kScalarKeyFrame;
	}
	else if (cID == Class_ID(TCBINTERP_POSITION_CLASS_ID,0) ||
			 cID == Class_ID(TCBINTERP_POINT3_CLASS_ID, 0))
	{
		return hsKeyFrame::kPoint3KeyFrame;
	}
	else if (cID == Class_ID(TCBINTERP_FLOAT_CLASS_ID,0) )
	{
		return hsKeyFrame::kScalarKeyFrame;
	}
	else if (cID == Class_ID(TCBINTERP_SCALE_CLASS_ID,0) )
	{
		return hsKeyFrame::kScaleKeyFrame;
	}
	else
	{
		return hsKeyFrame::kUnknownKeyFrame;
	}
}

//
//
//
Int32 hsControlConverter::IGetRangeCoverKeyIndices(char* nodeName, Control* cont, Int32 &start, Int32 &end)
{
	hsGuardBegin("hsControlConverter::IGetRangeCoverKeyIndices");

	if (!cont)
	{
		return 0;
	}

	IKeyControl* keys = GetKeyControlInterface(cont);
	int numKeys=keys->GetNumKeys();
	if (numKeys == 0)
		return 0;

	IKey* key=(IKey*)(new byte [keys->GetKeySize()]);

	start = numKeys;
	for (int i=0; i<numKeys; i++)
	{
		keys->GetKey(i, key);
		if (IIsKeyInRange(key))
		{
			if (start > i)
				start = i;
			end = i;
		}
	}

	// If the keys aren't on the exact endpoints of our range, we need to include the next or previous
	// one so that we have data for any time within our range.

	if (start == numKeys) // No keys inside the range
	{
		for (int i = 0; i < numKeys; i++)
		{
			keys->GetKey(i, key);
			if (key->time < fSegStart)
				start = i;
		}

		if ((start == numKeys) ||		// no keys before the start time
			(start == numKeys - 1))		// no keys after end (since the latest key is before start)
		{
			delete [] key;
			return 0;
		}

		end = start + 1;
	}
	else
	{
		keys->GetKey(start, key);
		if (key->time > fSegStart && start > 0)
			start -= 1;

		keys->GetKey(end, key);
		if (key->time < fSegEnd && end < numKeys - 1)
			end += 1;
	}

	delete [] key;

	//fErrorMsg->Set(numInRange>1 && numInRange!=numKeys, nodeName ? nodeName : "?", 
//			"Warning: Object has controller with keyframes outside of animation interval").CheckAndAsk();


	return end - start + 1;
	hsGuardEnd;
}

//
// find the closest ancestor (if any) that is animated.
// this node's space will be our local space.
//
plMaxNode* hsControlConverter::GetXformParent(plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::GetXformParent");

	while( node && (node = (plMaxNode *)node->GetParentNode()) &&
		!(ForceOrigin(node) || ForceLocal(node) || IsAnimated(node)) );

	return node;
	hsGuardEnd;
}

// ###########################################################################
// Note that ForceWorldSpace Overrides ForceOrigin which Overrides ForceLocal
// ###########################################################################
hsBool hsControlConverter::ForceWorldSpace(plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::ForceWorldSpace");

	return false;
	
	hsGuardEnd;
}

// ###########################################################################
// Note that ForceWorldSpace Overrides ForceOrigin which Overrides ForceLocal
// ###########################################################################
hsBool hsControlConverter::ForceOrigin(plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::ForceOrigin");

	char* nn = node->GetName();

	if (node->IsRootNode())
	{
		return false;
	}

	if (ForceWorldSpace(node))
	{
		return false;
	}

	return false;
	hsGuardEnd;
}

// ###########################################################################
// Note that ForceWorldSpace Overrides ForceOrigin which Overrides ForceLocal
// This is significant because things that require ForceLocal because they are
// animated or what-not, are still okay with ForceOrigin, but not v.v.
// ###########################################################################
hsBool hsControlConverter::ForceLocal(plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::ForceLocal");


	const char* nn = node->GetName();

	if( !node->CanConvert() )
		return false;

	if (node->IsRootNode())
	{
		return false;
	}

	if( node->GetForceLocal() )
		return true;

	if( ISkinNode((plMaxNode*)node->GetParentNode()) )
	{
		node->SetForceLocal(true);
		return true;
	}

	Object* objectRef = node->GetObjectRef();
	if (fConverterUtils.IsInstanced(objectRef) && 
		gUserPropMgr.UserPropExists(node,"AllowInstancing"))
	{
		node->SetForceLocal(true);
		return true;
	}


	return false;
	
	hsGuardEnd;
}


hsBool hsControlConverter::IsAnimated(plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::IsAnimated");

	return node->IsAnimated();
	
	hsGuardEnd;
}

hsBool hsControlConverter::OwnsMaterialCopy(plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::OwnsMaterialCopy");

	return false;
	hsGuardEnd;
}

hsBool hsControlConverter::HasFrameEvents(plMaxNode *node)
{
	hsGuardBegin("hsSceneConverter::HasFrameEvents");
	
	if (!node)
	{
		return false;
	}
	
	TSTR sdata;
	if (gUserPropMgr.GetUserPropString(node,"FESound",sdata) ||
		gUserPropMgr.GetUserPropString(node,"FESoundEmitter",sdata) ||
		gUserPropMgr.GetUserPropString(node,"FEGrab",sdata) ||
		gUserPropMgr.GetUserPropString(node,"FEDrop",sdata) ||
		gUserPropMgr.GetUserPropString(node,"FEEventOn",sdata) ||
		gUserPropMgr.GetUserPropString(node,"FEEventOnPermanent",sdata) ||
		gUserPropMgr.GetUserPropString(node,"FEEventOff",sdata) ||
		gUserPropMgr.GetUserPropString(node,"FEActor",sdata)) 
	{
		return false;
	}
	
	return false;
	hsGuardEnd; 
}

hsBool hsControlConverter::GetControllerByName(Animatable* anim, TSTR &name, Control* &ctl)
{
	hsGuardBegin("hsControlConverter::GetControllerByName");

	if( anim )
	{
		int nSub = anim->NumSubs();
		int i;
		for( i = 0; i < nSub; i++ )
		{
			if (anim->SubAnim(i)==nil)
				continue;
			TSTR subName = anim->SubAnimName(i);
			if( subName == name )
			{
				fErrorMsg->Set(!anim->SubAnim(i), name, "Found controller by name, but nobody home").Check();
				ctl = GetControlInterface(anim->SubAnim(i));
				return true;
			}
			else if( GetControllerByName(anim->SubAnim(i), name, ctl) )
			{
				return true;
			}
		}
	}
	ctl = nil;

	return false;
	hsGuardEnd;
}

Control *hsControlConverter::GetControllerByID(IParamBlock2 *pblock, int paramID)
{
	hsGuardBegin("hsControlConverter::GetControllerByID");

	if (pblock)
	{
		int animIdx = pblock->GetAnimNum(paramID);
		if (animIdx != -1)
		{
			Animatable* anim = pblock->SubAnim(animIdx);
			if (anim)
				return GetControlInterface(anim);
		}
	}

	return NULL;
	hsGuardEnd;
}

void hsControlConverter::CompositeKeyTimes(Control* ctl, Tab<TimeValue> &time)
{
	hsGuardBegin("hsControlConverter::CompositeKeyTimes");

	if( !ctl )
	{
		return;
	}

	int curTime = 0;
	int i;
	for( i = 0; i < ctl->NumKeys(); i++ )
	{
		TimeValue t = ctl->GetKeyTime(i);
		// advance times
		while( (curTime < time.Count())&&(t > time[curTime]) )
			curTime++;
		// if past end, append it
		if( curTime >= time.Count() )
			time.Append(1, &t);
		else // if less 
		if( t < time[curTime] )
			time.Insert(curTime++, 1, &t);
		// already there, skip
	}
	
	hsGuardEnd;
}

//
//
//
ControllerType hsControlConverter::IGetControlType(TSTR ctrlName)
{
	hsGuardBegin("hsControlConverter::IGetControlType");

	ControllerType ct = ctrlTypeUnknown;
	if (ctrlName && !strcmp(ctrlName, "Ease Curve"))
	{
		ct = ctrlTypeEase;
	}
	else if (ctrlName && !strcmp(ctrlName, "Mult Curve"))
	{
		ct = ctrlTypeMult;
	} 
	else if (ctrlName && !strcmp(ctrlName, "Position"))
	{
		ct = ctrlTypePosition;
	}
	else if (ctrlName && !strcmp(ctrlName, "Rotation"))
	{
		ct = ctrlTypeRotation;
	}
	else if (ctrlName && !strcmp(ctrlName, "Scale"))
	{
		ct = ctrlTypeScale;
	} 
	else if (ctrlName && !strcmp(ctrlName, "Transform"))
	{
		ct = ctrlTypeTransform;
	}
	else if (ctrlName && !strcmp(ctrlName, "Roll Angle"))
	{
		ct = ctrlTypeRollAngle;
	}
#if 0
	// biped controllers are good for nothing
	else if (ctrlName && !strcmp(ctrlName, "Vertical"))
	{
		ct = ctrlTypeVert;
	}
	else if (ctrlName && !strcmp(ctrlName, "Horizontal"))
	{
		ct = ctrlTypeHoriz;
	}
	else if (ctrlName && !strcmp(ctrlName, "Turning"))
	{
		ct = ctrlTypeTurn;
	}
#endif

	return ct;
	hsGuardEnd;
}

bool hsControlConverter::IIsKeyTimeInRange(TimeValue time)
{
	hsGuardBegin("hsControlConverter::IIsKeyTimeInRange");

	Interval interval = fInterface->GetAnimRange();
	TimeValue startTime = interval.Start();	// in ticks
	TimeValue endTime = interval.End();		// in ticks


	return (time >= startTime && time <= endTime) && // Max's range
		   (time >= fSegStart && time <= fSegEnd);

	hsGuardEnd;
}

bool hsControlConverter::IIsKeyInRange(IKey* key)
{
	hsGuardBegin("hsControlConverter::IIsKeyInRange");

	return IIsKeyTimeInRange(key->time);
	hsGuardEnd;
}

void hsControlConverter::IGetUnEasedLocalTM(plMaxNode* node, Control* control, hsMatrix44* out, TimeValue time)
{
	hsGuardBegin("hsControlConverter::IGetUnEasedLocalTM");

	// disable easing so that GetTM won't give us an eased answer.
	// we want the uneased "key" value, so that we can do the easing ourselves
	IEnableEaseCurves(control, false);	

	// Make scale key match nodeTM
	fErrorMsg->Set(!node, "ICreateHSInterpKey", "nil node").Check();
	*out = node->GetLocalToParent44(time);

	IEnableEaseCurves(control, true);	// re-enable

	hsGuardEnd;
}

//
//
//
void hsControlConverter::IEnableEaseCurves(Animatable* control, bool enable)
{	
	hsGuardBegin("hsControlConverter::IEnableEaseCurves");

	if (control)
	{
		int n = control->NumSubs();
		for (int i=0; i<n; i++)
			IEnableEaseCurves(control->SubAnim(i), enable);

		EaseCurveList* el = GetEaseListInterface(control);
		if (el)
		{
			for(int i=0; i<el->NumEaseCurves(); i++)
			{
				if (enable)
					el->EnableEaseCurve(i);
				else
					el->DisableEaseCurve(i);
			}
		}
	}

	hsGuardEnd;
}

// We don't actually use this ID for a plugin, just to keep track of our AppData chunks
#define CONTROL_CONVERTER_CID Class_ID(0xae807d2, 0x523808c7)

Matrix3 hsControlConverter::IFlipY()
{
	hsGuardBegin("hsControlConverter::IFlipY");

	Matrix3 xfm = ScaleMatrix(Point3(1.0, -1.0, 1.0)) * TransMatrix(Point3(0.0, 1.0, 0.0));

	return xfm;
	hsGuardEnd;
}

bool hsControlConverter::ISkinNode(plMaxNode* node)
{
	hsGuardBegin("hsControlConverter::ISkinNode");

    /*
	if( fForceSkinning )
		return true;
	if( fForceNoSkinning )
		return false;
    */
	if (gUserPropMgr.UserPropExists(node,"MATSkin")) 
	{
		return true;
	}

	if (gUserPropMgr.UserPropExists(node,"MATSkinColor")) 
	{
		return true;
	}

	if( node && node->GetName() && strstr(node->GetName(), "%skin") )
	{
		return true;
	}

	return false;
	hsGuardEnd;
}

void hsControlConverter::Matrix3ToHsMatrix44(Matrix3* m3, hsMatrix44* hsM)
{
	hsGuardBegin("hsControlConverter::Matrix3ToHsMatrix44");

    MRow* m = m3->GetAddr();
    
    hsM->Reset();
    hsM->fMap[0][0] = m[0][0];
    hsM->fMap[0][1] = m[1][0];
    hsM->fMap[0][2] = m[2][0];
    hsM->fMap[0][3] = m[3][0];
    
    hsM->fMap[1][0] = m[0][1];
    hsM->fMap[1][1] = m[1][1];
    hsM->fMap[1][2] = m[2][1];
    hsM->fMap[1][3] = m[3][1];
    
    hsM->fMap[2][0] = m[0][2];
    hsM->fMap[2][1] = m[1][2];
    hsM->fMap[2][2] = m[2][2];
    hsM->fMap[2][3] = m[3][2];
    
    hsM->NotIdentity();

	hsGuardEnd;
}

//// IGetEditableMeshKeyTimes ////////////////////////////////////////////////
//	Moved here after hsMeshConverter was obliterated. Only used in this class
//	anyway...

hsBool	hsControlConverter::IGetEditableMeshKeyTimes( plMaxNode *node, Tab<TimeValue> &times )
{
	hsGuardBegin( "hsControlConverter::GetEditableMeshKeyTimes" );

	Animatable *anim;
	if( IGetSubAnimByName(node, TSTR("Object (Editable Mesh)"), anim) )
	{
		fErrorMsg->Set(!anim, node->GetName(), "First she says yes, then she says no.").Check();
		
		int i;
		int nSub = anim->NumSubs();
		for( i = 0; i < nSub; i++ )
		{
			if( anim->SubAnim(i) )
			{
				Control *ctl = GetControlInterface(anim->SubAnim(i));
				hsControlConverter::Instance().CompositeKeyTimes(ctl, times);
			}
		}
	}
	return times.Count() > 0;
	hsGuardEnd; 
}

//// IGetGeomKeyTimes ////////////////////////////////////////////////////////
//	Moved here after hsMeshConverter was obliterated. Only used in this class
//	anyway...

hsBool	hsControlConverter::IGetGeomKeyTimes( plMaxNode *node, Tab<TimeValue> &times )
{
	hsGuardBegin( "hsControlConverter::GetGeomKeyTimes" );

	char *dgbNodeName = node->GetName();
	Object *obj = node->GetObjectRef();
	if( !obj )
		return false;
	IDerivedObject *derObj = nil;
	if( obj->CanConvertToType(derivObjClassID) )
	{
		derObj = (IDerivedObject *)obj->ConvertToType(fConverterUtils.GetTime(fInterface), derivObjClassID);
	}
	else
	{
		SClass_ID objID = obj->SuperClassID();
		SClass_ID genID(GEN_DERIVOB_CLASS_ID);
		if( !(obj->SuperClassID() == SClass_ID(GEN_DERIVOB_CLASS_ID)) )
			return false;
		if( objID != genID )
			return false;
		derObj = (IDerivedObject *)obj;
	}

	int i;
	int nKeys = 0;
	for( i = 0; i < derObj->NumModifiers(); i++ )
	{
		Modifier *mod = derObj->GetModifier(i);
		char *dbgModName = mod->GetName();
		if( mod )
		{
			ChannelMask mask = mod->ChannelsChanged();
			if( mask & GEOM_CHANNEL )
			{
				IGetGeomKeyTimesRecur(mod, times);
			}
		}
	}

	return (times.Count() > 0);
	hsGuardEnd; 
}

//// IGetGeomKeyTimesRecur ///////////////////////////////////////////////////
//	Moved here after hsMeshConverter was obliterated. Only used in this class
//	anyway...

void	hsControlConverter::IGetGeomKeyTimesRecur( Animatable *anim, Tab<TimeValue> &times )
{
	hsGuardBegin( "hsControlConverter::IGetGeomKeyTimesRecur" );

	Control * ctl = GetControlInterface(anim);
	hsControlConverter::Instance().CompositeKeyTimes(ctl, times);

	int iSub;
	int nSub = anim->NumSubs();
	for( iSub = 0; iSub < nSub; iSub++ )
	{
		if( anim->SubAnim(iSub) )
			IGetGeomKeyTimesRecur(anim->SubAnim(iSub), times);
	}

	hsGuardEnd;
}

//// IGetGeomKeyTimesRecur ///////////////////////////////////////////////////
//	Moved here after hsMeshConverter was obliterated. Only used in this class
//	anyway...

hsBool	hsControlConverter::IGetSubAnimByName( Animatable *anim, TSTR &name, Animatable *&subAnim )
{
	hsGuardBegin( "hsControlConverter::IGetSubAnimByName" );

	if( anim )
	{
		int nSub = anim->NumSubs();
		int i;
		for( i = 0; i < nSub; i++ )
		{
			if (anim->SubAnim(i)==nil)
				continue;
			TSTR subName = anim->SubAnimName(i);
			if( subName == name )
			{
				fErrorMsg->Set(!anim->SubAnim(i), name, "Found controller by name, but nobody home").Check();
				subAnim = anim->SubAnim(i);
				return true;
			}
			else if( IGetSubAnimByName(anim->SubAnim(i), name, subAnim) )
			{
				return true;
			}
		}
	}
	subAnim = nil;
	return false;
	hsGuardEnd; 
}

// bad craziness, isolated here.
#include "plConvert.h"
#include "plgDispatch.h"
#include "../MaxComponent/plAnimComponent.h"
#include "../MaxComponent/plCameraComponents.h"
#include "../../../Sources/Plasma/NucleusLib/pnMessage/plCameraMsg.h"
#include "../../../Sources/Plasma/PubUtilLib/plMessage/plAnimCmdMsg.h"
#include "../../../Sources/Plasma/FeatureLib/pfCamera/plCameraModifier.h"
#include "../../../Sources/Plasma/NucleusLib/pnSceneObject/plSceneObject.h"

void hsControlConverter::IExportAnimatedCameraFOV(plMaxNode* node, hsTArray <hsG3DSMaxKeyFrame>* kfArray)
{
	// grab the FOV settings at each keyframe here
	// create callback messages for the animation to send to the camera
	// to interpolate to the correct FOV at each keyframe
	
	plAnimComponentBase* pAnim = nil;
	int count = node->NumAttachedComponents();
	int i;
	for (i = 0; i < count; i++)
	{
		plComponentBase *comp = node->GetAttachedComponent(i);
		if (comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID)
		{
			pAnim = (plAnimComponentBase*)comp;
			break;
		}
	}

	plCamera1Component* pCamComp = nil;
	for (i = 0; i < count; i++)
	{
		plComponentBase *comp = node->GetAttachedComponent(i);
		if (comp->ClassID() == FIXEDCAM_CID)
		{
			pCamComp = (plCamera1Component*)comp;
			break;
		}
	}

	const plCameraModifier1* pCamMod = nil;
	count = node->GetSceneObject()->GetNumModifiers();
	for (i=0; i < count; i++)
	{
		pCamMod = plCameraModifier1::ConvertNoRef(node->GetSceneObject()->GetModifier(i));
		if (pCamMod)
			break;
			
	}
	plCameraMsg* pCamMsg = TRACKED_NEW plCameraMsg;
	pCamMsg->SetCmd(plCameraMsg::kSetAnimated);
	pCamMsg->AddReceiver(pCamMod->GetKey());
	plConvert::Instance().AddMessageToQueue(pCamMsg);
	Object* obj = node->EvalWorldState(hsConverterUtils::Instance().GetTime(node->GetInterface())).obj;
	GenCamera* theCam;
	hsTArray<hsScalar> fovW;
	hsTArray<hsScalar> fovH;
	for (i=0; i < kfArray->Count(); i++)
	{
		TimeValue t = TimeValue(GetTicksPerFrame() * (kfArray[0][i].fFrame));
		theCam = (GenCamera *) obj->ConvertToType(t, Class_ID(LOOKAT_CAM_CLASS_ID, 0));
		float FOVvalue= 0.0;			//Currently in Radians
		// radians
		FOVvalue = theCam->GetFOV(t);
		// convert
		FOVvalue = FOVvalue*(180/3.141592);
		int FOVType = theCam->GetFOVType();
		hsScalar wDeg, hDeg;
		switch(FOVType)
		{
		case 0: // FOV_W
			{
				wDeg = FOVvalue;
				hDeg = (wDeg*3)/4;
			}
			break;
		case 1: // FOV_H
			{
				hDeg = FOVvalue;
				wDeg = (hDeg*4)/3;
			}
			break;
		}
		fovW.Append(wDeg);
		fovH.Append(hDeg);
			
	}
	for (i=0; i < kfArray->Count(); i++)
	{
		
		plCameraMsg* pFOVMsg = TRACKED_NEW plCameraMsg;
		plCameraConfig* pCfg = pFOVMsg->GetConfig();

		if (i == kfArray->Count() - 1)
		{
			pCfg->fFOVh = fovH[0];
			pCfg->fFOVw = fovW[0];
			pCfg->fAccel = kfArray[0][0].fFrame / MAX_FRAMES_PER_SEC;
		}
		else
		{
			pCfg->fFOVh = fovH[i + 1];
			pCfg->fFOVw = fovW[i + 1];
			pCfg->fAccel = kfArray[0][i + 1].fFrame / MAX_FRAMES_PER_SEC;
		}
		
		
		pFOVMsg->SetCmd(plCameraMsg::kAddFOVKeyframe);
		pFOVMsg->AddReceiver(pCamMod->GetKey());
		
		plEventCallbackMsg* pCall = TRACKED_NEW plEventCallbackMsg;
		pCall->fEvent = kTime;
		pCall->fEventTime = kfArray[0][i].fFrame / MAX_FRAMES_PER_SEC;
		pCall->fIndex = i;
		pCall->AddReceiver(pCamMod->GetKey());
		plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
		pMsg->AddReceiver(pCamMod->GetKey());
		pMsg->SetSender(pAnim->GetModKey(node));
		pMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
		pMsg->SetAnimName(ENTIRE_ANIMATION_NAME);
		pMsg->fTime = kfArray[0][i].fFrame / MAX_FRAMES_PER_SEC;
		pMsg->AddCallback(pCall);
		hsRefCnt_SafeUnRef(pCall);
		plConvert::Instance().AddMessageToQueue(pFOVMsg);
		plConvert::Instance().AddMessageToQueue(pMsg);
	}
}