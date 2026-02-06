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

#ifndef plEAXStructures_h_inc
#define plEAXStructures_h_inc

typedef struct _EAXVECTOR {
        float x;
        float y;
        float z;
} EAXVECTOR;

/////////////////////////////////
//
// EAX 4.0 Listener Properties
//
typedef struct _EAXLISTENERPROPERTIES {
    unsigned long ulEnvironment;                   // Reverb properties preset index
    float         flEnvironmentSize;               // Environment size in meters
    float         flEnvironmentDiffusion;          // Environment diffusion
    long          lRoom;                           // Room effect level (at mid frequencies)
    long          lRoomHF;                         // Relative room effect level at high frequencies
    long          lRoomLF;                         // Relative room effect level at low frequencies
    float         flDecayTime;                     // Reverberation decay time at mid frequencies
    float         flDecayHFRatio;                  // High-frequency to mid-frequency decay time ratio
    float         flDecayLFRatio;                  // Low-frequency to mid-frequency decay time ratio
    long          lReflections;                    // Early reflections level relative to room effect
    float         flReflectionsDelay;              // Initial reflection delay time
    EAXVECTOR     vReflectionsPan;                 // Early reflections panning vector
    long          lReverb;                         // Late reverberation level relative to room effect
    float         flReverbDelay;                   // Late reverberation delay time relative to initial reflection
    EAXVECTOR     vReverbPan;                      // Late reverberation panning vector
    float         flEchoTime;                      // Echo time
    float         flEchoDepth;                     // Echo depth
    float         flModulationTime;                // Modulation time
    float         flModulationDepth;               // Modulation depth
    float         flAirAbsorptionHF;               // Change in level per meter at high frequencies
    float         flHFReference;                   // Reference high frequency
    float         flLFReference;                   // Reference low frequency
    float         flRoomRolloffFactor;             // Rolloff Factor for room effects
    unsigned long ulFlags;                         // Modifies the behavior of properties
} EAXLISTENERPROPERTIES, *LPEAXLISTENERPROPERTIES; // EAXLISTENERPROPERTIES

// This flag limits high-frequency decay time according to air absorption.
constexpr auto EAXLISTENERFLAGS_DECAYHFLIMIT = 0x00000020UL;


#endif //plEAXStructures_h_inc
