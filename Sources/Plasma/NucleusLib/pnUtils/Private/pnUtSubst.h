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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
    template<typename char_type>
    struct SubstBlock {
        bool        isVar;
        char_type *  data;
        unsigned    strLen;

        SubstBlock()
        :   isVar(false)
        ,   data(nil)
        {
        }

        ~SubstBlock() {
            free(data);
        }
    };

    ARRAY(SubstBlock<chartype>*)        blocks;

    ~SubstParsedData() {
        for (unsigned i = 0; i < blocks.Count(); ++i) {
            SubstBlock<chartype> * block = blocks[i];
            delete block;
        }
    }
};

bool ParseForSubst (
    SubstParsedData<wchar_t> *    dest,
    const wchar_t                 src[]
);
bool ParseForSubst (
    SubstParsedData<char> *     dest,
    const char                  src[]
);

// Return value is for validation purposes only; it may be ignored
bool VarSubstitute (
    ARRAY(wchar_t) *  dst,
    const wchar_t     src[],
    unsigned        varCount,
    const wchar_t *   varNames[],   // [varCount]
    const wchar_t *   varValues[]   // [varCount]
);
bool VarSubstitute (
    ARRAY(char) *   dst,
    const char      src[],
    unsigned        varCount,
    const char *    varNames[],    // [varCount]
    const char *    varValues[]    // [varCount]
);
bool VarSubstitute (
    ARRAY(wchar_t) *                      dst,
    const SubstParsedData<wchar_t> *      src,
    unsigned                            varCount,
    const wchar_t *                       varNames[],   // [varCount]
    const wchar_t *                       varValues[]   // [varCount]
);
bool VarSubstitute (
    ARRAY(char) *                       dst,
    const SubstParsedData<char> *       src,
    unsigned                            varCount,
    const char *                        varNames[],   // [varCount]
    const char *                        varValues[]   // [varCount]
);
