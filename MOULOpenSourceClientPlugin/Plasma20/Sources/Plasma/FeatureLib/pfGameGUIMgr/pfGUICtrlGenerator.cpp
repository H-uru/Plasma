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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfGUICtrlGenerator Definitions  										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUICtrlGenerator.h"
#include "pfGameGUIMgr.h"
#include "pfGUIControlMod.h"
#include "pfGUIDialogMod.h"
#include "pfGUIButtonMod.h"
#include "pfGUIDragBarCtrl.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIMenuItem.h"

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayer.h"
#include "../plGImage/plMipmap.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plDrawableGenerator.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../plPipeline/plTextGenerator.h"
#include "../plScene/plPostEffectMod.h"
#include "../plScene/plSceneNode.h"
#include "../pnMessage/plClientMsg.h"
#include "../plMessage/plLayRefMsg.h"
#include "../pnMessage/plAttachMsg.h"

#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUICtrlGenerator::pfGUICtrlGenerator()
{
	strcpy( fFontFace, "Arial" );
	fFontSize = 18;
}

pfGUICtrlGenerator::~pfGUICtrlGenerator()
{
	Shutdown();
}

void	pfGUICtrlGenerator::Shutdown( void )
{
	int i;
	

	// Destroy our scene nodes and dialogs
	for( i = 0; i < fDynDlgNodes.GetCount(); i++ )
	{
		pfGameGUIMgr::GetInstance()->UnloadDialog( fDynDialogs[ i ] );
		fDynDlgNodes[ i ]->GetKey()->UnRefObject();
	}
	fDynDlgNodes.Reset();
	fDynDialogs.Reset();
	
	for( i = 0; i < fTextGens.GetCount(); i++ )
		delete fTextGens[ i ];
	fTextGens.Reset();

}

//// Instance ////////////////////////////////////////////////////////////////

pfGUICtrlGenerator	&pfGUICtrlGenerator::Instance( void )
{
	static pfGUICtrlGenerator		myInstance;

	return myInstance;
}

//// IGetNextKeyName /////////////////////////////////////////////////////////

void	pfGUICtrlGenerator::IGetNextKeyName( char *name, const char *prefix )
{
	static UInt32 keyCount = 0;


	sprintf( name, "%s%d", prefix, keyCount++ );
}

//// IAddKey /////////////////////////////////////////////////////////////////

plKey pfGUICtrlGenerator::IAddKey( hsKeyedObject *ko, const char *prefix )
{
	char	keyName[ 128 ];


	IGetNextKeyName( keyName, prefix );
	return hsgResMgr::ResMgr()->NewKey( keyName, ko, plLocation::kGlobalFixedLoc );
}

//// SetFont /////////////////////////////////////////////////////////////////

void	pfGUICtrlGenerator::SetFont( const char *face, UInt16 size )
{
	strcpy( fFontFace, face );
	fFontSize = size;
}

//// ICreateSolidMaterial ////////////////////////////////////////////////////
//	Creates a material with no texture, just color.

hsGMaterial	*pfGUICtrlGenerator::ICreateSolidMaterial( hsColorRGBA &color )
{
	hsColorRGBA		black;


	// Create a material with a simple blank layer, fully ambient
	hsGMaterial *material = TRACKED_NEW hsGMaterial;
	IAddKey( material, "GUIMaterial" );

	plLayer *lay = material->MakeBaseLayer();
	black.Set( 0.f,0.f,0.f,1.f );

	lay->SetRuntimeColor( black );
	lay->SetPreshadeColor( black );
	lay->SetAmbientColor( color );

	return material;
}

//// ICreateTextMaterial /////////////////////////////////////////////////////
//	Creates a material with a texture that has a string centered on it.

