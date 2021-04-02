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

#include "plFontConverter.h"

#include "HeadSpin.h"
#include "hsWindows.h"

#include "pnAllCreatables.h"

#include "pnKeyedObject/plKeyImp.h"
#include "pnKeyedObject/plUoid.h"

#include "plGImage/plBitmap.h"
#include "plGImage/plFont.h"
#include "plGImage/plMipmap.h"
#include "plMessage/plResMgrHelperMsg.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plResMgrCreatable.h"
#include "plResMgr/plResMgrSettings.h"

#include <functional>
#include <QApplication>
#include <QDialog>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressDialog>


#include "plFontFreeType.h"
#include "res/ui_MainDialog.h"
#include "res/ui_FonChooser.h"
#include "res/ui_FreeType.h"
#include "res/ui_FreeTypeBatch.h"

// Dammit Qt
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#   define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#else
#   define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts
#endif

REGISTER_NONCREATABLE(plBitmap);
REGISTER_CREATABLE(plFont);
REGISTER_CREATABLE(plMipmap);
REGISTER_CREATABLE(plResMgrHelperMsg);

static void IAboutDialog(QWidget *parent)
{
    QDialog dlg(parent);
    QLabel *image = new QLabel(&dlg);
    image->setPixmap(QPixmap(":/icon1.ico"));
    QLabel *text = new QLabel(QObject::tr(R"(plFontConverter

A simple Plasma 2.0 utility for converting public
font formats into our own bitmap font format.)"), &dlg);
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

plFontConverter::plFontConverter()
    : QMainWindow(), fFont(nullptr)
{
    fUI = new Ui_MainDialog;
    fUI->setupUi(this);

    setAcceptDrops(true);

    connect(fUI->fImportFNTAction, SIGNAL(triggered()), SLOT(OpenFNT()));
    connect(fUI->fImportBDFAction, SIGNAL(triggered()), SLOT(OpenBDF()));
    connect(fUI->fImportFONAction, SIGNAL(triggered()), SLOT(OpenFON()));
    connect(fUI->fImportTTFAction, SIGNAL(triggered()), SLOT(OpenTTF()));
    connect(fUI->fOpenAction, SIGNAL(triggered()), SLOT(OpenP2F()));
    connect(fUI->fExportAction, SIGNAL(triggered()), SLOT(ExportP2F()));
    connect(fUI->fExitAction, SIGNAL(triggered()), SLOT(close()));
    connect(fUI->fAboutAction, &QAction::triggered, std::bind(&IAboutDialog, this));

    connect(fUI->fExportButton, SIGNAL(clicked()), SLOT(ExportP2F()));

    connect(fUI->fSampleText, &QLineEdit::textChanged, [this](const QString &text)
    {
        fUI->fPreview->Update(fFont, text);
    });
}

plFontConverter::~plFontConverter()
{
    delete fUI;
}

void plFontConverter::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void plFontConverter::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();

    for (const QUrl &url : urls)
    {
        if (!url.isLocalFile())
            continue;

        plFileName fileName = url.toLocalFile().toUtf8().constData();
        ST::string ext = fileName.GetFileExt();
        if (ext.compare_i("fnt") == 0)
            IImportFNT(fileName);
        else if (ext.compare_i("bdf") == 0)
            IImportBDF(fileName);
        else if (ext.compare_i("fon") == 0 || ext.compare_i("exe") == 0)
            IImportFON(fileName);
        else if (ext.compare_i("ttf") == 0 || ext.compare_i("ttc") == 0)
            IImportFreeType(fileName);
        else if (ext.compare_i("p2f") == 0)
            IOpenP2F(fileName);
        else
        {
            // Try using our freeType converter
            IImportFreeType(fileName);
        }
    }

    IUpdateInfo();
}

void plFontConverter::OpenFNT()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a FNT file to convert"),
                            QString(), "Windows FNT files (*.fnt);;All files (*.*)");

    if (!fileName.isEmpty())
        IImportFNT(fileName.toUtf8().constData());
}

