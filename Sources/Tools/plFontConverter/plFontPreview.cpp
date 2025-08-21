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
#include "plFontPreview.h"

#include "plGImage/plFont.h"
#include "plGImage/plMipmap.h"

#include <memory>
#include <QPainter>
#include <QPicture>

void plFontPreview::Update(plFont *font, const QString &text)
{
    fFont = font;
    fText = text;
    fPreview = QImage(width(), height(), QImage::Format_ARGB32);

    if (fFont == nullptr) {
        QPainter p(&fPreview);
        p.fillRect(0, 0, width(), height(), Qt::white);

        update();
        return;
    }

    ST::string testString = text.toUtf8().constData();

    // Create us a mipmap to render onto, render onto it, then copy that to our DC
    plMipmap *mip = new plMipmap(width(), height(), plMipmap::kARGB32Config, 1);
    memset(mip->GetImage(), 0xff, mip->GetWidth() * mip->GetHeight() * 4);

    fFont->SetRenderColor(0xff000000);
    fFont->SetRenderFlag(plFont::kRenderClip, true);
    fFont->SetRenderClipRect(0, 0, (int16_t)width(), (int16_t)height());
    uint16_t w, h, a, lastX, lastY;
    fFont->CalcStringExtents(testString, w, h, a, lastX, lastY);

    int cY = ((height() - h) >> 1) + a;

    if (cY < 0)
        cY = 0;
    else if (cY > height() - 1)
        cY = height() - 1;

    memset(mip->GetAddr32(8, cY), 0xc0, (width() - 8) * 4);

    fFont->RenderString(mip, 8, cY, testString);

    int x, y;
    for (y = 0; y < height(); y++) {
        for (x = 0; x < width(); x++) {
            uint32_t color = *mip->GetAddr32(x, y);

            // One pixel at a time?  Surely we can do better...
            // But for now this is a pure port
            fPreview.setPixel(x, y, color | 0xff000000);
        }
    }
    delete mip;

    update();
}

void plFontPreview::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.drawImage(0, 0, fPreview);
}