hsGMaterial	*pfGUICtrlGenerator::ICreateTextMaterial( const char *text, hsColorRGBA &bgColor, 
													 hsColorRGBA &textColor, float objWidth, float objHeight )
{
	UInt16			pixWidth, pixHeight, strWidth, strHeight;
	hsColorRGBA		black, white;


	// Guess at some pixel width and heights we want. We're guessing b/c we want it to look reasonably
	// good on the screen, but we don't know exactly how big is big, so we guess
	pixWidth = (UInt16)(objWidth * 64.f);
	pixHeight = (UInt16)(objHeight * 64.f);

	// Create blank mipmap
	plMipmap *bitmap = TRACKED_NEW plMipmap( 1, 1, plMipmap::kRGB32Config, 1 );
	IAddKey( bitmap, "GUIMipmap" );

	// Create textGen to write string with
	plTextGenerator *textGen = TRACKED_NEW plTextGenerator( bitmap, pixWidth, pixHeight );
	textGen->SetFont( fFontFace, (UInt16)fFontSize );
	textGen->ClearToColor( bgColor );
	textGen->SetTextColor( textColor );
	strWidth = textGen->CalcStringWidth( text, &strHeight );
	textGen->DrawString( ( pixWidth - strWidth ) >> 1, ( pixHeight - strHeight ) >> 1, text );
	textGen->FlushToHost();
	fTextGens.Append( textGen );

	// Create a material with a simple blank layer, fully ambient
	hsGMaterial *material = TRACKED_NEW hsGMaterial;
	IAddKey( material, "GUIMaterial" );

	plLayer *lay = material->MakeBaseLayer();
	white.Set( 1.f,1.f,1.f,1.f );
	black.Set( 0.f,0.f,0.f,1.f );

	lay->SetRuntimeColor( black );
	lay->SetPreshadeColor( black );
	lay->SetAmbientColor( white );

	hsgResMgr::ResMgr()->AddViaNotify( bitmap->GetKey(), TRACKED_NEW plLayRefMsg( lay->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );
//	lay->SetTexture( bitmap );
	lay->SetTransform( textGen->GetLayerTransform() );

	return material;
}

//// GenerateDialog //////////////////////////////////////////////////////////

void	pfGUICtrlGenerator::GenerateDialog( const char *name )
{
	IGenerateDialog( name, 20.f, false );
}

//// IGenSceneObject /////////////////////////////////////////////////////////

plSceneObject	*pfGUICtrlGenerator::IGenSceneObject( pfGUIDialogMod *dlg, plDrawable *myDraw, plSceneObject *parent,
														hsMatrix44 *l2w, hsMatrix44 *w2l )
{
	plKey snKey = ( dlg != nil ) ? ( dlg->GetTarget() != nil ? dlg->GetTarget()->GetSceneNode() : nil ) : nil;
	if( snKey == nil )
		snKey = fDynDlgNodes.Peek()->GetKey();

	hsgResMgr::ResMgr()->SendRef( myDraw->GetKey(), TRACKED_NEW plNodeRefMsg( snKey, plRefMsg::kOnCreate, 0, plNodeRefMsg::kDrawable ), plRefFlags::kActiveRef );		

	plDrawInterface *newDI = TRACKED_NEW plDrawInterface;
	IAddKey( newDI, "GUIDrawIFace" );

	plSceneObject	*newObj = TRACKED_NEW plSceneObject;
	IAddKey( newObj, "GUISceneObject" );

	plCoordinateInterface *newCI = TRACKED_NEW plCoordinateInterface;
	IAddKey( newCI, "GUICoordIFace" );

	hsgResMgr::ResMgr()->SendRef( newCI->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );
	hsgResMgr::ResMgr()->SendRef( newDI->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );
	hsgResMgr::ResMgr()->SendRef( myDraw->GetKey(), TRACKED_NEW plIntRefMsg( newDI->GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kDrawable ), plRefFlags::kActiveRef );

	if( parent == nil )
	{
		parent = ( fDynDragBars.GetCount() > 0 ) ? fDynDragBars.Peek() : nil;
		if( parent == nil )
			parent = dlg->GetTarget();
	}

	if( parent != nil )
//		hsgResMgr::ResMgr()->SendRef( newCI->GetKey(), TRACKED_NEW plIntRefMsg( parent->GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kChild ), plRefFlags::kActiveRef );
		hsgResMgr::ResMgr()->SendRef( newCI->GetKey(), TRACKED_NEW plAttachMsg( parent->GetKey(), nil, plRefMsg::kOnRequest ), plRefFlags::kActiveRef );
	
	newObj->SetSceneNode( snKey );

	if( l2w != nil )
	{
		newObj->SetTransform( *l2w, *w2l );
//		newCI->SetLocalToParent( *l2w, *w2l );
//		myDraw->SetTransform( -1, *l2w, *w2l );
	}

	return newObj;
}

//// GenerateRectButton //////////////////////////////////////////////////////

pfGUIButtonMod	*pfGUICtrlGenerator::GenerateRectButton( const char *title, float x, float y, float width, float height,
									const char *consoleCmd, hsColorRGBA &color, hsColorRGBA &textColor )
{
	hsGMaterial		*material;
	hsMatrix44		l2w, w2l;
	hsVector3		vec;
	pfGUIDialogMod	*dlgToAddTo = IGetDialog();


	// Get us a material
	material = ICreateTextMaterial( title, color, textColor, width * 20.f, height * 20.f );

	pfGUIButtonMod *but = CreateRectButton( dlgToAddTo, title, x, y, width, height, material );
	if( but != nil )
		but->SetHandler( TRACKED_NEW pfGUIConsoleCmdProc( consoleCmd ) );

	return but;
}

//// CreateRectButton ////////////////////////////////////////////////////////

pfGUIButtonMod	*pfGUICtrlGenerator::CreateRectButton( pfGUIDialogMod *parent, const char *title, float x, float y, float width, float height,
									hsGMaterial *material, hsBool asMenuItem )
{
	wchar_t *wTitle = hsStringToWString(title);
	pfGUIButtonMod *retVal = CreateRectButton(parent,wTitle,x,y,width,height,material,asMenuItem);
	delete [] wTitle;
	return retVal;
}

pfGUIButtonMod	*pfGUICtrlGenerator::CreateRectButton( pfGUIDialogMod *parent, const wchar_t *title, float x, float y, float width, float height,
									hsGMaterial *material, hsBool asMenuItem )
{
	plDrawableSpans	*myDraw;
	hsMatrix44		l2w, w2l;
	hsVector3		vec;


	// Translate x and y from (0:1) to (-10:10)
	x = ( x - 0.5f ) * 20.f;
	y = ( y - 0.5f ) * 20.f;
	// Translate width and height from (0:1) to (-10:10)
	width *= 20.f;
	height *= 20.f;

	// Create drawable that is rectangular
	l2w.Reset();
	hsPoint3 corner( x, -y, -100 );
	hsVector3 xVec( width, 0, 0 ), yVec( 0, height, 0 ), zVec( 0, 0, 0.1f );

	myDraw = plDrawableGenerator::GeneratePlanarDrawable( corner, xVec, yVec, material, l2w );

	plSceneObject *newObj = IGenSceneObject( parent, myDraw );

	pfGUIButtonMod *newBtn = asMenuItem ? TRACKED_NEW pfGUIMenuItem : TRACKED_NEW pfGUIButtonMod;
	IAddKey( newBtn, "GUIButton" );
	hsgResMgr::ResMgr()->SendRef( newBtn->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );
	parent->AddControl( newBtn );
	hsgResMgr::ResMgr()->AddViaNotify( newBtn->GetKey(), TRACKED_NEW plGenRefMsg( parent->GetKey(), plRefMsg::kOnCreate, parent->GetNumControls() - 1, pfGUIDialogMod::kControlRef ), plRefFlags::kActiveRef );

	return newBtn;
}

//// GenerateSphereButton ////////////////////////////////////////////////////

pfGUIButtonMod	*pfGUICtrlGenerator::GenerateSphereButton( float x, float y, float radius,
															const char *consoleCmd, hsColorRGBA &color )
{
	hsGMaterial		*material;
	plDrawableSpans	*myDraw;
	hsMatrix44		l2w, w2l;
	hsVector3		vec;
	hsPoint3		pt( x, -y, -100.f );
	pfGUIDialogMod	*dlgToAddTo = IGetDialog();


	// Translate x and y from (0:1) to (-10:10)
	x = ( x - 0.5f ) * 20.f;
	y = ( y - 0.5f ) * 20.f;
	// Translate width and height from (0:1) to (-10:10)
	radius *= 20.f;

	// Get us a material
	material = ICreateSolidMaterial( color );

	// Create drawable that is rectangular
	l2w.Reset();
	// We bump up the quality since we're actually far closer to these things then the normal
	// world camera would put us
	myDraw = plDrawableGenerator::GenerateSphericalDrawable( pt, radius, material, l2w,
															false, nil, nil, nil, 100.f );

	vec.Set( x, -y, 0 );
	l2w.MakeTranslateMat( &vec );
	l2w.GetInverse( &w2l );

	plSceneObject *newObj = IGenSceneObject( dlgToAddTo, myDraw );//, nil, &l2w, &w2l );

	pfGUIButtonMod *newBtn = TRACKED_NEW pfGUIButtonMod;
	IAddKey( newBtn, "GUIButton" );
	newBtn->SetHandler( TRACKED_NEW pfGUIConsoleCmdProc( consoleCmd ) );
	hsgResMgr::ResMgr()->AddViaNotify( newBtn->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );
	dlgToAddTo->AddControl( newBtn );

	return newBtn;
}

//// GenerateDragBar //////////////////////////////////////////////////////

pfGUIDragBarCtrl *pfGUICtrlGenerator::GenerateDragBar( float x, float y, float width, float height, hsColorRGBA &color )
{
	hsGMaterial		*material;
	plDrawableSpans	*myDraw;
	hsMatrix44		l2w, w2l;
	hsVector3		vec;
	pfGUIDialogMod	*dlgToAddTo = IGetDialog();


	// Translate x and y from (0:1) to (-10:10)
	x = ( x - 0.5f ) * 20.f;
	y = ( y - 0.5f ) * 20.f;
	// Translate width and height from (0:1) to (-10:10)
	width *= 20.f;
	height *= 20.f;

	// Get us a material
	material = ICreateSolidMaterial( color );

	// Create drawable that is rectangular
	l2w.Reset();

	hsPoint3 corner( x, -y, -100 );//x - width / 2.f, -y - height / 2.f, -100 );
	hsVector3 xVec( width, 0, 0 ), yVec( 0, height, 0 ), zVec( 0, 0, 0.1f );

	myDraw = plDrawableGenerator::GenerateBoxDrawable( corner, xVec, yVec, zVec,/*width, height, 0.01f, */material, l2w );

	// Drag bars are special--everything else gets attached to them and they get attached to the dialog
	vec.Set( x, -y, -100 );
	l2w.MakeTranslateMat( &vec );
	l2w.GetInverse( &w2l );

	plSceneObject *newObj = IGenSceneObject( dlgToAddTo, myDraw, dlgToAddTo->GetTarget(), &l2w, &w2l );

	fDynDragBars[ fDynDragBars.GetCount() - 1 ] = newObj;

	pfGUIDragBarCtrl *newBtn = TRACKED_NEW pfGUIDragBarCtrl;
	IAddKey( newBtn, "GUIDragBar" );
	hsgResMgr::ResMgr()->AddViaNotify( newBtn->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );
	dlgToAddTo->AddControl( newBtn );

/*	vec.Set( -x, y, 100 );
	l2w.MakeTranslateMat( &vec );
	l2w.GetInverse( &w2l );

	plCoordinateInterface *ci = (plCoordinateInterface *)dlgToAddTo->GetTarget()->GetCoordinateInterface();
	ci->SetLocalToParent( l2w, w2l );
*/
	return newBtn;
}

//// IGetDialog //////////////////////////////////////////////////////////////

pfGUIDialogMod	*pfGUICtrlGenerator::IGetDialog( void )
{
	if( fDynDialogs.GetCount() == 0 )
		IGenerateDialog( "GUIBaseDynamicDlg", 20.f );

	hsAssert( fDynDialogs.GetCount() > 0, "Unable to get a dynamic dialog to add buttons to" );
	return fDynDialogs.Peek();
}

//// IGenerateDialog /////////////////////////////////////////////////////////

pfGUIDialogMod	*pfGUICtrlGenerator::IGenerateDialog( const char *name, float scrnWidth, hsBool show )
{
	float			fovX, fovY;
	plSceneNode		*node;
	pfGUIDialogMod	*dialog;


	// Create the rendermod
	plPostEffectMod	*renderMod = TRACKED_NEW plPostEffectMod;
	IAddKey( renderMod, "GUIRenderMod" );

	renderMod->SetHither( 0.5f );
	renderMod->SetYon( 200.f );

	// fovX should be such that scrnWidth is the projected width at z=100
	fovX = atan( scrnWidth / ( 2.f * 100.f ) ) * 2.f;
	fovY = fovX;// * 3.f / 4.f;

	renderMod->SetFovX( fovX * 180.f / hsScalarPI );
	renderMod->SetFovY( fovY * 180.f / hsScalarPI );

	// Create the sceneNode to go with it
	node = TRACKED_NEW plSceneNode;
	IAddKey( node, "GUISceneNode" );
	node->GetKey()->RefObject();
	fDynDlgNodes.Append( node );
	fDynDragBars.Append( nil );

	hsgResMgr::ResMgr()->AddViaNotify( node->GetKey(), TRACKED_NEW plGenRefMsg( renderMod->GetKey(), plRefMsg::kOnCreate, 0, plPostEffectMod::kNodeRef ), plRefFlags::kPassiveRef );		

	// Create the dialog
	dialog = TRACKED_NEW pfGUIDialogMod;
	IAddKey( dialog, "GUIDialog" );

	dialog->SetRenderMod( renderMod );
	dialog->SetName( name );

	// Create the dummy scene object to hold the dialog
	plSceneObject	*newObj = TRACKED_NEW plSceneObject;
	IAddKey( newObj, "GUISceneObject" );

	// *#&$(*@&#$ need a coordIface...
	plCoordinateInterface *newCI = TRACKED_NEW plCoordinateInterface;
	IAddKey( newCI, "GUICoordIFace" );

	hsMatrix44 l2w, w2l;
	l2w.Reset();
//	l2w.NotIdentity();

	l2w.GetInverse( &w2l );

	// Using SendRef here because AddViaNotify will queue the messages up, which doesn't do us any good
	// if we need these refs right away
	hsgResMgr::ResMgr()->SendRef( dialog->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );		

	hsgResMgr::ResMgr()->AddViaNotify( newCI->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );		
	hsgResMgr::ResMgr()->AddViaNotify( renderMod->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );		

	// Add the dialog to the GUI mgr
	plGenRefMsg *refMsg = TRACKED_NEW plGenRefMsg( pfGameGUIMgr::GetInstance()->GetKey(), 
											plRefMsg::kOnCreate, 0, pfGameGUIMgr::kDlgModRef );
	hsgResMgr::ResMgr()->AddViaNotify( dialog->GetKey(), refMsg, plRefFlags::kActiveRef );		

	newObj->SetSceneNode( node->GetKey() );

	newObj->SetTransform( l2w, w2l );
//	newCI->SetLocalToParent( l2w, w2l );

	if( show )
		pfGameGUIMgr::GetInstance()->ShowDialog( dialog );

	fDynDialogs.Append( dialog );
	return dialog;
}
