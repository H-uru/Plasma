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
#ifndef SERVER

#ifndef plNetAddress_h_inc
#define plNetAddress_h_inc

#include "HeadSpin.h"
#include "hsWindows.h" // FIXME
#include "plString.h"

#include "hsStream.h"

#if defined(HS_BUILD_FOR_WIN32)

#elif defined( HS_BUILD_FOR_UNIX )
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#error  "Must Include net stuff for this OS here"
#endif


#ifdef SetPort
#undef SetPort
#endif

typedef sockaddr_in  AddressType;

/**
 * A class representing a network address endpoint, as a pair of host address
 * and port number.
 *
 * Internally, this class stores the network address using the sockaddr_in
 * structure, but provides methods to transparently get and set the host
 * address and port.
 */
class plNetAddress
{
    // fAddr must be first field
    AddressType     fAddr;

public:
    /**
     * Initializes an empty network address.
     * All the fields of the sockaddr will be zeroed.
     */
    plNetAddress();

    /**
     * Initializes a new network address from the given IPv4 address and port
     * number.
     *
     * @param addr The IPv4 address as a 32-bit network byte order integer.
     * @param port The port number as a 16-bit host order integer.
     */
    plNetAddress(uint32_t addr, uint16_t port);

    /**
     * Initializes a new network address from the given hostname and port
     * number.
     *
     * @param addr The DNS hostname of the host.
     * @param port The port number as a 16-bit host order integer.
     */
    plNetAddress(const char* addr, uint16_t port);

    virtual ~plNetAddress(){}

    bool operator==(const plNetAddress& other) const {
        return (GetHost() == other.GetHost()) && (GetPort() == other.GetPort());
    }

    bool operator!=(const plNetAddress& other) const {
        return !(*this == other);
    }

    /**
     * Clears the address and zeros out the sockaddr fields.
     */
    void Clear();

    /**
     * Sets the address to INADDR_ANY for binding to any host.
     */
    bool SetAnyAddr();

    /**
     * Sets the port number to 0 to allow binding to any port.
     */
    bool SetAnyPort();

    /**
     * Gets the port number of the host.
     *
     * @return The host port number.
     */
    uint16_t GetPort() const;

    /**
     * Sets the port number of the host.
     *
     * @param port The port number in host byte order.
     */
    bool SetPort(uint16_t port);

    /**
     * Gets the IPv4 address of the host as a 32-bit integer in network byte
     * order (big endian).
     *
     * @return The IPv4 host address.
     */
    uint32_t GetHost() const;

    /**
     * Sets the IPv4 address of the host from a DNS name.
     *
     * @param hostname The DNS name of the host.
     */
    bool SetHost(const char* hostname);

    /**
     * Sets the IPv4 address of the host from an unsigned 32-bit integer in
     * network byte order (big endian).
     *
     * @param ip4addr The host IPv4 address in network byte order.
     */
    bool SetHost(uint32_t ip4addr);

    /**
     * Retrieves the internal address type.
     *
     * @return A constant sockaddr_in.
     */
    const AddressType& GetAddressInfo() const { return fAddr; }

    /**
     * Retrieves the internal address type.
     *
     * @return A sockaddr_in.
     */
    AddressType& GetAddressInfo() { return fAddr; }

    /**
     * Returns the IPv4 address of the host as a string in 4-octet dotted
     * notation.
     *
     * @return A string of the IPv4 host address.
     */
    plString GetHostString() const;

    /**
     * Return the IPv4 address and port number of the host as a string in
     * 4-octet dotted notation with a colon separated port.
     *
     * @return A string of the IPv4 host address and port number.
     */
    plString GetHostWithPort() const;

    /**
     * Returns a string representation of the host address and port number.
     *
     * @return A string representation of the address.
     */
    plString AsString() const;

    /**
     * Reads and deserializes the address from a stream.
     *
     * @param stream The stream from which to read the address.
     */
    void Read(hsStream * stream);

    /**
     * Serializes and writes the address to a stream.
     *
     * @param stream The stream to which to write the address.
     */
    void Write(hsStream * stream);
};



#endif // plNetAddress_h_inc
#endif // SERVER
