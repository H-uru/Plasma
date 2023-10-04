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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plLayerConverter - Utility class that converts plPlasmaMAXLayers into    //
//                     other stuff.                                          //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  1.13.2002 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plLayerConverter_h
#define _plLayerConverter_h

#include <vector>

//// Class Definition /////////////////////////////////////////////////////////

class plErrorMsg;
class plFileName;
class plLayerInterface;
class plMaxNode;
class plPlasmaMAXLayer;
class plLayer;
class plLocation;
class plBitmapData;
class plDynamicTextMap;
class plBitmap;
class plCubicRenderTarget;
class hsConverterUtils;
class plDynamicTextLayer;

class plLayerConverter
{
    private:

        plLayerConverter();

    protected:

        static const Class_ID   fDerivedTypes[];

    public:

        ~plLayerConverter();
        static plLayerConverter &Instance();

        void    Init( bool save, plErrorMsg *msg );
        void    DeInit();

        plLayerInterface    *ConvertTexmap( Texmap *texmap, plMaxNode *maxNode,
                                            uint32_t blendFlags, bool preserveUVOffset, bool upperLayer );
        plBitmap *CreateSimpleTexture(plFileName fileName, const plLocation &loc, uint32_t clipID = 0, uint32_t texFlags = 0, bool usePNG = false);
        
        void    MuteWarnings();
        void    UnmuteWarnings();

    protected:

        plErrorMsg  *fErrorMsg;
        uint32_t      fWarned, fSavedWarned;
        bool        fSaving;

        Interface           *fInterface;
        hsConverterUtils    &fConverterUtils;

        const MCHAR* fDbgNodeName;

        std::vector<plPlasmaMAXLayer *> fConvertedLayers;


        plLayer             *ICreateLayer( const ST::string &name, bool upperLayer, plLocation &loc );
        void                IProcessUVGen( plPlasmaMAXLayer *srcLayer, plLayer *destLayer, plBitmapData *bitmapData, bool preserveUVOffset );
        plDynamicTextMap    *ICreateDynTextMap( const ST::string &layerName, uint32_t width, uint32_t height, bool includeAlpha, plMaxNode *node );

        plLayer             *IAssignTexture( plBitmapData *bd, plMaxNode *maxNode, plLayer *destLayer, bool upperLayer, int clipID = -1 );
        plCubicRenderTarget *IMakeCubicRenderTarget( const ST::string &name, plMaxNode *maxNode, plMaxNode *anchor );

        // Add your function to process your layer type here
        plLayerInterface    *IConvertLayerTex( plPlasmaMAXLayer *layer, plMaxNode *maxNode, uint32_t blendFlags, bool preserveUVOffset, bool upperLayer );
        plLayerInterface    *IConvertStaticEnvLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, uint32_t blendFlags, bool preserveUVOffset, bool upperLayer );
        plLayerInterface    *IConvertDynamicEnvLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, uint32_t blendFlags, bool preserveUVOffset, bool upperLayer );
        plLayerInterface    *IConvertCameraLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, uint32_t blendFlags, bool preserveUVOffset, bool upperLayer );
        plLayerInterface    *IConvertDynamicTextLayer( plPlasmaMAXLayer *layer, plMaxNode *maxNode, uint32_t blendFlags, bool preserveUVOffset, bool upperLayer );

        plBitmap*           IGetAttenRamp( plMaxNode *maxNode, BOOL isAdd, int loClamp, int hiClamp);
        plLayer*            ICreateAttenuationLayer(const ST::string& name, plMaxNode *maxNode, int uvwSrc, float tr0, float op0, float tr1, float op1, int loClamp, int hiClamp);
        plLayerInterface*   IConvertAngleAttenLayer(plPlasmaMAXLayer *layer, plMaxNode *maxNode, uint32_t blendFlags, bool preserveUVOffset, bool upperLayer);

        void                IRegisterConversion( plPlasmaMAXLayer *origLayer, plLayerInterface *convertedLayer );

        uint32_t            *IGetInitBitmapBuffer( plDynamicTextLayer *layer ) const;

};

#endif // _plLayerConverter_h
