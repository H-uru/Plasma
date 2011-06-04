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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSubst.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSUBST_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSubst.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTSUBST_H

template<typename chartype>
struct SubstParsedData {
	template<typename chartype>
	struct SubstBlock {
		bool		isVar;
		chartype *	data;
		unsigned	strLen;

		SubstBlock()
		:	isVar(false)
		,	data(nil)
		{
		}

		~SubstBlock() {
			FREE(data);
		}
	};

	ARRAY(SubstBlock<chartype>*)		blocks;

	~SubstParsedData() {
		for (unsigned i = 0; i < blocks.Count(); ++i) {
			SubstBlock<chartype> * block = blocks[i];
			DEL(block);
		}
	}
};

bool ParseForSubst (
	SubstParsedData<wchar> *	dest,
	const wchar					src[]
);
bool ParseForSubst (
	SubstParsedData<char> *		dest,
	const char					src[]
);

// Return value is for validation purposes only; it may be ignored
bool VarSubstitute (
    ARRAY(wchar) *  dst,
    const wchar     src[],
    unsigned        varCount,
    const wchar *   varNames[],   // [varCount]
    const wchar *   varValues[]   // [varCount]
);
bool VarSubstitute (
    ARRAY(char) *   dst,
    const char      src[],
    unsigned        varCount,
    const char *    varNames[],    // [varCount]
    const char *    varValues[]    // [varCount]
);
bool VarSubstitute (
    ARRAY(wchar) *						dst,
    const SubstParsedData<wchar> *		src,
    unsigned							varCount,
    const wchar *						varNames[],   // [varCount]
    const wchar *						varValues[]   // [varCount]
);
bool VarSubstitute (
    ARRAY(char) *						dst,
    const SubstParsedData<char> *		src,
    unsigned							varCount,
    const char *						varNames[],   // [varCount]
    const char *						varValues[]   // [varCount]
);
