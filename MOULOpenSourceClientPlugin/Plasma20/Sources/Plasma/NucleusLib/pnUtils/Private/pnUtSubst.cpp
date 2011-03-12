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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtSubst.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop

#define SUBST_BLOCK SubstParsedData<chartype>::SubstBlock<chartype>

/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
template <typename chartype>
bool IVarSubstitute (
    ARRAY(chartype) *   dst,
    const chartype      src[],
    unsigned            varCount,
    const chartype *    varNames[],   // [varCount]
    const chartype *    varValues[]   // [varCount]
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(varNames);
    ASSERT(varValues);

    dst->Reserve(StrLen(src) * 5/4);

    bool result = true;
    while (*src) {
        // Copy non-substituted strings and escape %% symbols
        if ((*src != L'%') || (*++src == L'%')) {
            dst->Push(*src++);
            continue;
        }

        // Find variable definition
        const chartype * varStart   = src;
        const chartype * varEnd     = StrChr(varStart, L'%');
        if (!varEnd) {
            // Skip % character and continue
            result = false;
            continue;
        }

        // Validate variable name length
        chartype varBuffer[256];
        if (varEnd - varStart >= arrsize(varBuffer)) {
            result = false;
            src = varEnd + 1;
            continue;
        }

        // Copy variable name excluding trailing '%'
        StrCopy(varBuffer, varStart, varEnd - varStart + 1);
        src = varEnd + 1;

        // Find the variable value and perform substitution
        bool found = false;
        for (unsigned i = 0; i < varCount; ++i) {
            if (StrCmp(varBuffer, varNames[i]))
                continue;
            dst->Add(varValues[i], StrLen(varValues[i]));
            found = true;
            break;
        }

        // Check that variable definition exists
        result = result && found;
    }

    // Terminate string
    dst->Push(0);
    return result;
}

//============================================================================
template <typename chartype>
bool IParseForSubst (
    SubstParsedData<chartype> *		dest,
	const chartype					src[]
) {
	const chartype * current = src;
    bool result = true;
    while (*current) {
        // Copy non-substituted strings and escape %% symbols
        if ((*current != L'%') || (*++current == L'%')) {
            current++;
            continue;
        }

        // Find variable definition
        const chartype * varStart   = current;
        const chartype * varEnd     = StrChr(varStart, L'%');
        if (!varEnd) {
            // Skip % character and continue
            result = false;
            continue;
        }

		// We've found a variable, copy the current data to a new object
		if (current != src) {
			int strLen = (current - src) - 1;
			SUBST_BLOCK * block = NEW(SUBST_BLOCK);
			block->isVar	= false;
			block->strLen	= strLen;
			block->data		= (chartype*)ALLOCZERO((strLen + 1) * sizeof(chartype));
			MemCopy(block->data, src, strLen * sizeof(chartype));

			dest->blocks.Add(block);
		}

        // Validate variable name length
        chartype varBuffer[256];
        if (varEnd - varStart >= arrsize(varBuffer)) {
            result = false;
            src = current = varEnd + 1;
            continue;
        }

        // Copy variable name excluding trailing '%'
		int strLen = (varEnd - varStart);
        SUBST_BLOCK * block = NEW(SUBST_BLOCK);
		block->isVar	= true;
		block->strLen	= strLen;
		block->data		= (chartype*)ALLOCZERO((strLen + 1) * sizeof(chartype));
		MemCopy(block->data, varStart, strLen * sizeof(chartype));

		dest->blocks.Add(block);

		src = current = varEnd + 1;
    }

	// Check and see if there's any data remaining
	if (current != src) {
		int strLen = (current - src);
		SUBST_BLOCK * block = NEW(SUBST_BLOCK);
		block->isVar	= false;
		block->strLen	= strLen;
		block->data		= (chartype*)ALLOCZERO((strLen + 1) * sizeof(chartype));
		MemCopy(block->data, src, strLen * sizeof(chartype));

		dest->blocks.Add(block);
	}

	return result;
}

//============================================================================
template <typename chartype>
bool IVarSubstPreParsed (
    ARRAY(chartype) *					dst,
    const SubstParsedData<chartype> *	src,
    unsigned							varCount,
    const chartype *					varNames[],   // [varCount]
    const chartype *					varValues[]   // [varCount]
) {
	unsigned approxTotalSize = 0;
	for (unsigned i = 0; i < src->blocks.Count(); ++i) {
		approxTotalSize += src->blocks[i]->strLen;
	}

	dst->Reserve(approxTotalSize * 5/4);

	bool foundAll = true;
	for (unsigned blockIndex = 0; blockIndex < src->blocks.Count(); ++blockIndex) {
		SUBST_BLOCK * block = src->blocks[blockIndex];
		if (block->isVar) {
			bool found = false;
			for (unsigned varIndex = 0; varIndex < varCount; ++varIndex) {
				if (StrCmp(block->data, varNames[varIndex])) {
					continue;
				}

				dst->Add(varValues[varIndex], StrLen(varValues[varIndex]));
				found = true;
				break;
			}

			foundAll &= found;
		}
		else {
			dst->Add(block->data, block->strLen);
		}
	}
	dst->Push(0);

	return foundAll;
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
bool ParseForSubst (
	SubstParsedData<wchar> *	dest,
	const wchar					src[]
) {
	return IParseForSubst(dest, src);
}

//============================================================================
bool ParseForSubst (
	SubstParsedData<char> *		dest,
	const char					src[]
) {
	return IParseForSubst(dest, src);
}

//============================================================================
bool VarSubstitute (
    ARRAY(wchar) *  dst,
    const wchar     src[],
    unsigned        varCount,
    const wchar *   varNames[],   // [varCount]
    const wchar *   varValues[]   // [varCount]
) {
    return IVarSubstitute(dst, src, varCount, varNames, varValues);
}

//============================================================================
bool VarSubstitute (
    ARRAY(char) *   dst,
    const char      src[],
    unsigned        varCount,
    const char *    varNames[],    // [varCount]
    const char *    varValues[]    // [varCount]
) {
    return IVarSubstitute(dst, src, varCount, varNames, varValues);
}

//============================================================================
bool VarSubstitute (
    ARRAY(wchar) *						dst,
    const SubstParsedData<wchar> *		src,
    unsigned							varCount,
    const wchar *						varNames[],   // [varCount]
    const wchar *						varValues[]   // [varCount]
) {
	return IVarSubstPreParsed(dst, src, varCount, varNames, varValues);
}

//============================================================================
bool VarSubstitute (
    ARRAY(char) *						dst,
    const SubstParsedData<char> *		src,
    unsigned							varCount,
    const char *						varNames[],   // [varCount]
    const char *						varValues[]   // [varCount]
) {
	return IVarSubstPreParsed(dst, src, varCount, varNames, varValues);
}
