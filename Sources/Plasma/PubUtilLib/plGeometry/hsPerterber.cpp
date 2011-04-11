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
#include "hsStream.h"
#include "hsPerterber.h"
#include "hsOscillator.h"
#include "hsGMesh.h"
#if 0 // GET_RID_OF_SHAPE_LAYER_DEFER
#include "hsGShape3.h"
#include "hsGShape3MegaMesh.h"
#endif// GET_RID_OF_SHAPE_LAYER_DEFER
#include "../plResMgr/plKey.h"
#include "../plSurface/hsGMaterial.h"
#include "hsTimer.h"
#include "../plPipeline/plPipeline.h"

hsBool32 hsPerterber::fDisabled = false;

hsPerterber::hsPerterber()
{
}

hsPerterber::~hsPerterber()
{
}

void hsPerterber::IUpdate(hsScalar secs, plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
}

void hsPerterber::TimeStampAndSave(hsStream* s)
{
	hsScalar secs = hsTimer::GetSeconds();

	hsKeyedObject::Save(s, nil);

	Save(s, secs);
}

void hsPerterber::TimeStampAndLoad(hsStream* s)
{
	hsScalar secs = hsTimer::GetSeconds();

	hsKeyedObject::Load(s, nil);

	Load(s, secs);
}

void hsPerterber::LabelAndWrite(hsStream* s)
{
	s->WriteSwap32(GetType());
	Write(s);
}

hsPerterber* hsPerterber::CreateAndRead(hsStream* s)
{
	hsPerterber* retVal = nil;

	UInt32 t = s->ReadSwap32();
	switch( t )
	{
	case kTypeOscillator:
		retVal = new hsOscillator;
		break;
	default:
		hsAssert(false, "Unknown perterber type");
		return nil;
	}
	retVal->Read(s);

	return retVal;
}

hsGMesh* hsPerterber::IGetMesh(hsGShape3* shape)
{
	hsGMesh* mesh = nil;
#if 0 // GET_RID_OF_SHAPE_LAYER_DEFER
	if( shape->GetShapeType() == hsGShape3::kTypeTriMesh )
	{
		hsGShape3TriMesh* shp = (hsGShape3TriMesh*)shape;
		mesh = shp->GetMesh();

#if 0 // move to export
		if( mesh->GetKey() && strstr(mesh->GetKey()->GetName(), "create") )
		{

			hsTArray<hsGMaterial*> matList;
			shp->AppendMaterials(matList);

			hsGTriMesh* newMesh = hsOscillator::MakeWaveMesh(40, hsPoint3(0,0,0), 4.f, 75.f, 1200.f, 1200.f, 0.75f, false);
			newMesh->SetMaterial(matList[0]);
			hsRefCnt_SafeUnRef(matList[0]);

			shp->SetMesh(newMesh);
			hsRefCnt_SafeUnRef(newMesh);
			mesh = newMesh;
		}
		else if( mesh->GetKey() && strstr(mesh->GetKey()->GetName(), "destroy") )
		{

			hsTArray<hsGMaterial*> matList;
			shp->AppendMaterials(matList);

			hsGTriMesh* newMesh = hsOscillator::MakeWaveMesh(50, hsPoint3(0,0,0), 1.5f, 30.f, 600.f, 600.f, 0.6f, true);
			newMesh->SetMaterial(matList[0]);
			hsRefCnt_SafeUnRef(matList[0]);

			shp->SetMesh(newMesh);
			hsRefCnt_SafeUnRef(newMesh);
			mesh = newMesh;
		}
		else
#endif // move to export
		{
			hsGTriMesh* triMesh = (hsGTriMesh*)shp->GetMesh();
			if( triMesh->GetTriangle(0)->fFlags & hsTriangle3::kHasFacePlane )
				triMesh->TrashPlanes();
			mesh = triMesh;
		}
	}
	else if( shape->GetShapeType() == hsGShape3::kTypeMegaMesh )
	{
		hsGShape3MegaMesh* mega = (hsGShape3MegaMesh*)shape;
		hsGMegaMesh* megaMesh = (hsGMegaMesh*)mega->GetMegaMesh();
		hsGTriMesh* triMesh = (hsGTriMesh*)megaMesh->GetMesh(0);
		if( triMesh->GetTriangle(0)->fFlags & hsTriangle3::kHasFacePlane )
		{
			int iMesh;
			for( iMesh = 0; iMesh < megaMesh->GetMeshCount(); iMesh++ )
			{
				triMesh = (hsGTriMesh*)megaMesh->GetMesh(iMesh);
				triMesh->TrashPlanes();
			}
		}
		mesh = mega->GetMegaMesh();
	}
#endif // GET_RID_OF_SHAPE_LAYER_DEFER
	return mesh;
}

