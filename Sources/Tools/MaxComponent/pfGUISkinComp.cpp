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

#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "MaxMain/MaxCompat.h"
#include "MaxMain/plMaxNodeBase.h"
#include "resource.h"

#include "pfGUISkinComp.h"
#include "plGUICompClassIDs.h"


pfGUISkinEditProc   *pfGUISkinEditProc::fInstance = nullptr;
int                 pfGUISkinEditProc::fZoom = 3;       // So re-opening the dialog will keep the same zoom level

extern HINSTANCE hInstance;

pfGUISkinEditProc::pfGUISkinEditProc( plGUISkinComp *comp ) 
{
    fInstance = this; 
    fComp = comp;

    fDblDC = nullptr;
    fDblBitmap = nullptr;

    fXOffset = fYOffset = 0;

    fCurrPBRefSet = plGUISkinComp::kRefUpLeftCorner;

    fDefPen = CreatePen( PS_SOLID, 3, RGB( 200, 0, 0 ) );
    fOtherPen = CreatePen( PS_DOT, 1, RGB( 200, 0, 0 ) );

    fDragging = false;
    fDragTimer = 0;

    // Back up the old settings
    IParamBlock2 *pb = fComp->GetParamBlockByID( plComponent::kBlkComp );
    for( int i = 0; i < pfGUISkin::kNumElements; i++ )
    {
        int id = i * 4 + plGUISkinComp::kRefUpLeftCorner;
        fBackups[ i ].fX = pb->GetInt( id + 0 ); 
        fBackups[ i ].fY = pb->GetInt( id + 1 ); 
        fBackups[ i ].fWidth = pb->GetInt( id + 2 ); 
        fBackups[ i ].fHeight = pb->GetInt( id + 3 ); 
    }
}

pfGUISkinEditProc::~pfGUISkinEditProc() 
{
    fInstance = nullptr;
    DeleteObject( fDefPen );
    IKillDblBuffer();
}

void    pfGUISkinEditProc::IJustDrawOneRect( int whichElement, IParamBlock2 *pb, HDC hDC, HPEN whichPen, int refToIgnore )
{
    whichElement = ( whichElement * 4 ) + plGUISkinComp::kRefUpLeftCorner;

    if( whichElement == refToIgnore )
        return;

    RECT    r;
    SetRect( &r, pb->GetInt( whichElement + 0 ),
                 pb->GetInt( whichElement + 1 ),
                 pb->GetInt( whichElement + 0 ) + pb->GetInt( whichElement + 2 ),
                 pb->GetInt( whichElement + 1 ) + pb->GetInt( whichElement + 3 ) );
    
    r.left *= fZoom; r.right *= fZoom; r.top *= fZoom; r.bottom *= fZoom;

    SelectObject( hDC, whichPen );
    int rop2 = SetROP2( hDC, R2_NOTXORPEN );

    MoveToEx(hDC, r.left, r.top, nullptr);
    LineTo( hDC, r.right, r.top );
    LineTo( hDC, r.right, r.bottom );
    LineTo( hDC, r.left, r.bottom );
    LineTo( hDC, r.left, r.top );

    SetROP2( hDC, rop2 );
}

