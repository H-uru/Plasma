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
//  plPipelineCreate Class Header                                            //
//  Cyan, Inc.                                                               //
//                                                                           //
//  Here's a fun static class. All it does is contain wrapper functions for  //
//  returning pointers to new pipelines of various types. This way, someone  //
//  else, say, plClient, can create a new plDXPipeline without having to     //
//  include every DX header in the known universe. Its messy and there's    //
//  probably a better way to do it with the resource manager, but neither    //
//  Chris nor Matt can tell me what it is, so there. :P                      //
//                                                                           //
//  Note: complile-time trick is that the actual functions for these are     //
//  in the pipeline sources themselves. Makes more sense, esp. since we      //
//  then avoid any possible header conflict between the pipelines, should    //
//  one ever exist.                                                          //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  3.8.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plPipelineCreate_h
#define _plPipelineCreate_h


//// plPipelineCreate Class Definition ////////////////////////////////////////

class plPipeline;

class plPipelineCreate
{
    protected:

        static plPipeline   *ICreateDXPipeline( hsWinRef hWnd, const hsG3DDeviceModeRecord *devMode );

    public:

        static plPipeline   *CreatePipeline( hsWinRef hWnd, const hsG3DDeviceModeRecord *devMode )
        {
            // Just this for now. Later we'll key off of the devMode
            return ICreateDXPipeline( hWnd, devMode );
        }

};


#endif // _plPipelineCreate_h

