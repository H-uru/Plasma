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
#include "plLOSDispatch.h"
#include "plSimulationMgr.h"
#include "plgDispatch.h"
#include "../plMessage/plLOSRequestMsg.h"
#include "../plMessage/plLOSHitMsg.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plModifier/plLogicModifier.h"
#include "plPXPhysical.h"
#include "plPXPhysicalControllerCore.h"
#include "plPXConvert.h"

#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureMod.h"

#include "NxPhysics.h"

#include "plProfile.h"
plProfile_Extern(LineOfSight);

class myRaycastReport : public NxUserRaycastReport
{
public:
	void InitCast(plSimDefs::plLOSDB db, plLOSRequestMsg::TestType type)
	{
		fDB = db;
		fType = type;
		fHitObj = nil;
		fNormal.Set(0, 0, 0);
		fPoint.Set(0, 0, 0);
		fDist = FLT_MAX;
	}

	bool GotHit() { return fDist != FLT_MAX; }
	plKey GetObj() { return fHitObj; }
	hsScalar GetDistance() { return fDist; }
	const hsVector3& GetNormal() { return fNormal; }
	const hsPoint3& GetPoint() { return fPoint; }
	void ResetHitObj(){fHitObj=nil;}
private:
	virtual bool onHit(const NxRaycastHit& hit)
	{
		NxActor& hitActor = hit.shape->getActor();
		plPXPhysical* phys = (plPXPhysical*)hitActor.userData;

		plKey objKey = nil;
		UInt16 objDB = plSimDefs::kLOSDBNone;

		if (phys)
		{
			objKey = phys->GetObjectKey();
			objDB = phys->GetAllLOSDBs();
		}
		else
		{
			bool isController;
			plPXPhysicalControllerCore* controller = plPXPhysicalControllerCore::GetController(hitActor,&isController);
			if (controller)
			{
				objKey = controller->GetOwner();
				objDB = controller->GetLOSDB();
			}
		}

		// is the object's physic enabled and is it in the database that we are looking for?
		if ( !phys || !phys->GetProperty(plSimulationInterface::kDisable) )
		{
			if ( (fDB & objDB) != 0)
			{
				if (fType == plLOSRequestMsg::kTestAny || hit.distance < fDist)
				{
					// need one more test... if it is a clickable need to see if it is enabled
					bool disabled = false;
					if ( objKey )
					{
						plSceneObject* so = plSceneObject::ConvertNoRef( objKey->GetObjectPtr() );
						if (so)
						{
							int i;
							for ( i=0; i < so->GetNumModifiers(); i++)
							{
								plLogicModifier* lo = (plLogicModifier*)plLogicModifier::ConvertNoRef(so->GetModifier(i) );
								if (lo)
								{
									disabled = lo->Disabled();
									break;
								}
							}
						}
					}

					if (!disabled)
					{
						fHitObj = objKey;
						fNormal.Set(hit.worldNormal.x, hit.worldNormal.y, hit.worldNormal.z);
						fPoint.Set(hit.worldImpact.x, hit.worldImpact.y, hit.worldImpact.z);
						fDist = hit.distance;

						if (fType == plLOSRequestMsg::kTestAny)
							return false;
					}
				}
			}
		}

		return true;
	}

	plSimDefs::plLOSDB fDB;
	plLOSRequestMsg::TestType fType;

	plKey fHitObj;
	hsVector3 fNormal;
	hsPoint3 fPoint;
	hsScalar fDist;
} gMyReport;

plLOSDispatch::plLOSDispatch()
{
	RegisterAs(kLOSObject_KEY);
	plgDispatch::Dispatch()->RegisterForExactType(plLOSRequestMsg::Index(), GetKey());
}

plLOSDispatch::~plLOSDispatch()
{
	plgDispatch::Dispatch()->UnRegisterForExactType(plLOSRequestMsg::Index(), GetKey());
}