void plFontConverter::OpenBDF()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a BDF file to convert"),
                            QString(), "Adobe BDF files (*.bdf);;All files (*.*)");

    if (!fileName.isEmpty())
        IImportBDF(fileName.toUtf8().constData());
}

void plFontConverter::OpenFON()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a FON file to convert"),
                            QString(), "Windows FON files (*.fon *.exe);;All files (*.*)");

    if (!fileName.isEmpty())
        IImportFON(fileName.toUtf8().constData());
}

void plFontConverter::OpenTTF()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a TrueType font to convert"),
                            QString(), "TrueType files (*.ttf *.ttc);;All files (*.*)");

    if (!fileName.isEmpty())
        IImportFreeType(fileName.toUtf8().constData());
}

void plFontConverter::OpenP2F()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a P2F file to open"),
                            QString(), "Plasma 2 font files (*.p2f);;All files (*.*)");

    if (!fileName.isEmpty())
        IOpenP2F(fileName.toUtf8().constData());
}

void plFontConverter::ExportP2F()
{
    // Grab updated values for the font
    ST::string fileName = fUI->fFaceName->text().toUtf8().constData();
    fFont->SetFace(fileName);
    fFont->SetSize(fUI->fFontSize->value());
    fFont->SetFlag(plFont::kFlagBold, fUI->fBold->isChecked());
    fFont->SetFlag(plFont::kFlagItalic, fUI->fItalic->isChecked());

    // Write out
    QString saveFile = QFileDialog::getSaveFileName(this, tr("Specify a file to export to"),
                            QString("%1-%2.p2f").arg(fFont->GetFace().c_str()).arg(fFont->GetSize()),
                            "Plasma 2 font files (*.p2f)");

    if (!saveFile.isEmpty())
    {
        fileName = saveFile.toUtf8().constData();

        hsUNIXStream stream;
        if (!stream.Open(fileName, "wb"))
            QMessageBox::critical(this, tr("Error"), tr("Can't open file for writing"));
        else
        {
            /*
            sprintf( fileName, "%s-%d", gFont->GetFace(), gFont->GetSize() );

            if (gFont->GetKey() == nullptr)
                hsgResMgr::ResMgr()->NewKey( fileName, gFont, plLocation::kGlobalFixedLoc );
            */
            fFont->WriteRaw(&stream);
            stream.Close();
        }
    }
}

void plFontConverter::IMakeFontGoAway()
{
    if (fFont != nullptr) {
        plKeyImp *imp = (plKeyImp *)(fFont->GetKey());
        if (imp != nullptr)
            imp->SetObjectPtr(nullptr);
        fFont = nullptr;
    }
}

void plFontConverter::IMakeNewFont()
{
    IMakeFontGoAway();
    fFont = new plFont();
}

void plFontConverter::IUpdateInfo()
{
    if (fFont != nullptr && fFont->GetNumChars() == 0)
        IMakeFontGoAway();

    if (fFont == nullptr)
    {
        fUI->fExportAction->setEnabled(false);
        fUI->fExportButton->setEnabled(false);

        fUI->fFaceName->setText("");
        fUI->fFontSize->setValue(0);
        fUI->fStartingGlyph->setText("");
        fUI->fGlyphCount->setText("");

        fUI->fBitmapWidth->setText("");
        fUI->fBitmapHeight->setText("");
        fUI->fBPP->setText("");

        fUI->fBold->setChecked(false);
        fUI->fItalic->setChecked(false);

        fUI->fPreview->Update(nullptr, QString());
        return;
    }

    fUI->fExportAction->setEnabled(true);
    fUI->fExportButton->setEnabled(true);

    fUI->fFaceName->setText(fFont->GetFace().c_str());
    fUI->fFontSize->setValue(fFont->GetSize());
    fUI->fStartingGlyph->setText(QString::number(fFont->GetFirstChar()));
    fUI->fGlyphCount->setText(QString::number(fFont->GetNumChars()));

    fUI->fBitmapWidth->setText(QString::number(fFont->GetBitmapWidth()));
    fUI->fBitmapHeight->setText(QString::number(fFont->GetBitmapHeight()));
    fUI->fBPP->setText(QString::number(fFont->GetBitmapBPP()));

    fUI->fBold->setChecked(fFont->IsFlagSet(plFont::kFlagBold));
    fUI->fItalic->setChecked(fFont->IsFlagSet(plFont::kFlagItalic));

    fUI->fPreview->Update(fFont, fUI->fSampleText->text());
}

