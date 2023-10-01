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
//  plPlates - Header file for the plates and plPlateManager                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plPlates_h
#define _plPlates_h

#include "HeadSpin.h"

#include <string_theory/string>
#include <vector>

#include "hsColorRGBA.h"

#include "hsMatrix44.h"



//// plPlate Class Definition ////////////////////////////////////////////////
//  plPlate is the actual plate object that represents one plate on the 
//  screen. It has a transform matrix (which includes position, scale and
//  rotation), a material, a depth value and a color that is applied to all
//  four corners. All plates are parallelograms. 

class plPlateManager;
class hsGMaterial;
class plMipmap;
class plBitmap;

class plPlate
{
    friend class plPlateManager;

    protected:
        
        hsMatrix44      fXformMatrix;
        hsGMaterial     *fMaterial;
        plMipmap        *fMipmap;
        float        fDepth, fOpacity;
        uint32_t          fFlags;
        ST::string fTitle;

        plPlate         *fNext;
        plPlate         **fPrevPtr;

        plPlate         **fOwningHandle;

        static uint32_t   fMagicUniqueKeyInt;

        plPlate( plPlate** owningHandle );
        virtual ~plPlate();

        void    ILink( plPlate **back );

        void    IUnlink()
        {
            hsAssert( fPrevPtr, "Plate not in list" );
            if( fNext )
                fNext->fPrevPtr = fPrevPtr;
            *fPrevPtr = fNext;

            fNext = nullptr;
            fPrevPtr = nullptr;
        }

        void ISetResourceAlphas(uint32_t colorKey);

    public:

        enum 
        {
            kFlagVisible        = 0x00000001,
            kFlagLocalMaterial  = 0x00000002,
            kFlagIsAGraph       = 0x00000004
        };

        /// Basic properties

        void    SetTransform( hsMatrix44 &matrix, bool reSort = true );
        void    SetMaterial( hsGMaterial *material );
        void    SetTexture(plBitmap *texture); // Creates a new single layer material to use the texture.
        void SetTitle(ST::string title) { fTitle = std::move(title); }

        hsGMaterial     *GetMaterial() { return fMaterial; }
        hsMatrix44      &GetTransform() { return fXformMatrix; }
        ST::string GetTitle() { return fTitle; }
        uint32_t          GetFlags() { return fFlags; }
        const plMipmap  *GetMipmap() { return fMipmap; }

        void    SetVisible( bool vis ) { if( vis ) fFlags |= kFlagVisible; else fFlags &= ~kFlagVisible; }
        bool    IsVisible();

        void    SetOpacity( float opacity = 1.f );

        plPlate *GetNext() { return fNext; }


        /// Helper functions
        
        void    SetDepth( float depth) { fDepth = depth; }
        void    SetPosition( float x, float y, float z = -1.0f );
        void    SetSize( float width, float height, bool adjustByAspectRatio = false );

        plMipmap        *CreateMaterial(uint32_t width, uint32_t height, bool withAlpha, plMipmap* texture = nullptr);
        void            CreateFromResource(const ST::string& resName);
        void            ReloadFromResource(const ST::string& resName);
};

//// plGraphPlate Class Definition ///////////////////////////////////////////
//  A derivation of plPlate that maintains a procedural texture which displays
//  a scrolling graph of data.

class plGraphPlate : public plPlate
{
    protected:
        uint32_t          fBGHexColor, fAxesHexColor, fGraphHexColor;
        std::vector<uint32_t> fDataHexColors;
        uint32_t          fMin, fMax, fLabelMin, fLabelMax;
        std::vector<int32_t>  fLastValues;
        std::vector<ST::string> fLabelText;

        uint32_t      IMakePow2( uint32_t value );
        void        IDrawNumber( uint32_t number, uint32_t *dataPtr, uint32_t stride, uint32_t color );
        void        IDrawDigit( char digit, uint32_t *dataPtr, uint32_t stride, uint32_t color );

    public:
        plGraphPlate( plPlate **owningHandle );
        virtual ~plGraphPlate();

        void    SetDataRange( uint32_t min, uint32_t max, uint32_t width );
        void    SetDataLabels( uint32_t min, uint32_t max );
        void SetLabelText(std::vector<ST::string> text) { fLabelText = std::move(text); }
        void    ClearData();

        void    AddData( int32_t value, int32_t value2 = -1, int32_t value3 = -1, int32_t value4 = -1 );
        void    AddData( std::vector<int32_t> values );

        void    SetColors( uint32_t bgHexColor = 0x80000000, uint32_t axesHexColor = 0xffffffff, uint32_t dataHexColor = 0xff00ff00, uint32_t graphHexColor = 0x80ff0000 );
        void    SetDataColors( uint32_t hexColor1 = 0xff00ff00, uint32_t hexColor2 = 0xff0000ff, uint32_t hexColor3 = 0xffffff00, uint32_t hexColor4 = 0xffff00ff );
        void    SetDataColors( const std::vector<uint32_t> & hexColors );

        ST::string GetLabelText(int i) { return fLabelText[i]; }
        uint32_t        GetDataColor( int i ) { return fDataHexColors[ i ]; }
        uint32_t        GetNumLabels() { return (uint32_t)fLabelText.size(); }
        uint32_t        GetNumColors() { return (uint32_t)fDataHexColors.size(); }
};

//// plPlateManager Class Definition /////////////////////////////////////////
//  This class handles all the plates--it keeps track of all the plates, 
//  creates and destroys them, and draws them when the pipeline tells it to.

class plPipeline;

class plPlateManager
{
    friend class plPlate;

    private:

        static plPlateManager   *fInstance;

    protected:

        plPlate     *fPlates;
        plPipeline  *fOwner;
        bool        fCreatedSucessfully;

        plPlateManager( plPipeline *pipe ) 
        {
            fInstance = this;
            fPlates = nullptr;
            fOwner = pipe;
            fCreatedSucessfully = true;
        }

        virtual void    IDrawToDevice( plPipeline *pipe ) = 0;

        void            IResortPlate( plPlate *plate, bool fromCurrent );

    public:

        virtual ~plPlateManager();
        
        static plPlateManager   &Instance() { return *fInstance; }
        static bool InstanceValid() { return fInstance != nullptr; }

        void        CreatePlate( plPlate **handle );
        void        CreatePlate( plPlate **handle, float width, float height );
        void        CreatePlate( plPlate **handle, float x, float y, float width, float height );

        void        CreateGraphPlate( plGraphPlate **handle );

        void        DestroyPlate( plPlate *plate );

        void        SetPlateScreenPos( plPlate *plate, uint32_t x, uint32_t y );
        void        SetPlatePixelSize( plPlate *plate, uint32_t pWidth, uint32_t pHeight );

        uint32_t      GetPipeWidth();
        uint32_t      GetPipeHeight();
        void        DrawToDevice( plPipeline *pipe );

        bool        IsValid() { return fCreatedSucessfully; }
};

// Sets the hInstance that we load our resources from.  A SceneViewer hack.
void SetHInstance(void *instance);

#endif //_plPlates_h

