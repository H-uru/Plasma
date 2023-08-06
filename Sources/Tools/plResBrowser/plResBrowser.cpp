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

#include "plResBrowser.h"
#include "res/ui_ResBrowser.h"

#if HS_BUILD_FOR_WIN32
#   include "plWinRegistryTools.h"
#endif

#include "pnAllCreatables.h"

#include "pnFactory/plFactory.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plKeyImp.h"
#include "pnKeyedObject/plUoid.h"

#include "plResMgr/plPageInfo.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plRegistryNode.h"
#include "plResMgr/plResMgrCreatable.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plResMgrSettings.h"
#include "plMessage/plResMgrHelperMsg.h"

#include <functional>
#include <QApplication>
#include <QDialog>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>

REGISTER_CREATABLE(plResMgrHelperMsg);

static void IAboutDialog(QWidget *parent)
{
    QDialog dlg(parent);
    QLabel *image = new QLabel(&dlg);
    image->setPixmap(QPixmap(":/icon1.ico"));
    QLabel *text = new QLabel(QObject::tr(R"(plResBrowser
A simple Plasma 2.0 packfile browsing utility
Copyright (C) 2002 Cyan Worlds, Inc.

Who needs log files?)"), &dlg);
    QPushButton *ok = new QPushButton(QObject::tr("OK"), &dlg);
    ok->setDefault(true);

    QHBoxLayout *layout = new QHBoxLayout(&dlg);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(10);
    layout->addWidget(image);
    layout->addWidget(text);
    layout->addWidget(ok);

    dlg.connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
    dlg.exec();
}

plResBrowser::plResBrowser()
    : QMainWindow()
{
    fUI = new Ui_ResBrowser;
    fUI->setupUi(this);

    setAcceptDrops(true);

    // Use window background color for read-only text fields
    // Note that we don't set them disabled, so you can still copy their contents
    QPalette pal = fUI->fAgeName->palette();
    pal.setColor(QPalette::Text, fUI->label->palette().color(QPalette::Text));
    pal.setColor(QPalette::Base, fUI->label->palette().color(QPalette::Window));
    fUI->fAgeName->setPalette(pal);
    fUI->fPageName->setPalette(pal);
    fUI->fLocation->setPalette(pal);
    fUI->fDataVersion->setPalette(pal);
    fUI->fChecksum->setPalette(pal);
    fUI->fChecksumType->setPalette(pal);
    fUI->fObjectName->setPalette(pal);
    fUI->fObjectClass->setPalette(pal);
    fUI->fStartPos->setPalette(pal);
    fUI->fObjectSize->setPalette(pal);

    connect(fUI->fOpenAction, SIGNAL(triggered()), SLOT(OpenFile()));
    connect(fUI->fOpenDirectoryAction, SIGNAL(triggered()), SLOT(OpenDirectory()));
    connect(fUI->fSaveSelectedAction, SIGNAL(triggered()), SLOT(SaveSelectedObject()));
    connect(fUI->fFindAction, SIGNAL(triggered()), fUI->fTreeView, SLOT(FindObject()));
    connect(fUI->fFindNextAction, SIGNAL(triggered()), fUI->fTreeView, SLOT(FindNextObject()));
    connect(fUI->fShowOnlyLoadableAction, SIGNAL(triggered()), SLOT(RefreshTree()));
    connect(fUI->fExitAction, SIGNAL(triggered()), SLOT(close()));
    connect(fUI->fAboutAction, &QAction::triggered, std::bind(&IAboutDialog, this));

    connect(fUI->fTreeView, &QTreeWidget::currentItemChanged,
            std::bind(&plResBrowser::UpdateInfoPage, this));
    connect(fUI->fHexValues, SIGNAL(clicked()), SLOT(UpdateInfoPage()));
    connect(fUI->fFindButton, SIGNAL(clicked()), fUI->fTreeView, SLOT(FindObject()));
    connect(fUI->fFindNextButton, SIGNAL(clicked()), fUI->fTreeView, SLOT(FindNextObject()));
    connect(fUI->fSaveButton, SIGNAL(clicked()), SLOT(SaveSelectedObject()));

    RegisterFileTypes();
    plResMgrSettings::Get().SetLoadPagesOnInit(false);

    QStringList args = qApp->arguments();
    if (args.size() > 1)
    {
        plFileName path = args[1].toUtf8().constData();
        if (path.GetFileExt() == "prp")
            LoadPrpFile(args[1]);
    }
}

plResBrowser::~plResBrowser()
{
    delete fUI;
}

void plResBrowser::SetWindowTitle(const QString &title)
{
    setWindowTitle(QString("plResBrowser%1").arg(title.isEmpty() ? "" : " - " + title));
}

void plResBrowser::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void plResBrowser::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.size() == 0)
        return;

    fUI->fTreeView->clear();
    hsgResMgr::Reset();
    plResManager *mgr = static_cast<plResManager *>(hsgResMgr::ResMgr());

    QString path = urls[0].toLocalFile();

    if (urls.size() == 1 && urls[0].isLocalFile() && path.lastIndexOf('.') == -1)
    {
        // Must be a directory
        std::vector<plFileName> prpFiles = plFileSystem::ListDir(path.toUtf8().constData(), "*.prp");
        for (auto iter = prpFiles.begin(); iter != prpFiles.end(); ++iter)
            mgr->AddSinglePage(*iter);
    }
    else
    {
        for (const QUrl &url : urls)
        {
            if (!url.isLocalFile())
                continue;

            plFileName fileName = url.toLocalFile().toUtf8().constData();
            if (fileName.GetFileExt() == "prp")
            {
                mgr->AddSinglePage(fileName);
                path = fileName.StripFileName().AsString().c_str();
            }
        }
    }

    fUI->fTreeView->LoadFromRegistry(fUI->fShowOnlyLoadableAction->isChecked());
    SetWindowTitle(path);
}

