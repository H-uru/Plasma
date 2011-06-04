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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetCli/pnNcEncrypt.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


namespace pnNetCli {

/****************************************************************************
*
*   Diffie-Hellman constants
*

    // g and n are pregenerated and published
    // (built into both client and server software)
    BigNum g(4);
    BigNum n; n.RandPrime(kKeyBits, &seed);

    // a and x are pregenerated; a is built into server software, and x is
    // built into client software
    BigNum a; a.Rand(kKeyBits, &seed);
    BigNum x; x.PowMod(g, a, n);

    // client chooses b and y on connect, and sends y to the server
    BigNum b; b.Rand(kKeyBits, &seed);
    BigNum y; y.PowMod(g, b, n);

    // server computes key: k = y^a mod n
    BigNum ka; ka.PowMod(y, a, n);

    // client computes key: k = x^b mod n
    BigNum kb; kb.PowMod(x, b, n);

***/

COMPILER_ASSERT(IS_POW2(kNetDiffieHellmanKeyBits));


/*****************************************************************************
*
*   Private
*
***/

//============================================================================
// TODO: Cache computed keys
static void GetCachedServerKey (
    NetMsgChannel * channel,
    BigNum *        ka,
    const BigNum &  dh_y
) {
    // Get diffie-hellman constants
    unsigned        DH_G;
    const BigNum *  DH_A;
    const BigNum *  DH_N;
    NetMsgChannelGetDhConstants(channel, &DH_G, &DH_A, &DH_N);

    // Compute the result
    ka->PowMod(dh_y, *DH_A, *DH_N);
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void NetMsgCryptClientStart (
    NetMsgChannel * channel,
    unsigned        seedBytes,
    const byte      seedData[],
    BigNum *        clientSeed,
    BigNum *        serverSeed
) {
    unsigned DH_G;
    const BigNum * DH_X;
    const BigNum * DH_N;
    NetMsgChannelGetDhConstants(channel, &DH_G, &DH_X, &DH_N);

    // Client chooses b and y on connect
    BigNum g(DH_G);
    BigNum seed(seedBytes, seedData);
    BigNum b; b.Rand(kNetDiffieHellmanKeyBits, &seed);

    // Client computes key: kb = x^b mod n
    clientSeed->PowMod(*DH_X, b, *DH_N);

    // Client sends y to server
    serverSeed->PowMod(g, b, *DH_N);
}

//============================================================================
void NetMsgCryptServerConnect (
    NetMsgChannel * channel,
    unsigned        seedBytes,
    const byte      seedData[],
    BigNum *        clientSeed
) {
    // Server computes client key: ka = y^a mod n
    const BigNum dh_y(seedBytes, seedData);
    GetCachedServerKey(channel, clientSeed, dh_y);
}

}   // namespace pnNetCli
