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
#include "plPipeline.h"
#include "hsGColorizer.h"

hsBool hsGColorizer::Colorizing() 
{ 
	return fPipeline ? 0 != (fPipeline->GetColorOverride().fFlags & hsColorOverride::kModColor) : false; 
}

hsBool hsGColorizer::Alpharizing() 
{ 
	return fPipeline ? 0 != (fPipeline->GetColorOverride().fFlags & hsColorOverride::kModAlpha) : false; 
}

hsColorRGBA hsGColorizer::GetCurrentColor() 
{ 
	return fPipeline ? fPipeline->GetColorOverride().fColor : hsColorRGBA().Set(1.f,1.f,1.f,1.f); 
}

void hsGColorizer::Init(plPipeline* pipe)
{
	fPipeline = pipe;
}

void hsGColorizer::PushColorize(hsColorRGBA& col, hsBool alphaOnly)
{
	if( fPipeline )
	{
		hsColorOverride colorOver;
		colorOver.fFlags = alphaOnly ? hsColorOverride::kModAlpha : hsColorOverride::kModColor | hsColorOverride::kModAlpha;
		colorOver.fColor = col;
		fResetColor = fPipeline->PushColorOverride(colorOver);
	}
}

void hsGColorizer::PopColorize()
{
	if( fPipeline )
	{
		fPipeline->PopColorOverride(fResetColor);
	}
}

