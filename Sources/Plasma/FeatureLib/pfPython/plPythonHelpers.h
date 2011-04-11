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
#ifndef _plPythonHelpers_h_
#define _plPythonHelpers_h_

//////////////////////////////////////////////////////////////////////
//
// plPythonHelpers   - helper definitions for plPythonFileMod
//
//////////////////////////////////////////////////////////////////////

// notetrack data structure
typedef struct
{
	plKey		objKey;		// plKey to the object
	const char*	note_name;	// notetrack name
	plKey		modKey;	// the animation modifier plKey that is associated with the notetrack name
} PY_NOTETRACK;


// material animation record structure
typedef struct
{
	const char*	material_name;		// material name
	const char*	note_name;			// notetrack name
	plKey		modKey;		// the animation modifier plKey that is associated with this material animation
} PY_MATERIAL_ANIM;

// material animation record structure
typedef struct
{
	const char*	sound_name;		// the sound name
	int			sound_index;	// the sound index that goes with the sound
} PY_SOUND_IDX;


#endif // _plPythonHelpers_h
