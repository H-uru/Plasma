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
#include "plMtlImport.h"

extern ClassDesc2* GetPassMtlDesc();
extern ClassDesc2* GetLayerTexDesc();
extern ClassDesc2* GetStaticEnvLayerDesc();
extern ClassDesc2* GetMultiMtlDesc();
extern ClassDesc2* GetDecalMtlDesc();
extern ClassDesc2* GetCompMtlDesc();
extern ClassDesc2* GetParticleMtlDesc();
extern ClassDesc2* GetDynamicEnvLayerDesc();
extern ClassDesc2* GetBumpMtlDesc();
extern ClassDesc2* GetDynamicTextLayerDesc();
extern ClassDesc2* GetClothingMtlDesc();
extern ClassDesc2* GetAngleAttenLayerDesc();
extern ClassDesc2* GetStealthClassDesc();
extern ClassDesc2* GetBinkClassDesc();
extern ClassDesc2* GetMAXCameraLayerDesc();

int			plPlasmaMtlImport::GetNumMtlDescs( void )
{
	return 15;
}

ClassDesc2	*plPlasmaMtlImport::GetMtlDesc( int i )
{
	switch (i)
	{
		case 0: return GetPassMtlDesc();
		case 1: return GetLayerTexDesc();
		case 2: return GetMultiMtlDesc();
		case 3: return GetDecalMtlDesc();
		case 4: return GetCompMtlDesc();
		case 5: return GetStaticEnvLayerDesc();
		case 6: return GetParticleMtlDesc();
		case 7: return GetDynamicEnvLayerDesc();
		case 8: return GetBumpMtlDesc();
		case 9: return GetDynamicTextLayerDesc();
		case 10: return GetClothingMtlDesc();
		case 11: return GetAngleAttenLayerDesc();
		case 12: return GetStealthClassDesc();
		case 13: return GetBinkClassDesc();
		case 14: return GetMAXCameraLayerDesc();
		default: return 0;
	}
}
