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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtPath.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/****************************************************************************
*
*   Exported functions
*
***/

//===========================================================================
void PathGetProgramDirectory (
    wchar       *dst,
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    PathGetProgramName(dst, dstChars);
    PathRemoveFilename(dst, dst, dstChars);
}

//===========================================================================
void PathAddFilename (
    wchar       *dst, 
    const wchar  src[], 
    const wchar  fname[],
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(dstChars);

    wchar temp[MAX_PATH];
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
    wchar       *dst, 
    const wchar  src[], 
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(dstChars);

    wchar drive[MAX_DRIVE];
    wchar dir[MAX_DIR];
    PathSplitPath(src, drive, dir, 0, 0);
    PathMakePath(dst, dstChars, drive, dir, 0, 0);
}

//===========================================================================
void PathRemoveExtension (
    wchar       *dst, 
    const wchar  src[], 
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(dstChars);

    wchar drive[MAX_DRIVE];
    wchar dir[MAX_DIR];
    wchar fname[MAX_FNAME];
    PathSplitPath(src, drive, dir, fname, 0);
    PathMakePath(dst, dstChars, drive, dir, fname, 0);
}

//===========================================================================
void PathSetExtension (
    wchar       *dst, 
    const wchar  src[],
    const wchar  ext[],
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(dst != ext);
    ASSERT(dstChars);

    wchar drive[MAX_DRIVE];
    wchar dir[MAX_DIR];
    wchar fname[MAX_FNAME];
    PathSplitPath(src, drive, dir, fname, 0);
    PathMakePath(dst, dstChars, drive, dir, fname, ext);
}

//===========================================================================
void PathAddExtension (
    wchar       *dst, 
    const wchar  src[],
    const wchar  ext[],
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(dst != ext);
    ASSERT(dstChars);

    wchar drive[MAX_DRIVE];
    wchar dir[MAX_DIR];
    wchar fname[MAX_FNAME];
    wchar oldext[MAX_EXT];
    PathSplitPath(src, drive, dir, fname, oldext);
    PathMakePath(
        dst, 
        dstChars,
        drive, 
        dir, 
        fname, 
        oldext[0] ? oldext : ext
    );
}

//===========================================================================
void PathRemoveDirectory (
    wchar       *dst, 
    const wchar  src[], 
    unsigned     dstChars
) {
    ASSERT(dst);
    ASSERT(src);
    ASSERT(dstChars);

    wchar fname[MAX_FNAME];
    wchar ext[MAX_EXT];
    PathSplitPath(src, 0, 0, fname, ext);
    PathMakePath(dst, dstChars, 0, 0, fname, ext);
}


/*****************************************************************************
*
*   Email formatting functions
*
***/

//============================================================================
void PathSplitEmail (
	const wchar	emailAddr[],
	wchar *		user,
	unsigned	userChars,
	wchar *		domain,
	unsigned	domainChars,
	wchar *		tld,
	unsigned	tldChars,
	wchar *		subDomains,
	unsigned	subDomainChars,
	unsigned	subDomainCount
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
	wchar * tmp = ALLOCA(wchar, len + 1);
	StrCopy(tmp, emailAddr, len + 1);
	const wchar * work = tmp;

	// parse user	
	wchar token[MAX_PATH];
	if (!StrTokenize(&work, token, arrsize(token), L"@"))
		return;

	// copy user to output parameter
	if (userChars)
		StrCopy(user, token, userChars);

	// skip past the '@' symbol
	if (!*work++)
		return;
	
	// parse all domains
	ARRAY(wchar *) arr;
	while (StrTokenize(&work, token, arrsize(token), L".")) {
		unsigned toklen = StrLen(token);
		wchar * str = ALLOCA(wchar, toklen + 1);
		StrCopy(str, token, toklen + 1);
		arr.Add(str);
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
	
	#undef SUB_DOMAIN
}