void hsPerterber::Perterb(hsScalar secs, plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l, hsGShape3* shape)
{
	if( GetDisabled() )
		return;

	hsGMesh* mesh = IGetMesh(shape);

	IUpdate(secs, pipe, l2w, w2l);

	if( !mesh->HasOrigPoints() )
		mesh->StoreOrigPoints();
	int i;
	for( i = 0; i < mesh->GetNumPoints(); i++ )
	{
		IPerterb(*mesh->GetOrigPoint(i), *mesh->GetVertex(i));
	}
}

const int kPertCount = 6;
const int kMaxPertParams = 32;

struct hsPertDesc
{

	char		fName[256];
	UInt32		fType;
	UInt32		fNumParams;
	hsScalar	fParams[kMaxPertParams];
};

// NumWaves				= 1
// AttenScale			= 2
// WorldCenterBounds	= 6
// Period				= 2
// Amp					= 2
// Rate					= 2
// Life					= 2
// 
hsPertDesc		sPertTable[kPertCount] =
{
	{
		"mystocean",
			hsPerterber::kTypeOscillator,
			17,
		{
			5.f,
				1.f/100.f, 1.f/100.f,
				-100.f, 100.f, 0, 100.f, 150.f, 0,
				2.f, 5.f,
				0.5, 0.75f,
				0.68f, 0.68f,
				5.f, 10.f
		}
	},
	{
		"stoneocean",
			hsPerterber::kTypeOscillator,
			17,
		{
			5.f,
				1.f/100.f, 1.f/100.f,
				-100.f, 100.f, 0, 100.f, 150.f, 0,
				2.f, 5.f,
				0.5, 0.75f,
				0.68f, 0.68f,
				5.f, 10.f
		}
	},
	{
		"seleniticocean",
			hsPerterber::kTypeOscillator,
			17,
		{
			5.f,
				1.f/100.f, 1.f/100.f,
				-100.f, 100.f, 0, 100.f, 150.f, 0,
				2.f, 5.f,
				0.25, 0.45f,
				0.6f, 0.6f,
				5.f, 10.f
		}
	},
	{
		"channelocean",
			hsPerterber::kTypeOscillator,
			17,
		{
			5.f,
				1.f/30.f, 1.f/30.f,
				-100.f, -100.f, 0, 100.f, 100.f, 0,
				0.25f, 0.5f,
				0.1, 0.2f,
				0.4f, 0.8f,
				5.f, 10.f
		}
	},
	{
		"mechocean",
			hsPerterber::kTypeOscillator,
			17,
		{
			5.f,
				1.f/100.f, 1.f/100.f,
				-100.f, 100.f, 0, 100.f, 150.f, 0,
				2.f, 5.f,
				0.5, 0.4f,
				0.68f, 0.68f,
				5.f, 10.f
		}
	},
	{
		"rimeocean",
			hsPerterber::kTypeOscillator,
			17,
		{
			5.f,
				1.f/100.f, 1.f/100.f,
				-100.f, 100.f, 0, 100.f, 150.f, 0,
				2.f, 5.f,
				0.5, 0.75,
				0.68f, 0.68f,
				5.f, 10.f
		}
	}
};



#if 0	// Used Registry...need to change paulg
void hsPerterber::InitSystem(hsRegistry* reg)
{
	if( GetDisabled() )
		return;

	int i;
	for( i = 0; i < kPertCount; i++ )
	{
		switch( sPertTable[i].fType )
		{
		case kTypeOscillator:
			{
				hsOscillator* oscar = new hsOscillator;
				oscar->Init(sPertTable[i].fNumParams, sPertTable[i].fParams);

#ifdef PAULFIX
				oscar->Register(reg, sPertTable[i].fName, 0, true);
#endif
			}
			break;
		default:
			hsAssert(false, "Unknown perterber type");
			break;
		}
	}
}

void hsPerterber::Shutdown(hsRegistry* reg)
{
#ifdef PAULFIX
	int i;

	for( i = 0; i < reg->GetNumKeys(); i++ )
	{
		hsPerterber* pert = (hsPerterber*)(reg->GetKey(i)->GetObjectPtr());
		delete pert;
	}
#endif
}

#endif