void    pfGUISkinEditProc::IRefreshDblBuffer()
{
    // Image buffer is where we keep our resized image. Dbl buffer is where we draw our bounds
    if (fDblDC == nullptr)
        IInitDblBuffer();
    else
    {
        // Copy the zoomed image as our backdrop
        BitBlt( fDblDC, 0, 0, fDblWidth, fDblHeight, fImageDC, 0, 0, SRCCOPY );

        RECT    r;
        IParamBlock2    *pb = fComp->GetParamBlockByID( plComponent::kBlkComp );
        if (pb != nullptr)
        {
            // Draw all the other elements other than our current one
            for( int i = 0; i < pfGUISkin::kNumElements; i++ )
                IJustDrawOneRect( i, pb, fDblDC, fOtherPen, fCurrPBRefSet );

            // Now draw the bounds of our current element
            SetRect( &r, pb->GetInt( fCurrPBRefSet + 0 ),
                         pb->GetInt( fCurrPBRefSet + 1 ),
                         pb->GetInt( fCurrPBRefSet + 0 ) + pb->GetInt( fCurrPBRefSet + 2 ),
                         pb->GetInt( fCurrPBRefSet + 1 ) + pb->GetInt( fCurrPBRefSet + 3 ) );
            
            // While we have it here, go ahead and update our status text for this element
            TCHAR str[ 256 ];
            _sntprintf( str, std::size(str), _T("Left: %d\nTop: %d\nWidth: %d\nHeight: %d\n"), r.left, r.top, r.right - r.left, r.bottom - r.top );
            SetDlgItemText( fHWnd, IDC_GUI_INFO, str );
                            
            r.left *= fZoom; r.right *= fZoom; r.top *= fZoom; r.bottom *= fZoom;

            SelectObject( fDblDC, fDefPen );
            int rop2 = SetROP2( fDblDC, R2_NOTXORPEN );

            MoveToEx(fDblDC, r.left, r.top, nullptr);
            LineTo( fDblDC, r.right, r.top );
            LineTo( fDblDC, r.right, r.bottom );
            LineTo( fDblDC, r.left, r.bottom );
            LineTo( fDblDC, r.left, r.top );

            SetROP2( fDblDC, rop2 );

            fCurrElemRect = r;
            MapWindowPoints( GetDlgItem( fHWnd, IDC_GUI_PREVIEW ), fHWnd, (POINT *)&fCurrElemRect, 2 );
            OffsetRect( &fCurrElemRect, -fXOffset, -fYOffset );
        }
    }
}

void    pfGUISkinEditProc::IRefreshImageBuffer()
{
    IInitDblBuffer();

    plLayerTex *layer = fComp->GetSkinBitmap();
    PBBitmap *pbBMap = layer->GetPBBitmap();

    if (pbBMap->bm == nullptr)
        pbBMap->Load();
    if (pbBMap->bm != nullptr)
    {
        // Copy into a new temp bitmap that is the right format for us to read
        Bitmap *newBM;
        BitmapInfo bi;
        bi.SetName( _T("y879873b") );
        bi.SetType( BMM_TRUE_32 );
        bi.SetFlags( MAP_HAS_ALPHA );
        bi.SetWidth( fDblWidth );
        bi.SetHeight( fDblHeight );
        newBM = TheManager->Create( &bi );

        BMM_Color_64 foo = BMMCOLOR(0, 0, 0, 0);
        newBM->CopyImage(pbBMap->bm, COPY_IMAGE_RESIZE_LO_QUALITY, foo, nullptr);

        // Now copy from our newly created bitmap into our DC....way slow :(
        BITMAPINFO *bitInfo = newBM->ToDib(24, nullptr, false);
        if (bitInfo != nullptr)
        {
            SetDIBitsToDevice( fImageDC, 0, 0, fDblWidth, fDblHeight,
                                0, 0, 0, fDblHeight,
                                ( (uint8_t *)bitInfo ) + bitInfo->bmiHeader.biSize,
                                bitInfo,
                                DIB_RGB_COLORS );
        }

        newBM->DeleteThis();
    }

    IRefreshDblBuffer();
}

