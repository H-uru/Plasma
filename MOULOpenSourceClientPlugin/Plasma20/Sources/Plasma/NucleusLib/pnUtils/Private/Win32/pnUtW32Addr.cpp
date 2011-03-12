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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/Win32/pnUtW32Addr.cpp
*   
***/

#include "../../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

// hardcoded byte ordering -- Intel only
#ifdef _M_IX86

    const unsigned kHostClassALoopbackAddr  = 0x7f000001; // 127.0.0.1
    const unsigned kHostClassALoopbackMask  = 0x00ffffff;
    const unsigned kNetClassALoopbackAddr   = 0x0100007f; // 127.0.0.1
    const unsigned kNetClassALoopbackMask   = 0xffffff00;

    const unsigned kHostClassANatAddr       = 0x000000a0; // 10.0.0.0 - 10.255.255.255
    const unsigned kHostClassANatMask       = 0x000000ff;   
    const unsigned kNetClassANatAddr        = 0x0a000000; // 10.0.0.0 - 10.255.255.255
    const unsigned kNetClassANatMask        = 0xff000000;   

    const unsigned kHostClassBNetAddr       = 0x000010ac; // 172.16.0.0 - 172.31.255.255
    const unsigned kHostClassBNetMask       = 0x0000f0ff;
    const unsigned kNetClassBNetAddr        = 0xac100000; // 172.16.0.0 - 172.31.255.255
    const unsigned kNetClassBNetMask        = 0xfff00000;

    const unsigned kHostClassCNatAddr       = 0x0000a8c0; // 192.168.0.0 - 192.168.255.255
    const unsigned kHostClassCNatMask       = 0x0000ffff;
    const unsigned kNetClassCNatAddr        = 0xc0a80000; // 192.168.0.0 - 192.168.255.255
    const unsigned kNetClassCNatMask        = 0xffff0000;

#else

#error "Must implement for this architecture"

#endif  // ifdef _M_IX86


/*****************************************************************************
*
*   Internal functions
*
***/

//===========================================================================
// Address sort order:
//  (highest)
//      externally visible address
//      10.0.0.0     - 10.255.255.255    
//      172.16.0.0   - 172.31.255.255  
//      192.168.0.0  - 192.168.255.255
//      127.0.0.0    - 127.0.0.255
//  (lowest)
static int NetAddressNodeSortValueNetOrder (NetAddressNode addr) {
	ref(NetAddressNodeSortValueNetOrder);
    // Loopback addresses
    if ((addr & kNetClassALoopbackMask) == (kNetClassALoopbackAddr & kNetClassALoopbackMask))
        return 4;

    // Private addresses
    if ((addr & kNetClassCNatMask) == (kNetClassCNatAddr & kNetClassCNatMask))
        return 3;
    if ((addr & kNetClassBNetMask) == (kNetClassBNetAddr & kNetClassBNetMask))
        return 2;
    if ((addr & kNetClassANatMask) == (kNetClassANatAddr & kNetClassANatMask))
        return 1;

    // Public addresses
    return 0;
}

//===========================================================================
static int NetAddressNodeSortValueHostOrder (NetAddressNode addr) {
    // Loopback addresses
    if ((addr & kHostClassALoopbackMask) == (kHostClassALoopbackAddr & kHostClassALoopbackMask))
        return 4;

    // Private addresses
    if ((addr & kHostClassCNatMask) == (kHostClassCNatAddr & kHostClassCNatMask))
        return 3;
    if ((addr & kHostClassBNetMask) == (kHostClassBNetAddr & kHostClassBNetMask))
        return 2;
    if ((addr & kHostClassANatMask) == (kHostClassANatAddr & kHostClassANatMask))
        return 1;

    // Public addresses
    return 0;
}

//===========================================================================
static NetAddressNode NodeFromString (const wchar * string[]) {
    // skip leading whitespace
    const wchar * str = *string;
    while (iswspace(*str))
        ++str;

    // This function handles partial ip addresses (61.33)
    // as well as full dotted quads. The address can be
    // terminated by whitespace or ':' as well as '\0'
    byte data[4];
    * (dword *) data = 0;
    for (unsigned i = sizeof(data); i--; ) {
        if (!iswdigit(*str))
            return (unsigned)-1;

        unsigned value = StrToUnsigned(str, &str, 10);
        if (value >= 256)
            return (unsigned)-1;
        data[i] = (byte) value;

        if (!*str || (*str == ':') || iswspace(*str))
            break;

        static const wchar s_separator[] = L"\0...";
        if (*str++ != s_separator[i])
            return (unsigned)-1;
    }

    *string = str;
    return * (NetAddressNode *) &data[0];
}


/*****************************************************************************
*
*   Exports
*
***/

//===========================================================================
int NetAddressCompare (const NetAddress & a1, const NetAddress & a2) {
    const sockaddr_in & i1 = * (const sockaddr_in *) &a1;
    const sockaddr_in & i2 = * (const sockaddr_in *) &a2;

    int   d = i1.sin_addr.S_un.S_addr - i2.sin_addr.S_un.S_addr;
    return d ? d : i1.sin_port - i2.sin_port;
}

//===========================================================================
bool NetAddressSameSystem (const NetAddress & a1, const NetAddress & a2) {
    const sockaddr_in & i1 = * (const sockaddr_in *) &a1;
    const sockaddr_in & i2 = * (const sockaddr_in *) &a2;
    return i1.sin_addr.S_un.S_addr == i2.sin_addr.S_un.S_addr;
}

