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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnAsyncCore/pnAcDns.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PNACDNS_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNASYNCCORE_PNACDNS_H
#include "pnNetCommon/plNetAddress.h"

/****************************************************************************
*
*   Dns functions
*
***/

/** Asynchrone Dynamic Name Service.
 * 
 *  Provide functions to resolve an domain name or to get it from an IP without blocking the caller thread.
 *  \note On error or cancelation, no callback is send!
 */
class AsyncDns {
public:
    /** DNS Callback function type.
     *  \param param User defined param.
     *  \param name Peer domain name.
     *  \param addrCount Number of IP address in \p addrs.
     *  \param addrs Array of @e addrCount IP address.
     */
    typedef void (* FProc) (
        void *              param,
        const char          name[],
        unsigned            addrCount,
        const plNetAddress  addrs[]
    );
    
    /** DNS Cancel handler. */
    class Cancel {
        void * ptr;
        inline Cancel (void * p) : ptr(p) {}
        
    public:
        /** Create a new handler without linked Async DNS operation. */
        inline Cancel () : ptr(nullptr) {}
        
        /** Cancel the linked async DNS operation.
         *  \return \b true if cancelation realy append.
         */
        bool LookupCancel ();
        /** Unlink to the DNS operation */
        inline void Clear () { ptr = nullptr; }
        
        /** test if an operation is linked */
        inline operator bool ()   { return  ptr; }
        /** test if no operation is linked */
        inline bool operator ! () { return !ptr; }
        
        friend class AsyncDns;
    };
    
    /** Asynchronly search IPs associate with a domain name.
     *  \param name Peer domain name.
     *  \param port port to use.
     *  \param lookupProc Callback function called on operation success with the result.
     *  \param param user defined param send to the callback.
     *  \return Cancelation handler for this operation.
     */
    static Cancel LookupName (
        const char      name[],
        unsigned        port,
        FProc           lookupProc,
        void *          param = nullptr
    );
    
    /** Asynchronly search the domain name associate with an IP.
     *  \param address IP address to search.
     *  \param lookupProc Callback function called on operation success with the result.
     *  \param param user defined param send to the callback.
     *  \return Cancelation handler for this operation.
     */
    static Cancel LookupAddr (
        const plNetAddress& address,
        FProc               lookupProc,
        void *              param = nullptr
    );
    
    /** Cancel all pending operations that use a specific callback function.
     *  \param lookupProc function used by all operations to cancel.
     *  \warning Due to asynchrone, some operations can call this callback after LookupCancel return;
     */
    static void LookupCancel (FProc lookupProc);
    
    struct P; // private
};

#endif