void plResBrowser::OpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a file to browse"),
                            QString(), "Plasma 2 Pack Files (*.prp);;All files (*.*)");

    if (!fileName.isEmpty())
        LoadPrpFile(fileName);
}

void plResBrowser::OpenDirectory()
{
    QString path = QFileDialog::getExistingDirectory(this,
                tr("Select a Plasma 2 Data Directory:"),
                QDir::current().absolutePath(),
                QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);

    if (!path.isEmpty())
        LoadResourcePath(path);
}

void plResBrowser::SaveSelectedObject()
{
    plResTreeViewItem *item = static_cast<plResTreeViewItem *>(fUI->fTreeView->currentItem());
    plKey itemKey = item ? item->GetKey() : nullptr;
    if (!itemKey)
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export object"),
                            QString("%1.bin").arg(itemKey->GetName().c_str()),
                            "Binary Files (*.bin);;All files (*.*)");

    if (!fileName.isEmpty())
    {
        plKeyImp* keyImp = plKeyImp::GetFromKey(itemKey);

        if (keyImp->GetDataLen() <= 0)
            return;

        plResManager *resMgr = static_cast<plResManager *>(hsgResMgr::ResMgr());
        plRegistryPageNode* pageNode = resMgr->FindPage(itemKey->GetUoid().GetLocation());

        hsStream *stream = pageNode->OpenStream();
        if (!stream)
            return;

        uint8_t *buffer = new uint8_t[keyImp->GetDataLen()];
        if (buffer)
        {
            stream->SetPosition(keyImp->GetStartPos());
            stream->Read(keyImp->GetDataLen(), buffer);
        }

        if (!buffer)
            return;

        hsUNIXStream outStream;
        outStream.Open(fileName.toUtf8().constData(), "wb");
        outStream.Write(keyImp->GetDataLen(), buffer);
        outStream.Close();

        delete[] buffer;
    }
}

void plResBrowser::RefreshTree()
{
    fUI->fTreeView->clear();
    fUI->fTreeView->LoadFromRegistry(fUI->fShowOnlyLoadableAction->isChecked());
}

void plResBrowser::UpdateInfoPage()
{
    const bool showAsHex = fUI->fHexValues->isChecked();

    fUI->fObjectName->setText("");
    fUI->fObjectClass->setText("");
    fUI->fObjectSize->setText("");
    fUI->fStartPos->setText("");
    fUI->fSaveSelectedAction->setEnabled(false);
    fUI->fSaveButton->setEnabled(false);

    plResTreeViewItem *item = static_cast<plResTreeViewItem *>(fUI->fTreeView->currentItem());
    if (item)
    {
        if (item->GetPage())
        {
            const plPageInfo &pageInfo = item->GetPage()->GetPageInfo();
            fUI->fAgeName->setText(pageInfo.GetAge().c_str());
            fUI->fPageName->setText(pageInfo.GetPage().c_str());
            fUI->fLocation->setText(pageInfo.GetLocation().StringIze().c_str());

            fUI->fReserved->setChecked((pageInfo.GetLocation().GetFlags() & plLocation::kReserved) != 0);
            fUI->fBuiltIn->setChecked((pageInfo.GetLocation().GetFlags() & plLocation::kBuiltIn) != 0);
            fUI->fVolatile->setChecked((pageInfo.GetLocation().GetFlags() & plLocation::kVolatile) != 0);
            fUI->fLocalOnly->setChecked((pageInfo.GetLocation().GetFlags() & plLocation::kLocalOnly) != 0);

            fUI->fDataVersion->setText(QString::number(pageInfo.GetMajorVersion()));

            if (showAsHex)
                fUI->fChecksum->setText(QString("0x%1").arg(pageInfo.GetChecksum(), 0, 16));
            else
                fUI->fChecksum->setText(QString::number(pageInfo.GetChecksum()));

            fUI->fChecksumType->setText("Basic (file size)");
        }

        if (item->GetKey())
        {
            plKey key = item->GetKey();
            fUI->fObjectName->setText(key->GetUoid().GetObjectName().c_str());

            const char *cname = plFactory::GetNameOfClass(key->GetUoid().GetClassType());
            fUI->fObjectClass->setText(QString("%1 (%2)").arg(cname ? cname : "<unknown>")
                                       .arg(key->GetUoid().GetClassType()));

            plKeyImp* imp = plKeyImp::GetFromKey(key);
            if (showAsHex)
                fUI->fStartPos->setText(QString("0x%1").arg(imp->GetStartPos(), 0, 16));
            else
                fUI->fStartPos->setText(QString::number(imp->GetStartPos()));

            if (showAsHex)
            {
                fUI->fObjectSize->setText(QString("0x%1").arg(imp->GetDataLen(), 0, 16));
            }
            else
            {
                if (imp->GetDataLen() < 1024)
                    fUI->fObjectSize->setText(QString("%1 bytes").arg(imp->GetDataLen()));
                else if (imp->GetDataLen() < 1024 * 1024)
                    fUI->fObjectSize->setText(QString("%1 kB").arg(imp->GetDataLen() / 1024.f, 0, 'f', 2));
                else
                    fUI->fObjectSize->setText(QString("%1 MB").arg(imp->GetDataLen() / 1024.f / 1024.f, 0, 'f', 2));
            }

            fUI->fSaveSelectedAction->setEnabled(true);
            fUI->fSaveButton->setEnabled(true);
        }
    }
}