//===========================================================================
unsigned NetAddressHash (const NetAddress & addr) {
    // by using only the node number as the hash value, users can safely use
    // hash value to find addresses by either using either "SameSystem" or "Equal"
    const sockaddr_in & iAddr = * (const sockaddr_in *) &addr;
    return iAddr.sin_addr.S_un.S_addr;
}   

//===========================================================================
void NetAddressToString (
    const NetAddress &  addr, 
    wchar *             str, 
    unsigned            chars, 
    ENetAddressFormat   format
) {
    ASSERT(str);

    static const wchar * s_fmts[] = {
        L"%S",      // kNetAddressFormatNodeNumber
        L"%S:%u",   // kNetAddressFormatAll
    };
    ASSERT(format < arrsize(s_fmts));
    const sockaddr_in & inetaddr = * (const sockaddr_in *) &addr;
    StrPrintf(
        str,
        chars,
        s_fmts[format],
        inet_ntoa(inetaddr.sin_addr),
        ntohs(inetaddr.sin_port)
    );
}

//===========================================================================
bool NetAddressFromString (NetAddress * addr, const wchar str[], unsigned defaultPort) {
    ASSERT(addr);
    ASSERT(str);

    // NetAddress is bigger than sockaddr_in so start by zeroing the whole thing
    ZEROPTR(addr);
    
    for (;;) {
        NetAddressNode node = NodeFromString(&str);
        if (node == (unsigned)-1)
			break;

        if (*str == L':')
            defaultPort = StrToUnsigned(str + 1, nil, 10);
   
        sockaddr_in * inetaddr          = (sockaddr_in *) addr;
        inetaddr->sin_family            = AF_INET;
        inetaddr->sin_port              = htons((word) defaultPort);
        inetaddr->sin_addr.S_un.S_addr  = htonl(node);
        // inetaddr->sin_zero already zeroed

        return true;
    }

    // address already zeroed
    return false;
}

//===========================================================================
unsigned NetAddressGetPort (
    const NetAddress & addr
) {
    return ntohs(((sockaddr_in *) &addr)->sin_port);
}

//===========================================================================
void NetAddressSetPort (
    unsigned        port,
    NetAddress *    addr
) {
    ((sockaddr_in *) addr)->sin_port = htons((word) port);
}

//============================================================================
NetAddressNode NetAddressGetNode (const NetAddress & addr) {
    return ntohl(((const sockaddr_in *) &addr)->sin_addr.S_un.S_addr);
}

//===========================================================================
void NetAddressFromNode (
    NetAddressNode  node,
    unsigned        port,
    NetAddress *    addr
) {
    ZEROPTR(addr);
    sockaddr_in * inetaddr          = (sockaddr_in *) addr;
    inetaddr->sin_family            = AF_INET;
    inetaddr->sin_addr.S_un.S_addr  = htonl(node);
    inetaddr->sin_port              = htons((word) port);
}

//===========================================================================
void NetAddressNodeToString (
    NetAddressNode  node,
    wchar *         str,
    unsigned        chars
) {
    in_addr addr;
    addr.S_un.S_addr = htonl(node);
    StrPrintf(str, chars, L"%S", inet_ntoa(addr));
}

//===========================================================================
NetAddressNode NetAddressNodeFromString (
    const wchar     string[],
    const wchar *   endPtr[]
) {
    if (!endPtr)
        endPtr = &string;
    *endPtr = string;
    return NodeFromString(endPtr);
}

//===========================================================================
void NetAddressGetLoopback (
    unsigned            port,
    NetAddress *        addr
) {
    NetAddressFromNode(
        kHostClassALoopbackAddr,
        port,
        addr
    );
}

//===========================================================================
unsigned NetAddressGetLocal (
    unsigned        count,
    NetAddressNode  addresses[]
) {
    ASSERT(count);
    ASSERT(addresses);

    for (;;) {
        // Get local computer name
        char name[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = arrsize(name);
        if (!GetComputerName(name, &size))
            StrCopy(name, "localhost", arrsize(name));

        // Get IPv4 addresses for local system
        const struct hostent * host = gethostbyname(name);
        if (!host || !host->h_name)
            break;
        host = gethostbyname(host->h_name);
        if (!host)
            break;
        if (host->h_length != sizeof(dword))
            break;

        // Count total number of addresses
        unsigned found = 0;
        const dword ** addr = (const dword **) host->h_addr_list;
        for (; *addr; ++addr)
            ++found;
        if (!found)
            break;

        // Create a buffer to sort the addresses
        NetAddressNode * dst;
        if (found > count)
            dst = ALLOCA(NetAddressNode, found);
        else
            dst = addresses;

        // Fill address buffer
        const dword * src = * (const dword **) host->h_addr_list;
        for (unsigned index = 0; index < found; ++index)
            dst[index] = ntohl(src[index]);

        // Sort addresses by priority
        QSORT(
            NetAddressNode,
            dst,
            found,
            NetAddressNodeSortValueHostOrder(elem1) - NetAddressNodeSortValueHostOrder(elem2)
        );

        // Return the number of addresses the user actually requested
        if (found > count) {
            for (unsigned index = 0; index < count; ++index)
                addresses[index] = dst[index];
            return count;
        }

        return found;
    }

    // Initialize with a valid value
    addresses[0] = kHostClassALoopbackAddr;
    return 1;
}
