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

#ifndef _PLAUDIO_PLAUDIOENDPOINTVOLUME_H
#define _PLAUDIO_PLAUDIOENDPOINTVOLUME_H

#include "HeadSpin.h"
#include <string_theory/string>

#include "plAudioSystem.h"

enum class plAudioEndpointType
{
    /** Represents an audio capture endpoint such as a microphone or loopback device. */
    kCapture,

    /** Represents an audio playback endpoint such as a speaker or headphones. */
    kPlayback,
};

/** Volume controller for any arbitrary audio endpoint. */
class plAudioEndpointVolume
{
public:
    virtual ~plAudioEndpointVolume() = default;

    /**
     * Gets the volume of this audio endpoint.
     * This gets the volume of the given audio endpoint as a percentage from 0.0 to 1.0, inclusive.
     */
    virtual float GetVolume() const = 0;

    /** Binds to the default audio endpoint. */
    virtual bool SetDefaultDevice(plAudioEndpointType endpoint) = 0;

    /** Binds to an audio input by name. */
    virtual bool SetDevice(plAudioEndpointType endpoint, const ST::string& deviceName) = 0;

    /**
     * Sets the volume of this audio endpoint.
     * This sets the volume of the given audio endpoint as a percentage from 0.0 to 1.0, inclusive.
     */
    virtual bool SetVolume(float pct) = 0;

    /**
     * Returns if the endpoint's volume can be manipulated.
     * \remarks A value of "false" can be for many reasons. For example, this operation may not be
     *          supported on the current platform, no endpoint was selected, an invalid endpoint was
     *          selected, or the endpoint just doesn't support volume operations.
     */
    virtual bool Supported() const = 0;

    /** Creates an instance of the volume controller for the current platform. */
    static plAudioEndpointVolume* Create();
};

#endif