void plFontConverter::IImportFNT(const plFileName &path)
{
    IMakeNewFont();
    if (!fFont->LoadFromFNT(path))
        QMessageBox::critical(this, tr("ERROR"), tr("Failure converting FNT file"));

    IUpdateInfo();
}

class ProgressRAII
{
    QWidget *fParent;
    QProgressDialog *fProgress;

public:
    ProgressRAII(QWidget *parent)
        : fParent(parent)
    {
        fProgress = new QProgressDialog(parent->tr("Importing Font..."), QString(), 0, 100, parent);
        fProgress->setAttribute(Qt::WA_DeleteOnClose);
        fParent->setEnabled(false);
    }

    ~ProgressRAII()
    {
        fProgress->close();
        fParent->setEnabled(true);
    }

    void SetRange(int max) { fProgress->setRange(0, max); }
    void SetValue(int val) { fProgress->setValue(val); }
};

class plMyBDFCallback : public plBDFConvertCallback
{
    ProgressRAII fProgress;
    uint16_t  fPoint;

public:
    plMyBDFCallback(QWidget *parent) : fProgress(parent), fPoint() { }

    void NumChars(uint16_t chars) override
    {
        fProgress.SetRange(chars);
    }

    void CharDone() override
    {
        fPoint++;
        fProgress.SetValue(fPoint);

        qApp->processEvents();
    }
};

void plFontConverter::IImportBDF(const plFileName &path)
{
    IMakeNewFont();

    plMyBDFCallback callback(this);
    if (!fFont->LoadFromBDF(path, &callback))
        QMessageBox::critical(this, tr("ERROR"), tr("Failure converting BDF file"));

    IUpdateInfo();
}

#if defined(Q_OS_WIN) && !defined(Q_OS_WIN64)
struct ResRecord
{
    HRSRC   fHandle;
    QString fName;

    ResRecord() : fHandle(nullptr) { }
    ResRecord(HRSRC h, const QString &n) : fHandle(h), fName(n) { }
};

BOOL CALLBACK ResEnumProc(HMODULE module, LPCTSTR type, LPTSTR name, LONG_PTR lParam)
{
    HRSRC res = FindResource(module, name, type);
    if (res != nullptr)
    {
        QList<ResRecord> *array = reinterpret_cast<QList<ResRecord> *>(lParam);
        array->append(ResRecord(res, name));
    }

    return true;
}

static ResRecord DoSelectResource(const QList<ResRecord> &resources)
{
    QDialog dlg;
    Ui_FonChooser ui;
    ui.setupUi(&dlg);

    for (const auto &rec : resources)
        ui.fResourceList->addItem(rec.fName);

    if (dlg.exec() == QDialog::Accepted && ui.fResourceList->currentRow() >= 0)
        return resources[ui.fResourceList->currentRow()];

    return ResRecord();
}
#endif

