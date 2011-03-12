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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Intern.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_INTERN_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Intern.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_INTERN_H



/*****************************************************************************
*
*   Crypt
*
***/

namespace Crypt {

//============================================================================
class KeyBase {
public:
    virtual void Codec (bool encrypt, ARRAY(byte) * dest, unsigned sourceBytes, const void * sourceData) = 0;
    virtual unsigned GetBlockSize () const = 0;
};

//============================================================================
class KeyRc4 : public KeyBase {
private:
	unsigned m_x;
	unsigned m_y;
	byte     m_state[256];

	void Initialize (unsigned bytes, const void * data);

public:
	KeyRc4 (unsigned bytes, const void * data) { Initialize(bytes, data); }

	void Codec (bool encrypt, ARRAY(byte) * dest, unsigned sourceBytes, const void * sourceData);
	unsigned GetBlockSize () const { return 1; }

	static void KeyGen (
		unsigned		randomBytes, 
		const void *	randomData,
		ARRAY(byte) *	privateData
	);
};


} // namespace Crypt
