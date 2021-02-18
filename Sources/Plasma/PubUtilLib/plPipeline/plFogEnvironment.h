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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plFogEnvironment.h - Header file for the fog environment class          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plFogEnvironment_h
#define _plFogEnvironment_h

#include "HeadSpin.h"
#include "hsColorRGBA.h"

#include "pnKeyedObject/hsKeyedObject.h"


//// plFogEnvironment Class Definition ////////////////////////////////////////////
//  Defines a fog environment. Bite me.

class plFogEnvironment : public hsKeyedObject
{
    protected:

        uint8_t       fType;
        float    fStart;         // Used for linear fog only
        float    fEnd, fDensity; // Always used!
        hsColorRGBA fColor;

    public:

        CLASSNAME_REGISTER( plFogEnvironment );
        GETINTERFACE_ANY( plFogEnvironment, hsKeyedObject );

        enum FogType
        {
            kLinearFog      = 0,
            kExpFog,
            kExp2Fog,
            kNoFog
        };

        plFogEnvironment();
        plFogEnvironment( float start, float end, float density, hsColorRGBA &color );
        plFogEnvironment( FogType type, float end, float density, hsColorRGBA &color );
        ~plFogEnvironment();

        plFogEnvironment(const plFogEnvironment &copy) { operator=(copy); }
        plFogEnvironment &operator=(const plFogEnvironment &copy);

        // Sets the parameters for linear fog
        void    Set(float start, float end, float density, const hsColorRGBA *color = nullptr);

        // Sets the parameters for exp or exp^2 fog
        void    SetExp(FogType type, float end, float density, const hsColorRGBA *color = nullptr);

        // Sets the color
        void    SetColor( hsColorRGBA &color ) { fColor = color; }

        // Clear the environment to no fog
        void    Clear() { fType = kNoFog; }

        // Gets the type
        uint8_t   GetType() const { return fType; }

        // Gets the color
        hsColorRGBA &GetColor() { return fColor; }

        // Gets the parameters. Sets start to 0 if the type is not linear (can be nil)
        void    GetParameters( float *start, float *end, float *density, hsColorRGBA *color ) const;

        // Gets linear pipeline (DX) specific parameters.
        void    GetPipelineParams( float *start, float *end, hsColorRGBA *color ) const;

        // Gets exp or exp^2 pipeline (DX) specific parameters.
        void    GetPipelineParams( float *density, hsColorRGBA *color ) const;

        void Read(hsStream *s, hsResMgr *mgr) override;
        void Write(hsStream *s, hsResMgr *mgr) override;
};

#endif //_plFogEnvironment_h