void plFontConverter::IImportFON(const plFileName &path)
{
    // TODO:  Get rid of the Windows-y stuff and parse this shit by hand

#if !defined(Q_OS_WIN) || defined(Q_OS_WIN64)
    QMessageBox::critical(this, tr("ERROR"),
            tr("FON import is currently only available on 32-bit Windows OSes"));
    return;
#else
    BOOL isWow64;
    if (IsWow64Process(GetCurrentProcess(), &isWow64) && isWow64) {
        // Nope, even WoW64 doesn't count...  Win64 can't import a Win16
        // image, regardless of whether we're running in 32- or 64-bit mode.
        QMessageBox::critical(this, tr("ERROR"),
                tr("FON import is currently only available on 32-bit Windows OSes"));
        return;
    }

    // FON files are really just resource modules
    IMakeNewFont();
    HMODULE file = LoadLibraryExA(path.AsString().c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | DONT_RESOLVE_DLL_REFERENCES);
    if (file == nullptr)
    {
        char msg[512];

        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg, sizeof(msg), nullptr);

        QMessageBox::critical(this, tr("Error"),
                tr("Failure importing FON file: can't open as resource library (%1)").arg(msg));
    }
    else
    {
        QList<ResRecord> resList;

        if (EnumResourceNames(file, "Font", ResEnumProc, (LPARAM)&resList))
        {
            // Put up a list of the resources so the user can choose which one
            ResRecord res = DoSelectResource(resList);
            if (res.fHandle != nullptr)
            {
                // Load the resource into a ram stream
                hsRAMStream stream;

                HGLOBAL glob = LoadResource(file, res.fHandle);
                if (glob != nullptr)
                {
                    void *data = LockResource(glob);
                    if (data != nullptr)
                    {
                        stream.Write(SizeofResource(file, res.fHandle), data);
                        stream.Rewind();

                        if (!fFont->LoadFromFNTStream(&stream))
                        {
                            QMessageBox::critical(this, tr("Error"),
                                    tr("Failure importing FON file: can't parse resource as FNT"));
                        }
                    }
                }
            }
        }
        else
        {
            char msg[512];

            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg, sizeof(msg), nullptr);

            QMessageBox::critical(this, tr("Error"),
                    tr("Failure importing FON file: can't enumerate resources (%1)").arg(msg));
        }

        FreeLibrary(file);
    }

    IUpdateInfo();
#endif
}

class NumListValidator : public QValidator
{
public:
    NumListValidator(QObject *parent = nullptr) : QValidator(parent) { }

    State validate(QString &input, int &pos) const override
    {
        for (int ch = 0; ch < input.size(); ++ch)
        {
            ushort theChar = input[ch].unicode();
            if ((theChar < '0' || theChar > '9') && (theChar != ' '))
                return Invalid;
        }
        return Acceptable;
    }
};

