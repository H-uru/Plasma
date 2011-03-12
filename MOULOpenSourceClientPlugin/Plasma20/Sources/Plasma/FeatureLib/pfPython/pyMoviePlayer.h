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
#ifndef _pyMoviePlayer_h_
#define _pyMoviePlayer_h_

//////////////////////////////////////////////////////////////////////
//
// pyMoviePlayer   - a wrapper class all the movie player functions
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"
#include "pyColor.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyMoviePlayer
{
protected:
	char*	fMovieName;
	plKey	fSelfKey;

	pyMoviePlayer(): fMovieName(nil), fSelfKey(nil) {} // only used by python glue, do NOT call
	pyMoviePlayer(const char* movieName,pyKey& selfKey);
public:
	~pyMoviePlayer();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMoviePlayer);
	static PyObject *New(const char* movieName, pyKey& selfKey);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMoviePlayer object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMoviePlayer); // converts a PyObject to a pyMoviePlayer (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void AddPlasmaConstantsClasses(PyObject *m);

	void MakeMovie(const char* movieName, pyKey& selfKey); // only used by python glue, do NOT call

	// getters and setters
	virtual void SetCenter(hsScalar x, hsScalar y);
	virtual void SetScale(hsScalar width, hsScalar height);
	virtual void SetColor(pyColor color);
	virtual void SetVolume(hsScalar volume);
	virtual void SetOpacity(hsScalar opacity);

	// actions
	virtual void Play();		// kStart
	virtual void PlayPaused();	// kStart and kPause
	virtual void Pause();		// kPause
	virtual void Resume();		// kResume
	virtual void Stop();		// kStop

};

#endif // _pyMoviePlayer_h_
