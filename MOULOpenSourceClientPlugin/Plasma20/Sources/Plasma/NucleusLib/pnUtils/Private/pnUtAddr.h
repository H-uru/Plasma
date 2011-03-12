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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtAddr.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTADDR_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtAddr.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTADDR_H


/*****************************************************************************
*
*   Types and constants
*
***/


struct NetAddress {
    byte data[24];
};

typedef unsigned NetAddressNode;


extern NetAddress   kNilNetAddress;

typedef bool (*FNetAddressEqualityProc)(
    const NetAddress & a1,
    const NetAddress & a2
);


class CNetAddressHash {
    NetAddress              m_addr;
    FNetAddressEqualityProc m_equals;
public:
    CNetAddressHash (
        const NetAddress &      addr
    );
    CNetAddressHash (
        const NetAddress &      addr,
        FNetAddressEqualityProc equals
            // Useful values for 'equals':
            //      NetAddressEqual         --> address node and port numbers match
            //      NetAddressSameSystem    --> address node numbers match
    );
    void operator= (const CNetAddressHash & rhs) const; // not impl.
    bool operator== (const CNetAddressHash & rhs) const;
    unsigned GetHash () const;
    const NetAddress & GetAddr () const;
};


/*****************************************************************************
*
*   Functions
*
***/

enum ENetAddressFormat {
    kNetAddressFormatNodeNumber,
    kNetAddressFormatAll,
    kNumNetAddressFormats
};

unsigned NetAddressHash (const NetAddress & addr);

int NetAddressCompare (const NetAddress & a1, const NetAddress & a2);
bool NetAddressSameSystem (const NetAddress & a1, const NetAddress & a2);
inline bool NetAddressEqual (const NetAddress & a1, const NetAddress & a2) {
    return NetAddressCompare(a1, a2) == 0;
}

void NetAddressToString (
    const NetAddress &  addr, 
    wchar *             str, 
    unsigned            chars, 
    ENetAddressFormat   format
);

// 'str' must be in the form of a dotted IP address (IPv4 or IPv6)
// - names which require DNS lookup will cause the function to return false
bool NetAddressFromString (
    NetAddress *    addr,
    const wchar     str[],
    unsigned        defaultPort
);

unsigned NetAddressGetPort (
    const NetAddress &  addr
);
void NetAddressSetPort (
    unsigned            port,
    NetAddress *        addr
);

void NetAddressNodeToString (
    NetAddressNode      node,
    wchar *             str,
    unsigned            chars
);
NetAddressNode NetAddressNodeFromString (
    const wchar         string[],
    const wchar *       endPtr[]
);

NetAddressNode NetAddressGetNode (
    const NetAddress &  addr
);
void NetAddressFromNode (
    NetAddressNode      node,
    unsigned            port,
    NetAddress     *    addr
);

void NetAddressGetLoopback (
    unsigned            port,
    NetAddress *        addr
);

// Returns number of addresses set, which is guaranteed to be non-zero.
// Furthermore, it sorts the addresses so that loopback and NAT addresses
// are at the end of the array, and "real" addresses are at the beginning.
unsigned NetAddressGetLocal (
    unsigned        count,
    NetAddressNode  addresses[]
);