void plFontConverter::IBatchFreeType(const plFileName &path, void *init)
{
    plFontFreeType::Options info;
    if (init != nullptr)
        info = *reinterpret_cast<plFontFreeType::Options*>(init);

    QDialog dlg;
    Ui_FreeTypeBatch ui;
    ui.setupUi(&dlg);
    dlg.layout()->setSizeConstraint(QLayout::SetFixedSize);

    ui.fPointSizes->setValidator(new NumListValidator(&dlg));

    ui.fPointSizes->setText(QString::number(info.fSize));
    ui.fFontName->setText(path.GetFileNameNoExt().c_str());
    ui.fResolution->setValue(info.fScreenRes);
    ui.fMaxChar->setValue(info.fMaxCharLimit);
    if (info.fBitDepth == 1)
        ui.fMonochrome->setChecked(true);
    else
        ui.fGrayscale->setChecked(true);

    if (dlg.exec() == QDialog::Rejected)
        return;

    QStringList sSizes = ui.fPointSizes->text().split(' ', QT_SKIP_EMPTY_PARTS);
    QList<int> iSizes;
    for (const QString &s : sSizes)
        iSizes.append(s.toInt());

    info.fScreenRes = ui.fResolution->value();
    info.fMaxCharLimit = ui.fMaxChar->value();
    info.fBitDepth = (ui.fMonochrome->isChecked() ? 1 : 8);

    QString outPath = QFileDialog::getExistingDirectory(this,
                tr("Select a path to write the P2F fonts to:"),
                QDir::current().absolutePath(), QFileDialog::ShowDirsOnly);
    if (outPath.isEmpty())
        return;

    ST::string fontName = ui.fFontName->text().toUtf8().constData();
    ST::string destPath = outPath.toUtf8().constData();

    plMyBDFCallback callback(this);
    callback.NumChars(iSizes.size());

    for (int size : iSizes)
    {
        IMakeNewFont();
        plFontFreeType *ft2Convert = reinterpret_cast<plFontFreeType *>(fFont);

        info.fSize = size;
        if (!ft2Convert->ImportFreeType(path, &info, nullptr))
        {
            QMessageBox::critical(this, tr("ERROR"), tr("Failure converting TrueType file"));
            continue;
        }

        fFont->SetFace(fontName);
        plFileName fileName = plFileName::Join(destPath,
                        ST::format("{}-{}.p2f", fFont->GetFace(), fFont->GetSize()));
        hsUNIXStream stream;
        if (!stream.Open(fileName, "wb"))
            QMessageBox::critical(this, tr("ERROR"), tr("Can't open file for writing"));
        else
        {
            fFont->WriteRaw(&stream);
            stream.Close();
        }

        callback.CharDone();
    }

    IUpdateInfo();
}

void plFontConverter::IImportFreeType(const plFileName &path)
{
    enum { kBatchConvertAction = 100 };

    plFontFreeType::Options info;

    QDialog dlg;
    Ui_FreeType ui;
    ui.setupUi(&dlg);
    dlg.layout()->setSizeConstraint(QLayout::SetFixedSize);

    // Designer can't do this...
    QPushButton *batchButton = new QPushButton(tr("&Batch..."), ui.fButtons);
    ui.fButtons->addButton(batchButton, QDialogButtonBox::ResetRole);
    connect(batchButton, &QPushButton::clicked, &dlg,
            std::bind(&QDialog::done, &dlg, kBatchConvertAction));

    ui.fPointSize->setValue(info.fSize);
    ui.fResolution->setValue(info.fScreenRes);
    ui.fMaxChar->setValue(info.fMaxCharLimit);
    if (info.fBitDepth == 1)
        ui.fMonochrome->setChecked(true);
    else
        ui.fGrayscale->setChecked(true);

    int ret = dlg.exec();

    info.fSize = ui.fPointSize->value();
    info.fScreenRes = ui.fResolution->value();
    info.fMaxCharLimit = ui.fMaxChar->value();
    info.fBitDepth = (ui.fMonochrome->isChecked() ? 1 : 8);

    if (ret == QDialog::Rejected)
        return;
    else if (ret == kBatchConvertAction)
    {
        IBatchFreeType(path);
        return;
    }

    IMakeNewFont();
    plMyBDFCallback callback(this);

    plFontFreeType *ft2Convert = reinterpret_cast<plFontFreeType *>(fFont);
    if (!ft2Convert->ImportFreeType(path, &info, &callback))
        QMessageBox::critical(this, tr("ERROR"), tr("Failure converting TrueType file"));

    IUpdateInfo();
}

void plFontConverter::IOpenP2F(const plFileName &path)
{
    IMakeNewFont();
    if (!fFont->LoadFromP2FFile(path))
        QMessageBox::critical(this, tr("ERROR"), tr("Failure opening P2F file"));

    IUpdateInfo();
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("plFontConverter");
    app.setWindowIcon(QIcon(":/icon1.ico"));

    plResManager *rMgr = new plResManager;
    hsgResMgr::Init(rMgr);

    plFontConverter mainDialog;
    mainDialog.show();
    int retn = app.exec();

    hsgResMgr::Shutdown();

    return retn;
}
