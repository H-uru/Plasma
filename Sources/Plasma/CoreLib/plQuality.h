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

#ifndef plQuality_inc
#define plQuality_inc

class plQuality
{
public:
	enum
	{
		kMinimum	= 0,

		kPS_1_1		= 2,
		kPS_1_4		= 3,
		kPS_2_Plus	= 4
	};
protected:
	// These two are instanciated in plLoadMask.cpp, as well as
	// function implementations.
	static		int			fQuality; 
	static		int			fCapability;

	friend class plClient;
	friend class plQualitySlider;
	friend class plDXPipeline;

	// Set by the app according to user preference.
	static		void SetQuality(int q);

	// Set by the pipeline according to platform capabilities.
	static		void SetCapability(int c);

public:
	// Set by the app according to user preference.
	static		int GetQuality() { return fQuality; }

	// Set by the pipeline according to platform capabilities.
	static		int GetCapability() { return fCapability; }

};


#endif // plQuality_inc
