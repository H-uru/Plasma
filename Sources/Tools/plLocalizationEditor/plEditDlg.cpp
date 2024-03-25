/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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
// Basic edit dialog stuff
#include "plEditDlg.h"
#include "plLocTreeView.h"
#include "plAddDlgs.h"
#include "res/ui_EditDialog.h"

#include "pfLocalizationMgr/pfLocalizationMgr.h"
#include "pfLocalizationMgr/pfLocalizationDataMgr.h"

#include <functional>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

static void IAboutDialog(QWidget *parent)
{
    QDialog dlg(parent);
    QLabel *image = new QLabel(&dlg);
    image->setPixmap(QPixmap(":/icon1.ico"));
    QLabel *text = new QLabel(QObject::tr(R"(plLocalizationEditor
A basic editor for Plasma 21 localization resource files
Copyright (C) 2004 Cyan Worlds, Inc.)"), &dlg);
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

EditDialog::EditDialog()
    : fEditMode(kEditNothing)
{
    fUI = new Ui_EditDialog;
    fUI->setupUi(this);

    connect(fUI->fOpenAction, SIGNAL(triggered()), SLOT(OpenDataDirectory()));
    connect(fUI->fSaveCurrentAction, SIGNAL(triggered()), SLOT(SaveToCurrent()));
    connect(fUI->fSaveOtherAction, SIGNAL(triggered()), SLOT(SaveToDirectory()));
    connect(fUI->fExitAction, SIGNAL(triggered()), SLOT(close()));
    connect(fUI->fAboutAction, &QAction::triggered, std::bind(&IAboutDialog, this));

    connect(fUI->fLocalizationTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(LocPathChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    connect(fUI->fAddButton, SIGNAL(clicked()), SLOT(AddClicked()));
    connect(fUI->fDeleteButton, SIGNAL(clicked()), SLOT(DeleteClicked()));

    EnableEdit(false);
}

EditDialog::~EditDialog()
{
    pfLocalizationMgr::Shutdown();
    delete fUI;
}

// saves the current localization text to the data manager
void EditDialog::SaveLocalizationText()
{
    if (fCurrentLocPath.empty())
        return; // no path to save

    ST::string text = fUI->fLocalizationText->toPlainText().toUtf8().constData();

    ST::string ageName, setName, elementName, elementLanguage;
    SplitLocalizationPath(fCurrentLocPath, ageName, setName, elementName, elementLanguage);

    if (ageName.empty() || setName.empty() || elementName.empty() || elementLanguage.empty())
        return;

    ST::string name = ST::format("{}.{}.{}", ageName, setName, elementName);
    pfLocalizationDataMgr::Instance().SetElementPlainTextData(name, elementLanguage, text);
}

void EditDialog::LoadLocalization(const ST::string &locPath)
{
    if (locPath == fCurrentLocPath)
        return;

    fCurrentLocPath = locPath;
    fUI->fTextPathLabel->setText(QString("&Text (%1):").arg(locPath.c_str()));

    ST::string ageName, setName, elementName, elementLanguage;
    SplitLocalizationPath(locPath, ageName, setName, elementName, elementLanguage);

    // now make sure they've drilled down deep enough to enable the dialog
    if (elementLanguage.empty()) // not deep enough
        EnableEdit(false);
    else
    {
        EnableEdit(true);
        ST::string key = ST::format("{}.{}.{}", ageName, setName, elementName);
        ST::string elementText = pfLocalizationDataMgr::Instance().GetElementPlainTextData(key, elementLanguage);
        fUI->fLocalizationText->setPlainText(elementText.c_str());
    }

    // now to setup the add/delete buttons
    if (!elementLanguage.empty()) // they have selected a language
    {
        fEditMode = kEditLocalization;
        fUI->fAddButton->setText(tr("&Add Localization"));
        fUI->fAddButton->setEnabled(true);
        fUI->fDeleteButton->setText(tr("&Delete Localization"));

        // don't allow them to delete the default language
        fUI->fDeleteButton->setEnabled(elementLanguage != "English");
    }
    else // they have selected something else
    {
        fEditMode = kEditElement;
        fUI->fAddButton->setText(tr("&Add Element"));
        fUI->fAddButton->setEnabled(true);
        fUI->fDeleteButton->setText(tr("&Delete Element"));
        if (!elementName.empty()) // they have selected an individual element
        {
            std::vector<ST::string> elementNames = pfLocalizationDataMgr::Instance().GetElementList(ageName, setName);

            // they can't delete the only subtitle in a set
            fUI->fDeleteButton->setEnabled(elementNames.size() > 1);
        }
        else
            fUI->fDeleteButton->setEnabled(false);
    }
}

void EditDialog::EnableEdit(bool enable)
{
    if (!enable)
        fUI->fLocalizationText->setPlainText("");

    fUI->fLocalizationText->setEnabled(enable);
}

void EditDialog::closeEvent(QCloseEvent *event)
{
    if (fCurrentSavePath.isEmpty()) // no data open
    {
        event->accept();
        return;
    }

    SaveLocalizationText(); // make sure any changed text is saved to the manager

    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Save Changes"),
                tr("Do you wish to save your changes?"),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (result == QMessageBox::Yes)
        SaveToDirectory();

    if (result == QMessageBox::Cancel)
        event->ignore();
    else
        event->accept();
}

void EditDialog::OpenDataDirectory()
{
    QSettings settings;
    QString path = settings.value("dataDir", QDir::current().absolutePath()).toString();

    path = QFileDialog::getExistingDirectory(this,
                tr("Select a localization data directory:"),
                path,
                QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);

    if (!path.isEmpty())
    {
        plWaitCursor waitCursor(this);

        pfLocalizationMgr::Shutdown();

        fCurrentSavePath = path;
        pfLocalizationMgr::Initialize(fCurrentSavePath.toUtf8().constData());
        settings.setValue("dataDir", path);

        fUI->fLocalizationTree->clear();
        fUI->fLocalizationTree->LoadData("");

        SetTitle(path);

        fUI->fSaveCurrentAction->setEnabled(true);
        fUI->fSaveOtherAction->setEnabled(true);
    }
}

void EditDialog::SaveToCurrent()
{
    SaveLocalizationText(); // make sure any changed text is saved to the manager

    // save it to our current directory
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("Save to Current Directory"),
                tr("Are you sure you want to save to the current directory? Current data will be overwritten!"),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (result == QMessageBox::Yes)
    {
        plWaitCursor waitCursor(this);
        pfLocalizationDataMgr::Instance().WriteDatabaseToDisk(fCurrentSavePath.toUtf8().constData());
    }
    else if (result == QMessageBox::No)
        SaveToDirectory();
    // and if it's cancel we don't do anything
}

void EditDialog::SaveToDirectory()
{
    SaveLocalizationText(); // make sure any changed text is saved to the manager

    QString path = QFileDialog::getExistingDirectory(this,
                tr("Select a directory to save the localization data to:"),
                fCurrentSavePath, QFileDialog::ShowDirsOnly);

    // save it to a new directory
    if (!path.isEmpty())
    {
        plWaitCursor waitCursor(this);

        fCurrentSavePath = path;

        QSettings settings;
        settings.setValue("dataDir", path);

        SetTitle(path);
        pfLocalizationDataMgr::Instance().WriteDatabaseToDisk(fCurrentSavePath.toUtf8().constData());
    }
}

void EditDialog::LocPathChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    SaveLocalizationText(); // save any current changes to the database
    LoadLocalization(fUI->fLocalizationTree->CurrentPath());
}

void EditDialog::AddClicked()
{
    SaveLocalizationText(); // save any current changes to the database

    if (fEditMode == kEditElement)
    {
        plAddElementDlg dlg(fCurrentLocPath, this);
        if (dlg.DoPick())
        {
            ST::string path = dlg.GetValue(); // path is age.set.name
            if (!pfLocalizationDataMgr::Instance().AddElement(path))
            {
                QMessageBox::critical(this, tr("Error"),
                    tr("Couldn't add new element because one already exists with that name!"));
            }
            else
            {
                fCurrentLocPath = "";
                fUI->fLocalizationTree->clear();
                fUI->fLocalizationTree->LoadData(path);
                LoadLocalization(path);
            }
        }
    }
    else if (fEditMode == kEditLocalization)
    {
        plAddLocalizationDlg dlg(fCurrentLocPath, this);
        if (dlg.DoPick())
        {
            ST::string newLanguage = dlg.GetValue();
            ST::string ageName, setName, elementName, elementLanguage;
            SplitLocalizationPath(fCurrentLocPath, ageName, setName, elementName, elementLanguage);
            ST::string key = ST::format("{}.{}.{}", ageName, setName, elementName);
            if (!pfLocalizationDataMgr::Instance().AddLocalization(key, newLanguage))
                QMessageBox::critical(this, tr("Error"), tr("Couldn't add additional localization!"));
            else
            {
                ST::string path = ST::format("{}.{}", key, newLanguage);
                fCurrentLocPath = ST::string();
                fUI->fLocalizationTree->clear();
                fUI->fLocalizationTree->LoadData(path);
                LoadLocalization(path);
            }
        }
    }
}

void EditDialog::DeleteClicked()
{
    SaveLocalizationText(); // save any current changes to the database

    QMessageBox::StandardButton reply =  QMessageBox::question(this, tr("Delete"),
            tr("Are you sure that you want to delete %1?").arg(fCurrentLocPath.c_str()));

    if (reply == QMessageBox::Yes)
    {
        if (fEditMode == kEditElement)
        {
            if (!pfLocalizationDataMgr::Instance().DeleteElement(fCurrentLocPath))
                QMessageBox::critical(this, tr("Error"), tr("Couldn't delete element!"));
            else
            {
                ST::string path = fCurrentLocPath;
                fCurrentLocPath = "";
                fUI->fLocalizationTree->clear();
                fUI->fLocalizationTree->LoadData(path);
                LoadLocalization(path);
            }
        }
        else if (fEditMode == kEditLocalization)
        {
            ST::string ageName, setName, elementName, elementLanguage;
            SplitLocalizationPath(fCurrentLocPath, ageName, setName, elementName, elementLanguage);
            ST::string key = ST::format("{}.{}.{}", ageName, setName, elementName);
            if (!pfLocalizationDataMgr::Instance().DeleteLocalization(key, elementLanguage))
                QMessageBox::critical(this, tr("Error"), tr("Couldn't delete localization!"));
            else
            {
                ST::string path = key + ".English";
                fCurrentLocPath = "";
                fUI->fLocalizationTree->clear();
                fUI->fLocalizationTree->LoadData(path);
                LoadLocalization(path);
            }
        }
    }
}

// split a subtitle path up into its component parts
void SplitLocalizationPath(const ST::string &path, ST::string &ageName,
        ST::string &setName, ST::string &locName, ST::string &locLanguage)
{
    ageName = setName = locName = locLanguage = ST::string();

    std::vector<ST::string> tokens = path.tokenize(".");
    if (tokens.size() >= 1)
        ageName = tokens[0];
    if (tokens.size() >= 2)
        setName = tokens[1];
    if (tokens.size() >= 3)
        locName = tokens[2];
    if (tokens.size() >= 4)
        locLanguage = tokens[3];
}