void    pfGUISkinEditProc::IInitDblBuffer()
{
    if (fDblDC == nullptr)
    {
        int     width, height;
        HDC     desk = GetDC(nullptr);

        plLayerTex *layer = fComp->GetSkinBitmap();
        PBBitmap *pbBMap = layer->GetPBBitmap();
        if (pbBMap == nullptr)
            return;
        width = pbBMap->bi.Width() * fZoom;
        height = pbBMap->bi.Height() * fZoom;
//              GetClientRect( fHWnd, &r );
//              width = r.right - r.left;
//              height = r.bottom - r.top;

        //  Note: For some strange reason, grabbing the HDC of the window doesn't do
        //  any good, 'cause it's black-and-white (right, THAT makes sense). Grabbing
        //  the desktop DC works, however.

        fDblDC = CreateCompatibleDC( desk );
        fDblBitmap = CreateCompatibleBitmap( desk/*fDblDC*/, width, height );
        SelectObject( fDblDC, fDblBitmap );

        fImageDC = CreateCompatibleDC( desk );
        fImageBitmap = CreateCompatibleBitmap( desk/*fDblDC*/, width, height );
        SelectObject( fImageDC, fImageBitmap );

        ReleaseDC(nullptr, desk);

        fDblWidth = width;
        fDblHeight = height;

        ISetScrollRanges();
        IRefreshImageBuffer();
    }
}

void    pfGUISkinEditProc::IKillDblBuffer()
{
    if (fDblDC != nullptr)
    {
        SelectObject(fDblDC, nullptr);
        DeleteObject( fDblBitmap );
        DeleteDC( fDblDC );
    }

    if (fImageDC != nullptr)
    {
        SelectObject(fImageDC, nullptr);
        DeleteObject( fImageBitmap );
        DeleteDC( fImageDC );
    }

    fDblDC = fImageDC = nullptr;
    fDblBitmap = fImageBitmap = nullptr;
}

void    pfGUISkinEditProc::ISetScrollRanges()
{
    SCROLLINFO  info;


    int visW = fPreviewRect.right - fPreviewRect.left;
    int visH = fPreviewRect.bottom - fPreviewRect.top;

    if( visW < fDblWidth )
    {
        if( fXOffset > fDblWidth - visW )
            fXOffset = fDblWidth - visW;
        else if( fXOffset < 0 )
            fXOffset = 0;

        info.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        info.nMin = 0;
        info.nMax = fDblWidth;// - visW;
        info.nPage = visW;
        info.nPos = fXOffset;
        info.cbSize = sizeof( info );

        SetScrollInfo( GetDlgItem( fHWnd, IDC_GUI_HORZSCROLL ), SB_CTL, &info, true );
        ShowWindow( GetDlgItem( fHWnd, IDC_GUI_HORZSCROLL ), true );
    }
    else
    {
        ShowWindow( GetDlgItem( fHWnd, IDC_GUI_HORZSCROLL ), false );
        fXOffset = 0;
    }

    if( visH < fDblHeight )
    {
        if( fYOffset > fDblHeight - visH )
            fYOffset = fDblHeight - visH;
        else if( fYOffset < 0 )
            fYOffset = 0;

        info.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        info.nMin = 0;
        info.nMax = fDblHeight;// - visH;
        info.nPage = visH;
        info.nPos = fYOffset;
        info.cbSize = sizeof( info );

        SetScrollInfo( GetDlgItem( fHWnd, IDC_GUI_VERTSCROLL ), SB_CTL, &info, true );
        ShowWindow( GetDlgItem( fHWnd, IDC_GUI_VERTSCROLL ), true );
    }
    else
    {
        ShowWindow( GetDlgItem( fHWnd, IDC_GUI_VERTSCROLL ), false );
        fYOffset = 0;
    }
}

bool    pfGUISkinEditProc::IPointWithinRange( int x, int y, int ptX, int ptY )
{
    if( x > ptX - kRangeSlop && x < ptX + kRangeSlop &&
        y > ptY - kRangeSlop && y < ptY + kRangeSlop )
        return true;
    return false;
}

bool    pfGUISkinEditProc::IPointWithinVertRange( int x, int y, int ptX, int ptY1, int ptY2 )
{
    if( x > ptX - kRangeSlop && x < ptX + kRangeSlop &&
        y > ptY1 - kRangeSlop && y < ptY2 + kRangeSlop )
        return true;
    return false;
}

