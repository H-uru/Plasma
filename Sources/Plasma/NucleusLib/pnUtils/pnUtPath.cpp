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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtPath.cpp
*   
***/

#include "pnUtPath.h"
#include "pnUtStr.h"


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
void PathGetProgramDirectory (
    wchar_t       *dst,
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    PathGetProgramName(dst, dstChars);
    PathRemoveFilename(dst, dst, dstChars);
}

//===========================================================================
void PathAddFilename (
    wchar_t       *dst, 
    const wchar_t  src[], 
    const wchar_t  fname[],
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    wchar_t temp[MAX_PATH];
    if (dst == src) {
        StrCopy(temp, src, arrsize(temp));
        src = temp;
    }
    else if (dst == fname) {
        StrCopy(temp, fname, arrsize(temp));
        fname = temp;
    }

    PathMakePath(dst, dstChars, 0, src, fname, 0);
}

//===========================================================================
void PathRemoveFilename (
    wchar_t       *dst, 
    const wchar_t  src[], 
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(dstChars);

    wchar_t drive[MAX_DRIVE];
    wchar_t dir[MAX_DIR];
    PathSplitPath(src, drive, dir, 0, 0);
    PathMakePath(dst, dstChars, drive, dir, 0, 0);
}

/*****************************************************************************
*
*   Email formatting functions
*
***/

//============================================================================
void PathSplitEmail (
    const wchar_t emailAddr[],
    wchar_t *     user,
    unsigned    userChars,
    wchar_t *     domain,
    unsigned    domainChars,
    wchar_t *     tld,
    unsigned    tldChars,
    wchar_t *     subDomains,
    unsigned    subDomainChars,
    unsigned    subDomainCount
) {
    ASSERT(emailAddr);
    
    #define SUB_DOMAIN(i) subDomains[(i) * subDomainChars]

    // null-terminate all output parameters
    if (userChars) {
        ASSERT(user);
        user[0] = 0;
    }
    if (domainChars) {
        ASSERT(domain);
        domain[0] = 0;
    }
    if (tldChars) {
        ASSERT(tld);
        tld[0] = 0;
    }
    if (subDomainChars || subDomainCount) {
        ASSERT(subDomains);
        for (unsigned i = 0; i < subDomainCount; ++i)
            SUB_DOMAIN(i) = 0;
    }

    // bail now if email address is zero-length
    unsigned len = StrLen(emailAddr);
    if (!len)
        return;

    // copy email address so we can tokenize it
    wchar_t * tmp = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
    StrCopy(tmp, emailAddr, len + 1);
    const wchar_t * work = tmp;

    // parse user   
    wchar_t token[MAX_PATH];
    if (!StrTokenize(&work, token, arrsize(token), L"@"))
        return;

    // copy user to output parameter
    if (userChars)
        StrCopy(user, token, userChars);

    // skip past the '@' symbol
    if (!*work++)
        return;
    
    // parse all domains
    ARRAY(wchar_t *) arr;
    while (StrTokenize(&work, token, arrsize(token), L".")) {
        unsigned toklen = StrLen(token);
        wchar_t * str = (wchar_t*)malloc(sizeof(wchar_t) * (toklen + 1));
        StrCopy(str, token, toklen + 1);
        arr.Add(str);

        free(str);
    }

    // copy domains to output parameters
    unsigned index = 0;
    if (arr.Count() > 2) {
        // all domains except for the last two are sub-domains
        for (index = 0; index < arr.Count() - 2; ++index) {
            if (index < subDomainCount)
                if (subDomains)
                    StrCopy(&SUB_DOMAIN(index), arr[index], subDomainChars);
        }
    }
    if (arr.Count() > 1) {
        // second to last domain is the primary domain
        if (domain)
            StrCopy(domain, arr[index], domainChars);
        // last comes the top level domain
        ++index;
        if (tld)
            StrCopy(tld, arr[index], tldChars);
    }
    else if (arr.Count() == 1) {
        // if only one domain, return it as a sub-domain
        if (index < subDomainCount)
            if (subDomains)
                StrCopy(&SUB_DOMAIN(index), arr[index], subDomainChars);
    }

    free(tmp);
    
    #undef SUB_DOMAIN
}
