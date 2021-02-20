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

#include "plResTreeView.h"
#include "res/ui_FindDialog.h"

#include "HeadSpin.h"

#include "pnFactory/plFactory.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plKeyImp.h"
#include "pnKeyedObject/plUoid.h"

#include "plResMgr/plResManager.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plRegistryNode.h"
#include "plResMgr/plPageInfo.h"

#include <QDialog>
#include <QLayout>
#include <QMessageBox>

struct plKeyInfo
{
    plKey               fKey;
    plRegistryPageNode *fPage;
};

plResTreeViewItem::~plResTreeViewItem()
{
    delete fData;
}

plKey plResTreeViewItem::GetKey() const
{
    return fData ? fData->fKey : nullptr;
}

plRegistryPageNode *plResTreeViewItem::GetPage() const
{
    return fData ? fData->fPage : nullptr;
}


// How's this for functionality?
class plResDlgLoader : public plRegistryPageIterator, public plRegistryKeyIterator
{
protected:
    plResTreeView      *fTree;
    plResTreeViewItem  *fCurrItem;
    plResTreeViewItem  *fCurrTypeItem;
    uint16_t            fCurrType;
    bool                fFilter;

    plRegistryPageNode *fCurrPage;

public:
    plResDlgLoader(plResTreeView *tree, bool filter)
        : fFilter(filter), fTree(tree), fCurrItem(), fCurrTypeItem(),
          fCurrType(), fCurrPage()
    {
        static_cast<plResManager *>(hsgResMgr::ResMgr())->IterateAllPages(this);
    }

    bool EatPage(plRegistryPageNode *page) override
    {
        fCurrPage = page;
        const plPageInfo &info = page->GetPageInfo();
        QString name = QString("%1->%2").arg(info.GetAge().c_str()).arg(info.GetPage().c_str());
        fCurrItem = new plResTreeViewItem(fTree, name, new plKeyInfo { nullptr, fCurrPage });

        fCurrType = static_cast<uint16_t>(-1);
        page->LoadKeys();
        page->IterateKeys(this);
        return true;
    }

    bool EatKey(const plKey &key) override
    {
        if (fCurrType != key->GetUoid().GetClassType())
        {
            fCurrType = key->GetUoid().GetClassType();
            const char *className = plFactory::GetNameOfClass(fCurrType);
            fCurrTypeItem = new plResTreeViewItem(fCurrItem, className ? className : "<unknown>", nullptr);
        }

        if (!fFilter) {
            new plResTreeViewItem(fCurrTypeItem, key->GetUoid().GetObjectName().c_str(),
                                  new plKeyInfo { key, fCurrPage });
        }
        return true;
    }
};

void plResTreeView::LoadFromRegistry(bool filter)
{
    plResDlgLoader loader(this, filter);
    sortItems(0, Qt::AscendingOrder);
}

static QTreeWidgetItem *IGetNextTreeItem(QTreeWidgetItem *item)
{
    if (!item)
        return nullptr;

    QTreeWidgetItemIterator iter(item);
    return *(++iter);
}

void plResTreeView::FindObject()
{
    QDialog findDialog(this);
    Ui_FindDialog ui;
    ui.setupUi(&findDialog);

    findDialog.layout()->setSizeConstraint(QLayout::SetFixedSize);

    if (findDialog.exec() == QDialog::Accepted)
    {
        fFoundItem = invisibleRootItem();
        fSearchString = ui.fObjectName->text().toUtf8().constData();
        IFindNextObject();
    }
}

void plResTreeView::FindNextObject()
{
    if (!fFoundItem)
        FindObject();
    else
    {
        fFoundItem = IGetNextTreeItem(fFoundItem);
        IFindNextObject();
    }
}

void plResTreeView::IFindNextObject()
{
    while (fFoundItem)
    {
        // Get the item
        plKey key = fFoundItem->type() == plResTreeViewItem::Type
                  ? static_cast<plResTreeViewItem *>(fFoundItem)->GetKey()
                  : nullptr;
        if (key && key->GetUoid().GetObjectName().contains(fSearchString, ST::case_insensitive))
        {
            /// FOUND
            setCurrentItem(fFoundItem);
            return;
        }

        // Keep searching. First try child items of this one
        fFoundItem = IGetNextTreeItem(fFoundItem);
    }

    QMessageBox::critical(this, tr("Find Objects"), tr("No objects found"));
}