void plResBrowser::RegisterFileTypes()
{
    // Only meaningful on Windows currently...  For Linux, consider creating
    // a .desktop file as part of the make install process

#if HS_BUILD_FOR_WIN32
    static bool sFileTypesRegistered = false;
    if (sFileTypesRegistered)
        return;

    // Make sure our file types are created
    QString path = QCoreApplication::applicationFilePath();

    //plWinRegistryTools::AssociateFileType("PlasmaIdxFile", "Plasma 2 Index File", path, 1);
    //plWinRegistryTools::AssociateFileType("PlasmaDatFile", "Plasma 2 Data File", path, 2);
    //plWinRegistryTools::AssociateFileType("PlasmaPatchFile", "Plasma 2 Patch File", path, 3);
    plWinRegistryTools::AssociateFileType("PlasmaPackFile", "Plasma 2 Packfile", path, 4);

    // Check our file extensions
    QString prpAssoc = plWinRegistryTools::GetCurrentFileExtensionAssociation(".prp");

    if (prpAssoc != "PlasmaPackFile")
    {
        int retn = QMessageBox::question(this, tr("plResBrowser File Type Association"),
                        tr("The Plasma 2 packed data file extension .prp is not currently associated "
                           "with plResBrowser. Would you like to associate it now?"));
        if (retn == QMessageBox::Yes)
        {
            // Associate 'em
            plWinRegistryTools::AssociateFileExtension(".prp", "PlasmaPackFile");
        }
    }

    sFileTypesRegistered = true;
#endif
}

void plResBrowser::LoadPrpFile(const QString &fileName)
{
    fUI->fTreeView->clear();
    hsgResMgr::Reset();

    // Load that source
    plResManager *mgr = static_cast<plResManager *>(hsgResMgr::ResMgr());
    mgr->AddSinglePage(fileName.toUtf8().constData());
    fUI->fTreeView->LoadFromRegistry(fUI->fShowOnlyLoadableAction->isChecked());

    SetWindowTitle(fileName);
}

void plResBrowser::LoadResourcePath(const QString &path)
{
    fUI->fTreeView->clear();
    hsgResMgr::Reset();

    // Load that source
    plResManager *mgr = static_cast<plResManager *>(hsgResMgr::ResMgr());

    std::vector<plFileName> prpFiles = plFileSystem::ListDir(path.toUtf8().constData(), "*.prp");
    for (auto iter = prpFiles.begin(); iter != prpFiles.end(); ++iter)
        mgr->AddSinglePage(*iter);

    fUI->fTreeView->LoadFromRegistry(fUI->fShowOnlyLoadableAction->isChecked());

    SetWindowTitle(path);
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("plResBrowser");
    app.setWindowIcon(QIcon(":/icon1.ico"));

    plResMgrSettings::Get().SetFilterNewerPageVersions(false);
    plResMgrSettings::Get().SetFilterOlderPageVersions(false);

    plResManager *rMgr = new plResManager;
    hsgResMgr::Init(rMgr);

    int retn;
    {
        plResBrowser mainWindow;
        mainWindow.show();
        retn = app.exec();
    }

    hsgResMgr::Shutdown();

    return retn;
}
