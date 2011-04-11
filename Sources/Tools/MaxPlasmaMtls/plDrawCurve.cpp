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
#if 0
        int ybot, ytop, ylast, i, iy;
        HPEN linePen = (HPEN)GetStockObject(WHITE_PEN);
        HPEN fgPen = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNFACE));
        HPEN bgPen = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNSHADOW));
        int width = rect.w() - 4;
        int height = rect.h() - 4;
        int miplevel = 0;
        const float depth = 9;
        float detailDropoffStart = theHsMaxLayer->GetDetailDropoffStart(curTime) * depth;
        float detailDropoffStop = theHsMaxLayer->GetDetailDropoffStop(curTime) * depth;
        float detailMax = theHsMaxLayer->GetDetailMax(curTime);
        float detailMin = theHsMaxLayer->GetDetailMin(curTime);
        int nextmip = 1;

        ytop = rect.top + 2;
        ybot = ytop + height;
        ylast = -1;

        for (i=0; i < width; i++) {
            if (i==nextmip) {
                miplevel++;
                nextmip *= 2;
            }

            int ix = i + rect.left + 2;

            float alpha = (miplevel - detailDropoffStart) * (detailMin - detailMax) / (detailDropoffStop - detailDropoffStart) 
                + detailMax;

            if (alpha > detailMax)
                alpha = detailMax;

            if (alpha < detailMin)
                alpha = detailMin;

            iy = (int)(ybot - alpha * height);
            
            SelectPen(hdc, fgPen);
            VertLine(hdc, ix, ybot, iy);
            
            if (iy-1 > ytop) {
                // Fill in above curve
                SelectPen(hdc,bgPen);
                VertLine(hdc, ix, ytop, iy-1);
            }
            if (ylast>=0) {
                SelectPen(hdc,linePen);
                VertLine(hdc, ix-1, iy-1, ylast);
            }
            
            ylast = iy;
        }
        SelectObject( hdc, linePen );
        DeleteObject(fgPen);
        DeleteObject(bgPen);
        WhiteRect3D(hdc, rect, 1);
#endif