bool    pfGUISkinEditProc::IPointWithinHorzRange( int x, int y, int ptX1, int ptX2, int ptY )
{
    if( x > ptX1 - kRangeSlop && x < ptX2 + kRangeSlop &&
        y > ptY - kRangeSlop && y < ptY + kRangeSlop )
        return true;
    return false;
}

uint8_t   pfGUISkinEditProc::IGetDragTypeFlags( int x, int y )
{
    // Corners
    if( IPointWithinRange( x, y, fCurrElemRect.left, fCurrElemRect.top ) )
        return kLeft | kTop;
    if( IPointWithinRange( x, y, fCurrElemRect.right, fCurrElemRect.top ) )
        return kRight | kTop;
    if( IPointWithinRange( x, y, fCurrElemRect.right, fCurrElemRect.bottom ) )
        return kRight | kBottom;
    if( IPointWithinRange( x, y, fCurrElemRect.left, fCurrElemRect.bottom ) )
        return kLeft | kBottom;

    // Edges
    if( IPointWithinVertRange( x, y, fCurrElemRect.left, fCurrElemRect.top, fCurrElemRect.bottom ) )
        return kLeft;
    if( IPointWithinVertRange( x, y, fCurrElemRect.right, fCurrElemRect.top, fCurrElemRect.bottom ) )
        return kRight;

    if( IPointWithinHorzRange( x, y, fCurrElemRect.left, fCurrElemRect.right, fCurrElemRect.top ) )
        return kTop;
    if( IPointWithinHorzRange( x, y, fCurrElemRect.left, fCurrElemRect.right, fCurrElemRect.bottom ) )
        return kBottom;

    // The middle
    if( x >= fCurrElemRect.left && x <= fCurrElemRect.right && y >= fCurrElemRect.top && y <= fCurrElemRect.bottom )
        return kDragAll;

    return 0;
}

INT_PTR CALLBACK    pfGUISkinEditProc::DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return fInstance->DialogProc( hDlg, msg, wParam, lParam );
}