hsBool plLOSDispatch::MsgReceive(plMessage* msg)
{

	plLOSRequestMsg* requestMsg = plLOSRequestMsg::ConvertNoRef(msg);
	if (requestMsg)
	{
		plProfile_BeginTiming(LineOfSight);

		plSimulationMgr* sim = plSimulationMgr::GetInstance();

		plKey worldKey = requestMsg->fWorldKey;
		if (!worldKey)
		{
			plArmatureMod* av = plAvatarMgr::GetInstance()->GetLocalAvatar();
			if ( av && av->GetController() )
				worldKey = av->GetController()->GetSubworld();
		}

		hsPoint3 from = requestMsg->fFrom;
		hsPoint3 at = requestMsg->fTo;

		// requests are always sent in world space, but they might
		// need to be converted to subworld space
		hsMatrix44 l2w, w2l;
		if (worldKey)
		{
			plSceneObject* so = plSceneObject::ConvertNoRef(worldKey->ObjectIsLoaded());
			if (so)
			{
				l2w = so->GetLocalToWorld();
				w2l = so->GetWorldToLocal();
				from = w2l * from;
				at = w2l * at;
			}
		}
		else
		{
			l2w.Reset();
			w2l.Reset();
		}

		NxScene* scene = sim->GetScene(worldKey);

		gMyReport.InitCast(requestMsg->GetRequestType(), requestMsg->GetTestType());

		hsVector3 norm = hsVector3(at - from);
		hsScalar dist = norm.Magnitude();
		norm.Normalize();

		NxRay worldRay;
		worldRay.dir = plPXConvert::Vector(norm);
		worldRay.orig = plPXConvert::Point(from);
		//PhysX will complain to log if ray distance is less than or equal to Zero, besides shouldn't  bother throwing 
		// a point, and if we have negative we have some serious problems
		if(dist>0.0f)
		{
			scene->raycastAllShapes(worldRay, gMyReport, NX_ALL_SHAPES, 0xffffffff, dist, NX_RAYCAST_DISTANCE | NX_RAYCAST_IMPACT | NX_RAYCAST_NORMAL);
		}
		else{
			SimLog("%s sent out a LOS request with a ray length of %d.", requestMsg->GetSender()->GetName(), dist);
		}
		if (gMyReport.GotHit())
		{
			// We got a hit, save off the info
			plMessage* hitMsg = ICreateHitMsg(requestMsg, l2w);

			if (requestMsg->GetCullDB() != plSimDefs::kLOSDBNone)
			{
				// If we have a cull db, adjust the length of the raycast to be from the
				// original point to the object we hit.  If we find anything from the cull
				// db in there, the cast fails.
				hsScalar dist = gMyReport.GetDistance();
				if(dist!=0.0)
				{
					gMyReport.InitCast(requestMsg->GetCullDB(), plLOSRequestMsg::kTestAny);
					scene->raycastAllShapes(worldRay, gMyReport, NX_ALL_SHAPES, 0xffffffff, dist, NX_RAYCAST_DISTANCE | NX_RAYCAST_IMPACT | NX_RAYCAST_NORMAL);

					if (gMyReport.GotHit())
					{
						delete hitMsg;
						hitMsg = nil;

						if (requestMsg->GetReportType() == plLOSRequestMsg::kReportMiss ||
							requestMsg->GetReportType() == plLOSRequestMsg::kReportHitOrMiss)
						{
							ICreateMissMsg(requestMsg)->Send();
						}
					}
				}
				else// we are right on top of the object I assume that means we hit it
				{// since PhysX would have complained we will log it anyways. Just so we have a better idea, where this
					//was happening previously
					SimLog("%s sent out a LOS request. The second cast for culling was of length 0. ABORTING and assuming hit.", requestMsg->GetSender()->GetName());
				}

			}

			if (hitMsg &&
				(requestMsg->GetReportType() == plLOSRequestMsg::kReportHit ||
				requestMsg->GetReportType() == plLOSRequestMsg::kReportHitOrMiss))
				hitMsg->Send();
		}
		else
		{
			if (requestMsg->GetReportType() == plLOSRequestMsg::kReportMiss ||
				requestMsg->GetReportType() == plLOSRequestMsg::kReportHitOrMiss)
			{
				ICreateMissMsg(requestMsg)->Send();
			}
		}

		plProfile_EndTiming(LineOfSight);
		gMyReport.ResetHitObj();
		return true;
	}

	return hsKeyedObject::MsgReceive(msg);
}

plMessage* plLOSDispatch::ICreateHitMsg(plLOSRequestMsg* requestMsg, hsMatrix44& l2w)
{
	plKey ourKey = GetKey();
	plKey rcvKey = requestMsg->GetSender();
	plLOSHitMsg* hitMsg = TRACKED_NEW plLOSHitMsg(ourKey, rcvKey, nil);
	hitMsg->fNoHit = false;
	hitMsg->fObj = gMyReport.GetObj();
	hitMsg->fDistance = gMyReport.GetDistance();
	hitMsg->fNormal = l2w * gMyReport.GetNormal();
	hitMsg->fHitPoint = l2w * gMyReport.GetPoint();
	hitMsg->fRequestID = requestMsg->GetRequestID();
	return hitMsg;
}

plMessage* plLOSDispatch::ICreateMissMsg(plLOSRequestMsg* requestMsg)
{
	plKey ourKey = GetKey();
	plKey rcvKey = requestMsg->GetSender();
	plLOSHitMsg* missMsg = TRACKED_NEW plLOSHitMsg(ourKey, rcvKey, nil);
	missMsg->fNoHit = true;
	missMsg->fObj = nil;
	missMsg->fRequestID = requestMsg->GetRequestID();
	return missMsg;
}
