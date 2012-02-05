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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtRand.cpp
*   
***/

#include "pnUtRand.h"


/*****************************************************************************
*
*   Private
*
***/

class RandomContext {
    uint32_t m_seed;
    uint32_t m_value;

    void UpdateValue ();

public:
    RandomContext ();

    void     Reset ();
    void     SetSeed (unsigned seed);
    float    GetFloat ();
    float    GetFloat (float minVal, float maxVal);
    unsigned GetUnsigned ();
    unsigned GetUnsigned (unsigned minVal, unsigned maxVal);
};


/*****************************************************************************
*
*   Private data
*
***/

static const uint32_t kDefaultRandomSeed = 0x075bd924;

static RandomContext s_random;


/*****************************************************************************
*
*   RandomContext
*
***/

//============================================================================
RandomContext::RandomContext () 
:   m_seed(kDefaultRandomSeed)
{
    Reset();
}

//============================================================================
void RandomContext::UpdateValue () {
    const uint32_t A = 0xbc8f;
    const uint32_t Q = 0xadc8;
    const uint32_t R = 0x0d47;

    uint32_t div = m_value / Q;
    m_value   = A * (m_value - Q * div) - R * div;
    if (m_value > kRandomMax)
        m_value -= kRandomMax + 1;
    if (!m_value)
        m_value = kDefaultRandomSeed;
}

//============================================================================
void RandomContext::Reset () {
    m_value = m_seed;
}

//============================================================================
void RandomContext::SetSeed (unsigned seed) {
    // Never allow a seed of zero
    m_seed = seed ? seed : kDefaultRandomSeed;
    Reset();
}

//============================================================================
float RandomContext::GetFloat () {
    UpdateValue();
    return m_value * (1.0f / kRandomMax);
}

//============================================================================
float RandomContext::GetFloat (float minVal, float maxVal) {
    float value = GetFloat();
    return minVal + value * (maxVal - minVal);
}

//============================================================================
unsigned RandomContext::GetUnsigned () {
    UpdateValue();
    return (unsigned)m_value;
}

//============================================================================
unsigned RandomContext::GetUnsigned (unsigned minVal, unsigned maxVal) {
    unsigned value = GetUnsigned();
    return minVal + value % (maxVal - minVal + 1);
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void RandReset () {
    s_random.Reset();
}

//============================================================================
void RandSetSeed (unsigned seed) {
    s_random.SetSeed(seed);
}

//============================================================================
float RandFloat () {
    return s_random.GetFloat();
}

//============================================================================
float RandFloat (float minVal, float maxVal) {
    return s_random.GetFloat(minVal, maxVal);
}

//============================================================================
unsigned RandUnsigned () {
    return s_random.GetUnsigned();
}

//============================================================================
unsigned RandUnsigned (unsigned minVal, unsigned maxVal) {
    return s_random.GetUnsigned(minVal, maxVal);
}