INT_PTR CALLBACK    pfGUISkinEditProc::DialogProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT     pInfo;
    RECT            r;
    HDC             hDC;
    int             maxDim, i, j, x, y;
    bool            timerActive = false;


    static struct plElemPair
    {
        pfGUISkin::Elements el;
        const char          *name;
    } sElemPairs[] = { { pfGUISkin::kUpLeftCorner,          "Upper-left Corner" },
                       { pfGUISkin::kTopSpan,               "Top Span" },
                       { pfGUISkin::kUpRightCorner,         "Upper-right Corner" },
                       { pfGUISkin::kRightSpan,             "Right Span" },
                       { pfGUISkin::kLowerRightCorner,      "Lower-right Corner" },
                       { pfGUISkin::kBottomSpan,            "Bottom Span" },
                       { pfGUISkin::kLowerLeftCorner,       "Lower-left Corner" },
                       { pfGUISkin::kLeftSpan,              "Left Span" },
                       { pfGUISkin::kMiddleFill,            "Middle Fill" },
                       { pfGUISkin::kSelectedFill,          "Selected Middle Fill" },
                       { pfGUISkin::kSubMenuArrow,          "Sub-Menu Arrow" },
                       { pfGUISkin::kSelectedSubMenuArrow,  "Selected Sub-Menu Arrow" },
                       { pfGUISkin::kTreeButtonClosed,      "Tree-view Button, Closed" },
                       { pfGUISkin::kTreeButtonOpen,        "Tree-view Button, Open" },
                       { pfGUISkin::kNumElements, nullptr } };


    fHWnd = hDlg;

    switch( msg )
    {
        case WM_INITDIALOG:

            // Get preview rect
            GetClientRect( GetDlgItem( hDlg, IDC_GUI_PREVIEW ), &fPreviewRect );
            MapWindowPoints( GetDlgItem( hDlg, IDC_GUI_PREVIEW ), hDlg, (POINT *)&fPreviewRect, 2 );

            SendDlgItemMessage( hDlg, IDC_GUI_ZIN, BM_SETIMAGE, (WPARAM)IMAGE_ICON, 
                                        (LPARAM)LoadImage( hInstance, MAKEINTRESOURCE( IDI_ZOOMIN ), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR ) );
            SendDlgItemMessage( hDlg, IDC_GUI_ZOUT, BM_SETIMAGE, (WPARAM)IMAGE_ICON, 
                                        (LPARAM)LoadImage( hInstance, MAKEINTRESOURCE( IDI_ZOOMOUT ), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR ) );

            // Fill element list
            SendDlgItemMessage( hDlg, IDC_GUI_ELEMENTS, LB_RESETCONTENT, 0, 0 );
            for( i = 0; sElemPairs[ i ].el != pfGUISkin::kNumElements; i++ )
            {
                int idx = (int)SendDlgItemMessage(hDlg, IDC_GUI_ELEMENTS, LB_ADDSTRING, 0, (LPARAM)sElemPairs[i].name);
                SendDlgItemMessage( hDlg, IDC_GUI_ELEMENTS, LB_SETITEMDATA, (WPARAM)idx, (LPARAM)sElemPairs[ i ].el );
                if( sElemPairs[ i ].el == pfGUISkin::kUpLeftCorner )
                    j = idx;
            }
            SendDlgItemMessage( hDlg, IDC_GUI_ELEMENTS, LB_SETCURSEL, j, 0 );

            fOrigCursor = LoadCursor(nullptr, IDC_ARROW);//GetCursor();

            break;

        case WM_COMMAND:
            if( LOWORD( wParam ) == IDCANCEL )
            {
                // Since we've been editing the PB directly, we have to now restore them
                // to their original values
                IParamBlock2 *pb = fComp->GetParamBlockByID( plComponent::kBlkComp );
                for( int i = 0; i < pfGUISkin::kNumElements; i++ )
                {
                    int id = i * 4 + plGUISkinComp::kRefUpLeftCorner;
                    pb->SetValue( id + 0, 0, (int)fBackups[ i ].fX );
                    pb->SetValue( id + 1, 0, (int)fBackups[ i ].fY ); 
                    pb->SetValue( id + 2, 0, (int)fBackups[ i ].fWidth );
                    pb->SetValue( id + 3, 0, (int)fBackups[ i ].fHeight );
                }
                EndDialog( hDlg, 1 );
            }
            else if( LOWORD( wParam ) == IDOK )
                EndDialog( hDlg, 0 );
            else if( LOWORD( wParam ) == IDC_GUI_ZIN )
            {
                fXOffset /= fZoom;  fYOffset /= fZoom;
                fZoom++;
                fXOffset *= fZoom;  fYOffset *= fZoom;

                IKillDblBuffer();
                IRefreshImageBuffer();
                InvalidateRect( hDlg, &fPreviewRect, false );
            }
            else if( LOWORD( wParam ) == IDC_GUI_ZOUT )
            {
                if( fZoom > 1 )
                {
                    fXOffset /= fZoom;  fYOffset /= fZoom;
                    fZoom--;
                    fXOffset *= fZoom;  fYOffset *= fZoom;

                    IKillDblBuffer();
                    IRefreshImageBuffer();
                    InvalidateRect( hDlg, &fPreviewRect, false );
                }
            }
            else if( LOWORD( wParam ) == IDC_GUI_ELEMENTS )
            {
                int idx = (int)SendDlgItemMessage(hDlg, IDC_GUI_ELEMENTS, LB_GETCURSEL, 0, 0);
                fCurrPBRefSet = (int)SendDlgItemMessage(hDlg, IDC_GUI_ELEMENTS, LB_GETITEMDATA, (WPARAM)idx, 0) * 4 + plGUISkinComp::kRefUpLeftCorner;

                IRefreshDblBuffer();
                InvalidateRect( hDlg, &fPreviewRect, false );
            }
            return true;

        case WM_CLOSE:
            EndDialog( hDlg, 0 );
            return true;

        case WM_HSCROLL:
            OffsetRect( &fCurrElemRect, fXOffset, fYOffset );
            switch( LOWORD( wParam ) )
            {
                case SB_PAGEUP:     fXOffset -= 300; break;
                case SB_PAGEDOWN:   fXOffset += 300; break;
                case SB_LINEUP:     fXOffset -= 16; break;
                case SB_LINEDOWN:   fXOffset += 16; break;
                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                    fXOffset = HIWORD( wParam );
                    break;
            }
            maxDim = fDblWidth - ( fPreviewRect.right - fPreviewRect.left );
            if( fXOffset < 0 )
                fXOffset = 0;
            else if( fXOffset > maxDim )
                fXOffset = maxDim;
            SetScrollPos( GetDlgItem( hDlg, IDC_GUI_HORZSCROLL ), SB_CTL, fXOffset, true );

            OffsetRect( &fCurrElemRect, -fXOffset, -fYOffset );
            InvalidateRect( hDlg, &fPreviewRect, false );
            break;

        case WM_VSCROLL:
            OffsetRect( &fCurrElemRect, fXOffset, fYOffset );
            switch( LOWORD( wParam ) )
            {
                case SB_PAGEUP:     fYOffset -= 300; break;
                case SB_PAGEDOWN:   fYOffset += 300; break;
                case SB_LINEUP:     fYOffset -= 16; break;
                case SB_LINEDOWN:   fYOffset += 16; break;
                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                    fYOffset = HIWORD( wParam );
                    break;
            }
            maxDim = fDblHeight - ( fPreviewRect.bottom - fPreviewRect.top );
            if( fYOffset < 0 )
                fYOffset = 0;
            else if( fYOffset > maxDim )
                fYOffset = maxDim;
            SetScrollPos( GetDlgItem( hDlg, IDC_GUI_VERTSCROLL ), SB_CTL, fYOffset, true );

            OffsetRect( &fCurrElemRect, -fXOffset, -fYOffset );
            InvalidateRect( hDlg, &fPreviewRect, false );
            break;

        case WM_PAINT:
            {
                BeginPaint( hDlg, &pInfo );
                hDC = (HDC)pInfo.hdc;

                if (fDblDC == nullptr)
                    IInitDblBuffer();

                int width = fDblWidth;
                int height = fDblHeight;
                if( width > fPreviewRect.right - fPreviewRect.left )
                    width = fPreviewRect.right - fPreviewRect.left;
                if( height > fPreviewRect.bottom - fPreviewRect.top )
                    height = fPreviewRect.bottom - fPreviewRect.top;

                BitBlt( hDC, fPreviewRect.left, fPreviewRect.top, width, height, fDblDC, fXOffset, fYOffset, SRCCOPY );

                r = fPreviewRect;
                r.left += width;
                FillRect( hDC, &r, ColorMan()->GetBrush( kBackground ) );

                r = fPreviewRect;
                r.top += height;
                FillRect( hDC, &r, ColorMan()->GetBrush( kBackground ) );

                EndPaint( hDlg, &pInfo );
            }
            break;

        case WM_LBUTTONDOWN:
            SetCapture( hDlg );
            fDragging = true;
            fDragType = IGetDragTypeFlags( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
            if( fDragType == kDragAll )
            {
                fDragOffsetX = fCurrElemRect.left - GET_X_LPARAM( lParam );
                fDragOffsetY = fCurrElemRect.top - GET_Y_LPARAM( lParam );
            }
            else if( fDragType == 0 )
            {
                fDragOffsetX = GET_X_LPARAM( lParam ) + fXOffset;
                fDragOffsetY = GET_Y_LPARAM( lParam ) + fYOffset;
            }
            else
                fDragOffsetX = fDragOffsetY = 0;

            break;

        case WM_LBUTTONUP:
            ReleaseCapture();
            fDragging = false;
            break;

        case WM_TIMER:
            // We do the same processing as MOUSEMOVE, but we need to make sure we have the right
            // mouse position first
            {
                POINT pt;
                GetCursorPos( &pt );
                MapWindowPoints(nullptr, hDlg, &pt, 1);
                lParam = MAKELPARAM( pt.x, pt.y );
            }
            // Fall thru...

        case WM_MOUSEMOVE:
            x = GET_X_LPARAM( lParam );
            y = GET_Y_LPARAM( lParam );

            if( fDragging )
            {
                if( fDragType == 0 )
                {
                    fXOffset = fDragOffsetX - x;
                    fYOffset = fDragOffsetY - y;
                    ISetScrollRanges(); // Will clamp offset \for us
                }
                else
                {
                    // Translate x and y into bitmap space
                    POINT pt;
                    pt.x = x;
                    pt.y = y;

                    if( PtInRect( &fPreviewRect, pt ) )
                    {
                        MapWindowPoints( hDlg, GetDlgItem( hDlg, IDC_GUI_PREVIEW ), &pt, 1 );

                        pt.x += fDragOffsetX;
                        pt.y += fDragOffsetY;

                        // Note the + 1/2 zoom so it's the closest pixel by center, not by area
                        x = ( pt.x + fXOffset + ( fZoom >> 1 ) ) / fZoom;
                        y = ( pt.y + fYOffset + ( fZoom >> 1 ) ) / fZoom;

                        // Set depending on our current drag flags
                        // Note the logic here: if we drag left, we want width and left changing,
                        // if we drag both, just left, if we drag right, just width
                        IParamBlock2 *pb = fComp->GetParamBlockByID( plComponent::kBlkComp );
                        if( fDragType & kLeft )
                        {
                            if( fDragType & kRight )
                                pb->SetValue( fCurrPBRefSet + 0, 0, (int)x );
                            else 
                            {
                                int old = pb->GetInt( fCurrPBRefSet + 0 ) + pb->GetInt( fCurrPBRefSet + 2 );

                                pb->SetValue( fCurrPBRefSet + 0, 0, (int)x );
                                pb->SetValue( fCurrPBRefSet + 2, 0, (int)old - x );
                            }
                        }
                        else if( fDragType & kRight )
                            pb->SetValue( fCurrPBRefSet + 2, 0, (int)x - pb->GetInt( fCurrPBRefSet + 0 ) );

                        if( fDragType & kTop )
                        {
                            if( fDragType & kBottom )
                                pb->SetValue( fCurrPBRefSet + 1, 0, (int)y );
                            else 
                            {
                                int old = pb->GetInt( fCurrPBRefSet + 1 ) + pb->GetInt( fCurrPBRefSet + 3 );

                                pb->SetValue( fCurrPBRefSet + 1, 0, (int)y );
                                pb->SetValue( fCurrPBRefSet + 3, 0, (int)old - y );
                            }
                        }
                        else if( fDragType & kBottom )
                            pb->SetValue( fCurrPBRefSet + 3, 0, (int)y - pb->GetInt( fCurrPBRefSet + 1 ) );

                        // Clamp width and height
                        if( pb->GetInt( fCurrPBRefSet + 2 ) < 0 )
                            pb->SetValue( fCurrPBRefSet + 2, 0, (int)0 );
                        if( pb->GetInt( fCurrPBRefSet + 3 ) < 0 )
                            pb->SetValue( fCurrPBRefSet + 3, 0, (int)0 );

                        // Clamp X and Y
                        if( pb->GetInt( fCurrPBRefSet + 0 ) < 0 )
                            pb->SetValue( fCurrPBRefSet + 0, 0, (int)0 );
                        else if( pb->GetInt( fCurrPBRefSet + 0 ) + pb->GetInt( fCurrPBRefSet + 2 ) > fDblWidth / fZoom )
                            pb->SetValue( fCurrPBRefSet + 0, 0, (int)fDblWidth / fZoom - pb->GetInt( fCurrPBRefSet + 2 ) );

                        if( pb->GetInt( fCurrPBRefSet + 1 ) < 0 )
                            pb->SetValue( fCurrPBRefSet + 1, 0, (int)0 );
                        else if( pb->GetInt( fCurrPBRefSet + 1 ) + pb->GetInt( fCurrPBRefSet + 3 ) > fDblHeight / fZoom )
                            pb->SetValue( fCurrPBRefSet + 1, 0, (int)fDblHeight / fZoom - pb->GetInt( fCurrPBRefSet + 3 ) );
                    }
                    else
                    {
                        // Mouse is outside our preview, so scroll if possible
                        int dX = ( x < fPreviewRect.left ) ? x - fPreviewRect.left : ( x > fPreviewRect.right ) ? x - fPreviewRect.right : 0;
                        int dY = ( y < fPreviewRect.top ) ? y - fPreviewRect.top : ( y > fPreviewRect.bottom ) ? y - fPreviewRect.bottom : 0;

                        fXOffset += dX;
                        fYOffset += dY;         
                        OffsetRect( &fCurrElemRect, -dX, -dY );

                        ISetScrollRanges(); // Will clamp origin for us

                        // Don't actually drag our bounds, 'cause we're scrolling

                        // Note: since we only get MOUSEMOVE when, gee, the mouse moves, if we've scrolled over, it'll only
                        // do it once and then wait for the mouse to nudge again. We'd rather it keep going until the user
                        // moves the mouse again, so we create a timer that calls us back in n somethingths so we can check again
                        if( fDragTimer == 0 )
                            fDragTimer = SetTimer(hDlg, 0, 200, nullptr);
                        timerActive = true;     // So we don't kill it at the end here...
                    }
                }

                IRefreshDblBuffer();
                InvalidateRect( hDlg, &fPreviewRect, false );
            }
            else
            {
                uint8_t dragType = IGetDragTypeFlags( x, y );
                HCURSOR cursor;
                switch( dragType )
                {
                    case kLeft | kTop:      
                    case kRight | kBottom:
                        cursor = LoadCursor(nullptr, IDC_SIZENWSE);
                        break;
                    case kLeft | kBottom:       
                    case kRight | kTop:
                        cursor = LoadCursor(nullptr, IDC_SIZENESW);
                        break;
                    case kLeft:     
                    case kRight:
                        cursor = LoadCursor(nullptr, IDC_SIZEWE);
                        break;
                    case kTop:      
                    case kBottom:
                        cursor = LoadCursor(nullptr, IDC_SIZENS);
                        break;
                    case kLeft | kTop | kRight | kBottom:       
                        cursor = LoadCursor(nullptr, IDC_SIZEALL);
                        break;
                    default:
                        {
                            POINT pt;
                            pt.x = x;
                            pt.y = y;
                            if( PtInRect( &fPreviewRect, pt ) )
                                cursor = LoadCursor(nullptr, IDC_HAND);
                            else
                                cursor = fOrigCursor;
                        }
                        break;
                }
                
                SetCursor( cursor );

                if( !timerActive )
                {
                    // No longer need our trick timer, so kill it
                    KillTimer( hDlg, fDragTimer );
                    fDragTimer = 0;
                }
            }
            break;
    }

    return false;
}

plGUISkinComp   *plGUISkinComp::GetGUIComp( INode *node )
{
    if (node == nullptr)
        return nullptr;

    plComponentBase *base = ( ( plMaxNodeBase *)node )->ConvertToComponent();
    if (base == nullptr)
        return nullptr;

    if( base->ClassID() == GUI_SKIN_CLASSID )
        return (plGUISkinComp *)base;

    return nullptr;
}

